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

## Documentation

## Example
A simple 'Hello World' example using gunslinger: 

```c
#define GS_IMPL
#include <gs.h>

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
   return (gs_app_desc_t){0};
}
```

