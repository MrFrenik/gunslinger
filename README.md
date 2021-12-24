![gunslinger](https://raw.githubusercontent.com/MrFrenik/gunslinger/master/docs/gs_logo2.png)
![GitHub](https://img.shields.io/github/license/mrfrenik/gunslinger)
[![Discord](https://img.shields.io/discord/485178488203116567?label=discord&logo=discord)](https://discord.gg/QXwpETB)
![GitHub top language](https://img.shields.io/github/languages/top/mrfrenik/gunslinger?label=c99)

Gunslinger is a header-only c99 framework for multimedia applications.

## Features
- Header-only: drag-drop into any project without any additional compiling required.
- All externals included in the framework itself.
- Simple API inspired by [sokol](https://github.com/floooh/sokol) headers.
- Provides core framework for quickly developing multimedia applications: [Platform](https://github.com/MrFrenik/gunslinger/blob/master/docs/platform.md), [Graphics](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md), [Audio layers](https://github.com/MrFrenik/gunslinger/blob/master/docs/audio.md). 
- Provides custom utilities for [math](https://github.com/MrFrenik/gunslinger/blob/master/docs/math.md) and [generic data structures](https://github.com/MrFrenik/gunslinger/blob/master/docs/containers.md).
- Optional [helper utilties](https://github.com/MrFrenik/gunslinger/tree/master/util) are provided, such as [OpenGL 2.0-style immediate-mode rendering](https://github.com/MrFrenik/gunslinger/blob/master/util/gs_idraw.h), [asset management system](https://github.com/MrFrenik/gunslinger/blob/master/util/gs_asset.h), [physics util](https://github.com/MrFrenik/gunslinger/blob/master/util/gs_physics.h), and a [data reflection utility](https://github.com/MrFrenik/gunslinger/blob/master/util/gs_meta.h). 
- Supports a growing list of platforms: `Windows`, `OSX`, `Linux`, `Android`, and `HTML5` currently with plans to add `UWP`, `RPI`, `IOS`.
- Graphics pipeline follows an explicit rendering framework, making it easier to write for modern backends, such as `Vulkan`/`DX12`/`Metal`.
- All core layers can be fully swapped out with custom user implementations.
- Large collection of [examples](https://github.com/MrFrenik/gs_examples) for quickly getting started.
- An available [project template](https://github.com/MrFrenik/gs_project_template) for various platforms and build systems to get started with a blank gunslinger project.
- Official framework used for all [Game Engineering](https://www.youtube.com/watch?v=VLZjd_Y1gJ8&list=PLIozaEI1hFu3Cd0YJMwOBQKTKfe9uZoyn) YouTube videos.

## Documentation

* [Official Documentation (Docsforge)](https://gunslinger.docsforge.com/)
* [Online Sample Repo](https://mrfrenik.github.io/gunslinger)
* [Getting Started](https://github.com/MrFrenik/gunslinger/blob/master/docs/getting_started.md)
* [Containers](https://github.com/MrFrenik/gunslinger/blob/master/docs/containers.md)
* [Examples](https://github.com/MrFrenik/gs_examples)
* Platform: WIP
* Audio: WIP
* Graphics: WIP
* Math: WIP
* Utils: WIP

[//]: # "(* [Platform](https://github.com/MrFrenik/gunslinger/blob/master/docs/platform.md) )"
[//]: # "(* [Graphics](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md) )" 
[//]: # "(* [Audio](https://github.com/MrFrenik/gunslinger/blob/master/docs/audio.md) )" 
[//]: # "(* [Math](https://github.com/MrFrenik/gunslinger/blob/master/docs/math.md) )" 
[//]: # "(* [Utils](https://github.com/MrFrenik/gunslinger/blob/master/docs/utils.md) )" 

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
## Roadmap

* Support Android/iOS/RPI/UWP backends
* Support Vulkan/Metal/DX12 backends
* Add support for more texture sampler types: 1D, 3D, Cube Maps, Arrays
* Material Asset utils
* Language Ports: (Python, JS, Rust, C#)
* Add platform-independent threading utils to framework
* MRT
* Job System Util
* Write more docs for github
* Hot-reload util
* Remove all externals from core framework
* Add more texture formats (including compressed)
