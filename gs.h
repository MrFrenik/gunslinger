/*==============================================================================================================
    * Copyright: 2020 John Jackson 
    * Gunslinger: A simple, header-only c99 multi-media framework
    * File: gs.h
    * Github: https://github.com/MrFrenik/gunslinger

    All Rights Reserved

    BSD 3-Clause License

    Copyright (c) 2020 John Jackson

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its contributors may be used to 
    endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIEDi
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=================================================================================================================*/

#ifndef __GS_INCLUDED_H__
#define __GS_INCLUDED_H__

/*═█═════════════════════════════════════════════════════════════════════════════════════█═╗
████ ██████╗ ██╗   ██╗███╗   ██╗███████╗██╗     ██╗███╗   ██╗ ██████╗ ███████╗██████╗ ██████═█
█║█ ██╔════╝ ██║   ██║████╗  ██║██╔════╝██║     ██║████╗  ██║██╔════╝ ██╔════╝██╔══██╗ ██═████
███ ██║  ███╗██║   ██║██╔██╗ ██║███████╗██║     ██║██╔██╗ ██║██║ ███╗█████╗  ██████╔╝ █████═██
╚██ ██║   ██║██║   ██║██║╚██╗██║╚════██║██║     ██║██║╚██╗██║██║   ██║██╔══╝  ██╔══██╗ ███ █╝█
█║█ ╚██████╔╝╚██████╔╝██║ ╚████║███████║███████╗██║██║ ╚████║╚██████╔╝███████╗██║  ██║ ██═████
████ ╚═════╝  ╚═════╝ ╚═╝  ╚═══╝╚══════╝╚══════╝╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝ █═█═██
╚═██════════════════════════════════════════════════════════════════════════════════════██═╝*/

/*
    Gunslinger is a header-only, c99 framework for multi-media applications and tools.

    USAGE: (IMPORTANT)

    =================================================================================================================

    Before including, define the gunslinger implementation like this:

          #define GS_IMPL

    in EXACTLY ONE C or C++ file that includes this header, BEFORE the
    include, like this:

          #define GS_IMPL
          #include "gs.h"

    All other files should just #include "gs.h" without the #define.

    (Thanks to SB for the template for this instructional message. I was too lazy to come up with my own wording.)

    NOTE: 
        All main interface stuff in here, then system implementations can be in separate header files
        All provided implementations will be in impl/xxx.h
        This is just to keep everything from being in one huge file

    ================================================================================================================

    Contained within (Contents):
        * GS_APPLICATION
        * GS_UTIL
        * GS_CONTAINERS
        * GS_ASSET_TYPES
        * GS_MATH
        * GS_PLATFORM
        * GS_GRAPHICS
        * GS_AUDIO

    ================================================================================================================

    GS_APPLICATION:

    Gunslinger is a framework that expects to take flow control over your app and calls into your code
    at certain sync points. These points are 'init', 'update', and 'shutdown'. When creating your application, 
    you provide certain information to the framework via a `gs_app_desc_t` descriptor object. Gunslinger acts as the
    main entry point to your application, so instead of defining "main" as you typically would for a c/c++ program, 
    you instead implement the `gs_main` function that will be called by gunslinger. This last bit is optional and 
    a different method for entry is covered, however this is the most convenient way to use the framework.

    Basic Example Application:

        #define GS_IMPL
        #include "gs.h"

        void my_init() {
            // Do your initialization   
        }

        void my_update() {
            // Do your updates  
        }

        void my_shutdown() {
            // Do your shutdown
        }

        gs_app_desc_t gs_main(int32_t argc, char** argv) {
            return (gs_app_desc_t) {
                .init = my_init,        // Function pointer to call into your init code
                .update = my_update,    // Function pointer to call into your update code
                .shutdown = my_shutdown // Function pointer to call into your shutdown code
            };
        }

    If you do not provide information for any of this information, defaults will be provided by the framework.
    Therefore, it is possible to return a completely empty app descriptor back to the framework to run:

        gs_app_desc_t gs_main(int32_t argc, char** argv) {
            return (gs_app_desc_t){0};
        }

    NOTE: 
        In addition to function callbacks, there are quite a few options available to provide for this descriptor, 
        such as window width, window height, window flags, window title, etc. Refer to the section for gs_app_desc_t 
        further in the code for all provided options.

    It's also possible to define GS_NO_HIJACK_MAIN for your application. This will make it so gunslinger will not be
    the main entry point to your application. You will instead be responsible for creating an engine instance and 
    passing in your application description to it.

        #define GS_NO_HIJACK_MAIN

        int32_t main(int32_t argv, char** argc)
        {
            gs_app_desc_t app = {0};        // Fill this with whatever your app needs
            gs_engine_create(app)->run();   // Create instance of engine for framework and run
            return 0;
       }

    NOTE: 
        Lastly, while it is possible to use gunslinger without it controlling the main application loop, this isn't recommended. 
        Internally, gunslinger does its best to handle the boiler plate drudge work of implementing (in correct order) 
        the various layers required for a basic hardware accelerated multi-media application program to work. This involves allocating 
        memory for internal data structures for these layers as well initializing them in a particular order so they can inter-operate
        as expected. If you're interested in taking care of this yourself, look at the `gs_engine_run()` function to get a feeling
        for how this is being handled.

    GS_MATH:

        Gunslinger includes utilities for common graphics/game related math structures. These aren't required to be used, however they are 
        used internally for certain function calls in various APIs. Where possible, I've tried to add as much redundancy as possible to
        the APIs so that these structures are not required if you prefer.

        * common utils:
            - interpolation

        * Vectors:
            - gs_vec2: float[2]
            - gs_vec3: float[3]
            - gs_vec4: float[4]
                * vector ops: 
                    - dot product
                    - normalize
                    - add/subtract (component wise)
                    - multiply
                    - cross product
                    - length
                
        *Quaternions:
            - gs_quat: float[4]
                * quaternion ops:
                    - add/subtract/multiply
                    - inverse
                    - conjugate
                    - angle/axis | axis/angle
                    - to euler | from euler
                    - to mat4

        *Mat4x4: 
            - gs_mat4: float[16]
                * mat4 ops:
                    - add/subtract/multiply
                    - transpose
                    - inverse
                    - homogenous transformations:
                        - rotation 
                        - scale
                        - translate
                    - view: 
                        - lookat
                    - projection: 
                        - orthographic
                        - perspective

        *VQS:
            - gs_vqs:  gs_vec3, gs_quat, gs_vec3

        (SPECIAL NOTE): 

        `gs_vqs` is a transform structure that's commonly used in games/physics sims, especially with complex child/parent hierarchies. It stands for
        `Vector-Quaternion-Scalar` to denote its internal components. Typically this encodes position, rotation (in quaternion), and a uniform scale
        for transformation. Gunslinger uses non-uniform scale in the form of a gs_vec3. 

            gs_vqs xform = {0};
            xform.position = gs_v3(...);
            xform.rotation = gs_quat(...);
            xform.scale = gs_v3(...);

        This structure can then be converted into a final form float[16] mat4x4 for any typical homogenous graphics transformations:

            gs_mat4 model = gs_vqs_to_mat4(&xform);

        The real power in VQS transforms is the ability to easily encode parent/child hierarchies. This is done using the two functions
        `gs_vqs_absolute_transform` and `gs_vqs_relative_transform`: 

            gs_vqs parent = ...;
            gs_vqs child =  ...; {

            gs_vqs relative = gs_vqs_relative_transform(&child, &parent);   // Get relative transform with respect to parent
            gs_vqs absolute = gs_vqs_absolute_transform(&local, &parent);   // Get absolute transform with respect to local

    GS_UTIL:
        
        memory allocation 
        hashing(32/64 bit)
        siphash (hash generic bytes)
        string utils
        file utils

    GS_CONTAINERS:

        gs_dyn_array: 

            Inspired GREATLY from Shawn Barret's "stretchy buffer" implementation. A generic, dynamic array of type T, 
            which is defined by the user:

                gs_dyn_array(float) arr = NULL;     // Dynamic array of type float

            This works by using the macro `gs_dyn_array(T)`, which evaluates to `T*`. The dynamic array stores a bit of header information
            right before the actual array in memory for a table to describe properties of the array: 

                [header][actual array data]

            The header is a structure of uint32_t[2]: 

                typedef struct gs_array_header_t {
                    uint32_t size;
                    uint32_t capacity;
                } gs_array_header_t;

            The array can be randomly accessed using the [] operator, just like any regular c array. There are also provided functions for accessing 
            information using this provided table. This dynamic structure is the baseline for all other containers provided in gunslinger.

            Array Usage:

                gs_dyn_array(float) arr = NULL;             // Create dynamic array of type float.
                gs_dyn_array_push(arr, 10.f);               // Push value into back of array. Will dynamically grow/initialize on demand.
                float v = arr[0];                           // Gets value of array data at index 0;
                float* vp = &arr[0];                        // Gets pointer reference of array data at index 0;
                uint32_t sz = gs_dyn_array_size(arr);       // Gets size of array. Return 0 if NULL.
                uint32_t cap = gs_dyn_array_capacity(arr);  // Gets capacity of array. Return 0 if NULL.
                bool is_empty = gs_dyn_array_empty(arr);    // Returns whether array is empty. Return true if NULL.
                gs_dyn_array_reserve(arr, 10);              // Reserves internal space in the array for N, non-initialized elements.
                gs_dyn_array_clear(arr);                    // Clears all elements. Simply sets array size to 0.
                gs_dyn_array_free(arr);                     // Frees array data calling `gs_free` internally.

        gs_hash_table: generic hash table of key:K and val:V

            Inspired GREATLY from Shawn Barret's "stb_ds.h" implementation. A generic hash table of K,V which is defined 
            by the user:

                gs_hash_table(uint32_t, float) ht = NULL;       // Creates a hash table with K = uint32_t, V = float

            Internally, the hash table uses a 64-bit siphash to hash generic byte data to an unsigned 64-bit key. This means it's possible to pass up
            arbitrary data to the hash table and it will hash accordingly, such as structures:

                typedef struct key_t {
                    uint32_t id0;
                    uint64_t id1;
                } key_t;

                gs_hash_table(key_t, float) ht = NULL;  // Create hash table with K = key_t, V = float

            Inserting into the array with "complex" types is as simple as: 

                key_t k = {.ido0 = 5, .id1 = 32};   // Create structure for "key"
                gs_hash_table_insert(ht, k, 5.f);   // Insert into hash table using key
                float v = gs_hash_table_get(ht, k); // Get data at key

            It is possible to return a reference to the data using `gs_hash_table_getp()`. However, keep in mind that this comes with the
            danger that the reference could be lost IF the internal data array grows or shrinks in between you caching the pointer 
            and using it.

                float* val = gs_hash_table_getp(ht, k);    // Cache pointer to internal data. Dangerous game.
                gs_hash_table_insert(ht, new_key);         // At this point, your pointer could be invalidated due to growing internal array.

            Hash tables provide iterators to iterate the data:

                for (
                    gs_hash_table_iter it = 0; 
                    gs_hash_table_iter_valid(ht, it);
                    gs_hash_table_iter_advance(ht, it) 
                ) {
                    float v = gs_hash_table_iter_get(ht, it);         // Get value using iterator
                    float* vp = gs_hash_table_iter_getp(ht, it);      // Get value pointer using iterator
                    key_t k = gs_hash_table_iter_get_key(ht, it);     // Get key using iterator
                    key_t* kp = gs_hash_table_iter_get_keyp(ht, it);  // Get key pointer using iterator
                }

            Hash Table Usage:

                gs_hash_table(uint32_t, float) ht = NULL;       // Create hash table with key = uint32_t, val = float
                gs_hash_table_insert(ht, 64, 3.145f);           // Insert key/val pair {64, 3.145f} into hash table. Will dynamically grow/init on demand. 
                bool exists = gs_hash_table_key_exists(ht, 64); // Use to query whether or not a key exists in the table.
                float v = gs_hash_table_get(ht, 64);            // Get value at key = 64. Will crash if not available. 
                float* vp = gs_hash_table_get(ht, 64);          // Get pointer reference to data at key = 64. Will crash if not available. 
                bool is_empty = gs_hash_table_empty(ht);        // Returns whether hash table is empty. Returns true if NULL.
                uint32_t sz = gs_hash_table_size(ht);           // Get size of hash table. Returns 0 if NULL.               
                uint32_t cap = gs_hash_table_capacity(ht);      // Get capacity of hash table. Returns 0 if NULL.
                gs_hash_table_clear(ht);                        // Clears all elements. Sets size to 0.
                gs_hash_table_free(ht);                         // Frees hash table internal data calling `gs_free` internally.

        gs_slot_array: 

            Slot arrays are internally just dynamic arrays but alleviate the issue with losing references to internal 
            data when the arrays grow. Slot arrays therefore hold two internals arrays: 

                gs_dyn_array(T)        your_data;
                gs_dyn_array(uint32_t) indirection_array;

            The indirection array takes an opaque uint32_t handle and then dereferences it to find the actual index 
            for the data you're interested in. Just like dynamic arrays, they are NULL initialized and then 
            allocated/initialized internally upon use:

                gs_slot_array(float) arr = NULL;                    // Slot array with internal 'float' data
                uint32_t hndl = gs_slot_array_insert(arr, 3.145f);  // Inserts your data into the slot array, returns handle to you
                float val = gs_slot_array_get(arr, hndl);           // Returns copy of data to you using handle as lookup

            It is possible to return a reference to the data using `gs_slot_array_getp()`. However, keep in mind that this comes with the
            danger that the reference could be lost IF the internal data array grows or shrinks in between you caching the pointer 
            and using it.

                float* val = gs_slot_array_getp(arr, hndl);     // Cache pointer to internal data. Dangerous game.
                gs_slot_array_insert(arr, 5.f);                 // At this point, your pointer could be invalidated due to growing internal array.

            Slot arrays provide iterators to iterate the data:

                for (
                    gs_slot_array_iter it = 0; 
                    gs_slot_array_iter_valid(sa, it);
                    gs_slot_array_iter_advance(sa, it) 
                ) {
                    float v = gs_slot_array_iter_get(sa, it);         // Get value using iterator
                    float* vp = gs_slot_array_iter_getp(sa, it);      // Get value pointer using iterator
                }

            Slot Array Usage:

                gs_slot_array(float) sa = NULL;                   // Create slot array with internal 'float' data
                uint32_t hndl = gs_slot_array_insert(sa, 3.145);  // Insert data into slot array. Returns uint32_t handle. Init/Grow on demand.
                float v = gs_slot_array_get(sa, hndl);            // Get data at hndl.
                float* vp = gs_slot_array_getp(sa, hndl);         // Get pointer reference at hndl. Dangerous.
                uint32_t sz = gs_slot_array_size(sa);             // Size of slot array. Returns 0 if NULL.
                uint32_t cap = gs_slot_array_capacity(sa);        // Capacity of slot array. Returns 0 if NULL.
                gs_slot_array_empty(sa);                          // Returns whether slot array is empty. Returns true if NULL.
                gs_slot_array_clear(sa);                          // Clears array. Sets size to 0.
                gs_slot_array_free(sa);                           // Frees array memory. Calls `gs_free` internally.

        gs_slot_map:

            Works exactly the same, functionally, as gs_slot_array, however allows the user to use one more layer of indirection by 
            hashing any data as a key type. Also worth note, the slot map does not return a handle to the user. Instead, the user is 
            expected to use the key to access data.

                gs_slot_map(float, uint64_t) sm = NULL;    // Slot map with key = float, val = uint64_t
                gs_slot_map_insert(sm, 1.f, 32);           // Insert data into slot map.
                uint64_t v = gs_slot_map_get(sm, 1.f);     // Returns copy of data to you at key `1.f`

            Like the slot array, it is possible to return a reference via pointer using `gs_slot_map_getp()`. Again, this comes with the same 
            danger of losing references if not careful about growing the internal data. 

                uint64_t* v = gs_slot_map_getp(sm, 1.f);    // Cache pointer to data
                gs_slot_map_insert(sm, 2.f, 10);            // Possibly have just invalidated your previous pointer

            Slot maps provide iterators to iterate the data:

                for (
                    gs_slot_map_iter it = 0; 
                    gs_slot_map_iter_valid(sm, it);
                    gs_slot_map_iter_advance(sm, it) 
                ) {
                    uint64_t v = gs_slot_map_iter_get(sm, it);      // Get value using iterator
                    uint64_t* vp = gs_slot_map_iter_getp(sm, it);   // Get value pointer using iterator
                    float k = gs_slot_map_iter_get_key(sm, it);     // Get key using iterator
                    float* kp = gs_slot_map_iter_get_keyp(sm, it);  // Get key pointer using iterator
                }

            Slot Map Usage:

                gs_slot_map(float, uint64_t) sm = NULL;             // Create slot map with K = float, V = uint64_t
                uint32_t hndl = gs_slot_map_insert(sm, 3.145f, 32); // Insert data into slot map. Init/Grow on demand.
                uint64_t v = gs_slot_map_get(sm, 3.145f);           // Get data at key.
                uint64_t* vp = gs_slot_map_getp(sm, 3.145f);        // Get pointer reference at hndl. Dangerous.
                uint32_t sz = gs_slot_map_size(sm);                 // Size of slot map. Returns 0 if NULL.
                uint32_t cap = gs_slot_map_capacity(sm);            // Capacity of slot map. Returns 0 if NULL.
                gs_slot_map_empty(sm);                              // Returns whether slot map is empty. Returns true if NULL.
                gs_slot_map_clear(sm);                              // Clears map. Sets size to 0.
                gs_slot_map_free(sm);                               // Frees map memory. Calls `gs_free` internally.

    GS_PLATFORM:

        By default, Gunslinger supports (via included GLFW) the following platforms:
            - Win32
            - OSX
            - Linux 

        To define your own custom implementation and not use the included glfw implementation, define GS_PLATFORM_IMPL_CUSTOM in your
        project. Gunslinger will see this and leave the implementation of the platform API up to you:

            // For custom platform implementation
            #define GS_PLATFORM_IMPL_CUSTOM

        Internally, the platform interface holds the following data: 

            gs_platform_settings_t settings;         // Settings for platform, including video driver settings
            gs_platform_time_t time;                 // Time structure, used to query frame time, delta time, render time
            gs_platform_input_t input;               // Input struture, used to query input state for mouse, keyboard, controllers
            gs_slot_array(void*) windows;            // Slot array of raw platform window data and handles
            void* cursors[GS_PLATFORM_CURSOR_COUNT]; // Raw platform cursors
            void* user_data;                         // Specific user data (for custom implementations)

        Upon creation, the framework will create a main window for you and then store its handle with slot id 0. All platform related window 
        query functions require passing in an opaque uint32_t handle to get the actual data internally. There is a convenience function available for 
        querying the main window handle created for you:

            uint32_t main_window_hndl = gs_platform_main_window();

        Internally, the platform layer queries the platform backend and updates its exposed gs_platform_input_t data structure for you to use. Several 
        utility functions exist: 

            gs_platform_key_down(gs_platform_keycode)                   // Checks to see if a key is held down (pressed last frame and this frame)
            gs_platform_key_released(gs_platform_keycode)               // Checks to see if a key was released this frame
            gs_platform_key_pressed(gs_platform_keycode)                // Checks to see if a key was pressed this frame (not pressed last frame)
            gs_platform_mouse_down(gs_platform_mouse_button_code)       // Checks to see if mouse button is held down
            gs_platform_mouse_pressed(gs_platform_mouse_button_code)    // Checks to see if mouse button was pressed this frame (not pressed last frame)
            gs_platform_mouse_released(gs_platform_mouse_button_code)   // Checks to see if mouse button was released this frame

    GS_AUDIO:

        By default, Gunslinger includes and uses miniaudio for its audio backend. 
        To define your own custom implementation and not use the included miniaudio implementation, define GS_AUDIO_IMPL_CUSTOM in your
        project. Gunslinger will see this and leave the implementation of the audio API up to you:

            // For custom audio implementation
            #define GS_AUDIO_IMPL_CUSTOM

    GS_GRAPHICS:

            // For custom graphics implementation
            #define GS_GRAPHICS_IMPL_CUSTOM

*/

/*===== Gunslinger Include ======*/

#ifdef __cplusplus
extern "C" {
#endif

/*========================
// Defines
========================*/

#include <stdarg.h>     // valist
#include <stddef.h>     // ptrdiff_t
#include <stdlib.h>     // malloc, realloc, free
#include <stdint.h>     // standard types
#include <limits.h>     // INT32_MAX, UINT32_MAX
#include <string.h>     // memset
#include <float.h>      // FLT_MAX
#include <stdio.h>      // FILE
#include <time.h>       // time
#include <ctype.h>      // tolower
#include <math.h>       // floor, acos, sin, sqrt, tan
#include <assert.h>     // assert

/*===========================================================
// gs_inline, gs_global, gs_local_persist, gs_force_inline
===========================================================*/

#ifndef gs_inline
    #define gs_inline static inline
#endif

#ifndef gs_local_persist
    #define gs_local_persist static
#endif

#ifndef gs_global
    #define gs_global static
#endif

 #if (defined _WIN32 || defined _WIN64)
    #define gs_force_inline gs_inline
#elif (defined __APPLE__ || defined _APPLE)
    #define gs_force_inline static __attribute__((always_inline))
#else
    #define gs_force_inline gs_inline
#endif

#ifdef __cplusplus
    #pragma warning(disable:4996)
#endif

/*===================
// GS_API_DECL
===================*/
#ifdef __cplusplus
   #define GS_API_DECL   extern "C"
#else
   #define GS_API_DECL   extern
#endif

/*============================================================
// C primitive types
============================================================*/

#ifndef __cplusplus
        #define false     0
        #define true      1
#endif

#ifdef __cplusplus
        typedef bool      b8;
#else
        typedef _Bool     bool;
        typedef bool      b8;
#endif

typedef size_t            usize;

typedef uint8_t           u8;
typedef int8_t            s8;
typedef uint16_t          u16;
typedef int16_t           s16;
typedef uint32_t          u32;
typedef int32_t           s32;
typedef s32               b32;
typedef uint64_t          u64;
typedef int64_t           s64;
typedef float             f32;
typedef double            f64;
typedef const char*       const_str;

typedef int32_t           bool32_t;
typedef float             float32_t;
typedef double            float64_t;

typedef bool32_t          bool32;

#define uint16_max        UINT16_MAX
#define uint32_max        UINT32_MAX
#define int32_max         INT32_MAX
#define float_max         FLT_MAX
#define float_min         FLT_MIN

/*============================================================
// gs utils
============================================================*/

// Helper macro for compiling to nothing
#define gs_empty_instruction(...)

#define gs_array_size(__ARR) sizeof(__ARR) / sizeof(__ARR[0])

#ifndef gs_assert
    #define gs_assert assert
#endif
    
#if defined (__cplusplus)
    #define gs_default_val() {}
#else
    #define gs_default_val() {0}
#endif

// Helper macro for an in place for-range loop
#define gs_for_range_i(__COUNT)\
    for (uint32_t i = 0; i < __COUNT; ++i)

// Helper macro for an in place for-range loop
#define gs_for_range_j(__COUNT)\
    for (uint32_t j = 0; j < __COUNT; ++j)

#define gs_for_range(__COUNT)\
    for(uint32_t gs_macro_token_paste(__T, __LINE__) = 0;\
        gs_macro_token_paste(__T, __LINE__) < __COUNT;\
        ++(gs_macro_token_paste(__T, __LINE__)))

#define gs_max(A, B) ((A) > (B) ? (A) : (B))

#define gs_min(A, B) ((A) < (B) ? (A) : (B))

#define gs_clamp(V, MIN, MAX) ((V) > (MAX) ? (MAX) : (V) < (MIN) ? (MIN) : (V))

// Helpful macro for casting one type to another
#define gs_cast(A, B) ((A*)(B))

// Helpful marco for calculating offset (in bytes) of an element from a given structure type
#define gs_offset(TYPE, ELEMENT) ((size_t)(&(((TYPE*)(0))->ELEMENT)))

// macro for turning any given type into a const char* of itself
#define gs_to_str(TYPE) ((const char*)#TYPE)

#define gs_macro_token_paste(X, Y) X##Y
#define gs_macro_cat(X, Y) gs_macro_token_paste(X, Y)

#define gs_timed_action(INTERVAL, ...)\
    do {\
        static uint32_t gs_macro_cat(gs_macro_cat(__T, __LINE__), t) = 0;\
        if (gs_macro_cat(gs_macro_cat(__T, __LINE__), t)++ > INTERVAL) {\
            gs_macro_cat(gs_macro_cat(__T, __LINE__), t) = 0;\
            __VA_ARGS__\
        }\
    } while (0)

#define gs_int2voidp(I) (void*)(uintptr_t)(I)

/*===================================
// Memory Allocation Utils
===================================*/

#ifndef gs_malloc
    #define gs_malloc(__SZ) malloc(__SZ)
#endif

#ifndef gs_free
    #define gs_free(__MEM) free(__MEM)
#endif

#ifndef gs_realloc
    #define gs_realloc(__MEM, __AZ) realloc(__MEM, __AZ)
#endif

#ifndef gs_calloc
    #define gs_calloc(__NUM, __AZ) calloc(__NUM, __AZ)
#endif

gs_force_inline 
void* _gs_malloc_init_impl(size_t sz)
{
    void* data = gs_malloc(sz);
    memset(data, 0, sz);
    return data;
}

#define gs_malloc_init(__TYPE) (__TYPE*)_gs_malloc_init_impl(sizeof(__TYPE))

/*============================================================
// Result
============================================================*/

typedef enum gs_result
{
    GS_RESULT_SUCCESS,
    GS_RESULT_IN_PROGRESS,
    GS_RESULT_INCOMPLETE,
    GS_RESULT_FAILURE
} gs_result;

/*===================================
// Resource Handles
===================================*/

// Useful typedefs for typesafe, internal resource handles

#define gs_handle(TYPE)\
    gs_handle_##TYPE

#define gs_handle_decl(TYPE)\
    typedef struct {uint32_t id;} gs_handle(TYPE);\
    gs_inline\
    gs_handle(TYPE) gs_handle_invalid_##TYPE()\
    {\
        gs_handle(TYPE) h;\
        h.id = UINT32_MAX;\
        return h;\
    }\
\
    gs_inline\
    gs_handle(TYPE) gs_handle_create_##TYPE(uint32_t id)\
    {\
        gs_handle(TYPE) h;\
        h.id = id;\
        return h;\
    }

#define gs_handle_invalid(__TYPE)\
    gs_handle_invalid_##__TYPE()

#define gs_handle_create(__TYPE, __ID)\
    gs_handle_create_##__TYPE(__ID)

/*===================================
// Color
===================================*/

#define gs_hsv(...) gs_hsv_ctor(__VA_ARGS__)
#define gs_color(...) gs_color_ctor(__VA_ARGS__)

typedef struct gs_hsv_t
{
    union 
    {
        float hsv[3];
        struct 
        {
            float h, s, v;
        };
    };
} gs_hsv_t;

gs_force_inline
gs_hsv_t gs_hsv_ctor(float h, float s, float v)
{
    gs_hsv_t hsv;
    hsv.h = h;
    hsv.s = s;
    hsv.v = v;
    return hsv;
}

typedef struct gs_color_t
{
    union 
    {
        uint8_t rgba[4];
        struct 
        {
            uint8_t r, g, b, a;
        };
    };
} gs_color_t;

gs_force_inline
gs_color_t gs_color_ctor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    gs_color_t color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    return color;
}

#define GS_COLOR_BLACK  gs_color(0, 0, 0, 255)
#define GS_COLOR_WHITE  gs_color(255, 255, 255, 255)
#define GS_COLOR_RED    gs_color(255, 0, 0, 255)
#define GS_COLOR_GREEN  gs_color(0, 255, 0, 255)
#define GS_COLOR_BLUE   gs_color(0, 0, 255, 255)
#define GS_COLOR_ORANGE gs_color(255, 100, 0, 255)
#define GS_COLOR_PURPLE gs_color(128, 0, 128, 255)

gs_force_inline 
gs_color_t gs_color_alpha(gs_color_t c, uint8_t a)
{
    return gs_color(c.r, c.g, c.b, a); 
}

/*===================================
// String Utils
===================================*/

gs_force_inline uint32_t 
gs_string_length(const char* txt)
{
    uint32_t sz = 0;
    while (txt != NULL && txt[ sz ] != '\0') 
    {
        sz++;
    }
    return sz;
}

// Expects null terminated strings
gs_force_inline b32 
gs_string_compare_equal
(
    const char*     txt, 
    const char*     cmp 
)
{
    // Grab sizes of both strings
    uint32_t a_sz = gs_string_length(txt);
    uint32_t b_sz = gs_string_length(cmp);

    // Return false if sizes do not match
    if (a_sz != b_sz) 
    {
        return false;
    }

    for(uint32_t i = 0; i < a_sz; ++i) 
    {
        if (*txt++ != *cmp++)
        {
            return false;
        }
    };

    return true;
}

gs_force_inline b32 
gs_string_compare_equal_n
(
    const char* txt, 
    const char* cmp, 
    uint32_t n 
)
{
    uint32_t a_sz = gs_string_length(txt);
    uint32_t b_sz = gs_string_length(cmp);

    // Not enough characters to do operation
    if (a_sz < n || b_sz < n)
    {
        return false;
    }

    for (uint32_t i = 0; i < n; ++i) 
    {
        if (*txt++ != *cmp++)
        {
            return false;
        }
    };

    return true;
}

gs_force_inline void
gs_util_str_to_lower
(
    const char* src,
    char* buffer,
    size_t buffer_sz
)
{
    size_t src_sz = gs_string_length(src);
    size_t len = gs_min(src_sz, buffer_sz);

    for (uint32_t i = 0; i < len; ++i) {
        buffer[i] = tolower(src[i]);
    }
}

gs_force_inline b32
gs_util_str_is_numeric(const char* str)
{
    const char* at = str;
    while (at && *at)
    {
        while (*at == '\n' || *at == '\t' || *at == ' ' || *at == '\r') at++;;
        char c = *at++;
        if (c >= '0' && c <= '9')
        {
            return false;
        } 
    }   

    return true;
}

// Will return a null buffer if file does not exist or allocation fails
gs_force_inline 
char* gs_read_file_contents_into_string_null_term
(
    const char* file_path, 
    const char* mode,
    size_t* _sz
)
{
    char* buffer = 0;
    FILE* fp = fopen(file_path, mode);
    size_t sz = 0;
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        sz = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = (char*)gs_malloc(sz + 1);
        if (buffer)
        {
            fread(buffer, 1, sz, fp);
        }
        fclose(fp);
        buffer[sz] = '0';
        if (_sz) *_sz = sz;
    }
    return buffer;
}

gs_force_inline 
b32 gs_util_file_exists(const char* file_path)
{
    FILE* fp = fopen(file_path, "r");   
    if (fp)
    {
        fclose(fp);
        return true;
    }
    return false;
}

gs_force_inline 
void gs_util_get_file_extension
(
    char* buffer,
    uint32_t buffer_size,
    const char* file_path 
)
{
    uint32_t str_len = gs_string_length(file_path);
    const char* at = (file_path + str_len - 1);
    while (*at != '.' && at != file_path)
    {
        at--;
    }

    if (*at == '.')
    {
        at++;
        uint32_t i = 0; 
        while (*at)
        {
            char c = *at;
            buffer[ i++ ] = *at++;
        }
        buffer[ i ] = '\0';
    }
}

gs_force_inline 
void gs_util_get_dir_from_file
(
    char* buffer, 
    uint32_t buffer_size,
    const char* file_path 
)
{
    uint32_t str_len = gs_string_length(file_path);
    const char* end = (file_path + str_len);
    while (*end != '/' && end != file_path)
    {
        end--;
    }
    memcpy(buffer, file_path, gs_min(buffer_size, (end - file_path) + 1));
}

gs_force_inline 
void gs_util_get_file_name
(
    char* buffer, 
    uint32_t buffer_size,
    const char* file_path 
)
{
    uint32_t str_len = gs_string_length(file_path);
    const char* end = (file_path + str_len);
    const char* dot_at = end;
    while (*end != '.' && end != file_path)
    {
        end--;
    }
    const char* start = end; 
    while (*start != '/' && start != file_path)
    {
        start--;
    }
    memcpy(buffer, start, (end - start));
}

gs_force_inline 
void gs_util_string_substring
(
    const char* src,
    char* dst,
    size_t sz,
    uint32_t start,
    uint32_t end
)
{
    uint32_t str_len = gs_string_length(src);
    if (end > str_len) {
        end = str_len;
    }
    if (start > str_len) {
        start = str_len;
    }

    const char* at = src + start;
    const char* e = src + end;
    uint32_t ct = 0;
    while (at && *at != '\0' && at != e)
    {
        dst[ ct ] = *at;
        at++;
        ct++;
    }
}

gs_force_inline 
void gs_util_string_remove_character
(
    const char* src,
    char* buffer, 
    uint32_t buffer_size,
    char delimiter
)
{
    uint32_t ct = 0;
    uint32_t str_len = gs_string_length(src);
    const char* at = src;
    while (at && *at != '\0' && ct < buffer_size)
    {
        char c = *at; 
        if (c != delimiter) {
            buffer[ ct ] = c;
            ct++;
        }
        at++;
    }
}

gs_force_inline 
void gs_util_string_replace
(
    const char* source_str, 
    char* buffer, 
    uint32_t buffer_size, 
    char delimiter,
    char replace 
)
{
    uint32_t str_len = gs_string_length(source_str);
    const char* at = source_str;
    while (at && *at != '\0')
    {
        char c = *at; 
        if (c == delimiter) {
            c = replace;
        }
        buffer[(at - source_str)] = c;
        at++;
    }
}

gs_force_inline 
void gs_util_normalize_path
(
    const char* path, 
    char* buffer, 
    uint32_t buffer_size 
)
{
    // Normalize the path somehow...
}

#ifdef __MINGW32__
    #define gs_printf(__FMT, ...) __mingw_printf(__FMT, ##__VA_ARGS__)
#else
    gs_force_inline void 
    gs_printf
    (
        const char* fmt,
        ... 
    )
    {
        va_list args;
        va_start (args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
#endif

#define gs_println(__FMT, ...)\
    do {\
        gs_printf(__FMT, ##__VA_ARGS__);\
        gs_printf("\n");\
    } while (0)

gs_force_inline 
void gs_fprintf
(
    FILE* fp, 
    const char* fmt, 
    ... 
)
{
    va_list args;
    va_start (args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
}

gs_force_inline 
void gs_fprintln
(
    FILE* fp, 
    const char* fmt, 
    ... 
)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
    gs_fprintf(fp, "\n");
}

#ifdef __MINGW32__
#define gs_snprintf(__NAME, __SZ, __FMT, ...) __mingw_snprintf(__NAME, __SZ, __FMT, ## __VA_ARGS__)
#else
gs_force_inline 
void gs_snprintf
(
    char* buffer, 
    size_t buffer_size, 
    const char* fmt, 
    ... 
)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, buffer_size, fmt, args);
    va_end(args);
}
#endif

#define gs_transient_buffer(__N, __SZ)\
    char __N[__SZ] = gs_default_val();\
    memset(__N, 0, __SZ);

#define gs_snprintfc(__NAME, __SZ, __FMT, ...)\
    char __NAME[__SZ] = gs_default_val();\
    gs_snprintf(__NAME, __SZ, __FMT, ## __VA_ARGS__);

gs_force_inline
uint32_t gs_util_safe_truncate_u64(uint64_t value)
{
  gs_assert(value <= 0xFFFFFFFF); 
  uint32_t result = (uint32_t)value;
  return result;
}

gs_force_inline 
uint32_t gs_hash_uint32_t(uint32_t x)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

#define gs_hash_u32_ip(__X, __OUT)\
    do {\
        __OUT = ((__X >> 16) ^ __X) * 0x45d9f3b;\
        __OUT = ((__OUT >> 16) ^ __OUT) * 0x45d9f3b;\
        __OUT = (__OUT >> 16) ^ __OUT;\
    } while (0) 

gs_force_inline 
uint32_t gs_hash_u64(uint64_t x)
{
    x = (x ^ (x >> 31) ^ (x >> 62)) * UINT64_C(0x319642b2d24d8ec3);
    x = (x ^ (x >> 27) ^ (x >> 54)) * UINT64_C(0x96de1b173f119089);
    x = x ^ (x >> 30) ^ (x >> 60);
    return (uint32_t)x; 
}

// Note(john): source: http://www.cse.yorku.ca/~oz/hash.html
// djb2 hash by dan bernstein
gs_force_inline 
uint32_t gs_hash_str(const char* str)
{
    uint32_t hash = 5381;
    s32 c;
    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

gs_force_inline 
uint64_t gs_hash_str64(const char* str)
{
    uint32_t hash1 = 5381;
    uint32_t hash2 = 52711;
    uint32_t i = gs_string_length(str);
    while(i--) 
    {
        char c = str[ i ];
        hash1 = (hash1 * 33) ^ c;
        hash2 = (hash2 * 33) ^ c;
    }

    return (hash1 >> 0) * 4096 + (hash2 >> 0);
}

gs_force_inline
bool gs_compare_bytes(void* b0, void* b1, size_t len)
{
    return 0 == memcmp(b0, b1, len);
}

// Hash generic bytes using (ripped directly from Sean Barret's stb_ds.h)
#define GS_SIZE_T_BITS  ((sizeof(size_t)) * 8)
#define GS_SIPHASH_C_ROUNDS 1
#define GS_SIPHASH_D_ROUNDS 1
#define gs_rotate_left(__V, __N)   (((__V) << (__N)) | ((__V) >> (GS_SIZE_T_BITS - (__N))))
#define gs_rotate_right(__V, __N)  (((__V) >> (__N)) | ((__V) << (GS_SIZE_T_BITS - (__N))))

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4127) // conditional expression is constant, for do..while(0) and sizeof()==
#endif

gs_force_inline 
size_t gs_hash_siphash_bytes(void *p, size_t len, size_t seed)
{
  unsigned char *d = (unsigned char *) p;
  size_t i,j;
  size_t v0,v1,v2,v3, data;

  // hash that works on 32- or 64-bit registers without knowing which we have
  // (computes different results on 32-bit and 64-bit platform)
  // derived from siphash, but on 32-bit platforms very different as it uses 4 32-bit state not 4 64-bit
  v0 = ((((size_t) 0x736f6d65 << 16) << 16) + 0x70736575) ^  seed;
  v1 = ((((size_t) 0x646f7261 << 16) << 16) + 0x6e646f6d) ^ ~seed;
  v2 = ((((size_t) 0x6c796765 << 16) << 16) + 0x6e657261) ^  seed;
  v3 = ((((size_t) 0x74656462 << 16) << 16) + 0x79746573) ^ ~seed;

  #ifdef STBDS_TEST_SIPHASH_2_4
  // hardcoded with key material in the siphash test vectors
  v0 ^= 0x0706050403020100ull ^  seed;
  v1 ^= 0x0f0e0d0c0b0a0908ull ^ ~seed;
  v2 ^= 0x0706050403020100ull ^  seed;
  v3 ^= 0x0f0e0d0c0b0a0908ull ^ ~seed;
  #endif

  #define gs_sipround() \
    do {                   \
      v0 += v1; v1 = gs_rotate_left(v1, 13);  v1 ^= v0; v0 = gs_rotate_left(v0,GS_SIZE_T_BITS/2); \
      v2 += v3; v3 = gs_rotate_left(v3, 16);  v3 ^= v2;                                                 \
      v2 += v1; v1 = gs_rotate_left(v1, 17);  v1 ^= v2; v2 = gs_rotate_left(v2,GS_SIZE_T_BITS/2); \
      v0 += v3; v3 = gs_rotate_left(v3, 21);  v3 ^= v0;                                                 \
    } while (0)

  for (i=0; i+sizeof(size_t) <= len; i += sizeof(size_t), d += sizeof(size_t)) {
    data = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
    data |= (size_t) (d[4] | (d[5] << 8) | (d[6] << 16) | (d[7] << 24)) << 16 << 16; // discarded if size_t == 4

    v3 ^= data;
    for (j=0; j < GS_SIPHASH_C_ROUNDS; ++j)
      gs_sipround();
    v0 ^= data;
  }
  data = len << (GS_SIZE_T_BITS-8);
  switch (len - i) {
    case 7: data |= ((size_t) d[6] << 24) << 24; // fall through
    case 6: data |= ((size_t) d[5] << 20) << 20; // fall through
    case 5: data |= ((size_t) d[4] << 16) << 16; // fall through
    case 4: data |= (d[3] << 24); // fall through
    case 3: data |= (d[2] << 16); // fall through
    case 2: data |= (d[1] << 8); // fall through
    case 1: data |= d[0]; // fall through
    case 0: break;
  }
  v3 ^= data;
  for (j=0; j < GS_SIPHASH_C_ROUNDS; ++j)
    gs_sipround();
  v0 ^= data;
  v2 ^= 0xff;
  for (j=0; j < GS_SIPHASH_D_ROUNDS; ++j)
    gs_sipround();

#if 0
  return v0^v1^v2^v3;
#else
  return v1^v2^v3; // slightly stronger since v0^v3 in above cancels out final round operation? I tweeted at the authors of SipHash about this but they didn't reply
#endif
}

gs_force_inline
size_t gs_hash_bytes(void *p, size_t len, size_t seed)
{
#if 0
  return gs_hash_siphash_bytes(p,len,seed);
#else
  unsigned char *d = (unsigned char *) p;

  // Len == 4 (off for now, so to force 64 bit hash)
  if (len == 4) {
    unsigned int hash = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
    hash ^= seed;
    hash *= 0xcc9e2d51;
    hash = (hash << 17) | (hash >> 15);
    hash *= 0x1b873593;
    hash ^= seed;
    hash = (hash << 19) | (hash >> 13);
    hash = hash*5 + 0xe6546b64;
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= seed;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;
    return (((size_t) hash << 16 << 16) | hash) ^ seed;
  } else if (len == 8 && sizeof(size_t) == 8) {
    size_t hash = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
    hash |= (size_t) (d[4] | (d[5] << 8) | (d[6] << 16) | (d[7] << 24)) << 16 << 16; // avoid warning if size_t == 4
    hash ^= seed;
    hash = (~hash) + (hash << 21);
    hash ^= gs_rotate_right(hash,24);
    hash *= 265;
    hash ^= gs_rotate_right(hash,14);
    hash ^= seed;
    hash *= 21;
    hash ^= gs_rotate_right(hash,28);
    hash += (hash << 31);
    hash = (~hash) + (hash << 18);
    return hash;
  } else {
    return gs_hash_siphash_bytes(p,len,seed);
  }
#endif
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

/* Resource Loading Util */
GS_API_DECL bool32_t gs_util_load_texture_data_from_file(const char* file_path, int32_t* width, int32_t* height, uint32_t* num_comps, void** data, bool32_t flip_vertically_on_load);

/*========================
// GS_CONTAINERS
========================*/

/*========================
// Byte Buffer
========================*/

#define GS_BYTE_BUFFER_DEFAULT_CAPCITY  1024

typedef struct gs_byte_buffer_t
{
    uint8_t* data;      // Buffer that actually holds all relevant byte data
    uint32_t size;      // Current size of the stored buffer data
    uint32_t position;  // Current read/write position in the buffer
    uint32_t capacity;  // Current max capacity for the buffer
} gs_byte_buffer_t;

// Generic "write" function for a byte buffer
#define gs_byte_buffer_write(__BB, __T, __VAL)\
do {\
    gs_byte_buffer_t* __BUFFER = __BB;\
    usize __SZ = sizeof(__T);\
    usize __TWS = __BUFFER->position + __SZ;\
    if (__TWS >= (usize)__BUFFER->capacity)\
    {\
        usize __CAP = __BUFFER->capacity * 2;\
        while(__CAP < __TWS)\
        {\
            __CAP *= 2;\
        }\
        gs_byte_buffer_resize(__BUFFER, __CAP);\
    }\
    *(__T*)(__BUFFER->data + __BUFFER->position) = __VAL;\
    __BUFFER->position += (uint32_t)__SZ;\
    __BUFFER->size += (uint32_t)__SZ;\
} while (0)

// Generic "read" function
#define gs_byte_buffer_read(__BUFFER, __T, __VAL_P)\
do {\
    __T* __V = (__T*)(__VAL_P);\
    gs_byte_buffer_t* __BB = (__BUFFER);\
    *(__V) = *(__T*)(__BB->data + __BB->position);\
    __BB->position += sizeof(__T);\
} while (0)

// Defines variable and sets value from buffer in place
// Use to construct a new variable
#define gs_byte_buffer_readc(__BUFFER, __T, __NAME)\
    __T __NAME = gs_default_val();\
    gs_byte_buffer_read((__BUFFER), __T, &__NAME);

#define gs_byte_buffer_read_bulkc(__BUFFER, __T, __NAME, __SZ)\
    __T __NAME = gs_default_val();\
    __T* gs_macro_cat(__NAME, __LINE__) = &(__NAME);\
    gs_byte_buffer_read_bulk(__BUFFER, (void**)&gs_macro_cat(__NAME, __LINE__), __SZ);

/* Desc */
GS_API_DECL void gs_byte_buffer_init(gs_byte_buffer_t* buffer);

/* Desc */
GS_API_DECL gs_byte_buffer_t gs_byte_buffer_new();

/* Desc */
GS_API_DECL void gs_byte_buffer_free(gs_byte_buffer_t* buffer);

/* Desc */
GS_API_DECL void gs_byte_buffer_clear(gs_byte_buffer_t* buffer);

/* Desc */
GS_API_DECL void gs_byte_buffer_resize(gs_byte_buffer_t* buffer, size_t sz);

/* Desc */
GS_API_DECL void gs_byte_buffer_seek_to_beg(gs_byte_buffer_t* buffer);

/* Desc */
GS_API_DECL void gs_byte_buffer_seek_to_end(gs_byte_buffer_t* buffer);

/* Desc */
GS_API_DECL void gs_byte_buffer_advance_position(gs_byte_buffer_t* buffer, size_t sz);

/* Desc */
GS_API_DECL void gs_byte_buffer_write_str(gs_byte_buffer_t* buffer, const char* str);                   // Expects a null terminated string

/* Desc */
GS_API_DECL void gs_byte_buffer_read_str(gs_byte_buffer_t* buffer, char* str);                          // Expects an allocated string

/* Desc */
GS_API_DECL void gs_byte_buffer_write_bulk(gs_byte_buffer_t* buffer, void* src, size_t sz);

/* Desc */
GS_API_DECL void gs_byte_buffer_read_bulk(gs_byte_buffer_t* buffer, void** dst, size_t sz);

/* Desc */
GS_API_DECL gs_result gs_byte_buffer_write_to_file(gs_byte_buffer_t* buffer, const char* output_path);  // Assumes that the output directory exists

/* Desc */
GS_API_DECL gs_result gs_byte_buffer_read_from_file(gs_byte_buffer_t* buffer, const char* file_path);   // Assumes an allocated byte buffer

/*===================================
// Dynamic Array
===================================*/

typedef struct gs_dyn_array
{
    int32_t size;
    int32_t capacity;
} gs_dyn_array;

#define gs_dyn_array_head(__ARR)\
    ((gs_dyn_array*)((uint8_t*)(__ARR) - sizeof(gs_dyn_array)))

#define gs_dyn_array_size(__ARR)\
    (__ARR == NULL ? 0 : gs_dyn_array_head((__ARR))->size)

#define gs_dyn_array_capacity(__ARR)\
    (__ARR == NULL ? 0 : gs_dyn_array_head((__ARR))->capacity)

#define gs_dyn_array_full(__ARR)\
    ((gs_dyn_array_size((__ARR)) == gs_dyn_array_capacity((__ARR))))    

gs_inline 
void* gs_dyn_array_resize_impl(void* arr, size_t sz, size_t amount) 
{
    size_t capacity;

    if (arr) {
        capacity = amount;  
    } else {
        capacity = 0;
    }

    // Create new gs_dyn_array with just the header information
    gs_dyn_array* data = (gs_dyn_array*)gs_realloc(arr ? gs_dyn_array_head(arr) : 0, capacity * sz + sizeof(gs_dyn_array));

    if (data) {
        if (!arr) {
            data->size = 0;
        }
        data->capacity = (int32_t)capacity;
        return ((int32_t*)data + 2);
    }

    return NULL;
}

#define gs_dyn_array_need_grow(__ARR, __N)\
    ((__ARR) == 0 || gs_dyn_array_size(__ARR) + (__N) >= gs_dyn_array_capacity(__ARR))

#define gs_dyn_array_grow(__ARR)\
    gs_dyn_array_resize_impl((__ARR), sizeof(*(__ARR)), gs_dyn_array_capacity(__ARR) ? gs_dyn_array_capacity(__ARR) * 2 : 1)

#define gs_dyn_array_grow_size(__ARR, __SZ  )\
    gs_dyn_array_resize_impl((__ARR), (__SZ ), gs_dyn_array_capacity(__ARR) ? gs_dyn_array_capacity(__ARR) * 2 : 1)

gs_force_inline
void** gs_dyn_array_init(void** arr, size_t val_len)
{
    if (*arr == NULL) {
        gs_dyn_array* data = (gs_dyn_array*)gs_malloc(val_len + sizeof(gs_dyn_array));  // Allocate capacity of one
        data->size = 0;
        data->capacity = 1;
        *arr = ((int32_t*)data + 2);
        return arr;
    }
    return NULL;
}

gs_force_inline
void gs_dyn_array_push_data(void** arr, void* val, size_t val_len)
{
    if (*arr == NULL) {
        gs_dyn_array_init(arr, val_len);
    }
    if (gs_dyn_array_need_grow(*arr, 1)) 
    {
        int32_t capacity = gs_dyn_array_capacity(*arr) * 2;

        // Create new gs_dyn_array with just the header information
        gs_dyn_array* data = (gs_dyn_array*)gs_realloc(gs_dyn_array_head(*arr), capacity * val_len + sizeof(gs_dyn_array));

        if (data) {
            data->capacity = capacity;
            *arr = ((int32_t*)data + 2);
        }
    }
    size_t offset = gs_dyn_array_size(*arr);
    memcpy(((uint8_t*)(*arr)) + offset * val_len, val, val_len);
    gs_dyn_array_head(*arr)->size++;
}

gs_force_inline
void gs_dyn_array_set_data_i(void** arr, void* val, size_t val_len, uint32_t offset)
{
    memcpy(((char*)(*arr)) + offset * val_len, val, val_len);
}

#define gs_dyn_array_push(__ARR, __ARRVAL)\
    do {\
        gs_dyn_array_init((void**)&(__ARR), sizeof(*(__ARR)));\
        if (!(__ARR) || ((__ARR) && gs_dyn_array_need_grow(__ARR, 1))) {\
            *((void **)&(__ARR)) = gs_dyn_array_grow(__ARR); \
        }\
        (__ARR)[gs_dyn_array_size(__ARR)] = (__ARRVAL);\
        gs_dyn_array_head(__ARR)->size++;\
    } while(0)

#define gs_dyn_array_reserve(__ARR, __AMOUNT)\
    do {\
        if ((!__ARR)) gs_dyn_array_init((void**)&(__ARR), sizeof(*(__ARR)));\
        if ((!__ARR) || (size_t)__AMOUNT > gs_dyn_array_capacity(__ARR)) {\
            *((void **)&(__ARR)) = gs_dyn_array_resize_impl(__ARR, sizeof(*__ARR), __AMOUNT);\
        }\
    } while(0)

#define gs_dyn_array_empty(__ARR)\
    (gs_dyn_array_init((void**)&(__ARR), sizeof(*(__ARR))), (gs_dyn_array_size(__ARR) == 0))

#define gs_dyn_array_pop(__ARR)\
    do {\
        if (__ARR && !gs_dyn_array_empty(__ARR)) {\
            gs_dyn_array_head(__ARR)->size -= 1;\
        }\
    } while (0)

#define gs_dyn_array_back(__ARR)\
    *(__ARR + (gs_dyn_array_size(__ARR) ? gs_dyn_array_size(__ARR) - 1 : 0))

#define gs_dyn_array_for(__ARR, __T, __IT_NAME)\
    for (__T* __IT_NAME = __ARR; __IT_NAME != gs_dyn_array_back(__ARR); ++__IT_NAME)

#define gs_dyn_array_new(__T)\
    ((__T*)gs_dyn_array_resize_impl(NULL, sizeof(__T), 0))

#define gs_dyn_array_clear(__ARR)\
    do {\
        if (__ARR) {\
            gs_dyn_array_head(__ARR)->size = 0;\
        }\
    } while (0)

#define gs_dyn_array(__T)   __T*

#define gs_dyn_array_free(__ARR)\
    do {\
        if (__ARR) {\
            gs_free(gs_dyn_array_head(__ARR));\
            (__ARR) = NULL;\
        }\
    } while (0)

/*===================================
// Hash Table
===================================*/

/* 
    If using struct for keys, requires struct to be word-aligned.
*/

#define GS_HASH_TABLE_HASH_SEED         0x31415296
#define GS_HASH_TABLE_INVALID_INDEX     UINT32_MAX

typedef enum gs_hash_table_entry_state
{
    GS_HASH_TABLE_ENTRY_INACTIVE = 0x00,
    GS_HASH_TABLE_ENTRY_ACTIVE = 0x01
} gs_hash_table_entry_state;

#define __gs_hash_table_entry(__HMK, __HMV)\
    struct\
    {\
        __HMK key;\
        __HMV val;\
        gs_hash_table_entry_state state;\
    }

#define gs_hash_table(__HMK, __HMV)\
    struct {\
        __gs_hash_table_entry(__HMK, __HMV)* data;\
        __HMK tmp_key;\
        __HMV tmp_val;\
        size_t stride;\
        size_t klpvl;\
    }*

// Need a way to create a temporary key so I can take the address of it

#define gs_hash_table_new(__K, __V)\
    NULL

gs_force_inline
void __gs_hash_table_init_impl(void** ht, size_t sz)
{
    *ht = gs_malloc(sz);
}

#define gs_hash_table_init(__HT, __K, __V)\
    do {\
        size_t entry_sz = sizeof(__K) + sizeof(__V) + sizeof(gs_hash_table_entry_state);\
        size_t ht_sz = sizeof(__K) + sizeof(__V) + sizeof(void*) + 2 * sizeof(size_t);\
        __gs_hash_table_init_impl((void**)&(__HT), ht_sz);\
        memset((__HT), 0, ht_sz);\
        gs_dyn_array_reserve(__HT->data, 2);\
        __HT->data[0].state = GS_HASH_TABLE_ENTRY_INACTIVE;\
        __HT->data[1].state = GS_HASH_TABLE_ENTRY_INACTIVE;\
        uintptr_t d0 = (uintptr_t)&((__HT)->data[0]);\
        uintptr_t d1 = (uintptr_t)&((__HT)->data[1]);\
        ptrdiff_t diff = (d1 - d0);\
        ptrdiff_t klpvl = (uintptr_t)&(__HT->data[0].state) - (uintptr_t)(&__HT->data[0]);\
        (__HT)->stride = (size_t)(diff);\
        (__HT)->klpvl = (size_t)(klpvl);\
    } while (0)

#define gs_hash_table_size(__HT)\
    ((__HT) != NULL ? gs_dyn_array_size((__HT)->data) : 0)

#define gs_hash_table_capacity(__HT)\
    ((__HT) != NULL ? gs_dyn_array_capacity((__HT)->data) : 0)

#define gs_hash_table_load_factor(__HT)\
    (gs_hash_table_capacity(__HT) ? (float)(gs_hash_table_size(__HT)) / (float)(gs_hash_table_capacity(__HT)) : 0.f)

#define gs_hash_table_grow(__HT, __C)\
    ((__HT)->data = gs_dyn_array_resize_impl((__HT)->data, sizeof(*((__HT)->data)), (__C)))

#define gs_hash_table_empty(__HT)\
    ((__HT) != NULL ? gs_dyn_array_size((__HT)->data) == 0 : true)

#define gs_hash_table_clear(__HT)\
    do {\
        if ((__HT) != NULL) {\
            uint32_t capacity = gs_dyn_array_capacity((__HT)->data);\
            for (uint32_t i = 0; i < capacity; ++i) {\
                (__HT)->data[i].state = GS_HASH_TABLE_ENTRY_INACTIVE;\
            }\
            /*memset((__HT)->data, 0, gs_dyn_array_capacity((__HT)->data) * );*/\
            gs_dyn_array_clear((__HT)->data);\
        }\
    } while (0)

#define gs_hash_table_free(__HT)\
    do {\
        if ((__HT) != NULL) {\
            gs_dyn_array_free((__HT)->data);\
            (__HT)->data = NULL;\
            gs_free(__HT);\
            (__HT) = NULL;\
        }\
    } while (0)

// Find available slot to insert k/v pair into
#define gs_hash_table_insert(__HT, __HMK, __HMV)\
    do {\
        /* Check for null hash table, init if necessary */\
        if ((__HT) == NULL) {\
            gs_hash_table_init((__HT), (__HMK), (__HMV));\
        }\
    \
        /* Grow table if necessary */\
        uint32_t __CAP = gs_hash_table_capacity(__HT);\
        float __LF = gs_hash_table_load_factor(__HT);\
        if (__LF >= 0.5f || !__CAP)\
        {\
            uint32_t NEW_CAP = __CAP ? __CAP * 2 : 2;\
            size_t ENTRY_SZ = sizeof((__HT)->tmp_key) + sizeof((__HT)->tmp_val) + sizeof(gs_hash_table_entry_state);\
            gs_dyn_array_reserve((__HT)->data, NEW_CAP);\
            /**((void **)&(__HT->data)) = gs_dyn_array_resize_impl(__HT->data, ENTRY_SZ, NEW_CAP);*/\
            /* Iterate through data and set state to null, from __CAP -> __CAP * 2 */\
            /* Memset here instead */\
            for (uint32_t __I = __CAP; __I < NEW_CAP; ++__I) {\
                (__HT)->data[__I].state = GS_HASH_TABLE_ENTRY_INACTIVE;\
            }\
            __CAP = gs_hash_table_capacity(__HT);\
        }\
    \
        /* Get hash of key */\
        (__HT)->tmp_key = (__HMK);\
        size_t __HSH = gs_hash_bytes((void*)&((__HT)->tmp_key), sizeof((__HT)->tmp_key), GS_HASH_TABLE_HASH_SEED);\
        size_t __HSH_IDX = __HSH % __CAP;\
        (__HT)->tmp_key = (__HT)->data[__HSH_IDX].key;\
        uint32_t c = 0;\
    \
        /* Find valid idx and place data */\
        while (\
            c < __CAP\
            && __HSH != gs_hash_bytes((void*)&(__HT)->tmp_key, sizeof((__HT)->tmp_key), GS_HASH_TABLE_HASH_SEED)\
            && (__HT)->data[__HSH_IDX].state == GS_HASH_TABLE_ENTRY_ACTIVE)\
        {\
            __HSH_IDX = ((__HSH_IDX + 1) % __CAP);\
            (__HT)->tmp_key = (__HT)->data[__HSH_IDX].key;\
            ++c;\
        }\
        (__HT)->data[__HSH_IDX].key = (__HMK);\
        (__HT)->data[__HSH_IDX].val = (__HMV);\
        (__HT)->data[__HSH_IDX].state = GS_HASH_TABLE_ENTRY_ACTIVE;\
        gs_dyn_array_head((__HT)->data)->size++;\
    } while (0)

// Need size difference between two entries
// Need size of key + val

gs_force_inline
uint32_t gs_hash_table_get_key_index_func(void** data, void* key, size_t key_len, size_t val_len, size_t stride, size_t klpvl)
{
    // Need a better way to handle this. Can't do it like this anymore.
    // Need to fix this. Seriously messing me up.
    uint32_t capacity = gs_dyn_array_capacity(*data);
    size_t idx = (size_t)GS_HASH_TABLE_INVALID_INDEX;
    size_t hash = (size_t)gs_hash_bytes(key, key_len, GS_HASH_TABLE_HASH_SEED);
    size_t hash_idx = (hash % capacity);

    // Iterate through data 
    for (size_t i = hash_idx, c = 0; c < capacity; ++c, i = ((i + 1) % capacity))
    {
        size_t offset = (i * stride);
        void* k = ((char*)(*data) + (offset));  
        size_t kh = gs_hash_bytes(k, key_len, GS_HASH_TABLE_HASH_SEED);
        bool comp = gs_compare_bytes(k, key, key_len);
        gs_hash_table_entry_state state = *(gs_hash_table_entry_state*)((char*)(*data) + offset + (klpvl)); 
        if (comp && hash == kh && state == GS_HASH_TABLE_ENTRY_ACTIVE) {
            idx = i;
            break;
        }
    }
    return (uint32_t)idx;
}

// Get key at index
#define gs_hash_table_getk(__HT, __I)\
    (((__HT))->data[(__I)].key)

// Get val at index
#define gs_hash_table_geti(__HT, __I)\
    ((__HT)->data[(__I)].val)

// Could search for the index in the macro instead now. Does this help me?
#define gs_hash_table_get(__HT, __HTK)\
    ((__HT)->tmp_key = (__HTK),\
        (gs_hash_table_geti((__HT),\
            gs_hash_table_get_key_index_func((void**)&(__HT)->data, (void*)&((__HT)->tmp_key), sizeof((__HT)->tmp_key), sizeof((__HT)->tmp_val), (__HT)->stride, (__HT)->klpvl))))

#define gs_hash_table_getp(__HT, __HTK)\
    ((__HT)->tmp_key = (__HTK),\
        (&gs_hash_table_geti((__HT),\
            gs_hash_table_get_key_index_func((void**)&(__HT)->data, (void*)&((__HT)->tmp_key), sizeof((__HT)->tmp_key), sizeof((__HT)->tmp_val), (__HT)->stride, (__HT)->klpvl))))

#define gs_hash_table_key_exists(__HT, __HTK)\
    ((__HT)->tmp_key = (__HTK),\
        (gs_hash_table_get_key_index_func((void**)&(__HT->data), (void*)&(__HT->tmp_key), sizeof(__HT->tmp_key), sizeof(__HT->tmp_val), __HT->stride, __HT->klpvl) != GS_HASH_TABLE_INVALID_INDEX))

#define gs_hash_table_erase(__HT, __HTK)\
    do {\
        /* Get idx for key */\
        (__HT)->tmp_key = (__HTK);\
        uint32_t __IDX = gs_hash_table_get_key_index_func((void**)&(__HT)->data, (void*)&((__HT)->tmp_key), sizeof((__HT)->tmp_key), sizeof((__HT)->tmp_val), (__HT)->stride, (__HT)->klpvl);\
        if (__IDX != GS_HASH_TABLE_INVALID_INDEX) {\
            (__HT)->data[__IDX].state = GS_HASH_TABLE_ENTRY_INACTIVE;\
        }\
    } while (0)

/*===== Hash Table Iterator ====*/

typedef uint32_t gs_hash_table_iter;

// uint32_t gs_hash_table_get_key_index_func(void** data, void* key, size_t key_len, size_t val_len, size_t stride, size_t klpvl)

gs_force_inline
uint32_t __gs_find_first_valid_iterator(void* data, size_t key_len, size_t val_len, uint32_t idx, size_t stride, size_t klpvl)
{
    uint32_t it = (uint32_t)idx;
    for (; it < (uint32_t)gs_dyn_array_capacity(data); ++it)
    {
        size_t offset = (it * stride);
        gs_hash_table_entry_state state = *(gs_hash_table_entry_state*)((uint8_t*)data + offset + (klpvl));
        if (state == GS_HASH_TABLE_ENTRY_ACTIVE)
        {
            break;
        }
    }
    return it;
}

/* Find first valid iterator idx */
#define gs_hash_table_iter_new(__HT)\
    (__gs_find_first_valid_iterator((__HT)->data, sizeof((__HT)->tmp_key), sizeof((__HT)->tmp_val), 0, (__HT)->stride, (__HT)->klpvl))

#define gs_hash_table_iter_valid(__HT, __IT)\
    ((__IT) < gs_hash_table_capacity((__HT)))

// Have to be able to do this for hash table...
gs_force_inline
void __gs_hash_table_iter_advance_func(void** data, size_t key_len, size_t val_len, uint32_t* it, size_t stride, size_t klpvl)
{
    (*it)++;
    for (; *it < (uint32_t)gs_dyn_array_capacity(*data); ++*it)
    {
        size_t offset = (size_t)(*it * stride);
        gs_hash_table_entry_state state = *(gs_hash_table_entry_state*)((uint8_t*)*data + offset + (klpvl));
        if (state == GS_HASH_TABLE_ENTRY_ACTIVE)
        {
            break;
        }
    }
}

#define gs_hash_table_find_valid_iter(__HT, __IT)\
    ((__IT) = __gs_find_first_valid_iterator((void**)&(__HT)->data, sizeof((__HT)->tmp_key), sizeof((__HT)->tmp_val), (__IT), (__HT)->stride, (__HT)->klpvl))

#define gs_hash_table_iter_advance(__HT, __IT)\
    (__gs_hash_table_iter_advance_func((void**)&(__HT)->data, sizeof((__HT)->tmp_key), sizeof((__HT)->tmp_val), &(__IT), (__HT)->stride, (__HT)->klpvl))

#define gs_hash_table_iter_get(__HT, __IT)\
    gs_hash_table_geti(__HT, __IT)

#define gs_hash_table_iter_getp(__HT, __IT)\
    (&(gs_hash_table_geti(__HT, __IT)))

#define gs_hash_table_iter_getk(__HT, __IT)\
    (gs_hash_table_getk(__HT, __IT))

#define gs_hash_table_iter_getkp(__HT, __IT)\
    (&(gs_hash_table_getk(__HT, __IT)))

/*===================================
// Slot Array
===================================*/

#define GS_SLOT_ARRAY_INVALID_HANDLE    UINT32_MAX

#define gs_slot_array_handle_valid(__SA, __ID)\
    (__ID < gs_dyn_array_size((__SA)->indices) && (__SA)->indices[__ID] != GS_SLOT_ARRAY_INVALID_HANDLE)

typedef struct __gs_slot_array_dummy_header {
    gs_dyn_array(uint32_t) indices;
    gs_dyn_array(uint32_t) data;
} __gs_slot_array_dummy_header;

#define gs_slot_array(__T)\
    struct\
    {\
        gs_dyn_array(uint32_t) indices;\
        gs_dyn_array(__T) data;\
        __T tmp;\
    }*

#define gs_slot_array_new(__T)\
    NULL

gs_force_inline
uint32_t __gs_slot_array_find_next_available_index(gs_dyn_array(uint32_t) indices)
{
    uint32_t idx = GS_SLOT_ARRAY_INVALID_HANDLE;
    for (uint32_t i = 0; i < (uint32_t)gs_dyn_array_size(indices); ++i)
    {
        uint32_t handle = indices[i];
        if (handle == GS_SLOT_ARRAY_INVALID_HANDLE)
        {
            idx = i;
            break;
        }
    }
    if (idx == GS_SLOT_ARRAY_INVALID_HANDLE)
    {
        idx = gs_dyn_array_size(indices);
    }

    return idx;
}

gs_force_inline
void** gs_slot_array_init(void** sa, size_t sz)
{
    if (*sa == NULL) {
        *sa = gs_malloc(sz);
        memset(*sa, 0, sz);
        return sa;
    }
    else {
        return NULL;
    }
}

#define gs_slot_array_init_all(__SA)\
    (gs_slot_array_init((void**)&(__SA), sizeof(*(__SA))), gs_dyn_array_init((void**)&((__SA)->indices), sizeof(uint32_t)),\
        gs_dyn_array_init((void**)&((__SA)->data), sizeof((__SA)->tmp)))

gs_force_inline
uint32_t gs_slot_array_insert_func(void** indices, void** data, void* val, size_t val_len, uint32_t* ip)
{
    // Find next available index
    u32 idx = __gs_slot_array_find_next_available_index((uint32_t*)*indices);

    if (idx == gs_dyn_array_size(*indices)) {
        uint32_t v = 0;
        gs_dyn_array_push_data(indices, &v, sizeof(uint32_t));  
        idx = gs_dyn_array_size(*indices) - 1;
    }

    // Push data to array
    gs_dyn_array_push_data(data, val, val_len);

    // Set data in indices
    uint32_t bi = gs_dyn_array_size(*data) - 1;
    gs_dyn_array_set_data_i(indices, &bi, sizeof(uint32_t), idx);

    if (ip){
        *ip = idx;
    }

    return idx;
}

#define gs_slot_array_insert(__SA, __VAL)\
    (gs_slot_array_init_all(__SA), (__SA)->tmp = (__VAL),\
        gs_slot_array_insert_func((void**)&((__SA)->indices), (void**)&((__SA)->data), (void*)&((__SA)->tmp), sizeof(((__SA)->tmp)), NULL))

#define gs_slot_array_insert_hp(__SA, __VAL, __hp)\
    (gs_slot_array_init_all(__SA), (__SA)->tmp = (__VAL),\
        gs_slot_array_insert_func((void**)&((__SA)->indices), (void**)&((__SA)->data), &((__SA)->tmp), sizeof(((__SA)->tmp)), (__hp)))

#define gs_slot_array_insert_no_init(__SA, __VAL)\
    ((__SA)->tmp = (__VAL), gs_slot_array_insert_func((void**)&((__SA)->indices), (void**)&((__SA)->data), &((__SA)->tmp), sizeof(((__SA)->tmp)), NULL))

#define gs_slot_array_size(__SA)\
    ((__SA) == NULL ? 0 : gs_dyn_array_size((__SA)->data))

 #define gs_slot_array_empty(__SA)\
    (gs_slot_array_size(__SA) == 0)

#define gs_slot_array_clear(__SA)\
    do {\
        if ((__SA) != NULL) {\
            gs_dyn_array_clear((__SA)->data);\
            gs_dyn_array_clear((__SA)->indices);\
        }\
    } while (0)

#define gs_slot_array_exists(__SA, __SID)\
    (__SID < gs_slot_array_size(__SA))

 #define gs_slot_array_get(__SA, __SID)\
    ((__SA)->data[(__SA)->indices[(__SID) % gs_dyn_array_size(((__SA)->indices))]])

 #define gs_slot_array_getp(__SA, __SID)\
    (&(gs_slot_array_get(__SA, (__SID))))

 #define gs_slot_array_free(__SA)\
    do {\
        if ((__SA) != NULL) {\
            gs_dyn_array_free((__SA)->data);\
            gs_dyn_array_free((__SA)->indices);\
            (__SA)->indices = NULL;\
            (__SA)->data = NULL;\
            gs_free((__SA));\
            (__SA) = NULL;\
        }\
    } while (0)

 #define gs_slot_array_erase(__SA, __id)\
    do {\
        uint32_t __H0 = (__id) % gs_dyn_array_size((__SA)->indices);\
        if (gs_slot_array_size(__SA) == 1) {\
            gs_slot_array_clear(__SA);\
        }\
        else if (!gs_slot_array_handle_valid(__SA, __H0)) {\
            gs_println("Warning: Attempting to erase invalid slot array handle (%zu)", __H0);\
        }\
        else {\
            uint32_t __OG_DATA_IDX = (__SA)->indices[__H0];\
            /* Iterate through handles until last index of data found */\
            uint32_t __H = 0;\
            for (uint32_t __I = 0; __I < gs_dyn_array_size((__SA)->indices); ++__I)\
            {\
                if ((__SA)->indices[__I] == gs_dyn_array_size((__SA)->data) - 1)\
                {\
                    __H = __I;\
                    break;\
                }\
            }\
        \
            /* Swap and pop data */\
            (__SA)->data[__OG_DATA_IDX] = gs_dyn_array_back((__SA)->data);\
            gs_dyn_array_pop((__SA)->data);\
        \
            /* Point new handle, Set og handle to invalid */\
            (__SA)->indices[__H] = __OG_DATA_IDX;\
            (__SA)->indices[__H0] = GS_SLOT_ARRAY_INVALID_HANDLE;\
        }\
    } while (0)

/*=== Slot Array Iterator ===*/

// Slot array iterator new
typedef uint32_t gs_slot_array_iter;

#define gs_slot_array_iter_new(__SA) 0

#define gs_slot_array_iter_valid(__SA, __IT)\
    ((__IT) < (uint32_t)gs_slot_array_size((__SA)))

gs_force_inline
void __gs_slot_array_iter_advance_func(gs_dyn_array(uint32_t) indices, uint32_t* it)
{
    (*it)++;
    for (; *it < (uint32_t)gs_dyn_array_size(indices); ++*it)
    {\
        if (indices[*it] != GS_SLOT_ARRAY_INVALID_HANDLE)\
        {\
            break;\
        }\
    }\
}

#define gs_slot_array_iter_advance(__SA, __IT)\
    __gs_slot_array_iter_advance_func((__SA)->indices, &(__IT))

#define gs_slot_array_iter_get(__SA, __IT)\
    gs_slot_array_get(__SA, __IT)

#define gs_slot_array_iter_getp(__SA, __IT)\
    gs_slot_array_getp(__SA, __IT)

/*===================================
// Slot Map
===================================*/

#define gs_slot_map(__SMK, __SMV)\
    struct {\
        gs_hash_table(__SMK, uint32_t) ht;\
        gs_slot_array(__SMV) sa;\
    }*

#define gs_slot_map_new(__SMK, __SMV)\
    NULL

gs_force_inline
void** gs_slot_map_init(void** sm)
{
    if (*sm == NULL) {
        (*sm) = gs_malloc(sizeof(size_t) * 2);\
        memset((*sm), 0, sizeof(size_t) * 2);\
        return sm;
    }   
    return NULL;
}

// Could return something, I believe?
#define gs_slot_map_insert(__SM, __SMK, __SMV)\
    do {\
        gs_slot_map_init(&(__SM));\
        uint32_t __H = gs_slot_array_insert((__SM)->sa, ((__SMV)));\
        gs_hash_table_insert((__SM)->ht, (__SMK), __H);\
    } while (0)

#define gs_slot_map_get(__SM, __SMK)\
    (gs_slot_array_get((__SM)->sa, gs_hash_table_get((__SM)->ht, (__SMK))))

#define gs_slot_map_getp(__SM, __SMK)\
    (gs_slot_array_getp((__SM)->sa, gs_hash_table_get((__SM)->ht, (__SMK))))

#define gs_slot_map_size(__SM)\
    (gs_slot_array_size((__SM)->sa))

#define gs_slot_map_clear(__SM)\
    do {\
        if ((__SM) != NULL) {\
            gs_hash_table_clear((__SM)->ht);\
            gs_slot_array_clear((__SM)->sa);\
        }\
    } while (0)

#define gs_slot_map_erase(__SM, __SMK)\
    do {\
        uint32_t __K = gs_hash_table_get((__SM)->ht, (__SMK));\
        gs_hash_table_erase((__SM)->ht, (__SMK));\
        gs_slot_array_erase((__SM)->sa, __K);\
    } while (0)

#define gs_slot_map_free(__SM)\
    do {\
        if (__SM != NULL) {\
            gs_hash_table_free((__SM)->ht);\
            gs_slot_array_free((__SM)->sa);\
            gs_free((__SM));\
            (__SM) = NULL;\
        }\
    } while (0)

#define gs_slot_map_capacity(__SM)\
    (gs_hash_table_capacity((__SM)->ht))

/*=== Slot Map Iterator ===*/

typedef uint32_t gs_slot_map_iter;

/* Find first valid iterator idx */
#define gs_slot_map_iter_new(__SM)\
    gs_hash_table_iter_new((__SM)->ht)

#define gs_slot_map_iter_valid(__SM, __IT)\
    ((__IT) < gs_hash_table_capacity((__SM)->ht))

#define gs_slot_map_iter_advance(__SM, __IT)\
    __gs_hash_table_iter_advance_func((void**)&((__SM)->ht->data), sizeof((__SM)->ht->tmp_key), sizeof((__SM)->ht->tmp_val), &(__IT), (__SM)->ht->stride, (__SM)->ht->klpvl)

#define gs_slot_map_iter_getk(__SM, __IT)\
    gs_hash_table_iter_getk((__SM)->ht, (__IT))
    //(gs_hash_table_find_valid_iter(__SM->ht, __IT), gs_hash_table_geti((__SM)->ht, (__IT)))

#define gs_slot_map_iter_getkp(__SM, __IT)\
    (gs_hash_table_find_valid_iter(__SM->ht, __IT), &(gs_hash_table_geti((__SM)->ht, (__IT))))

#define gs_slot_map_iter_get(__SM, __IT)\
    ((__SM)->sa->data[gs_hash_table_iter_get((__SM)->ht, (__IT))])

    // ((__SM)->sa->data[gs_hash_table_geti((__SM)->ht, (__IT))])
    // (gs_hash_table_find_valid_iter(__SM->ht, __IT), (__SM)->sa->data[gs_hash_table_geti((__SM)->ht, (__IT))])

#define gs_slot_map_iter_getp(__SM, __IT)\
    (&((__SM)->sa->data[gs_hash_table_geti((__SM)->ht, (__IT))]))

    // (gs_hash_table_find_valid_iter(__SM->ht, __IT), &((__SM)->sa->data[gs_hash_table_geti((__SM)->ht, (__IT))]))

typedef uint32_t gs_slot_map_iter;

/*===================================
// Command Buffer
===================================*/

typedef struct gs_command_buffer_t
{
    uint32_t num_commands;
    gs_byte_buffer_t commands;
} gs_command_buffer_t;

gs_force_inline
gs_command_buffer_t gs_command_buffer_new()
{
    gs_command_buffer_t cb = gs_default_val();
    cb.commands = gs_byte_buffer_new();
    return cb;
}

#define gs_command_buffer_write(__CB, __T, __VAL)\
    do {\
        gs_byte_buffer_write(&__CB->commands, __T, __VAL);\
        __CB->num_commands++;\
    } while (0)

gs_force_inline 
void gs_command_buffer_clear(gs_command_buffer_t* cb)
{
    cb->num_commands = 0;
    gs_byte_buffer_clear(&cb->commands);
}

gs_force_inline
void gs_command_buffer_free(gs_command_buffer_t* cb)
{
    gs_byte_buffer_free(&cb->commands);
}

#ifndef GS_NO_SHORT_NAME
    typedef gs_command_buffer_t gs_cmdbuf;
#endif

/*========================
// GS_MATH
========================*/

// Defines
#define GS_PI       3.1415926535897932
#define GS_TAU      2.0 * GS_PI

// Useful Utility
#define gs_v2(...)  gs_vec2_ctor(__VA_ARGS__)
#define gs_v3(...)  gs_vec3_ctor(__VA_ARGS__)
#define gs_v4(...)  gs_vec4_ctor(__VA_ARGS__)
#define gs_quat(...) gs_quat_ctor(__VA_ARGS__)

#define gs_v2s(__S)  gs_vec2_ctor((__S), (__S))
#define gs_v3s(__S)  gs_vec3_ctor((__S), (__S), (__S))
#define gs_v4s(__S)  gs_vec4_ctor((__S), (__S), (__S), (__S))

#define gs_v4_xy_v(__X, __Y, __V) gs_vec4_ctor((__X), (__Y), (__V).x, (__V).y)

#define GS_XAXIS    gs_v3(1.f, 0.f, 0.f)
#define GS_YAXIS    gs_v3(0.f, 1.f, 0.f)
#define GS_ZAXIS    gs_v3(0.f, 0.f, 1.f)

/*================================================================================
// Useful Common Math Functions
================================================================================*/

#define gs_rad2deg(__R)\
    (float)((__R * 180.0f) / GS_PI) 

#define gs_deg2rad(__D)\
    (float)((__D * GS_PI) / 180.0f)

// Interpolation
// Source: https://codeplea.com/simple-interpolation

gs_inline float
gs_interp_linear(float a, float b, float t)
{
    return (a + t * (b - a));
}

gs_inline float
gs_interp_smooth_step(float a, float b, float t)
{
    return gs_interp_linear(a, b, t * t * (3.0f - 2.0f * t));
}

gs_inline float 
gs_interp_cosine(float a, float b, float t)
{
    return gs_interp_linear(a, b, (float)-cos(GS_PI * t) * 0.5f + 0.5f);
}

gs_inline float 
gs_interp_acceleration(float a, float b, float t) 
{
    return gs_interp_linear(a, b, t * t);
}

gs_inline float 
gs_interp_deceleration(float a, float b, float t) 
{
    return gs_interp_linear(a, b, 1.0f - (1.0f - t) * (1.0f - t));
}

gs_inline float 
gs_round(float val) 
{
    return (float)floor(val + 0.5f);
}

gs_inline float
gs_map_range(float input_start, float input_end, float output_start, float output_end, float val)
{
    float slope = (output_end - output_start) / (input_end - input_start);
    return (output_start + (slope * (val - input_start)));
}

// Easings from: https://github.com/raysan5/raylib/blob/ea0f6c7a26f3a61f3be542aa8f066ce033766a9f/examples/others/easings.h
gs_inline
float gs_ease_cubic_in(float t, float b, float c, float d) 
{ 
    t /= d; 
    return (c*t*t*t + b); 
}

gs_inline
float gs_ease_cubic_out(float t, float b, float c, float d) 
{ 
    t = t/d - 1.0f; 
    return (c*(t*t*t + 1.0f) + b); 
}

gs_inline
float gs_ease_cubic_in_out(float t, float b, float c, float d)
{
    if ((t/=d/2.0f) < 1.0f) 
    {
        return (c/2.0f*t*t*t + b);
    }
    t -= 2.0f; 
    return (c/2.0f*(t*t*t + 2.0f) + b);
}

/*================================================================================
// Vec2
================================================================================*/

typedef struct 
{
    union 
    {
        f32 xy[2];
        struct 
        {
            f32 x, y;
        };
    };
} gs_vec2;

gs_inline gs_vec2 
gs_vec2_ctor(f32 _x, f32 _y) 
{
    gs_vec2 v;
    v.x = _x;
    v.y = _y;
    return v;
}

gs_inline gs_vec2 
gs_vec2_add(gs_vec2 v0, gs_vec2 v1) 
{
    return gs_vec2_ctor(v0.x + v1.x, v0.y + v1.y);
}

gs_inline gs_vec2 
gs_vec2_sub(gs_vec2 v0, gs_vec2 v1)
{
    return gs_vec2_ctor(v0.x - v1.x, v0.y - v1.y);
}

gs_inline gs_vec2 
gs_vec2_mul(gs_vec2 v0, gs_vec2 v1) 
{
    return gs_vec2_ctor(v0.x * v1.x, v0.y * v1.y);
}

gs_inline gs_vec2 
gs_vec2_div(gs_vec2 v0, gs_vec2 v1) 
{
    return gs_vec2_ctor(v0.x / v1.x, v0.y / v1.y);
}

gs_inline gs_vec2 
gs_vec2_scale(gs_vec2 v, f32 s)
{
    return gs_vec2_ctor(v.x * s, v.y * s);
}

gs_inline f32 
gs_vec2_dot(gs_vec2 v0, gs_vec2 v1) 
{
    return (f32)(v0.x * v1.x + v0.y * v1.y);
}

gs_inline f32 
gs_vec2_len(gs_vec2 v)
{
    return (f32)sqrt(gs_vec2_dot(v, v));
}

gs_inline gs_vec2
gs_vec2_project_onto(gs_vec2 v0, gs_vec2 v1)
{
    f32 dot = gs_vec2_dot(v0, v1);
    f32 len = gs_vec2_dot(v1, v1);

    // Orthogonal, so return v1
    if (len == 0.f) return v1;

    return gs_vec2_scale(v1, dot / len);
}

gs_inline gs_vec2 gs_vec2_norm(gs_vec2 v) 
{
    f32 len = gs_vec2_len(v);
    return gs_vec2_scale(v, len != 0.f ? 1.0f / gs_vec2_len(v) : 1.f);
}

gs_inline 
f32 gs_vec2_dist(gs_vec2 a, gs_vec2 b)
{
    f32 dx = (a.x - b.x);
    f32 dy = (a.y - b.y);
    return (float)(sqrt(dx * dx + dy * dy));
}

gs_inline
f32 gs_vec2_cross(gs_vec2 a, gs_vec2 b) 
{
    return a.x * b.y - a.y * b.x;
}

gs_inline
f32 gs_vec2_angle(gs_vec2 a, gs_vec2 b) 
{
    return (float)acos(gs_vec2_dot(a, b) / (gs_vec2_len(a) * gs_vec2_len(b)));
}

gs_inline
b32 gs_vec2_equal(gs_vec2 a, gs_vec2 b)
{
    return (a.x == b.x && a.y == b.y);
}

/*================================================================================
// Vec3
================================================================================*/

typedef struct
{
    union 
    {
        f32 xyz[3];
        struct 
        {
            f32 x, y, z;
        };
    };
} gs_vec3;

gs_inline gs_vec3 
gs_vec3_ctor(f32 _x, f32 _y, f32 _z)
{
    gs_vec3 v;
    v.x = _x;
    v.y = _y;
    v.z = _z;
    return v;
}

gs_inline gs_vec3 
gs_vec3_add(gs_vec3 v0, gs_vec3 v1)
{
    return gs_vec3_ctor(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z);
}

gs_inline gs_vec3 
gs_vec3_sub(gs_vec3 v0, gs_vec3 v1) 
{
    return gs_vec3_ctor(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
}

gs_inline gs_vec3 
gs_vec3_mul(gs_vec3 v0, gs_vec3 v1) 
{
    return gs_vec3_ctor(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z);
}

gs_inline gs_vec3 
gs_vec3_div(gs_vec3 v0, gs_vec3 v1) 
{
    return gs_vec3_ctor(v0.x / v1.x, v0.y / v1.y, v0.z / v1.z);
}

gs_inline gs_vec3 
gs_vec3_scale(gs_vec3 v, f32 s) 
{
    return gs_vec3_ctor(v.x * s, v.y * s, v.z * s);
}

gs_inline f32 
gs_vec3_dot(gs_vec3 v0, gs_vec3 v1) 
{
    f32 dot = (f32)((v0.x * v1.x) + (v0.y * v1.y) + v0.z * v1.z);
    return dot;
}

gs_inline f32 
gs_vec3_len(gs_vec3 v)
{
    return (f32)sqrt(gs_vec3_dot(v, v));
}

gs_inline gs_vec3
gs_vec3_project_onto(gs_vec3 v0, gs_vec3 v1)
{
    f32 dot = gs_vec3_dot(v0, v1);
    f32 len = gs_vec3_dot(v1, v1);

    // Orthogonal, so return v1
    if (len == 0.f) return v1;

    return gs_vec3_scale(v1, dot / len);
}

gs_inline 
f32 gs_vec3_dist(gs_vec3 a, gs_vec3 b)
{
    f32 dx = (a.x - b.x);
    f32 dy = (a.y - b.y);
    f32 dz = (a.z - b.z);
    return (float)(sqrt(dx * dx + dy * dy + dz * dz));
}

gs_inline gs_vec3 
gs_vec3_norm(gs_vec3 v)
{
    f32 len = gs_vec3_len(v);
    return len == 0.f ? v : gs_vec3_scale(v, 1.f / len);
}

gs_inline gs_vec3 
gs_vec3_cross(gs_vec3 v0, gs_vec3 v1) 
{
    return gs_vec3_ctor
    (
        v0.y * v1.z - v0.z * v1.y,
        v0.z * v1.x - v0.x * v1.z,
        v0.x * v1.y - v0.y * v1.x
    );
}

gs_inline void gs_vec3_scale_ip(gs_vec3* vp, f32 s)
{
    vp->x *= s;
    vp->y *= s;
    vp->z *= s;
}

/*================================================================================
// Vec4
================================================================================*/

typedef struct
{
    union 
    {
        f32 xyzw[4];
        struct 
        {
            f32 x, y, z, w;
        };
    };
} gs_vec4;

gs_inline gs_vec4 
gs_vec4_ctor(f32 _x, f32 _y, f32 _z, f32 _w)
{
    gs_vec4 v; 
    v.x = _x;
    v.y = _y; 
    v.z = _z; 
    v.w = _w;
    return v;
} 

gs_inline gs_vec4
gs_vec4_add(gs_vec4 v0, gs_vec4 v1) 
{
    return gs_vec4_ctor(v0.x + v1.y, v0.y + v1.y, v0.z + v1.z, v0.w + v1.w);
}

gs_inline gs_vec4
gs_vec4_sub(gs_vec4 v0, gs_vec4 v1) 
{
    return gs_vec4_ctor(v0.x - v1.y, v0.y - v1.y, v0.z - v1.z, v0.w - v1.w);
}

gs_inline gs_vec4
gs_vec4_div(gs_vec4 v0, gs_vec4 v1) 
{
    return gs_vec4_ctor(v0.x / v1.x, v0.y / v1.y, v0.z / v1.z, v0.w / v1.w);
}

gs_inline gs_vec4
gs_vec4_scale(gs_vec4 v, f32 s) 
{
    return gs_vec4_ctor(v.x / s, v.y / s, v.z / s, v.w / s);
}

gs_inline f32
gs_vec4_dot(gs_vec4 v0, gs_vec4 v1) 
{
    return (f32)(v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w);
}

gs_inline f32
gs_vec4_len(gs_vec4 v) 
{
    return (f32)sqrt(gs_vec4_dot(v, v));
}

gs_inline gs_vec4
gs_vec4_project_onto(gs_vec4 v0, gs_vec4 v1)
{
    f32 dot = gs_vec4_dot(v0, v1);
    f32 len = gs_vec4_dot(v1, v1);

    // Orthogonal, so return v1
    if (len == 0.f) return v1;

    return gs_vec4_scale(v1, dot / len);
}

gs_inline gs_vec4
gs_vec4_norm(gs_vec4 v) 
{
    return gs_vec4_scale(v, 1.0f / gs_vec4_len(v));
}

gs_inline f32
gs_vec4_dist(gs_vec4 v0, gs_vec4 v1)
{
    f32 dx = (v0.x - v1.x);
    f32 dy = (v0.y - v1.y);
    f32 dz = (v0.z - v1.z);
    f32 dw = (v0.w - v1.w);
    return (float)(sqrt(dx * dx + dy * dy + dz * dz + dw * dw));
}

/*================================================================================
// Useful Vector Functions
================================================================================*/

gs_inline
gs_vec3 gs_v4_to_v3(gs_vec4 v) 
{
    return gs_v3(v.x, v.y, v.z);
}

/*================================================================================
// Mat4x4
================================================================================*/

/*
    Matrices are stored in linear, contiguous memory and assume a column-major ordering.
*/

typedef struct gs_mat4
{
    f32 elements[16];
} gs_mat4;

gs_inline gs_mat4 
gs_mat4_diag(f32 val)
{
    gs_mat4 m;
    memset(m.elements, 0, sizeof(m.elements));
    m.elements[0 + 0 * 4] = val;
    m.elements[1 + 1 * 4] = val;
    m.elements[2 + 2 * 4] = val;
    m.elements[3 + 3 * 4] = val;
    return m;
}

#define gs_mat4_identity()\
    gs_mat4_diag(1.0f)

gs_inline gs_mat4
gs_mat4_ctor() {
    gs_mat4 mat = gs_default_val();
    return mat;
}

gs_inline
gs_mat4 gs_mat4_elem(const float* elements)
{
    gs_mat4 mat = gs_mat4_ctor();
    memcpy(mat.elements, elements, sizeof(f32) * 16);
    return mat;
}

gs_inline gs_mat4 
gs_mat4_mul(gs_mat4 m0, gs_mat4 m1)
{
    gs_mat4 m_res = gs_mat4_ctor(); 
    for (u32 y = 0; y < 4; ++y)
    {
        for (u32 x = 0; x < 4; ++x)
        {
            f32 sum = 0.0f;
            for (u32 e = 0; e < 4; ++e)
            {
                sum += m0.elements[x + e * 4] * m1.elements[e + y * 4];
            }
            m_res.elements[x + y * 4] = sum;
        }
    }

    return m_res;
}

gs_inline 
gs_mat4 gs_mat4_mul_list(uint32_t count, ...)
{
    va_list ap;
    gs_mat4 m = gs_mat4_identity();
    va_start(ap, count);
    for (uint32_t i = 0; i < count; ++i) {
        m = gs_mat4_mul(m, va_arg(ap, gs_mat4));
    }
    va_end(ap);
    return m;
}

gs_inline
void gs_mat4_set_elements(gs_mat4* m, float* elements, uint32_t count)
{
    for (u32 i = 0; i < count; ++i)
    {
        m->elements[i] = elements[i];
    }
}

gs_inline
gs_mat4 gs_mat4_transpose(gs_mat4 m)
{
    gs_mat4 t = gs_mat4_identity();

    // First row
    t.elements[0 * 4 + 0] = m.elements[0 * 4 + 0];
    t.elements[1 * 4 + 0] = m.elements[0 * 4 + 1];
    t.elements[2 * 4 + 0] = m.elements[0 * 4 + 2];
    t.elements[3 * 4 + 0] = m.elements[0 * 4 + 3];

    // Second row
    t.elements[0 * 4 + 1] = m.elements[1 * 4 + 0];
    t.elements[1 * 4 + 1] = m.elements[1 * 4 + 1];
    t.elements[2 * 4 + 1] = m.elements[1 * 4 + 2];
    t.elements[3 * 4 + 1] = m.elements[1 * 4 + 3];

    // Third row
    t.elements[0 * 4 + 2] = m.elements[2 * 4 + 0];
    t.elements[1 * 4 + 2] = m.elements[2 * 4 + 1];
    t.elements[2 * 4 + 2] = m.elements[2 * 4 + 2];
    t.elements[3 * 4 + 2] = m.elements[2 * 4 + 3];

    // Fourth row
    t.elements[0 * 4 + 3] = m.elements[3 * 4 + 0];
    t.elements[1 * 4 + 3] = m.elements[3 * 4 + 1];
    t.elements[2 * 4 + 3] = m.elements[3 * 4 + 2];
    t.elements[3 * 4 + 3] = m.elements[3 * 4 + 3];

    return t;
}

gs_inline
gs_mat4 gs_mat4_inverse(gs_mat4 m)
{
    gs_mat4 res = gs_mat4_identity();

    f32 temp[16];

    temp[0] = m.elements[5] * m.elements[10] * m.elements[15] -
        m.elements[5] * m.elements[11] * m.elements[14] -
        m.elements[9] * m.elements[6] * m.elements[15] +
        m.elements[9] * m.elements[7] * m.elements[14] +
        m.elements[13] * m.elements[6] * m.elements[11] -
        m.elements[13] * m.elements[7] * m.elements[10];

    temp[4] = -m.elements[4] * m.elements[10] * m.elements[15] +
        m.elements[4] * m.elements[11] * m.elements[14] +
        m.elements[8] * m.elements[6] * m.elements[15] -
        m.elements[8] * m.elements[7] * m.elements[14] -
        m.elements[12] * m.elements[6] * m.elements[11] +
        m.elements[12] * m.elements[7] * m.elements[10];

    temp[8] = m.elements[4] * m.elements[9] * m.elements[15] -
        m.elements[4] * m.elements[11] * m.elements[13] -
        m.elements[8] * m.elements[5] * m.elements[15] +
        m.elements[8] * m.elements[7] * m.elements[13] +
        m.elements[12] * m.elements[5] * m.elements[11] -
        m.elements[12] * m.elements[7] * m.elements[9];

    temp[12] = -m.elements[4] * m.elements[9] * m.elements[14] +
        m.elements[4] * m.elements[10] * m.elements[13] +
        m.elements[8] * m.elements[5] * m.elements[14] -
        m.elements[8] * m.elements[6] * m.elements[13] -
        m.elements[12] * m.elements[5] * m.elements[10] +
        m.elements[12] * m.elements[6] * m.elements[9];

    temp[1] = -m.elements[1] * m.elements[10] * m.elements[15] +
        m.elements[1] * m.elements[11] * m.elements[14] +
        m.elements[9] * m.elements[2] * m.elements[15] -
        m.elements[9] * m.elements[3] * m.elements[14] -
        m.elements[13] * m.elements[2] * m.elements[11] +
        m.elements[13] * m.elements[3] * m.elements[10];

    temp[5] = m.elements[0] * m.elements[10] * m.elements[15] -
        m.elements[0] * m.elements[11] * m.elements[14] -
        m.elements[8] * m.elements[2] * m.elements[15] +
        m.elements[8] * m.elements[3] * m.elements[14] +
        m.elements[12] * m.elements[2] * m.elements[11] -
        m.elements[12] * m.elements[3] * m.elements[10];

    temp[9] = -m.elements[0] * m.elements[9] * m.elements[15] +
        m.elements[0] * m.elements[11] * m.elements[13] +
        m.elements[8] * m.elements[1] * m.elements[15] -
        m.elements[8] * m.elements[3] * m.elements[13] -
        m.elements[12] * m.elements[1] * m.elements[11] +
        m.elements[12] * m.elements[3] * m.elements[9];

    temp[13] = m.elements[0] * m.elements[9] * m.elements[14] -
        m.elements[0] * m.elements[10] * m.elements[13] -
        m.elements[8] * m.elements[1] * m.elements[14] +
        m.elements[8] * m.elements[2] * m.elements[13] +
        m.elements[12] * m.elements[1] * m.elements[10] -
        m.elements[12] * m.elements[2] * m.elements[9];

    temp[2] = m.elements[1] * m.elements[6] * m.elements[15] -
        m.elements[1] * m.elements[7] * m.elements[14] -
        m.elements[5] * m.elements[2] * m.elements[15] +
        m.elements[5] * m.elements[3] * m.elements[14] +
        m.elements[13] * m.elements[2] * m.elements[7] -
        m.elements[13] * m.elements[3] * m.elements[6];

    temp[6] = -m.elements[0] * m.elements[6] * m.elements[15] +
        m.elements[0] * m.elements[7] * m.elements[14] +
        m.elements[4] * m.elements[2] * m.elements[15] -
        m.elements[4] * m.elements[3] * m.elements[14] -
        m.elements[12] * m.elements[2] * m.elements[7] +
        m.elements[12] * m.elements[3] * m.elements[6];

    temp[10] = m.elements[0] * m.elements[5] * m.elements[15] -
        m.elements[0] * m.elements[7] * m.elements[13] -
        m.elements[4] * m.elements[1] * m.elements[15] +
        m.elements[4] * m.elements[3] * m.elements[13] +
        m.elements[12] * m.elements[1] * m.elements[7] -
        m.elements[12] * m.elements[3] * m.elements[5];

    temp[14] = -m.elements[0] * m.elements[5] * m.elements[14] +
        m.elements[0] * m.elements[6] * m.elements[13] +
        m.elements[4] * m.elements[1] * m.elements[14] -
        m.elements[4] * m.elements[2] * m.elements[13] -
        m.elements[12] * m.elements[1] * m.elements[6] +
        m.elements[12] * m.elements[2] * m.elements[5];

    temp[3] = -m.elements[1] * m.elements[6] * m.elements[11] +
        m.elements[1] * m.elements[7] * m.elements[10] +
        m.elements[5] * m.elements[2] * m.elements[11] -
        m.elements[5] * m.elements[3] * m.elements[10] -
        m.elements[9] * m.elements[2] * m.elements[7] +
        m.elements[9] * m.elements[3] * m.elements[6];

    temp[7] = m.elements[0] * m.elements[6] * m.elements[11] -
        m.elements[0] * m.elements[7] * m.elements[10] -
        m.elements[4] * m.elements[2] * m.elements[11] +
        m.elements[4] * m.elements[3] * m.elements[10] +
        m.elements[8] * m.elements[2] * m.elements[7] -
        m.elements[8] * m.elements[3] * m.elements[6];

    temp[11] = -m.elements[0] * m.elements[5] * m.elements[11] +
        m.elements[0] * m.elements[7] * m.elements[9] +
        m.elements[4] * m.elements[1] * m.elements[11] -
        m.elements[4] * m.elements[3] * m.elements[9] -
        m.elements[8] * m.elements[1] * m.elements[7] +
        m.elements[8] * m.elements[3] * m.elements[5];

    temp[15] = m.elements[0] * m.elements[5] * m.elements[10] -
        m.elements[0] * m.elements[6] * m.elements[9] -
        m.elements[4] * m.elements[1] * m.elements[10] +
        m.elements[4] * m.elements[2] * m.elements[9] +
        m.elements[8] * m.elements[1] * m.elements[6] -
        m.elements[8] * m.elements[2] * m.elements[5];

    float determinant = m.elements[0] * temp[0] + m.elements[1] * temp[4] + m.elements[2] * temp[8] + m.elements[3] * temp[12];
    determinant = 1.0f / determinant;

    for (int i = 0; i < 4 * 4; i++)
        res.elements[i] = (float)(temp[i] * (float)determinant);

    return res;
}

/*
    f32 l : left
    f32 r : right
    f32 b : bottom
    f32 t : top
    f32 n : near
    f32 f : far
*/
gs_inline gs_mat4 
gs_mat4_ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
    gs_mat4 m_res = gs_mat4_identity();     

    // Main diagonal
    m_res.elements[0 + 0 * 4] = 2.0f / (r - l);
    m_res.elements[1 + 1 * 4] = 2.0f / (t - b);
    m_res.elements[2 + 2 * 4] = -2.0f / (f - n);

    // Last column
    m_res.elements[0 + 3 * 4] = -(r + l) / (r - l);
    m_res.elements[1 + 3 * 4] = -(t + b) / (t - b);
    m_res.elements[2 + 3 * 4] = -(f + n) / (f - n);

    return m_res;
}

gs_inline gs_mat4 
gs_mat4_perspective(f32 fov, f32 asp_ratio, f32 n, f32 f)
{
    // Zero matrix
    gs_mat4 m_res = gs_mat4_ctor();

    f32 q = 1.0f / (float)tan(gs_deg2rad(0.5f * fov));
    f32 a = q / asp_ratio;
    f32 b = (n + f) / (n - f);
    f32 c = (2.0f * n * f) / (n - f);

    m_res.elements[0 + 0 * 4] = a;
    m_res.elements[1 + 1 * 4] = q;
    m_res.elements[2 + 2 * 4] = b;
    m_res.elements[2 + 3 * 4] = c;
    m_res.elements[3 + 2 * 4] = -1.0f;

    return m_res;
}

gs_inline gs_mat4 
gs_mat4_translatev(const gs_vec3 v)
{
    gs_mat4 m_res = gs_mat4_identity();

    m_res.elements[0 + 4 * 3] = v.x;
    m_res.elements[1 + 4 * 3] = v.y;
    m_res.elements[2 + 4 * 3] = v.z;

    return m_res;
}

gs_inline gs_mat4 
gs_mat4_translate(float x, float y, float z)
{
    return gs_mat4_translatev(gs_v3(x, y, z));
}

gs_inline gs_mat4 
gs_mat4_scalev(const gs_vec3 v)
{
    gs_mat4 m_res = gs_mat4_identity();
    m_res.elements[0 + 0 * 4] = v.x;
    m_res.elements[1 + 1 * 4] = v.y;
    m_res.elements[2 + 2 * 4] = v.z;
    return m_res;
}

gs_inline gs_mat4
gs_mat4_scale(float x, float y, float z)
{
    return (gs_mat4_scalev(gs_v3(x, y, z)));
}

// Assumes normalized axis
gs_inline gs_mat4 gs_mat4_rotatev(float angle, gs_vec3 axis)
{
    gs_mat4 m_res = gs_mat4_identity();

    float a = angle;
    float c = (float)cos(a);
    float s = (float)sin(a);

    gs_vec3 naxis = gs_vec3_norm(axis);
    float x = naxis.x;
    float y = naxis.y;
    float z = naxis.z;

    //First column
    m_res.elements[0 + 0 * 4] = x * x * (1 - c) + c;    
    m_res.elements[1 + 0 * 4] = x * y * (1 - c) + z * s;    
    m_res.elements[2 + 0 * 4] = x * z * (1 - c) - y * s;    
    
    //Second column
    m_res.elements[0 + 1 * 4] = x * y * (1 - c) - z * s;    
    m_res.elements[1 + 1 * 4] = y * y * (1 - c) + c;    
    m_res.elements[2 + 1 * 4] = y * z * (1 - c) + x * s;    
    
    //Third column
    m_res.elements[0 + 2 * 4] = x * z * (1 - c) + y * s;    
    m_res.elements[1 + 2 * 4] = y * z * (1 - c) - x * s;    
    m_res.elements[2 + 2 * 4] = z * z * (1 - c) + c;    

    return m_res;
}

gs_inline gs_mat4
gs_mat4_rotate(float angle, float x, float y, float z)
{
    return gs_mat4_rotatev(angle, gs_v3(x, y, z));
}

gs_inline gs_mat4 
gs_mat4_look_at(gs_vec3 position, gs_vec3 target, gs_vec3 up)
{
    gs_vec3 f = gs_vec3_norm(gs_vec3_sub(target, position));
    gs_vec3 s = gs_vec3_norm(gs_vec3_cross(f, up));
    gs_vec3 u = gs_vec3_cross(s, f);

    gs_mat4 m_res = gs_mat4_identity();
    m_res.elements[0 * 4 + 0] = s.x;
    m_res.elements[1 * 4 + 0] = s.y;
    m_res.elements[2 * 4 + 0] = s.z;

    m_res.elements[0 * 4 + 1] = u.x;
    m_res.elements[1 * 4 + 1] = u.y;
    m_res.elements[2 * 4 + 1] = u.z;

    m_res.elements[0 * 4 + 2] = -f.x;
    m_res.elements[1 * 4 + 2] = -f.y;
    m_res.elements[2 * 4 + 2] = -f.z;

    m_res.elements[3 * 4 + 0] = -gs_vec3_dot(s, position);;
    m_res.elements[3 * 4 + 1] = -gs_vec3_dot(u, position);
    m_res.elements[3 * 4 + 2] = gs_vec3_dot(f, position); 

    return m_res;
}

gs_inline
gs_vec3 gs_mat4_mul_vec3(gs_mat4 m, gs_vec3 v)
{
    m = gs_mat4_transpose(m);
    return gs_vec3_ctor
    (
        m.elements[0 * 4 + 0] * v.x + m.elements[0 * 4 + 1] * v.y + m.elements[0 * 4 + 2] * v.z,  
        m.elements[1 * 4 + 0] * v.x + m.elements[1 * 4 + 1] * v.y + m.elements[1 * 4 + 2] * v.z,  
        m.elements[2 * 4 + 0] * v.x + m.elements[2 * 4 + 1] * v.y + m.elements[2 * 4 + 2] * v.z
    );
}
    
gs_inline
gs_vec4 gs_mat4_mul_vec4(gs_mat4 m, gs_vec4 v)
{
    m = gs_mat4_transpose(m);
    return gs_vec4_ctor
    (
        m.elements[0 * 4 + 0] * v.x + m.elements[0 * 4 + 1] * v.y + m.elements[0 * 4 + 2] * v.z + m.elements[0 * 4 + 3] * v.w,  
        m.elements[1 * 4 + 0] * v.x + m.elements[1 * 4 + 1] * v.y + m.elements[1 * 4 + 2] * v.z + m.elements[1 * 4 + 3] * v.w,  
        m.elements[2 * 4 + 0] * v.x + m.elements[2 * 4 + 1] * v.y + m.elements[2 * 4 + 2] * v.z + m.elements[2 * 4 + 3] * v.w,  
        m.elements[3 * 4 + 0] * v.x + m.elements[3 * 4 + 1] * v.y + m.elements[3 * 4 + 2] * v.z + m.elements[3 * 4 + 3] * v.w
    );
}

/*================================================================================
// Quaternion
================================================================================*/

typedef struct
{
    union 
    {
        f32 xyzw[4];
        struct 
        {
            f32 x, y, z, w;
        };
    };
} gs_quat;

gs_inline
gs_quat gs_quat_default()
{
    gs_quat q;
    q.x = 0.f;  
    q.y = 0.f;  
    q.z = 0.f;  
    q.w = 1.f;  
    return q;
}

gs_inline
gs_quat gs_quat_ctor(f32 _x, f32 _y, f32 _z, f32 _w)
{
    gs_quat q;
    q.x = _x;
    q.y = _y;
    q.z = _z;
    q.w = _w;
    return q;
}

gs_inline gs_quat 
gs_quat_add(gs_quat q0, gs_quat q1) 
{
    return gs_quat_ctor(q0.x + q1.x, q0.y + q1.y, q0.z + q1.z, q0.w + q1.w);
}

gs_inline gs_quat 
gs_quat_sub(gs_quat q0, gs_quat q1)
{
    return gs_quat_ctor(q0.x - q1.x, q0.y - q1.y, q0.z - q1.z, q0.w - q1.w);
}

gs_inline gs_quat
gs_quat_mul(gs_quat q0, gs_quat q1)
{
    return gs_quat_ctor(
        q0.w * q1.x + q1.w * q0.x + q0.y * q1.z - q1.y * q0.z,
        q0.w * q1.y + q1.w * q0.y + q0.z * q1.x - q1.z * q0.x,
        q0.w * q1.z + q1.w * q0.z + q0.x * q1.y - q1.x * q0.y,
        q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z
    );
}

gs_inline 
gs_quat gs_quat_mul_list(u32 count, ...)
{
    va_list ap;
    gs_quat q = gs_quat_default();
    va_start(ap, count);
    for (u32 i = 0; i < count; ++i)
    {
        q = gs_quat_mul(q, va_arg(ap, gs_quat));
    }
    va_end(ap);
    return q;
}

gs_inline gs_quat 
gs_quat_mul_quat(gs_quat q0, gs_quat q1)
{
    return gs_quat_ctor(
        q0.w * q1.x + q1.w * q0.x + q0.y * q1.z - q1.y * q0.z,
        q0.w * q1.y + q1.w * q0.y + q0.z * q1.x - q1.z * q0.x,
        q0.w * q1.z + q1.w * q0.z + q0.x * q1.y - q1.x * q0.y,
        q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z
    );
}

gs_inline 
gs_quat gs_quat_scale(gs_quat q, f32 s)
{
    return gs_quat_ctor(q.x * s, q.y * s, q.z * s, q.w * s);
}

gs_inline f32 
gs_quat_dot(gs_quat q0, gs_quat q1)
{
    return (f32)(q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w);
}

gs_inline 
gs_quat gs_quat_conjugate(gs_quat q)
{
    return (gs_quat_ctor(-q.x, -q.y, -q.z, q.w));
}

gs_inline f32
gs_quat_len(gs_quat q)
{
    return (f32)sqrt(gs_quat_dot(q, q));
}

gs_inline gs_quat
gs_quat_norm(gs_quat q) 
{
    return gs_quat_scale(q, 1.0f / gs_quat_len(q));
}

gs_inline gs_quat
gs_quat_cross(gs_quat q0, gs_quat q1)
{
    return gs_quat_ctor (                                           
        q0.x * q1.x + q0.x * q1.w + q0.y * q1.z - q0.z * q1.y,  
        q0.w * q1.y + q0.y * q1.w + q0.z * q1.x - q0.x * q1.z,  
        q0.w * q1.z + q0.z * q1.w + q0.x * q1.y - q0.y * q1.x,  
        q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z   
    );
}

// Inverse := Conjugate / Dot;
gs_inline
gs_quat gs_quat_inverse(gs_quat q)
{
    return (gs_quat_scale(gs_quat_conjugate(q), 1.0f / gs_quat_dot(q, q)));
}

gs_inline gs_vec3 gs_quat_rotate(gs_quat q, gs_vec3 v)
{
    // nVidia SDK implementation
    gs_vec3 qvec = gs_vec3_ctor(q.x, q.y, q.z);
    gs_vec3 uv = gs_vec3_cross(qvec, v);
    gs_vec3 uuv = gs_vec3_cross(qvec, uv);
    uv = gs_vec3_scale(uv, 2.f * q.w);
    uuv = gs_vec3_scale(uuv, 2.f);
    return (gs_vec3_add(v, gs_vec3_add(uv, uuv)));
}

gs_inline gs_quat gs_quat_angle_axis(f32 rad, gs_vec3 axis)
{
    // Normalize axis
    gs_vec3 a = gs_vec3_norm(axis);

    // Get scalar
    f32 half_angle = 0.5f * rad;
    f32 s = (float)sin(half_angle);

    return gs_quat_ctor(a.x * s, a.y * s, a.z * s, (float)cos(half_angle));
}

gs_inline
gs_quat gs_quat_slerp(gs_quat a, gs_quat b, f32 t)
{
    f32 c = gs_quat_dot(a, b);
    gs_quat end = b;

    if (c < 0.0f)
    {
        // Reverse all signs
        c *= -1.0f;
        end.x *= -1.0f;
        end.y *= -1.0f;
        end.z *= -1.0f;
        end.w *= -1.0f;
    }

    // Calculate coefficients
    f32 sclp, sclq;
    if ((1.0f - c) > 0.0001f)
    {
        f32 omega = (float)acosf(c);
        f32 s = (float)sinf(omega);
        sclp = (float)sinf((1.0f - t) * omega) / s;
        sclq = (float)sinf(t * omega) / s; 
    }
    else
    {
        sclp = 1.0f - t;
        sclq = t;
    }

    gs_quat q;
    q.x = sclp * a.x + sclq * end.x;
    q.y = sclp * a.y + sclq * end.y;
    q.z = sclp * a.z + sclq * end.z;
    q.w = sclp * a.w + sclq * end.w;

    return q;
}

#define quat_axis_angle(__AXS, __ANG)\
    gs_quat_angle_axis(__ANG, __AXS)

/*
* @brief Convert given quaternion param into equivalent 4x4 rotation matrix
* @note: From http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm 
*/
gs_inline gs_mat4 gs_quat_to_mat4(gs_quat _q)
{
    gs_mat4 mat = gs_mat4_identity();
    gs_quat q = gs_quat_norm(_q);

    f32 xx = q.x * q.x; 
    f32 yy = q.y * q.y; 
    f32 zz = q.z * q.z; 
    f32 xy = q.x * q.y;
    f32 xz = q.x * q.z;
    f32 yz = q.y * q.z;
    f32 wx = q.w * q.x;
    f32 wy = q.w * q.y;
    f32 wz = q.w * q.z;

    mat.elements[0 * 4 + 0] = 1.0f - 2.0f * (yy + zz);
    mat.elements[1 * 4 + 0] = 2.0f * (xy - wz);
    mat.elements[2 * 4 + 0] = 2.0f * (xz + wy);

    mat.elements[0 * 4 + 1] = 2.0f * (xy + wz);
    mat.elements[1 * 4 + 1] = 1.0f - 2.0f * (xx + zz);
    mat.elements[2 * 4 + 1] = 2.0f * (yz - wx);

    mat.elements[0 * 4 + 2] = 2.0f * (xz - wy);
    mat.elements[1 * 4 + 2] = 2.0f * (yz + wx);
    mat.elements[2 * 4 + 2] = 1.0f - 2.0f * (xx + yy);

    return mat;
}

gs_inline 
gs_quat gs_quat_from_euler(f32 yaw_deg, f32 pitch_deg, f32 roll_deg)
{
    f32 yaw = gs_deg2rad(yaw_deg);
    f32 pitch = gs_deg2rad(pitch_deg);
    f32 roll = gs_deg2rad(roll_deg);

    gs_quat q;
    f32 cy = (float)cos(yaw * 0.5f);
    f32 sy = (float)sin(yaw * 0.5f);
    f32 cr = (float)cos(roll * 0.5f);
    f32 sr = (float)sin(roll * 0.5f);
    f32 cp = (float)cos(pitch * 0.5f);
    f32 sp = (float)sin(pitch * 0.5f);

    q.x = cy * sr * cp - sy * cr * sp;
    q.y = cy * cr * sp + sy * sr * cp;
    q.z = sy * cr * cp - cy * sr * sp;
    q.w = cy * cr * cp + sy * sr * sp;

    return q;
}

/*================================================================================
// Transform (Non-Uniform Scalar VQS)
================================================================================*/

/*
    - This follows a traditional 'VQS' structure for complex object transformations, 
        however it differs from the standard in that it allows for non-uniform 
        scaling in the form of a vec3.
*/
// Source: https://www.eurosis.org/cms/files/conf/gameon-asia/gameon-asia2007/R-SESSION/G1.pdf

typedef struct 
{
    gs_vec3     position;
    gs_quat     rotation;
    gs_vec3     scale;      
} gs_vqs;

gs_inline gs_vqs gs_vqs_ctor(gs_vec3 tns, gs_quat rot, gs_vec3 scl)
{
    gs_vqs t;   
    t.position = tns;
    t.rotation = rot;
    t.scale = scl;
    return t;
}

gs_inline 
gs_vqs gs_vqs_default()
{
    gs_vqs t = gs_vqs_ctor
    (
        gs_vec3_ctor(0.0f, 0.0f, 0.0f),
        gs_quat_ctor(0.0f, 0.0f, 0.0f, 1.0f),
        gs_vec3_ctor(1.0f, 1.0f, 1.0f)
    );
    return t;
}

// AbsScale = ParentScale * LocalScale
// AbsRot   = LocalRot * ParentRot
// AbsTrans = ParentPos + [ParentRot * (ParentScale * LocalPos)]
gs_inline gs_vqs gs_vqs_absolute_transform(const gs_vqs* local, const gs_vqs* parent)
{
    // Normalized rotations
    gs_quat p_rot_norm = gs_quat_norm(parent->rotation);
    gs_quat l_rot_norm = gs_quat_norm(local->rotation);

    // Scale
    gs_vec3 scl = gs_vec3_mul(local->scale, parent->scale);
    // Rotation
    gs_quat rot = gs_quat_norm(gs_quat_mul(p_rot_norm, l_rot_norm));
    // position
    gs_vec3 tns = gs_vec3_add(parent->position, gs_quat_rotate(p_rot_norm, gs_vec3_mul(parent->scale, local->position)));

    return gs_vqs_ctor(tns, rot, scl);
}

// RelScale = AbsScale / ParentScale 
// RelRot   = Inverse(ParentRot) * AbsRot
// RelTrans = [Inverse(ParentRot) * (AbsPos - ParentPosition)] / ParentScale;
gs_inline gs_vqs gs_vqs_relative_transform(const gs_vqs* absolute, const gs_vqs* parent)
{
    // Get inverse rotation normalized
    gs_quat p_rot_inv = gs_quat_norm(gs_quat_inverse(parent->rotation));
    // Normalized abs rotation
    gs_quat a_rot_norm = gs_quat_norm(absolute->rotation);

    // Scale
    gs_vec3 scl = gs_vec3_div(absolute->scale, parent->scale);
    // Rotation
    gs_quat rot = gs_quat_norm(gs_quat_mul(p_rot_inv, a_rot_norm));
    // position
    gs_vec3 tns = gs_vec3_div(gs_quat_rotate(p_rot_inv, gs_vec3_sub(absolute->position, parent->position)), parent->scale);

    return gs_vqs_ctor(tns, rot, scl);
}

gs_inline gs_mat4 gs_vqs_to_mat4(const gs_vqs* transform)
{
    gs_mat4 mat = gs_mat4_identity();
    gs_mat4 trans = gs_mat4_translatev(transform->position);
    gs_mat4 rot = gs_quat_to_mat4(transform->rotation);
    gs_mat4 scl = gs_mat4_scalev(transform->scale);
    mat = gs_mat4_mul(mat, trans);
    mat = gs_mat4_mul(mat, rot);
    mat = gs_mat4_mul(mat, scl);
    return mat;
}

/*================================================================================
// Ray
================================================================================*/

typedef struct 
{
    gs_vec3 point;
    gs_vec3 direction;  
} gs_ray;

gs_inline gs_ray gs_ray_ctor(gs_vec3 pt, gs_vec3 dir)
{
    gs_ray r;
    r.point = pt;
    r.direction = dir;
    return r;
}

/*================================================================================
// Plane
================================================================================*/

typedef struct gs_plane_t
{
    union
    {
        gs_vec3 n;
        struct 
        {
            f32 a;
            f32 b;
            f32 c;
        };
    };

    f32 d;
} gs_plane_t;

/*================================================================================
// Camera
================================================================================*/

typedef enum gs_projection_type
{
    GS_PROJECTION_TYPE_ORTHOGRAPHIC,
    GS_PROJECTION_TYPE_PERSPECTIVE
} gs_projection_type;

typedef struct gs_camera_t
{
    gs_vqs transform;
    float fov; 
    float aspect_ratio; 
    float near_plane; 
    float far_plane;
    float ortho_scale;
    gs_projection_type proj_type;
} gs_camera_t;

GS_API_DECL gs_camera_t gs_camera_default();
GS_API_DECL gs_camera_t gs_camera_perspective();
GS_API_DECL gs_mat4 gs_camera_get_view(gs_camera_t* cam);
GS_API_DECL gs_mat4 gs_camera_get_projection(gs_camera_t* cam, int32_t view_width, int32_t view_height);
GS_API_DECL gs_mat4 gs_camera_get_view_projection(gs_camera_t* cam, int32_t view_width, int32_t view_height);
GS_API_DECL gs_vec3 gs_camera_forward(gs_camera_t* cam);
GS_API_DECL gs_vec3 gs_camera_backward(gs_camera_t* cam);
GS_API_DECL gs_vec3 gs_camera_up(gs_camera_t* cam);
GS_API_DECL gs_vec3 gs_camera_down(gs_camera_t* cam);
GS_API_DECL gs_vec3 gs_camera_right(gs_camera_t* cam);
GS_API_DECL gs_vec3 gs_camera_left(gs_camera_t* cam);
GS_API_DECL gs_vec3 gs_camera_screen_to_world(gs_camera_t* cam, gs_vec3 coords, int32_t view_width, int32_t view_height);
GS_API_DECL void gs_camera_offset_orientation(gs_camera_t* cam, float yaw, float picth);

/*================================================================================
// Utils
================================================================================*/

// AABBs
/*
    min is top left of rect,
    max is bottom right
*/
typedef struct gs_aabb_t
{
    gs_vec2 min;
    gs_vec2 max;
} gs_aabb_t;

// Collision Resolution: Minimum Translation Vector 
gs_force_inline
gs_vec2 gs_aabb_aabb_mtv(gs_aabb_t* a0, gs_aabb_t* a1)
{
    gs_vec2 diff = gs_v2(a0->min.x - a1->min.x, a0->min.y - a1->min.y);    

    f32 l, r, b, t;
    gs_vec2 mtv = gs_v2(0.f, 0.f);

    l = a1->min.x - a0->max.x;
    r = a1->max.x - a0->min.x;
    b = a1->min.y - a0->max.y;
    t = a1->max.y - a0->min.y;

    mtv.x = fabsf(l) > r ? r : l;
    mtv.y = fabsf(b) > t ? t : b;

    if ( fabsf(mtv.x) <= fabsf(mtv.y)) {
        mtv.y = 0.f;
    } else {
        mtv.x = 0.f;
    }
    
    return mtv;
}

// 2D AABB collision detection (rect. vs. rect.)
gs_force_inline
b32 gs_aabb_vs_aabb(gs_aabb_t* a, gs_aabb_t* b)
{
    if (a->max.x > b->min.x && 
         a->max.y > b->min.y && 
         a->min.x < b->max.x && 
         a->min.y < b->max.y)
    {
        return true;
    }

    return false;
}

gs_force_inline
gs_vec4 gs_aabb_window_coords(gs_aabb_t* aabb, gs_camera_t* camera, gs_vec2 window_size)
{
    // AABB of the player
    gs_vec4 bounds = gs_default_val();
    gs_vec4 tl = gs_v4(aabb->min.x, aabb->min.y, 0.f, 1.f);
    gs_vec4 br = gs_v4(aabb->max.x, aabb->max.y, 0.f, 1.f);

    gs_mat4 view_mtx = gs_camera_get_view(camera);
    gs_mat4 proj_mtx = gs_camera_get_projection(camera, (int32_t)window_size.x, (int32_t)window_size.y);
    gs_mat4 vp = gs_mat4_mul(proj_mtx, view_mtx);

    // Transform verts
    tl = gs_mat4_mul_vec4(vp, tl);            
    br = gs_mat4_mul_vec4(vp, br);

    // Perspective divide   
    tl = gs_vec4_scale(tl, 1.f / tl.w);
    br = gs_vec4_scale(br, 1.f / br.w);

    // NDC [0.f, 1.f] and NDC
    tl.x = (tl.x * 0.5f + 0.5f);
    tl.y = (tl.y * 0.5f + 0.5f);
    br.x = (br.x * 0.5f + 0.5f);
    br.y = (br.y * 0.5f + 0.5f);

    // Window Space
    tl.x = tl.x * window_size.x;
    tl.y = gs_map_range(1.f, 0.f, 0.f, 1.f, tl.y) * window_size.y;
    br.x = br.x * window_size.x;
    br.y = gs_map_range(1.f, 0.f, 0.f, 1.f, br.y) * window_size.y;

    bounds = gs_v4(tl.x, tl.y, br.x, br.y);

    return bounds;
}

/*========================
// GS_PLATFORM
========================*/

/* Platform Apple */
#if (defined __APPLE__ || defined _APPLE)   

    #define GS_PLATFORM_APPLE

/* Platform Windows */
#elif (defined _WIN32 || defined _WIN64)

    // Necessary windows defines before including windows.h, because it's retarded.
    #define OEMRESOURCE

    #define GS_PLATFORM_WIN
    #include <windows.h>

    #define WIN32_LEAN_AND_MEAN

/* Platform Linux */
#elif (defined linux || defined _linux || defined __linux__)

    #define GS_PLATFORM_LINUX

/* Else - Platform Undefined and Unsupported */

#endif

/*============================================================
// Platform Time
============================================================*/

typedef struct gs_platform_time_t
{
    f64 max_fps;
    f64 current;
    f64 previous;
    f64 update;
    f64 render;
    f64 delta;
    f64 frame;
} gs_platform_time_t;

/*============================================================
// Platform UUID
============================================================*/

#define GS_UUID_STR_SIZE_CONSTANT       32

// 33 characters, all set to 0
#define gs_uuid_temp_str_buffer()\
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} 

typedef struct gs_uuid_t
{
    const char* id;
    uint8_t bytes[16];
} gs_uuid_t;

/*============================================================
// Platform Window
============================================================*/

#define GS_WINDOW_FLAGS_NO_RESIZE   0x01
#define GS_WINDOW_FLAGS_FULLSCREEN  0x02

// Should have an internal resource cache of window handles (controlled by the platform api)

typedef enum gs_platform_cursor
{
    GS_PLATFORM_CURSOR_ARROW,
    GS_PLATFORM_CURSOR_IBEAM,
    GS_PLATFORM_CURSOR_SIZE_NW_SE,
    GS_PLATFORM_CURSOR_SIZE_NE_SW,
    GS_PLATFORM_CURSOR_SIZE_NS,
    GS_PLATFORM_CURSOR_SIZE_WE,
    GS_PLATFORM_CURSOR_SIZE_ALL,
    GS_PLATFORM_CURSOR_HAND,
    GS_PLATFORM_CURSOR_NO,
    GS_PLATFORM_CURSOR_COUNT
} gs_platform_cursor;

typedef enum gs_platform_keycode
{
    GS_KEYCODE_A,
    GS_KEYCODE_B,
    GS_KEYCODE_C,
    GS_KEYCODE_D,
    GS_KEYCODE_E,
    GS_KEYCODE_F,
    GS_KEYCODE_G,
    GS_KEYCODE_H,
    GS_KEYCODE_I,
    GS_KEYCODE_J,
    GS_KEYCODE_K,
    GS_KEYCODE_L,
    GS_KEYCODE_M,
    GS_KEYCODE_N,
    GS_KEYCODE_O,
    GS_KEYCODE_P,
    GS_KEYCODE_Q,
    GS_KEYCODE_R,
    GS_KEYCODE_S,
    GS_KEYCODE_T,
    GS_KEYCODE_U,
    GS_KEYCODE_V,
    GS_KEYCODE_W,
    GS_KEYCODE_X,
    GS_KEYCODE_Y,
    GS_KEYCODE_Z,
    GS_KEYCODE_LSHIFT,
    GS_KEYCODE_RSHIFT,
    GS_KEYCODE_LALT,
    GS_KEYCODE_RALT,
    GS_KEYCODE_LCTRL,
    GS_KEYCODE_RCTRL,
    GS_KEYCODE_BSPACE,
    GS_KEYCODE_BSLASH,
    GS_KEYCODE_QMARK,
    GS_KEYCODE_TILDE,
    GS_KEYCODE_COMMA,
    GS_KEYCODE_PERIOD,
    GS_KEYCODE_ESC, 
    GS_KEYCODE_SPACE,
    GS_KEYCODE_LEFT,
    GS_KEYCODE_UP,
    GS_KEYCODE_RIGHT,
    GS_KEYCODE_DOWN,
    GS_KEYCODE_ZERO,
    GS_KEYCODE_ONE,
    GS_KEYCODE_TWO,
    GS_KEYCODE_THREE,
    GS_KEYCODE_FOUR,
    GS_KEYCODE_FIVE,
    GS_KEYCODE_SIX,
    GS_KEYCODE_SEVEN,
    GS_KEYCODE_EIGHT,
    GS_KEYCODE_NINE,
    GS_KEYCODE_NPZERO,
    GS_KEYCODE_NPONE,
    GS_KEYCODE_NPTWO,
    GS_KEYCODE_NPTHREE,
    GS_KEYCODE_NPFOUR,
    GS_KEYCODE_NPFIVE,
    GS_KEYCODE_NPSIX,
    GS_KEYCODE_NPSEVEN,
    GS_KEYCODE_NPEIGHT,
    GS_KEYCODE_NPNINE,
    GS_KEYCODE_CAPS,
    GS_KEYCODE_DELETE,
    GS_KEYCODE_END,
    GS_KEYCODE_F1,
    GS_KEYCODE_F2,
    GS_KEYCODE_F3,
    GS_KEYCODE_F4,
    GS_KEYCODE_F5,
    GS_KEYCODE_F6,
    GS_KEYCODE_F7,
    GS_KEYCODE_F8,
    GS_KEYCODE_F9,
    GS_KEYCODE_F10,
    GS_KEYCODE_F11,
    GS_KEYCODE_F12,
    GS_KEYCODE_HOME,
    GS_KEYCODE_PLUS,
    GS_KEYCODE_MINUS,
    GS_KEYCODE_LBRACKET,
    GS_KEYCODE_RBRACKET,
    GS_KEYCODE_SEMI_COLON,
    GS_KEYCODE_ENTER,
    GS_KEYCODE_INSERT,
    GS_KEYCODE_PGUP,
    GS_KEYCODE_PGDOWN,
    GS_KEYCODE_NUMLOCK,
    GS_KEYCODE_TAB,
    GS_KEYCODE_NPMULT,
    GS_KEYCODE_NPDIV,
    GS_KEYCODE_NPPLUS,
    GS_KEYCODE_NPMINUS,
    GS_KEYCODE_NPENTER,
    GS_KEYCODE_NPDEL,
    GS_KEYCODE_MUTE,
    GS_KEYCODE_VOLUP,
    GS_KEYCODE_VOLDOWN,
    GS_KEYCODE_PAUSE,
    GS_KEYCODE_PRINT,
    GS_KEYCODE_COUNT
} gs_platform_keycode;

typedef enum gs_platform_mouse_button_code
{
    GS_MOUSE_LBUTTON,
    GS_MOUSE_RBUTTON,
    GS_MOUSE_MBUTTON,
    GS_MOUSE_BUTTON_CODE_COUNT
} gs_platform_mouse_button_code;

typedef struct gs_platform_mouse_t
{
    b32 button_map[GS_MOUSE_BUTTON_CODE_COUNT];
    b32 prev_button_map[GS_MOUSE_BUTTON_CODE_COUNT];
    gs_vec2 position;
    gs_vec2 prev_position;
    gs_vec2 wheel;
    b32 moved_this_frame;
} gs_platform_mouse_t;

typedef struct gs_platform_input_t
{
    b32 key_map[GS_KEYCODE_COUNT];
    b32 prev_key_map[GS_KEYCODE_COUNT];
    gs_platform_mouse_t mouse;
} gs_platform_input_t;

// Enumeration of all platform type
typedef enum gs_platform_type
{
    GS_PLATFORM_TYPE_UNKNOWN = 0,
    GS_PLATFORM_TYPE_WINDOWS,
    GS_PLATFORM_TYPE_LINUX,
    GS_PLATFORM_TYPE_MAC
} gs_platform_type;

typedef enum gs_platform_video_driver_type
{
    GS_PLATFORM_VIDEO_DRIVER_TYPE_NONE = 0,
    GS_PLATFORM_VIDEO_DRIVER_TYPE_OPENGL,
    GS_PLATFORM_VIDEO_DRIVER_TYPE_OPENGLES,
    GS_PLATFORM_VIDEO_DRIVER_TYPE_DIRECTX,
    GS_PLATFORM_VIDEO_DRIVER_TYPE_VULKAN,
    GS_PLATFORM_VIDEO_DRIVER_TYPE_METAL,
    GS_PLATFORM_VIDEO_DRIVER_TYPE_SOFTWARE
} gs_platform_video_driver_type;

typedef enum gs_opengl_compatibility_flags
{
   GS_OPENGL_COMPATIBILITY_FLAGS_LEGACY        = 0,
   GS_OPENGL_COMPATIBILITY_FLAGS_CORE          = 1 << 1,
   GS_OPENGL_COMPATIBILITY_FLAGS_COMPATIBILITY = 1 << 2,
   GS_OPENGL_COMPATIBILITY_FLAGS_FORWARD       = 1 << 3,
   GS_OPENGL_COMPATIBILITY_FLAGS_ES            = 1 << 4,
} gs_opengl_compatibility_flags;

// A structure that contains OpenGL video settings
typedef struct gs_opengl_video_settings_t 
{
    gs_opengl_compatibility_flags compability_flags;
    uint32_t                      major_version;
    uint32_t                      minor_version;
    uint8_t                       multi_sampling_count;
    void*                         ctx;
} gs_opengl_video_settings_t;
    
typedef union gs_graphics_api_settings_t
{
    gs_opengl_video_settings_t opengl;
    int32_t                    dummy;   
} gs_graphics_api_settings_t;

typedef struct gs_platform_video_settings_t
{
    gs_graphics_api_settings_t      graphics;
    gs_platform_video_driver_type   driver;
    b32                             vsync_enabled;
} gs_platform_video_settings_t;

typedef struct gs_platform_settings_t
{
    gs_platform_video_settings_t video;
} gs_platform_settings_t;

typedef enum gs_platform_event_type 
{
    GS_PLATFORM_EVENT_MOUSE,
    GS_PLATFORM_EVENT_KEY,
    GS_PLATFORM_EVENT_WINDOW
} gs_platform_event_type;

typedef enum gs_platform_key_action_type
{
    GS_PLATFORM_KEY_PRESSED,
    GS_PLATFORM_KEY_DOWN,
    GS_PLATFORM_KEY_RELEASED
} gs_platform_key_action_type;

typedef struct gs_platform_key_event_t
{
    int32_t codepoint;
    gs_platform_keycode key;
    gs_platform_key_action_type action;
} gs_platform_key_event_t;

typedef enum gs_platform_mousebutton_action_type
{
    GS_PLATFORM_MBUTTON_PRESSED,
    GS_PLATFORM_MBUTTON_DOWN,
    GS_PLATFORM_MBUTTON_RELEASED
} gs_platform_mousebutton_action_type;

typedef struct gs_platform_mouse_event_t 
{
    int32_t codepoint;
    gs_platform_mouse_button_code button;
    gs_platform_mousebutton_action_type action;
} gs_platform_mouse_event_t;

typedef enum gs_platform_window_action_type
{
    GS_PLATFORM_WINDOW_MOUSE_ENTER,
    GS_PLATFORM_WINDOW_MOUSE_LEAVE,
    GS_PLATFORM_WINDOW_RESIZE
} gs_platform_window_action_type;

typedef struct gs_platform_window_event_t
{
    uint32_t hndl;
    gs_platform_window_action_type action;
} gs_platform_window_event_t;

// Platform events
typedef struct gs_platform_event_t
{
    gs_platform_event_type type;
    union {
        gs_platform_key_event_t key;
        gs_platform_mouse_event_t mouse;
        gs_platform_window_event_t window;
    };
    uint32_t idx;
} gs_platform_event_t;

// Necessary function pointer typedefs
typedef void (* gs_dropped_files_callback_t)(void*, int32_t count, const char** file_paths);
typedef void (* gs_window_close_callback_t)(void*);

/*===============================================================================================
// Platform Interface
===============================================================================================*/

typedef struct gs_platform_i
{
    // Settings for platform, including video
    gs_platform_settings_t settings;

    // Time
    gs_platform_time_t time;

    // Input
    gs_platform_input_t input;

    // Window data and handles
    gs_slot_array(void*) windows;

    // Events that user can poll
    gs_dyn_array(gs_platform_event_t) events;

    // Cursors
    void* cursors[GS_PLATFORM_CURSOR_COUNT];

    // Specific user data (for custom implementations)
    void* user_data;
} gs_platform_i;

/*===============================================================================================
// Platform API
===============================================================================================*/

// Platform Init / Shutdown
GS_API_DECL gs_platform_i*  gs_platform_create();
GS_API_DECL void            gs_platform_destroy(gs_platform_i* platform);
GS_API_DECL gs_result       gs_platform_init(gs_platform_i* platform);      // Initialize global platform layer
GS_API_DECL gs_result       gs_platform_update(gs_platform_i* platform);    // Update platform layer
GS_API_DECL gs_result       gs_platform_shutdown(gs_platform_i* platform);  // Shutdown gloabl platform layer

// Platform Util
GS_API_DECL void   gs_platform_sleep(float ms); // Sleeps platform for time in ms
GS_API_DECL double gs_platform_elapsed_time();  // Returns time in ms since initialization of platform
GS_API_DECL float  gs_platform_delta_time();

// Platform Video
GS_API_DECL void gs_platform_enable_vsync(int32_t enabled);

// Platform UUID
GS_API_DECL gs_uuid_t gs_platform_generate_uuid();
GS_API_DECL void      gs_platform_uuid_to_string(char* temp_buffer, const gs_uuid_t* uuid); // Expects a temp buffer with at least 32 bytes
GS_API_DECL uint32_t  gs_platform_hash_uuid(const gs_uuid_t* uuid);

// Platform Input
GS_API_DECL gs_result gs_platform_process_input(gs_platform_input_t* input);
GS_API_DECL void      gs_platform_update_input(gs_platform_input_t* input);
GS_API_DECL void      gs_platform_press_key(gs_platform_keycode code);
GS_API_DECL void      gs_platform_release_key(gs_platform_keycode code);
GS_API_DECL bool      gs_platform_was_key_down(gs_platform_keycode code);
GS_API_DECL bool      gs_platform_key_pressed(gs_platform_keycode code);
GS_API_DECL bool      gs_platform_key_down(gs_platform_keycode code);
GS_API_DECL bool      gs_platform_key_released(gs_platform_keycode code);
GS_API_DECL uint32_t  gs_platform_key_to_codepoint(gs_platform_keycode code);
GS_API_DECL void      gs_platform_press_mouse_button(gs_platform_mouse_button_code code);
GS_API_DECL void      gs_platform_release_mouse_button(gs_platform_mouse_button_code code);
GS_API_DECL bool      gs_platform_was_mouse_down(gs_platform_mouse_button_code code);
GS_API_DECL bool      gs_platform_mouse_pressed(gs_platform_mouse_button_code code);
GS_API_DECL bool      gs_platform_mouse_down(gs_platform_mouse_button_code code);
GS_API_DECL bool      gs_platform_mouse_released(gs_platform_mouse_button_code code);
GS_API_DECL void      gs_platform_set_mouse_position(uint32_t handle, float x, float y);
GS_API_DECL gs_vec2   gs_platform_mouse_deltav();
GS_API_DECL void      gs_platform_mouse_delta(float* x, float* y);
GS_API_DECL gs_vec2   gs_platform_mouse_positionv();
GS_API_DECL void      gs_platform_mouse_position(int32_t* x, int32_t* y);
GS_API_DECL void      gs_platform_mouse_wheel(float* x, float* y);
GS_API_DECL bool      gs_platform_mouse_moved();

// Platform Events
GS_API_DECL bool      gs_platform_poll_event(gs_platform_event_t* evt);

// Platform Window
GS_API_DECL uint32_t gs_platform_create_window(const char* title, uint32_t width, uint32_t height);
GS_API_DECL void*    gs_platform_create_window_internal(const char* title, uint32_t width, uint32_t height);
GS_API_DECL void     gs_platform_window_swap_buffer(uint32_t handle);
GS_API_DECL gs_vec2  gs_platform_window_sizev(uint32_t handle);
GS_API_DECL void     gs_platform_window_size(uint32_t handle, uint32_t* width, uint32_t* height);
GS_API_DECL uint32_t gs_platform_window_width(uint32_t handle);
GS_API_DECL uint32_t gs_platform_window_height(uint32_t handle);
GS_API_DECL void     gs_platform_set_window_size(uint32_t handle, uint32_t width, uint32_t height);
GS_API_DECL void     gs_platform_set_window_sizev(uint32_t handle, gs_vec2 v);
GS_API_DECL void     gs_platform_set_cursor(uint32_t handle, gs_platform_cursor cursor);
GS_API_DECL uint32_t gs_platform_main_window();
GS_API_DECL void     gs_platform_set_dropped_files_callback(uint32_t handle, gs_dropped_files_callback_t cb);
GS_API_DECL void     gs_platform_set_window_close_callback(uint32_t handle, gs_window_close_callback_t cb);
GS_API_DECL void*    gs_platform_raw_window_handle(uint32_t handle);
GS_API_DECL gs_vec2  gs_platform_framebuffer_sizev(uint32_t handle);
GS_API_DECL void     gs_platform_framebuffer_size(uint32_t handle, uint32_t* w, uint32_t* h);
GS_API_DECL uint32_t gs_platform_framebuffer_width(uint32_t handle);
GS_API_DECL uint32_t gs_platform_framebuffer_height(uint32_t handle);

// Platform File IO
GS_API_DECL char*      gs_platform_read_file_contents(const char* file_path, const char* mode, int32_t* sz);
GS_API_DECL gs_result  gs_platform_write_file_contents(const char* file_path, const char* mode, void* data, size_t data_size);
GS_API_DECL bool       gs_platform_file_exists(const char* file_path);
GS_API_DECL int32_t    gs_platform_file_size_in_bytes(const char* file_path);
GS_API_DECL void       gs_platform_file_extension(char* buffer, size_t buffer_sz, const char* file_path);

/*=============================
// GS_AUDIO
=============================*/

typedef enum gs_audio_file_type
{
    GS_OGG = 0x00,
    GS_WAV,
    GS_MP3  
} gs_audio_file_type;

/*==================
// Audio Source
==================*/

typedef struct gs_audio_source_t
{
    int32_t channels;
    int32_t sample_rate;
    void* samples;
    int32_t sample_count;
} gs_audio_source_t;

gs_handle_decl(gs_audio_source_t);

typedef struct gs_audio_instance_decl_t
{
    gs_handle(gs_audio_source_t) src;
    float volume;
    bool32_t loop;
    bool32_t persistent;
    bool32_t playing;
    double sample_position;
    void* user_data;
} gs_audio_instance_decl_t;

typedef gs_audio_instance_decl_t gs_audio_instance_t;
gs_handle_decl(gs_audio_instance_t);

/*=============================
// Audio Interface
=============================*/

typedef struct gs_audio_i
{
    /* Audio source data cache */
    gs_slot_array(gs_audio_source_t) sources;

    /* Audio instance data cache */
    gs_slot_array(gs_audio_instance_t) instances;

    /* Max global volume setting */
    float max_audio_volume;

    /* Min global volume setting */
    float min_audio_volume;

    /* Samples to actually write to hardware */
    void* sample_out;

    /* Amount of samples to write */
    uint32_t sample_count_to_output;    

    /* Samples per second for hardware */
    uint32_t samples_per_second;

    /* User data for custom impl */
    void* user_data;
} gs_audio_i;

/*=============================
// Audio API
=============================*/

/* Audio Create, Destroy, Init, Shutdown, Submit */
GS_API_DECL gs_audio_i* gs_audio_create();
GS_API_DECL void        gs_audio_destroy(gs_audio_i* audio);
GS_API_DECL gs_result   gs_audio_init(gs_audio_i* audio);
GS_API_DECL gs_result   gs_audio_shutdown(gs_audio_i* audio);

/* Audio create source */
GS_API_DECL gs_handle(gs_audio_source_t) gs_audio_load_from_file(const char* file_path);

/* Audio create instance */
GS_API_DECL gs_handle(gs_audio_instance_t) gs_audio_instance_create(gs_audio_instance_decl_t* decl);

/* Audio play instance */
GS_API_DECL void     gs_audio_play_source(gs_handle(gs_audio_source_t) src, float volume);
GS_API_DECL void     gs_audio_play(gs_handle(gs_audio_instance_t) inst);
GS_API_DECL void     gs_audio_pause(gs_handle(gs_audio_instance_t) inst);
GS_API_DECL void     gs_audio_stop(gs_handle(gs_audio_instance_t) inst);
GS_API_DECL void     gs_audio_restart(gs_handle(gs_audio_instance_t) inst);
GS_API_DECL bool32_t gs_audio_is_playing(gs_handle(gs_audio_instance_t) inst);

/* Audio instance data */
GS_API_DECL void                     gs_audio_set_instance_data(gs_handle(gs_audio_instance_t) inst, gs_audio_instance_decl_t decl);
GS_API_DECL gs_audio_instance_decl_t gs_audio_get_instance_data(gs_handle(gs_audio_instance_t) inst);
GS_API_DECL float                    gs_audio_get_volume(gs_handle(gs_audio_instance_t) inst);
GS_API_DECL void                     gs_audio_set_volume(gs_handle(gs_audio_instance_t) inst, float volume);

/* Audio source data */
GS_API_DECL gs_audio_source_t* gs_audio_get_source_data(gs_handle(gs_audio_source_t) src);
GS_API_DECL void               gs_audio_get_runtime(gs_handle(gs_audio_source_t) src, int32_t* minutes, int32_t* seconds);
GS_API_DECL void               gs_audio_convert_to_runtime(int32_t sample_count, int32_t sample_rate, int32_t num_channels, int32_t position, int32_t* minutes_out, int32_t* seconds_out);
GS_API_DECL int32_t            gs_audio_get_sample_count(gs_handle(gs_audio_source_t) src);
GS_API_DECL int32_t            gs_audio_get_sample_rate(gs_handle(gs_audio_source_t) src);
GS_API_DECL int32_t            gs_audio_get_num_channels(gs_handle(gs_audio_source_t) src);

/* Resource Loading */
GS_API_DECL bool32_t gs_audio_load_ogg_data_from_file(const char* file_path, int32_t* sample_count, int32_t* channels, int32_t* sample_rate, void** samples);
GS_API_DECL bool32_t gs_audio_load_wav_data_from_file(const char* file_path, int32_t* sample_count, int32_t* channels, int32_t* sample_rate, void** samples);
GS_API_DECL bool32_t gs_audio_load_mp3_data_from_file(const char* file_path, int32_t* sample_count, int32_t* channels, int32_t* sample_rate, void** samples);

/* Short name definitions for convenience */
#ifndef GS_NO_SHORT_NAME
    /* Resources */
    #define gsa_src      gs_handle(gs_audio_source_t)
    #define gsa_inst     gs_handle(gs_audio_instance_t)
    #define gsa_instdecl gs_audio_instance_decl_t
    /* Create */
    #define gsa_create   gs_audio_create
    #define gsa_destroy  gs_audio_destroy
    #define gsa_init     gs_audio_init
    #define gsa_shutdown gs_audio_shutdown
    #define gsa_submit   gs_audio_submit
    /* Source */
    #define gsa_load     gs_audio_load_from_file            
    /* Instance */
    #define gsa_make_inst gs_audio_instance_create

    /* Audio play instance */
    #define gsa_isplaying gs_audio_is_playing
    #define gsa_play      gs_audio_play
    #define gsa_play_src  gs_audio_play_source
    #define gsa_pause     gs_audio_pause
    #define gsa_restart   gs_audio_restart
#endif

/*=============================
// GS_GRAPHICS
=============================*/

// Graphics Pipeline

// Main graphics resources:
// Shader description: vertex, fragment, compute, geometry, tesselation
// Texture Description: texture, depth, render target
// Buffer Description: vertex, index, uniform, frame, pixel
// Pipeline Description: vert-layout, shader, bindables, render states
// Pass Description: render pass, action on render targets (clear, set viewport, etc.)

/* Useful macro for forcing enum decl to be uint32_t type with default = 0x00 for quick init */
#define gs_enum_decl(NAME, ...)\
    typedef enum NAME {\
        _gs_##NAME##_default = 0x0,\
        __VA_ARGS__,\
        _gs_##NAME##_count,\
        _gs_##NAME##_force_u32 = 0x7fffffff\
    } NAME;

#define gs_enum_count(NAME)\
    _gs_##NAME##_count

/* Shader Stage Type */
gs_enum_decl(gs_graphics_shader_stage_type,
    GS_GRAPHICS_SHADER_STAGE_VERTEX,
    GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
    GS_GRAPHICS_SHADER_STAGE_COMPUTE
);

/* Blend Equation Type */
gs_enum_decl(gs_graphics_blend_equation_type,
    GS_GRAPHICS_BLEND_EQUATION_ADD,
    GS_GRAPHICS_BLEND_EQUATION_SUBTRACT,
    GS_GRAPHICS_BLEND_EQUATION_REVERSE_SUBTRACT,
    GS_GRAPHICS_BLEND_EQUATION_MIN,
    GS_GRAPHICS_BLEND_EQUATION_MAX
);

/* Winding Order Type */
gs_enum_decl(gs_graphics_winding_order_type,
    GS_GRAPHICS_WINDING_ORDER_CW,
    GS_GRAPHICS_WINDING_ORDER_CCW
);

/* Face Culling Type */
gs_enum_decl(gs_graphics_face_culling_type,
    GS_GRAPHICS_FACE_CULLING_FRONT,
    GS_GRAPHICS_FACE_CULLING_BACK,
    GS_GRAPHICS_FACE_CULLING_FRONT_AND_BACK
);

/* Blend Mode Type */
gs_enum_decl(gs_graphics_blend_mode_type,
    GS_GRAPHICS_BLEND_MODE_ZERO,
    GS_GRAPHICS_BLEND_MODE_ONE,
    GS_GRAPHICS_BLEND_MODE_SRC_COLOR,
    GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_COLOR,
    GS_GRAPHICS_BLEND_MODE_DST_COLOR,
    GS_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_COLOR,
    GS_GRAPHICS_BLEND_MODE_SRC_ALPHA,
    GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA,
    GS_GRAPHICS_BLEND_MODE_DST_ALPHA,
    GS_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_ALPHA,
    GS_GRAPHICS_BLEND_MODE_CONSTANT_COLOR,
    GS_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_COLOR,
    GS_GRAPHICS_BLEND_MODE_CONSTANT_ALPHA,
    GS_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA
);

/* Shader Language Type */
gs_enum_decl(gs_graphics_shader_language_type,
    GS_GRAPHICS_SHADER_LANGUAGE_GLSL
);

/* Uniform Type */
gs_enum_decl(gs_graphics_uniform_type,
    GS_GRAPHICS_UNIFORM_FLOAT,
    GS_GRAPHICS_UNIFORM_INT,
    GS_GRAPHICS_UNIFORM_VEC2,
    GS_GRAPHICS_UNIFORM_VEC3,
    GS_GRAPHICS_UNIFORM_VEC4,
    GS_GRAPHICS_UNIFORM_MAT4
);

/* Uniform Block Usage Type */
gs_enum_decl(gs_graphics_uniform_block_usage_type, 
    GS_GRAPHICS_UNIFORM_BLOCK_USAGE_STATIC,             // Default of 0x00 is static
    GS_GRAPHICS_UNIFORM_BLOCK_USAGE_PUSH_CONSTANT
);

/* Sampler Type */
gs_enum_decl(gs_graphics_sampler_type,
    GS_GRAPHICS_SAMPLER_2D
);

/* Primitive Type */
gs_enum_decl(gs_graphics_primitive_type,
    GS_GRAPHICS_PRIMITIVE_LINES,
    GS_GRAPHICS_PRIMITIVE_TRIANGLES,
    GS_GRAPHICS_PRIMITIVE_QUADS
);

/* Vertex Atribute Type */
gs_enum_decl(gs_graphics_vertex_attribute_type,
    GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4,
    GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3,
    GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2,
    GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT,
    GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT4,
    GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT3,
    GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT2,
    GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT,
    GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4,
    GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE3,
    GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE2,
    GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE
);

/* Buffer Type */
gs_enum_decl(gs_graphics_buffer_type,
    GS_GRAPHICS_BUFFER_VERTEX,
    GS_GRAPHICS_BUFFER_INDEX,
    GS_GRAPHICS_BUFFER_FRAME,
    GS_GRAPHICS_BUFFER_UNIFORM,
    GS_GRAPHICS_BUFFER_SAMPLER
);

/* Buffer Usage Type */
gs_enum_decl(gs_graphics_buffer_usage_type,
    GS_GRAPHICS_BUFFER_USAGE_STATIC,
    GS_GRAPHICS_BUFFER_USAGE_STREAM,
    GS_GRAPHICS_BUFFER_USAGE_DYNAMIC
);

/* Buffer Update Type */
gs_enum_decl(gs_graphics_buffer_update_type,
    GS_GRAPHICS_BUFFER_UPDATE_RECREATE,
    GS_GRAPHICS_BUFFER_UPDATE_SUBDATA
);

/* Texture Format */
gs_enum_decl(gs_graphics_texture_format_type,
    GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
    GS_GRAPHICS_TEXTURE_FORMAT_RGB8,
    GS_GRAPHICS_TEXTURE_FORMAT_RGBA16F,
    GS_GRAPHICS_TEXTURE_FORMAT_RGBA32F,
    GS_GRAPHICS_TEXTURE_FORMAT_A8,
    GS_GRAPHICS_TEXTURE_FORMAT_R8,
    GS_GRAPHICS_TEXTURE_FORMAT_DEPTH8,
    GS_GRAPHICS_TEXTURE_FORMAT_DEPTH16,
    GS_GRAPHICS_TEXTURE_FORMAT_DEPTH24,
    GS_GRAPHICS_TEXTURE_FORMAT_DEPTH32F,
    GS_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8,
    GS_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8,
    GS_GRAPHICS_TEXTURE_FORMAT_STENCIL8
);

/* Texture Wrapping */
gs_enum_decl(gs_graphics_texture_wrapping_type,
    GS_GRAPHICS_TEXTURE_WRAP_REPEAT,
    GS_GRAPHICS_TEXTURE_WRAP_MIRRORED_REPEAT,
    GS_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE,
    GS_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_BORDER
);

/* Texture Filtering Type */
gs_enum_decl(gs_graphics_texture_filtering_type,
    GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
    GS_GRAPHICS_TEXTURE_FILTER_LINEAR
);

/* Render Pass Action Flag */
gs_enum_decl(gs_graphics_render_pass_action_flag,
    GS_GRAPHICS_RENDER_PASS_ACTION_CLEAR_COLOR = 0x01,
    GS_GRAPHICS_RENDER_PASS_ACTION_CLEAR_DEPTH = 0x02,
    GS_GRAPHICS_RENDER_PASS_ACTION_CLEAR_STENCIL = 0x04,
    GS_GRAPHICS_RENDER_PASS_ACTION_CLEAR_NONE = 0x08
);

#define GS_GRAPHICS_RENDER_PASS_ACTION_CLEAR_ALL\
    GS_GRAPHICS_RENDER_PASS_ACTION_CLEAR_COLOR |\
    GS_GRAPHICS_RENDER_PASS_ACTION_CLEAR_DEPTH |\
    GS_GRAPHICS_RENDER_PASS_ACTION_CLEAR_STENCIL

/* Bind Type */
gs_enum_decl(gs_graphics_bind_type,
    GS_GRAPHICS_BIND_VERTEX_BUFFER,
    GS_GRAPHICS_BIND_INDEX_BUFFER,
    GS_GRAPHICS_BIND_UNIFORM_BUFFER,
    GS_GRAPHICS_BIND_SAMPLER_BUFFER
);

/* Depth Function Type */
gs_enum_decl(gs_graphics_depth_func_type,       // Default value of 0x00 means depth is disabled
    GS_GRAPHICS_DEPTH_FUNC_NEVER,
    GS_GRAPHICS_DEPTH_FUNC_LESS,
    GS_GRAPHICS_DEPTH_FUNC_EQUAL,
    GS_GRAPHICS_DEPTH_FUNC_LEQUAL,
    GS_GRAPHICS_DEPTH_FUNC_GREATER,
    GS_GRAPHICS_DEPTH_FUNC_NOTEQUAL,
    GS_GRAPHICS_DEPTH_FUNC_GEQUAL,
    GS_GRAPHICS_DEPTH_FUNC_ALWAYS
);

/* Stencil Function Type */
gs_enum_decl(gs_graphics_stencil_func_type,
    GS_GRAPHICS_STENCIL_FUNC_NEVER,             // Default value of 0x00 means stencil is disabled
    GS_GRAPHICS_STENCIL_FUNC_LESS,
    GS_GRAPHICS_STENCIL_FUNC_EQUAL,
    GS_GRAPHICS_STENCIL_FUNC_LEQUAL,
    GS_GRAPHICS_STENCIL_FUNC_GREATER,
    GS_GRAPHICS_STENCIL_FUNC_NOTEQUAL,
    GS_GRAPHICS_STENCIL_FUNC_GEQUAL,
    GS_GRAPHICS_STENCIL_FUNC_ALWAYS
);

/* Stencil Op Type */
gs_enum_decl(gs_graphics_stencil_op_type,       // Default value of 0x00 means keep is used
    GS_GRAPHICS_STENCIL_OP_KEEP,
    GS_GRAPHICS_STENCIL_OP_ZERO,
    GS_GRAPHICS_STENCIL_OP_REPLACE,
    GS_GRAPHICS_STENCIL_OP_INCR,
    GS_GRAPHICS_STENCIL_OP_INCR_WRAP,
    GS_GRAPHICS_STENCIL_OP_DECR,
    GS_GRAPHICS_STENCIL_OP_DECR_WRAP,
    GS_GRAPHICS_STENCIL_OP_INVERT
);

/* Internal Graphics Resource Handles */
gs_handle_decl(gs_graphics_shader_t);
gs_handle_decl(gs_graphics_texture_t);
gs_handle_decl(gs_graphics_buffer_t);
gs_handle_decl(gs_graphics_uniform_block_t);
gs_handle_decl(gs_graphics_render_pass_t);
gs_handle_decl(gs_graphics_pipeline_t);

/* Graphics Shader Source Desc */
typedef struct gs_graphics_shader_source_desc_t
{
    gs_graphics_shader_stage_type type; // Shader stage type (vertex, fragment, tesselation, geometry, compute)
    const char* source;         // Source for shader
} gs_graphics_shader_source_desc_t;

/* Graphics Shader Desc */
typedef struct gs_graphics_shader_desc_t
{
    gs_graphics_shader_source_desc_t* sources;  // Array of shader source descriptions
    size_t size;                    // Size in bytes of shader source desc array
    const char* name;               // Optional (for logging and debugging mainly)
} gs_graphics_shader_desc_t;

/* Graphics Uniform Desc */
typedef struct gs_graphics_uniform_desc_t
{
    const char* name;               // Name of uniform in shader (used for opengl/es), MUST PROVIDE
    uint32_t type;                  // Type of uniform/sampler
    uint32_t slot;                  // Binding slot for textures to be bound
} gs_graphics_uniform_desc_t;

typedef gs_graphics_uniform_desc_t gs_graphics_sampler_desc_t;

/* Graphics Uniform Block Desc */
typedef struct gs_graphics_uniform_block_desc_t
{
    gs_handle(gs_graphics_shader_t) shader;     // Shader associated with uniforms, MUST PROVIDE
    gs_graphics_uniform_desc_t* uniforms;       // Array of uniform descriptions
    size_t size;                                // Size in bytes of uniform description array
    gs_graphics_shader_stage_type shader_stage; // Stage for this uniform block to applied (used for explicit renderers VK/DX12/MTL)
    gs_graphics_uniform_block_usage_type usage; // Designates whether this block is to be applied seldomly or frequently as a push constant
    const char* name;                           // Used for debugging
} gs_graphics_uniform_block_desc_t;

/* Graphics Texture Desc */
typedef struct gs_graphics_texture_desc_t
{
    uint32_t width;                                 // Width of texture in pixels
    uint32_t height;                                // Height of texture in pixels
    gs_graphics_texture_format_type format;         // Format of texture data (rgba32, rgba8, rgba32f, r8, depth32f, etc...)
    gs_graphics_texture_wrapping_type wrap_s;       // Wrapping type for s axis of texture
    gs_graphics_texture_wrapping_type wrap_t;       // Wrapping type for t axis of texture
    gs_graphics_texture_filtering_type min_filter;  // Minification filter for texture
    gs_graphics_texture_filtering_type mag_filter;  // Magnification filter for texture
    gs_graphics_texture_filtering_type mip_filter;  // Mip filter for texture
    uint32_t num_mips;                              // Number of mips to generate (default 0 is disable mip generation)
    void* data;                                     // Texture data to upload (can be null)
    b32 render_target;                              // Default to false (not a render target)
} gs_graphics_texture_desc_t;

/* Graphics Buffer Desc */
typedef struct gs_graphics_buffer_desc_t
{
    gs_graphics_buffer_type type;               // Type of buffer (vertex, index, frame, uniform, sampler)
    gs_graphics_buffer_usage_type usage;        // Usage type of buffer (static, dynamic, stream, draw, read)
    void* data;                                 // Array of buffer data
    size_t size;                                // Size in bytes of buffer data array
    const char* name;                           // Name of buffer (required for sampler/uniform buffers)
    struct {
        size_t offset;
        gs_graphics_buffer_update_type type;
    } update;
} gs_graphics_buffer_desc_t;

typedef gs_graphics_buffer_desc_t gs_graphics_vertex_buffer_desc_t;
typedef gs_graphics_buffer_desc_t gs_graphics_index_buffer_desc_t;
typedef gs_graphics_buffer_desc_t gs_graphics_uniform_buffer_desc_t;
typedef gs_graphics_buffer_desc_t gs_graphics_sampler_buffer_desc_t;

/* Graphics Render Pass Action */
typedef struct gs_graphics_render_pass_action_t
{
    gs_graphics_render_pass_action_flag flag;   // Flag to be set (clear color, clear depth, clear stencil, clear all)
    union 
    {
        float color[4];                             // Clear color value
        float depth;                                // Clear depth value
        int32_t stencil;                            // Clear stencil value
    };
} gs_graphics_render_pass_action_t;

/* Graphics Render Pass Desc */
typedef struct gs_graphics_render_pass_desc_t
{
    gs_handle(gs_graphics_buffer_t) fbo;      // Default is set to invalid for backbuffer
    gs_handle(gs_graphics_texture_t)* color;  // Array of color attachments to be bound (useful for MRT, if supported) 
    size_t color_size;                        // Size of color attachment array
    gs_handle(gs_graphics_texture_t) depth;   // Depth attachment to be bound
    gs_handle(gs_graphics_texture_t) stencil; // Depth attachment to be bound
} gs_graphics_render_pass_desc_t;

/* 
    // If you want to write to a color attachment, you have to have a frame buffer attached that isn't the backbuffer
*/

typedef enum gs_graphics_vertex_data_type
{
    GS_GRAPHICS_VERTEX_DATA_INTERLEAVED = 0x00,
    GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED
} gs_graphics_vertex_data_type;

/* Graphics Binding Desc */
typedef struct gs_graphics_bind_desc_t
{
    gs_graphics_bind_type type;             // Type of data to bind (vertex buffer, index buffer, uniform buffer, sampler buffer)
    gs_handle(gs_graphics_buffer_t) buffer; // Buffer to bind (vertex, index, uniform, sampler)
    void* data;                             // Data associated with bind
    size_t size;                            // Size of data in bytes
    uint32_t binding;                       // Binding for tex units
    size_t offset;                          // For manual offset with vertex data (used for non-interleaved data) 
    gs_graphics_vertex_data_type data_type; // Interleaved/Non-interleaved data
    uint32_t clear_previous;                // Clearing previous vertex buffer data binds
} gs_graphics_bind_desc_t;

/* Graphics Blend State Desc */
typedef struct gs_graphics_blend_state_desc_t
{
    gs_graphics_blend_equation_type func;   // Equation function to use for blend ops
    gs_graphics_blend_mode_type src;        // Source blend mode
    gs_graphics_blend_mode_type dst;        // Destination blend mode
} gs_graphics_blend_state_desc_t;

/* Graphics Depth State Desc */
typedef struct gs_graphics_depth_state_desc_t
{
    gs_graphics_depth_func_type func;           // Function to set for depth test
} gs_graphics_depth_state_desc_t;

/* Graphics Stencil State Desc */
typedef struct gs_graphics_stencil_state_desc_t
{
    gs_graphics_stencil_func_type   func;   // Function to set for stencil test
    uint32_t                        ref;    // Specifies reference val for stencil test
    uint32_t                        mask;   // Specifies mask that is ANDed with both ref val and stored stencil val
    gs_graphics_stencil_op_type     sfail;  // Action to take when stencil test fails
    gs_graphics_stencil_op_type     dpfail; // Action to take when stencil test passes but depth test fails
    gs_graphics_stencil_op_type     dppass; // Action to take when both stencil test passes and either depth passes or is not enabled
} gs_graphics_stencil_state_desc_t;

/* Graphics Raster State Desc */
typedef struct gs_graphics_raster_state_desc_t
{
    gs_graphics_face_culling_type face_culling;   // Face culling mode to be used (front, back, front and back)
    gs_graphics_winding_order_type winding_order; // Winding order mode to be used (ccw, cw)
    gs_graphics_primitive_type primitive;         // Primitive type for drawing (lines, quads, triangles, triangle strip)
    gs_handle(gs_graphics_shader_t) shader;       // Shader to bind and use (might be in bindables later on, not sure)
    size_t index_buffer_element_size;             // Element size of index buffer (used for parsing internal data)
} gs_graphics_raster_state_desc_t;

/* Graphics Vertex Attribute Desc */
typedef struct gs_graphics_vertex_attribute_desc_t {
    gs_graphics_vertex_attribute_type format;           // Format for vertex attribute
    size_t stride;                                      // Total stride of vertex layout (optional, calculated by default)
    size_t offset;                                      // Offset of this vertex from base pointer of data (optional, calaculated by default)
    size_t divisor;                                     // Used for instancing. (optional, default = 0x00 for no instancing)
    uint32_t buffer_idx;                                // Vertex buffer to use (optional, default = 0x00)
} gs_graphics_vertex_attribute_desc_t;

/* Graphics Vertex Layout Desc */
typedef struct gs_graphics_vertex_layout_desc_t {
    gs_graphics_vertex_attribute_desc_t* attrs;      // Vertex attribute array
    size_t size;                                    // Size in bytes of vertex attribute array
} gs_graphics_vertex_layout_desc_t;

/* Graphics Pipeline Desc */
typedef struct gs_graphics_pipeline_desc_t
{
    gs_graphics_blend_state_desc_t blend;       // Blend state desc for pipeline
    gs_graphics_depth_state_desc_t depth;       // Depth state desc for pipeline
    gs_graphics_raster_state_desc_t raster;     // Raster state desc for pipeline
    gs_graphics_stencil_state_desc_t stencil;   // Stencil state desc for pipeline
    gs_graphics_vertex_layout_desc_t layout; // Vertex layout desc for pipeline
} gs_graphics_pipeline_desc_t;

/*==========================
// Graphics Interface
==========================*/

typedef struct gs_graphics_i
{
    void* user_data; // For internal use
} gs_graphics_i;

/*==========================
// Graphics API
==========================*/

/* Graphics Interface Creation / Initialization / Shutdown / Destruction */
GS_API_DECL gs_graphics_i* gs_graphics_create();
GS_API_DECL void           gs_graphics_destroy(gs_graphics_i* graphics);
GS_API_DECL gs_result      gs_graphics_init(gs_graphics_i* graphics);
GS_API_DECL gs_result      gs_graphics_shutdown(gs_graphics_i* graphics);

/* Resource Creation */
GS_API_DECL gs_handle(gs_graphics_texture_t)     gs_graphics_texture_create(gs_graphics_texture_desc_t* desc);
GS_API_DECL gs_handle(gs_graphics_buffer_t)      gs_graphics_buffer_create(gs_graphics_buffer_desc_t* desc);
GS_API_DECL gs_handle(gs_graphics_shader_t)      gs_graphics_shader_create(gs_graphics_shader_desc_t* desc);
GS_API_DECL gs_handle(gs_graphics_render_pass_t) gs_graphics_render_pass_create(gs_graphics_render_pass_desc_t* desc);
GS_API_DECL gs_handle(gs_graphics_pipeline_t)    gs_graphics_pipeline_create(gs_graphics_pipeline_desc_t* desc);

/* Resource Destruction */
GS_API_DECL void gs_graphics_texture_destroy(gs_handle(gs_graphics_texture_t) hndl);
GS_API_DECL void gs_graphics_buffer_destroy(gs_handle(gs_graphics_buffer_t) hndl);
GS_API_DECL void gs_graphics_shader_destroy(gs_handle(gs_graphics_shader_t) hndl);
GS_API_DECL void gs_graphics_render_pass_destroy(gs_handle(gs_graphics_render_pass_t) hndl);
GS_API_DECL void gs_graphics_pipeline_destroy(gs_handle(gs_graphics_pipeline_t) hndl);

/* Resource In-Flight Update*/
GS_API_DECL void gs_graphics_texture_request_update(gs_command_buffer_t* cb, gs_handle(gs_graphics_texture_t) hndl, gs_graphics_texture_desc_t* desc);
GS_API_DECL void gs_graphics_buffer_request_update(gs_command_buffer_t* cb, gs_handle(gs_graphics_buffer_t) hndl, gs_graphics_buffer_desc_t* desc);

/* Pipeline / Pass / Bind / Draw */
GS_API_DECL void gs_graphics_begin_render_pass(gs_command_buffer_t* cb, gs_handle(gs_graphics_render_pass_t) hndl, gs_graphics_render_pass_action_t* actions, size_t actions_size);
GS_API_DECL void gs_graphics_end_render_pass(gs_command_buffer_t* cb);
GS_API_DECL void gs_graphics_set_viewport(gs_command_buffer_t* cb, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
GS_API_DECL void gs_graphics_set_view_scissor(gs_command_buffer_t* cb, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
GS_API_DECL void gs_graphics_bind_pipeline(gs_command_buffer_t* cb, gs_handle(gs_graphics_pipeline_t) hndl);
GS_API_DECL void gs_graphics_bind_bindings(gs_command_buffer_t* cb, gs_graphics_bind_desc_t* binds, size_t binds_size);
GS_API_DECL void gs_graphics_draw(gs_command_buffer_t* cb, uint32_t start, uint32_t count, uint32_t instance_count);

/* Submission (Main Thread) */
GS_API_DECL void gs_graphics_submit_command_buffer(gs_command_buffer_t* cb);
GS_API_DECL void gs_graphics_submit_frame();

#ifndef GS_NO_SHORT_NAME
    
typedef gs_handle(gs_graphics_shader_t)      gs_shader;
typedef gs_handle(gs_graphics_texture_t)     gs_texture;
typedef gs_handle(gs_graphics_buffer_t)      gs_gfxbuffer;
typedef gs_handle(gs_graphics_render_pass_t) gs_renderpass;
typedef gs_handle(gs_graphics_pipeline_t)    gs_pipeline;

#endif

/*==========================
// GS_ASSET_TYPES
==========================*/

// Texture
typedef struct gs_asset_texture_t
{
    gs_handle(gs_graphics_texture_t) hndl;
    gs_graphics_texture_desc_t desc;
} gs_asset_texture_t;

GS_API_DECL void gs_asset_texture_load_from_file(const char* path, void* out, gs_graphics_texture_desc_t* desc, bool32_t flip_on_load, bool32_t keep_data);

// Font
typedef struct gs_baked_char_t
{
   u16 x0,y0,x1,y1;
   f32 xoff,yoff,xadvance;
} gs_baked_char_t;

typedef struct gs_asset_font_t
{
    void* font_info;
    gs_baked_char_t glyphs[96];
    gs_asset_texture_t texture;
} gs_asset_font_t; 

GS_API_DECL void gs_asset_font_load_from_file(const char* path, void* out, uint32_t point_size);

// Audio
typedef struct gs_asset_audio_t
{
    gs_handle(gs_audio_source_t) hndl;
} gs_asset_audio_t;

GS_API_DECL void gs_asset_audio_load_from_file(const char* path, void* out);

// Mesh
gs_enum_decl(gs_asset_mesh_attribute_type,
    GS_ASSET_MESH_ATTRIBUTE_TYPE_POSITION,
    GS_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL,
    GS_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT,
    GS_ASSET_MESH_ATTRIBUTE_TYPE_JOINT,
    GS_ASSET_MESH_ATTRIBUTE_TYPE_WEIGHT,
    GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD,
    GS_ASSET_MESH_ATTRIBUTE_TYPE_COLOR
);

typedef struct gs_asset_mesh_layout_t {
    gs_asset_mesh_attribute_type type;     // Type of attribute
    uint32_t idx;                          // Optional index (for joint/weight/texcoord/color)
} gs_asset_mesh_layout_t;

typedef struct gs_asset_mesh_decl_t
{
    gs_asset_mesh_layout_t* layout;        // Mesh attribute layout array
    size_t layout_size;                    // Size of mesh attribute layout array in bytes
    size_t index_buffer_element_size;      // Size of index data size in bytes
} gs_asset_mesh_decl_t;

typedef struct gs_asset_mesh_primitive_t
{
    gs_handle(gs_graphics_buffer_t) vbo;
    gs_handle(gs_graphics_buffer_t) ibo;
    uint32_t count; 
} gs_asset_mesh_primitive_t;

typedef struct gs_asset_mesh_t
{
    gs_dyn_array(gs_asset_mesh_primitive_t) primitives;
} gs_asset_mesh_t;

// Structured/packed raw mesh data
typedef struct gs_asset_mesh_raw_data_t 
{
    uint32_t prim_count;
    size_t* vertex_sizes;
    size_t* index_sizes;
    void** vertices;
    void** indices;
} gs_asset_mesh_raw_data_t;

GS_API_DECL void gs_asset_mesh_load_from_file(const char* path, void* out, gs_asset_mesh_decl_t* decl, void* data_out, size_t data_size);

GS_API_DECL void gs_util_load_gltf_data_from_file(const char* path, gs_asset_mesh_decl_t* decl, gs_asset_mesh_raw_data_t** out, uint32_t* mesh_count);

/*
    // Could pass in a mesh decl? Then it'll just give you back packed vertex/index data for each primitive?
    GS_API_DECL gs_util_load_gltf_from_file(const char* path, gs_asset_mesh_decl_t* decl, uint32_t* primitive_count, void*** verticies, size_t** vertex_sizes, void*** indices, size_t** index_sizes);
    
    To use: 

    // For primitives
    uint32_t prim_count = 0;
    size_t* vertex_sizes = NULL;
    size_t* index_sizes = NULL;
    float** vertices = NULL;
    uint32_t** indices = NULL;

    gs_asset_mesh_raw_data_t data = {};
    gs_util_load_gltf_from_file("path", &decl, &data);
*/

/*==========================
// GS_ENGINE / GS_APP
==========================*/

// Application descriptor for user application
typedef struct gs_app_desc_t
{
    void (* init)();
    void (* update)();
    void (* shutdown)();
    const char* window_title;
    uint32_t window_width;
    uint32_t window_height;
    uint32_t window_flags;
    float frame_rate;
    bool32 enable_vsync;
    bool32 is_running;
    void* user_data;
} gs_app_desc_t;

/*
    Game Engine Context: 

    * This is the main context for the gunslinger engine. Holds pointers to 
        all interfaces registered with the engine, including the description 
        for your application.
*/
typedef struct gs_engine_context_t
{
    gs_platform_i* platform;
    gs_graphics_i* graphics;
    gs_audio_i* audio;
    gs_app_desc_t app;
} gs_engine_context_t;

typedef struct gs_engine_t
{
    gs_engine_context_t ctx;
    gs_result (* run)();
    gs_result (* shutdown)();
} gs_engine_t;

/* Desc */
GS_API_DECL gs_engine_t* gs_engine_create(gs_app_desc_t app_desc);
/* Desc */
GS_API_DECL void gs_engine_destroy();
/* Desc */
GS_API_DECL gs_engine_t* gs_engine_instance();
/* Desc */
GS_API_DECL gs_engine_context_t* gs_engine_ctx();
/* Desc */
GS_API_DECL void gs_engine_quit();
/* Desc */
GS_API_DECL gs_app_desc_t gs_main(int32_t argc, char** argv);

#define gs_engine_subsystem(__T)\
    (gs_engine_instance()->ctx.__T)

#define gs_engine_user_data(__T)\
    (__T*)(gs_engine_instance()->ctx.app.user_data)

/*==================================================================================================================================
// ===== Gunslinger Implementation ============================================================================================== //
==================================================================================================================================*/

#ifdef GS_IMPL

/*========================
// gs_byte_buffer
========================*/

void gs_byte_buffer_init(gs_byte_buffer_t* buffer)
{
    buffer->data     = (uint8_t*)gs_malloc(GS_BYTE_BUFFER_DEFAULT_CAPCITY);
    buffer->capacity = GS_BYTE_BUFFER_DEFAULT_CAPCITY;
    buffer->size     = 0;
    buffer->position = 0;
}

gs_byte_buffer_t gs_byte_buffer_new()
{
    gs_byte_buffer_t buffer;
    gs_byte_buffer_init(&buffer);
    return buffer;
}

void gs_byte_buffer_free(gs_byte_buffer_t* buffer)
{
    if (buffer && buffer->data) {
        gs_free(buffer->data);
    }
}

void gs_byte_buffer_clear(gs_byte_buffer_t* buffer)
{
    buffer->size = 0;
    buffer->position = 0;   
}

void gs_byte_buffer_resize(gs_byte_buffer_t* buffer, size_t sz)
{
    uint8_t* data = (uint8_t*)gs_realloc(buffer->data, sz);

    if (data == NULL) {
        return;
    }

    buffer->data = data;    
    buffer->capacity = (uint32_t)sz;
}

void gs_byte_buffer_seek_to_beg(gs_byte_buffer_t* buffer)
{
    buffer->position = 0;
}

void gs_byte_buffer_seek_to_end(gs_byte_buffer_t* buffer)
{
    buffer->position = buffer->size;
}

void gs_byte_buffer_advance_position(gs_byte_buffer_t* buffer, size_t sz)
{
    buffer->position += (uint32_t)sz; 
}

void gs_byte_buffer_write_bulk(gs_byte_buffer_t* buffer, void* src, size_t size)
{
    // Check for necessary resize
    size_t total_write_size = buffer->position + size;
    if (total_write_size >= (size_t)buffer->capacity)
    {
        size_t capacity = buffer->capacity * 2;
        while(capacity <= total_write_size)
        {
            capacity *= 2;
        }

        gs_byte_buffer_resize(buffer, capacity);
    } 

    // memcpy data
     memcpy((buffer->data + buffer->position), src, size);

    buffer->size += (uint32_t)size;
    buffer->position += (uint32_t)size;
}

void gs_byte_buffer_read_bulk(gs_byte_buffer_t* buffer, void** dst, size_t size)
{
    memcpy(*dst, (buffer->data + buffer->position), size);
    buffer->position += (uint32_t)size;
}

void gs_byte_buffer_write_str(gs_byte_buffer_t* buffer, const char* str)
{
    // Write size of string
    uint32_t str_len = gs_string_length(str);
    gs_byte_buffer_write(buffer, uint16_t, str_len);

    size_t i; 
    for (i = 0; i < str_len; ++i)
    {
        gs_byte_buffer_write(buffer, uint8_t, str[i]);
    }
}

void gs_byte_buffer_read_str(gs_byte_buffer_t* buffer, char* str)
{
    // Read in size of string from buffer
    uint16_t sz;
    gs_byte_buffer_read(buffer, uint16_t, &sz);

    uint32_t i;
    for (i = 0; i < sz; ++i)
    {
        gs_byte_buffer_read(buffer, uint8_t, &str[i]);
    }
    str[i] = '\0';
}

gs_result 
gs_byte_buffer_write_to_file
(
    gs_byte_buffer_t* buffer, 
    const char* output_path 
)
{
    FILE* fp = fopen(output_path, "wb");
    if (fp) 
    {
        int32_t ret = (int32_t)fwrite(buffer->data, sizeof(u8), buffer->size, fp);
        if (ret == buffer->size)
        {
            return GS_RESULT_SUCCESS;
        }
    }
    return GS_RESULT_FAILURE;
}

gs_result 
gs_byte_buffer_read_from_file
(
    gs_byte_buffer_t* buffer, 
    const char* file_path 
)
{
    buffer->data = (u8*)gs_read_file_contents_into_string_null_term(file_path, "rb", (usize*)&buffer->size);
    if (!buffer->data) {
        gs_assert(false);   
        return GS_RESULT_FAILURE;
    }
    buffer->position = 0;
    buffer->capacity = buffer->size;
    return GS_RESULT_SUCCESS;
}

/*=============================
// Camera
=============================*/

gs_camera_t gs_camera_default()
{
    // Construct default camera parameters
    gs_camera_t cam = gs_default_val();
    cam.transform = gs_vqs_default();
    cam.transform.position.z = 1.f;
    cam.fov = 60.f;
    cam.near_plane = 0.1f;
    cam.far_plane = 1000.f;
    cam.ortho_scale = 1.f;
    cam.proj_type = GS_PROJECTION_TYPE_ORTHOGRAPHIC;
    return cam;
}

gs_camera_t gs_camera_perspective()
{
    gs_camera_t cam = gs_camera_default();
    cam.proj_type = GS_PROJECTION_TYPE_PERSPECTIVE;
    cam.transform.position.z = 1.f;
    return cam;
}

gs_vec3 gs_camera_forward(gs_camera_t* cam)
{
    return (gs_quat_rotate(cam->transform.rotation, gs_v3(0.0f, 0.0f, -1.0f)));
} 

gs_vec3 gs_camera_backward(gs_camera_t* cam)
{
    return (gs_quat_rotate(cam->transform.rotation, gs_v3(0.0f, 0.0f, 1.0f)));
} 

gs_vec3 gs_camera_up(gs_camera_t* cam)
{
    return (gs_quat_rotate(cam->transform.rotation, gs_v3(0.0f, 1.0f, 0.0f)));
}

gs_vec3 gs_camera_down(gs_camera_t* cam)
{
    return (gs_quat_rotate(cam->transform.rotation, gs_v3(0.0f, -1.0f, 0.0f)));
}

gs_vec3 gs_camera_right(gs_camera_t* cam)
{
    return (gs_quat_rotate(cam->transform.rotation, gs_v3(1.0f, 0.0f, 0.0f)));
}

gs_vec3 gs_camera_left(gs_camera_t* cam)
{
    return (gs_quat_rotate(cam->transform.rotation, gs_v3(-1.0f, 0.0f, 0.0f)));
}

gs_vec3 gs_camera_screen_to_world(gs_camera_t* cam, gs_vec3 coords, s32 view_width, s32 view_height)
{
    gs_vec3 wc = gs_default_val();

    // Get inverse of view projection from camera
    gs_mat4 inverse_vp = gs_mat4_inverse(gs_camera_get_view_projection(cam, view_width, view_height));  

    f32 w_x = (f32)coords.x;
    f32 w_y = (f32)coords.y;
    f32 w_z = (f32)coords.z;

    // Transform from ndc
    gs_vec4 in;
    in.x = (w_x / (f32)view_width) * 2.f - 1.f;
    in.y = 1.f - (w_y / (f32)view_height) * 2.f;
    in.z = 2.f * w_z - 1.f;
    in.w = 1.f;

    // To world coords
    gs_vec4 out = gs_mat4_mul_vec4(inverse_vp, in);
    if (out.w == 0.f)
    {
        // Avoid div by zero
        return wc;
    }

    out.w = 1.f / out.w;
    wc = gs_v3(
        out.x * out.w,
        out.y * out.w,
        out.z * out.w
    );

    return wc;
}

gs_mat4 gs_camera_get_view_projection(gs_camera_t* cam, s32 view_width, s32 view_height)
{
    gs_mat4 view = gs_camera_get_view(cam);
    gs_mat4 proj = gs_camera_get_projection(cam, view_width, view_height);
    return gs_mat4_mul(proj, view); 
}

gs_mat4 gs_camera_get_view(gs_camera_t* cam)
{
    gs_vec3 up = gs_camera_up(cam);
    gs_vec3 forward = gs_camera_forward(cam);
    gs_vec3 target = gs_vec3_add(forward, cam->transform.position);
    return gs_mat4_look_at(cam->transform.position, target, up);
}

gs_mat4 gs_camera_get_projection(gs_camera_t* cam, s32 view_width, s32 view_height)
{
    gs_mat4 proj_mat = gs_mat4_identity();

    switch(cam->proj_type)
    {
        case GS_PROJECTION_TYPE_PERSPECTIVE:
        {
            proj_mat = gs_mat4_perspective(cam->fov, (f32)view_width / (f32)view_height, cam->near_plane, cam->far_plane);
        } break;

        // Don't like this...
        case GS_PROJECTION_TYPE_ORTHOGRAPHIC:
        {
            f32 _ar = (f32)view_width / (f32)view_height;
            f32 distance = 0.5f * (cam->far_plane - cam->near_plane);
            const f32 ortho_scale = cam->ortho_scale;
            const f32 aspect_ratio = _ar;
            proj_mat = gs_mat4_ortho
            (
                -ortho_scale * aspect_ratio, 
                ortho_scale * aspect_ratio, 
                -ortho_scale, 
                ortho_scale, 
                -distance, 
                distance    
            );
            // (
            //  0.f, 
            //  view_width, 
            //  view_height, 
            //  0.f, 
            //  cam->near_plane, 
            //  cam->far_plane  
            //);
        } break;
    }

    return proj_mat;
}

void gs_camera_offset_orientation(gs_camera_t* cam, f32 yaw, f32 pitch)
{
    gs_quat x = gs_quat_angle_axis(gs_deg2rad(yaw), gs_v3(0.f, 1.f, 0.f));      // Absolute up
    gs_quat y = gs_quat_angle_axis(gs_deg2rad(pitch), gs_camera_right(cam));            // Relative right
    cam->transform.rotation = gs_quat_mul(gs_quat_mul(x, y), cam->transform.rotation);
}

/*=============================
// GS_UTIL
=============================*/

#ifndef GS_NO_STB_RECT_PACK
    #define STB_RECT_PACK_IMPLEMENTATION
#endif

#ifndef GS_NO_STB_TRUETYPE
    #define STB_TRUETYPE_IMPLEMENTATION
#endif

#ifndef GS_NO_STB_DEFINE
    #define STB_DEFINE
#endif

#ifndef GS_NO_STB_IMAGE
    #define STB_IMAGE_IMPLEMENTATION
    #define STB_IMAGE_WRITE_IMPLEMENTATION
#endif

#ifndef GS_NO_CGLTF
    #define CGLTF_IMPLEMENTATION
#endif

// STB
#include "external/stb/stb.h"
#include "external/stb/stb_image_write.h"
#include "external/stb/stb_truetype.h"
#include "external/stb/stb_image.h"

// CGLTF
#include "external/cgltf/cgltf.h"

bool32_t gs_util_load_texture_data_from_file(const char* file_path, int32_t* width, int32_t* height, uint32_t* num_comps, void** data, bool32_t flip_vertically_on_load)
{
    // Load texture data
    stbi_set_flip_vertically_on_load(flip_vertically_on_load);

    // NOTE(john): For now, this data will always have 4 components, since STBI_rgb_alpha is being passed in as required components param. Could optimize this later.
    *data = stbi_load(file_path, (s32*)width, (s32*)height, (s32*)num_comps, STBI_rgb_alpha);

    if (!*data) {
        gs_println("Warning: could not load texture: %s", file_path);
        return false;
    }

    return true;
}

/*=============================
// GS_PLATFORM
=============================*/

#ifndef GS_PLATFORM_IMPL_CUSTOM
    #define GS_PLATFORM_IMPL_GLFW
    #include "impl/gs_platform_impl.h"
#endif

/*=============================
// GS_GRAPHICS
=============================*/

#ifndef GS_GRAPHICS_IMPL_CUSTOM
    #define GS_GRAPHICS_IMPL_OPENGL
    #include "impl/gs_graphics_impl.h"
#endif

/*=============================
// GS_AUDIO
=============================*/

#ifndef GS_AUDIO_IMPL_CUSTOM
    #define GS_AUDIO_IMPL_MINIAUDIO
    #include "impl/gs_audio_impl.h"
#endif

/*==========================
// GS_ASSET_TYPES
==========================*/

void gs_asset_texture_load_from_file(const char* path, void* out, gs_graphics_texture_desc_t* desc, bool32_t flip_on_load, bool32_t keep_data)
{
    gs_asset_texture_t* t = (gs_asset_texture_t*)out; 

    memset(&t->desc, 0, sizeof(gs_graphics_texture_desc_t));

    if (desc) {
        t->desc = *desc;
    } else {
        t->desc.format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8;
        t->desc.min_filter = GS_GRAPHICS_TEXTURE_FILTER_LINEAR;
        t->desc.mag_filter = GS_GRAPHICS_TEXTURE_FILTER_LINEAR; 
        t->desc.wrap_s = GS_GRAPHICS_TEXTURE_WRAP_REPEAT;
        t->desc.wrap_t = GS_GRAPHICS_TEXTURE_WRAP_REPEAT;
    }

    // Load texture data
    int32_t num_comps = 0;
    bool32_t loaded = gs_util_load_texture_data_from_file(path, (int32_t*)&t->desc.width, 
        (int32_t*)&t->desc.height, (uint32_t*)&num_comps, (void**)&t->desc.data, flip_on_load);

    if (!loaded) {
        gs_println("Warning: could not load texture: %s", path);
        return;
    }

    t->hndl = gs_graphics_texture_create(&t->desc);

    if (!keep_data) {
        gs_free(t->desc.data);
        t->desc.data = NULL;
    }
}

void gs_asset_font_load_from_file(const char* path, void* out, uint32_t point_size)
{ 
    gs_asset_font_t* f = (gs_asset_font_t*)out;

    if (!point_size) {
        gs_println("Warning: Font: %s: Point size not declared. Setting to default 16.", path);
        point_size = 16;
    }

    stbtt_fontinfo font = gs_default_val();
    char* ttf = gs_read_file_contents_into_string_null_term(path, "rb", NULL);
    const u32 w = 512;
    const u32 h = 512;
    const u32 num_comps = 4;
    u8* alpha_bitmap = (u8*)gs_malloc(w * h);
    u8* flipmap = (u8*)gs_malloc(w * h * num_comps);
    memset(alpha_bitmap, 0, w * h);
    memset(flipmap, 0, w * h * num_comps);
    s32 v = stbtt_BakeFontBitmap((u8*)ttf, 0, (float)point_size, alpha_bitmap, w, h, 32, 96, (stbtt_bakedchar*)f->glyphs); // no guarantee this fits!

    // Flip texture
    u32 r = h - 1;
    for (u32 i = 0; i < h; ++i)
    {
        for (u32 j = 0; j < w; ++j)
        {
            u32 i0 = i * w + j;
            u32 i1 = i * w * num_comps + j * num_comps;
            u8 a = alpha_bitmap[i0];
            flipmap[i1 + 0] = 255;
            flipmap[i1 + 1] = 255;
            flipmap[i1 + 2] = 255;
            flipmap[i1 + 3] = a;
        }
        r--;
    }

    gs_graphics_texture_desc_t desc = gs_default_val();
    desc.width = w;
    desc.height = h;
    desc.data = flipmap;
    desc.format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    desc.min_filter = GS_GRAPHICS_TEXTURE_FILTER_LINEAR;

    // Generate atlas texture for bitmap with bitmap data
    f->texture.hndl = gs_graphics_texture_create(&desc);
    f->texture.desc = desc;
    f->texture.desc.data = NULL;

    if (v == 0) {
        gs_println("Font Failed to Load: %s, %d", path, v);
    }
    else {
        gs_println("Font Successfully Load: %s, %d", path, v);
    }

    gs_free(ttf);
    gs_free(alpha_bitmap);
    gs_free(flipmap);
}

// Audio
void gs_asset_audio_load_from_file(const char* path, void* out)
{
    gs_asset_audio_t* a = (gs_asset_audio_t*)out;
    a->hndl = gs_audio_load_from_file(path);
}

// Mesh
void gs_util_load_gltf_data_from_file(const char* path, gs_asset_mesh_decl_t* decl, gs_asset_mesh_raw_data_t** out, uint32_t* mesh_count)
{
    // Use cgltf like a boss
    cgltf_options options = gs_default_val();
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, path, &data);

    if (result != cgltf_result_success)
    {
        gs_println("Mesh:LoadFromFile:Failed load gltf: %s", path);
        return;
    }

    // Load buffers as well
    result = cgltf_load_buffers(&options, data, path);
    if (result != cgltf_result_success)
    {
        cgltf_free(data);
        gs_println("Mesh:LoadFromFile:Failed to load buffers: %s", path);
        return;
    }

    // Type of index data
    size_t index_element_size = decl ? decl->index_buffer_element_size : 0;

    // Temporary structures
    gs_dyn_array(gs_vec3) positions = NULL;
    gs_dyn_array(gs_vec3) normals = NULL;
    gs_dyn_array(gs_vec3) tangents = NULL;
    gs_dyn_array(gs_color_t) colors = NULL;
    gs_dyn_array(gs_vec2) uvs = NULL;
    gs_dyn_array(gs_asset_mesh_layout_t) layouts = NULL;
    gs_byte_buffer_t v_data = gs_byte_buffer_new();
    gs_byte_buffer_t i_data = gs_byte_buffer_new();

    // Allocate memory for buffers
    *mesh_count = data->meshes_count;
    *out = (gs_asset_mesh_raw_data_t*)gs_malloc(data->meshes_count * sizeof(gs_asset_mesh_raw_data_t));
    memset(*out, 0, sizeof(gs_asset_mesh_raw_data_t) * data->meshes_count);

    // Iterate through meshes in data
    for (uint32_t i = 0; i < data->meshes_count; ++i)
    {
        // Initialize mesh data
        gs_asset_mesh_raw_data_t* mesh = out[i];
        mesh->prim_count = data->meshes[i].primitives_count;
        mesh->vertex_sizes = (size_t*)gs_malloc(sizeof(size_t) * mesh->prim_count);
        mesh->index_sizes = (size_t*)gs_malloc(sizeof(size_t) * mesh->prim_count);
        mesh->vertices = (void**)gs_malloc(sizeof(size_t) * mesh->prim_count);
        mesh->indices = (void**)gs_malloc(sizeof(size_t) * mesh->prim_count);

        // For each primitive in mesh 
        for (uint32_t p = 0; p < data->meshes[i].primitives_count; ++p)
        {
            // Clear temp data from previous use
            gs_dyn_array_clear(positions);
            gs_dyn_array_clear(normals);
            gs_dyn_array_clear(tangents);
            gs_dyn_array_clear(uvs);
            gs_dyn_array_clear(colors);
            gs_dyn_array_clear(layouts);
            gs_byte_buffer_clear(&v_data);
            gs_byte_buffer_clear(&i_data);

            #define __GLTF_PUSH_ATTR(ATTR, TYPE, COUNT, ARR, ARR_TYPE, LAYOUTS, LAYOUT_TYPE)\
                do {\
                    int32_t N = 0;\
                    TYPE* BUF = (TYPE*)ATTR->buffer_view->buffer->data + ATTR->buffer_view->offset/sizeof(TYPE) + ATTR->offset/sizeof(TYPE);\
                    gs_assert(BUF);\
                    TYPE V[COUNT] = gs_default_val();\
                    /* For each vertex */\
                    for (uint32_t k = 0; k < ATTR->count; k++)\
                    {\
                        /* For each element */\
                        for (int l = 0; l < COUNT; l++) {\
                            V[l] = BUF[N + l];\
                        }\
                        N += (int32_t)(ATTR->stride/sizeof(TYPE));\
                        /* Add to temp data array */\
                        ARR_TYPE ELEM = gs_default_val();\
                        memcpy((void*)&ELEM, (void*)V, sizeof(ARR_TYPE));\
                        gs_dyn_array_push(ARR, ELEM);\
                    }\
                    /* Push into layout */\
                    gs_asset_mesh_layout_t LAYOUT = gs_default_val();\
                    LAYOUT.type = LAYOUT_TYPE;\
                    gs_dyn_array_push(LAYOUTS, LAYOUT);\
                } while (0)

            // For each attribute in primitive
            for (uint32_t a = 0; a < data->meshes[i].primitives[p].attributes_count; ++a)
            {
                // Accessor for attribute data
                cgltf_accessor* attr = data->meshes[i].primitives[p].attributes[a].data;

                // Switch on type for reading data
                switch (data->meshes[i].primitives[p].attributes[a].type)
                {
                    case cgltf_attribute_type_position: {
                        __GLTF_PUSH_ATTR(attr, float, 3, positions, gs_vec3, layouts, GS_ASSET_MESH_ATTRIBUTE_TYPE_POSITION);
                    } break;

                    case cgltf_attribute_type_normal: {
                        __GLTF_PUSH_ATTR(attr, float, 3, normals, gs_vec3, layouts, GS_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL);
                    } break;

                    case cgltf_attribute_type_tangent: {
                        __GLTF_PUSH_ATTR(attr, float, 3, tangents, gs_vec3, layouts, GS_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT);
                    } break;

                    case cgltf_attribute_type_texcoord: {
                        __GLTF_PUSH_ATTR(attr, float, 2, uvs, gs_vec2, layouts, GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD);
                    } break;

                    case cgltf_attribute_type_color: {
                        __GLTF_PUSH_ATTR(attr, uint8_t, 4, colors, gs_color_t, layouts, GS_ASSET_MESH_ATTRIBUTE_TYPE_COLOR);
                    } break;

                    // Not sure what to do with these for now
                    case cgltf_attribute_type_joints: 
                    {
                        // Push into layout
                        gs_asset_mesh_layout_t layout = gs_default_val();
                        layout.type = GS_ASSET_MESH_ATTRIBUTE_TYPE_JOINT;
                        gs_dyn_array_push(layouts, layout);
                    } break;

                    case cgltf_attribute_type_weights:
                    {
                        // Push into layout
                        gs_asset_mesh_layout_t layout = gs_default_val();
                        layout.type = GS_ASSET_MESH_ATTRIBUTE_TYPE_WEIGHT;
                        gs_dyn_array_push(layouts, layout);
                    } break;

                    // Shouldn't hit here...   
                    default: 
                    {
                    } break;
                }
            }

            // Indices for primitive
            cgltf_accessor* acc = data->meshes[i].primitives[p].indices;

            #define __GLTF_PUSH_IDX(BB, ACC, TYPE)\
                do {\
                    int32_t n = 0;\
                    TYPE* buf = (TYPE*)acc->buffer_view->buffer->data + acc->buffer_view->offset/sizeof(TYPE) + acc->offset/sizeof(TYPE);\
                    gs_assert(buf);\
                    TYPE v = 0;\
                    /* For each index */\
                    for (uint32_t k = 0; k < acc->count; k++) {\
                        /* For each element */\
                        for (int l = 0; l < 1; l++) {\
                            v = buf[n + l];\
                        }\
                        n += (int32_t)(acc->stride/sizeof(TYPE));\
                        /* Add to temp positions array */\
                        switch (index_element_size) {\
                            case 0: gs_byte_buffer_write(BB, uint16_t, (uint16_t)v); break;\
                            case 2: gs_byte_buffer_write(BB, uint16_t, (uint16_t)v); break;\
                            case 4: gs_byte_buffer_write(BB, uint32_t, (uint32_t)v); break;\
                        }\
                    }\
                } while (0)

            // If indices are available
            if (acc) 
            {
                switch (acc->component_type) 
                {
                    case cgltf_component_type_r_8:   __GLTF_PUSH_IDX(&i_data, acc, int8_t);   break;
                    case cgltf_component_type_r_8u:  __GLTF_PUSH_IDX(&i_data, acc, uint8_t);  break;
                    case cgltf_component_type_r_16:  __GLTF_PUSH_IDX(&i_data, acc, int16_t);  break;
                    case cgltf_component_type_r_16u: __GLTF_PUSH_IDX(&i_data, acc, uint16_t); break;
                    case cgltf_component_type_r_32u: __GLTF_PUSH_IDX(&i_data, acc, uint32_t); break;
                    case cgltf_component_type_r_32f: __GLTF_PUSH_IDX(&i_data, acc, float);    break;

                    // Shouldn't hit here
                    default: {
                    } break;
                }
            }
            else 
            {
                // Iterate over positions size, then just push back indices
                for (uint32_t i = 0; i < gs_dyn_array_size(positions); ++i) 
                {
                    switch (index_element_size)
                    {
                        default:
                        case 0: gs_byte_buffer_write(&i_data, uint16_t, (uint16_t)i); break;
                        case 2: gs_byte_buffer_write(&i_data, uint16_t, (uint16_t)i); break;
                        case 4: gs_byte_buffer_write(&i_data, uint32_t, (uint32_t)i); break;
                    }
                }
            }

            bool warnings[gs_enum_count(gs_asset_mesh_attribute_type)] = gs_default_val();

            // Grab mesh layout pointer to use
            gs_asset_mesh_layout_t* layoutp = decl ? decl->layout : layouts;
            uint32_t layout_ct = decl ? decl->layout_size / sizeof(gs_asset_mesh_layout_t) : gs_dyn_array_size(layouts);

            // Iterate layout to fill data buffers according to provided layout
            {
                uint32_t vct = 0; 
                vct = gs_max(vct, gs_dyn_array_size(positions)); 
                vct = gs_max(vct, gs_dyn_array_size(colors)); 
                vct = gs_max(vct, gs_dyn_array_size(uvs));
                vct = gs_max(vct, gs_dyn_array_size(normals));
                vct = gs_max(vct, gs_dyn_array_size(tangents));

                #define __GLTF_WRITE_DATA(IT, VDATA, ARR, ARR_TYPE, ARR_DEF_VAL, LAYOUT_TYPE)\
                    do {\
                        /* Grab data at index, if available */\
                        if (IT < gs_dyn_array_size(ARR)) {\
                            gs_byte_buffer_write(&(VDATA), ARR_TYPE, ARR[IT]);\
                        }\
                        else {\
                            /* Write default value and give warning.*/\
                            gs_byte_buffer_write(&(VDATA), ARR_TYPE, ARR_DEF_VAL);\
                            if (!warnings[LAYOUT_TYPE]) {\
                                gs_println("Warning:Mesh:LoadFromFile:%s:Index out of range.", #LAYOUT_TYPE);\
                                warnings[LAYOUT_TYPE] = true;\
                            }\
                        }\
                    } while (0)

                for (uint32_t it = 0; it < vct; ++it)
                {
                    // For each attribute in layout
                    for (uint32_t l = 0; l < layout_ct; ++l)
                    {
                        switch (layoutp[l].type)
                        {
                            case GS_ASSET_MESH_ATTRIBUTE_TYPE_POSITION: {
                                __GLTF_WRITE_DATA(it, v_data, positions, gs_vec3, gs_v3(0.f, 0.f, 0.f), GS_ASSET_MESH_ATTRIBUTE_TYPE_POSITION); 
                            } break;

                            case GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD: {
                                __GLTF_WRITE_DATA(it, v_data, uvs, gs_vec2, gs_v2(0.f, 0.f), GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD); 
                            } break;

                            case GS_ASSET_MESH_ATTRIBUTE_TYPE_COLOR: {
                                __GLTF_WRITE_DATA(it, v_data, colors, gs_color_t, GS_COLOR_WHITE, GS_ASSET_MESH_ATTRIBUTE_TYPE_COLOR); 
                            } break;

                            case GS_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL: {
                                __GLTF_WRITE_DATA(it, v_data, normals, gs_vec3, gs_v3(0.f, 0.f, 1.f), GS_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL); 
                            } break;

                            case GS_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT: {
                                __GLTF_WRITE_DATA(it, v_data, tangents, gs_vec3, gs_v3(0.f, 1.f, 0.f), GS_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT); 
                            } break;

                            default:
                            {
                            } break;
                        }
                    }
                }
            }

            // Add to out data
            mesh->vertices[p] = gs_malloc(v_data.size);
            mesh->indices[p] = gs_malloc(i_data.size);
            mesh->vertex_sizes[p] = v_data.size;
            mesh->index_sizes[p] = i_data.size;

            // Copy data
            memcpy(mesh->vertices[p], v_data.data, v_data.size);
            memcpy(mesh->indices[p], i_data.data, i_data.size);
        }
    }

    // Free all data at the end
    cgltf_free(data);
    gs_dyn_array_free(positions);
    gs_dyn_array_free(normals);
    gs_dyn_array_free(tangents);
    gs_dyn_array_free(colors);
    gs_dyn_array_free(uvs);
    gs_dyn_array_free(layouts);
    gs_byte_buffer_free(&v_data);
    gs_byte_buffer_free(&i_data);
}

void gs_asset_mesh_load_from_file(const char* path, void* out, gs_asset_mesh_decl_t* decl, void* data_out, size_t data_size)
{
    // Cast mesh data to use
    gs_asset_mesh_t* mesh = (gs_asset_mesh_t*)out;

    // Mesh data to fill out
    uint32_t mesh_count = 0;
    gs_asset_mesh_raw_data_t* meshes = NULL;

    // Get file extension from path
    gs_transient_buffer(file_ext, 32);
    gs_platform_file_extension(file_ext, 32, path);

    // GLTF
    if (gs_string_compare_equal(file_ext, "gltf"))
    {
        gs_util_load_gltf_data_from_file(path, decl, &meshes, &mesh_count);
    }
    else 
    {
        gs_println("Warning:MeshLoadFromFile:File extension not supported: %s, file: %s", file_ext, path);
        return;
    }

    // For now, handle meshes with only single mesh count
    if (mesh_count != 1) {
        // Error
        // Free all the memory
        return;
    }

    // Process all mesh data, add meshes
    for (uint32_t i = 0; i < mesh_count; ++i)
    {
        gs_asset_mesh_raw_data_t* m = &meshes[i];

        for (uint32_t p = 0; p < m->prim_count; ++p)
        {
            // Construct primitive
            gs_asset_mesh_primitive_t prim = gs_default_val();
            prim.count = m->index_sizes[p] / sizeof(uint16_t);

            // Vertex buffer decl
            gs_graphics_buffer_desc_t vdesc = gs_default_val();
            vdesc.type = GS_GRAPHICS_BUFFER_VERTEX;
            vdesc.data = m->vertices[p];
            vdesc.size = m->vertex_sizes[p];

            // Construct vertex buffer for primitive
            prim.vbo = gs_graphics_buffer_create(&vdesc);

            // Index buffer decl
            gs_graphics_buffer_desc_t idesc = gs_default_val();
            idesc.type = GS_GRAPHICS_BUFFER_INDEX;
            idesc.data = m->indices[p];
            idesc.size = m->index_sizes[p];

            // Construct index buffer for primitive
            prim.ibo = gs_graphics_buffer_create(&idesc);

            // Add primitive to mesh
            gs_dyn_array_push(mesh->primitives, prim);
        } 
    }

    // Free all mesh data
}

/*=============================
// GS_ENGINE
=============================*/

gs_result gs_engine_run();
gs_result gs_engine_shutdown();
void gs_default_app_func();
void gs_default_main_window_close_callback(void* window);

// Global instance of gunslinger engine (...THERE CAN ONLY BE ONE)
gs_global gs_engine_t* __g_engine_instance = gs_default_val();

gs_engine_t* gs_engine_create(gs_app_desc_t app_desc)
{
    if (gs_engine_instance() == NULL)
    {
        // Check app desc for defaults
        if (app_desc.window_width == 0)     app_desc.window_width = 800;
        if (app_desc.window_height == 0)    app_desc.window_height = 600;
        if (app_desc.window_title == 0)     app_desc.window_title = "App";
        if (app_desc.frame_rate <= 0.f)     app_desc.frame_rate = 60.f;
        if (app_desc.update == NULL)        app_desc.update = &gs_default_app_func;
        if (app_desc.shutdown == NULL)      app_desc.shutdown = &gs_default_app_func;
        if (app_desc.init == NULL)          app_desc.init = &gs_default_app_func;

        // Construct instance
        __g_engine_instance = gs_malloc_init(gs_engine_t);

        // Set application description for engine
        gs_engine_instance()->ctx.app = app_desc;

        // Set up function pointers
        gs_engine_instance()->run       = &gs_engine_run;
        gs_engine_instance()->shutdown  = &gs_engine_shutdown;

        // Need to have video settings passed down from user
        gs_engine_subsystem(platform) = gs_platform_create();

        // Default initialization for platform here
        gs_platform_init(gs_engine_subsystem(platform));

        // Set frame rate for application
        gs_engine_subsystem(platform)->time.max_fps = app_desc.frame_rate;

        // Set vsync for video
        gs_platform_enable_vsync(app_desc.enable_vsync);

        // Construct main window
        gs_platform_create_window(app_desc.window_title, app_desc.window_width, app_desc.window_height);

        // Construct graphics api 
        gs_engine_subsystem(graphics) = gs_graphics_create();

        // Initialize graphics here
        gs_graphics_init(gs_engine_subsystem(graphics));

        // Construct audio api
        gs_engine_subsystem(audio) = gs_audio_create();

        // Initialize audio
        gs_audio_init(gs_engine_subsystem(audio));

        // Initialize application
        app_desc.init();

        gs_engine_ctx()->app.is_running = true;

        // Set default callback for when main window close button is pressed
        gs_platform_set_window_close_callback(gs_platform_main_window(), &gs_default_main_window_close_callback);
    }

    return gs_engine_instance();
}

gs_engine_t* gs_engine_instance()
{
    return __g_engine_instance;
}

gs_engine_context_t* gs_engine_ctx()
{
    return &gs_engine_instance()->ctx;
}

gs_result gs_engine_run()
{
    // Main engine loop
    while (true)
    {
        // TODO(john): Get rid of these...
        static uint32_t curr_ticks = 0; 
        static uint32_t prev_ticks = 0;

        // Cache platform pointer
        gs_platform_i* platform = gs_engine_subsystem(platform);

        // Cache times at start of frame
        platform->time.current  = gs_platform_elapsed_time();
        platform->time.update   = platform->time.current - platform->time.previous;
        platform->time.previous = platform->time.current;

        // Update platform and process input
        if (gs_platform_update(platform) != GS_RESULT_IN_PROGRESS)
        {
            return (gs_engine_instance()->shutdown());
        }

        // Process application context
        gs_engine_instance()->ctx.app.update();
        if (!gs_engine_instance()->ctx.app.is_running) 
        {
            // Shutdown engine and return
            return (gs_engine_instance()->shutdown());
        }

        // NOTE(John): This won't work forever. Must change eventually.
        // Swap all platform window buffers? Sure...
        for 
        (
            gs_slot_array_iter it = 0;
            gs_slot_array_iter_valid(platform->windows, it);
            gs_slot_array_iter_advance(platform->windows, it)
        )
        {
            gs_platform_window_swap_buffer(it);
        }

        // Frame locking
        platform->time.current  = gs_platform_elapsed_time();
        platform->time.render   = platform->time.current - platform->time.previous;
        platform->time.previous = platform->time.current;
        platform->time.frame    = platform->time.update + platform->time.render;            // Total frame time
        platform->time.delta    = platform->time.frame / 1000.f;

        float target = (1000.f / platform->time.max_fps);

        if (platform->time.frame < target)
        {
            gs_platform_sleep((float)(target - platform->time.frame));
            
            platform->time.current = gs_platform_elapsed_time();
            double wait_time = platform->time.current - platform->time.previous;
            platform->time.previous = platform->time.current;
            platform->time.frame += wait_time;
            platform->time.delta = platform->time.frame / 1000.f;
        }
    }

    // Shouldn't hit here
    gs_assert(false);
    return GS_RESULT_FAILURE;
}

gs_result gs_engine_shutdown()
{
    // Shutdown application
    gs_engine_ctx()->app.shutdown();

    // Shutdown subsystems
    gs_graphics_shutdown(gs_engine_subsystem(graphics));
    gs_graphics_destroy(gs_engine_subsystem(graphics));

    gs_audio_shutdown(gs_engine_subsystem(audio));
    gs_audio_destroy(gs_engine_subsystem(audio));

    gs_platform_shutdown(gs_engine_subsystem(platform)); 
    gs_platform_destroy(gs_engine_subsystem(platform));

    // Free engine
    gs_free(__g_engine_instance);
    __g_engine_instance = NULL;

    return GS_RESULT_SUCCESS;
}

void gs_default_app_func()
{
    // Nothing...
}

void gs_default_main_window_close_callback(void* window)
{
    gs_engine_instance()->ctx.app.is_running = false;
}

void gs_engine_quit()
{
    gs_engine_instance()->ctx.app.is_running = false;
}

/* Main entry point */
#ifndef GS_NO_HIJACK_MAIN

    int32_t main(int32_t argv, char** argc)
    {
        gs_engine_create(gs_main(argv, argc))->run();
        return 0;
    }

#endif // GS_NO_HIJACK_MAIN

#undef GS_IMPL
#endif // GS_IMPL

#ifdef __cplusplus
}
#endif // c++

#endif // __GS_INCLUDED_H__

/*
    Layout decl

    // Pipeline should have layout desc for vertices?
    // Or should it have layout for EACH buffer?
        
    non-interleaved vertex data
    have to be able to specific stride/offset for vertex layouts

    What are ways to interleave data?

    layout descriptor? 

    gs_vertex_attribute_type layouts[] = 
    {

    };

    // Need to codify strides/offsets/divisors

    // This can hold multiple layouts
    gs_vertex_layout_desc_t layout = 
    {
        .layouts = layouts, 
        .size = sizeof(layouts) 
    };

    Don't want to have to make user calculate strides, right?

    // If you don't provide manual stride/offset, then it'll calculate it for you based on layout?

    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec3 aColor;
    layout (location = 2) in vec2 aOffset;

    out vec3 fColor;

    void main()
    {
        gl_Position = vec4(aPos + aOffset, 0.0, 1.0);
        fColor = aColor;
    }

    typedef struct gs_vertex_attribute_layout_desc_t {
        gs_vertex_attribute_type format; 
        size_t stride;
        size_t offset;
        size_t divisor;
    } gs_vertex_attribute_layout_desc_t;

    gs_vertex_attribute_layout_desc_t layout[] = {
        {.format = GS_VERTEX_ATTRIBUTE_FLOAT2},
        {.format = GS_VERTEX_ATTRIBUTE_FLOAT3},
        {.format = GS_VERTEX_ATTRIBUTE_FLOAT2, .stride = 2 * sizeof(float), .offset = 0, .divisor = 1}
    };

    What about non-interleaved data? Almost would need to provide an offset.
    It's specific to the data itself, so have to provide manual offsets for data in binding data

    gs_graphics_bind_desc_t binds[] = {
        {.type = GS_GRAPHICS_BIND_VERTEX_BUFFER, .buffer = , .offset = };
    }

    gs_graphics_bind_desc_t binds[] = {
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_VERTEX_BUFFER, .buffer = vbo, .offset = ...},
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_VERTEX_BUFFER, .buffer = vbo, .offset = ...},
    };

    .layout = {
        .attributes = &(){
            {.format = GS_VERTEX_ATTRIBUTE_FLOAT2},
            {.format = GS_VERTEX_ATTRIBUTE_FLOAT2},
            {.format = GS_VERTEX_ATTRIBUTE_FLOAT4, .stride = 64, .offset =, .divisor = }
        } 
    };

    sg_pipeline pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        .layout = {
            .attrs = {
                [0].format=SG_VERTEXFORMAT_FLOAT3,
                [1].format=SG_VERTEXFORMAT_FLOAT4
            }
        }
    });

    .layout = {
        .attrs = attrs, 
        .attr_size = sizeof(attrs), 
        .strides = strides, 
        .stride_size = sizeof(strides),

    }

    sg_pipeline pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            .buffers = {
                [0] = { .stride = 28 },
                [1] = { .stride = 12, .step_func=SG_VERTEXSTEP_PER_INSTANCE }
            },
            .attrs = {
                [0] = { .offset = 0,  .format=SG_VERTEXFORMAT_FLOAT3, .buffer_index=0 },
                [1] = { .offset = 12, .format=SG_VERTEXFORMAT_FLOAT4, .buffer_index=0 },
                [2] = { .offset = 0,  .format=SG_VERTEXFORMAT_FLOAT3, .buffer_index=1 }
            }
        },
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16,
        .depth_stencil = {
            .depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL,
            .depth_write_enabled = true
        },
        .rasterizer.cull_mode = SG_CULLMODE_BACK
    });

    float quadVertices[] = {
        // positions     // colors
        -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
         0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
        -0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

        -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
         0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
         0.05f,  0.05f,  0.0f, 1.0f, 1.0f
    };

    // also set instance data
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from a different vertex buffer
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.

/*
    gltf loading

    Mesh Attributes:
        cgltf_attribute_type_invalid,
        cgltf_attribute_type_position,
        cgltf_attribute_type_normal,
        cgltf_attribute_type_tangent,
        cgltf_attribute_type_texcoord,
        cgltf_attribute_type_color,
        cgltf_attribute_type_joints,
        cgltf_attribute_type_weights,

    Primitive types:
        cgltf_primitive_type_points,
        cgltf_primitive_type_lines,
        cgltf_primitive_type_line_loop,
        cgltf_primitive_type_line_strip,
        cgltf_primitive_type_triangles,
        cgltf_primitive_type_triangle_strip,
        cgltf_primitive_type_triangle_fan,

    For each mesh: 
        For each primitive: 
            For each attribute:
                Get data and push into mesh definition

    Is there a way to have the user be able to specify a layout and then use that for the mesh?

    gs_enum_decl(gs_asset_mesh_attribute_type,
        GS_ASSET_MESH_ATTRIBUTE_TYPE_POSITION,
        GS_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL,
        GS_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT,
        GS_ASSET_MESH_ATTRIBUTE_TYPE_JOINT,
        GS_ASSET_MESH_ATTRIBUTE_TYPE_WEIGHT,
        GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD,
        GS_ASSET_MESH_ATTRIBUTE_TYPE_COLOR
    });

    typedef struct gs_asset_mesh_layout_t {
        gs_asset_mesh_attribute_type type;     // Type of attribute
        uint32_t idx;                          // Optional index (for joint/weight/texcoord/color)
    } gs_asset_mesh_layout_t;

    typedef struct gs_asset_mesh_decl_t
    {
        gs_asset_mesh_layout_t* layout;        // Mesh attribute layout array
        size_t layout_size;                    // Size of mesh attribute layout array in bytes
    } gs_asset_mesh_decl_t;

    // Mesh holds...what?
    // A pipeline? Shouldn't have to.
    // Material? Nope.
    // It's just mesh data. (so an index/vertex buffer)

    typedef struct gs_asset_mesh_t
    {
        gs_handle(gs_graphics_buffer_t) vbo;
        gs_handle(gs_graphics_buffer_t) ibo;
    } gs_asset_mesh_t;

    void gs_asset_mesh_load_from_file(const char* path, void* out, gs_asset_mesh_decl_t* decl, void* data_out, size_t data_size)
    {
        gs_asset_mesh_t* mesh = (gs_asset_mesh_t*)out;

        // Parse gltf data
    }

    Does this need to line up with a pipeline? Not necessarily, right?

    // At LEAST position is required to be passed in for the layout, so maybe it's not necessary 
    // to provide this in the layout?
    // Can you duplicate? I don't think so...
    gs_asset_mesh_attribute_type layout[] =
    {
        POSITION,
        NORMAL,
        TANGENT,
        JOINTS_XXX,
        WEIGHTS_XXX,
        TEXCOORD_XXX,
        COLOR_XXX

    };

    gs_asset_mesh_t mesh = gs_asset_load_gltf(path, layout, sizeof(layout));

    // Do you HAVE to have certain attributes for a mesh to make any sense? For instance, do you HAVE to have position?
    // What if position is NOT the first attribute layout for your vertex attribute?

    // Need to fill out data for each attribute, then interleave?

    ^()
    {
        For each mesh: 
            For each primitive: 
                For each attribute:
                    Get data and push into mesh definition 
    }
*/