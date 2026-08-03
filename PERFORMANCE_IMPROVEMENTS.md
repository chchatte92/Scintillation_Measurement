# Performance improvements and proposed changes

This document collects all proposed changes to improve runtime and memory performance in the Scintillation_Measurement repository. It is based on a review of src/Process.cpp and src/main.cpp and covers hotspots, rationale, and concrete code suggestions and examples you can apply incrementally.

---

## Summary (top priorities)

1. Measure first: add lightweight timing to identify real hotspots.
2. Avoid unnecessary copies of large vectors (use std::move or references/shared_ptr).
3. Reserve vector capacity before push_back when possible.
4. Reduce the number of small ROOT writes and object creations (batch/write once, reuse TGraph/TTree objects).
5. Parse files faster and consider parallel parsing where safe.
6. Use unordered_map for O(1) lookups if appropriate.

---

## Detailed proposed changes

### 1) Add timers / profiling helpers (measure before optimizing)

Why: Do not optimize blind—measure I/O vs parsing vs ROOT writes.

What to add: a small utility that uses std::chrono to time phases. Example usage in ReadData, ReadBkg, ReadCalibration and BookPlots.

Example snippet (C++):

```cpp
// simple Timer helper (put in a small header e.g. include/Timer.h)
#include <chrono>
struct Timer {
  using clock = std::chrono::steady_clock;
  clock::time_point start;
  Timer():start(clock::now()){}
  double elapsed_ms(){
    return std::chrono::duration<double, std::milli>(clock::now()-start).count();
  }
};

// usage in code:
Timer t;
// ... work ...
std::cout << "ReadData took " << t.elapsed_ms() << " ms\n";
```

Suggested places to instrument: directory scanning, per-file parse, total parsing, ROOT object creation & Write.

---

### 2) Avoid expensive vector copies — use move semantics

Where: In ReadData and other places you do `m_treeSignal = m_dataReadings;` (copies entire vector).

Change to:

```cpp
m_treeSignal = std::move(m_dataReadings);
// and then clear m_dataReadings or reinitialize as needed
m_dataReadings.clear();
```

Rationale: Avoids O(N) copy per file; move is O(1) pointer swap.

Also change assignments like `m_treeBkg1 = it->second.bg1;` to use const references or shared_ptrs (see next section).

---

### 3) Reserve capacity for vectors to avoid repeated reallocations

Where: Before reading a file you clear vectors and then push_back in a loop.

Change pattern to reserve a heuristic capacity. If you can estimate lines or expect a typical size, reserve to reduce reallocations.

Example:

```cpp
m_dataWL.clear(); m_dataReadings.clear();
// heuristic reserve — tune (example 2048)
m_dataWL.reserve(2048);
m_dataReadings.reserve(2048);
```

If you can cheaply count lines (e.g., single pass or using file size / average line length), use that to set reserve precisely.

---

### 4) Avoid copying background vectors — store shared ownership

Problem: m_backgrounds stores BackgroundData with vectors; copying these into per-file members is expensive.

Change options:
- Store shared_ptr<std::vector<double>> in BackgroundData and assign shared_ptrs instead of copies.
- Or store BackgroundData in-place and use const references (or std::span) in the per-file processing step.

Example change to BackgroundData (conceptual):

```cpp
struct BackgroundData{
  int nFiles;
  std::shared_ptr<std::vector<double>> wl;
  std::shared_ptr<std::vector<double>> bg1;
  std::shared_ptr<std::vector<double>> bg2;
};

// when building:
bg.wl = std::make_shared<std::vector<double>>(std::move(local_wl));
// then in ReadData use `m_treeBkg1_ptr = bg.bg1;` (pointer/reference)
```

Or use `std::unordered_map<std::string, BackgroundData>` and in ReadData hold `const auto& bg = it->second;` then reference bg.bg1 without copy.

---

### 5) Use unordered_map for frequent lookups

If m_normalization and m_backgrounds are currently std::map (balanced tree), switching to std::unordered_map<string, ...> or unordered_map<int, ...> reduces lookup costs to average O(1).

Example:

```cpp
// prefer:
std::unordered_map<int,double> m_normalization;
std::unordered_map<std::string, BackgroundData> m_backgrounds;
```

---

### 6) Faster text parsing

Problem: operator>> and stoi/strtod are convenient but not the fastest for large files.

Options:
- Use getline + strtod/strtol on char* pointers for faster parsing.
- Use a fast number parsing library (fast_float).
- For simple formats ("double double" per line), scanning the buffer with strtod will outperform operator>>.

Example snippet (C style, faster):

```cpp
std::string line;
while (std::getline(input, line)){
  char *end;
  double x = strtod(line.c_str(), &end);
  double y = strtod(end, nullptr);
  // use x,y
}
```

Measure before/after.

---

### 7) Parallelize file parsing (thread pool)

Where: ReadBkg and ReadData iterate many independent files.

Approach:
- Parse files in parallel threads (std::async or a thread pool). Each thread reads and produces an in-memory result (vector<double>) for that file.
- Aggregate results in a single thread for ROOT writes (ROOT I/O is often not fully thread-safe). Write/Fill/ROOT operations should generally happen single-threaded unless using thread-aware ROOT features.

Example sketch:

```cpp
// pseudo
std::vector<std::future<FileResult>> futures;
for (auto& txtFile: txtFiles)
  futures.push_back(std::async(std::launch::async, ParseFile, txtFile));

// collect
for (auto &f: futures){
  FileResult r = f.get();
  // move r into main containers
}
```

Be careful: limit concurrency to hardware concurrency and watch memory.

---

### 8) Reduce ROOT churn: reuse graphs & batch writes

Problems seen: creating many TGraph objects, calling SetPoint in loops, calling Write per-graph, and frequent m_file->cd().

Recommendations:

- Reuse a single TGraph object for similar graphs (clear it or reuse points) instead of creating a new unique_ptr every file.
- Avoid calling `m_file->cd("...")` repeatedly — call it once per batch or hold a TDirectory* for repeated writes.
- Write fewer objects: for many small graphs, consider storing arrays or a single TTree with metadata rather than many TGraph objects.
- Adjust compression (TFile::SetCompressionSettings) and TTree basket size to improve throughput.

Example reuse pattern (conceptual):

```cpp
m_gr_data_Wl->Set(0); // reset number of points if supported by TGraph API
int i = 0;
for(...) { m_gr_data_Wl->SetPoint(i++, x, y); }
// change name/title then Write()
```

If TGraph lacks a direct Set(0) in your ROOT version, reuse by creating it once and calling `Delete()` of the internal arrays or recreate only when necessary.

---

### 9) Avoid excessive logging in hot loops

Calls to std::cout or printf in loops slow things and can cause I/O blocking. Buffer/log summary info at the end or throttle logs.

---

### 10) Minor micro-optimizations

- Avoid repeated .string() allocations and substr() + stoi when extracting run id; parse digits manually to int or parse from filename with minimal allocations.
- Combine repeated calculations (e.g., energy conversion) if used multiple times — compute once and reuse.
- Standardize on a single logging method to avoid iostream/stdio sync costs.

---

## Suggested incremental roadmap (small steps you can run & measure)

1. Add basic timers around ReadBkg, ReadData, ReadCalibration, and BookPlots and run with real dataset.
2. Replace vector copies with std::move where the source is temporary (m_treeSignal = std::move(m_dataReadings)).
3. Reserve vector capacities with a conservative heuristic and measure.
4. Change container type to unordered_map for m_normalization and m_backgrounds if currently std::map.
5. Replace expensive stoi(substr(3)) parsing with fast integer parse (manual or using std::from_chars).
6. Rework ROOT writing: change to write once per run or batch; reuse graphs; minimize cd()/Write.
7. If still I/O-bound, add parallel parsing (std::async/thread pool) and keep ROOT writes single-threaded.
8. If parsing CPU-bound, consider replacing operator>> with getline+strtod or fast_float.

---

## Example concrete patches to apply (small, safe)

1) Use std::move when assigning m_treeSignal

Replace in ReadData:
```cpp
m_treeSignal = m_dataReadings;
```
with:
```cpp
m_treeSignal = std::move(m_dataReadings);
// prepare m_dataReadings for next use
m_dataReadings.clear();
```

2) Reserve vectors before push_back in ReadData and ReadBkg

Add after clearing:
```cpp
constexpr size_t kDefaultReserve = 2048; // tune
m_dataWL.reserve(kDefaultReserve);
m_dataReadings.reserve(kDefaultReserve);
```

3) Use std::from_chars for faster integer parse of run id

Replace:
```cpp
m_runID = std::stoi(runName.substr(3));
```
with:
```cpp
// assumes runName like "Run123"
int id = 0;
std::from_chars(runName.data()+3, runName.data()+runName.size(), id);
m_runID = id;
```

(Include <charconv>)

4) Prefer unordered_map declarations (example)

```cpp
#include <unordered_map>
std::unordered_map<int,double> m_normalization;
std::unordered_map<std::string, BackgroundData> m_backgrounds;
```

5) Add Timer instrumentation (example above). Put Timer.h in include/ and include where needed.

---

## Root-specific tuning suggestions

- Set TFile compression to a lower level during processing if speed is more important than size.
  Example: `m_file->SetCompressionSettings(1, 0);` or `TFile::SetCompressionSettings(ROOT::kZLIB, 1);` — check ROOT API for version-specific calls.
- Adjust TTree basket sizes: `tree->SetBasketSize(buffer_size)` to match typical event sizes.
- Use `TTree::AutoSave("FlushBaskets")` frequency to reduce per-fill overhead.

Measure before/after these settings.

---

## Risks & notes

- Parallel parsing: be careful with thread-safety of any global state and ROOT objects; keep ROOT I/O single-threaded.
- Moving vectors invalidates the moved-from container; ensure you reinitialize it before reuse.
- Using unordered_map increases memory usage slightly vs tree map.

---

## Next steps I can take for you

- Create minimal patches for the low-risk changes (std::move, reserve, from_chars, timers) and push as a commit.
- Add a tiny benchmark driver that reads a sample data directory and reports timings.
- Implement a thread-pool based parser prototype and a serialization step for ROOT writing.

Tell me which of these you want me to implement next and I will prepare a patch/PR.
