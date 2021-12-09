
/*================================================================
    * Copyright: 2020 John Jackson 
    * File: gs_platform_impl.h
    All Rights Reserved
=================================================================*/

#ifndef GS_PLATFORM_IMPL_H
#define GS_PLATFORM_IMPL_H

/*=================================
// Default Platform Implemenattion
=================================*/

// Define default platform implementation if certain platforms are enabled
#if (!defined GS_PLATFORM_IMPL_NO_DEFAULT)
    #define GS_PLATFORM_IMPL_DEFAULT
#endif

/*=============================
// Default Impl
=============================*/

#ifdef GS_PLATFORM_IMPL_DEFAULT

#if !( defined GS_PLATFORM_WIN )
    #include <sys/stat.h>
    #include <dirent.h>
#else
	#include "../external/dirent/dirent.h"
#endif

/*== Platform Window ==*/

gs_platform_t* gs_platform_create()
{
    // Construct new platform interface
    gs_platform_t* platform = gs_malloc_init(gs_platform_t);

    // Initialize windows
    platform->windows = gs_slot_array_new(void*);

    // Set up video mode (for now, just do opengl)
    platform->settings.video.driver = GS_PLATFORM_VIDEO_DRIVER_TYPE_OPENGL;

    return platform;
}

void gs_platform_destroy(gs_platform_t* platform)
{
    if (platform == NULL) return;

    // Free all resources
    gs_slot_array_free(platform->windows);

    // Free platform
    gs_free(platform);
    platform = NULL;
}

uint32_t gs_platform_create_window(const char* title, uint32_t width, uint32_t height, uint32_t monitor_index)
{
    gs_assert(gs_engine_instance() != NULL);
    gs_platform_t* platform = gs_engine_subsystem(platform);
    void* win = gs_platform_create_window_internal(title, width, height, monitor_index);
    return (gs_slot_array_insert(platform->windows, win));
}

uint32_t gs_platform_main_window()
{
    // Should be the first element of the slot array...Great assumption to make.
    return 0;
}

/*== Platform Time ==*/

float gs_platform_delta_time()
{
    return (float)gs_engine_subsystem(platform)->time.delta;
}

/*== Platform UUID ==*/

struct gs_uuid_t gs_platform_uuid_generate()
{
    gs_uuid_t uuid;

    srand(clock());
    char guid[40];
    int32_t t = 0;
    const char* sz_temp = "xxxxxxxxxxxx4xxxyxxxxxxxxxxxxxxx";
    const char* sz_hex = "0123456789abcdef-";
    int32_t n_len = (int32_t)strlen(sz_temp);

    for (t=0; t < n_len + 1; t++)
    {
        int32_t r = rand () % 16;
        char c = ' ';   

        switch (sz_temp[t])
        {
            case 'x' : { c = sz_hex [r]; } break;
            case 'y' : { c = sz_hex [(r & 0x03) | 0x08]; } break;
            case '-' : { c = '-'; } break;
            case '4' : { c = '4'; } break;
        }

        guid[t] = (t < n_len) ? c : 0x00;
    }

    // Convert to uuid bytes from string
    const char* hex_string = guid, *pos = hex_string;

     /* WARNING: no sanitization or error-checking whatsoever */
    for (size_t count = 0; count < 16; count++) 
    {
        sscanf(pos, "%2hhx", &uuid.bytes[count]);
        pos += 2;
    }

    return uuid;
}

// Mutable temp buffer 'tmp_buffer'
void gs_platform_uuid_to_string(char* tmp_buffer, const gs_uuid_t* uuid)
{
    gs_snprintf 
    (
        tmp_buffer, 
        32,
        "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
        uuid->bytes[0],
        uuid->bytes[1],
        uuid->bytes[2],
        uuid->bytes[3],
        uuid->bytes[4],
        uuid->bytes[5],
        uuid->bytes[6],
        uuid->bytes[7],
        uuid->bytes[8],
        uuid->bytes[9],
        uuid->bytes[10],
        uuid->bytes[11],
        uuid->bytes[12],
        uuid->bytes[13],
        uuid->bytes[14],
        uuid->bytes[15]
    );
}

uint32_t gs_platform_uuid_hash(const gs_uuid_t* uuid)
{
    char temp_buffer[] = gs_uuid_temp_str_buffer();
    gs_platform_uuid_to_string(temp_buffer, uuid);
    return (gs_hash_str(temp_buffer));
}

#define __gs_input()\
    (&gs_engine_subsystem(platform)->input)

/*=== Platform Input ===*/
void gs_platform_update_input(gs_platform_input_t* input)
{
    // Update all input and mouse keys from previous frame
    // Previous key presses
    gs_for_range_i(GS_KEYCODE_COUNT) {
        input->prev_key_map[i] = input->key_map[i];
    }

    // Previous mouse button presses
    gs_for_range_i(GS_MOUSE_BUTTON_CODE_COUNT) {
        input->mouse.prev_button_map[i] = input->mouse.button_map[i];
    }

    input->mouse.wheel = gs_v2s(0.0f);
    input->mouse.delta = gs_v2s(0.f);
    input->mouse.moved_this_frame = false;

    // Update all touch deltas
    for (uint32_t i = 0; i < GS_PLATFORM_MAX_TOUCH; ++i) {
        input->touch.points[i].delta = gs_v2s(0.f);
        input->touch.points[i].down = input->touch.points[i].pressed;
    }
}

void gs_platform_poll_all_events()
{
    gs_platform_t* platform = gs_engine_subsystem(platform);
   
   platform->input.mouse.delta.x = 0;
   platform->input.mouse.delta.y = 0;

    // Iterate through events, don't consume
    gs_platform_event_t evt = gs_default_val();
    while (gs_platform_poll_events(&evt, false))
    {
        switch (evt.type)
        {
            case GS_PLATFORM_EVENT_MOUSE:
            {
                switch (evt.mouse.action)
                {
                    case GS_PLATFORM_MOUSE_MOVE:
                    {
                        // If locked, then movement amount will be applied to delta, 
                        // otherwise set position
                        if (gs_platform_mouse_locked()) {
                            platform->input.mouse.delta = gs_vec2_add(platform->input.mouse.delta, evt.mouse.move);
                        } else {
                            platform->input.mouse.delta = gs_vec2_sub(evt.mouse.move, platform->input.mouse.position);
                            platform->input.mouse.position = evt.mouse.move;
                        }
                    } break;

                    case GS_PLATFORM_MOUSE_WHEEL:
                    {
                        platform->input.mouse.wheel = evt.mouse.wheel;
                    } break;

                    case GS_PLATFORM_MOUSE_BUTTON_PRESSED:
                    {
                        gs_platform_press_mouse_button(evt.mouse.button);
                    } break;

                    case GS_PLATFORM_MOUSE_BUTTON_RELEASED:
                    {
                        gs_platform_release_mouse_button(evt.mouse.button);
                    } break;

                    case GS_PLATFORM_MOUSE_BUTTON_DOWN:
                    {
                        gs_platform_press_mouse_button(evt.mouse.button);
                    } break;

                    case GS_PLATFORM_MOUSE_ENTER:
                    {
                        // If there are user callbacks, could trigger them here
                    } break;

                    case GS_PLATFORM_MOUSE_LEAVE:
                    {
                        // If there are user callbacks, could trigger them here
                    } break;
                }

            } break;

            case GS_PLATFORM_EVENT_KEY:
            {
                switch (evt.key.action) 
                {
                    case GS_PLATFORM_KEY_PRESSED:
                    {
                        gs_platform_press_key(evt.key.keycode);
                    } break;

                    case GS_PLATFORM_KEY_DOWN:
                    {
                        gs_platform_press_key(evt.key.keycode);
                    } break;

                    case GS_PLATFORM_KEY_RELEASED:
                    {
                        gs_platform_release_key(evt.key.keycode);
                    } break;
                }

            } break;

            case GS_PLATFORM_EVENT_WINDOW:
            {
                switch (evt.window.action)
                {
                    default: break;
                }

            } break;

            case GS_PLATFORM_EVENT_TOUCH:
            {
                gs_platform_point_event_data_t* point = &evt.touch.point;

                switch (evt.touch.action)
                {
                    case GS_PLATFORM_TOUCH_DOWN:
                    {
                        uintptr_t id = point->id;
                        gs_vec2 *pos = &point->position;
                        gs_vec2 *p = &platform->input.touch.points[id].position;
                        gs_vec2 *d = &platform->input.touch.points[id].delta;
                        gs_platform_press_touch(id);
                        *p = *pos;
                        gs_engine_subsystem(platform)->input.touch.size++;
                    } break;

                    case GS_PLATFORM_TOUCH_UP:
                    {
                        uintptr_t id = point->id;
                        gs_println("Releasing ID: %zu", id);
                        gs_platform_release_touch(id);
                        gs_engine_subsystem(platform)->input.touch.size--;
                    } break;

                    case GS_PLATFORM_TOUCH_MOVE:
                    {
                        uintptr_t id = point->id;
                        gs_vec2* pos = &point->position;
                        gs_vec2* p = &platform->input.touch.points[id].position;
                        gs_vec2* d = &platform->input.touch.points[id].delta;
                        gs_platform_press_touch(id);  // Not sure if this is causing issues...
                        *d = gs_vec2_sub(*pos, *p);
                        *p = *pos;
                    } break;

                    case GS_PLATFORM_TOUCH_CANCEL:
                    {
                        uintptr_t id = point->id;
                        gs_platform_release_touch(id);
                        gs_engine_subsystem(platform)->input.touch.size--;
                    } break;
                }
            } break;

            default: break;
        } 
    }
}

void gs_platform_update(gs_platform_t* platform)
{
    // Update platform input from previous frame        
    gs_platform_update_input(&platform->input);

    // Process input for this frame (user dependent update)
    gs_platform_process_input(&platform->input);

    // Poll all events
    gs_platform_poll_all_events();
}

bool gs_platform_poll_events(gs_platform_event_t* evt, bool32_t consume)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);

    if (!evt) return false;
    if (gs_dyn_array_empty(platform->events)) return false;
    if (evt->idx >= gs_dyn_array_size(platform->events)) return false;

    if (consume) {
        // Back event
        *evt = gs_dyn_array_back(platform->events);
        // Pop back
        gs_dyn_array_pop(platform->events);
    }
    else {
        uint32_t idx = evt->idx;
        *evt = platform->events[idx++]; 
        evt->idx = idx;
    }

    return true;
}

void gs_platform_add_event(gs_platform_event_t* evt)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);
    if (!evt) return;
    gs_dyn_array_push(platform->events, *evt);
}

bool gs_platform_was_key_down(gs_platform_keycode code)
{
    gs_platform_input_t* input = __gs_input();
    return (input->prev_key_map[code]);
}

bool gs_platform_key_down(gs_platform_keycode code)
{
    gs_platform_input_t* input = __gs_input();
    return (input->key_map[code]);
}

bool gs_platform_key_pressed(gs_platform_keycode code)
{
    gs_platform_input_t* input = __gs_input();
    return (gs_platform_key_down(code) && !gs_platform_was_key_down(code));
}

bool gs_platform_key_released(gs_platform_keycode code)
{
    gs_platform_input_t* input = __gs_input();
    return (gs_platform_was_key_down(code) && !gs_platform_key_down(code));
}

bool gs_platform_touch_down(uint32_t idx)
{
    gs_platform_input_t* input = __gs_input();
    if (idx < GS_PLATFORM_MAX_TOUCH) {
        return input->touch.points[idx].pressed;
    }
    return false;
}

bool gs_platform_touch_pressed(uint32_t idx)
{
    gs_platform_input_t* input = __gs_input();
    if (idx < GS_PLATFORM_MAX_TOUCH) {
        return (gs_platform_was_touch_down(idx) && !gs_platform_touch_down(idx));
    }
    return false;
}

bool gs_platform_touch_released(uint32_t idx)
{
    gs_platform_input_t* input = __gs_input();
    if (idx < GS_PLATFORM_MAX_TOUCH) {
        return (gs_platform_was_touch_down(idx) && !gs_platform_touch_down(idx));
    }
    return false;
}

bool gs_platform_was_mouse_down(gs_platform_mouse_button_code code)
{
    gs_platform_input_t* input = __gs_input();
    return (input->mouse.prev_button_map[code]);
}

void gs_platform_press_mouse_button(gs_platform_mouse_button_code code)
{
    gs_platform_input_t* input = __gs_input();
    if ((u32)code < (u32)GS_MOUSE_BUTTON_CODE_COUNT) 
    {
        input->mouse.button_map[code] = true;
    }
}

void gs_platform_release_mouse_button(gs_platform_mouse_button_code code)
{
    gs_platform_input_t* input = __gs_input();
    if ((u32)code < (u32)GS_MOUSE_BUTTON_CODE_COUNT) 
    {
        input->mouse.button_map[code] = false;
    }
}

bool gs_platform_mouse_down(gs_platform_mouse_button_code code)
{
    gs_platform_input_t* input = __gs_input();
    return (input->mouse.button_map[code]);
}

bool gs_platform_mouse_pressed(gs_platform_mouse_button_code code)
{
    gs_platform_input_t* input = __gs_input();
    if (gs_platform_mouse_down(code) && !gs_platform_was_mouse_down(code))
    {
        return true;
    }
    return false;
}

bool gs_platform_mouse_released(gs_platform_mouse_button_code code)
{
    gs_platform_input_t* input = __gs_input();
    return (gs_platform_was_mouse_down(code) && !gs_platform_mouse_down(code));
}

void gs_platform_mouse_delta(float* x, float* y)
{
    gs_platform_input_t* input = __gs_input();
    *x = input->mouse.delta.x;
    *y = input->mouse.delta.y;
}

gs_vec2 gs_platform_mouse_deltav()
{
    gs_platform_input_t* input = __gs_input();
    gs_vec2 delta = gs_default_val();
    gs_platform_mouse_delta(&delta.x, &delta.y);
    return delta;
}

gs_vec2 gs_platform_mouse_positionv()
{
    gs_platform_input_t* input = __gs_input();

    return gs_v2( 
        input->mouse.position.x, 
        input->mouse.position.y
    );
}

void gs_platform_mouse_position(int32_t* x, int32_t* y)
{
    gs_platform_input_t* input = __gs_input();
    *x = (int32_t)input->mouse.position.x;
    *y = (int32_t)input->mouse.position.y;
}

void gs_platform_mouse_wheel(f32* x, f32* y)
{
    gs_platform_input_t* input = __gs_input();
    *x = input->mouse.wheel.x;
    *y = input->mouse.wheel.y;  
}

bool gs_platform_mouse_locked()
{
    return (__gs_input())->mouse.locked;
}

void gs_platform_touch_delta(uint32_t idx, float* x, float* y)
{
    gs_platform_input_t* input = __gs_input();
    if (idx < GS_PLATFORM_MAX_TOUCH) {
        *x = input->touch.points[idx].delta.x;
        *y = input->touch.points[idx].delta.y;
    }
}

gs_vec2 gs_platform_touch_deltav(uint32_t idx)
{
    gs_vec2 delta = gs_v2s(0.f);
    gs_platform_touch_delta(idx, &delta.x, &delta.y);
    return delta;
}

void gs_platform_touch_position(uint32_t idx, float* x, float* y)
{
    gs_platform_input_t* input = __gs_input();
    if (idx < GS_PLATFORM_MAX_TOUCH) {
        *x = input->touch.points[idx].position.x;
        *y = input->touch.points[idx].position.y;
    }
}

gs_vec2 gs_platform_touch_positionv(uint32_t idx)
{
    gs_vec2 p = gs_default_val();
    gs_platform_touch_position(idx, &p.x, &p.y);
    return p;
}

void gs_platform_press_touch(uint32_t idx)
{
    gs_platform_input_t* input = __gs_input();
    if (idx < GS_PLATFORM_MAX_TOUCH) {
        input->touch.points[idx].pressed = true;
    }
}

void gs_platform_release_touch(uint32_t idx)
{
    gs_platform_input_t* input = __gs_input();
    if (idx < GS_PLATFORM_MAX_TOUCH) {
        gs_println("releasing: %zu", idx);
        input->touch.points[idx].pressed = false;
    }
}

bool gs_platform_was_touch_down(uint32_t idx)
{
    gs_platform_input_t* input = __gs_input();
    if (idx < GS_PLATFORM_MAX_TOUCH) {
        return input->touch.points[idx].down;
    }
    return false;
}

void gs_platform_press_key(gs_platform_keycode code)
{
    gs_platform_input_t* input = __gs_input();
    if (code < GS_KEYCODE_COUNT) {
        input->key_map[code] = true;
    }
}

void gs_platform_release_key(gs_platform_keycode code)
{
    gs_platform_input_t* input = __gs_input();
    if (code < GS_KEYCODE_COUNT) {
        input->key_map[code] = false;
    }
}

// Platform File IO
char* gs_platform_read_file_contents_default_impl(const char* file_path, const char* mode, size_t* sz)
{
    const char* path = file_path;

    #ifdef GS_PLATFORM_ANDROID
        const char* internal_data_path = gs_engine_app()->android.internal_data_path;
        gs_snprintfc(tmp_path, 1024, "%s/%s", internal_data_path, file_path);
        path = tmp_path;
    #endif

    char* buffer = 0;
    FILE* fp = fopen(path, mode);
    size_t read_sz = 0;
    if (fp)
    {
        read_sz = gs_platform_file_size_in_bytes(file_path);
        buffer = (char*)gs_malloc(read_sz + 1);
        if (buffer) {
           size_t _r = fread(buffer, 1, read_sz, fp);
        }
        buffer[read_sz] = '\0';
        fclose(fp);
        if (sz) *sz = read_sz;
    }

    return buffer;
}

gs_result gs_platform_write_file_contents_default_impl(const char* file_path, const char* mode, void* data, size_t sz)
{
    const char* path = file_path;

    #ifdef GS_PLATFORM_ANDROID
        const char* internal_data_path = gs_engine_app()->android.internal_data_path;
        gs_snprintfc(tmp_path, 1024, "%s/%s", internal_data_path, file_path);
        path = tmp_path;
    #endif

    FILE* fp = fopen(path, mode);
    if (fp) 
    {
        size_t ret = fwrite(data, sizeof(uint8_t), sz, fp);
        if (ret == sz)
        {
            fclose(fp);
            return GS_RESULT_SUCCESS;
        }
        fclose(fp);
    }
    return GS_RESULT_FAILURE;
}

GS_API_DECL bool gs_platform_dir_exists_default_impl(const char* dir_path)
{
	DIR* dir = opendir(dir_path);
	if (dir)
	{
		closedir(dir);
		return true;
	}
	return false;
}

bool gs_platform_file_exists_default_impl(const char* file_path)
{
    const char* path = file_path;

    #ifdef GS_PLATFORM_ANDROID
        const char* internal_data_path = gs_engine_app()->android.internal_data_path;
        gs_snprintfc(tmp_path, 1024, "%s/%s", internal_data_path, file_path);
        path = tmp_path;
    #endif

    gs_println("Checking: %s", path);
    FILE* fp = fopen(path, "r");
    if (fp) {
        fclose(fp);
        return true;
    }
    return false;
}

int32_t gs_platform_file_size_in_bytes_default_impl(const char* file_path)
{
    #ifdef GS_PLATFORM_WIN

        HANDLE hFile = CreateFile(file_path, GENERIC_READ, 
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile==INVALID_HANDLE_VALUE)
            return -1; // error condition, could call GetLastError to find out more

        LARGE_INTEGER size;
        if (!GetFileSizeEx(hFile, &size))
        {
            CloseHandle(hFile);
            return -1; // error condition, could call GetLastError to find out more
        }

        CloseHandle(hFile);
        return gs_util_safe_truncate_u64(size.QuadPart);

    #elif (defined GS_PLATFORM_ANDROID)

        const char* internal_data_path = gs_engine_app()->android.internal_data_path;
        gs_snprintfc(tmp_path, 1024, "%s/%s", internal_data_path, file_path);
        struct stat st;
        stat(tmp_path, &st);
        return (int32_t)st.st_size;

    #else

        struct stat st;
        stat(file_path, &st);
        return (int32_t)st.st_size; 

    #endif
}

void gs_platform_file_extension_default_impl(char* buffer, size_t buffer_sz, const char* file_path)
{
    gs_util_get_file_extension(buffer, buffer_sz, file_path);
}

#undef GS_PLATFORM_IMPL_DEFAULT
#endif // GS_PLATFORM_IMPL_DEFAULT

/*======================
// GLFW Implemenation
======================*/

#ifdef GS_PLATFORM_IMPL_GLFW

#define GLAD_IMPL
#include "../external/glad/glad_impl.h"

#define GLFW_IMPL
#include "../external/glfw/glfw_impl.h"

#if (defined GS_PLATFORM_APPLE || defined GS_PLATFORM_LINUX)

    #include <sched.h>
    #include <unistd.h>

#elif (defined GS_PLATFORM_WINDOWS)

    #include <windows.h>

#endif

// Forward Decls.
void __glfw_key_callback(GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mods);
void __glfw_char_callback(GLFWwindow* window, uint32_t codepoint);
void __glfw_mouse_button_callback(GLFWwindow* window, s32 button, s32 action, s32 mods);
void __glfw_mouse_cursor_position_callback(GLFWwindow* window, f64 x, f64 y);
void __glfw_mouse_scroll_wheel_callback(GLFWwindow* window, f64 xoffset, f64 yoffset);
void __glfw_mouse_cursor_enter_callback(GLFWwindow* window, s32 entered);
void __glfw_frame_buffer_size_callback(GLFWwindow* window, s32 width, s32 height);
void __glfw_drop_callback(GLFWwindow* window);

#define __glfw_window_from_handle(platform, handle)\
    ((GLFWwindow*)(gs_slot_array_get((platform)->windows, (handle))))

/*== Platform Init / Shutdown == */

void gs_platform_init(gs_platform_t* pf)
{
    gs_assert(pf);

    gs_println("Initializing GLFW");
    glfwInit();

    switch (pf->settings.video.driver)
    {
        case GS_PLATFORM_VIDEO_DRIVER_TYPE_OPENGL:
        {
            #if (defined GS_PLATFORM_APPLE)
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
            #else
                glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
                // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, platform->settings.video.graphics.opengl.major_version);
                // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, platform->settings.video.graphics.opengl.minor_version);
                // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
                // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
                // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                if (pf->settings.video.graphics.debug)
                {
                    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
                }
            #endif
            // glfwSwapInterval(platform->settings.video.vsync_enabled);
            // glfwSwapInterval(0);
        } break;

        default:
        {
            // Default to no output at all.
            gs_println("Video format not supported.");
            gs_assert(false);
        } break;
    }

    // Construct cursors
    pf->cursors[(u32)GS_PLATFORM_CURSOR_ARROW]      = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    pf->cursors[(u32)GS_PLATFORM_CURSOR_IBEAM]      = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    pf->cursors[(u32)GS_PLATFORM_CURSOR_SIZE_NW_SE] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    pf->cursors[(u32)GS_PLATFORM_CURSOR_SIZE_NE_SW] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    pf->cursors[(u32)GS_PLATFORM_CURSOR_SIZE_NS]    = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    pf->cursors[(u32)GS_PLATFORM_CURSOR_SIZE_WE]    = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    pf->cursors[(u32)GS_PLATFORM_CURSOR_SIZE_ALL]   = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    pf->cursors[(u32)GS_PLATFORM_CURSOR_HAND]       = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    pf->cursors[(u32)GS_PLATFORM_CURSOR_NO]         = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
}

void gs_platform_shutdown(gs_platform_t* pf)
{
    // Free all windows in glfw
    // TODO(john): Figure out crash with glfwDestroyWindow && glfwTerminate
    for
    (
        gs_slot_array_iter it = 0;
        gs_slot_array_iter_valid(pf->windows, it);
        gs_slot_array_iter_advance(pf->windows, it)
    )
    {
        GLFWwindow* win =  __glfw_window_from_handle(pf, it);
        // glfwDestroyWindow(win);
    }

    // glfwTerminate();
}

uint32_t gs_platform_key_to_codepoint(gs_platform_keycode key)
{
    uint32_t code = 0;
    switch (key)
    {
        default:
        case GS_KEYCODE_COUNT:
        case GS_KEYCODE_INVALID:          code = 0; break;
        case GS_KEYCODE_SPACE:            code = 32; break;
        case GS_KEYCODE_APOSTROPHE:       code = 39; break;
        case GS_KEYCODE_COMMA:            code = 44; break;
        case GS_KEYCODE_MINUS:            code = 45; break;
        case GS_KEYCODE_PERIOD:           code = 46; break;
        case GS_KEYCODE_SLASH:            code = 47; break;
        case GS_KEYCODE_0:                code = 48; break;
        case GS_KEYCODE_1:                code = 49; break;
        case GS_KEYCODE_2:                code = 50; break;
        case GS_KEYCODE_3:                code = 51; break;
        case GS_KEYCODE_4:                code = 52; break;
        case GS_KEYCODE_5:                code = 53; break;
        case GS_KEYCODE_6:                code = 54; break;
        case GS_KEYCODE_7:                code = 55; break;
        case GS_KEYCODE_8:                code = 56; break;
        case GS_KEYCODE_9:                code = 57; break;
        case GS_KEYCODE_SEMICOLON:        code = 59; break;  /* ; */
        case GS_KEYCODE_EQUAL:            code = 61; break;  /* code = */
        case GS_KEYCODE_A:                code = 65; break;
        case GS_KEYCODE_B:                code = 66; break;
        case GS_KEYCODE_C:                code = 67; break;
        case GS_KEYCODE_D:                code = 68; break;
        case GS_KEYCODE_E:                code = 69; break;
        case GS_KEYCODE_F:                code = 70; break;
        case GS_KEYCODE_G:                code = 71; break;
        case GS_KEYCODE_H:                code = 72; break;
        case GS_KEYCODE_I:                code = 73; break;
        case GS_KEYCODE_J:                code = 74; break;
        case GS_KEYCODE_K:                code = 75; break;
        case GS_KEYCODE_L:                code = 76; break;
        case GS_KEYCODE_M:                code = 77; break;
        case GS_KEYCODE_N:                code = 78; break;
        case GS_KEYCODE_O:                code = 79; break;
        case GS_KEYCODE_P:                code = 80; break;
        case GS_KEYCODE_Q:                code = 81; break;
        case GS_KEYCODE_R:                code = 82; break;
        case GS_KEYCODE_S:                code = 83; break;
        case GS_KEYCODE_T:                code = 84; break;
        case GS_KEYCODE_U:                code = 85; break;
        case GS_KEYCODE_V:                code = 86; break;
        case GS_KEYCODE_W:                code = 87; break;
        case GS_KEYCODE_X:                code = 88; break;
        case GS_KEYCODE_Y:                code = 89; break;
        case GS_KEYCODE_Z:                code = 90; break;
        case GS_KEYCODE_LEFT_BRACKET:     code = 91; break;  /* [ */
        case GS_KEYCODE_BACKSLASH:        code = 92; break;  /* \ */
        case GS_KEYCODE_RIGHT_BRACKET:    code = 93; break;  /* ] */
        case GS_KEYCODE_GRAVE_ACCENT:     code = 96; break;  /* ` */
        case GS_KEYCODE_WORLD_1:          code = 161; break; /* non-US #1 */
        case GS_KEYCODE_WORLD_2:          code = 162; break; /* non-US #2 */
        case GS_KEYCODE_ESC:              code = 256; break;
        case GS_KEYCODE_ENTER:            code = 257; break;
        case GS_KEYCODE_TAB:              code = 258; break;
        case GS_KEYCODE_BACKSPACE:        code = 259; break;
        case GS_KEYCODE_INSERT:           code = 260; break;
        case GS_KEYCODE_DELETE:           code = GLFW_KEY_DELETE; break;
        case GS_KEYCODE_RIGHT:            code = 262; break;
        case GS_KEYCODE_LEFT:             code = 263; break;
        case GS_KEYCODE_DOWN:             code = 264; break;
        case GS_KEYCODE_UP:               code = 265; break;
        case GS_KEYCODE_PAGE_UP:          code = 266; break;
        case GS_KEYCODE_PAGE_DOWN:        code = 267; break;
        case GS_KEYCODE_HOME:             code = 268; break;
        case GS_KEYCODE_END:              code = 269; break;
        case GS_KEYCODE_CAPS_LOCK:        code = 280; break;
        case GS_KEYCODE_SCROLL_LOCK:      code = 281; break;
        case GS_KEYCODE_NUM_LOCK:         code = 282; break;
        case GS_KEYCODE_PRINT_SCREEN:     code = 283; break;
        case GS_KEYCODE_PAUSE:            code = 284; break;
        case GS_KEYCODE_F1:               code = 290; break;
        case GS_KEYCODE_F2:               code = 291; break;
        case GS_KEYCODE_F3:               code = 292; break;
        case GS_KEYCODE_F4:               code = 293; break;
        case GS_KEYCODE_F5:               code = 294; break;
        case GS_KEYCODE_F6:               code = 295; break;
        case GS_KEYCODE_F7:               code = 296; break;
        case GS_KEYCODE_F8:               code = 297; break;
        case GS_KEYCODE_F9:               code = 298; break;
        case GS_KEYCODE_F10:              code = 299; break;
        case GS_KEYCODE_F11:              code = 300; break;
        case GS_KEYCODE_F12:              code = 301; break;
        case GS_KEYCODE_F13:              code = 302; break;
        case GS_KEYCODE_F14:              code = 303; break;
        case GS_KEYCODE_F15:              code = 304; break;
        case GS_KEYCODE_F16:              code = 305; break;
        case GS_KEYCODE_F17:              code = 306; break;
        case GS_KEYCODE_F18:              code = 307; break;
        case GS_KEYCODE_F19:              code = 308; break;
        case GS_KEYCODE_F20:              code = 309; break;
        case GS_KEYCODE_F21:              code = 310; break;
        case GS_KEYCODE_F22:              code = 311; break;
        case GS_KEYCODE_F23:              code = 312; break;
        case GS_KEYCODE_F24:              code = 313; break;
        case GS_KEYCODE_F25:              code = 314; break;
        case GS_KEYCODE_KP_0:             code = 320; break;
        case GS_KEYCODE_KP_1:             code = 321; break;
        case GS_KEYCODE_KP_2:             code = 322; break;
        case GS_KEYCODE_KP_3:             code = 323; break;
        case GS_KEYCODE_KP_4:             code = 324; break;
        case GS_KEYCODE_KP_5:             code = 325; break;
        case GS_KEYCODE_KP_6:             code = 326; break;
        case GS_KEYCODE_KP_7:             code = 327; break;
        case GS_KEYCODE_KP_8:             code = 328; break;
        case GS_KEYCODE_KP_9:             code = 329; break;
        case GS_KEYCODE_KP_DECIMAL:       code = 330; break;
        case GS_KEYCODE_KP_DIVIDE:        code = 331; break;
        case GS_KEYCODE_KP_MULTIPLY:      code = 332; break;
        case GS_KEYCODE_KP_SUBTRACT:      code = 333; break;
        case GS_KEYCODE_KP_ADD:           code = 334; break;
        case GS_KEYCODE_KP_ENTER:         code = 335; break;
        case GS_KEYCODE_KP_EQUAL:         code = 336; break;
        case GS_KEYCODE_LEFT_SHIFT:       code = 340; break;
        case GS_KEYCODE_LEFT_CONTROL:     code = 341; break;
        case GS_KEYCODE_LEFT_ALT:         code = 342; break;
        case GS_KEYCODE_LEFT_SUPER:       code = 343; break;
        case GS_KEYCODE_RIGHT_SHIFT:      code = 344; break;
        case GS_KEYCODE_RIGHT_CONTROL:    code = 345; break;
        case GS_KEYCODE_RIGHT_ALT:        code = 346; break;
        case GS_KEYCODE_RIGHT_SUPER:      code = 347; break;
        case GS_KEYCODE_MENU:             code = 348; break;
    }
    return code;
}

// This doesn't work. Have to set up keycodes for emscripten instead. FUN.
gs_platform_keycode gs_platform_codepoint_to_key(uint32_t code)
{
    gs_platform_keycode key = GS_KEYCODE_INVALID;
    switch (code)
    {
        default:
        case 0:   key = GS_KEYCODE_INVALID; break;
        case 32:  key = GS_KEYCODE_SPACE; break;
        case 39:  key = GS_KEYCODE_APOSTROPHE; break;
        case 44:  key = GS_KEYCODE_COMMA; break;
        case 45:  key = GS_KEYCODE_MINUS; break;
        case 46:  key = GS_KEYCODE_PERIOD; break;
        case 47:  key = GS_KEYCODE_SLASH; break;
        case 48:  key = GS_KEYCODE_0; break;
        case 49:  key = GS_KEYCODE_1; break;
        case 50:  key = GS_KEYCODE_2; break;
        case 51:  key = GS_KEYCODE_3; break;
        case 52:  key = GS_KEYCODE_4; break;
        case 53:  key = GS_KEYCODE_5; break;
        case 54:  key = GS_KEYCODE_6; break;
        case 55:  key = GS_KEYCODE_7; break;
        case 56:  key = GS_KEYCODE_8; break;
        case 57:  key = GS_KEYCODE_9; break;
        case 59:  key = GS_KEYCODE_SEMICOLON; break;
        case 61:  key = GS_KEYCODE_EQUAL; break;
        case 65:  key = GS_KEYCODE_A; break;
        case 66:  key = GS_KEYCODE_B; break;
        case 67:  key = GS_KEYCODE_C; break;
        case 68:  key = GS_KEYCODE_D; break;
        case 69:  key = GS_KEYCODE_E; break;
        case 70:  key = GS_KEYCODE_F; break;
        case 71:  key = GS_KEYCODE_G; break;
        case 72:  key = GS_KEYCODE_H; break;
        case 73:  key = GS_KEYCODE_I; break;
        case 74:  key = GS_KEYCODE_J; break;
        case 75:  key = GS_KEYCODE_K; break;
        case 76:  key = GS_KEYCODE_L; break;
        case 77:  key = GS_KEYCODE_M; break;
        case 78:  key = GS_KEYCODE_N; break;
        case 79:  key = GS_KEYCODE_O; break;
        case 80:  key = GS_KEYCODE_P; break;
        case 81:  key = GS_KEYCODE_Q; break;
        case 82:  key = GS_KEYCODE_R; break;
        case 83:  key = GS_KEYCODE_S; break;
        case 84:  key = GS_KEYCODE_T; break;
        case 85:  key = GS_KEYCODE_U; break;
        case 86:  key = GS_KEYCODE_V; break;
        case 87:  key = GS_KEYCODE_W; break;
        case 88:  key = GS_KEYCODE_X; break;
        case 89:  key = GS_KEYCODE_Y; break;
        case 90:  key = GS_KEYCODE_Z; break;
        case 91:  key = GS_KEYCODE_LEFT_BRACKET; break;
        case 92:  key = GS_KEYCODE_BACKSLASH; break;
        case 93:  key = GS_KEYCODE_RIGHT_BRACKET; break;
        case 96:  key = GS_KEYCODE_GRAVE_ACCENT; break;
        case 161: key = GS_KEYCODE_WORLD_1; break;
        case 162: key = GS_KEYCODE_WORLD_2; break;
        case 256: key = GS_KEYCODE_ESC; break;
        case 257: key = GS_KEYCODE_ENTER; break;
        case 258: key = GS_KEYCODE_TAB; break;
        case 259: key = GS_KEYCODE_BACKSPACE; break;
        case 260: key = GS_KEYCODE_INSERT; break;
        case GLFW_KEY_DELETE: key = GS_KEYCODE_DELETE; break;
        case 262: key = GS_KEYCODE_RIGHT; break; 
        case 263: key = GS_KEYCODE_LEFT; break; 
        case 264: key = GS_KEYCODE_DOWN; break;
        case 265: key = GS_KEYCODE_UP; break; 
        case 266: key = GS_KEYCODE_PAGE_UP; break;
        case 267: key = GS_KEYCODE_PAGE_DOWN; break;
        case 268: key = GS_KEYCODE_HOME; break;    
        case 269: key = GS_KEYCODE_END; break;    
        case 280: key = GS_KEYCODE_CAPS_LOCK; break; 
        case 281: key = GS_KEYCODE_SCROLL_LOCK; break;
        case 282: key = GS_KEYCODE_NUM_LOCK; break;  
        case 283: key = GS_KEYCODE_PRINT_SCREEN; break;
        case 284: key = GS_KEYCODE_PAUSE; break;      
        case 290: key = GS_KEYCODE_F1; break;        
        case 291: key = GS_KEYCODE_F2; break;       
        case 292: key = GS_KEYCODE_F3; break;      
        case 293: key = GS_KEYCODE_F4; break;     
        case 294: key = GS_KEYCODE_F5; break;    
        case 295: key = GS_KEYCODE_F6; break;   
        case 296: key = GS_KEYCODE_F7; break;  
        case 297: key = GS_KEYCODE_F8; break; 
        case 298: key = GS_KEYCODE_F9; break;
        case 299: key = GS_KEYCODE_F10; break;
        case 300: key = GS_KEYCODE_F11; break;
        case 301: key = GS_KEYCODE_F12; break;
        case 302: key = GS_KEYCODE_F13; break;
        case 303: key = GS_KEYCODE_F14; break;
        case 304: key = GS_KEYCODE_F15; break;
        case 305: key = GS_KEYCODE_F16; break;
        case 306: key = GS_KEYCODE_F17; break;
        case 307: key = GS_KEYCODE_F18; break;
        case 308: key = GS_KEYCODE_F19; break;
        case 309: key = GS_KEYCODE_F20; break;
        case 310: key = GS_KEYCODE_F21; break;
        case 311: key = GS_KEYCODE_F22; break;
        case 312: key = GS_KEYCODE_F23; break;
        case 313: key = GS_KEYCODE_F24; break;
        case 314: key = GS_KEYCODE_F25; break;
        case 320: key = GS_KEYCODE_KP_0; break;
        case 321: key = GS_KEYCODE_KP_1; break;
        case 322: key = GS_KEYCODE_KP_2; break;
        case 323: key = GS_KEYCODE_KP_3; break;
        case 324: key = GS_KEYCODE_KP_4; break;
        case 325: key = GS_KEYCODE_KP_5; break;
        case 326: key = GS_KEYCODE_KP_6; break;
        case 327: key = GS_KEYCODE_KP_7; break;
        case 328: key = GS_KEYCODE_KP_8; break;
        case 329: key = GS_KEYCODE_KP_9; break;
        case 330: key = GS_KEYCODE_KP_DECIMAL; break;
        case 331: key = GS_KEYCODE_KP_DIVIDE; break;
        case 332: key = GS_KEYCODE_KP_MULTIPLY; break;
        case 333: key = GS_KEYCODE_KP_SUBTRACT; break;
        case 334: key = GS_KEYCODE_KP_ADD; break;    
        case 335: key = GS_KEYCODE_KP_ENTER; break;  
        case 336: key = GS_KEYCODE_KP_EQUAL; break;  
        case 340: key = GS_KEYCODE_LEFT_SHIFT; break; 
        case 341: key = GS_KEYCODE_LEFT_CONTROL; break; 
        case 342: key = GS_KEYCODE_LEFT_ALT; break;    
        case 343: key = GS_KEYCODE_LEFT_SUPER; break; 
        case 344: key = GS_KEYCODE_RIGHT_SHIFT; break;
        case 345: key = GS_KEYCODE_RIGHT_CONTROL; break;  
        case 346: key = GS_KEYCODE_RIGHT_ALT; break;     
        case 347: key = GS_KEYCODE_RIGHT_SUPER; break;  
        case 348: key = GS_KEYCODE_MENU; break;        
    }
    return key;
}

/*=== GLFW Callbacks ===*/

gs_platform_keycode glfw_key_to_gs_keycode(u32 code)
{
    switch (code)
    {
        case GLFW_KEY_A:                return GS_KEYCODE_A; break;
        case GLFW_KEY_B:                return GS_KEYCODE_B; break;
        case GLFW_KEY_C:                return GS_KEYCODE_C; break;
        case GLFW_KEY_D:                return GS_KEYCODE_D; break;
        case GLFW_KEY_E:                return GS_KEYCODE_E; break;
        case GLFW_KEY_F:                return GS_KEYCODE_F; break;
        case GLFW_KEY_G:                return GS_KEYCODE_G; break;
        case GLFW_KEY_H:                return GS_KEYCODE_H; break;
        case GLFW_KEY_I:                return GS_KEYCODE_I; break;
        case GLFW_KEY_J:                return GS_KEYCODE_J; break;
        case GLFW_KEY_K:                return GS_KEYCODE_K; break;
        case GLFW_KEY_L:                return GS_KEYCODE_L; break;
        case GLFW_KEY_M:                return GS_KEYCODE_M; break;
        case GLFW_KEY_N:                return GS_KEYCODE_N; break;
        case GLFW_KEY_O:                return GS_KEYCODE_O; break;
        case GLFW_KEY_P:                return GS_KEYCODE_P; break;
        case GLFW_KEY_Q:                return GS_KEYCODE_Q; break;
        case GLFW_KEY_R:                return GS_KEYCODE_R; break;
        case GLFW_KEY_S:                return GS_KEYCODE_S; break;
        case GLFW_KEY_T:                return GS_KEYCODE_T; break;
        case GLFW_KEY_U:                return GS_KEYCODE_U; break;
        case GLFW_KEY_V:                return GS_KEYCODE_V; break;
        case GLFW_KEY_W:                return GS_KEYCODE_W; break;
        case GLFW_KEY_X:                return GS_KEYCODE_X; break;
        case GLFW_KEY_Y:                return GS_KEYCODE_Y; break;
        case GLFW_KEY_Z:                return GS_KEYCODE_Z; break;
        case GLFW_KEY_LEFT_SHIFT:       return GS_KEYCODE_LEFT_SHIFT; break;
        case GLFW_KEY_RIGHT_SHIFT:      return GS_KEYCODE_RIGHT_SHIFT; break;
        case GLFW_KEY_LEFT_ALT:         return GS_KEYCODE_LEFT_ALT; break;
        case GLFW_KEY_RIGHT_ALT:        return GS_KEYCODE_RIGHT_ALT; break;
        case GLFW_KEY_LEFT_CONTROL:     return GS_KEYCODE_LEFT_CONTROL; break;
        case GLFW_KEY_RIGHT_CONTROL:    return GS_KEYCODE_RIGHT_CONTROL; break;
        case GLFW_KEY_BACKSPACE:        return GS_KEYCODE_BACKSPACE; break;
        case GLFW_KEY_BACKSLASH:        return GS_KEYCODE_BACKSLASH; break;
        case GLFW_KEY_SLASH:            return GS_KEYCODE_SLASH; break;
        case GLFW_KEY_GRAVE_ACCENT:     return GS_KEYCODE_GRAVE_ACCENT; break;
        case GLFW_KEY_COMMA:            return GS_KEYCODE_COMMA; break;
        case GLFW_KEY_PERIOD:           return GS_KEYCODE_PERIOD; break;
        case GLFW_KEY_ESCAPE:           return GS_KEYCODE_ESC; break; 
        case GLFW_KEY_SPACE:            return GS_KEYCODE_SPACE; break;
        case GLFW_KEY_LEFT:             return GS_KEYCODE_LEFT; break;
        case GLFW_KEY_UP:               return GS_KEYCODE_UP; break;
        case GLFW_KEY_RIGHT:            return GS_KEYCODE_RIGHT; break;
        case GLFW_KEY_DOWN:             return GS_KEYCODE_DOWN; break;
        case GLFW_KEY_0:                return GS_KEYCODE_0; break;
        case GLFW_KEY_1:                return GS_KEYCODE_1; break;
        case GLFW_KEY_2:                return GS_KEYCODE_2; break;
        case GLFW_KEY_3:                return GS_KEYCODE_3; break;
        case GLFW_KEY_4:                return GS_KEYCODE_4; break;
        case GLFW_KEY_5:                return GS_KEYCODE_5; break;
        case GLFW_KEY_6:                return GS_KEYCODE_6; break;
        case GLFW_KEY_7:                return GS_KEYCODE_7; break;
        case GLFW_KEY_8:                return GS_KEYCODE_8; break;
        case GLFW_KEY_9:                return GS_KEYCODE_9; break;
        case GLFW_KEY_KP_0:             return GS_KEYCODE_KP_0; break;
        case GLFW_KEY_KP_1:             return GS_KEYCODE_KP_1; break;
        case GLFW_KEY_KP_2:             return GS_KEYCODE_KP_2; break;
        case GLFW_KEY_KP_3:             return GS_KEYCODE_KP_3; break;
        case GLFW_KEY_KP_4:             return GS_KEYCODE_KP_4; break;
        case GLFW_KEY_KP_5:             return GS_KEYCODE_KP_5; break;
        case GLFW_KEY_KP_6:             return GS_KEYCODE_KP_6; break;
        case GLFW_KEY_KP_7:             return GS_KEYCODE_KP_7; break;
        case GLFW_KEY_KP_8:             return GS_KEYCODE_KP_8; break;
        case GLFW_KEY_KP_9:             return GS_KEYCODE_KP_9; break;
        case GLFW_KEY_CAPS_LOCK:        return GS_KEYCODE_CAPS_LOCK; break;
        case GLFW_KEY_DELETE:           return GS_KEYCODE_DELETE; break;
        case GLFW_KEY_END:              return GS_KEYCODE_END; break;
        case GLFW_KEY_F1:               return GS_KEYCODE_F1; break;
        case GLFW_KEY_F2:               return GS_KEYCODE_F2; break;
        case GLFW_KEY_F3:               return GS_KEYCODE_F3; break;
        case GLFW_KEY_F4:               return GS_KEYCODE_F4; break;
        case GLFW_KEY_F5:               return GS_KEYCODE_F5; break;
        case GLFW_KEY_F6:               return GS_KEYCODE_F6; break;
        case GLFW_KEY_F7:               return GS_KEYCODE_F7; break;
        case GLFW_KEY_F8:               return GS_KEYCODE_F8; break;
        case GLFW_KEY_F9:               return GS_KEYCODE_F9; break;
        case GLFW_KEY_F10:              return GS_KEYCODE_F10; break;
        case GLFW_KEY_F11:              return GS_KEYCODE_F11; break;
        case GLFW_KEY_F12:              return GS_KEYCODE_F12; break;
        case GLFW_KEY_HOME:             return GS_KEYCODE_HOME; break;
        case GLFW_KEY_EQUAL:            return GS_KEYCODE_EQUAL; break;
        case GLFW_KEY_MINUS:            return GS_KEYCODE_MINUS; break;
        case GLFW_KEY_LEFT_BRACKET:     return GS_KEYCODE_LEFT_BRACKET; break;
        case GLFW_KEY_RIGHT_BRACKET:    return GS_KEYCODE_RIGHT_BRACKET; break;
        case GLFW_KEY_SEMICOLON:        return GS_KEYCODE_SEMICOLON; break;
        case GLFW_KEY_ENTER:            return GS_KEYCODE_ENTER; break;
        case GLFW_KEY_INSERT:           return GS_KEYCODE_INSERT; break;
        case GLFW_KEY_PAGE_UP:          return GS_KEYCODE_PAGE_UP; break;
        case GLFW_KEY_PAGE_DOWN:        return GS_KEYCODE_PAGE_DOWN; break;
        case GLFW_KEY_NUM_LOCK:         return GS_KEYCODE_NUM_LOCK; break;
        case GLFW_KEY_TAB:              return GS_KEYCODE_TAB; break;
        case GLFW_KEY_KP_MULTIPLY:      return GS_KEYCODE_KP_MULTIPLY; break;
        case GLFW_KEY_KP_DIVIDE:        return GS_KEYCODE_KP_DIVIDE; break;
        case GLFW_KEY_KP_ADD:           return GS_KEYCODE_KP_ADD; break;
        case GLFW_KEY_KP_SUBTRACT:      return GS_KEYCODE_KP_SUBTRACT; break;
        case GLFW_KEY_KP_ENTER:         return GS_KEYCODE_KP_ENTER; break;
        case GLFW_KEY_KP_DECIMAL:       return GS_KEYCODE_KP_DECIMAL; break;
        case GLFW_KEY_PAUSE:            return GS_KEYCODE_PAUSE; break;
        case GLFW_KEY_PRINT_SCREEN:     return GS_KEYCODE_PRINT_SCREEN; break;
        default:                        return GS_KEYCODE_COUNT; break;
    }

    // Shouldn't reach here
    return GS_KEYCODE_COUNT;
}

gs_platform_mouse_button_code __glfw_button_to_gs_mouse_button(s32 code)
{
    switch (code)
    {
        case GLFW_MOUSE_BUTTON_LEFT:    return GS_MOUSE_LBUTTON; break;
        case GLFW_MOUSE_BUTTON_RIGHT:   return GS_MOUSE_RBUTTON; break;
        case GLFW_MOUSE_BUTTON_MIDDLE: return GS_MOUSE_MBUTTON; break;
    }   

    // Shouldn't reach here
    return GS_MOUSE_BUTTON_CODE_COUNT;
}

void __glfw_char_callback(GLFWwindow* window, uint32_t codepoint)
{
    // Grab platform instance from engine
    gs_platform_t* platform = gs_engine_subsystem(platform); 

    gs_platform_event_t evt = gs_default_val();
    evt.type = GS_PLATFORM_EVENT_TEXT;
    evt.text.codepoint = codepoint;

    // Add action
    gs_platform_add_event(&evt);
}

void __glfw_key_callback(GLFWwindow* window, s32 code, s32 scancode, s32 action, s32 mods) 
{
    // Grab platform instance from engine
    gs_platform_t* platform = gs_engine_subsystem(platform);

    // Get keycode from key
    gs_platform_keycode key = glfw_key_to_gs_keycode(code);

    // Push back event into platform events
    gs_platform_event_t evt = gs_default_val();
    evt.type = GS_PLATFORM_EVENT_KEY;
    evt.key.codepoint = code;
    evt.key.keycode = key;
    evt.key.modifier = (gs_platform_key_modifier_type)mods;

    switch (action)
    {
        // Released
        case 0: {
            gs_platform_release_key(key);
            evt.key.action = GS_PLATFORM_KEY_RELEASED;
        } break;

        // Pressed
        case 1: {
            gs_platform_press_key(key);
            evt.key.action = GS_PLATFORM_KEY_PRESSED;
        } break;

        // Down
        case 2: {
            gs_platform_press_key(key);
            evt.key.action = GS_PLATFORM_KEY_DOWN;
        } break;

        default: {
        } break;
    }

    // Add action
    gs_platform_add_event(&evt);
}

void __glfw_mouse_button_callback(GLFWwindow* window, s32 code, s32 action, s32 mods)
{
    // Grab platform instance from engine
    gs_platform_t* platform = gs_engine_subsystem(platform);

    // Get mouse code from key
    gs_platform_mouse_button_code button = __glfw_button_to_gs_mouse_button(code);

    // Push back event into platform events
    gs_platform_event_t evt = gs_default_val();
    evt.type = GS_PLATFORM_EVENT_MOUSE;
    evt.mouse.codepoint = code;
    evt.mouse.button = button;

    switch (action)
    {
        // Released
        case 0:
        {
            gs_platform_release_mouse_button(button);
            evt.mouse.action = GS_PLATFORM_MOUSE_BUTTON_RELEASED;
        } break;

        // Pressed
        case 1:
        {
            gs_platform_press_mouse_button(button);
            evt.mouse.action = GS_PLATFORM_MOUSE_BUTTON_PRESSED;
        } break;

        // Down
        case 2:
        {
            gs_platform_press_mouse_button(button);
            evt.mouse.action = GS_PLATFORM_MOUSE_BUTTON_DOWN;
        } break;
    }

    // Add action
    gs_platform_add_event(&evt);
}

void __glfw_mouse_cursor_position_callback(GLFWwindow* window, f64 x, f64 y)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);
    // platform->input.mouse.position = gs_v2((f32)x, (f32)y);
    // platform->input.mouse.moved_this_frame = true;

    gs_platform_event_t gs_evt = gs_default_val();
    gs_evt.type = GS_PLATFORM_EVENT_MOUSE;
    gs_evt.mouse.action = GS_PLATFORM_MOUSE_MOVE;

    // gs_println("pos: <%.2f, %.2f>, old: <%.2f, %.2f>", x, y, platform->input.mouse.position.x, platform->input.mouse.position.y);

    // gs_evt.mouse.move = gs_v2((f32)x, (f32)y);

    // Calculate mouse move based on whether locked or not
    if (gs_platform_mouse_locked()) {
        gs_evt.mouse.move.x = x - platform->input.mouse.position.x;
        gs_evt.mouse.move.y = y - platform->input.mouse.position.y;
        platform->input.mouse.position.x = x;
        platform->input.mouse.position.y = y;
    } else {
        gs_evt.mouse.move = gs_v2((f32)x, (f32)y);
    }

    // Push back event into platform events
    gs_platform_add_event(&gs_evt);
}

void __glfw_mouse_scroll_wheel_callback(GLFWwindow* window, f64 x, f64 y)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);
    platform->input.mouse.wheel = gs_v2((f32)x, (f32)y);

    // Push back event into platform events
    gs_platform_event_t gs_evt = gs_default_val();
    gs_evt.type = GS_PLATFORM_EVENT_MOUSE;
    gs_evt.mouse.action = GS_PLATFORM_MOUSE_WHEEL;
    gs_evt.mouse.wheel = gs_v2((f32)x, (f32)y);
    gs_platform_add_event(&gs_evt);
}

// Gets called when mouse enters or leaves frame of window
void __glfw_mouse_cursor_enter_callback(GLFWwindow* window, s32 entered)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);
    gs_platform_event_t gs_evt = gs_default_val();
    gs_evt.type = GS_PLATFORM_EVENT_MOUSE;
    gs_evt.mouse.action = entered ? GS_PLATFORM_MOUSE_ENTER : GS_PLATFORM_MOUSE_LEAVE;
    gs_platform_add_event(&gs_evt);
}

void __glfw_frame_buffer_size_callback(GLFWwindow* window, s32 width, s32 height)
{
    // Nothing for now
}

/*== Platform Input == */

void gs_platform_process_input(gs_platform_input_t* input)
{
    glfwPollEvents();
}

/*== Platform Util == */

void  gs_platform_sleep(float ms)
{
    #if (defined GS_PLATFORM_WIN)

            timeBeginPeriod(1);
            Sleep((uint64_t)ms);
            timeEndPeriod(1);

    #elif (defined GS_PLATFORM_APPLE)

            usleep(ms * 1000.f); // unistd.h
    #else
            usleep(ms * 1000.f); // unistd.h
    #endif
}

double gs_platform_elapsed_time()
{
    return (glfwGetTime() * 1000.0);
}

/*== Platform Video == */

void  gs_platform_enable_vsync(int32_t enabled)
{
    glfwSwapInterval(enabled ? 1 : 0);
}

/*== OpenGL debug callback == */
void GLAPIENTRY __gs_platform_gl_debug(GLenum source, GLenum type, GLuint id, GLenum severity,
                            GLsizei len, const GLchar* msg, const void* user)
{
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
    {
        gs_println("GL: %s", msg);
    }
}

/*== Platform Window == */

void* gs_platform_create_window_internal(const char* title, uint32_t width, uint32_t height, uint32_t monitor_index)
{
    // Grab window hints from application desc
    u32 window_hints = gs_engine_instance()->ctx.app.window_flags;

    // Set whether or not the screen is resizable
    glfwWindowHint(GLFW_RESIZABLE, (window_hints & GS_WINDOW_FLAGS_NO_RESIZE) != GS_WINDOW_FLAGS_NO_RESIZE);

    // Get monitor if fullscreen
    GLFWmonitor* monitor = NULL;
    if ((window_hints & GS_WINDOW_FLAGS_FULLSCREEN) == GS_WINDOW_FLAGS_FULLSCREEN)
    {
        int monitor_count;
        GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);
        if (monitor_index < monitor_count)
        {
            monitor = monitors[monitor_index];
        }
    }

    GLFWwindow* window = glfwCreateWindow(width, height, title, monitor, NULL);
    if (window == NULL)
    {
        gs_println("Failed to create window.");
        glfwTerminate();
        return NULL;
   }

    // Callbacks for window
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, &__glfw_key_callback);
    glfwSetCharCallback(window, &__glfw_char_callback);
    glfwSetMouseButtonCallback(window, &__glfw_mouse_button_callback);
    glfwSetCursorEnterCallback(window, &__glfw_mouse_cursor_enter_callback);
    glfwSetCursorPosCallback(window, &__glfw_mouse_cursor_position_callback);
    glfwSetScrollCallback(window, &__glfw_mouse_scroll_wheel_callback);

    // Need to make sure this is ONLY done once.
    if (gs_slot_array_empty(gs_engine_subsystem(platform)->windows))
    {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            gs_println("Failed to initialize GLFW.");
            return NULL;
        }

        switch (gs_engine_subsystem(platform)->settings.video.driver)
        {
            case GS_PLATFORM_VIDEO_DRIVER_TYPE_OPENGL: 
            {
                gs_println("OpenGL Version: %s", glGetString(GL_VERSION));
                if (gs_engine_subsystem(platform)->settings.video.graphics.debug)
                {
                    glDebugMessageCallback(__gs_platform_gl_debug, NULL);
                }
            } break;

            default: break;
        }
    }

    return window;
}

// Platform callbacks
GS_API_DECL void gs_platform_set_dropped_files_callback(uint32_t handle, gs_dropped_files_callback_t cb)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    glfwSetDropCallback(win, (GLFWdropfun)cb);
}

GS_API_DECL void gs_platform_set_window_close_callback(uint32_t handle, gs_window_close_callback_t cb)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    glfwSetWindowCloseCallback(win, (GLFWwindowclosefun)cb);
}

GS_API_DECL void gs_platform_set_character_callback(uint32_t handle, gs_character_callback_t cb)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    glfwSetCharCallback(win, (GLFWcharfun)cb);
}

GS_API_DECL void gs_platform_set_framebuffer_resize_callback(uint32_t handle, gs_framebuffer_resize_callback_t cb)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    glfwSetFramebufferSizeCallback(win, (GLFWframebuffersizefun)cb);
}

void gs_platform_mouse_set_position(uint32_t handle, float x, float y)
{
    struct gs_platform_t* platform = gs_engine_subsystem(platform);
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    glfwSetCursorPos(win, x, y);
}

void* gs_platform_raw_window_handle(uint32_t handle)
{
    // Grab instance of platform from engine
    gs_platform_t* platform = gs_engine_subsystem(platform);

    // Grab window from handle
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    return (void*)win;
}

void gs_platform_window_swap_buffer(uint32_t handle)
{
    // Grab instance of platform from engine
    gs_platform_t* platform = gs_engine_subsystem(platform);

    // Grab window from handle
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    glfwSwapBuffers(win);
}

gs_vec2 gs_platform_window_sizev(uint32_t handle)
{
    GLFWwindow* win = __glfw_window_from_handle(gs_engine_subsystem(platform), handle);
    int32_t w, h;
    glfwGetWindowSize(win, &w, &h);
    return gs_v2((float)w, (float)h);
}

void gs_platform_window_size(uint32_t handle, uint32_t* w, uint32_t* h)
{
    GLFWwindow* win = __glfw_window_from_handle(gs_engine_instance()->ctx.platform, handle);
    glfwGetWindowSize(win, (int32_t*)w, (int32_t*)h);
}

uint32_t gs_platform_window_width(uint32_t handle)
{
    GLFWwindow* win = __glfw_window_from_handle(gs_engine_instance()->ctx.platform, handle);
    int32_t w, h;
    glfwGetWindowSize(win, &w, &h);
    return w;
}

uint32_t gs_platform_window_height(uint32_t handle)
{
    GLFWwindow* win = __glfw_window_from_handle(gs_engine_instance()->ctx.platform, handle);
    int32_t w, h;
    glfwGetWindowSize(win, &w, &h);
    return h;
}

bool32_t gs_platform_window_fullscreen(uint32_t handle)
{
    GLFWwindow* win = __glfw_window_from_handle(gs_engine_subsystem(platform), handle);
    return glfwGetWindowMonitor(win) != NULL;
}

void gs_platform_window_position(uint32_t handle, uint32_t* x, uint32_t* y)
{
    GLFWwindow* win = __glfw_window_from_handle(gs_engine_instance()->ctx.platform, handle);
    glfwGetWindowPos(win, (int32_t*)x, (int32_t*)y);
}

gs_vec2 gs_platform_window_positionv(uint32_t handle)
{
    GLFWwindow* win = __glfw_window_from_handle(gs_engine_subsystem(platform), handle);
    int32_t x, y;
    glfwGetWindowPos(win, &x, &y);
    return gs_v2((float)x, (float)y);
}

void gs_platform_set_window_size(uint32_t handle, uint32_t w, uint32_t h)
{
    GLFWwindow* win = __glfw_window_from_handle(gs_engine_subsystem(platform), handle);
    glfwSetWindowSize(win, (int32_t)w, (int32_t)h);
}

void gs_platform_set_window_sizev(uint32_t handle, gs_vec2 v)
{
    GLFWwindow* win = __glfw_window_from_handle(gs_engine_subsystem(platform), handle);
    glfwSetWindowSize(win, (uint32_t)v.x, (uint32_t)v.y);
}

void gs_platform_set_window_fullscreen(uint32_t handle, bool32_t fullscreen)
{
    GLFWwindow* win = __glfw_window_from_handle(gs_engine_subsystem(platform), handle);
    GLFWmonitor* monitor = NULL;

    int32_t x, y, w, h;
    glfwGetWindowPos(win, &x, &y);
    glfwGetWindowSize(win, &w, &h);

    if (fullscreen)
    {
        uint32_t monitor_index = gs_engine_instance()->ctx.app.monitor_index;
        int monitor_count;
        GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);
        if (monitor_index < monitor_count)
        {
            monitor = monitors[monitor_index];
        }
    }

    glfwSetWindowMonitor(win, monitor, x, y, w, h, GLFW_DONT_CARE);
}

void gs_platform_set_window_position(uint32_t handle, uint32_t x, uint32_t y)
{
    GLFWwindow* win = __glfw_window_from_handle(gs_engine_subsystem(platform), handle);
    glfwSetWindowPos(win, (int32_t)x, (int32_t)y);
}

void gs_platform_set_window_positionv(uint32_t handle, gs_vec2 v)
{
    GLFWwindow* win = __glfw_window_from_handle(gs_engine_subsystem(platform), handle);
    glfwSetWindowPos(win, (int32_t)v.x, (int32_t)v.y);
}

void gs_platform_framebuffer_size(uint32_t handle, uint32_t* w, uint32_t* h)
{
    GLFWwindow* win = __glfw_window_from_handle(gs_engine_subsystem(platform), handle);
    glfwGetFramebufferSize(win, (int32_t*)w, (int32_t*)h);
}

gs_vec2 gs_platform_framebuffer_sizev(uint32_t handle)
{
    uint32_t w = 0, h = 0;
    gs_platform_framebuffer_size(handle, &w, &h);
    return gs_v2((float)w, (float)h);
}

uint32_t gs_platform_framebuffer_width(uint32_t handle)
{
    uint32_t w = 0, h = 0;
    gs_platform_framebuffer_size(handle, &w, &h);
    return w;
}

uint32_t gs_platform_framebuffer_height(uint32_t handle)
{
    uint32_t w = 0, h = 0;
    gs_platform_framebuffer_size(handle, &w, &h);
    return h;
}

void gs_platform_set_cursor(uint32_t handle, gs_platform_cursor cursor)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    GLFWcursor* cp = ((GLFWcursor*)platform->cursors[(u32)cursor]); 
    glfwSetCursor(win, cp);
}

void gs_platform_lock_mouse(uint32_t handle, bool32_t lock)
{
    __gs_input()->mouse.locked = lock;
    gs_platform_t* platform = gs_engine_subsystem(platform);
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    glfwSetInputMode(win, GLFW_CURSOR, lock ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

    // Not sure if I want to support this or not
    // if (glfwRawMouseMotionSupported()) {
    //     glfwSetInputMode(win, GLFW_RAW_MOUSE_MOTION, lock ? GLFW_TRUE : GLFW_FALSE);
    // }
}

/* Main entry point for platform*/
#ifndef GS_NO_HIJACK_MAIN

    int32_t main(int32_t argv, char** argc)
    {
        gs_engine_t* inst = gs_engine_create(gs_main(argv, argc));
        while (gs_engine_app()->is_running) {
            gs_engine_frame();
        }
        // Free engine
        gs_free(inst);
        return 0;
    }

#endif // GS_NO_HIJACK_MAIN

#undef GS_PLATFORM_IMPL_GLFW
#endif // GS_PLATFORM_IMPL_GLFW

/*==========================
// Emscripten Implemenation
==========================*/

#ifdef GS_PLATFORM_IMPL_EMSCRIPTEN

#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#include <GLES3/gl3.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

// Emscripten context data 
typedef struct gs_ems_t
{
    const char* canvas_name;
    double canvas_width;
    double canvas_height;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;
    bool32_t mouse_down[GS_MOUSE_BUTTON_CODE_COUNT];
} gs_ems_t;

#define GS_EMS_DATA()\
    ((gs_ems_t*)(gs_engine_subsystem(platform)->user_data))

uint32_t gs_platform_key_to_codepoint(gs_platform_keycode key)
{
    uint32_t code = 0;
    switch (key)
    {
        default:
        case GS_KEYCODE_COUNT:
        case GS_KEYCODE_INVALID:          code = 0; break;
        case GS_KEYCODE_SPACE:            code = 32; break;
        case GS_KEYCODE_APOSTROPHE:       code = 222; break;
        case GS_KEYCODE_COMMA:            code = 44; break;
        case GS_KEYCODE_MINUS:            code = 45; break;
        case GS_KEYCODE_PERIOD:           code = 46; break;
        case GS_KEYCODE_SLASH:            code = 47; break;
        case GS_KEYCODE_0:                code = 48; break;
        case GS_KEYCODE_1:                code = 49; break;
        case GS_KEYCODE_2:                code = 50; break;
        case GS_KEYCODE_3:                code = 51; break;
        case GS_KEYCODE_4:                code = 52; break;
        case GS_KEYCODE_5:                code = 53; break;
        case GS_KEYCODE_6:                code = 54; break;
        case GS_KEYCODE_7:                code = 55; break;
        case GS_KEYCODE_8:                code = 56; break;
        case GS_KEYCODE_9:                code = 57; break;
        case GS_KEYCODE_SEMICOLON:        code = 59; break;  /* ; */
        case GS_KEYCODE_EQUAL:            code = 61; break;  /* code = */
        case GS_KEYCODE_A:                code = 65 + 32; break;
        case GS_KEYCODE_B:                code = 66 + 32; break;
        case GS_KEYCODE_C:                code = 67 + 32; break;
        case GS_KEYCODE_D:                code = 68 + 32; break;
        case GS_KEYCODE_E:                code = 69 + 32; break;
        case GS_KEYCODE_F:                code = 70 + 32; break;
        case GS_KEYCODE_G:                code = 71 + 32; break;
        case GS_KEYCODE_H:                code = 72 + 32; break;
        case GS_KEYCODE_I:                code = 73 + 32; break;
        case GS_KEYCODE_J:                code = 74 + 32; break;
        case GS_KEYCODE_K:                code = 75 + 32; break;
        case GS_KEYCODE_L:                code = 76 + 32; break;
        case GS_KEYCODE_M:                code = 77 + 32; break;
        case GS_KEYCODE_N:                code = 78 + 32; break;
        case GS_KEYCODE_O:                code = 79 + 32; break;
        case GS_KEYCODE_P:                code = 80 + 32; break;
        case GS_KEYCODE_Q:                code = 81 + 32; break;
        case GS_KEYCODE_R:                code = 82 + 32; break;
        case GS_KEYCODE_S:                code = 83 + 32; break;
        case GS_KEYCODE_T:                code = 84 + 32; break;
        case GS_KEYCODE_U:                code = 85 + 32; break;
        case GS_KEYCODE_V:                code = 86 + 32; break;
        case GS_KEYCODE_W:                code = 87 + 32; break;
        case GS_KEYCODE_X:                code = 88 + 32; break;
        case GS_KEYCODE_Y:                code = 89 + 32; break;
        case GS_KEYCODE_Z:                code = 90 + 32; break;
        case GS_KEYCODE_LEFT_BRACKET:     code = 91; break;  /* [ */
        case GS_KEYCODE_BACKSLASH:        code = 92; break;  /* \ */
        case GS_KEYCODE_RIGHT_BRACKET:    code = 93; break;  /* ] */
        case GS_KEYCODE_GRAVE_ACCENT:     code = 96; break;  /* ` */
        case GS_KEYCODE_WORLD_1:          code = 161; break; /* non-US #1 */
        case GS_KEYCODE_WORLD_2:          code = 162; break; /* non-US #2 */
        case GS_KEYCODE_ESC:              code = 27; break;
        case GS_KEYCODE_ENTER:            code = 13; break;
        case GS_KEYCODE_TAB:              code = 9; break;
        case GS_KEYCODE_BACKSPACE:        code = 8; break;
        case GS_KEYCODE_INSERT:           code = 260; break;
        case GS_KEYCODE_DELETE:           code = 261; break;
        case GS_KEYCODE_LEFT:             code = 37; break;
        case GS_KEYCODE_UP:               code = 38; break;
        case GS_KEYCODE_RIGHT:            code = 39; break;
        case GS_KEYCODE_DOWN:             code = 40; break;
        case GS_KEYCODE_PAGE_UP:          code = 266; break;
        case GS_KEYCODE_PAGE_DOWN:        code = 267; break;
        case GS_KEYCODE_HOME:             code = 268; break;
        case GS_KEYCODE_END:              code = 269; break;
        case GS_KEYCODE_CAPS_LOCK:        code = 280; break;
        case GS_KEYCODE_SCROLL_LOCK:      code = 281; break;
        case GS_KEYCODE_NUM_LOCK:         code = 282; break;
        case GS_KEYCODE_PRINT_SCREEN:     code = 283; break;
        case GS_KEYCODE_PAUSE:            code = 284; break;
        case GS_KEYCODE_F1:               code = 290; break;
        case GS_KEYCODE_F2:               code = 291; break;
        case GS_KEYCODE_F3:               code = 292; break;
        case GS_KEYCODE_F4:               code = 293; break;
        case GS_KEYCODE_F5:               code = 294; break;
        case GS_KEYCODE_F6:               code = 295; break;
        case GS_KEYCODE_F7:               code = 296; break;
        case GS_KEYCODE_F8:               code = 297; break;
        case GS_KEYCODE_F9:               code = 298; break;
        case GS_KEYCODE_F10:              code = 299; break;
        case GS_KEYCODE_F11:              code = 300; break;
        case GS_KEYCODE_F12:              code = 301; break;
        case GS_KEYCODE_F13:              code = 302; break;
        case GS_KEYCODE_F14:              code = 303; break;
        case GS_KEYCODE_F15:              code = 304; break;
        case GS_KEYCODE_F16:              code = 305; break;
        case GS_KEYCODE_F17:              code = 306; break;
        case GS_KEYCODE_F18:              code = 307; break;
        case GS_KEYCODE_F19:              code = 308; break;
        case GS_KEYCODE_F20:              code = 309; break;
        case GS_KEYCODE_F21:              code = 310; break;
        case GS_KEYCODE_F22:              code = 311; break;
        case GS_KEYCODE_F23:              code = 312; break;
        case GS_KEYCODE_F24:              code = 313; break;
        case GS_KEYCODE_F25:              code = 314; break;
        case GS_KEYCODE_KP_0:             code = 320; break;
        case GS_KEYCODE_KP_1:             code = 321; break;
        case GS_KEYCODE_KP_2:             code = 322; break;
        case GS_KEYCODE_KP_3:             code = 323; break;
        case GS_KEYCODE_KP_4:             code = 324; break;
        case GS_KEYCODE_KP_5:             code = 325; break;
        case GS_KEYCODE_KP_6:             code = 326; break;
        case GS_KEYCODE_KP_7:             code = 327; break;
        case GS_KEYCODE_KP_8:             code = 328; break;
        case GS_KEYCODE_KP_9:             code = 329; break;
        case GS_KEYCODE_KP_DECIMAL:       code = 330; break;
        case GS_KEYCODE_KP_DIVIDE:        code = 331; break;
        case GS_KEYCODE_KP_MULTIPLY:      code = 332; break;
        case GS_KEYCODE_KP_SUBTRACT:      code = 333; break;
        case GS_KEYCODE_KP_ADD:           code = 334; break;
        case GS_KEYCODE_KP_ENTER:         code = 335; break;
        case GS_KEYCODE_KP_EQUAL:         code = 336; break;
        case GS_KEYCODE_LEFT_SHIFT:       code = 16; break;
        case GS_KEYCODE_LEFT_CONTROL:     code = 17; break;
        case GS_KEYCODE_LEFT_ALT:         code = 18; break;
        case GS_KEYCODE_LEFT_SUPER:       code = 343; break;
        case GS_KEYCODE_RIGHT_SHIFT:      code = 16; break;
        case GS_KEYCODE_RIGHT_CONTROL:    code = 17; break;
        case GS_KEYCODE_RIGHT_ALT:        code = 18; break;
        case GS_KEYCODE_RIGHT_SUPER:      code = 347; break;
        case GS_KEYCODE_MENU:             code = 348; break;
    }
    return code;
}

/*
    key_to_code_map[count] = gs_default_val();
    code_to_key_map[count] = gs_default_val();
*/

// This doesn't work. Have to set up keycodes for emscripten instead. FUN.
gs_platform_keycode gs_platform_codepoint_to_key(uint32_t code)
{
    gs_platform_keycode key = GS_KEYCODE_INVALID;
    switch (code)
    {
        default:
        case 0:   key = GS_KEYCODE_INVALID; break;
        case 32:  key = GS_KEYCODE_SPACE; break;
        case 222:  key = GS_KEYCODE_APOSTROPHE; break;
        case 44:  key = GS_KEYCODE_COMMA; break;
        case 45:  key = GS_KEYCODE_MINUS; break;
        case 46:  key = GS_KEYCODE_PERIOD; break;
        case 47:  key = GS_KEYCODE_SLASH; break;
        case 48:  key = GS_KEYCODE_0; break;
        case 49:  key = GS_KEYCODE_1; break;
        case 50:  key = GS_KEYCODE_2; break;
        case 51:  key = GS_KEYCODE_3; break;
        case 52:  key = GS_KEYCODE_4; break;
        case 53:  key = GS_KEYCODE_5; break;
        case 54:  key = GS_KEYCODE_6; break;
        case 55:  key = GS_KEYCODE_7; break;
        case 56:  key = GS_KEYCODE_8; break;
        case 57:  key = GS_KEYCODE_9; break;
        case 59:  key = GS_KEYCODE_SEMICOLON; break;
        case 61:  key = GS_KEYCODE_EQUAL; break;
        case 65: case 65 + 32:  key = GS_KEYCODE_A; break;
        case 66: case 66 + 32:  key = GS_KEYCODE_B; break;
        case 67: case 67 + 32:  key = GS_KEYCODE_C; break;
        case 68: case 68 + 32:  key = GS_KEYCODE_D; break;
        case 69: case 69 + 32:  key = GS_KEYCODE_E; break;
        case 70: case 70 + 32:  key = GS_KEYCODE_F; break;
        case 71: case 71 + 32:  key = GS_KEYCODE_G; break;
        case 72: case 72 + 32:  key = GS_KEYCODE_H; break;
        case 73: case 73 + 32:  key = GS_KEYCODE_I; break;
        case 74: case 74 + 32:  key = GS_KEYCODE_J; break;
        case 75: case 75 + 32:  key = GS_KEYCODE_K; break;
        case 76: case 76 + 32:  key = GS_KEYCODE_L; break;
        case 77: case 77 + 32:  key = GS_KEYCODE_M; break;
        case 78: case 78 + 32:  key = GS_KEYCODE_N; break;
        case 79: case 79 + 32:  key = GS_KEYCODE_O; break;
        case 80: case 80 + 32:  key = GS_KEYCODE_P; break;
        case 81: case 81 + 32:  key = GS_KEYCODE_Q; break;
        case 82: case 82 + 32:  key = GS_KEYCODE_R; break;
        case 83: case 83 + 32:  key = GS_KEYCODE_S; break;
        case 84: case 84 + 32:  key = GS_KEYCODE_T; break;
        case 85: case 85 + 32:  key = GS_KEYCODE_U; break;
        case 86: case 86 + 32:  key = GS_KEYCODE_V; break;
        case 87: case 87 + 32:  key = GS_KEYCODE_W; break;
        case 88: case 88 + 32:  key = GS_KEYCODE_X; break;
        case 89: case 89 + 32:  key = GS_KEYCODE_Y; break;
        case 90: case 90 + 32:  key = GS_KEYCODE_Z; break;
        case 91:  key = GS_KEYCODE_LEFT_BRACKET; break;
        case 92:  key = GS_KEYCODE_BACKSLASH; break;
        case 93:  key = GS_KEYCODE_RIGHT_BRACKET; break;
        case 96:  key = GS_KEYCODE_GRAVE_ACCENT; break;
        case 161: key = GS_KEYCODE_WORLD_1; break;
        case 162: key = GS_KEYCODE_WORLD_2; break;
        case 27: key = GS_KEYCODE_ESC; break;
        case 13: key = GS_KEYCODE_ENTER; break;
        case 9: key = GS_KEYCODE_TAB; break;
        case 8: key = GS_KEYCODE_BACKSPACE; break;
        case 260: key = GS_KEYCODE_INSERT; break;
        case 261: key = GS_KEYCODE_DELETE; break;
        case 37: key = GS_KEYCODE_LEFT; break; 
        case 38: key = GS_KEYCODE_UP; break; 
        case 39: key = GS_KEYCODE_RIGHT; break; 
        case 40: key = GS_KEYCODE_DOWN; break;
        case 266: key = GS_KEYCODE_PAGE_UP; break;
        case 267: key = GS_KEYCODE_PAGE_DOWN; break;
        case 268: key = GS_KEYCODE_HOME; break;    
        case 269: key = GS_KEYCODE_END; break;    
        case 280: key = GS_KEYCODE_CAPS_LOCK; break; 
        case 281: key = GS_KEYCODE_SCROLL_LOCK; break;
        case 282: key = GS_KEYCODE_NUM_LOCK; break;  
        case 283: key = GS_KEYCODE_PRINT_SCREEN; break;
        case 284: key = GS_KEYCODE_PAUSE; break;      
        case 290: key = GS_KEYCODE_F1; break;        
        case 291: key = GS_KEYCODE_F2; break;       
        case 292: key = GS_KEYCODE_F3; break;      
        case 293: key = GS_KEYCODE_F4; break;     
        case 294: key = GS_KEYCODE_F5; break;    
        case 295: key = GS_KEYCODE_F6; break;   
        case 296: key = GS_KEYCODE_F7; break;  
        case 297: key = GS_KEYCODE_F8; break; 
        case 298: key = GS_KEYCODE_F9; break;
        case 299: key = GS_KEYCODE_F10; break;
        case 300: key = GS_KEYCODE_F11; break;
        case 301: key = GS_KEYCODE_F12; break;
        case 302: key = GS_KEYCODE_F13; break;
        case 303: key = GS_KEYCODE_F14; break;
        case 304: key = GS_KEYCODE_F15; break;
        case 305: key = GS_KEYCODE_F16; break;
        case 306: key = GS_KEYCODE_F17; break;
        case 307: key = GS_KEYCODE_F18; break;
        case 308: key = GS_KEYCODE_F19; break;
        case 309: key = GS_KEYCODE_F20; break;
        case 310: key = GS_KEYCODE_F21; break;
        case 311: key = GS_KEYCODE_F22; break;
        case 312: key = GS_KEYCODE_F23; break;
        case 313: key = GS_KEYCODE_F24; break;
        case 314: key = GS_KEYCODE_F25; break;
        case 320: key = GS_KEYCODE_KP_0; break;
        case 321: key = GS_KEYCODE_KP_1; break;
        case 322: key = GS_KEYCODE_KP_2; break;
        case 323: key = GS_KEYCODE_KP_3; break;
        case 324: key = GS_KEYCODE_KP_4; break;
        case 325: key = GS_KEYCODE_KP_5; break;
        case 326: key = GS_KEYCODE_KP_6; break;
        case 327: key = GS_KEYCODE_KP_7; break;
        case 328: key = GS_KEYCODE_KP_8; break;
        case 329: key = GS_KEYCODE_KP_9; break;
        case 330: key = GS_KEYCODE_KP_DECIMAL; break;
        case 331: key = GS_KEYCODE_KP_DIVIDE; break;
        case 332: key = GS_KEYCODE_KP_MULTIPLY; break;
        case 333: key = GS_KEYCODE_KP_SUBTRACT; break;
        case 334: key = GS_KEYCODE_KP_ADD; break;    
        case 335: key = GS_KEYCODE_KP_ENTER; break;  
        case 336: key = GS_KEYCODE_KP_EQUAL; break;  
        case 16: key = GS_KEYCODE_LEFT_SHIFT; break; 
        case 17: key = GS_KEYCODE_LEFT_CONTROL; break; 
        case 18: key = GS_KEYCODE_LEFT_ALT; break;    
        case 343: key = GS_KEYCODE_LEFT_SUPER; break; 
        case 347: key = GS_KEYCODE_RIGHT_SUPER; break;  
        case 348: key = GS_KEYCODE_MENU; break;        
    }
    return key;
}

EM_BOOL gs_ems_size_changed_cb(int32_t type, const EmscriptenUiEvent* evt, void* user_data)
{
    gs_println("size changed");
    gs_platform_t* platform = gs_engine_subsystem(platform);
    gs_ems_t* ems = (gs_ems_t*)platform->user_data;
    (void)type;
    (void)evt;
    (void)user_data;
    // gs_println("was: <%.2f, %.2f>", (float)ems->canvas_width, (float)ems->canvas_height);
    emscripten_get_element_css_size(ems->canvas_name, &ems->canvas_width, &ems->canvas_height);
    emscripten_set_canvas_element_size(ems->canvas_name, ems->canvas_width, ems->canvas_height);
    // gs_println("is: <%.2f, %.2f>", (float)ems->canvas_width, (float)ems->canvas_height);
    return true;
}

EM_BOOL gs_ems_fullscreenchange_cb(int32_t type, const EmscriptenFullscreenChangeEvent* evt, void* user_data)
{
    (void)user_data;
    (void)type;
    gs_ems_t* ems = GS_EMS_DATA();
    // emscripten_get_element_css_size(ems->canvas_name, &ems->canvas_width, &ems->canvas_height);
    if (evt->isFullscreen) {
        EmscriptenFullscreenStrategy strategy;
        strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF;
        strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
        strategy.canvasResizedCallback = gs_ems_size_changed_cb;
        emscripten_enter_soft_fullscreen(ems->canvas_name, &strategy);
        // gs_println("fullscreen!");
        // emscripten_enter_soft_fullscreen(ems->canvas_name, NULL);
        // ems->canvas_width = (float)evt->screenWidth;
        // ems->canvas_height = (float)evt->screenHeight;
        // emscripten_set_canvas_element_size(ems->canvas_name, ems->canvas_width, ems->canvas_height);
    } else {
        emscripten_exit_fullscreen();
        emscripten_set_canvas_element_size(ems->canvas_name, 800, 600);
    }
}

EM_BOOL gs_ems_key_cb(int32_t type, const EmscriptenKeyboardEvent* evt, void* user_data)
{
    (void)user_data;

    // Push back event into platform events
    gs_platform_event_t gs_evt = gs_default_val();
    gs_evt.type = GS_PLATFORM_EVENT_KEY;
    gs_evt.key.codepoint = evt->which;
    gs_evt.key.keycode = gs_platform_codepoint_to_key(evt->which);

    // gs_println("codepoint: %zu", evt->which);

    switch (type)
    {
        case EMSCRIPTEN_EVENT_KEYPRESS:
        {
            gs_evt.key.action = GS_PLATFORM_KEY_PRESSED;
        } break;

        case EMSCRIPTEN_EVENT_KEYDOWN: 
        {
            gs_evt.key.action = GS_PLATFORM_KEY_DOWN;
        } break;

        case EMSCRIPTEN_EVENT_KEYUP:
        {
            gs_evt.key.action = GS_PLATFORM_KEY_RELEASED;
        } break;

        default: break;
    }

    // Add action
    gs_platform_add_event(&gs_evt);

    return evt->which < 32;
}

EM_BOOL gs_ems_mouse_cb(int32_t type, const EmscriptenMouseEvent* evt, void* user_data)
{
    (void)user_data;

    gs_platform_t* platform = gs_engine_subsystem(platform);
    gs_ems_t* ems = GS_EMS_DATA();

    gs_platform_mouse_button_code button = GS_MOUSE_LBUTTON;
    switch (evt->button) {
        case 0: button = GS_MOUSE_LBUTTON; break;
        case 1: button = GS_MOUSE_MBUTTON; break;
        case 2: button = GS_MOUSE_RBUTTON; break;
    }

    // Push back event into platform events
    gs_platform_event_t gs_evt = gs_default_val();
    gs_evt.type = GS_PLATFORM_EVENT_MOUSE;
    gs_evt.mouse.codepoint = evt->button;
    gs_evt.mouse.button = button;

    switch (type)
    {
        case EMSCRIPTEN_EVENT_CLICK:
        {
            gs_evt.mouse.action = GS_PLATFORM_MOUSE_BUTTON_PRESSED;
        } break;

        // Emscripten doesn't register continuous presses, so have to manually store this state
        case EMSCRIPTEN_EVENT_MOUSEDOWN: 
        {
            gs_evt.mouse.action = GS_PLATFORM_MOUSE_BUTTON_DOWN;
            ems->mouse_down[(int32_t)button] = true; 
        } break;

        case EMSCRIPTEN_EVENT_MOUSEUP: 
        {
            gs_evt.mouse.action = GS_PLATFORM_MOUSE_BUTTON_RELEASED;
            ems->mouse_down[(int32_t)button] = false; 
        } break;

        case EMSCRIPTEN_EVENT_MOUSEMOVE:
        {
            gs_evt.mouse.action = GS_PLATFORM_MOUSE_MOVE;
            if (platform->input.mouse.locked) {
                gs_evt.mouse.move = gs_v2((float)evt->movementX, (float)evt->movementY);
            } else {
                gs_evt.mouse.move = gs_v2((float)evt->targetX, (float)evt->targetY);
            }
        } break;

        case EMSCRIPTEN_EVENT_MOUSEENTER:
        {
            gs_evt.mouse.action = GS_PLATFORM_MOUSE_ENTER;
            // Release all buttons
            ems->mouse_down[0] = false;
            ems->mouse_down[1] = false;
            ems->mouse_down[2] = false;
        } break;

        case EMSCRIPTEN_EVENT_MOUSELEAVE:
        {
            gs_evt.mouse.action = GS_PLATFORM_MOUSE_LEAVE;
            // Release all buttons
            ems->mouse_down[0] = false;
            ems->mouse_down[1] = false;
            ems->mouse_down[2] = false;
        } break;

        default:
        {
        }break;
    }

    gs_platform_add_event(&gs_evt);

    return true;
}

EM_BOOL gs_ems_mousewheel_cb(int32_t type, const EmscriptenWheelEvent* evt, void* user_data)
{
    (void)type;
    (void)user_data;

    // Push back event into platform events
    gs_platform_event_t gs_evt = gs_default_val();
    gs_evt.type = GS_PLATFORM_EVENT_MOUSE;
    gs_evt.mouse.action = GS_PLATFORM_MOUSE_WHEEL;
    gs_evt.mouse.wheel = gs_v2((float)evt->deltaX, -(float)evt->deltaY);
    gs_platform_add_event(&gs_evt);

    return true;
}

EM_BOOL gs_ems_pointerlock_cb(int32_t type, const EmscriptenPointerlockChangeEvent* evt, void* user_data)
{
    (void)type;
    (void)user_data;
    gs_platform_t* platform = gs_engine_subsystem(platform);
    platform->input.mouse.locked = evt->isActive;
    // gs_println("lock: %zu", platform->input.mouse.locked);
}

GS_API_DECL void       
gs_platform_init(gs_platform_t* platform)
{
    gs_println("Initializing Emscripten.");

    gs_app_desc_t* app = gs_engine_app();
    platform->user_data = gs_malloc_init(gs_ems_t);
    gs_ems_t* ems = (gs_ems_t*)platform->user_data;

    // ems->canvas_width = app->window_width;
    // ems->canvas_height = app->window_height;
    // double dpi = emscripten_get_device_pixel_ratio();

    // Just set this to defaults for now
    ems->canvas_name = "#canvas";
    emscripten_set_canvas_element_size(ems->canvas_name, app->window_width, app->window_height);
    emscripten_get_element_css_size(ems->canvas_name, &ems->canvas_width, &ems->canvas_height);

    // Set up callbacks
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, gs_ems_fullscreenchange_cb);
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, gs_ems_size_changed_cb);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, gs_ems_key_cb);
    emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, gs_ems_key_cb);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, gs_ems_key_cb);
    emscripten_set_click_callback(ems->canvas_name, NULL, true, gs_ems_mouse_cb);
    emscripten_set_mouseenter_callback(ems->canvas_name, NULL, true, gs_ems_mouse_cb);
    emscripten_set_mouseleave_callback(ems->canvas_name, NULL, true, gs_ems_mouse_cb);
    emscripten_set_mousedown_callback(ems->canvas_name, NULL, true, gs_ems_mouse_cb);
    emscripten_set_mouseup_callback(ems->canvas_name, NULL, true, gs_ems_mouse_cb);
    emscripten_set_mousemove_callback(ems->canvas_name, NULL, true, gs_ems_mouse_cb);
    emscripten_set_wheel_callback(ems->canvas_name, NULL, true, gs_ems_mousewheel_cb);
    emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, true, gs_ems_pointerlock_cb);

    // Set up webgl context
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.antialias = false;
    attrs.depth = true;
    attrs.premultipliedAlpha = false;
    attrs.stencil = true;
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;
    attrs.enableExtensionsByDefault = true;
    ems->ctx = emscripten_webgl_create_context(ems->canvas_name, &attrs);
    if (!ems->ctx) {
        gs_println("Emscripten Init: Unable to create webgl2 context. Reverting to webgl1.");
        attrs.majorVersion = 1;
        ems->ctx = emscripten_webgl_create_context(ems->canvas_name, &attrs);
    } else {
        gs_println("Emscripten Init: Successfully created webgl2 context.");
    }
    if (emscripten_webgl_make_context_current(ems->ctx) != EMSCRIPTEN_RESULT_SUCCESS) {
        gs_println("Emscripten Init: Unable to set current webgl context.");
    }
}

GS_API_DECL void                 
gs_platform_lock_mouse(uint32_t handle, bool32_t lock)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);
    gs_ems_t* ems = (gs_ems_t*)platform->user_data;
    // if (platform->input.mouse.locked == lock) return;
    platform->input.mouse.locked = lock;
    if (lock) {
        emscripten_request_pointerlock(ems->canvas_name, true);
    } else {
        emscripten_exit_pointerlock();
    }
}

GS_API_DECL void       
gs_platform_shutdown(gs_platform_t* platform)
{
    // Free memory
}

GS_API_DECL double 
gs_platform_elapsed_time()
{
    return emscripten_performance_now(); 
}

// Platform Video
GS_API_DECL void 
gs_platform_enable_vsync(int32_t enabled)
{
    // Nothing for now...
}

// Platform Util
GS_API_DECL void   
gs_platform_sleep(float ms)
{
    emscripten_sleep((uint32_t)ms);   
}

// Platform Input
GS_API_DECL void 
gs_platform_process_input(gs_platform_input_t* input)
{
    gs_ems_t* ems = GS_EMS_DATA();

    // Set mouse buttons
    for (uint32_t i = 0; i < GS_MOUSE_BUTTON_CODE_COUNT; ++i) {
        if (ems->mouse_down[i]) gs_platform_press_mouse_button((gs_platform_mouse_button_code)i);
        else                    gs_platform_release_mouse_button((gs_platform_mouse_button_code)i);
    }

    // Check for pointerlock, because Chrome is retarded.
    EmscriptenPointerlockChangeEvent evt = gs_default_val();
    emscripten_get_pointerlock_status(&evt);
    if (gs_platform_mouse_locked() && !evt.isActive) {
        gs_engine_subsystem(platform)->input.mouse.locked = false;
    }
}

GS_API_DECL void      
gs_platform_mouse_set_position(uint32_t handle, float x, float y)
{
    // Not sure this is possible...
    struct gs_platform_t* platform = gs_engine_subsystem(platform);
    platform->input.mouse.position = gs_v2(x, y);
}

GS_API_DECL void*    
gs_platform_create_window_internal(const char* title, uint32_t width, uint32_t height, uint32_t monitor_index)
{
    // Nothing for now, since we just create this internally...
    return NULL;
}

GS_API_DECL void     
gs_platform_window_swap_buffer(uint32_t handle)
{
    // Nothing for emscripten...but could handle swapping manually if preferred.
}

GS_API_DECL gs_vec2  
gs_platform_window_sizev(uint32_t handle)
{
    gs_ems_t* ems = GS_EMS_DATA();
    return gs_v2((float)ems->canvas_width, (float)ems->canvas_height);
}

GS_API_DECL void     
gs_platform_window_size(uint32_t handle, uint32_t* w, uint32_t* h)
{
    gs_ems_t* ems = GS_EMS_DATA();
    *w = (uint32_t)ems->canvas_width;
    *h = (uint32_t)ems->canvas_height;
}

GS_API_DECL uint32_t 
gs_platform_window_width(uint32_t handle)
{
    gs_ems_t* ems = GS_EMS_DATA();
    return (uint32_t)ems->canvas_width;
}

GS_API_DECL uint32_t 
gs_platform_window_height(uint32_t handle)
{
    gs_ems_t* ems = GS_EMS_DATA();
    return (uint32_t)ems->canvas_height;
}

GS_API_DECL bool32_t
gs_platform_window_fullscreen(uint32_t handle)
{
    return false;
}

GS_API_DECL void
gs_platform_window_position(uint32_t handle, uint32_t* x, uint32_t* y)
{
}

GS_API_DECL gs_vec2
gs_platform_window_positionv(uint32_t handle)
{
    return gs_v2(0, 0);
}

GS_API_DECL void     
gs_platform_set_window_size(uint32_t handle, uint32_t width, uint32_t height)
{
    gs_ems_t* ems = GS_EMS_DATA();
    emscripten_set_canvas_element_size(ems->canvas_name, width, height);
    ems->canvas_width = (uint32_t)width;
    ems->canvas_height = (uint32_t)height;
}

GS_API_DECL void     
gs_platform_set_window_sizev(uint32_t handle, gs_vec2 v)
{
    gs_ems_t* ems = GS_EMS_DATA();
    emscripten_set_canvas_element_size(ems->canvas_name, (uint32_t)v.x, (uint32_t)v.y);
    ems->canvas_width = (uint32_t)v.x;
    ems->canvas_height = (uint32_t)v.y;
}

GS_API_DECL void
gs_platform_set_window_fullscreen(uint32_t handle, bool32_t fullscreen)
{
}

GS_API_DECL void
gs_platform_set_window_position(uint32_t handle, uint32_t x, uint32_t y)
{
}

GS_API_DECL void
gs_platform_set_window_positionv(uint32_t handle, gs_vec2 v)
{
}

GS_API_DECL void     
gs_platform_set_cursor(uint32_t handle, gs_platform_cursor cursor)
{
}

GS_API_DECL void     
gs_platform_set_dropped_files_callback(uint32_t handle, gs_dropped_files_callback_t cb)
{
}

GS_API_DECL void     
gs_platform_set_window_close_callback(uint32_t handle, gs_window_close_callback_t cb)
{
}

GS_API_DECL void     
gs_platform_set_character_callback(uint32_t handle, gs_character_callback_t cb)
{
}

GS_API_DECL void*    
gs_platform_raw_window_handle(uint32_t handle)
{
    return NULL;
}

GS_API_DECL void     
gs_platform_framebuffer_size(uint32_t handle, uint32_t* w, uint32_t* h)
{
    gs_ems_t* ems = GS_EMS_DATA();
    // double dpi = emscripten_get_device_pixel_ratio();
    *w = (uint32_t)(ems->canvas_width);
    *h = (uint32_t)(ems->canvas_height);
}

GS_API_DECL gs_vec2  
gs_platform_framebuffer_sizev(uint32_t handle)
{
    uint32_t w = 0, h = 0;
    gs_platform_framebuffer_size(handle, &w, &h);
    return gs_v2(w, h);
}

GS_API_DECL uint32_t 
gs_platform_framebuffer_width(uint32_t handle)
{
    // Get ems width for now. Don't use handle.
    gs_ems_t* ems = GS_EMS_DATA();
    return (uint32_t)ems->canvas_width;
}

GS_API_DECL uint32_t 
gs_platform_framebuffer_height(uint32_t handle)
{
    gs_ems_t* ems = GS_EMS_DATA();
    return (uint32_t)ems->canvas_height;
}

#ifndef GS_NO_HIJACK_MAIN
    int32_t main(int32_t argc, char** argv)
    {
        gs_app_desc_t app = gs_main(argc, argv);
        gs_engine_create(app);
        emscripten_set_main_loop(gs_engine_frame, (int32_t)app.frame_rate, true);
        return 0;
    }
#endif // GS_NO_HIJACK_MAIN

#undef GS_PLATFORM_IMPL_EMSCRIPTEN
#endif // GS_PLATFORM_IMPL_EMSCRIPTEN

/*=======================
// Android Implemenation
========================*/

// Modified from sokol_app.h android impl

#ifdef GS_PLATFORM_IMPL_ANDROID

// Platform includes
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <android/native_activity.h>
//#include <android_native_app_glue.h>
#include <android/looper.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#ifndef GL_EXT_PROTOTYPES
    #define GL_GLEXT_PROTOTYPES
#endif

typedef enum gs_android_message_type {
    GS_ANDROID_MSG_CREATE,
    GS_ANDROID_MSG_START,
    GS_ANDROID_MSG_STOP,
    GS_ANDROID_MSG_RESUME,
    GS_ANDROID_MSG_PAUSE,
    GS_ANDROID_MSG_GAIN_FOCUS,
    GS_ANDROID_MSG_LOSE_FOCUS,
    GS_ANDROID_MSG_SET_NATIVE_WINDOW,
    GS_ANDROID_MSG_SET_INPUT_QUEUE,
    GS_ANDROID_MSG_DESTROY
} gs_android_message_type;

typedef struct gs_android_thread_t {
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int32_t read_from_main_fd;
    int32_t write_from_main_fd;
    bool is_thread_started;
    bool is_thread_stopping;
    bool is_thread_stopped;
    bool padding;
} gs_android_thread_t;

typedef struct gs_android_resources_t {
    ANativeWindow* window;
    AInputQueue* input;
} gs_android_resources_t;

typedef struct gs_android_egl_t {
    int32_t width;
    int32_t height;
    int32_t fbwidth;
    int32_t fbheight;
    EGLConfig config;
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
} gs_android_egl_t;

typedef struct gs_android_state_t {
    bool has_created;
    bool has_resumed;
    bool has_focus;
    bool padding;
} gs_android_state_t;

typedef struct gs_android_t {
    ANativeActivity* activity;
    gs_android_thread_t gt;
    gs_android_resources_t pending;
    gs_android_resources_t current;
    ALooper* looper; 
    gs_android_egl_t egl;
    gs_android_state_t state;
    const char* internal_data_path;
} gs_android_t;

#define GS_PLATFORM_ANDROID_DATA(...)\
    ((gs_android_t*)gs_engine_subsystem(platform)->user_data)

bool gs_android_should_update(gs_android_t* android)
{
    return (android->state.has_focus && android->state.has_resumed && android->egl.surface != EGL_NO_SURFACE);
}

void gs_android_shutdown(gs_android_t* android)
{
    android->gt.is_thread_stopping = true;
}

bool gs_android_init_egl(gs_android_t* android)
{
    gs_assert(android->egl.display == EGL_NO_DISPLAY);
    gs_assert(android->egl.context == EGL_NO_CONTEXT);

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        return false;
    }
    if (eglInitialize(display, NULL, NULL) == EGL_FALSE) {
        return false;
    }

    EGLint alpha_size = 8;
    const EGLint cfg_attributes[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, alpha_size,
        EGL_DEPTH_SIZE, 16,
        EGL_STENCIL_SIZE, 0,
        EGL_NONE,
    };
    EGLConfig available_cfgs[32];
    EGLint cfg_count;
    eglChooseConfig(display, cfg_attributes, available_cfgs, 32, &cfg_count);
    gs_assert(cfg_count > 0);
    gs_assert(cfg_count <= 32);

    /* find config with 8-bit rgb buffer if available, ndk sample does not trust egl spec */
    EGLConfig config;
    bool exact_cfg_found = false;
    for (int i = 0; i < cfg_count; ++i) {
        EGLConfig c = available_cfgs[i];
        EGLint r, g, b, a, d;
        if (eglGetConfigAttrib(display, c, EGL_RED_SIZE, &r) == EGL_TRUE &&
            eglGetConfigAttrib(display, c, EGL_GREEN_SIZE, &g) == EGL_TRUE &&
            eglGetConfigAttrib(display, c, EGL_BLUE_SIZE, &b) == EGL_TRUE &&
            eglGetConfigAttrib(display, c, EGL_ALPHA_SIZE, &a) == EGL_TRUE &&
            eglGetConfigAttrib(display, c, EGL_DEPTH_SIZE, &d) == EGL_TRUE &&
            r == 8 && g == 8 && b == 8 && (alpha_size == 0 || a == alpha_size) && d == 16) {
            exact_cfg_found = true;
            config = c;
            break;
        }
    }
    if (!exact_cfg_found) {
        config = available_cfgs[0];
    }

    // Force GLES3 for now
    EGLint ctx_attributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE,
    };
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctx_attributes);
    if (context == EGL_NO_CONTEXT) {
        return false;
    }

    gs_println("EGL: Successfully initialized.");

    android->egl.config = config;
    android->egl.display = display;
    android->egl.context = context;
    return true;
}

bool gs_android_create_egl_surface(gs_android_t* android, ANativeWindow* window)
{
    gs_assert(android);
    gs_assert(android->egl.display != EGL_NO_DISPLAY);
    gs_assert(android->egl.context != EGL_NO_CONTEXT);
    gs_assert(window);

    // Construct egl surface, make current
    EGLSurface surface = eglCreateWindowSurface(android->egl.display, android->egl.config, window, NULL);
    if (surface == EGL_NO_SURFACE) {
        return false;
    }
    if (eglMakeCurrent(android->egl.display, surface, surface, android->egl.context) == EGL_FALSE) {
        return false;
    }
    android->egl.surface = surface;
    return true;
}

bool gs_android_destroy_egl_surface(gs_android_t* android)
{
    if (android->egl.display == EGL_NO_DISPLAY) {
        return false;
    }

    eglMakeCurrent(android->egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (android->egl.surface != EGL_NO_SURFACE) {
        eglDestroySurface(android->egl.display, android->egl.surface);
        android->egl.surface = EGL_NO_SURFACE;
    }
    return true;
}

void gs_android_update_dimensions(gs_android_t* android, ANativeWindow* window, bool force_update)
{
    gs_assert(android->egl.display != EGL_NO_DISPLAY);
    gs_assert(android->egl.context != EGL_NO_CONTEXT);
    gs_assert(android->egl.surface != EGL_NO_SURFACE);
    gs_assert(window);

    const int32_t win_w = ANativeWindow_getWidth(window);
    const int32_t win_h = ANativeWindow_getHeight(window);
    gs_assert(win_w >= 0 && win_h >= 0);
    const bool win_changed = (win_w != android->egl.width) || (win_h != android->egl.height);
    android->egl.width = win_w;
    android->egl.height = win_h;
    if (win_changed || force_update) {
        if (true) { // High dpi setting
            /*
            const int32_t buf_w = win_w / 2;
            const int32_t buf_h = win_h / 2;
            EGLint format;
            EGLBoolean egl_result = eglGetConfigAttrib(android->egl.display, android->egl.config, EGL_NATIVE_VISUAL_ID, &format);
            gs_assert(egl_result == EGL_TRUE);
            int32_t result = ANativeWindow_setBuffersGeometry(window, buf_w, buf_h, format);
            gs_assert(result == 0);
            */
        }
    }

    /* query surface size */
    EGLint fb_w, fb_h;
    EGLBoolean egl_result_w = eglQuerySurface(android->egl.display, android->egl.surface, EGL_WIDTH, &fb_w);
    EGLBoolean egl_result_h = eglQuerySurface(android->egl.display, android->egl.surface, EGL_HEIGHT, &fb_h);
    gs_assert(egl_result_w == EGL_TRUE);
    gs_assert(egl_result_h == EGL_TRUE);
    const bool fb_changed = (fb_w != android->egl.fbwidth) || (fb_h != android->egl.fbheight);
    android->egl.fbwidth = fb_w;
    android->egl.fbheight = fb_h;
    gs_println("fb: <%.2f, %.2f>, ws: <%.2f, %.2f>", (float)fb_w, (float)fb_h, (float)win_w, (float)win_h);
    // _sapp.dpi_scale = (float)_sapp.framebuffer_width / (float)_sapp.window_width;
    if (win_changed || fb_changed || force_update) {
        /*
         if (!_sapp.first_frame) {
             SOKOL_LOG("SAPP_EVENTTYPE_RESIZED");
             _sapp_android_app_event(SAPP_EVENTTYPE_RESIZED);
         }
         */
    }
}

bool gs_android_touch_event(AInputEvent* evt)
{
    gs_android_t* android = GS_PLATFORM_ANDROID_DATA();
    gs_assert(android);
    gs_assert(android->current.input);

    if (AInputEvent_getType(evt) != AINPUT_EVENT_TYPE_MOTION) {
        return false;
    }
    // Construct touch events here and call add into platform layer
    int32_t action_idx = AMotionEvent_getAction(evt);
    int32_t action = action_idx & AMOTION_EVENT_ACTION_MASK;

    // Construct platform event to add
    gs_platform_event_t gsevt = gs_default_val();
    gsevt.type = GS_PLATFORM_EVENT_TOUCH;

    switch (action) {

        case AMOTION_EVENT_ACTION_DOWN:
        case AMOTION_EVENT_ACTION_POINTER_DOWN: {
            gsevt.touch.action = GS_PLATFORM_TOUCH_DOWN;
        } break;

        case AMOTION_EVENT_ACTION_UP:
        case AMOTION_EVENT_ACTION_POINTER_UP: {
            gsevt.touch.action = GS_PLATFORM_TOUCH_UP;
        } break;

        case AMOTION_EVENT_ACTION_MOVE: {
            gsevt.touch.action = GS_PLATFORM_TOUCH_MOVE;
        } break;

        case AMOTION_EVENT_ACTION_CANCEL: {
            gsevt.touch.action = GS_PLATFORM_TOUCH_CANCEL;
        } break;

        default: {
            return false;
        } break;
    }

    int32_t idx = action_idx >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
    uint32_t num_touches = (uint32_t)AMotionEvent_getPointerCount(evt);

    if (num_touches > GS_PLATFORM_MAX_TOUCH) {
        num_touches = GS_PLATFORM_MAX_TOUCH;
    }

    for (uint32_t i = 0; i < num_touches; ++i) {
        gsevt.touch.point.id = (uintptr_t)AMotionEvent_getPointerId(evt, (size_t)i);
        gsevt.touch.point.position.x = (AMotionEvent_getRawX(evt, (size_t)i) / android->egl.width) * android->egl.fbwidth;
        gsevt.touch.point.position.y = (AMotionEvent_getRawY(evt, (size_t)i) / android->egl.height) * android->egl.fbheight;
        if (action == AMOTION_EVENT_ACTION_POINTER_DOWN ||
            action == AMOTION_EVENT_ACTION_POINTER_UP) {
            gsevt.touch.point.changed = (uint16_t)(i == idx);
        } else {
            gsevt.touch.point.changed = (uint16_t)true;
        }
        gs_platform_add_event(&gsevt);
    }

    return true;
}

bool gs_android_key_event(AInputEvent* evt)
{
    if (AInputEvent_getType(evt) != AINPUT_EVENT_TYPE_KEY) {
        return false;
    }
    // Shutdown event?
    if (AKeyEvent_getKeyCode(evt) == AKEYCODE_BACK) {
        gs_println("Android: back key pressed");
        return false;
    }

    return false;
}

int32_t gs_android_input_cb(int32_t fd, int32_t events, void* data)
{
    gs_android_t* android = (gs_android_t*)data;
    gs_assert(android);
    gs_assert(android->current.input);

    if ((events & ALOOPER_EVENT_INPUT) == 0) {
        gs_println("Android Main CB:Unsupported event: %zu", events);
        return 1;
    }

    // Poll for all input events
    AInputEvent* evt = NULL;
    while (AInputQueue_getEvent(android->current.input, &evt) >= 0)
    {
        if (AInputQueue_preDispatchEvent(android->current.input, evt) != 0) {
            continue;
        }

        // Register event with platform events
        int32_t handled = 0;
        if (gs_android_touch_event(evt) || gs_android_key_event(evt)) {
            handled = 1;
        }
        // Finish events
        AInputQueue_finishEvent(android->current.input, evt, handled);
    }

    return 1;
}

int32_t gs_android_main_cb(int32_t fd, int32_t events, void* data) 
{
    gs_android_t* android = (gs_android_t*)data;
    gs_assert(android);

    if ((events & ALOOPER_EVENT_INPUT) == 0) {
        gs_println("Android Main CB:Unsupported event: %zu", events);
        return 1;
    }

    gs_android_message_type msg;
    if (read(fd, &msg, sizeof(msg)) != sizeof(msg)) {
        gs_println("Could not write to read_from_main_fd");
        return 1;
    }

    gs_platform_event_t gsevt = gs_default_val();

    pthread_mutex_lock(&android->gt.mutex);

    switch (msg) 
    {
        case GS_ANDROID_MSG_CREATE: {
            gs_println("MSG_CREATE");
            android->state.has_created = true;
        } break;

        case GS_ANDROID_MSG_START: {
            gs_println("MSG_START");
            android->state.has_resumed = true;
            if (gs_engine_instance()) {
                gsevt.type = GS_PLATFORM_EVENT_APP;
                gsevt.app.action = GS_PLATFORM_APP_START;
                gs_platform_add_event(&gsevt);
            }
        } break;

        case GS_ANDROID_MSG_STOP: {
            gs_println("MSG_STOP");
            android->state.has_resumed = false;
            if (gs_engine_instance()) {
                gsevt.type = GS_PLATFORM_EVENT_APP;
                gsevt.app.action = GS_PLATFORM_APP_STOP;
                gs_platform_add_event(&gsevt);
            }
        } break;

        case GS_ANDROID_MSG_RESUME: {
            gs_println("MSG_RESUME");
            android->state.has_resumed = true;
            if (gs_engine_instance()) {
                gsevt.type = GS_PLATFORM_EVENT_APP;
                gsevt.app.action = GS_PLATFORM_APP_RESUME;
                gs_platform_add_event(&gsevt);
            }
        } break;

        case GS_ANDROID_MSG_PAUSE: {
            gs_println("MSG_PAUSE");
            android->state.has_resumed = false;
            if (gs_engine_instance()) {
                gsevt.type = GS_PLATFORM_EVENT_APP;
                gsevt.app.action = GS_PLATFORM_APP_PAUSE;
                gs_platform_add_event(&gsevt);
            }
        } break;

        case GS_ANDROID_MSG_GAIN_FOCUS: {
            gs_println("MSG_GAIN_FOCUS");
            android->state.has_focus = true;
            if (gs_engine_instance()) {
                gsevt.type = GS_PLATFORM_EVENT_WINDOW;
                gsevt.app.action = GS_PLATFORM_WINDOW_GAIN_FOCUS;
                gs_platform_add_event(&gsevt);
            }
        } break;

        case GS_ANDROID_MSG_LOSE_FOCUS: {
            gs_println("MSG_NO_FOCUS");
            android->state.has_focus = false;
            if (gs_engine_instance()) {
                gsevt.type = GS_PLATFORM_EVENT_WINDOW;
                gsevt.app.action = GS_PLATFORM_WINDOW_LOSE_FOCUS;
                gs_platform_add_event(&gsevt);
            }
        } break;

        case GS_ANDROID_MSG_SET_NATIVE_WINDOW:
        {
            if (!gs_engine_instance()) {
                gs_println("Creating engine...");
                gs_app_desc_t app = gs_main(0, NULL);
                app.android.activity = android;
                app.android.internal_data_path = android->internal_data_path;
                gs_engine_create(app);
                android->state.has_focus = true;
                android->state.has_resumed = true;
            }
            else {
                gs_println("MSG_SET_NATIVE_WINDOW");
                if (android->current.window != android->pending.window) {
                    if (android->current.window != NULL) {
                        gs_android_destroy_egl_surface(android);
                    }
                    if (android->pending.window != NULL) {
                        gs_println("Creating egl surface ...");
                        if (gs_android_create_egl_surface(android, android->pending.window)) {
                            gs_println("... ok!");
                            gs_android_update_dimensions(android, android->pending.window, true);
                        } else {
                            gs_println("... failed!");
                            gs_engine_quit();
                        }
                    }
                }
            }
            android->current.window = android->pending.window;
        } break;

        case GS_ANDROID_MSG_SET_INPUT_QUEUE: 
        {
            gs_println("MSG_SET_INPUT_QUEUE");
             if (android->current.input != android->pending.input) {
                 if (android->current.input != NULL) {
                     AInputQueue_detachLooper(android->current.input);
                 }
                 if (android->pending.input != NULL) {
                     AInputQueue_attachLooper(
                         android->pending.input,
                         android->looper,
                         ALOOPER_POLL_CALLBACK,
                         gs_android_input_cb,
                         android); /* data */
                 }
             }
             android->current.input = android->pending.input;
        } break;

        case GS_ANDROID_MSG_DESTROY: {
            gs_println("MSG_DESTROY");
            if (gs_engine_instance()) {
                gsevt.type = GS_PLATFORM_EVENT_WINDOW;
                gsevt.app.action = GS_PLATFORM_WINDOW_DESTROY;
                gs_platform_add_event(&gsevt);
            }
            gs_engine_quit();
        } break;

        default: {
            gs_println("Unknown msg type received");
        } break;
    }

    pthread_cond_broadcast(&android->gt.cond);
    pthread_mutex_unlock(&android->gt.mutex);

    return 1;
}

void gs_android_msg(gs_android_t* android, gs_android_message_type msg) 
{
    gs_assert(android);
    if (write(android->gt.write_from_main_fd, &msg, sizeof(msg)) != sizeof(msg)) {
        gs_println("Could not write to write_from_main_fd");
    }
    gs_println("Write message sucessfully");
}

void gs_android_msg_set_native_window(ANativeActivity* activity, ANativeWindow* window)
{
    gs_android_t* android = (gs_android_t*)activity->instance;
    gs_assert(android);
    pthread_mutex_lock(&android->gt.mutex);
    android->pending.window = window;
    gs_android_msg(android, GS_ANDROID_MSG_SET_NATIVE_WINDOW);
    // Spin until we set the appropriate window
    while (android->current.window != window) {
        pthread_cond_wait(&android->gt.cond, &android->gt.mutex);
    }
    pthread_mutex_unlock(&android->gt.mutex);
}

void gs_android_msg_set_input_queue(ANativeActivity* activity, AInputQueue* input)
{
    gs_android_t* android = (gs_android_t*)activity->instance;
    gs_assert(android);
    pthread_mutex_lock(&android->gt.mutex);
    android->pending.input = input;
    gs_android_msg(android, GS_ANDROID_MSG_SET_INPUT_QUEUE);
    while (android->current.input != input) {
        pthread_cond_wait(&android->gt.cond, &android->gt.mutex);
    }
    pthread_mutex_unlock(&android->gt.mutex);
}

void gs_android_native_window_created_cb(ANativeActivity* activity, ANativeWindow* window)
{
    gs_println("NativeActivity:onNativeWindowCreated()");
    gs_android_msg_set_native_window(activity, window);
}

void gs_android_window_focus_change_cb(ANativeActivity* activity, int32_t has_focus)
{
    gs_android_t* android = (gs_android_t*)activity->instance;
    gs_assert(android);
    gs_println("NativeActivity:onNativeWindowFocusChange()");
    switch (has_focus) {
        case false:  gs_android_msg(android, GS_ANDROID_MSG_LOSE_FOCUS); break;
        case true:   gs_android_msg(android, GS_ANDROID_MSG_GAIN_FOCUS); break;
    }
}

void gs_android_resume_cb(ANativeActivity* activity)
{
    gs_android_t* android = (gs_android_t*)activity->instance;
    gs_assert(android);
    gs_println("NativeActivity:onNativeResume()");
    gs_android_msg(android, GS_ANDROID_MSG_RESUME);
}

void gs_android_start_cb(ANativeActivity* activity)
{
    gs_android_t* android = (gs_android_t*)activity->instance;
    gs_assert(android);
    gs_println("NativeActivity:onNativeStart()");
    gs_android_msg(android, GS_ANDROID_MSG_START);
}

void gs_android_pause_cb(ANativeActivity* activity)
{
    gs_android_t* android = (gs_android_t*)activity->instance;
    gs_assert(android);
    gs_println("NativeActivity:onNativePause()");
    gs_android_msg(android, GS_ANDROID_MSG_PAUSE);
}

void gs_android_stop_cb(ANativeActivity* activity)
{
    gs_android_t* android = (gs_android_t*)activity->instance;
    gs_assert(android);
    gs_println("NativeActivity:onNativeStop()");
    gs_android_msg(android, GS_ANDROID_MSG_STOP);
}

void gs_android_input_queue_created_cb(ANativeActivity* activity, AInputQueue* queue)
{
    gs_android_msg_set_input_queue(activity, queue);
}

void gs_android_destroy_cb(ANativeActivity* activity)
{
    gs_android_t* android = (gs_android_t*)activity->instance;
    gs_assert(android);
    gs_println("NativeActivity:onNativeDestroy()");
    gs_android_msg(android, GS_ANDROID_MSG_DESTROY);
}

void gs_android_low_memory_cb(ANativeActivity* activity)
{
    gs_println("NativeActivity:onLowMemory()");
}

void gs_android_config_changed_cb(ANativeActivity* activity)
{
    gs_println("NativeActivity:onConfigurationChanged()");
}

GS_API_DECL void* gs_android_loop(void* arg)
{
    gs_println("main loop");

    gs_android_t* android = (gs_android_t*)arg;
    gs_assert(android);

    // Set up callback for polling events
    android->looper = ALooper_prepare(0);
    ALooper_addFd(android->looper, android->gt.read_from_main_fd, ALOOPER_POLL_CALLBACK, ALOOPER_EVENT_INPUT, gs_android_main_cb, android);

    // Signal to start main thread
    pthread_mutex_lock(&android->gt.mutex);
    android->gt.is_thread_started = true;
    pthread_cond_broadcast(&android->gt.cond);
    pthread_mutex_unlock(&android->gt.mutex);

    // Main application loop
    while (!android->gt.is_thread_stopping)
    {
        if (gs_android_should_update(android)) {
            gs_engine_frame();
        }

        // Process all window/input events as they occur
        bool process = true;
        while (process && !android->gt.is_thread_stopping) {
            bool block_until_evt = !android->gt.is_thread_stopping && !gs_android_should_update(android);
            process = ALooper_pollOnce(block_until_evt ? -1 : 0, NULL, NULL, NULL) == ALOOPER_POLL_CALLBACK;
        }
    }

    // Cleanup thread after done
    if (android->current.input != NULL) {
        AInputQueue_detachLooper(android->current.input);
    }

    pthread_mutex_lock(&android->gt.mutex);
    android->gt.is_thread_stopped = true;
    pthread_cond_broadcast(&android->gt.cond);
    pthread_mutex_unlock(&android->gt.mutex);
    gs_println("Android:Thread cleanup done.");

    return NULL;
}

JNIEXPORT
void ANativeActivity_onCreate(ANativeActivity* activity, void* saved_state, size_t saved_state_size)
{
    // Construct application instance and store in activity
    activity->instance = gs_malloc_init(gs_android_t);
    gs_android_t* android = (gs_android_t*)activity->instance;
    gs_assert(android);

    // Store internal data path
    gs_println("internal data path: %s", activity->internalDataPath);
    android->internal_data_path = activity->internalDataPath;

    int32_t pfd[2];
    if (pipe(pfd) != 0) {
        gs_println("Error:ANativeActivity_onCreate:Could not create thread pipe.");
        return;
    }

    android->gt.read_from_main_fd = pfd[0];
    android->gt.write_from_main_fd = pfd[1];

    pthread_mutex_init(&android->gt.mutex, NULL);
    pthread_cond_init(&android->gt.cond, NULL);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    gs_println("detach state");
    pthread_create(&android->gt.thread, &attr, gs_android_loop, android);
    gs_println("create pthread");
    pthread_attr_destroy(&attr);
    gs_println("destroy attribute");

    // Wait until main loop has started
    pthread_mutex_lock(&android->gt.mutex);
    while (!android->gt.is_thread_started) {
        pthread_cond_wait(&android->gt.cond, &android->gt.mutex);
    }
    pthread_mutex_unlock(&android->gt.mutex);

    // Send create message
    pthread_mutex_lock(&android->gt.mutex);
    gs_android_msg(android, GS_ANDROID_MSG_CREATE);
    while (!android->state.has_created) {
        pthread_cond_wait(&android->gt.cond, &android->gt.mutex);
    }
    pthread_mutex_unlock(&android->gt.mutex);

    // Register all callbacks
    activity->callbacks->onStart = gs_android_stop_cb;
    activity->callbacks->onResume = gs_android_resume_cb;
    activity->callbacks->onPause = gs_android_pause_cb;
    activity->callbacks->onStop = gs_android_stop_cb;
    activity->callbacks->onDestroy = gs_android_destroy_cb;
//    activity->callbacks->onSaveInstanceState = _sapp_android_on_save_instance_state;
    activity->callbacks->onNativeWindowCreated = gs_android_native_window_created_cb;
    activity->callbacks->onWindowFocusChanged = gs_android_window_focus_change_cb;
//    /* activity->callbacks->onNativeWindowResized = _sapp_android_on_native_window_resized; */
//    /* activity->callbacks->onNativeWindowRedrawNeeded = _sapp_android_on_native_window_redraw_needed; */
//    activity->callbacks->onNativeWindowDestroyed = _sapp_android_on_native_window_destroyed;
    activity->callbacks->onInputQueueCreated = gs_android_input_queue_created_cb;
//    activity->callbacks->onInputQueueDestroyed = _sapp_android_on_input_queue_destroyed;
//    /* activity->callbacks->onContentRectChanged = _sapp_android_on_content_rect_changed; */
    activity->callbacks->onConfigurationChanged = gs_android_config_changed_cb;
    activity->callbacks->onLowMemory = gs_android_low_memory_cb;

    gs_println("NativeActivity successfully created");
}

GS_API_DECL void            
gs_platform_init(gs_platform_t* platform)
{
    // Get from application platform data
    gs_android_t* android = (gs_android_t*)gs_engine_app()->android.activity;
    gs_assert(android);
    gs_app_desc_t* app = gs_engine_app();
    platform->user_data = android;
    gs_println("Android: Init platform");
    gs_android_init_egl(platform->user_data);
    gs_println("Creating egl surface ...");
    if (gs_android_create_egl_surface(android, android->pending.window)) {
        gs_println("... ok!");
        gs_android_update_dimensions(android, android->pending.window, true);
    } else {
        gs_println("... failed!");
        // _sapp_android_shutdown();
    }
}

GS_API_DECL void
gs_platform_shutdown(gs_platform_t* platform)
{
    gs_android_shutdown(platform->user_data);
}

// Platform Util
GS_API_DECL double
gs_platform_elapsed_time()
{
    gs_android_t* android = GS_PLATFORM_ANDROID_DATA();
    return ((double)clock() / (double)CLOCKS_PER_SEC) * 1000.0;
}

GS_API_DECL void   
gs_platform_sleep(float ms)
{
    usleep(ms * 1000.f);
}

// Platform Video
GS_API_DECL void 
gs_platform_enable_vsync(int32_t enabled)
{
}

// Platform Input
GS_API_DECL void                
gs_platform_process_input(gs_platform_input_t* input)
{
}

GS_API_DECL uint32_t             
gs_platform_key_to_codepoint(gs_platform_keycode code)
{
    return 0;
}

GS_API_DECL gs_platform_keycode  
gs_platform_codepoint_to_key(uint32_t code)
{
    return GS_KEYCODE_INVALID;
}

GS_API_DECL void                 
gs_platform_mouse_set_position(uint32_t handle, float x, float y)
{
}

GS_API_DECL void                 
gs_platform_lock_mouse(uint32_t handle, bool32_t lock)
{
}

GS_API_DECL void*    
gs_platform_create_window_internal(const char* title, uint32_t width, uint32_t height, uint32_t monitor_index)
{
    return NULL;
}

GS_API_DECL void     
gs_platform_window_swap_buffer(uint32_t handle)
{
    gs_android_t* android = GS_PLATFORM_ANDROID_DATA();
    if (gs_android_should_update(android)) {
        eglSwapBuffers(android->egl.display, android->egl.surface);
    }
}

GS_API_DECL gs_vec2  
gs_platform_window_sizev(uint32_t handle)
{
    gs_android_t* android = GS_PLATFORM_ANDROID_DATA();
    return gs_v2(android->egl.width, android->egl.height);
}

GS_API_DECL void     
gs_platform_window_size(uint32_t handle, uint32_t* width, uint32_t* height)
{
    gs_android_t* android = GS_PLATFORM_ANDROID_DATA();
    *width = android->egl.width;
    *height = android->egl.height;
}

GS_API_DECL uint32_t 
gs_platform_window_width(uint32_t handle)
{
    gs_android_t* android = GS_PLATFORM_ANDROID_DATA();
    return android->egl.width;
}

GS_API_DECL uint32_t 
gs_platform_window_height(uint32_t handle)
{
    gs_android_t* android = GS_PLATFORM_ANDROID_DATA();
    return android->egl.height;
}

GS_API_DECL bool32_t
gs_platform_window_fullscreen(uint32_t handle)
{
    return false;
}

GS_API_DECL void
gs_platform_window_position(uint32_t handle, uint32_t* x, uint32_t* y)
{
}

GS_API_DECL gs_vec2
gs_platform_window_positionv(uint32_t handle)
{
    return gs_v2(0, 0);
}

GS_API_DECL void     
gs_platform_set_window_size(uint32_t handle, uint32_t width, uint32_t height)
{
}

GS_API_DECL void     
gs_platform_set_window_sizev(uint32_t handle, gs_vec2 v)
{
}

GS_API_DECL void
gs_platform_set_window_fullscreen(uint32_t handle, bool32_t fullscreen)
{
}

GS_API_DECL void
gs_platform_set_window_position(uint32_t handle, uint32_t x, uint32_t y)
{
}

GS_API_DECL void
gs_platform_set_window_positionv(uint32_t handle, gs_vec2 v)
{
}

GS_API_DECL void     
gs_platform_set_cursor(uint32_t handle, gs_platform_cursor cursor)
{
}

GS_API_DECL void     
gs_platform_set_dropped_files_callback(uint32_t handle, gs_dropped_files_callback_t cb)
{
}

GS_API_DECL void     
gs_platform_set_window_close_callback(uint32_t handle, gs_window_close_callback_t cb)
{
}

GS_API_DECL void     
gs_platform_set_character_callback(uint32_t handle, gs_character_callback_t cb)
{
}

GS_API_DECL void*    
gs_platform_raw_window_handle(uint32_t handle)
{
}

GS_API_DECL gs_vec2  
gs_platform_framebuffer_sizev(uint32_t handle)
{
    gs_android_t* android = GS_PLATFORM_ANDROID_DATA();
    return gs_v2(android->egl.fbwidth, android->egl.fbheight);
}

GS_API_DECL void     
gs_platform_framebuffer_size(uint32_t handle, uint32_t* w, uint32_t* h)
{
    gs_android_t* android = GS_PLATFORM_ANDROID_DATA();
    *w = android->egl.fbwidth;
    *h = android->egl.fbheight;
}

GS_API_DECL uint32_t 
gs_platform_framebuffer_width(uint32_t handle)
{
    gs_android_t* android = GS_PLATFORM_ANDROID_DATA();
    return android->egl.fbwidth;
}

GS_API_DECL uint32_t 
gs_platform_framebuffer_height(uint32_t handle)
{
    gs_android_t* android = GS_PLATFORM_ANDROID_DATA();
    return android->egl.fbheight;
}

#undef GS_PLATFORM_IMPL_ANDROID
#endif // GS_PLATFORM_IMPL_ANDROID

#endif // __GS_PLATFORM_IMPL_H__
























