# Gunslinger Library — Agent Reference

## Overview

Gunslinger is a **header-only C99 framework** for multimedia applications. It provides platform abstraction, graphics, audio, math, and generic containers in a single drop-in header with all externals bundled.

- **Upstream repo**: <https://github.com/MrFrenik/gunslinger>
- **Docsforge**: <https://gunslinger.docsforge.com/>
- **Examples**: <https://github.com/MrFrenik/gs_examples>
- **License**: MIT

---

## Directory Layout

```
third_party/include/gs/
├── gs.h                      # Main single-header API (~340 KB)
├── LICENSE                   # MIT License
├── README.md                 # Library readme
│
├── impl/                     # Internal implementation headers (included by gs.h)
│   ├── gs_audio_impl.h       # Audio backend (miniaudio)
│   ├── gs_graphics_impl.h    # Graphics backend (OpenGL/Vulkan/etc.)
│   └── gs_platform_impl.h    # Platform abstraction (windowing, input, file I/O)
│
├── util/                     # Optional utility headers — each requires its own _IMPL define
│   ├── gs_ai.h               # AI pathfinding/navigation
│   ├── gs_asset.h            # Asset management system
│   ├── gs_audxt.h            # Audio extensions
│   ├── gs_gfxt.h             # Graphics extensions
│   ├── gs_gui.h              # Immediate-mode GUI (ImGUI-style)
│   ├── gs_idraw.h            # OpenGL 2.0-style immediate-mode rendering
│   ├── gs_meta.h             # Meta-data reflection utility
│   ├── gs_physics.h          # Physics utilities
│   └── gs_vg.h               # Vector graphics
│
├── external/                 # Bundled third-party dependencies
│   ├── KHR/                  # Khronos OpenGL headers
│   ├── ccd/                  # Bullet collision detection
│   ├── cgltf/                # glTF loader
│   ├── dirent/               # POSIX dirent for Windows
│   ├── dr_libs/              # Dominic Rath's audio libs (dr_wav, dr_flac, etc.)
│   ├── glad/                 # OpenGL loader
│   ├── glfw/                 # Windowing/input library
│   ├── miniaudio/            # Audio engine
│   ├── minicoro/             # Lightweight coroutines
│   ├── sched/                # POSIX scheduling for Windows
│   ├── sg_noise/             # Simplex noise implementation
│   └── stb/                  # STB image/font libs
│
└── docs/                     # Documentation markdown
    ├── getting_started.md
    ├── graphics.md
    ├── containers.md
    ├── math.md               # (redirect)
    ├── audio.md              # (redirect)
    ├── platform.md           # (redirect)
    └── utils.md              # (redirect)
```

---

## Usage Rules

1. **`GS_IMPL` in exactly one `.c` file** — Define `GS_IMPL` before `#include <gs/gs.h>` in exactly one compilation unit per binary. This pulls in all implementation code. All other files include `<gs/gs.h>` without `GS_IMPL`.

2. **Include order for util headers** — Every util header requires core gunslinger to be included first:
   ```c
   #define GS_IMPL          // Only in one .c file
   #include <gs/gs.h>
   #include <gs/util/gs_gui.h>     // After gs.h
   ```

3. **Each util needs its own `_IMPL` define** — Optional utilities are compiled only when their corresponding macro is defined before inclusion:
   - `GS_IMMEDIATE_DRAW_IMPL` → `gs_idraw.h`
   - `GS_GUI_IMPL` → `gs_gui.h` (also requires `GS_IMMEDIATE_DRAW_IMPL`)
   - `GS_AI_IMPL` → `gs_ai.h`
   - `GS_GFXT_IMPL` → `gs_gfxt.h`
   - `GS_META_IMPL` → `gs_meta.h`

4. **`GS_NO_HIJACK_MAIN`** — By default, gunslinger provides a `main()` wrapper via `gs_main()`. Define `GS_NO_HIJACK_MAIN` when the project manages its own entry point.

5. **Graphics backend selection** — Choose a backend before `GS_IMPL`:
   - `GS_GRAPHICS_IMPL_OPENGL_CORE` — OpenGL Core
   - Other backends (Vulkan, Metal, DX12) are on the roadmap.

---

## Core Modules

| Module | Description |
|--------|-------------|
| **Application** | App lifecycle via `gs_main()` returning `gs_app_desc_t` with init/update/shutdown callbacks, or manual `gs_create()`/`gs_frame()` loop. |
| **Math** | vec2/vec3/vec4, quat, mat4 with standard operations: dot, cross, normalize, lookat, perspective, ortho, etc. |
| **Containers** | Generic containers via macros: `gs_dyn_array(T)`, `gs_hash_table(K,V)`, `gs_slot_array(T)`, `gs_slot_map(K,V)`, `gs_byte_buffer_t`, `gs_command_buffer_t`. |
| **Platform** | Window, input, time, file I/O abstraction. Cross-platform (Windows, macOS, Linux, Android, HTML5). |
| **Graphics** | Explicit rendering API: textures, shaders, vertex/index buffers, uniform buffers, pipelines, render passes, framebuffers. Inspired by sokol headers. |
| **Audio** | Audio playback via miniaudio backend. Supports multiple formats through bundled `dr_libs`. |

---

## Common Preprocessor Flags Reference

| Flag | Purpose |
|------|---------|
| `GS_IMPL` | **Required** — Must appear in exactly one `.c` file before including `gs.h`. Pulls in all implementation code. |
| `GS_NO_HIJACK_MAIN` | Don't provide a default `main()`; use manual `gs_create()`/`gs_frame()` lifecycle. |
| `GS_GRAPHICS_IMPL_OPENGL_CORE` | Select OpenGL Core as the graphics backend. |
| `GS_IMMEDIATE_DRAW_IMPL` | Enable `gs_idraw.h` (OpenGL 2.0-style immediate-mode rendering). |
| `GS_GUI_IMPL` | Enable `gs_gui.h` (immediate-mode GUI). Requires `GS_IMMEDIATE_DRAW_IMPL`. |
| `GS_AI_IMPL` | Enable `gs_ai.h` (AI pathfinding/navigation). |
| `GS_GFXT_IMPL` | Enable `gs_gfxt.h` (graphics extensions). |
| `GS_META_IMPL` | Enable `gs_meta.h` (meta-data reflection utility). |
| `GS_NO_OS_MEMORY_ALLOC_DEFAULT` | Disable default OS memory allocator; provide custom `gs_os_api_s`. |

---

## Common Pitfalls

- **Forgetting `GS_IMPL`** — The library will link but all functions will be undefined. Always verify exactly one `.c` file defines it per binary.
- **Wrong include order** — Util headers (`gs_gui.h`, `gs_idraw.h`, etc.) must come **after** `<gs/gs.h>`. They depend on core types and macros.
- **Missing `_IMPL` flag** — Each util header silently provides no implementation unless its corresponding `GS_*_IMPL` macro is defined in the same `.c` file that has `GS_IMPL`.
- **Container macro confusion** — Gunslinger containers use C macros (e.g., `gs_dyn_array(int)` creates a typedef). The type name is the macro result, not a separate struct.
- **GUI requires idraw** — `GS_GUI_IMPL` depends on `GS_IMMEDIATE_DRAW_IMPL`; both must be defined together.

---

## External References

- [GitHub Repository](https://github.com/MrFrenik/gunslinger)
- [Official Documentation (Docsforge)](https://gunslinger.docsforge.com/)
- [Getting Started Guide](https://github.com/MrFrenik/gunslinger/blob/master/docs/getting_started.md)
- [Graphics Docs](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md)
- [Containers Docs](https://github.com/MrFrenik/gunslinger/blob/master/docs/containers.md)
- [Example Projects](https://github.com/MrFrenik/gs_examples)
- [Project Template](https://github.com/MrFrenik/gs_project_template)
