
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
#if (defined GS_PLATFORM_IMPL_GLFW)
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

gs_platform_i* gs_platform_create()
{
    // Construct new platform interface
    gs_platform_i* platform = gs_malloc_init(gs_platform_i);

    // Initialize windows
    platform->windows = gs_slot_array_new(void*);

    // Set up video mode (for now, just do opengl)
    platform->settings.video.driver = GS_PLATFORM_VIDEO_DRIVER_TYPE_OPENGL;

    return platform;
}

void gs_platform_destroy(gs_platform_i* platform)
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
    gs_platform_i* platform = gs_engine_subsystem(platform);
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
    const char* hex_string = sz_temp, *pos = hex_string;

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

    input->mouse.prev_position = input->mouse.position;
    input->mouse.wheel = gs_v2(0.0f, 0.0f);
    input->mouse.moved_this_frame = false;
}

gs_result gs_platform_update(gs_platform_i* platform)
{
    // Clear all previous events
    gs_dyn_array_clear(platform->events);

    // Update platform input from previous frame        
    gs_platform_update_input(&platform->input);

    // Process input for this frame
    return gs_platform_process_input(&platform->input);
}

bool gs_platform_poll_event(gs_platform_event_t* evt)
{
    gs_platform_i* platform = gs_engine_subsystem(platform);

    if (!evt) return false;
    if (gs_dyn_array_empty(platform->events)) return false;
    if (evt->idx >= gs_dyn_array_size(platform->events)) return false;

    uint32_t tmp_idx = evt->idx;
    *evt = platform->events[evt->idx];
    evt->idx = tmp_idx;
    evt->idx++;

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

    if (input->mouse.prev_position.x < 0.0f || 
        input->mouse.prev_position.y < 0.0f ||
        input->mouse.position.x < 0.0f || 
        input->mouse.position.y < 0.0f)
    {
        *x = 0.f;
        *y = 0.f;
        return;
    }
    
    *x = input->mouse.position.x - input->mouse.prev_position.x;
    *y = input->mouse.position.y - input->mouse.prev_position.y;
}

gs_vec2 gs_platform_mouse_deltav()
{
    gs_platform_input_t* input = __gs_input();

    if (input->mouse.prev_position.x < 0.0f || 
        input->mouse.prev_position.y < 0.0f ||
        input->mouse.position.x < 0.0f || 
        input->mouse.position.y < 0.0f)
    {
        return gs_v2(0.0f, 0.0f);
    }
    
    return gs_v2(input->mouse.position.x - input->mouse.prev_position.x, 
                      input->mouse.position.y - input->mouse.prev_position.y);
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

gs_result gs_platform_init(gs_platform_i* pf)
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
                glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
            #else
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

    return GS_RESULT_SUCCESS;
}

gs_result gs_platform_shutdown(gs_platform_i* pf)
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

    return GS_RESULT_SUCCESS;
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
        case GLFW_KEY_LEFT_SHIFT:       return GS_KEYCODE_LSHIFT; break;
        case GLFW_KEY_RIGHT_SHIFT:      return GS_KEYCODE_RSHIFT; break;
        case GLFW_KEY_LEFT_ALT:         return GS_KEYCODE_LALT; break;
        case GLFW_KEY_RIGHT_ALT:        return GS_KEYCODE_RALT; break;
        case GLFW_KEY_LEFT_CONTROL:     return GS_KEYCODE_LCTRL; break;
        case GLFW_KEY_RIGHT_CONTROL:    return GS_KEYCODE_RCTRL; break;
        case GLFW_KEY_BACKSPACE:        return GS_KEYCODE_BSPACE; break;
        case GLFW_KEY_BACKSLASH:        return GS_KEYCODE_BSLASH; break;
        case GLFW_KEY_SLASH:            return GS_KEYCODE_QMARK; break;
        case GLFW_KEY_GRAVE_ACCENT:     return GS_KEYCODE_TILDE; break;
        case GLFW_KEY_COMMA:            return GS_KEYCODE_COMMA; break;
        case GLFW_KEY_PERIOD:           return GS_KEYCODE_PERIOD; break;
        case GLFW_KEY_ESCAPE:           return GS_KEYCODE_ESC; break; 
        case GLFW_KEY_SPACE:            return GS_KEYCODE_SPACE; break;
        case GLFW_KEY_LEFT:             return GS_KEYCODE_LEFT; break;
        case GLFW_KEY_UP:               return GS_KEYCODE_UP; break;
        case GLFW_KEY_RIGHT:            return GS_KEYCODE_RIGHT; break;
        case GLFW_KEY_DOWN:             return GS_KEYCODE_DOWN; break;
        case GLFW_KEY_0:                return GS_KEYCODE_ZERO; break;
        case GLFW_KEY_1:                return GS_KEYCODE_ONE; break;
        case GLFW_KEY_2:                return GS_KEYCODE_TWO; break;
        case GLFW_KEY_3:                return GS_KEYCODE_THREE; break;
        case GLFW_KEY_4:                return GS_KEYCODE_FOUR; break;
        case GLFW_KEY_5:                return GS_KEYCODE_FIVE; break;
        case GLFW_KEY_6:                return GS_KEYCODE_SIX; break;
        case GLFW_KEY_7:                return GS_KEYCODE_SEVEN; break;
        case GLFW_KEY_8:                return GS_KEYCODE_EIGHT; break;
        case GLFW_KEY_9:                return GS_KEYCODE_NINE; break;
        case GLFW_KEY_KP_0:             return GS_KEYCODE_NPZERO; break;
        case GLFW_KEY_KP_1:             return GS_KEYCODE_NPONE; break;
        case GLFW_KEY_KP_2:             return GS_KEYCODE_NPTWO; break;
        case GLFW_KEY_KP_3:             return GS_KEYCODE_NPTHREE; break;
        case GLFW_KEY_KP_4:             return GS_KEYCODE_NPFOUR; break;
        case GLFW_KEY_KP_5:             return GS_KEYCODE_NPFIVE; break;
        case GLFW_KEY_KP_6:             return GS_KEYCODE_NPSIX; break;
        case GLFW_KEY_KP_7:             return GS_KEYCODE_NPSEVEN; break;
        case GLFW_KEY_KP_8:             return GS_KEYCODE_NPEIGHT; break;
        case GLFW_KEY_KP_9:             return GS_KEYCODE_NPNINE; break;
        case GLFW_KEY_CAPS_LOCK:        return GS_KEYCODE_CAPS; break;
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
        case GLFW_KEY_EQUAL:            return GS_KEYCODE_PLUS; break;
        case GLFW_KEY_MINUS:            return GS_KEYCODE_MINUS; break;
        case GLFW_KEY_LEFT_BRACKET:     return GS_KEYCODE_LBRACKET; break;
        case GLFW_KEY_RIGHT_BRACKET:    return GS_KEYCODE_RBRACKET; break;
        case GLFW_KEY_SEMICOLON:        return GS_KEYCODE_SEMI_COLON; break;
        case GLFW_KEY_ENTER:            return GS_KEYCODE_ENTER; break;
        case GLFW_KEY_INSERT:           return GS_KEYCODE_INSERT; break;
        case GLFW_KEY_PAGE_UP:          return GS_KEYCODE_PGUP; break;
        case GLFW_KEY_PAGE_DOWN:        return GS_KEYCODE_PGDOWN; break;
        case GLFW_KEY_NUM_LOCK:         return GS_KEYCODE_NUMLOCK; break;
        case GLFW_KEY_TAB:              return GS_KEYCODE_TAB; break;
        case GLFW_KEY_KP_MULTIPLY:      return GS_KEYCODE_NPMULT; break;
        case GLFW_KEY_KP_DIVIDE:        return GS_KEYCODE_NPDIV; break;
        case GLFW_KEY_KP_ADD:           return GS_KEYCODE_NPPLUS; break;
        case GLFW_KEY_KP_SUBTRACT:      return GS_KEYCODE_NPMINUS; break;
        case GLFW_KEY_KP_ENTER:         return GS_KEYCODE_NPENTER; break;
        case GLFW_KEY_KP_DECIMAL:       return GS_KEYCODE_NPDEL; break;
        case GLFW_KEY_PAUSE:            return GS_KEYCODE_PAUSE; break;
        case GLFW_KEY_PRINT_SCREEN:     return GS_KEYCODE_PRINT; break;
        default:                        return GS_KEYCODE_COUNT; break;
    }

    // Shouldn't reach here
    return GS_KEYCODE_COUNT;
}

uint32_t gs_platform_key_to_codepoint(gs_platform_keycode key)
{
    switch (key)
    {
        case GS_KEYCODE_A:              return GLFW_KEY_A; break;
        case GS_KEYCODE_B:              return GS_KEYCODE_B; break;
        case GS_KEYCODE_C:              return GS_KEYCODE_C; break;
        case GS_KEYCODE_D:              return GS_KEYCODE_D; break;
        case GS_KEYCODE_E:              return GS_KEYCODE_E; break;
        case GS_KEYCODE_F:              return GS_KEYCODE_F; break;
        case GS_KEYCODE_G:              return GS_KEYCODE_G; break;
        case GS_KEYCODE_H:              return GS_KEYCODE_H; break;
        case GS_KEYCODE_I:              return GS_KEYCODE_I; break;
        case GS_KEYCODE_J:              return GS_KEYCODE_J; break;
        case GS_KEYCODE_K:              return GS_KEYCODE_K; break;
        case GS_KEYCODE_L:              return GS_KEYCODE_L; break;
        case GS_KEYCODE_M:              return GS_KEYCODE_M; break;
        case GS_KEYCODE_N:              return GS_KEYCODE_N; break;
        case GS_KEYCODE_O:              return GS_KEYCODE_O; break;
        case GS_KEYCODE_P:              return GS_KEYCODE_P; break;
        case GS_KEYCODE_Q:              return GS_KEYCODE_Q; break;
        case GS_KEYCODE_R:              return GS_KEYCODE_R; break;
        case GS_KEYCODE_S:              return GS_KEYCODE_S; break;
        case GS_KEYCODE_T:              return GS_KEYCODE_T; break;
        case GS_KEYCODE_U:              return GS_KEYCODE_U; break;
        case GS_KEYCODE_V:              return GS_KEYCODE_V; break;
        case GS_KEYCODE_W:              return GS_KEYCODE_W; break;
        case GS_KEYCODE_X:              return GS_KEYCODE_X; break;
        case GS_KEYCODE_Y:              return GS_KEYCODE_Y; break;
        case GS_KEYCODE_Z:              return GS_KEYCODE_Z; break;
        case GS_KEYCODE_LSHIFT:         return GLFW_KEY_LEFT_SHIFT; break;
        case GS_KEYCODE_RSHIFT:         return GLFW_KEY_RIGHT_SHIFT; break;
        case GS_KEYCODE_LALT:           return GLFW_KEY_LEFT_ALT; break;
        case GS_KEYCODE_RALT:           return GLFW_KEY_RIGHT_ALT; break;
        case GS_KEYCODE_LCTRL:          return GLFW_KEY_LEFT_CONTROL; break;
        case GS_KEYCODE_RCTRL:          return GLFW_KEY_RIGHT_CONTROL; break;
        case GS_KEYCODE_BSPACE:         return GLFW_KEY_BACKSPACE; break;
        case GS_KEYCODE_QMARK:          return GLFW_KEY_SLASH; break;
        case GS_KEYCODE_TILDE:          return GLFW_KEY_GRAVE_ACCENT; break;
        case GS_KEYCODE_COMMA:          return GLFW_KEY_COMMA; break;
        case GS_KEYCODE_PERIOD:         return GLFW_KEY_PERIOD; break;
        case GS_KEYCODE_ESC:            return GLFW_KEY_ESCAPE; break;
        case GS_KEYCODE_SPACE:          return GLFW_KEY_SPACE; break;
        case GS_KEYCODE_LEFT:           return GLFW_KEY_LEFT; break;
        case GS_KEYCODE_UP:             return GLFW_KEY_UP; break;
        case GS_KEYCODE_RIGHT:          return GLFW_KEY_RIGHT; break;
        case GS_KEYCODE_DOWN:           return GLFW_KEY_DOWN; break;
        case GS_KEYCODE_ZERO:           return GLFW_KEY_0; break;
        case GS_KEYCODE_ONE:            return GLFW_KEY_1; break;
        case GS_KEYCODE_TWO:            return GLFW_KEY_2; break;
        case GS_KEYCODE_THREE:          return GLFW_KEY_3; break;
        case GS_KEYCODE_FOUR:           return GLFW_KEY_4; break;
        case GS_KEYCODE_FIVE:           return GLFW_KEY_5; break;
        case GS_KEYCODE_SIX:            return GLFW_KEY_6; break;
        case GS_KEYCODE_SEVEN:          return GLFW_KEY_7; break;
        case GS_KEYCODE_EIGHT:          return GLFW_KEY_8; break;
        case GS_KEYCODE_NINE:           return GLFW_KEY_9; break;
        case GS_KEYCODE_NPZERO:         return GLFW_KEY_KP_0; break;
        case GS_KEYCODE_NPONE:          return GLFW_KEY_KP_1; break;
        case GS_KEYCODE_NPTWO:          return GLFW_KEY_KP_2; break;
        case GS_KEYCODE_NPTHREE:        return GLFW_KEY_KP_3; break;
        case GS_KEYCODE_NPFOUR:         return GLFW_KEY_KP_4; break;
        case GS_KEYCODE_NPFIVE:         return GLFW_KEY_KP_5; break;
        case GS_KEYCODE_NPSIX:          return GLFW_KEY_KP_6; break;
        case GS_KEYCODE_NPSEVEN:        return GLFW_KEY_KP_7; break;
        case GS_KEYCODE_NPEIGHT:        return GLFW_KEY_KP_8; break;
        case GS_KEYCODE_NPNINE:         return GLFW_KEY_KP_9; break;
        case GS_KEYCODE_CAPS:           return GLFW_KEY_CAPS_LOCK; break;
        case GS_KEYCODE_DELETE:         return GLFW_KEY_DELETE; break;
        case GS_KEYCODE_END:            return GLFW_KEY_END; break;
        case GS_KEYCODE_F1:             return GLFW_KEY_F1; break;
        case GS_KEYCODE_F2:             return GLFW_KEY_F2; break;
        case GS_KEYCODE_F3:             return GLFW_KEY_F3; break;
        case GS_KEYCODE_F4:             return GLFW_KEY_F4; break;
        case GS_KEYCODE_F5:             return GLFW_KEY_F5; break;
        case GS_KEYCODE_F6:             return GLFW_KEY_F6; break;
        case GS_KEYCODE_F7:             return GLFW_KEY_F7; break;
        case GS_KEYCODE_F8:             return GLFW_KEY_F8; break;
        case GS_KEYCODE_F9:             return GLFW_KEY_F9; break;
        case GS_KEYCODE_F10:            return GLFW_KEY_F10; break;
        case GS_KEYCODE_F11:            return GLFW_KEY_F11; break;
        case GS_KEYCODE_F12:            return GLFW_KEY_F12; break;
        case GS_KEYCODE_HOME:           return GLFW_KEY_HOME; break;
        case GS_KEYCODE_PLUS:           return GLFW_KEY_EQUAL; break;
        case GS_KEYCODE_MINUS:          return GLFW_KEY_MINUS; break;
        case GS_KEYCODE_LBRACKET:       return GLFW_KEY_LEFT_BRACKET; break;
        case GS_KEYCODE_RBRACKET:       return GLFW_KEY_RIGHT_BRACKET; break;
        case GS_KEYCODE_SEMI_COLON:     return GLFW_KEY_SEMICOLON; break;
        case GS_KEYCODE_ENTER:          return GLFW_KEY_ENTER; break;
        case GS_KEYCODE_INSERT:         return GLFW_KEY_INSERT; break;
        case GS_KEYCODE_PGUP:           return GLFW_KEY_PAGE_UP; break;
        case GS_KEYCODE_PGDOWN:         return GLFW_KEY_PAGE_DOWN; break;
        case GS_KEYCODE_NUMLOCK:        return GLFW_KEY_NUM_LOCK; break;
        case GS_KEYCODE_TAB:            return GLFW_KEY_TAB; break;
        case GS_KEYCODE_NPMULT:         return GLFW_KEY_KP_MULTIPLY; break;
        case GS_KEYCODE_NPDIV:          return GLFW_KEY_KP_DIVIDE; break;
        case GS_KEYCODE_NPPLUS:         return GLFW_KEY_KP_ADD; break;
        case GS_KEYCODE_NPMINUS:        return GLFW_KEY_KP_SUBTRACT; break;
        case GS_KEYCODE_NPENTER:        return GLFW_KEY_KP_ENTER; break;
        case GS_KEYCODE_NPDEL:          return GLFW_KEY_KP_DECIMAL; break;
        case GS_KEYCODE_PAUSE:          return GLFW_KEY_PAUSE; break;
        case GS_KEYCODE_PRINT:          return GLFW_KEY_PRINT_SCREEN; break;
        case GS_KEYCODE_COUNT:          return GLFW_KEY_UNKNOWN; break;
    }

    // Shouldn't reach here
    return GLFW_KEY_UNKNOWN;
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
    gs_platform_i* platform = gs_engine_subsystem(platform);

    // Get keycode from key
    gs_platform_keycode key = glfw_key_to_gs_keycode(code);

    // Push back event into platform events
    gs_platform_event_t evt = gs_default_val();
    evt.type = GS_PLATFORM_EVENT_KEY;
    evt.key.codepoint = code;
    evt.key.key = key;

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

        default: {
        } break;
    }

    // Add action
    gs_dyn_array_push(platform->events, evt);
}

void __glfw_mouse_button_callback(GLFWwindow* window, s32 code, s32 action, s32 mods)
{
    // Grab platform instance from engine
    gs_platform_i* platform = gs_engine_subsystem(platform);

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
            evt.mouse.action = GS_PLATFORM_MBUTTON_RELEASED;
        } break;

        // Pressed
        case 1:
        {
            gs_platform_press_mouse_button(button);
            evt.mouse.action = GS_PLATFORM_MBUTTON_PRESSED;
        } break;
    }

    // Add action
    gs_dyn_array_push(platform->events, evt);
}

void __glfw_mouse_cursor_position_callback(GLFWwindow* window, f64 x, f64 y)
{
    gs_platform_i* platform = gs_engine_subsystem(platform);
    platform->input.mouse.position = gs_v2((f32)x, (f32)y);
    platform->input.mouse.moved_this_frame = true;
}

void __glfw_mouse_scroll_wheel_callback(GLFWwindow* window, f64 x, f64 y)
{
    gs_platform_i* platform = gs_engine_subsystem(platform);
    platform->input.mouse.wheel = gs_v2((f32)x, (f32)y);
}

// Gets called when mouse enters or leaves frame of window
void __glfw_mouse_cursor_enter_callback(GLFWwindow* window, s32 entered)
{
    // Nothing for now, will capture state for windows later
}

void __glfw_frame_buffer_size_callback(GLFWwindow* window, s32 width, s32 height)
{
    // Nothing for now
}

/*== Platform Input == */

gs_result gs_platform_process_input(gs_platform_input_t* input)
{
    glfwPollEvents();
    return GS_RESULT_IN_PROGRESS;
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
    gs_platform_i* platform = gs_engine_subsystem(platform);
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    glfwSetDropCallback(win, (GLFWdropfun)cb);
}

void  gs_platform_set_window_close_callback(uint32_t handle, gs_window_close_callback_t cb)
{
    gs_platform_i* platform = gs_engine_subsystem(platform);
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    glfwSetWindowCloseCallback(win, (GLFWwindowclosefun)cb);
}

void gs_platform_set_mouse_position(uint32_t handle, float x, float y)
{
    struct gs_platform_i* platform = gs_engine_subsystem(platform);
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    glfwSetCursorPos(win, x, y);
}

void* gs_platform_raw_window_handle(uint32_t handle)
{
    // Grab instance of platform from engine
    gs_platform_i* platform = gs_engine_subsystem(platform);

    // Grab window from handle
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    return (void*)win;
}

void gs_platform_window_swap_buffer(uint32_t handle)
{
    // Grab instance of platform from engine
    gs_platform_i* platform = gs_engine_subsystem(platform);

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
    gs_platform_i* platform = gs_engine_subsystem(platform);
    GLFWwindow* win = __glfw_window_from_handle(platform, handle);
    GLFWcursor* cp = ((GLFWcursor*)platform->cursors[(u32)cursor]); 
    glfwSetCursor(win, cp);
}

#undef GS_PLATFORM_IMPL_GLFW
#endif // GS_PLATFORM_IMPL_GLFW

#endif // __GS_PLATFORM_IMPL_H__
