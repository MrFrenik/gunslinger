# Gunslinger Getting Started Guide
This document is meant as a guide for quickly getting started writing `gunslinger` applications.

## Basic Example
Let's look at a basic example. In fact, it's the simplest `gunslinger` application you can create and run successfully.

```c
#define GS_IMPL
#include <gs.h>

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
   return (gs_app_desc_t){0};
}
```
Before using `gunslinger`, you must first define its implementation. By defining `GS_IMPL` in ONE source file before including `gs.h`, the `gunslinger` framework will be implemented and ready to use. 

## GS Main
The default main entry point for any `gunslinger` application is `gs_main()`. It expects you to return a `gs_app_desc_t` instance that describes attributes about 
the application you wish to create. 

```cgs_app_desc_t gs_main(int32_t argc, char** argv)```

`gs_main()` conveniently wraps all of the framework initialization and startup for you. It is possible to use `gunslinger` without this entry point by defining `GS_NO_HIJACK_MAIN` before implementing the framework: 

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
### Important Note: 
```
While it is possible to use gunslinger without it controlling the main application loop, this isn't recommended. 
Internally, gunslinger does its best to handle the boiler plate drudge work of implementing (in correct order) 
the various layers required for a basic hardware accelerated multi-media application program to work. This involves allocating 
memory for internal data structures for these layers as well initializing them in a particular order so they can inter-operate
as expected. If you're interested in taking care of this yourself, look at the `gs_engine_run()` function to get a feeling
for how this is being handled.
```

## Application Descriptor
Gunslinger runs its own core loop and provides methods for hooking into your application at certain sync points. Your application is initialized at startup via a `gs_app_desc_t` object. `gs_app_desc_t` has the following fields: 
```c
typedef struct gs_app_desc_t
{
    void (* init)();              // Initialization callback for the application
    void (* update)();            // Update callback for the application
    void (* shutdown)();          // Shutdown callback for the application
    const char* window_title;     // Title of main window
    uint32_t window_width;        // Width of main window
    uint32_t window_height;       // Height of main window
    uint32_t window_flags;        // Flags for the window (resizeable, fullscreen, borderless, etc.)          
    float frame_rate;             // Desired frame rate for application
    bool32 enable_vsync;          // Whether or not vsync is enabled
    bool32 is_running;            // Internal indicator for framework to know whether application should continue running
    void* user_data;              // Any user data for the application
} gs_app_desc_t;
```


Note: While it is possible to use `gunslinger` without it controlling the main application loop, this isn't recommended. 
Internally, `gunslinger` does its best to handle the boiler plate drudge work of implementing (in correct order) 
the various layers required for a basic hardware accelerated multi-media application program to work. This involves allocating 
memory for internal data structures for these layers as well initializing them in a particular order so they can inter-operate
as expected. If you're interested in taking care of this yourself, look at the `gs_engine_run()` function to get a feeling
for how this is being handled.
