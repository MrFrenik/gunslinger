# gunslinger
![GitHub](https://img.shields.io/github/license/mrfrenik/gunslinger)
[![Discord](https://img.shields.io/discord/485178488203116567?label=discord&logo=discord)](https://discord.gg/QXwpETB)
![GitHub top language](https://img.shields.io/github/languages/top/mrfrenik/gunslinger?label=c99)

Gunslinger is a fast, lightweight, ([stb-style](https://github.com/nothings/stb)) header-only c99 framework for multimedia applications. Features include: 
- Header-only: drag-drop into any project without any additional compiling required.
- No external dependencies required. Everything is included in the framework itself.
- Provides core framework for quickly developing multimedia applications: Platform, Graphics, Audio layers. 
- Provides custom utilities for math and generic data structures.
- Large collection of ([examples](https://github.com/MrFrenik/gs_examples)) for quickly getting started.
- Supports a growing list of platforms: Windows, OSX, Linux.

## Documentation

* [Getting Started]()
* [Platform]()
* [Graphics]()
* [Audio]()
* [Math]()
* [Containers]()
* [Utils]()

## Example
A simple c99 'Hello World' example using gunslinger: 

```c
#define GS_IMPL
#include <gs.h>

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
   return (gs_app_desc_t){0};
}
```

Which gives this result (win32): 

[![hello_gs](https://raw.githubusercontent.com/MrFrenik/gs_examples/main/00_hello_gs/screenshot/screen.png)]

This does several things: 
* Before using `gunslinger`, the application defines `GS_IMPL` in ONE source file before including the framework.
* The main function returns a `gs_app_desc_t` for `gunslinger`, which gives various hints about your application: 
   * window title, 
   * window width 
   * window flags (resizeable, full screen, etc.)
   * desired frame rate
* Creates a graphics context with the specified graphics backend. OpenGL modern is provided and selected by default.
* Creates a platform context with your operating system as well as a window. 
* Initializes the `gs_engine_t` instance for the application, which is accessible globally via the `gs_engine_instance()` macro.

Gunslinger's main entry point into your application is via `gs_main()`. This conveniently wraps all of the framework initialization and startup for you. It is possible
to use `gunslinger` without this entry point by defining `GS_NO_HIJACK_MAIN` before implementing the framework: 

```c
#define GS_NO_HIJACK_MAIN
#define GS_IMPL
#include <gs.h>

int32_t main(int32_t argc, char** argv)
{
   gs_app_desc_t desc = {0};      // Fill this with whatever the application needs
   gs_engine_create(app)->run();  // Create framework instance and run application
   return 0;
}
```

Note: While it is possible to use `gunslinger` without it controlling the main application loop, this isn't recommended. 
Internally, `gunslinger` does its best to handle the boiler plate drudge work of implementing (in correct order) 
the various layers required for a basic hardware accelerated multi-media application program to work. This involves allocating 
memory for internal data structures for these layers as well initializing them in a particular order so they can inter-operate
as expected. If you're interested in taking care of this yourself, look at the `gs_engine_run()` function to get a feeling
for how this is being handled.

