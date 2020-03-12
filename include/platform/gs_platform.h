#ifndef GS_PLATFORM_H
#define GS_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gs_types.h"
#include "math/gs_math.h"
#include "common/gs_containers.h"
#include "common/gs_util.h"

#ifdef __APPLE__
	#define GS_PLATFORM_MAC
#endif

#ifdef WIN32
	#define GS_PLATFORM_WIN_32
#endif

#ifdef WIN64
	#define GS_PLATFORM_WIN_64
#endif

// Forward Decl. 
struct gs_uuid;
struct gs_platform_input;
struct gs_platform_window;

struct gs_platform_i* 		gs_platform_construct();

// /*============================================================
// // Platform UUID
// ============================================================*/

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

// // Forward Decl
struct gs_platform_window;

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

typedef enum gs_platform_mousebutton_code
{
	gs_mouse_lbutton,
	gs_mouse_rbutton,
	gs_mouse_mbutton,
	gs_mouse_button_code_count
} gs_mouse_button_code;

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
} gs_platform_video_settings;

typedef struct gs_platform_settings
{
	gs_platform_video_settings video;
} gs_platform_settings;

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
	u32 	( * ticks )();
	void 	( * delay )( u32 ticks );

	/*============================================================
	// Platform UUID
	============================================================*/
	struct gs_uuid 	( * generate_uuid )();
	void 			( * uuid_to_string )( char* temp_buffer, const struct gs_uuid* uuid ); // Expects a temp buffer with at leat 32 bytes
	u32 			( * hash_uuid )( const struct gs_uuid* uuid );

	/*============================================================
	// Platform Input
	============================================================*/
	struct gs_platform_input* ( * create_input )();
	gs_result ( * process_input )( struct gs_platform_input* input );

	b32 ( * was_key_down )( struct gs_platform_input* input, gs_platform_keycode code );
	b32 ( * key_pressed )( struct gs_platform_input* input, gs_platform_keycode code );
	b32 ( * key_down )( struct gs_platform_input* input, gs_platform_keycode code );
	b32 ( * key_released )( struct gs_platform_input* input, gs_platform_keycode code );

	b32 ( * was_mouse_down )( struct gs_platform_input* input, gs_mouse_button_code code );
	b32 ( * mouse_pressed )( struct gs_platform_input* input, gs_mouse_button_code code );
	b32 ( * mouse_down )( struct gs_platform_input* input, gs_mouse_button_code code );
	b32 ( * mouse_released )( struct gs_platform_input* input, gs_mouse_button_code code );

	gs_vec2 ( * mouse_delta )( struct gs_platform_input* input );
	gs_vec2 ( * mouse_position )( struct gs_platform_input* input );
	void ( * mouse_position_x_y )( struct gs_platform_input* input, f32* x, f32* y );
	void ( * mouse_wheel )( struct gs_platform_input* input, f32* x, f32* y );

	void ( * press_key )( struct gs_platform_input* input, gs_platform_keycode code );
	void ( * release_key )( struct gs_platform_input* input, gs_platform_keycode code );

	/*============================================================
	// Platform Window
	============================================================*/
	struct gs_platform_window* 	( * create_window )( const char* title, u32 width, u32 height );
	void 						( * window_swap_buffer )( struct gs_platform_window* win );
	gs_vec2 					( * window_size )( struct gs_platform_window* win );
	void 						( * window_size_w_h )( struct gs_platform_window* win, s32* width, s32* height );
	void 						( * set_cursor )( struct gs_platform_window* win, gs_platform_cursor cursor );

	/*============================================================
	// Platform File IO
	============================================================*/

	// Will return a null buffer if file does not exist or allocation fails
	char* 		( * read_file_contents_into_string_null_term )( const char* file_path, const char* mode, usize* sz );
	gs_result 	( * write_str_to_file )( const char* contents, const char* mode, usize sz, const char* output_path );

	// Settings for platform, including video, audio
	gs_platform_settings settings;

} gs_platform_i;

#ifdef __cplusplus
}
#endif 	// c++

#endif // GS_PLATFORM_H










