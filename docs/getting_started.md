
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
