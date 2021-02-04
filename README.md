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
- Supports a growing list of platforms: Windows, OSX, Linux currently with plans to add Android, RPI, IOS, and HTML5.
- Official framework used for all [Game Engineering](https://www.youtube.com/watch?v=VLZjd_Y1gJ8&list=PLIozaEI1hFu3Cd0YJMwOBQKTKfe9uZoyn&index=1&t=0s) YouTube videos.

## Documentation

* [Getting Started](https://github.com/MrFrenik/gunslinger/blob/master/docs/getting_started.md)
* [Platform](https://github.com/MrFrenik/gunslinger/blob/master/docs/platform.md)
* [Graphics](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md)
* [Audio](https://github.com/MrFrenik/gunslinger/blob/master/docs/audio.md)
* [Math](https://github.com/MrFrenik/gunslinger/blob/master/docs/math.md)
* [Containers](https://github.com/MrFrenik/gunslinger/blob/master/docs/containers.md)
* [Utils](https://github.com/MrFrenik/gunslinger/blob/master/docs/utils.md)
* [Examples](https://github.com/MrFrenik/gs_examples)

## Basic Example
A simple c99 'Hello World' example using gunslinger: 

```c
#define GS_IMPL
#include <gs.h>

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
   return (gs_app_desc_t){0};
}
```

