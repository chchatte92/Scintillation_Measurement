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
If builds succesfully, run the executable:
```
./build/C2F6_Transparency
```
The excutable:
-> Should Generate a root output ('C2F6Transparency.root') inside the Output directory.
Settings.cmake file can be handled to change configurations. 
