# gunslinger

A simple c99 framework for multimedia applications

# Building

windows: 
  - You'll need to have Visual Studio 2015 or greater.
  - From start menu, search for "x64 Native Tool Command Prompt for {Insert your Version Here}"
  - Navigate to where you have `gunslinger` repo placed
  - In the root directory for `gunslinger`, run `proc\win\compile_win_cl.bat`
  - This will build the library and place it in `gunslinger\bin`
  
 osx: 
  - From terminal, run `./bash proc/osx/compile_osx_gcc.sh`
  - This will build the library and place it in `gunslinger/bin`
  
 linux: 
  - From terminal, run `./bash proc/osx/compile_linux_gcc.sh`
  - This will build the library and place it in `gunslinger/bin`

# Examples

NOTE(): Currently all examples require at least OpenGL v3.3 to run.

There are multiple examples provided to show how to get up and running. For each of these examples: 
  - First, build the `gunslinger` library following the above instructions for your platform.
  - 'cd' into the directory for your example
  - windows: 
    - run `proc\win\compile_win_cl.bat`
    - The executable will be placed in `bin\{example_name}`
    - Run the executable
  - mac: 
    - run `proc/osx/compile_osx_gcc.sh`
    - The exectuable will be placed in `bin/{example_name}`
    - Run the executable
  - linux: 
    - run `proc/osx/compile_linux_gcc.sh`
    - The exectuable will be placed in `bin/{example_name}`
    - Run the executable
