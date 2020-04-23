#ifndef __GS_PLATFORM_H__
#define __GS_PLATFORM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gs_types.h"
#include "math/gs_math.h"
#include "common/gs_containers.h"
#include "common/gs_util.h"

#if ( defined __APPLE__ || defined _APPLE )

	#define GS_PLATFORM_APPLE

#elif ( defined _WIN32 || defined _WIN64 )

	#define GS_PLATFORM_WIN
	#include <windows.h>

#elif ( defined linux || defined _linux || defined __linux__ )

	#define GS_PLATFORM_LINUX

#endif

// Forward Decl. 
struct gs_uuid;
struct gs_platform_input;
struct gs_platform_window;

/*============================================================
// Platform Time
============================================================*/

typedef struct gs_platform_time 
{
	f64 max_fps;
	f64 current;
	f64 previous;
	f64 update;
	f64 render;
	f64 delta;
	f64 frame;
} gs_platform_time;

/*============================================================
// Platform UUID
============================================================*/

#define gs_uuid_str_size_constant 		32

// 33 characters, all set to 0
#define	gs_uuid_temp_str_buffer()\
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }	

typedef struct 
gs_uuid
{
	const char* id;
	u8 bytes[ 16 ];
} gs_uuid;

// /*============================================================
// // Platform Window
// ============================================================*/

#define gs_window_flags_resizable 	0x01
#define gs_window_flags_fullscreen 	0x02
#define gs_window_flags_default 	(gs_window_flags_resizable | gs_window_flags_resizable)

// // Forward Decl
struct gs_platform_window;
typedef void* gs_platform_window_ptr;

// Internal handle for windows
typedef u32 gs_resource_handle;

// Declare slot array
gs_slot_array_decl( gs_platform_window_ptr );

typedef enum gs_platform_cursor
{
	gs_platform_cursor_arrow,
	gs_platform_cursor_ibeam,
	gs_platform_cursor_size_nw_se,
	gs_platform_cursor_size_ne_sw,
	gs_platform_cursor_size_ns,
	gs_platform_cursor_size_we,
	gs_platform_cursor_size_all,
	gs_platform_cursor_hand,
	gs_platform_cursor_no,
	gs_platform_cursor_count
} gs_platform_cursor;

// /*============================================================
// // Platform Input
// ============================================================*/

typedef enum gs_platform_keycode
{
	gs_keycode_a,
	gs_keycode_b,
	gs_keycode_c,
	gs_keycode_d,
	gs_keycode_e,
	gs_keycode_f,
	gs_keycode_g,
	gs_keycode_h,
	gs_keycode_i,
	gs_keycode_j,
	gs_keycode_k,
	gs_keycode_l,
	gs_keycode_m,
	gs_keycode_n,
	gs_keycode_o,
	gs_keycode_p,
	gs_keycode_q,
	gs_keycode_r,
	gs_keycode_s,
	gs_keycode_t,
	gs_keycode_u,
	gs_keycode_v,
	gs_keycode_w,
	gs_keycode_x,
	gs_keycode_y,
	gs_keycode_z,
	gs_keycode_lshift,
	gs_keycode_rshift,
	gs_keycode_lalt,
	gs_keycode_ralt,
	gs_keycode_lctrl,
	gs_keycode_rctrl,
	gs_keycode_bspace,
	gs_keycode_bslash,
	gs_keycode_qmark,
	gs_keycode_tilde,
	gs_keycode_comma,
	gs_keycode_period,
	gs_keycode_esc, 
	gs_keycode_space,
	gs_keycode_left,
	gs_keycode_up,
	gs_keycode_right,
	gs_keycode_down,
	gs_keycode_zero,
	gs_keycode_one,
	gs_keycode_two,
	gs_keycode_three,
	gs_keycode_four,
	gs_keycode_five,
	gs_keycode_six,
	gs_keycode_seven,
	gs_keycode_eight,
	gs_keycode_nine,
	gs_keycode_npzero,
	gs_keycode_npone,
	gs_keycode_nptwo,
	gs_keycode_npthree,
	gs_keycode_npfour,
	gs_keycode_npfive,
	gs_keycode_npsix,
	gs_keycode_npseven,
	gs_keycode_npeight,
	gs_keycode_npnine,
	gs_keycode_caps,
	gs_keycode_delete,
	gs_keycode_end,
	gs_keycode_f1,
	gs_keycode_f2,
	gs_keycode_f3,
	gs_keycode_f4,
	gs_keycode_f5,
	gs_keycode_f6,
	gs_keycode_f7,
	gs_keycode_f8,
	gs_keycode_f9,
	gs_keycode_f10,
	gs_keycode_f11,
	gs_keycode_f12,
	gs_keycode_home,
	gs_keycode_plus,
	gs_keycode_minus,
	gs_keycode_lbracket,
	gs_keycode_rbracket,
	gs_keycode_semi_colon,
	gs_keycode_enter,
	gs_keycode_insert,
	gs_keycode_pgup,
	gs_keycode_pgdown,
	gs_keycode_numlock,
	gs_keycode_tab,
	gs_keycode_npmult,
	gs_keycode_npdiv,
	gs_keycode_npplus,
	gs_keycode_npminus,
	gs_keycode_npenter,
	gs_keycode_npdel,
	gs_keycode_mute,
	gs_keycode_volup,
	gs_keycode_voldown,
	gs_keycode_pause,
	gs_keycode_print,
	gs_keycode_count
} gs_platform_keycode;

typedef enum gs_platform_mouse_button_code
{
	gs_mouse_lbutton,
	gs_mouse_rbutton,
	gs_mouse_mbutton,
	gs_mouse_button_code_count
} gs_platform_mouse_button_code;

typedef struct gs_platform_mouse
{
	b32 button_map[ gs_mouse_button_code_count ];
	b32 prev_button_map[ gs_mouse_button_code_count ];
	gs_vec2 position;
	gs_vec2 prev_position;
	gs_vec2 wheel;
} gs_platform_mouse;

typedef struct gs_platform_input
{
	b32 key_map[ gs_keycode_count ];
	b32 prev_key_map[ gs_keycode_count ];
	gs_platform_mouse mouse;
} gs_platform_input;

/*===============================================================================================
// Platform API Struct
===============================================================================================*/

// Enumeration of all platform type
typedef enum gs_platform_type
{
	gs_platform_type_unknown = 0,
	gs_platform_type_windows,
	gs_platform_type_linux,
	gs_platform_type_mac
} gs_platform_type;

typedef enum gs_platform_video_driver_type
{
	gs_platform_video_driver_type_none = 0,
	gs_platform_video_driver_type_opengl,
	gs_platform_video_driver_type_opengles,
	gs_platform_video_driver_type_directx,
	gs_platform_video_driver_type_vulkan,
	gs_platform_video_driver_type_metal,
	gs_platform_video_driver_type_software
} gs_platform_video_driver_type;

typedef enum gs_opengl_compatibility_flags
{
	gs_opengl_compatibility_flags_legacy 		= 0,
	gs_opengl_compatibility_flags_core 			= 1 << 1,
	gs_opengl_compatibility_flags_compatibility = 1 << 2,
	gs_opengl_compatibility_flags_forward 		= 1 << 3,
	gs_opengl_compatibility_flags_es 			= 1 << 4,
} gs_opengl_compatibility_flags;

// A structure that contains OpenGL video settings
typedef struct gs_opengl_video_settings 
{
	gs_opengl_compatibility_flags 	compability_flags;
	u32 							major_version;
	u32 							minor_version;
	u8 								multi_sampling_count;
	void* 							ctx;
} gs_opengl_video_settings;

typedef union gs_graphics_api_settings
{
	gs_opengl_video_settings 	opengl;
	s32 						dummy;	
} gs_graphics_api_settings;

typedef struct gs_platform_video_settings
{
	gs_graphics_api_settings 		graphics;
	gs_platform_video_driver_type 	driver;
	u32 							vsync_enabled;
} gs_platform_video_settings;

typedef struct gs_platform_settings
{
	gs_platform_video_settings video;
} gs_platform_settings;

typedef void ( * dropped_files_callback_t )( void*, s32 count, const char** file_paths );
typedef void ( * window_close_callback_t )( void* );

// General API for platform
typedef struct gs_platform_i
{
	/*============================================================
	// Platform Initilization / De-Initialization
	============================================================*/
	gs_result ( * init )( struct gs_platform_i* );
	gs_result ( * shutdown )( struct gs_platform_i* );

	/*============================================================
	// Platform Util
	============================================================*/
	void 	( * sleep )( f32 ms );	// Sleeps platform for time in ms
	f64 	( * elapsed_time )(); 	// Returns time in ms since initialization of platform

	/*============================================================
	// Platform Video
	============================================================*/
	void ( * enable_vsync )( b32 enabled );

	/*============================================================
	// Platform UUID
	============================================================*/
	struct gs_uuid 	( * generate_uuid )();
	void 			( * uuid_to_string )( char* temp_buffer, const struct gs_uuid* uuid ); // Expects a temp buffer with at leat 32 bytes
	u32 			( * hash_uuid )( const struct gs_uuid* uuid );

	/*============================================================
	// Platform Input
	============================================================*/
	gs_result ( * process_input )();

	void ( * update_input )();
	void ( * press_key )( gs_platform_keycode code );
	void ( * release_key )( gs_platform_keycode code );
	b32 ( * was_key_down )( gs_platform_keycode code );
	b32 ( * key_pressed )( gs_platform_keycode code );
	b32 ( * key_down )( gs_platform_keycode code );
	b32 ( * key_released )( gs_platform_keycode code );

	void ( * press_mouse_button )( gs_platform_mouse_button_code code );
	void ( * release_mouse_button )( gs_platform_mouse_button_code code );
	b32 ( * was_mouse_down )( gs_platform_mouse_button_code code );
	b32 ( * mouse_pressed )( gs_platform_mouse_button_code code );
	b32 ( * mouse_down )( gs_platform_mouse_button_code code );
	b32 ( * mouse_released )( gs_platform_mouse_button_code code );

	gs_vec2 ( * mouse_delta )();
	gs_vec2 ( * mouse_position )();
	void ( * mouse_position_x_y )( f32* x, f32* y );
	void ( * mouse_wheel )( f32* x, f32* y );

	/*============================================================
	// Platform Window
	============================================================*/
	gs_resource_handle 		( * create_window )( const char* title, u32 width, u32 height );
	void* 					( * create_window_internal )( const char* title, u32 width, u32 height );
	void 					( * window_swap_buffer )( gs_resource_handle handle );
	gs_vec2 				( * window_size )( gs_resource_handle handle );
	void 					( * set_window_size )( gs_resource_handle handle, s32 width, s32 height );
	void 					( * window_size_w_h )( gs_resource_handle handle, s32* width, s32* height );
	void 					( * set_cursor )( gs_resource_handle handle, gs_platform_cursor cursor );
	gs_resource_handle 		( *main_window )();
	void 					( * set_dropped_files_callback )( gs_resource_handle, dropped_files_callback_t );
	void 					( * set_window_close_callback )( gs_resource_handle, window_close_callback_t );

	/*============================================================
	// Platform File IO
	============================================================*/

	// Will return a null buffer if file does not exist or allocation fails
	char* 		( * read_file_contents )( const char* file_path, const char* mode, usize* sz );
	gs_result 	( * write_str_to_file )( const char* contents, const char* mode, usize sz, const char* output_path );
	b32 		( * file_exists )( const char* file_path );

	// Settings for platform, including video, audio
	gs_platform_settings settings;

	// Time
	gs_platform_time time;

	// Input
	gs_platform_input input;

	// For now, just keep a window here as main window...
	gs_slot_array( gs_platform_window_ptr ) windows;
	gs_dyn_array( gs_resource_handle ) active_window_handles;

	// Cursors
	void* cursors[ gs_platform_cursor_count ];

} gs_platform_i;

/*===============================
// Platform User Provided Funcs
===============================*/

extern struct gs_platform_i* gs_platform_construct();

/*============================
// Platform Default Funcs
============================*/

void __gs_default_init_platform();

void __gs_verify_platform_correctness( struct gs_platform_i* platform );

/*============================
// Platform Input
============================*/

void __gs_platform_update_input( );

b32 __gs_platform_was_key_down( gs_platform_keycode code );

b32 __gs_platform_key_down( gs_platform_keycode code );

b32 __gs_platform_key_pressed( gs_platform_keycode code );

b32 __gs_platform_key_released( gs_platform_keycode code );

b32 __gs_platform_was_mouse_down( gs_platform_mouse_button_code code );

void __gs_platform_press_mouse_button( gs_platform_mouse_button_code code );

void __gs_platform_release_mouse_button( gs_platform_mouse_button_code code );

b32 __gs_platform_mouse_down( gs_platform_mouse_button_code code );

b32 __gs_platform_mouse_pressed( gs_platform_mouse_button_code code );

b32 __gs_platform_mouse_released( gs_platform_mouse_button_code code );

gs_vec2 __gs_platform_mouse_delta();

gs_vec2 __gs_platform_mouse_position();

void __gs_platform_mouse_position_x_y( f32* x, f32* y );

void __gs_platform_mouse_wheel( f32* x, f32* y );

void __gs_platform_press_key( gs_platform_keycode code );

void __gs_platform_release_key( gs_platform_keycode code );

/*============================
// Platform Window
============================*/
gs_resource_handle __gs_platform_create_window( const char* title, u32 width, u32 height );
gs_resource_handle __gs_platform_main_window();

/*============================
// Platform File IO
============================*/
b32 __gs_platform_file_exists( const char* file_path );
char* __gs_platform_read_file_contents_into_string_null_term( const char* file_path, const char* mode, usize* sz );
gs_result __gs_platform_write_str_to_file( const char* contents, const char* mode, usize sz, const char* output_path );

/*============================
// Platform Util
============================*/

struct gs_uuid __gs_platform_generate_uuid();

void __gs_platform_uuid_to_string( char* temp_buffer, const struct gs_uuid* uuid ); // Expects a temp buffer with at leat 32 bytes

u32 __gs_platform_hash_uuid( const struct gs_uuid* uuid );


#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_PLATFORM_H__










