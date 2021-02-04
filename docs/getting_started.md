# Gunslinger Getting Started Guide
This document is meant as a guide for quickly getting started writing gunslinger applications.

## Basic Example
Let's look at a basic example. In fact, it's the simplest gunslinger application you can create and run successfully.

```c
#define GS_IMPL
#include <gs.h>

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
   return (gs_app_desc_t){0};
}
```
Running this example gives the following result:

![hello_gs](https://raw.githubusercontent.com/MrFrenik/gs_examples/main/00_hello_gs/screenshot/screen.png)

*Default 800x600 resizable window titled "App"* 

Before using gunslinger, you must first define its implementation. By defining `GS_IMPL` in ONE source file before including `gs.h`, the gunslinger framework will be implemented and ready to use. 

## GS Main
The default main entry point for any gunslinger application is `gs_main()`. It expects you to return a `gs_app_desc_t` instance that describes attributes about 
the application you wish to create. It conveniently wraps all of the necessary framework initialization and startup for you.

```c
gs_app_desc_t gs_main(int32_t argc, char** argv)
```

It is possible to use gunslinger without this entry point by defining `GS_NO_HIJACK_MAIN` before implementing the framework: 

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
Gunslinger runs its own core loop and provides methods for hooking into your application at certain sync points. Your application is initialized at startup via a `gs_app_desc_t` object. It has the following fields: 
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

Gunslinger interfaces with your application via callbacks that you can register with your application descriptor. These are `init`, `update`, and `shutdown`. When creating a `gs_app_desc_t`, you can set these callbacks like so:

```c
void app_init() {
}

void app_update() {
}

void app_shutdown() {
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
   return (gs_app_desc_t){
      .init = app_init,
      .update = app_update,
      .shutdown = app_shutdown
   };
}
```

You can register a pointer to any application data you'd like to be globally accessible via the application descriptor `user_data` field: 

```c
typedef struct your_user_data {
   uint32_t u_val;
   float f_val;
} your_user_data;

gs_global user_data = {0};

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
   return (gs_app_desc_t){
      .user_data = &data;
   };
}
```

To access a mutable pointer to this data, you can use the `gs_engine_user_data(T)` macro, where `T` is the type of your data.

```c
void app_init() {
   your_user_data* data = gs_engine_user_data(your_user_data);
}
```



