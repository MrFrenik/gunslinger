
/*================================================================
    * Copyright: 2020 John Jackson 
    * File: gs_platform_impl.h
    All Rights Reserved
=================================================================*/

#ifndef __GS_PLATFORM_IMPL_H__
#define __GS_PLATFORM_IMPL_H__

/*=================================
// Default Platform Implemenattion
=================================*/

// Define default platform implementation if certain platforms are enabled
#if (!defined GS_PLATFORM_IMPL_CUSTOM)
    #define GS_PLATFORM_IMPL_DEFAULT
#endif

/*=============================
// Default Impl
=============================*/

#ifdef GS_PLATFORM_IMPL_DEFAULT

#if !( defined GS_PLATFORM_WIN )
    #include <sys/stat.h>
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

uint32_t gs_platform_create_window(const char* title, uint32_t width, uint32_t height)
{
    gs_assert(gs_engine_instance() != NULL);
    gs_platform_t* platform = gs_engine_subsystem(platform);
    void* win = gs_platform_create_window_internal(title, width, height);
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

struct gs_uuid_t gs_platform_generate_uuid()
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

uint32_t gs_platform_hash_uuid(const gs_uuid_t* uuid)
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
    gs_for_range_i(GS_KEYCODE_COUNT)
    {
        input->prev_key_map[i] = input->key_map[i];
    }

    // Previous mouse button presses
    gs_for_range_i(GS_MOUSE_BUTTON_CODE_COUNT)
    {
        input->mouse.prev_button_map[i] = input->mouse.button_map[i];
    }

    // input->mouse.prev_position = input->mouse.position;
    input->mouse.wheel = gs_v2(0.0f, 0.0f);
    input->mouse.delta = gs_v2(0.f, 0.f);
    input->mouse.moved_this_frame = false;
    // gs_println("mouse pos: %.2f, %.2f", input->mouse.position.x, input->mouse.position.y);
    // if (!input->mouse.locked) {
    //     input->mouse.position = gs_v2(0.f, 0.f);
    // }
}

void gs_platform_poll_all_events()
{
    gs_platform_t* platform = gs_engine_subsystem(platform);

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
                            platform->input.mouse.delta = evt.mouse.move;
                            platform->input.mouse.position = gs_vec2_add(evt.mouse.move, platform->input.mouse.position);
                        } else {
                            platform->input.mouse.delta = gs_vec2_sub(evt.mouse.move, platform->input.mouse.delta);
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
    if (gs_platform_key_down(code) && !gs_platform_was_key_down(code))
    {
        return true;
    }
    return false;
}

bool gs_platform_key_released(gs_platform_keycode code)
{
    gs_platform_input_t* input = __gs_input();
    return (gs_platform_was_key_down(code) && !gs_platform_key_down(code));
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

void gs_platform_press_key(gs_platform_keycode code)
{
    gs_platform_input_t* input = __gs_input();
    if (code < GS_KEYCODE_COUNT) 
    {
        input->key_map[code] = true;
    }
}

void gs_platform_release_key(gs_platform_keycode code)
{
    gs_platform_input_t* input = __gs_input();
    if (code < GS_KEYCODE_COUNT) 
    {
        input->key_map[code] = false;
    }
}

// Platform File IO
char* gs_platform_read_file_contents(const char* file_path, const char* mode, int32_t* sz)
{
     char* buffer = 0;
    FILE* fp = fopen(file_path, mode);
    usize _sz = 0;
    if (fp)
    {
        _sz = gs_platform_file_size_in_bytes(file_path);
        // fseek(fp, 0, SEEK_END);
        // _sz = ftell(fp);
        // fseek(fp, 0, SEEK_SET);
        buffer = (char*)gs_malloc(_sz);
        if (buffer)
        {
            fread(buffer, 1, _sz, fp);
        }
        fclose(fp);
        // buffer[_sz] = '\0';
    }
    if (sz)
        *sz = _sz;
    return buffer;
}

gs_result gs_platform_write_file_contents(const char* file_path, const char* mode, void* data, size_t sz)
{
    FILE* fp = fopen(file_path, mode);
    if (fp) 
    {
        size_t ret = fwrite(data, sizeof(uint8_t), sz, fp);
        if (ret == sz)
        {
            return GS_RESULT_SUCCESS;
        }
    }
    return GS_RESULT_FAILURE;
}

bool gs_platform_file_exists(const char* file_path)
{
    return (gs_util_file_exists(file_path));
}

int32_t gs_platform_file_size_in_bytes(const char* file_path)
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

    #else

        struct stat st;
        stat(file_path, &st);
        return (int32_t)st.st_size; 

    #endif
}

void gs_platform_file_extension(char* buffer, size_t buffer_sz, const char* file_path)
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
    gs_dyn_array_push(platform->events, evt);
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
    gs_dyn_array_push(platform->events, evt);
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
    } else {
        gs_evt.mouse.move = gs_v2((f32)x, (f32)y);
    }

    // Push back event into platform events
    gs_dyn_array_push(platform->events, gs_evt);
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
    gs_dyn_array_push(platform->events, gs_evt);
}

// Gets called when mouse enters or leaves frame of window
void __glfw_mouse_cursor_enter_callback(GLFWwindow* window, s32 entered)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);
    gs_platform_event_t gs_evt = gs_default_val();
    gs_evt.type = GS_PLATFORM_EVENT_MOUSE;
    gs_evt.mouse.action = entered ? GS_PLATFORM_MOUSE_ENTER : GS_PLATFORM_MOUSE_LEAVE;
    gs_dyn_array_push(platform->events, gs_evt);
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

            Sleep((uint64_t)ms);

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

/*== Platform Window == */

void* gs_platform_create_window_internal(const char* title, uint32_t width, uint32_t height)
{
    // Grab window hints from application desc
    u32 window_hints = gs_engine_instance()->ctx.app.window_flags;

    // Set whether or not the screen is resizable
    glfwWindowHint(GLFW_RESIZABLE, (window_hints & GS_WINDOW_FLAGS_NO_RESIZE) != GS_WINDOW_FLAGS_NO_RESIZE);

    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL)
    {
        gs_println("Failed to create window.");
        glfwTerminate();
        return NULL;
   }

    // Callbacks for window
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, &__glfw_key_callback);
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
            } break;

            default: break;
        }
    }

    return window;
}

void gs_platform_set_dropped_files_callback(uint32_t handle, gs_dropped_files_callback_t cb)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    glfwSetDropCallback(win, (GLFWdropfun)cb);
}

void  gs_platform_set_window_close_callback(uint32_t handle, gs_window_close_callback_t cb)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    glfwSetWindowCloseCallback(win, (GLFWwindowclosefun)cb);
}

void gs_platform_set_character_callback(uint32_t handle, gs_character_callback_t cb)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    glfwSetCharCallback(win, (GLFWcharfun)cb);
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

void gs_platform_framebuffer_size(uint32_t handle, uint32_t* w, uint32_t* h)
{
    GLFWwindow* win = __glfw_window_from_handle(gs_engine_subsystem(platform), handle);
    float xscale = 0.f, yscale = 0.f;
    glfwGetWindowContentScale(win, &xscale, &yscale);
    glfwGetWindowSize(win, (int32_t*)w, (int32_t*)h);
    *w = (uint32_t)((float)*w * xscale);
    *h = (uint32_t)((float)*h * yscale);
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
    gs_platform_t* platform = gs_engine_subsystem(platform);
    gs_ems_t* ems = (gs_ems_t*)platform->user_data;
    (void)type;
    (void)evt;
    (void)user_data;
    emscripten_get_element_css_size(ems->canvas_name, &ems->canvas_width, &ems->canvas_height);
    emscripten_set_canvas_element_size(ems->canvas_name, ems->canvas_width, ems->canvas_height);
    return true;
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
    gs_dyn_array_push(gs_engine_subsystem(platform)->events, gs_evt);

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

    gs_dyn_array_push(gs_engine_subsystem(platform)->events, gs_evt);

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
    gs_dyn_array_push(gs_engine_subsystem(platform)->events, gs_evt);

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
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, false, gs_ems_size_changed_cb);
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
gs_platform_create_window_internal(const char* title, uint32_t width, uint32_t height)
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

#endif // __GS_PLATFORM_IMPL_H__
