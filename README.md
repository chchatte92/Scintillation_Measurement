# Scintillation_Measurement
- git-clone:
  ```
  - git clone git@github.com:chchatte92/Scintillation_Measurement.git
  ```
- build:
  ```
  - cd Scintillation_Measurement
  - mkdir -p Output
  - cmake -B ./build -S . -Wno-dev
  - cmake --build ./build -j4
  ```
If builds successfully, run the executable:
```
./build/C2F6_Transparency
```
- The executable:
-> Should Generate a root output ('C2F6Transparency.root') inside the Output directory.
Settings.cmake file can be handled to change configurations. 

- You can find the run details here: [April CERN measurements run logbook](https://docs.google.com/spreadsheets/d/1BZJ2kLKJr1z_9nY-j9yVysaOfbZ1GfAg/edit?gid=1555539881#gid=1555539881)
