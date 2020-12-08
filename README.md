# gunslinger

A simple c99 framework for multimedia applications

## Building

**NOTE(john)**: For all platforms, make certain you are in the `root` directory for `gunslinger` when attempting to build.

**windows (MSVC)**: 
  - You'll need to have Visual Studio 2015/2017. (Note: Currently 2019 is having compile issues that are being tracked down).
  - From start menu, search for "x64 Native Tool Command Prompt for {Insert your Version Here}"
  - Navigate to where you have `gunslinger` repo placed
  - In the **root** directory for `gunslinger`, run `proc\win\compile_win_cl.bat`
  - This will build the library and place it in `gunslinger\bin`
 
**windows (mingw)**:
  - You'll need to have mingw64 installed and the `mingw\bin` directory in your environment path.
  - From the ***root*** directory for `gunslinger`, open `bash`. 
  - Run `bash ./proc/win/compile_win_mingw.sh`
  - This will build the library and place it in `gunslinger/bin`

**osx (GCC)**: 
  - From terminal, run `bash ./proc/osx/compile_osx_gcc.sh`
  - This will build the library and place it in `gunslinger/bin`

**linux (GCC)**: 
  - From terminal, run `bash ./proc/linux/compile_linux_gcc.sh`
  - This will build the library and place it in `gunslinger/bin`

## Examples

**NOTE(john)**: Currently all examples require at least **OpenGL v3.3** to run.

There are multiple examples provided to show how to get up and running. For each of these examples: 
  - First, build the `gunslinger` library following the above instructions for your platform.
  - `cd` into the directory for your example
  - **windows** (msvc): 
    - Run `proc\win\compile_win_cl.bat`
    - The executable will be placed in `bin\{example_name}`
    - Run the executable
  - **windows** (mingw):
    - From `bash` terminal, run `bash ./proc/win/compile_win_mingw.sh`
    - The executable will be placed in `bin{example_name}`
    - Run the executable
  - **mac**:
    - Run `bash ./proc/osx/compile_osx_gcc.sh`
    - The exectuable will be placed in `bin/{example_name}`
    - Run the executable
  - **linux**: 
    - Run `bash ./proc/linux/compile_linux_gcc.sh`
    - The exectuable will be placed in `bin/{example_name}`
    - Run the executable
