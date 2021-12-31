/*==============================================================================================================

		* Gunslinger
		* File: gs_gui.h
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

/*
	USAGE: (IMPORTANT)

	=================================================================================================================

	Before including, define the gunslinger asset manager implementation like this:

	    #define GS_GUI_IMPL

    in EXACTLY ONE C or C++ file that includes this header, BEFORE the
    include, like this:

		#define GS_GUI_IMPL
		#include "gs_gui.h"

    All other files should just #include "gs_asset.h" without the #define.

    MUST include "gs.h" and "gs_idraw.h" and declare GS_IMPL and GS_IMMEDIATE_DRAW_IMPL BEFORE this file, since this file relies on those:

    	#define GS_IMPL
    	#include "gs.h"

    	#define GS_IMMEDIATE_DRAW_IMPL
    	#include "gs_idraw.h"

    	#define GS_GUI_IMPL
    	#include "gs_gui.h"

	================================================================================================================
*/ 

#ifndef GS_GUI_H
#define GS_GUI_H 

// Directly modified from microui implementation: https://github.com/rxi/microui 

#define GS_GUI_SPLIT_SIZE 2.f

#define GS_GUI_MAX_CNT              100 
#define GS_GUI_COMMANDLIST_SIZE		(256 * 1024 * 4)
#define GS_GUI_ROOTLIST_SIZE		GS_GUI_MAX_CNT
#define GS_GUI_CONTAINERSTACK_SIZE	GS_GUI_MAX_CNT
#define GS_GUI_CLIPSTACK_SIZE		GS_GUI_MAX_CNT
#define GS_GUI_IDSTACK_SIZE			GS_GUI_MAX_CNT
#define GS_GUI_LAYOUTSTACK_SIZE		GS_GUI_MAX_CNT
#define GS_GUI_CONTAINERPOOL_SIZE	GS_GUI_MAX_CNT
#define GS_GUI_TREENODEPOOL_SIZE	GS_GUI_MAX_CNT
#define GS_GUI_GS_GUI_SPLIT_SIZE	GS_GUI_MAX_CNT
#define GS_GUI_GS_GUI_TAB_SIZE		GS_GUI_MAX_CNT
#define GS_GUI_MAX_WIDTHS			16
#define GS_GUI_REAL					float
#define GS_GUI_REAL_FMT				"%.3g"
#define GS_GUI_SLIDER_FMT			"%.2f"
#define GS_GUI_MAX_FMT				127
#define GS_GUI_TAB_ITEM_MAX         24

#define gs_gui_stack(T, n)			struct {int32_t idx; T items[n];}

enum {
	GS_GUI_CLIP_PART = 1,
	GS_GUI_CLIP_ALL
};

enum {
	GS_GUI_COMMAND_JUMP = 1,
	GS_GUI_COMMAND_CLIP,
    GS_GUI_COMMAND_SHAPE,
	GS_GUI_COMMAND_TEXT,
	GS_GUI_COMMAND_ICON,
	GS_GUI_COMMAND_IMAGE,
	GS_GUI_COMMAND_MAX
};

enum {
    GS_GUI_SHAPE_RECT = 1,
    GS_GUI_SHAPE_CIRCLE, 
    GS_GUI_SHAPE_TRIANGLE
};

enum {
	GS_GUI_COLOR_TEXT,
	GS_GUI_COLOR_TEXT_INACTIVE,
	GS_GUI_COLOR_BORDER,
	GS_GUI_COLOR_SHADOW,
	GS_GUI_COLOR_WINDOWBG,
	GS_GUI_COLOR_TITLEBG,
	GS_GUI_COLOR_TITLETEXT,
	GS_GUI_COLOR_PANELBG,
	GS_GUI_COLOR_BUTTON,
	GS_GUI_COLOR_BUTTONHOVER,
	GS_GUI_COLOR_BUTTONFOCUS,
	GS_GUI_COLOR_BASE,
	GS_GUI_COLOR_BASEHOVER,
	GS_GUI_COLOR_BASEFOCUS,
	GS_GUI_COLOR_SCROLLBASE,
	GS_GUI_COLOR_SCROLLTHUMB,
    GS_GUI_COLOR_INVISIBLE,
	GS_GUI_COLOR_MAX
};

enum {
	GS_GUI_ICON_CLOSE = 1,
	GS_GUI_ICON_CHECK,
	GS_GUI_ICON_COLLAPSED,
	GS_GUI_ICON_EXPANDED,
	GS_GUI_ICON_MAX
};

enum {
	GS_GUI_RES_ACTIVE			 = (1 << 0),
	GS_GUI_RES_SUBMIT			 = (1 << 1),
	GS_GUI_RES_CHANGE			 = (1 << 2)
};

typedef enum gs_gui_alt_drag_mode_type {
    GS_GUI_ALT_DRAG_QUAD = 0x00,        // Quadrants
    GS_GUI_ALT_DRAG_NINE,               // Nine splice the window
    GS_GUI_ALT_DRAG_SINGLE              // Single window drag (controls the width/height, leaving x/y position of window in tact)
} gs_gui_alt_drag_mode_type;

enum {
	GS_GUI_OPT_ALIGNCENTER	= (1 << 0),
	GS_GUI_OPT_ALIGNRIGHT	= (1 << 1),
	GS_GUI_OPT_NOINTERACT	= (1 << 2),
	GS_GUI_OPT_NOFRAME		= (1 << 3),
	GS_GUI_OPT_NORESIZE		= (1 << 4),
	GS_GUI_OPT_NOSCROLL		= (1 << 5),
	GS_GUI_OPT_NOCLOSE		= (1 << 6),
	GS_GUI_OPT_NOTITLE		= (1 << 7),
	GS_GUI_OPT_HOLDFOCUS	= (1 << 8),
	GS_GUI_OPT_AUTOSIZE		= (1 << 9),
	GS_GUI_OPT_POPUP		= (1 << 10),
	GS_GUI_OPT_CLOSED		= (1 << 11),
	GS_GUI_OPT_EXPANDED		= (1 << 12),
	GS_GUI_OPT_NOHOVER		= (1 << 13),
	GS_GUI_OPT_FORCESETRECT	= (1 << 14),
	GS_GUI_OPT_NOFOCUS	    = (1 << 15),
	GS_GUI_OPT_FORCEFOCUS	= (1 << 16),
    GS_GUI_OPT_NOMOVE       = (1 << 17),
    GS_GUI_OPT_NOCLIP       = (1 << 18),
    GS_GUI_OPT_NODOCK       = (1 << 19),
    GS_GUI_OPT_FULLSCREEN   = (1 << 20),
    GS_GUI_OPT_DOCKSPACE    = (1 << 21),
    GS_GUI_OPT_NOBRINGTOFRONT = (1 << 22) 
};

enum {
	GS_GUI_MOUSE_LEFT		= (1 << 0),
	GS_GUI_MOUSE_RIGHT		= (1 << 1),
	GS_GUI_MOUSE_MIDDLE		= (1 << 2)
};

enum {
	GS_GUI_KEY_SHIFT		= (1 << 0),
	GS_GUI_KEY_CTRL			= (1 << 1),
	GS_GUI_KEY_ALT			= (1 << 2),
	GS_GUI_KEY_BACKSPACE	= (1 << 3),
	GS_GUI_KEY_RETURN		= (1 << 4)
}; 

typedef struct gs_gui_context_t gs_gui_context_t;
typedef uint32_t gs_gui_id;
typedef GS_GUI_REAL gs_gui_real;

// Shapes
typedef struct {float x, y, w, h;} gs_gui_rect_t;
typedef struct {float radius; gs_vec2 center;} gs_gui_circle_t;
typedef struct {gs_vec2 points[3];} gs_gui_triangle_t;

typedef struct {gs_gui_id id; int32_t last_update;} gs_gui_pool_item_t; 

typedef struct {int32_t type, size;} gs_gui_basecommand_t;
typedef struct {gs_gui_basecommand_t base; void *dst;} gs_gui_jumpcommand_t;
typedef struct {gs_gui_basecommand_t base; gs_gui_rect_t rect;} gs_gui_clipcommand_t;
typedef struct {gs_gui_basecommand_t base; gs_asset_font_t* font; gs_vec2 pos; gs_color_t color; char str[1];} gs_gui_textcommand_t;
typedef struct {gs_gui_basecommand_t base; gs_gui_rect_t rect; int32_t id; gs_color_t color;} gs_gui_iconcommand_t;
typedef struct {gs_gui_basecommand_t base; gs_gui_rect_t rect; gs_handle(gs_graphics_texture_t) hndl; gs_vec4 uvs; gs_color_t color;} gs_gui_imagecommand_t;
typedef struct {
    gs_gui_basecommand_t base; 
    uint32_t type;
    union
    {
        gs_gui_rect_t rect;
        gs_gui_circle_t circle;
        gs_gui_triangle_t triangle;
    };
    gs_color_t color;
} gs_gui_shapecommand_t;

typedef union 
{
	int32_t type;
	gs_gui_basecommand_t base;
	gs_gui_jumpcommand_t jump;
	gs_gui_clipcommand_t clip;
	gs_gui_shapecommand_t shape;
	gs_gui_textcommand_t text;
	gs_gui_iconcommand_t icon;
	gs_gui_imagecommand_t image;
} gs_gui_command_t; 

struct gs_gui_context_t;

typedef void (* gs_gui_on_draw_button_callback)(struct gs_gui_context_t* ctx, gs_gui_rect_t rect, gs_gui_id id, bool hovered, bool focused, int32_t opt, const char* label, int32_t icon);

typedef struct gs_gui_layout_t
{
	gs_gui_rect_t body;
	gs_gui_rect_t next;
	gs_vec2 position;
	gs_vec2 size;
	gs_vec2 max;
	int32_t widths[GS_GUI_MAX_WIDTHS];
	int32_t items;
	int32_t item_index;
	int32_t next_row;
	int32_t next_type;
	int32_t indent;
} gs_gui_layout_t;

// Forward decl.
struct gs_gui_container_t;

typedef enum gs_gui_split_node_type
{
    GS_GUI_SPLIT_NODE_CONTAINER = 0x00,
    GS_GUI_SPLIT_NODE_SPLIT 
} gs_gui_split_node_type;

enum {
    GS_GUI_SPLIT_NODE_CHILD = 0x00,
    GS_GUI_SPLIT_NODE_PARENT
};

typedef struct gs_gui_split_node_t
{
    gs_gui_split_node_type type;
    union
    {
        uint32_t split;
        struct gs_gui_container_t* container;
    };
} gs_gui_split_node_t;

typedef enum gs_gui_split_type
{
    GS_GUI_SPLIT_LEFT = 0x00,
    GS_GUI_SPLIT_RIGHT,
    GS_GUI_SPLIT_TOP,
    GS_GUI_SPLIT_BOTTOM,
    GS_GUI_SPLIT_TAB
} gs_gui_split_type; 

typedef struct gs_gui_split_t
{
    gs_gui_split_type type;             // GS_GUI_SPLIT_LEFT, GS_GUI_SPLIT_RIGHT, GS_GUI_SPLIT_TAB, GS_GUI_SPLIT_BOTTOM, GS_GUI_SPLIT_TOP
    float ratio;                        // Split ratio between children [0.f, 1.f], (left node = ratio), right node = (1.f - ratio)
    gs_gui_rect_t rect;
    gs_gui_rect_t prev_rect;
    gs_gui_split_node_t children[2];
    uint32_t parent;
    uint32_t id;
    uint32_t zindex;
} gs_gui_split_t;

typedef enum gs_gui_window_flags { 
	GS_GUI_WINDOW_FLAGS_VISIBLE	= (1 << 0),
} gs_gui_window_flags; 

// Equidistantly sized tabs, based on rect of window
typedef struct gs_gui_tab_item_t
{
    gs_gui_id tab_bar;
    uint32_t zindex;        // Sorting index in tab bar
    void* data;             // User set data pointer for this item
	uint32_t idx;			// Internal index
} gs_gui_tab_item_t; 

typedef struct gs_gui_tab_bar_t
{
    gs_gui_tab_item_t items[GS_GUI_TAB_ITEM_MAX];
    uint32_t size;                                  // Current number of items in tab bar
    gs_gui_rect_t rect;                             // Cached sized for tab bar
    uint32_t focus;									// Focused item in tab bar
} gs_gui_tab_bar_t; 

typedef struct gs_gui_container_t 
{
	gs_gui_command_t *head, *tail;
	gs_gui_rect_t rect;
	gs_gui_rect_t body;
	gs_vec2 content_size;
	gs_vec2 scroll;
	int32_t zindex;
	int32_t open;
	gs_gui_id id;
    gs_gui_id split;                    // If container is docked, then will have owning split to get sizing (0x00 for NULL)
    uint32_t tab_bar;
    uint32_t tab_item;
    struct gs_gui_container_t* parent;  // Owning parent (for tabbing)
	int32_t opt;
    uint32_t frame;
    uint32_t visible;
    int32_t flags;
	char name[256];
} gs_gui_container_t;

typedef struct gs_gui_style_t
{
	gs_asset_font_t* font;
	gs_vec2 size;
	int32_t padding;
	int32_t spacing;
	int32_t indent;
	int32_t title_height;
	int32_t scrollbar_size;
	int32_t thumb_size;
	gs_color_t colors[GS_GUI_COLOR_MAX];
} gs_gui_style_t;

typedef enum gs_gui_request_type
{
    GS_GUI_SPLIT_NEW = 0x01,
    GS_GUI_CNT_MOVE,
    GS_GUI_CNT_FOCUS,
    GS_GUI_SPLIT_MOVE, 
    GS_GUI_SPLIT_RESIZE_SW,
    GS_GUI_SPLIT_RESIZE_SE,
    GS_GUI_SPLIT_RESIZE_NW,
    GS_GUI_SPLIT_RESIZE_NE,
    GS_GUI_SPLIT_RESIZE_W,
    GS_GUI_SPLIT_RESIZE_E,
    GS_GUI_SPLIT_RESIZE_N,
    GS_GUI_SPLIT_RESIZE_S,
    GS_GUI_SPLIT_RESIZE_CENTER,
    GS_GUI_SPLIT_RATIO,
    GS_GUI_SPLIT_RESIZE_INVALID,
	GS_GUI_TAB_SWAP_LEFT,
	GS_GUI_TAB_SWAP_RIGHT 
} gs_gui_request_type;

typedef struct gs_gui_request_t
{
    gs_gui_request_type type;
    union
    {
        gs_gui_split_type split_type;
        gs_gui_split_t* split;
        gs_gui_container_t* cnt;
    };
    uint32_t frame;
} gs_gui_request_t; 

typedef struct gs_gui_context_t 
{ 
	// Callbacks
    int32_t (*text_width)(gs_asset_font_t* font, const char* text, int32_t len);
    int32_t (*text_height)(gs_asset_font_t* font, const char* text, int32_t len);
    int32_t (*font_height)(gs_asset_font_t* font);
    gs_vec2 (*text_dimensions)(gs_asset_font_t* font, const char* text, int32_t len);
	void (*draw_frame)(gs_gui_context_t *ctx, gs_gui_rect_t rect, int32_t colorid);

	// Core state
	gs_gui_style_t _style;
	gs_gui_style_t* style;
	gs_gui_id hover;
	gs_gui_id focus;
	gs_gui_id last_id;
    gs_gui_id lock_focus;
	gs_gui_rect_t last_rect;
	int32_t last_zindex;
	int32_t updated_focus;
	int32_t frame;
	gs_gui_container_t* hover_root;
	gs_gui_container_t* next_hover_root;
	gs_gui_container_t* scroll_target;
    gs_gui_container_t* focus_root;
    gs_gui_container_t* next_focus_root;
    gs_gui_container_t* dockable_root;
    gs_gui_container_t* prev_dockable_root;
    gs_gui_container_t* docked_root;
    gs_gui_container_t* undock_root;
    gs_gui_split_t*     focus_split;
    gs_gui_split_t*     next_hover_split;
    gs_gui_split_t*     hover_split;
    gs_gui_id           next_lock_hover_id;
    gs_gui_id           lock_hover_id;
	char number_edit_buf[GS_GUI_MAX_FMT];
	gs_gui_id number_edit;
    gs_vec2 framebuffer;
    gs_gui_alt_drag_mode_type alt_drag_mode;
    gs_dyn_array(gs_gui_request_t) requests;

	// Stacks
	gs_gui_stack(uint8_t, GS_GUI_COMMANDLIST_SIZE) command_list;
	gs_gui_stack(gs_gui_container_t*, GS_GUI_ROOTLIST_SIZE) root_list;
	gs_gui_stack(gs_gui_container_t*, GS_GUI_CONTAINERSTACK_SIZE) container_stack;
	gs_gui_stack(gs_gui_rect_t, GS_GUI_CLIPSTACK_SIZE) clip_stack;
	gs_gui_stack(gs_gui_id, GS_GUI_IDSTACK_SIZE) id_stack;
	gs_gui_stack(gs_gui_layout_t, GS_GUI_LAYOUTSTACK_SIZE) layout_stack;

	// Retained state pools
	gs_gui_pool_item_t container_pool[GS_GUI_CONTAINERPOOL_SIZE];
	gs_gui_container_t containers[GS_GUI_CONTAINERPOOL_SIZE];
	gs_gui_pool_item_t treenode_pool[GS_GUI_TREENODEPOOL_SIZE];

    gs_slot_array(gs_gui_split_t) splits;
    gs_slot_array(gs_gui_tab_bar_t) tab_bars;

	// Input state
	gs_vec2 mouse_pos;
	gs_vec2 last_mouse_pos;
	gs_vec2 mouse_delta;
	gs_vec2 scroll_delta;
	int32_t mouse_down;
	int32_t mouse_pressed;
	int32_t key_down;
	int32_t key_pressed;
	char input_text[32]; 

    // Backend resources
    uint32_t window_hndl;
    gs_immediate_draw_t gsi;
    gs_immediate_draw_t overlay_draw_list;                                  
    gs_handle(gs_graphics_texture_t) atlas_tex;

    // Callbacks
    struct {
        gs_gui_on_draw_button_callback button;
    } callbacks;

} gs_gui_context_t; 

gs_gui_rect_t gs_gui_rect(float x, float y, float w, float h);

GS_API_DECL void gs_gui_init(gs_gui_context_t *ctx, uint32_t window_hndl);
GS_API_DECL void gs_gui_begin(gs_gui_context_t *ctx);
GS_API_DECL void gs_gui_end(gs_gui_context_t *ctx); 
GS_API_DECL void gs_gui_render(gs_gui_context_t* ctx, gs_command_buffer_t* cb);
GS_API_DECL void gs_gui_set_focus(gs_gui_context_t *ctx, gs_gui_id id);
GS_API_DECL gs_gui_id gs_gui_get_id(gs_gui_context_t *ctx, const void *data, int32_t size);
GS_API_DECL void gs_gui_push_id(gs_gui_context_t *ctx, const void *data, int32_t size);
GS_API_DECL void gs_gui_pop_id(gs_gui_context_t *ctx);
GS_API_DECL void gs_gui_push_clip_rect(gs_gui_context_t *ctx, gs_gui_rect_t rect);
GS_API_DECL void gs_gui_pop_clip_rect(gs_gui_context_t *ctx);
GS_API_DECL gs_gui_rect_t gs_gui_get_clip_rect(gs_gui_context_t *ctx);
GS_API_DECL int32_t gs_gui_check_clip(gs_gui_context_t *ctx, gs_gui_rect_t r);
GS_API_DECL gs_gui_container_t* gs_gui_get_current_container(gs_gui_context_t *ctx);
GS_API_DECL gs_gui_container_t* gs_gui_get_container(gs_gui_context_t *ctx, const char *name); 
GS_API_DECL gs_gui_container_t* gs_gui_get_top_most_container(gs_gui_context_t* ctx, gs_gui_split_t* split);
GS_API_DECL gs_gui_container_t* gs_gui_get_container_ex(gs_gui_context_t *ctx, gs_gui_id id, int32_t opt);
GS_API_DECL void gs_gui_bring_to_front(gs_gui_context_t *ctx, gs_gui_container_t *cnt); 
GS_API_DECL void gs_gui_bring_split_to_front(gs_gui_context_t* ctx, gs_gui_split_t* split); 
GS_API_DECL gs_gui_split_t* gs_gui_get_split(gs_gui_context_t* ctx, gs_gui_container_t* cnt);
GS_API_DECL gs_gui_tab_bar_t* gs_gui_get_tab_bar(gs_gui_context_t* ctx, gs_gui_container_t* cnt);
GS_API_DECL void gs_gui_tab_item_swap(gs_gui_context_t* ctx, gs_gui_container_t* cnt, int32_t direction);
GS_API_DECL gs_gui_container_t* gs_gui_get_root_container(gs_gui_context_t* ctx, gs_gui_container_t* cnt); 
GS_API_DECL gs_gui_container_t* gs_gui_get_root_container_from_split(gs_gui_context_t* ctx, gs_gui_split_t* split);
GS_API_DECL gs_gui_container_t* gs_gui_get_parent(gs_gui_context_t* ctx, gs_gui_container_t* cnt);

GS_API_DECL int32_t gs_gui_pool_init(gs_gui_context_t *ctx, gs_gui_pool_item_t *items, int32_t len, gs_gui_id id);
GS_API_DECL int32_t gs_gui_pool_get(gs_gui_context_t *ctx, gs_gui_pool_item_t *items, int32_t len, gs_gui_id id);
GS_API_DECL void gs_gui_pool_update(gs_gui_context_t *ctx, gs_gui_pool_item_t *items, int32_t idx);

GS_API_DECL void gs_gui_input_mousemove(gs_gui_context_t *ctx, int32_t x, int32_t y);
GS_API_DECL void gs_gui_input_mousedown(gs_gui_context_t *ctx, int32_t x, int32_t y, int32_t btn);
GS_API_DECL void gs_gui_input_mouseup(gs_gui_context_t *ctx, int32_t x, int32_t y, int32_t btn);
GS_API_DECL void gs_gui_input_scroll(gs_gui_context_t *ctx, int32_t x, int32_t y);
GS_API_DECL void gs_gui_input_keydown(gs_gui_context_t *ctx, int32_t key);
GS_API_DECL void gs_gui_input_keyup(gs_gui_context_t *ctx, int32_t key);
GS_API_DECL void gs_gui_input_text(gs_gui_context_t *ctx, const char *text);

GS_API_DECL gs_gui_command_t* gs_gui_push_command(gs_gui_context_t* ctx, int32_t type, int32_t size);
GS_API_DECL int32_t gs_gui_next_command(gs_gui_context_t* ctx, gs_gui_command_t** cmd);
GS_API_DECL void gs_gui_set_clip(gs_gui_context_t* ctx, gs_gui_rect_t rect);
GS_API_DECL void gs_gui_draw_rect(gs_gui_context_t* ctx, gs_gui_rect_t rect, gs_color_t color);
GS_API_DECL void gs_gui_draw_circle(gs_gui_context_t* ctx, gs_vec2 position, float radius, gs_color_t color);
GS_API_DECL void gs_gui_draw_triangle(gs_gui_context_t* ctx, gs_vec2 a, gs_vec2 b, gs_vec2 c, gs_color_t color);
GS_API_DECL void gs_gui_draw_box(gs_gui_context_t* ctx, gs_gui_rect_t rect, gs_color_t color);
GS_API_DECL void gs_gui_draw_text(gs_gui_context_t* ctx, gs_asset_font_t* font, const char *str, int32_t len, gs_vec2 pos, gs_color_t color);
GS_API_DECL void gs_gui_draw_icon(gs_gui_context_t* ctx, int32_t id, gs_gui_rect_t rect, gs_color_t color);
GS_API_DECL void gs_gui_draw_image(gs_gui_context_t* ctx, gs_handle(gs_graphics_texture_t) hndl, gs_gui_rect_t rect, gs_vec2 uv0, gs_vec2 uv1, gs_color_t color);
GS_API_DECL void gs_gui_draw_nine_rect(gs_gui_context_t* ctx, gs_handle(gs_graphics_texture_t) hndl, gs_gui_rect_t rect, gs_vec2 uv0, gs_vec2 uv1, uint32_t left, uint32_t right, uint32_t top, uint32_t bottom, gs_color_t color);
GS_API_DECL void gs_gui_layout_row(gs_gui_context_t *ctx, int32_t items, const int32_t *widths, int32_t height);
GS_API_DECL void gs_gui_layout_width(gs_gui_context_t *ctx, int32_t width);
GS_API_DECL void gs_gui_layout_height(gs_gui_context_t *ctx, int32_t height);
GS_API_DECL void gs_gui_layout_begin_column(gs_gui_context_t *ctx);
GS_API_DECL void gs_gui_layout_end_column(gs_gui_context_t *ctx);
GS_API_DECL void gs_gui_layout_set_next(gs_gui_context_t *ctx, gs_gui_rect_t r, int32_t relative);
GS_API_DECL gs_gui_rect_t gs_gui_layout_next(gs_gui_context_t *ctx);

GS_API_DECL void gs_gui_draw_control_frame(gs_gui_context_t *ctx, gs_gui_id id, gs_gui_rect_t rect, int32_t colorid, int32_t opt);
GS_API_DECL void gs_gui_draw_control_text(gs_gui_context_t *ctx, const char *str, gs_gui_rect_t rect, int32_t colorid, int32_t opt);
GS_API_DECL int32_t gs_gui_mouse_over(gs_gui_context_t *ctx, gs_gui_rect_t rect);
GS_API_DECL void gs_gui_update_control(gs_gui_context_t *ctx, gs_gui_id id, gs_gui_rect_t rect, int32_t opt);

// Widgets
#define gs_gui_button(_CTX, _LABEL)				    gs_gui_button_ex((_CTX), (_LABEL), 0, GS_GUI_OPT_ALIGNCENTER)
#define gs_gui_textbox(_CTX, _BUF, _BUFSZ)			gs_gui_textbox_ex((_CTX), (_BUF), (_BUFSZ), 0)
#define gs_gui_slider(_CTX, _VALUE, _LO, _HI)		gs_gui_slider_ex((_CTX), (_VALUE), (_LO), (_HI), 0, GS_GUI_SLIDER_FMT, GS_GUI_OPT_ALIGNCENTER)
#define gs_gui_number(_CTX, _VALUE, _STEP)			gs_gui_number_ex((_CTX), (_VALUE), (_STEP), GS_GUI_SLIDER_FMT, GS_GUI_OPT_ALIGNCENTER)
#define gs_gui_header(_CTX, _LABEL)		            gs_gui_header_ex((_CTX), (_LABEL), 0)
#define gs_gui_begin_treenode(_CTX, _LABEL)	        gs_gui_begin_treenode_ex((_CTX), (_LABEL), 0)
#define gs_gui_begin_window(_CTX, _TITLE, _RECT)    gs_gui_begin_window_ex((_CTX), (_TITLE), (_RECT), 0)
#define gs_gui_begin_popup(_CTX, _TITLE, _RECT)     gs_gui_begin_popup_ex((_CTX), (_TITLE), (_RECT), 0)
#define gs_gui_begin_panel(_CTX, _NAME)			    gs_gui_begin_panel_ex((_CTX), (_NAME), 0)
#define gs_gui_dock(_CTX, _DST, _SRC, _TYPE)        gs_gui_dock_ex((_CTX), (_DST), (_SRC), (_TYPE), 0.5f)
#define gs_gui_undock(_CTX, _NAME)                  gs_gui_undock_ex((_CTX), (_NAME))

GS_API_DECL void gs_gui_text(gs_gui_context_t* ctx, const char* text, int32_t text_wrap);
GS_API_DECL void gs_gui_label(gs_gui_context_t* ctx, const char* text);
GS_API_DECL int32_t gs_gui_button_ex(gs_gui_context_t* ctx, const char* label, int32_t icon, int32_t opt);
GS_API_DECL int32_t gs_gui_checkbox(gs_gui_context_t* ctx, const char* label, int32_t* state);
GS_API_DECL int32_t gs_gui_textbox_raw(gs_gui_context_t* ctx, char* buf, int32_t bufsz, gs_gui_id id, gs_gui_rect_t r, int32_t opt);
GS_API_DECL int32_t gs_gui_textbox_ex(gs_gui_context_t* ctx, char* buf, int32_t bufsz, int32_t opt);
GS_API_DECL int32_t gs_gui_slider_ex(gs_gui_context_t* ctx, gs_gui_real* value, gs_gui_real low, gs_gui_real high, gs_gui_real step, const char* fmt, int32_t opt);
GS_API_DECL int32_t gs_gui_number_ex(gs_gui_context_t* ctx, gs_gui_real* value, gs_gui_real step, const char* fmt, int32_t opt);
GS_API_DECL int32_t gs_gui_header_ex(gs_gui_context_t* ctx, const char* label, int32_t opt);
GS_API_DECL int32_t gs_gui_begin_treenode_ex(gs_gui_context_t * ctx, const char* label, int32_t opt);
GS_API_DECL void gs_gui_end_treenode(gs_gui_context_t* ctx);
GS_API_DECL int32_t gs_gui_begin_window_ex(gs_gui_context_t * ctx, const char* title, gs_gui_rect_t rect, int32_t opt);
GS_API_DECL void gs_gui_end_window(gs_gui_context_t* ctx);
GS_API_DECL void gs_gui_open_popup(gs_gui_context_t* ctx, const char* name);
GS_API_DECL int32_t gs_gui_begin_popup_ex(gs_gui_context_t* ctx, const char* name, gs_gui_rect_t r, int32_t opt);
GS_API_DECL void gs_gui_end_popup(gs_gui_context_t* ctx);
GS_API_DECL void gs_gui_begin_panel_ex(gs_gui_context_t* ctx, const char* name, int32_t opt);
GS_API_DECL void gs_gui_end_panel(gs_gui_context_t* ctx);

// Docking
GS_API_DECL void gs_gui_dock_ex(gs_gui_context_t* ctx, const char* dst, const char* src, int32_t split_type, float ratio);
GS_API_DECL void gs_gui_undock_ex(gs_gui_context_t* ctx, const char* name);
GS_API_DECL void gs_gui_dock_ex_cnt(gs_gui_context_t* ctx, gs_gui_container_t* dst, gs_gui_container_t* src, int32_t split_type, float ratio);
GS_API_DECL void gs_gui_undock_ex_cnt(gs_gui_context_t* ctx, gs_gui_container_t* cnt);

#ifdef GS_GUI_IMPL 

#define gs_gui_unused(x) ((void) (x))

#define gs_gui_expect(x)\
    do {																                    \
		if (!(x)) {																			\
			gs_println(stderr, "Fatal error: %s:%d: assertion '%s' failed\n",               \
				__FILE__, __LINE__, #x);												    \
			abort();																		\
		}																					\
	} while (0)

#define gs_gui_stack_push(stk, val)\
    do {												                                    \
		gs_gui_expect((stk).idx < (int32_t) (sizeof((stk).items) / sizeof(*(stk).items)));  \
		(stk).items[(stk).idx] = (val);													    \
		(stk).idx++; /* incremented after incase `val` uses this value */			        \
	} while (0)

#define gs_gui_stack_pop(stk)\
    do {			                    \
		gs_gui_expect((stk).idx > 0);   \
		(stk).idx--;					\
	} while (0) 

/* 32bit fnv-1a hash */
#define GS_GUI_HASH_INITIAL 2166136261

static void gs_gui_hash(gs_gui_id *hash, const void *data, int32_t size) 
{
	const unsigned char *p = data;
	while (size--) 
    {
		*hash = (*hash ^ *p++) * 16777619;
	}
} 

static gs_gui_rect_t gs_gui_unclipped_rect = { 0, 0, 0x1000000, 0x1000000 };

static gs_gui_style_t gs_gui_default_style = 
{
	/* font | size | padding | spacing | indent */
	NULL, { 68, 10 }, 5, 4, 24,
	/* title_height | scrollbar_size | thumb_size */
	24, 12, 8,
	{
		{255,  255, 255,  255}, /* GS_GUI_COLOR_TEXT */
		{155,  155, 155,   58}, /* GS_GUI_COLOR_TEXT_INACTIVE */
		{29,	29,  29,   76}, /* GS_GUI_COLOR_BORDER */
		{0,	     0,   0,   31}, /* GS_GUI_COLOR_SHADOW */
		{25,	25,	 25,  255}, /* GS_GUI_COLOR_WINDOWBG */
		{28,	28,	 28,  255}, /* GS_GUI_COLOR_TITLEBG */
		{240,  240, 240,  255}, /* GS_GUI_COLOR_TITLETEXT */
		{10,	10,  10,   59}, /* GS_GUI_COLOR_PANELBG */
		{35,	35,	 35,  255}, /* GS_GUI_COLOR_BUTTON */
		{40,	40,	 40,  255}, /* GS_GUI_COLOR_BUTTONHOVER */
		{0,    214, 121,  255}, /* GS_GUI_COLOR_BUTTONFOCUS */
		{20,	20,	 20,  255}, /* GS_GUI_COLOR_BASE */
		{12,	12,	 12,  255}, /* GS_GUI_COLOR_BASEHOVER */
		{10,	10,	 10,  255}, /* GS_GUI_COLOR_BASEFOCUS */
		{22,	22,	 22,  255}, /* GS_GUI_COLOR_SCROLLBASE */
		{30,	30,	 30,  255},	/* GS_GUI_COLOR_SCROLLTHUMB */
		{0,	     0,	 0,	   0 }	    /* GS_GUI_COLOR_INVISIBLE */
	}
}; 

GS_API_DECL gs_gui_rect_t gs_gui_rect(float x, float y, float w, float h) 
{
	gs_gui_rect_t res;
	res.x = x; res.y = y; res.w = w; res.h = h;
	return res;
} 

static gs_gui_rect_t gs_gui_expand_rect(gs_gui_rect_t rect, int32_t n) 
{
	return gs_gui_rect(rect.x - n, rect.y - n, rect.w + n * 2, rect.h + n * 2);
} 

static gs_gui_rect_t gs_gui_intersect_rects(gs_gui_rect_t r1, gs_gui_rect_t r2) 
{
	int32_t x1 = gs_max(r1.x, r2.x);
	int32_t y1 = gs_max(r1.y, r2.y);
	int32_t x2 = gs_min(r1.x + r1.w, r2.x + r2.w);
	int32_t y2 = gs_min(r1.y + r1.h, r2.y + r2.h);
	if (x2 < x1) { x2 = x1; }
	if (y2 < y1) { y2 = y1; }
	return gs_gui_rect(x1, y1, x2 - x1, y2 - y1);
}

static int32_t gs_gui_rect_overlaps_vec2(gs_gui_rect_t r, gs_vec2 p) 
{
	return p.x >= r.x && p.x < r.x + r.w && p.y >= r.y && p.y < r.y + r.h;
} 

gs_gui_container_t* gs_gui_get_root_most_container(gs_gui_context_t* ctx, gs_gui_split_t* split)
{
    // Get root split, continue to traverse splits until root container is found
}

GS_API_DECL gs_gui_container_t* gs_gui_get_top_most_container(gs_gui_context_t* ctx, gs_gui_split_t* split)
{
    if (!split) return NULL;
    if (split->children[0].type == GS_GUI_SPLIT_NODE_CONTAINER) return split->children[0].container;
    if (split->children[1].type == GS_GUI_SPLIT_NODE_CONTAINER) return split->children[1].container;
    gs_gui_container_t* c0 = gs_gui_get_top_most_container(ctx, gs_slot_array_getp(ctx->splits, split->children[0].split));
    gs_gui_container_t* c1 = gs_gui_get_top_most_container(ctx, gs_slot_array_getp(ctx->splits, split->children[1].split));
    if (c0->zindex > c1->zindex) return c0;
    return c1;
}

GS_API_DECL void gs_gui_bring_split_to_front(gs_gui_context_t* ctx, gs_gui_split_t* split)
{
    if (!split) return; 

    if (!split->parent)
    {
        gs_snprintfc(TMP, 256, "!dockspace%zu", (size_t)split); 
        gs_gui_id id = gs_gui_get_id(ctx, TMP, 256);
        gs_gui_container_t* cnt = gs_gui_get_container(ctx, TMP); 
        // if (cnt) gs_gui_bring_to_front(ctx, cnt);
        // cnt->zindex = 0;
    }
    
    gs_gui_split_node_t* c0 = &split->children[0];
    gs_gui_split_node_t* c1 = &split->children[1];

    if (c0->type == GS_GUI_SPLIT_NODE_CONTAINER) 
    {
        gs_gui_bring_to_front(ctx, c0->container); 
        ctx->hover = c0;
    }
    else
    {
        gs_gui_split_t* s = gs_slot_array_getp(ctx->splits, c0->split);
        gs_gui_bring_split_to_front(ctx, s);
    }

    if (c1->type == GS_GUI_SPLIT_NODE_CONTAINER) 
    {
        gs_gui_bring_to_front(ctx, c1->container); 
        ctx->hover = c1;
    }
    else
    {
        gs_gui_split_t* s = gs_slot_array_getp(ctx->splits, c1->split);
        gs_gui_bring_split_to_front(ctx, s);
    } 
}

static void gs_gui_update_split(gs_gui_context_t* ctx, gs_gui_split_t* split)
{
    // Iterate through children, resize them based on size/position 
    const gs_gui_rect_t* sr = &split->rect;
    const float ratio = split->ratio;
    switch (split->type)
    {
        case GS_GUI_SPLIT_LEFT:
        {
            if (split->children[GS_GUI_SPLIT_NODE_PARENT].type == GS_GUI_SPLIT_NODE_SPLIT)
            { 
                // Update split
                gs_gui_split_t* s = gs_slot_array_getp(ctx->splits, split->children[GS_GUI_SPLIT_NODE_PARENT].split);
                s->rect = gs_gui_rect(sr->x + sr->w * ratio, sr->y, sr->w * (1.f - ratio), sr->h); 
                gs_gui_update_split(ctx, s);
            }

            if (split->children[GS_GUI_SPLIT_NODE_CHILD].type == GS_GUI_SPLIT_NODE_SPLIT)
            {
                gs_gui_split_t* s = gs_slot_array_getp(ctx->splits, split->children[GS_GUI_SPLIT_NODE_CHILD].split);
                s->rect = gs_gui_rect(sr->x, sr->y, sr->w * (ratio), sr->h);                
                gs_gui_update_split(ctx, s);
            }
        } break;

        case GS_GUI_SPLIT_RIGHT:
        { 
            if (split->children[GS_GUI_SPLIT_NODE_CHILD].type == GS_GUI_SPLIT_NODE_SPLIT)
            { 
                // Update split
                gs_gui_split_t* s = gs_slot_array_getp(ctx->splits, split->children[GS_GUI_SPLIT_NODE_CHILD].split);
                s->rect = gs_gui_rect(sr->x + sr->w * (1.f - ratio), sr->y, sr->w * (ratio), sr->h);                
                gs_gui_update_split(ctx, s);
            }

            if (split->children[GS_GUI_SPLIT_NODE_PARENT].type == GS_GUI_SPLIT_NODE_SPLIT)
            {
                gs_gui_split_t* s = gs_slot_array_getp(ctx->splits, split->children[GS_GUI_SPLIT_NODE_PARENT].split);
                s->rect = gs_gui_rect(sr->x, sr->y, sr->w * (1.f - ratio), sr->h);                
                gs_gui_update_split(ctx, s);
            }

        } break;

        case GS_GUI_SPLIT_TOP:
        { 
            if (split->children[GS_GUI_SPLIT_NODE_PARENT].type == GS_GUI_SPLIT_NODE_SPLIT)
            { 
                // Update split
                gs_gui_split_t* s = gs_slot_array_getp(ctx->splits, split->children[GS_GUI_SPLIT_NODE_PARENT].split);
                s->rect = gs_gui_rect(sr->x, sr->y + sr->h * (ratio), sr->w, sr->h * (1.f - ratio));                
                gs_gui_update_split(ctx, s);
            }

            if (split->children[GS_GUI_SPLIT_NODE_CHILD].type == GS_GUI_SPLIT_NODE_SPLIT)
            {
                gs_gui_split_t* s = gs_slot_array_getp(ctx->splits, split->children[GS_GUI_SPLIT_NODE_CHILD].split);
                s->rect = gs_gui_rect(sr->x, sr->y, sr->w, sr->h * (ratio)); 
                gs_gui_update_split(ctx, s);
            } 
        } break;

        case GS_GUI_SPLIT_BOTTOM:
        { 
            if (split->children[GS_GUI_SPLIT_NODE_CHILD].type == GS_GUI_SPLIT_NODE_SPLIT)
            { 
                // Update split
                gs_gui_split_t* s = gs_slot_array_getp(ctx->splits, split->children[GS_GUI_SPLIT_NODE_CHILD].split);
                s->rect = gs_gui_rect(sr->x, sr->y + sr->h * (1.f - ratio), sr->w, sr->h * (ratio));                
                gs_gui_update_split(ctx, s);
            }

            if (split->children[GS_GUI_SPLIT_NODE_PARENT].type == GS_GUI_SPLIT_NODE_SPLIT)
            {
                gs_gui_split_t* s = gs_slot_array_getp(ctx->splits, split->children[GS_GUI_SPLIT_NODE_PARENT].split);
                s->rect = gs_gui_rect(sr->x, sr->y, sr->w, sr->h * (1.f - ratio));                
                gs_gui_update_split(ctx, s);
            } 
        } break;
    } 
}

static gs_gui_split_t* gs_gui_get_root_split_from_split(gs_gui_context_t* ctx, gs_gui_split_t* split)
{
    if (!split) return NULL;

    // Cache top root level split
    gs_gui_split_t* root_split = split && split->parent ? gs_slot_array_getp(ctx->splits, split->parent) : split ? split : NULL;
    while (root_split && root_split->parent)
    {
        root_split = gs_slot_array_getp(ctx->splits, root_split->parent); 
    }

    return root_split; 
}

static gs_gui_split_t* gs_gui_get_root_split(gs_gui_context_t* ctx, gs_gui_container_t* cnt)
{
    gs_gui_split_t* split = gs_gui_get_split(ctx, cnt); 
    if (split) return gs_gui_get_root_split_from_split(ctx, split);
    else return NULL;
} 

GS_API_DECL gs_gui_container_t* gs_gui_get_root_container_from_split(gs_gui_context_t* ctx, gs_gui_split_t* split)
{
    gs_gui_split_t* root = gs_gui_get_root_split_from_split(ctx, split);
    gs_gui_split_t* s = root;
    gs_gui_container_t* c = NULL;
    while (s && !c)
    {
        if (s->children[GS_GUI_SPLIT_NODE_PARENT].type == GS_GUI_SPLIT_NODE_SPLIT)
        {
            s = gs_slot_array_getp(ctx->splits, s->children[GS_GUI_SPLIT_NODE_PARENT].split);
        }
        else if (s->children[GS_GUI_SPLIT_NODE_PARENT].type == GS_GUI_SPLIT_NODE_CONTAINER)
        {
            c = s->children[GS_GUI_SPLIT_NODE_PARENT].container;
        } 
        else
        {
            s = NULL;
        }
    }
    return c;
}

GS_API_DECL gs_gui_container_t* gs_gui_get_root_container(gs_gui_context_t* ctx, gs_gui_container_t* cnt)
{
    gs_gui_container_t* parent = gs_gui_get_parent(ctx, cnt);
    if (parent->split)
    {
        gs_gui_split_t* root = gs_gui_get_root_split(ctx, parent);
        gs_gui_split_t* s = root;
        gs_gui_container_t* c = NULL;
        while (s && !c)
        {
            if (s->children[GS_GUI_SPLIT_NODE_PARENT].type == GS_GUI_SPLIT_NODE_SPLIT)
            {
                s = gs_slot_array_getp(ctx->splits, s->children[GS_GUI_SPLIT_NODE_PARENT].split);
            }
            else if (s->children[GS_GUI_SPLIT_NODE_PARENT].type == GS_GUI_SPLIT_NODE_CONTAINER)
            {
                c = s->children[GS_GUI_SPLIT_NODE_PARENT].container;
            } 
            else
            {
                s = NULL;
            }
        }
        return c;

    }

    return parent; 
}

GS_API_DECL gs_gui_tab_bar_t* gs_gui_get_tab_bar(gs_gui_context_t* ctx, gs_gui_container_t* cnt)
{
    return ((cnt->tab_bar && cnt->tab_bar< gs_slot_array_size(ctx->tab_bars)) ? gs_slot_array_getp(ctx->tab_bars, cnt->tab_bar) : NULL);
} 

GS_API_DECL gs_gui_split_t* gs_gui_get_split(gs_gui_context_t* ctx, gs_gui_container_t* cnt)
{
	gs_gui_tab_bar_t* tab_bar = gs_gui_get_tab_bar(ctx, cnt);
    gs_gui_tab_item_t* tab_item = tab_bar ? &tab_bar->items[cnt->tab_item] : NULL;
    gs_gui_split_t* split = cnt->split ? gs_slot_array_getp(ctx->splits, cnt->split) : NULL;

    // Look at split if in tab group
    if (!split && tab_bar)
    {
        for (uint32_t i = 0; i < tab_bar->size; ++i)
        {
            if (((gs_gui_container_t*)tab_bar->items[i].data)->split)
            {
                split = gs_slot_array_getp(ctx->splits, ((gs_gui_container_t*)tab_bar->items[i].data)->split);
            }
        }
    }

    return split;
} 

static gs_gui_command_t* gs_gui_push_jump(gs_gui_context_t* ctx, gs_gui_command_t* dst) 
{
	gs_gui_command_t* cmd;
	cmd = gs_gui_push_command(ctx, GS_GUI_COMMAND_JUMP, sizeof(gs_gui_jumpcommand_t));
	cmd->jump.dst = dst;
	return cmd;
} 

static void gs_gui_draw_frame(gs_gui_context_t* ctx, gs_gui_rect_t rect, int32_t colorid) 
{
	gs_gui_draw_rect(ctx, rect, ctx->style->colors[colorid]);

	if (
        colorid == GS_GUI_COLOR_SCROLLBASE || 
        colorid == GS_GUI_COLOR_SCROLLTHUMB || 
        colorid == GS_GUI_COLOR_TITLEBG
    ) 
    { 
        return; 
    }

	// draw border
	if (ctx->style->colors[GS_GUI_COLOR_BORDER].a) 
    {
		gs_gui_draw_box(ctx, gs_gui_expand_rect(rect, 1), ctx->style->colors[GS_GUI_COLOR_BORDER]);
	}
}

static int32_t gs_gui_compare_zindex(const void *a, const void *b) 
{
	return (*(gs_gui_container_t**) a)->zindex - (*(gs_gui_container_t**) b)->zindex;
} 

static void gs_gui_push_layout(gs_gui_context_t *ctx, gs_gui_rect_t body, gs_vec2 scroll) 
{
	gs_gui_layout_t layout;
	int32_t width = 0;
	memset(&layout, 0, sizeof(layout));
	layout.body = gs_gui_rect(body.x - scroll.x, body.y - scroll.y, body.w, body.h);
	layout.max = gs_v2(-0x1000000, -0x1000000);
	gs_gui_stack_push(ctx->layout_stack, layout);
	gs_gui_layout_row(ctx, 1, &width, 0);
} 

static gs_gui_layout_t* gs_gui_get_layout(gs_gui_context_t *ctx) 
{
	return &ctx->layout_stack.items[ctx->layout_stack.idx - 1];
}

static void gs_gui_pop_container(gs_gui_context_t *ctx) 
{
	gs_gui_container_t *cnt = gs_gui_get_current_container(ctx);
	gs_gui_layout_t *layout = gs_gui_get_layout(ctx);
	cnt->content_size.x = layout->max.x - layout->body.x;
	cnt->content_size.y = layout->max.y - layout->body.y;

	/* pop container, layout and id */
	gs_gui_stack_pop(ctx->container_stack);
	gs_gui_stack_pop(ctx->layout_stack);
	gs_gui_pop_id(ctx);
} 

#define gs_gui_scrollbar(ctx, cnt, b, cs, x, y, w, h)									    \
	do {																				    \
		/* only add scrollbar if content size is larger than body */						\
		int32_t maxscroll = cs.y - b->h;													\
																						    \
		if (maxscroll > 0 && b->h > 0) {													\
			gs_gui_rect_t base, thumb;														\
			gs_gui_id id = gs_gui_get_id(ctx, "!scrollbar" #y, 11);							\
																							\
			/* get sizing / positioning */													\
			base = *b;																	    \
			base.x = b->x + b->w;															\
			base.w = ctx->style->scrollbar_size;											\
																							\
			/* handle input */																\
			gs_gui_update_control(ctx, id, base, 0);										\
			if (ctx->focus == id && ctx->mouse_down == GS_GUI_MOUSE_LEFT) {					\
				cnt->scroll.y += ctx->mouse_delta.y * cs.y / base.h;					    \
			}																				\
			/* clamp scroll to limits */													\
			cnt->scroll.y = gs_clamp(cnt->scroll.y, 0, maxscroll);					        \
																							\
			/* draw base and thumb */														\
			ctx->draw_frame(ctx, base, GS_GUI_COLOR_SCROLLBASE);							\
			thumb = base;																    \
			thumb.h = gs_max(ctx->style->thumb_size, base.h * b->h / cs.y);			        \
			thumb.y += cnt->scroll.y * (base.h - thumb.h) / maxscroll;						\
			ctx->draw_frame(ctx, thumb, GS_GUI_COLOR_SCROLLTHUMB);							\
																							\
			/* set this as the scroll_target (will get scrolled on mousewheel) */           \
			/* if the mouse is over it */													\
			if (gs_gui_mouse_over(ctx, *b)) { ctx->scroll_target = cnt; }				    \
		} else {																			\
			cnt->scroll.y = 0;																\
		}																					\
	} while (0) 

static void gs_gui_scrollbars(gs_gui_context_t *ctx, gs_gui_container_t *cnt, gs_gui_rect_t *body) 
{
	int32_t sz = ctx->style->scrollbar_size;
	gs_vec2 cs = cnt->content_size;
	cs.x += ctx->style->padding * 2;
	cs.y += ctx->style->padding * 2;
	gs_gui_push_clip_rect(ctx, *body);

	/* resize body to make room for scrollbars */
	if (cs.y > cnt->body.h) { body->w -= sz; }
	if (cs.x > cnt->body.w) { body->h -= sz; }

	/* to create a horizontal or vertical scrollbar almost-identical code is
	** used; only the references to `x|y` `w|h` need to be switched */
	gs_gui_scrollbar(ctx, cnt, body, cs, x, y, w, h);
	gs_gui_scrollbar(ctx, cnt, body, cs, y, x, h, w);
	gs_gui_pop_clip_rect(ctx);
}


static void gs_gui_push_container_body(gs_gui_context_t *ctx, gs_gui_container_t *cnt, gs_gui_rect_t body, int32_t opt) 
{
	if (~opt & GS_GUI_OPT_NOSCROLL) {gs_gui_scrollbars(ctx, cnt, &body);}
	gs_gui_push_layout(ctx, gs_gui_expand_rect(body, -ctx->style->padding), cnt->scroll);
	cnt->body = body;
} 

static void gs_gui_begin_root_container(gs_gui_context_t *ctx, gs_gui_container_t *cnt, int32_t opt) 
{
	gs_gui_stack_push(ctx->container_stack, cnt);

	/* push container to roots list and push head command */
	gs_gui_stack_push(ctx->root_list, cnt);
	cnt->head = gs_gui_push_jump(ctx, NULL);

	/* set as hover root if the mouse is overlapping this container and it has a
	** higher zindex than the current hover root */
	if (
        gs_gui_rect_overlaps_vec2(cnt->rect, ctx->mouse_pos) &&
		(!ctx->next_hover_root || cnt->zindex > ctx->next_hover_root->zindex) &&
        ~opt & GS_GUI_OPT_NOHOVER && cnt->flags & GS_GUI_WINDOW_FLAGS_VISIBLE
	) 
    {
		ctx->next_hover_root = cnt;
	}

	/* clipping is reset here in case a root-container is made within
	** another root-containers's begin/end block; this prevents the inner
	** root-container being clipped to the outer */ 
	gs_gui_stack_push(ctx->clip_stack, gs_gui_unclipped_rect);
}

static void gs_gui_end_root_container(gs_gui_context_t *ctx) 
{
	/* push tail 'goto' jump command and set head 'skip' command. the final steps
	** on initing these are done in gs_gui_end() */
	gs_gui_container_t *cnt = gs_gui_get_current_container(ctx);
	cnt->tail = gs_gui_push_jump(ctx, NULL);
	cnt->head->jump.dst = ctx->command_list.items + ctx->command_list.idx;

	/* pop base clip rect and container */
	gs_gui_pop_clip_rect(ctx);
	gs_gui_pop_container(ctx);
} 

GS_API_DECL gs_gui_container_t* gs_gui_get_container_ex(gs_gui_context_t* ctx, gs_gui_id id, int32_t opt) 
{
	gs_gui_container_t *cnt;

	/* try to get existing container from pool */
	int32_t idx = gs_gui_pool_get(ctx, ctx->container_pool, GS_GUI_CONTAINERPOOL_SIZE, id);

	if (idx >= 0) 
    {
		if (ctx->containers[idx].open || ~opt & GS_GUI_OPT_CLOSED) 
        {
			gs_gui_pool_update(ctx, ctx->container_pool, idx);
		}
		return &ctx->containers[idx];
	}

	if (opt & GS_GUI_OPT_CLOSED) { return NULL; }

	/* container not found in pool: init new container */
	idx = gs_gui_pool_init(ctx, ctx->container_pool, GS_GUI_CONTAINERPOOL_SIZE, id);
	cnt = &ctx->containers[idx];
	memset(cnt, 0, sizeof(*cnt));
	cnt->open = 1;
	cnt->id = id;
    cnt->flags |= GS_GUI_WINDOW_FLAGS_VISIBLE;
	gs_gui_bring_to_front(ctx, cnt);

	return cnt;
}

static int32_t gs_gui_text_width(gs_asset_font_t* font, const char* text, int32_t len) 
{ 
    gs_vec2 td = gs_asset_font_text_dimensions(font, text, len);
    return (int32_t)td.x;
}

static int32_t gs_gui_text_height(gs_asset_font_t* font, const char* text, int32_t len) 
{
    return (int32_t)gs_asset_font_max_height(font);
    gs_vec2 td = gs_asset_font_text_dimensions(font, text, len);
    return (int32_t)td.y;
} 

// Grabs max height for a given font
static int32_t gs_gui_font_height(gs_asset_font_t* font)
{
    return (int32_t)gs_asset_font_max_height(font);
}

static gs_vec2 gs_gui_text_dimensions(gs_asset_font_t* font, const char* text, int32_t len) 
{
    gs_vec2 td = gs_asset_font_text_dimensions(font, text, len);
    return td;
} 

// =========================== //
// ======== Docking ========== //
// =========================== //

GS_API_DECL void gs_gui_dock_ex(gs_gui_context_t* ctx, const char* dst, const char* src, int32_t split_type, float ratio)
{
    gs_gui_container_t* dst_cnt = gs_gui_get_container(ctx, dst);
    gs_gui_container_t* src_cnt = gs_gui_get_container(ctx, src); 
    gs_gui_dock_ex_cnt(ctx, dst_cnt, src_cnt, split_type, ratio); 
}

GS_API_DECL void gs_gui_undock_ex(gs_gui_context_t* ctx, const char* name)
{
    gs_gui_container_t* cnt = gs_gui_get_container(ctx, name);
    gs_gui_undock_ex_cnt(ctx, cnt);
}

void gs_gui_set_split(gs_gui_context_t* ctx, gs_gui_container_t* cnt, uint32_t id)
{
    cnt->split = id; 
    gs_gui_tab_bar_t* tb = gs_gui_get_tab_bar(ctx, cnt);
    if (tb)
    {
        for (uint32_t i = 0; i < tb->size; ++i) 
        {
            ((gs_gui_container_t*)tb->items[i].data)->split = id;
        }
    }
}

GS_API_DECL gs_gui_container_t* gs_gui_get_parent(gs_gui_context_t* ctx, gs_gui_container_t* cnt)
{
    return (cnt->parent ? cnt->parent : cnt);
}

GS_API_DECL void gs_gui_dock_ex_cnt(gs_gui_context_t* ctx, gs_gui_container_t* child, gs_gui_container_t* parent, int32_t split_type, float ratio)
{ 
    // Get top-level parent windows 
    parent = gs_gui_get_parent(ctx, parent);
    child = gs_gui_get_parent(ctx, child);

    if (!child || !parent)
    {
        return;
    } 

    if (split_type == GS_GUI_SPLIT_TAB)
    { 
        // If the parent window has a tab bar, then need to get that tab bar item and add it 
        if (parent->tab_bar)
        {
            gs_println("add to tab bar");

            gs_gui_tab_bar_t* tab_bar = gs_slot_array_getp(ctx->tab_bars, parent->tab_bar);
            gs_assert(tab_bar); 
              
            // Set all tab bar children to this as well, if has children, then release previous tab bar
            if (child->tab_bar)
            {
                uint32_t tbid = child->tab_bar;
                gs_gui_tab_bar_t* ctb = gs_slot_array_getp(ctx->tab_bars, child->tab_bar); 
                for (uint32_t i = 0; i < ctb->size; ++i)
                {
                    gs_gui_tab_item_t* cti = &tab_bar->items[tab_bar->size]; 
                    gs_gui_container_t* c = (gs_gui_container_t*)ctb->items[i].data; 
                    cti->tab_bar = parent->tab_bar;
                    cti->zindex = tab_bar->size++;
					cti->idx = cti->zindex;
                    cti->data = c;
                    c->tab_item = cti->idx;
                    c->parent = parent;
                }

                // Free other tab bar
                // gs_slot_array_erase(ctx->tab_bars, tbid);
            }
			else
			{
				gs_gui_tab_item_t* tab_item = &tab_bar->items[tab_bar->size];
				tab_item->tab_bar = parent->tab_bar;
				tab_item->zindex = tab_bar->size++;
				tab_item->idx = tab_item->zindex;
				tab_bar->focus = tab_bar->size - 1;
				child->tab_item = tab_item->idx; 
			}

            tab_bar->items[child->tab_item].data = child;
            child->rect = parent->rect;
            child->parent = parent; 
            child->tab_bar = parent->tab_bar;
        }
        // Otherwise, create new tab bar
        else
        {
            gs_println("create tab bar");

            // Create tab bar
            gs_gui_tab_bar_t tb = gs_default_val();
            uint32_t hndl = gs_slot_array_insert(ctx->tab_bars, tb);
            gs_gui_tab_bar_t* tab_bar = gs_slot_array_getp(ctx->tab_bars, hndl);
            gs_assert(tab_bar);

            // Create tab items
            gs_gui_tab_item_t* parent_tab_item = &tab_bar->items[tab_bar->size];
            parent_tab_item->zindex = tab_bar->size++; 
            parent_tab_item->tab_bar = hndl;

            // Set parent tab item
            parent->tab_item = 0;
            parent_tab_item->data = parent; 

            uint32_t tbid = child->tab_bar;

            // Set all tab bar children to this as well, if has children, then release previous tab bar
            if (child->tab_bar)
            {
                gs_gui_tab_bar_t* ctb = gs_slot_array_getp(ctx->tab_bars, child->tab_bar); 
                for (uint32_t i = 0; i < ctb->size; ++i)
                {
                    gs_gui_tab_item_t* cti = &tab_bar->items[tab_bar->size]; 
                    gs_gui_container_t* c = (gs_gui_container_t*)ctb->items[i].data; 
                    cti->tab_bar = hndl;
                    cti->zindex = tab_bar->size++;
					cti->idx = cti->zindex;
                    cti->data = c;
                    c->tab_item = cti->idx;
                    c->parent = parent;
                    c->tab_bar = hndl;
                }

                // TODO(john): This erase is causing a crash.
                // gs_slot_array_erase(ctx->tab_bars, tbid);
            }
			else
			{
				gs_gui_tab_item_t* child_tab_item = &tab_bar->items[tab_bar->size];
				child_tab_item->zindex = tab_bar->size++; 
				child_tab_item->idx = child_tab_item->zindex;
				child_tab_item->tab_bar = hndl; 

				// Set child tab item
				child->tab_item = child_tab_item->idx; 
				child_tab_item->data = child;
			}

			tab_bar->focus = 1;
            child->rect = parent->rect; 
            child->parent = parent;
            tab_bar->rect = parent->rect;

			parent->tab_bar = hndl;
			child->tab_bar = hndl;

            // Bring everything to front...right?

        }

        gs_gui_split_t* root_split = gs_gui_get_root_split(ctx, parent);
        if (root_split)
        {
            gs_gui_update_split(ctx, root_split); 
            gs_gui_bring_split_to_front(ctx, root_split);
        }
    }
    else
    { 
        // Cache previous root splits
        gs_gui_split_t* ps = gs_gui_get_root_split(ctx, parent); 
        gs_gui_split_t* cs = gs_gui_get_root_split(ctx, child); 

        gs_gui_tab_bar_t* tab_bar = gs_gui_get_tab_bar(ctx, parent);

        gs_gui_split_t split = gs_default_val();
        split.type = split_type;       
        split.ratio = ratio; 
        gs_gui_split_node_t c0 = gs_default_val(); 
        c0.type = GS_GUI_SPLIT_NODE_CONTAINER;
        c0.container = child;
        gs_gui_split_node_t c1 = gs_default_val();
        c1.type = GS_GUI_SPLIT_NODE_CONTAINER;
        c1.container = parent; 
        split.children[GS_GUI_SPLIT_NODE_CHILD] = c0;
        split.children[GS_GUI_SPLIT_NODE_PARENT] = c1;
        split.rect = ps ? ps->rect : parent->rect;
        split.prev_rect = split.rect;

        // Add new split to array
        uint32_t hndl = gs_slot_array_insert(ctx->splits, split);

        // Get newly inserted split pointer
        gs_gui_split_t* sp = gs_slot_array_getp(ctx->splits, hndl);
        sp->id = hndl;

        // If both parents are null, creating a new split, new nodes, assigning to children's parents
        if (!cs && !ps)
        { 
            gs_println("0");
            parent->split = hndl;
            child->split = hndl;
        } 

        // Child has split
        else if (cs && !ps)
        { 
            gs_println("1");
            // If child has split, then the split is different...
            sp->children[GS_GUI_SPLIT_NODE_CHILD].type = GS_GUI_SPLIT_NODE_SPLIT;
            sp->children[GS_GUI_SPLIT_NODE_CHILD].split = cs->id;

            // Child split needs to be set to this parent 
            cs->parent = hndl;

            parent->split = hndl;
        }

        // Parent has split
        else if (ps && !cs)
        { 
            gs_println("2"); 

            // No child to tree to assign, so we can get the raw parent split here
            ps = gs_slot_array_getp(ctx->splits, parent->split); 

            // Assign parent split to previous
            sp->parent = ps->id; 

            // Fix up references
            if (ps->children[GS_GUI_SPLIT_NODE_PARENT].container == parent)
            {
                ps->children[GS_GUI_SPLIT_NODE_PARENT].type = GS_GUI_SPLIT_NODE_SPLIT; 
                ps->children[GS_GUI_SPLIT_NODE_PARENT].split = hndl;
            }
            else
            {
                ps->children[GS_GUI_SPLIT_NODE_CHILD].type = GS_GUI_SPLIT_NODE_SPLIT; 
                ps->children[GS_GUI_SPLIT_NODE_CHILD].split = hndl;
            }

            parent->split = hndl;
            child->split = hndl;
        }

        // Both have splits
        else
        { 
            gs_println("3"); 

            // Get parent split
            ps = gs_slot_array_getp(ctx->splits, parent->split); 

            // Set parent id for new split to parent previous split
            sp->parent = ps->id;

            // Fix up references
            sp->children[GS_GUI_SPLIT_NODE_CHILD].type = GS_GUI_SPLIT_NODE_SPLIT;
            sp->children[GS_GUI_SPLIT_NODE_CHILD].split = cs->id;

            // Need to check which node to replace
            if (ps->children[GS_GUI_SPLIT_NODE_CHILD].container == parent)
            {
                ps->children[GS_GUI_SPLIT_NODE_CHILD].type = GS_GUI_SPLIT_NODE_SPLIT;
                ps->children[GS_GUI_SPLIT_NODE_CHILD].split = hndl;
            }
            else
            {
                ps->children[GS_GUI_SPLIT_NODE_PARENT].type = GS_GUI_SPLIT_NODE_SPLIT;
                ps->children[GS_GUI_SPLIT_NODE_PARENT].split = hndl;
            }

            cs->parent = hndl;
            parent->split = hndl;
        } 

        gs_gui_split_t* root_split = gs_gui_get_root_split(ctx, parent);
        gs_gui_update_split(ctx, root_split); 
        gs_gui_bring_split_to_front(ctx, root_split);
    }
}

GS_API_DECL void gs_gui_undock_ex_cnt(gs_gui_context_t* ctx, gs_gui_container_t* cnt) 
{ 
    // If has a tab item idx, need to grab that
    gs_gui_split_t* split = gs_gui_get_split(ctx, cnt);

    // Get root split for container
    gs_gui_split_t* root_split = NULL;

    // Get parent split of this owning split
    gs_gui_split_t* ps = split && split->parent ? gs_slot_array_getp(ctx->splits, split->parent) : NULL;

    if (cnt->tab_bar)
    {
        // Get parent container for this container
        gs_gui_container_t* parent = gs_gui_get_parent(ctx, cnt);

        // Check if split
        if (parent->split)
        {
            // No split, so just do stuff normally...
            gs_gui_tab_bar_t* tab_bar = gs_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
            gs_gui_tab_item_t* tab_item = &tab_bar->items[cnt->tab_item];

            if (tab_bar->size > 2)
            {
                // Get index
                uint32_t idx = 0;
                for (uint32_t i = 0; i < tab_bar->size; ++i)
                {
                    if (tab_bar->items[i].data == cnt)
                    {
                        idx = i; 
                        break;
                    }
                } 

				// Swap all the way down the chain
				for (uint32_t i = idx; i < tab_bar->size; ++i)
				{
					gs_gui_tab_item_swap(ctx, tab_bar->items[i].data, +1);
				} 

                // Swap windows as well
                ((gs_gui_container_t*)(tab_item->data))->tab_item = tab_item->idx; 

                // Set focus to first window
				tab_bar->focus = idx ? idx - 1 : idx;
                gs_assert(tab_bar->items[tab_bar->focus].data != cnt); 

                // Set split for focus
                if (parent == cnt)
                {
					// Set parent for other containers (assuming parent was removed)
					for (uint32_t i = 0; i < tab_bar->size; ++i)
					{ 
						gs_gui_container_t* c = tab_bar->items[i].data;
						c->parent = tab_bar->items[tab_bar->focus].data;
                        tab_bar->items[i].idx = i;
                        tab_bar->items[i].zindex = i;
					}

                    gs_gui_container_t* fcnt = tab_bar->items[tab_bar->focus].data;
                    fcnt->split = parent->split;
                    fcnt->flags |= GS_GUI_WINDOW_FLAGS_VISIBLE;

                    // Fix up split reference
                    split = gs_slot_array_getp(ctx->splits, fcnt->split);
                    if (split->children[0].type == GS_GUI_SPLIT_NODE_CONTAINER && split->children[0].container == cnt)
                    {
                        split->children[0].container = fcnt;
                    }
                    else
                    {
                        split->children[1].container = fcnt;
                    }
                }

                // Set size
                tab_bar->size--; 
            }
            // Destroy tab
            else
            {
                uint32_t tbid = tab_item->tab_bar;

                // Get index
                uint32_t idx = 0;
                for (uint32_t i = 0; i < tab_bar->size; ++i)
                {
                    if (tab_bar->items[i].data == cnt)
                    {
                        idx = i; 
                        break;
                    }
                } 

				// Swap all the way down the chain
				for (uint32_t i = idx; i < tab_bar->size; ++i)
				{
					gs_gui_tab_item_swap(ctx, tab_bar->items[i].data, +1);
				} 

                for (uint32_t i = 0; i < tab_bar->size; ++i)
                {
                    gs_gui_container_t* fcnt = tab_bar->items[i].data;
                    fcnt->rect = tab_bar->rect; 
                    fcnt->tab_item = 0x00;
                    fcnt->tab_bar = 0x00;
                    fcnt->parent = NULL;
                    fcnt->flags |= GS_GUI_WINDOW_FLAGS_VISIBLE;
                }

                // Fix up split reference
                if (parent == cnt)
                { 
					tab_bar->focus = idx ? idx - 1 : idx;

                    gs_assert(tab_bar->items[tab_bar->focus].data != cnt);

                    gs_gui_container_t* fcnt = tab_bar->items[tab_bar->focus].data;
                    fcnt->split = parent->split;

                    // Fix up split reference
                    split = gs_slot_array_getp(ctx->splits, fcnt->split);
                    if (split->children[0].type == GS_GUI_SPLIT_NODE_CONTAINER && split->children[0].container == parent)
                    {
                        split->children[0].container = fcnt;
                    }
                    else
                    {
                        split->children[1].container = fcnt;
                    } 
                } 

                // gs_slot_array_erase(ctx->tab_bars, tbid);
            }

            // Remove tab index from container
            cnt->tab_item = 0x00;
            cnt->tab_bar = 0x00;
            // Remove parent
            cnt->parent = NULL;
            // Set split to 0
            cnt->split = 0x00;

            gs_gui_bring_to_front(ctx, cnt); 

        }
        else
        {
            // No split, so just do stuff normally...
            gs_gui_tab_bar_t* tab_bar = gs_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
            gs_gui_tab_item_t* tab_item = &tab_bar->items[cnt->tab_item];

            // Set next available window to visible/focused and rearrange all tab item zindices
            if (tab_bar->size > 2)
            { 
				uint32_t idx = 0;
				for (uint32_t i = 0; i < tab_bar->size; ++i)
				{
					if (tab_bar->items[i].data == cnt)
					{
						idx = i; 
						break;
					}
				}

				// Swap all the way down the chain
				for (uint32_t i = idx; i < tab_bar->size; ++i)
				{
					gs_gui_tab_item_swap(ctx, tab_bar->items[i].data, +1);
				} 

                // Set size
                tab_bar->size--; 

                // Set focus to first window
                tab_bar->focus = idx ? idx - 1 : idx; 

                // Set parent for other containers
				if (parent == cnt)
				{
					for (uint32_t i = 0; i < tab_bar->size; ++i)
					{
						gs_gui_container_t* c = tab_bar->items[i].data;
						c->parent = tab_bar->items[tab_bar->focus].data;
					} 
				}
            }
            // Only 2 windows left in tab bar
            else
            { 
                for (uint32_t i = 0; i < tab_bar->size; ++i)
                {
                    gs_gui_container_t* fcnt = tab_bar->items[i].data;
                    fcnt->rect = tab_bar->rect; 
                    fcnt->tab_item = 0x00;
                    fcnt->tab_bar = 0x00;
                    fcnt->parent = NULL;
                    fcnt->flags |= GS_GUI_WINDOW_FLAGS_VISIBLE;
                }

                tab_bar->size = 0;

                // Destroy tab bar, reset windows
                // gs_slot_array_erase(ctx->tab_bars, tab_item->tab_bar);
            } 

            // Remove tab index from container
            cnt->tab_item = 0x00;
            cnt->tab_bar = 0x00;
            // Remove parent
            cnt->parent = NULL;

            gs_gui_bring_to_front(ctx, cnt); 
        } 
    }
    else
    {
        // Rmove split reference from container
        cnt->split = 0x00; 

        gs_gui_split_node_t* remain_node = 
            split->children[GS_GUI_SPLIT_NODE_CHILD].container == cnt? &split->children[GS_GUI_SPLIT_NODE_PARENT]: 
            split->children[GS_GUI_SPLIT_NODE_PARENT].container == cnt? &split->children[GS_GUI_SPLIT_NODE_CHILD] : 
            NULL;

        gs_assert(remain_node);

        // Set child split in prev container split to split container parent
        if (ps)
        { 
            uint32_t node_id = ps->children[GS_GUI_SPLIT_NODE_CHILD].split == split->id ? GS_GUI_SPLIT_NODE_CHILD : GS_GUI_SPLIT_NODE_PARENT; 
            gs_gui_split_node_t* fix_node = &ps->children[node_id]; 
            *fix_node = *remain_node;
            switch (fix_node->type)
            {
                case GS_GUI_SPLIT_NODE_CONTAINER:
                {
                    gs_gui_container_t* remcnt = fix_node->container;    
                    remcnt->split = ps->id;
                } break;

                case GS_GUI_SPLIT_NODE_SPLIT:
                {
                    gs_gui_split_t* remsplit = gs_slot_array_getp(ctx->splits, fix_node->split);
                    remsplit->parent = ps->id;
                } break;
            } 

            root_split = gs_gui_get_root_split_from_split(ctx, ps);
        }
        // Otherwise, we were root dock and have to treat that case for remaining nodes
        else 
        { 
            switch (remain_node->type)
            {
                case GS_GUI_SPLIT_NODE_CONTAINER:
                {
                    gs_gui_container_t* remcnt = remain_node->container;    
                    remcnt->rect = split->rect;
                    remcnt->split = 0x00;
                    root_split = gs_gui_get_root_split(ctx, remcnt);
                } break;

                case GS_GUI_SPLIT_NODE_SPLIT:
                {
                    gs_gui_split_t* remsplit = gs_slot_array_getp(ctx->splits, remain_node->split);
                    remsplit->rect = split->rect;
                    remsplit->parent = 0x00;
                    root_split = gs_gui_get_root_split_from_split(ctx, remsplit);
                } break;
            }
        } 

        // Erase split
        gs_slot_array_erase(ctx->splits, split->id); 

        // Update
        if (root_split) gs_gui_update_split(ctx, root_split); 
        if (root_split) gs_gui_bring_split_to_front(ctx, root_split); 
        gs_gui_bring_to_front(ctx, cnt);
    } 
}

// ============================= //
// ========= Main API ========== //
// ============================= // 

GS_API_DECL void gs_gui_init(gs_gui_context_t *ctx, uint32_t window_hndl)
{
	memset(ctx, 0, sizeof(*ctx));
    ctx->text_height = gs_gui_text_height;
    ctx->text_width = gs_gui_text_width;
    ctx->font_height = gs_gui_font_height;
    ctx->text_dimensions = gs_gui_text_dimensions;
	ctx->draw_frame = gs_gui_draw_frame; 
    ctx->gsi = gs_immediate_draw_new(window_hndl); 
    ctx->overlay_draw_list = gs_immediate_draw_new(window_hndl);
	ctx->_style = gs_gui_default_style; 
    ctx->_style.font = gsi_default_font();
	ctx->style = &ctx->_style;
    ctx->window_hndl = window_hndl;
    ctx->last_zindex = 1000;
    gs_slot_array_reserve(ctx->splits, GS_GUI_GS_GUI_SPLIT_SIZE);
    gs_gui_split_t split = gs_default_val();
    gs_slot_array_insert(ctx->splits, split); // First item is set for 0x00 invalid
    gs_slot_array_reserve(ctx->tab_bars, GS_GUI_GS_GUI_TAB_SIZE);
    gs_gui_tab_bar_t tb = gs_default_val();
    gs_slot_array_insert(ctx->tab_bars, tb);
} 

static const char button_map[256] = {
  [GS_MOUSE_LBUTTON  & 0xff] =  GS_GUI_MOUSE_LEFT,
  [GS_MOUSE_RBUTTON  & 0xff] =  GS_GUI_MOUSE_RIGHT,
  [GS_MOUSE_MBUTTON  & 0xff] =  GS_GUI_MOUSE_MIDDLE
};

static const char key_map[512] = {
  [GS_KEYCODE_LEFT_SHIFT    & 0xff] = GS_GUI_KEY_SHIFT,
  [GS_KEYCODE_RIGHT_SHIFT   & 0xff] = GS_GUI_KEY_SHIFT,
  [GS_KEYCODE_LEFT_CONTROL  & 0xff] = GS_GUI_KEY_CTRL,
  [GS_KEYCODE_RIGHT_CONTROL & 0xff] = GS_GUI_KEY_CTRL,
  [GS_KEYCODE_LEFT_ALT      & 0xff] = GS_GUI_KEY_ALT,
  [GS_KEYCODE_RIGHT_ALT     & 0xff] = GS_GUI_KEY_ALT,
  [GS_KEYCODE_ENTER         & 0xff] = GS_GUI_KEY_RETURN,
  [GS_KEYCODE_BACKSPACE     & 0xff] = GS_GUI_KEY_BACKSPACE
};

static void gs_gui_draw_splits(gs_gui_context_t* ctx, gs_gui_split_t* split)
{
    if (!split) return;
    
    gs_gui_split_t* root_split = gs_gui_get_root_split_from_split(ctx, split);

    // Draw split
    const gs_gui_rect_t* sr = &split->rect;
    gs_vec2 hd = gs_v2(sr->w * 0.5f, sr->h * 0.5f);
    gs_gui_rect_t r = gs_default_val();
    gs_color_t c = gs_color(0, 0, 0, 0);
    const float ratio = split->ratio;
    gs_gui_container_t* top = gs_gui_get_top_most_container(ctx, root_split); 
    gs_gui_container_t* hover_cnt = ctx->hover ? gs_gui_get_container_ex(ctx, ctx->hover, 0x00) : ctx->next_hover_root ? gs_gui_get_container_ex(ctx, ctx->next_hover_root, 0x00) : NULL;
    bool valid_hover = hover_cnt && hover_cnt->zindex > top->zindex;
    valid_hover = false;

    bool can_draw = true;
    for (uint32_t i = 0; i < 2; ++i)
    {
        if (
            split->children[i].type == GS_GUI_SPLIT_NODE_CONTAINER && can_draw
        )
        { 
            gs_gui_container_t* cnt = split->children[i].container;

            // Don't draw split if this container belongs to a dockspace
            if (cnt->opt & GS_GUI_OPT_DOCKSPACE)
            {
                can_draw = false;
                continue;
            }

            switch (split->type)
            {
                case GS_GUI_SPLIT_LEFT: 
                {
                    r = gs_gui_rect(sr->x + sr->w * ratio - GS_GUI_SPLIT_SIZE * 0.5f, sr->y + GS_GUI_SPLIT_SIZE, GS_GUI_SPLIT_SIZE, sr->h - 2.f * GS_GUI_SPLIT_SIZE);
                } break;

                case GS_GUI_SPLIT_RIGHT: 
                {
                    r = gs_gui_rect(sr->x + sr->w * (1.f - ratio) - GS_GUI_SPLIT_SIZE * 0.5f, sr->y + GS_GUI_SPLIT_SIZE, GS_GUI_SPLIT_SIZE, sr->h - 2.f * GS_GUI_SPLIT_SIZE);
                } break;

                case GS_GUI_SPLIT_TOP:
                {
                    r = gs_gui_rect(sr->x + GS_GUI_SPLIT_SIZE, sr->y + sr->h * (ratio) - GS_GUI_SPLIT_SIZE * 0.5f, sr->w - 2.f * GS_GUI_SPLIT_SIZE, GS_GUI_SPLIT_SIZE);
                } break;

                case GS_GUI_SPLIT_BOTTOM:
                {
                    r = gs_gui_rect(sr->x + GS_GUI_SPLIT_SIZE, sr->y + sr->h * (1.f - ratio) - GS_GUI_SPLIT_SIZE * 0.5f, sr->w - 2.f * GS_GUI_SPLIT_SIZE, GS_GUI_SPLIT_SIZE);
                } break;
            }

            gs_gui_rect_t expand = gs_gui_expand_rect(r, 1); 
            bool hover = !valid_hover && !ctx->focus && !ctx->prev_dockable_root && gs_gui_rect_overlaps_vec2(expand, ctx->mouse_pos) && !ctx->lock_hover_id; 
            if (hover) ctx->next_hover_split = split;
            if (hover && ctx->mouse_down == GS_GUI_MOUSE_LEFT)
            {
                if (!ctx->focus_split) ctx->focus_split = split;
            } 
            bool active = ctx->focus_split == split;
            if (active)
            {
                ctx->next_hover_root = top;
                gs_gui_request_t req = gs_default_val();
                req.type = GS_GUI_SPLIT_RATIO;
                req.split = split;
                gs_dyn_array_push(ctx->requests, req);
            }
            if (
                (hover && (ctx->focus_split == split)) || 
                (hover && !ctx->focus_split) || 
                active && can_draw
            )
            {
                gs_gui_draw_rect(ctx, r, ctx->focus_split == split ? ctx->style->colors[GS_GUI_COLOR_BUTTONFOCUS] : ctx->style->colors[GS_GUI_COLOR_BUTTONHOVER]);
                can_draw = false;
            }
        } 
        else if (
                split->children[i].type == GS_GUI_SPLIT_NODE_SPLIT     
        )
        { 
            if (can_draw)
            {
                switch (split->type)
                {
                    case GS_GUI_SPLIT_LEFT: 
                    {
                        r = gs_gui_rect(sr->x + sr->w * ratio - GS_GUI_SPLIT_SIZE * 0.5f, sr->y + GS_GUI_SPLIT_SIZE, GS_GUI_SPLIT_SIZE, sr->h - 2.f * GS_GUI_SPLIT_SIZE);
                    } break;

                    case GS_GUI_SPLIT_RIGHT: 
                    {
                        r = gs_gui_rect(sr->x + sr->w * (1.f - ratio) - GS_GUI_SPLIT_SIZE * 0.5f, sr->y + GS_GUI_SPLIT_SIZE, GS_GUI_SPLIT_SIZE, sr->h - 2.f * GS_GUI_SPLIT_SIZE);
                    } break;

                    case GS_GUI_SPLIT_TOP:
                    {
                        r = gs_gui_rect(sr->x + GS_GUI_SPLIT_SIZE, sr->y + sr->h * (ratio) - GS_GUI_SPLIT_SIZE * 0.5f, sr->w - 2.f * GS_GUI_SPLIT_SIZE, GS_GUI_SPLIT_SIZE);
                    } break;

                    case GS_GUI_SPLIT_BOTTOM:
                    {
                        r = gs_gui_rect(sr->x + GS_GUI_SPLIT_SIZE, sr->y + sr->h * (1.f - ratio) - GS_GUI_SPLIT_SIZE * 0.5f, sr->w - 2.f * GS_GUI_SPLIT_SIZE, GS_GUI_SPLIT_SIZE);
                    } break;
                } 

                gs_gui_rect_t expand = gs_gui_expand_rect(r, 1); 
                bool hover = !valid_hover && !ctx->focus && !ctx->prev_dockable_root && gs_gui_rect_overlaps_vec2(expand, ctx->mouse_pos); 
                if (hover) ctx->next_hover_split = split;
                if (hover && ctx->mouse_down == GS_GUI_MOUSE_LEFT)
                {
                    if (!ctx->focus_split) ctx->focus_split = split;
                } 
                bool active = ctx->focus_split == split;
                if (active)
                {
                    ctx->next_hover_root = top;
                    gs_gui_request_t req = gs_default_val();
                    req.type = GS_GUI_SPLIT_RATIO;
                    req.split = split;
                    gs_dyn_array_push(ctx->requests, req);
                }
                if (
                    (hover && (ctx->focus_split == split)) || 
                    (hover && !ctx->focus_split) || 
                    active
                )
                {
                    gs_gui_draw_rect(ctx, r, ctx->focus_split == split ? ctx->style->colors[GS_GUI_COLOR_BUTTONFOCUS] : ctx->style->colors[GS_GUI_COLOR_BUTTONHOVER]);
                    can_draw = false;
                }
            }

            gs_gui_split_t* child = gs_slot_array_getp(ctx->splits, split->children[i].split);
            gs_gui_draw_splits(ctx, child);
        } 
    } 
    if (ctx->focus_split == split && ctx->mouse_down != GS_GUI_MOUSE_LEFT)
    {
        ctx->focus_split = NULL;
    }
}

static void gs_gui_get_split_lowest_zindex(gs_gui_context_t* ctx, gs_gui_split_t* split, int32_t* index)
{
    if (!split) return; 

    if (split->children[0].type == GS_GUI_SPLIT_NODE_CONTAINER && split->children[0].container->zindex < *index)
    {
        *index = split->children[0].container->zindex; 
    }
    if (split->children[0].type == GS_GUI_SPLIT_NODE_CONTAINER && split->children[0].container->tab_bar)
    {
        gs_gui_tab_bar_t* tab_bar = gs_slot_array_getp(ctx->tab_bars, split->children[0].container->tab_bar); 
        for (uint32_t i = 0; i < tab_bar->size; ++i)
        {
            if (((gs_gui_container_t*)tab_bar->items[i].data)->zindex < *index) *index = ((gs_gui_container_t*)tab_bar->items[i].data)->zindex;
        }
    }

    if (split->children[1].type == GS_GUI_SPLIT_NODE_CONTAINER && split->children[1].container->zindex < *index)
    {
        *index = split->children[1].container->zindex;
    }
    if (split->children[1].type == GS_GUI_SPLIT_NODE_CONTAINER && split->children[1].container->tab_bar)
    {
        gs_gui_tab_bar_t* tab_bar = gs_slot_array_getp(ctx->tab_bars, split->children[1].container->tab_bar); 
        for (uint32_t i = 0; i < tab_bar->size; ++i)
        {
            if (((gs_gui_container_t*)tab_bar->items[i].data)->zindex < *index) *index = ((gs_gui_container_t*)tab_bar->items[i].data)->zindex;
        }
    }

    if (split->children[0].type == GS_GUI_SPLIT_NODE_SPLIT)
    {
        gs_gui_get_split_lowest_zindex(ctx, gs_slot_array_getp(ctx->splits, split->children[0].split), index);
    }

    if (split->children[1].type == GS_GUI_SPLIT_NODE_SPLIT)
    {
        gs_gui_get_split_lowest_zindex(ctx, gs_slot_array_getp(ctx->splits, split->children[1].split), index);
    }
}

GS_API_DECL void gs_gui_begin(gs_gui_context_t* ctx) 
{
    // Capture event information
    gs_vec2 mouse_pos = gs_platform_mouse_positionv(); 
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
                        ctx->mouse_pos = evt.mouse.move;
                    } break;

                    case GS_PLATFORM_MOUSE_WHEEL:
                    {
                        gs_gui_input_scroll(ctx, 0, -(int32_t)evt.mouse.wheel.y * 30.f);
                    } break;

                    case GS_PLATFORM_MOUSE_BUTTON_DOWN:
                    case GS_PLATFORM_MOUSE_BUTTON_PRESSED:
                    {
                        int32_t code = 1 << evt.mouse.button;
                        gs_gui_input_mousedown(ctx, (int32_t)mouse_pos.x, (int32_t)mouse_pos.y, code);
                    } break;

                    case GS_PLATFORM_MOUSE_BUTTON_RELEASED:
                    {
                        int32_t code = 1 << evt.mouse.button;
                        gs_gui_input_mouseup(ctx, (int32_t)mouse_pos.x, (int32_t)mouse_pos.y, code);
                    } break; 

                    case GS_PLATFORM_MOUSE_ENTER:
                    {
                        // If there are user callbacks, could trigger them here
                    } break;

                    case GS_PLATFORM_MOUSE_LEAVE:
                    {
                        // If there are user callbacks, could trigger them here
                    } break;

                    default: break; 
                }

            } break;

            case GS_PLATFORM_EVENT_TEXT:
            {
                // Input text
                char txt[2] = {(char)(evt.text.codepoint & 255), 0};
                gs_gui_input_text(ctx, txt);
            } break;

            case GS_PLATFORM_EVENT_KEY:
            {
                switch (evt.key.action)
                { 
                    case GS_PLATFORM_KEY_DOWN:
                    case GS_PLATFORM_KEY_PRESSED:
                    { 
                        gs_gui_input_keydown(ctx, key_map[evt.key.keycode & 511]); 
                    } break; 

                    case GS_PLATFORM_KEY_RELEASED:
                    {
                        gs_gui_input_keyup(ctx, key_map[evt.key.keycode & 511]);
                    } break;

                    default: break;
                }

            } break;

            case GS_PLATFORM_EVENT_WINDOW:
            {
                switch (evt.window.action)
                {
                    default: break;
                }

            } break;


            default: break;
        }
    }

	ctx->command_list.idx = 0;
	ctx->root_list.idx = 0;
	ctx->scroll_target = NULL;
	ctx->hover_root = ctx->next_hover_root;
	ctx->next_hover_root = NULL;
    ctx->focus_root = ctx->next_focus_root;
    ctx->next_focus_root = NULL;
    ctx->prev_dockable_root = ctx->dockable_root;
    ctx->dockable_root = NULL;
    ctx->hover_split = ctx->next_hover_split;
    ctx->next_hover_split = NULL;
    ctx->lock_hover_id = ctx->next_lock_hover_id;
    ctx->next_lock_hover_id = 0x00;
	ctx->mouse_delta.x = ctx->mouse_pos.x - ctx->last_mouse_pos.x;
	ctx->mouse_delta.y = ctx->mouse_pos.y - ctx->last_mouse_pos.y;
	ctx->frame++; 
    ctx->framebuffer = gs_platform_framebuffer_sizev(ctx->window_hndl); 

    // Set up overlay draw list
    gsi_camera2D(&ctx->overlay_draw_list);
    gsi_defaults(&ctx->overlay_draw_list);

    for (
        gs_slot_array_iter it = gs_slot_array_iter_new(ctx->splits);
        gs_slot_array_iter_valid(ctx->splits, it);
        gs_slot_array_iter_advance(ctx->splits, it)
    )
    {
        if (!it) continue;

        gs_gui_split_t* split = gs_slot_array_iter_getp(ctx->splits, it);

        // Root split
        if (!split->parent)
        { 
            gs_gui_container_t* root_cnt = gs_gui_get_root_container_from_split(ctx, split); 
            gs_gui_rect_t r = split->rect;
            r.x -= 10.f; 
            r.w += 20.f;
            r.y -= 10.f;
            r.h += 20.f;
            gs_snprintfc(TMP, 256, "!dockspace%zu", (size_t)split);
            int32_t opt = GS_GUI_OPT_NOFRAME | GS_GUI_OPT_FORCESETRECT | GS_GUI_OPT_NOMOVE | GS_GUI_OPT_NOTITLE | GS_GUI_OPT_NOSCROLL | GS_GUI_OPT_NOCLIP | GS_GUI_OPT_NODOCK | GS_GUI_OPT_DOCKSPACE;
            gs_gui_begin_window_ex(ctx, TMP, r, opt); 
            {
                // Set zindex for sorting (always below the bottom most window in this split tree)
                gs_gui_container_t* ds = gs_gui_get_current_container(ctx);
                int32_t zindex = INT32_MAX - 1;
                gs_gui_get_split_lowest_zindex(ctx, split, &zindex);
                if (root_cnt->opt & GS_GUI_OPT_DOCKSPACE) ds->zindex = 0;
                else ds->zindex = gs_clamp((int32_t)zindex - 1, 0, INT32_MAX);

                gs_gui_rect_t fr = split->rect;
                fr.x += GS_GUI_SPLIT_SIZE; fr.y += GS_GUI_SPLIT_SIZE; fr.w -= 2.f * GS_GUI_SPLIT_SIZE; fr.h -= 2.f * GS_GUI_SPLIT_SIZE;
		        ctx->draw_frame(ctx, fr, GS_GUI_COLOR_WINDOWBG);

                // Draw splits
                gs_gui_draw_splits(ctx, split);

                // Do resize controls for dockspace
                gs_gui_container_t* top = gs_gui_get_top_most_container(ctx, split);
                const gs_gui_rect_t* sr = &split->rect;
                gs_gui_container_t* hover_cnt = ctx->hover ? gs_gui_get_container_ex(ctx, ctx->hover, 0x00) : ctx->next_hover_root ? gs_gui_get_container_ex(ctx, ctx->next_hover_root, 0x00) : NULL;
                bool valid_hover = hover_cnt && hover_cnt->zindex > top->zindex;

                // W
                {
                    // Cache rect
                    gs_gui_rect_t lr = gs_gui_rect(fr.x - 2.f * GS_GUI_SPLIT_SIZE, fr.y, GS_GUI_SPLIT_SIZE, fr.h); 
                    gs_gui_rect_t ex = lr; 
                    ex.x -= 10.f; ex.w += 20.f;
                    gs_gui_id id = gs_gui_get_id(ctx, "!hov_l", 6);
                    gs_gui_update_control(ctx, id, ex, opt); 

                    if (id == ctx->focus && ctx->mouse_down == GS_GUI_MOUSE_LEFT) 
                    {
                        gs_gui_draw_control_frame(ctx, id, lr, GS_GUI_COLOR_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        gs_gui_request_t req = gs_default_val();
                        req.type = GS_GUI_SPLIT_RESIZE_W;
                        req.split = split;
                        gs_dyn_array_push(ctx->requests, req);
                    } 
                }
                
                // E
                {
                    // Cache rect
                    gs_gui_rect_t rr = gs_gui_rect(fr.x + fr.w + GS_GUI_SPLIT_SIZE, fr.y, GS_GUI_SPLIT_SIZE, fr.h); 
                    gs_gui_rect_t ex = rr; 
                    ex.w += 20.f; 
                    gs_gui_id id = gs_gui_get_id(ctx, "!hov_r", 6);
                    gs_gui_update_control(ctx, id, ex, opt); 

                    if (id == ctx->focus && ctx->mouse_down == GS_GUI_MOUSE_LEFT) 
                    {
                        gs_gui_draw_control_frame(ctx, id, rr, GS_GUI_COLOR_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        gs_gui_request_t req = gs_default_val();
                        req.type = GS_GUI_SPLIT_RESIZE_E;
                        req.split = split;
                        gs_dyn_array_push(ctx->requests, req);
                    } 
                }

                // N
                {
                    // Cache rect
                    gs_gui_rect_t tr = gs_gui_rect(fr.x, fr.y - 2.f * GS_GUI_SPLIT_SIZE, fr.w, GS_GUI_SPLIT_SIZE); 
                    gs_gui_rect_t ex = tr; 
                    ex.y -= 10.f; 
                    ex.h += 20.f; 
                    gs_gui_id id = gs_gui_get_id(ctx, "!hov_t", 6);
                    gs_gui_update_control(ctx, id, ex, opt); 

                    if (id == ctx->focus && ctx->mouse_down == GS_GUI_MOUSE_LEFT) 
                    {
                        gs_gui_draw_control_frame(ctx, id, tr, GS_GUI_COLOR_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        gs_gui_request_t req = gs_default_val();
                        req.type = GS_GUI_SPLIT_RESIZE_N;
                        req.split = split;
                        gs_dyn_array_push(ctx->requests, req);
                    } 
                }

                // S
                {
                    // Cache rect
                    gs_gui_rect_t br = gs_gui_rect(fr.x, fr.y + fr.h + GS_GUI_SPLIT_SIZE, fr.w, GS_GUI_SPLIT_SIZE); 
                    gs_gui_rect_t ex = br; 
                    ex.h += 20.f; 
                    gs_gui_id id = gs_gui_get_id(ctx, "!hov_b", 6);
                    gs_gui_update_control(ctx, id, ex, opt); 

                    if (id == ctx->focus && ctx->mouse_down == GS_GUI_MOUSE_LEFT) 
                    {
                        gs_gui_draw_control_frame(ctx, id, br, GS_GUI_COLOR_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        gs_gui_request_t req = gs_default_val();
                        req.type = GS_GUI_SPLIT_RESIZE_S;
                        req.split = split;
                        gs_dyn_array_push(ctx->requests, req);
                    } 
                }
            }
            gs_gui_end_window(ctx);
        } 
    }

    if (ctx->mouse_down != GS_GUI_MOUSE_LEFT)
    {
        ctx->lock_focus = 0x00;
    }
} 

static void gs_gui_docking(gs_gui_context_t* ctx)
{ 
    if (ctx->undock_root)
    {
        gs_gui_undock_ex_cnt(ctx, ctx->undock_root);
    }

	if (!ctx->focus_root || ctx->focus_root->opt & GS_GUI_OPT_NODOCK) return; 

    if (ctx->dockable_root || ctx->prev_dockable_root)
    { 
        gs_gui_container_t* cnt = ctx->dockable_root ? ctx->dockable_root : ctx->prev_dockable_root; 

        if ( ctx->prev_dockable_root && !ctx->dockable_root && ctx->mouse_down != GS_GUI_MOUSE_LEFT )
        {
            int32_t b = 0;
        }

        // Cache hoverable tile information
        gs_vec2 c = gs_v2(cnt->rect.x + cnt->rect.w / 2.f, cnt->rect.y + cnt->rect.h / 2.f);

        const float sz = gs_clamp(gs_min(cnt->rect.w * 0.1f, cnt->rect.h * 0.1f), 15.f, 25.f);
        const float off = sz + sz * 0.2f;
        gs_color_t def_col = gs_color_alpha(ctx->style->colors[GS_GUI_COLOR_BUTTONFOCUS], 100); // gs_color(255, 0, 0, 100);
        gs_color_t hov_col = gs_color_alpha(ctx->style->colors[GS_GUI_COLOR_BUTTONFOCUS], 150); // gs_color(255, 0, 0, 200);

        gs_gui_rect_t center = gs_gui_rect(c.x, c.y, sz, sz);
        gs_gui_rect_t left   = gs_gui_rect(c.x - off, c.y, sz, sz);
        gs_gui_rect_t right  = gs_gui_rect(c.x + off, c.y, sz, sz); 
        gs_gui_rect_t top    = gs_gui_rect(c.x, c.y - off, sz, sz); 
        gs_gui_rect_t bottom = gs_gui_rect(c.x, c.y + off, sz, sz); 

        int32_t hov_c = (gs_gui_rect_overlaps_vec2(center, ctx->mouse_pos)); 
        int32_t hov_l = gs_gui_rect_overlaps_vec2(left, ctx->mouse_pos); 
        int32_t hov_r = gs_gui_rect_overlaps_vec2(right, ctx->mouse_pos); 
        int32_t hov_t = gs_gui_rect_overlaps_vec2(top, ctx->mouse_pos); 
        int32_t hov_b = gs_gui_rect_overlaps_vec2(bottom, ctx->mouse_pos); 
        int32_t hov_w = gs_gui_rect_overlaps_vec2(cnt->rect, ctx->mouse_pos); 

        bool can_dock = true;

        // Can't dock one dockspace into another
        if (ctx->focus_root->opt & GS_GUI_OPT_DOCKSPACE)
        {
            can_dock = false;
        } 

        if (ctx->focus_root->tab_bar)
        {
			gs_gui_container_t* tcmp = ctx->dockable_root ? ctx->dockable_root : ctx->prev_dockable_root;
            gs_gui_tab_bar_t* tab_bar = gs_slot_array_getp(ctx->tab_bars, ctx->focus_root->tab_bar);
            for (uint32_t i = 0; i < tab_bar->size; ++i)
            {
				gs_gui_container_t* tcnt = (gs_gui_container_t*)tab_bar->items[i].data;
                if (tcnt == tcmp)
                {
                    can_dock = false;
                }
            }
        }

        // Need to make sure you CAN dock here first
        if (ctx->dockable_root && can_dock)
        { 
            // Need to now grab overlay draw list, then draw rects into it
            gs_immediate_draw_t* dl = &ctx->overlay_draw_list; 

            bool is_dockspace = ctx->dockable_root->opt & GS_GUI_OPT_DOCKSPACE;

            // Draw center rect
            gsi_rectvd(dl, gs_v2(center.x, center.y), gs_v2(center.w, center.h), gs_v2s(0.f), gs_v2s(1.f), hov_c ? hov_col : def_col, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
            // gsi_rectvd(dl, gs_v2(center.x, center.y), gs_v2(center.w + 1, center.h), gs_v2s(0.f), gs_v2s(1.f), hov_col, GS_GRAPHICS_PRIMITIVE_LINES);

            if (!is_dockspace)
            {
                gsi_rectvd(dl, gs_v2(left.x, left.y), gs_v2(left.w, left.h), gs_v2s(0.f), gs_v2s(1.f), hov_l ? hov_col : def_col, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
                // gsi_rectvd(dl, gs_v2(left.x, left.y), gs_v2(left.w, left.h), gs_v2s(0.f), gs_v2s(1.f), hov_col, GS_GRAPHICS_PRIMITIVE_LINES);

                gsi_rectvd(dl, gs_v2(right.x, right.y), gs_v2(right.w, right.h), gs_v2s(0.f), gs_v2s(1.f), hov_r ? hov_col : def_col, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
                // gsi_rectvd(dl, gs_v2(right.x, right.y), gs_v2(right.w, right.h), gs_v2s(0.f), gs_v2s(1.f), hov_col, GS_GRAPHICS_PRIMITIVE_LINES);

                gsi_rectvd(dl, gs_v2(top.x, top.y), gs_v2(top.w, top.h), gs_v2s(0.f), gs_v2s(1.f), hov_t ? hov_col : def_col, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
                // gsi_rectvd(dl, gs_v2(top.x, top.y), gs_v2(top.w, top.h), gs_v2s(0.f), gs_v2s(1.f), hov_col, GS_GRAPHICS_PRIMITIVE_LINES);

                gsi_rectvd(dl, gs_v2(bottom.x, bottom.y), gs_v2(bottom.w, bottom.h), gs_v2s(0.f), gs_v2s(1.f), hov_b ? hov_col : def_col, GS_GRAPHICS_PRIMITIVE_TRIANGLES); 
                // gsi_rectvd(dl, gs_v2(bottom.x, bottom.y), gs_v2(bottom.w, bottom.h), gs_v2s(0.f), gs_v2s(1.f), hov_col, GS_GRAPHICS_PRIMITIVE_LINES); 
            }

            const float d = 0.5f;
            const float hs = sz * 0.5f; 

            if (is_dockspace)
            {
                if (hov_c)
                {
                    center = gs_gui_rect(cnt->rect.x, cnt->rect.y, cnt->rect.w, cnt->rect.h);
                    gsi_rectvd(dl, gs_v2(center.x, center.y), gs_v2(center.w, center.h), gs_v2s(0.f), gs_v2s(1.f), hov_col, GS_GRAPHICS_PRIMITIVE_LINES);
                    gsi_rectvd(dl, gs_v2(center.x, center.y), gs_v2(center.w, center.h), gs_v2s(0.f), gs_v2s(1.f), def_col, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
                }
            }
            else
            {
                if (hov_c && !ctx->focus_root->split)
                {
                    center = gs_gui_rect(cnt->rect.x, cnt->rect.y, cnt->rect.w, cnt->rect.h);
                    gsi_rectvd(dl, gs_v2(center.x, center.y), gs_v2(center.w, center.h), gs_v2s(0.f), gs_v2s(1.f), hov_col, GS_GRAPHICS_PRIMITIVE_LINES);
                    gsi_rectvd(dl, gs_v2(center.x, center.y), gs_v2(center.w, center.h), gs_v2s(0.f), gs_v2s(1.f), def_col, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
                } 
                else if (hov_l)
                {
                    left = gs_gui_rect(cnt->rect.x, cnt->rect.y, cnt->rect.w * d + hs, cnt->rect.h);
                    gsi_rectvd(dl, gs_v2(left.x, left.y), gs_v2(left.w, left.h), gs_v2s(0.f), gs_v2s(1.f), hov_col, GS_GRAPHICS_PRIMITIVE_LINES);
                    gsi_rectvd(dl, gs_v2(left.x, left.y), gs_v2(left.w, left.h), gs_v2s(0.f), gs_v2s(1.f), def_col, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
                } 
                else if (hov_r)
                {
                    right = gs_gui_rect(cnt->rect.x + cnt->rect.w * d + hs, cnt->rect.y, cnt->rect.w * (1.f - d) - hs, cnt->rect.h);
                    gsi_rectvd(dl, gs_v2(right.x, right.y), gs_v2(right.w, right.h), gs_v2s(0.f), gs_v2s(1.f), hov_col, GS_GRAPHICS_PRIMITIVE_LINES);
                    gsi_rectvd(dl, gs_v2(right.x, right.y), gs_v2(right.w, right.h), gs_v2s(0.f), gs_v2s(1.f), def_col, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
                } 
                else if (hov_b)
                {
                    bottom = gs_gui_rect(cnt->rect.x, cnt->rect.y + cnt->rect.h * d + hs, cnt->rect.w, cnt->rect.h * (1.f - d) - hs);
                    gsi_rectvd(dl, gs_v2(bottom.x, bottom.y), gs_v2(bottom.w, bottom.h), gs_v2s(0.f), gs_v2s(1.f), hov_col, GS_GRAPHICS_PRIMITIVE_LINES);
                    gsi_rectvd(dl, gs_v2(bottom.x, bottom.y), gs_v2(bottom.w, bottom.h), gs_v2s(0.f), gs_v2s(1.f), def_col, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
                }
                else if (hov_t)
                {
                    top = gs_gui_rect(cnt->rect.x, cnt->rect.y, cnt->rect.w, cnt->rect.h * d + hs);
                    gsi_rectvd(dl, gs_v2(top.x, top.y), gs_v2(top.w, top.h), gs_v2s(0.f), gs_v2s(1.f), hov_col, GS_GRAPHICS_PRIMITIVE_LINES);
                    gsi_rectvd(dl, gs_v2(top.x, top.y), gs_v2(top.w, top.h), gs_v2s(0.f), gs_v2s(1.f), def_col, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
                }
            } 
        }

        // Handle docking
        if (ctx->prev_dockable_root && !ctx->dockable_root && ctx->mouse_down != GS_GUI_MOUSE_LEFT) 
        { 
            gs_gui_container_t* parent = ctx->prev_dockable_root;
            gs_gui_container_t* child = ctx->focus_root; 

            bool is_dockspace = ctx->prev_dockable_root->opt & GS_GUI_OPT_DOCKSPACE;

            if (is_dockspace)
            {
                if (hov_c) gs_gui_dock_ex_cnt(ctx, child, parent, GS_GUI_SPLIT_TOP, 1.0f);
            }
            else
            {
                if (hov_c && !ctx->focus_root->split)   gs_gui_dock_ex_cnt(ctx, child, parent, GS_GUI_SPLIT_TAB, 0.5f);
                else if (hov_l)                         gs_gui_dock_ex_cnt(ctx, child, parent, GS_GUI_SPLIT_LEFT, 0.5f);
                else if (hov_r)                         gs_gui_dock_ex_cnt(ctx, child, parent, GS_GUI_SPLIT_RIGHT, 0.5f);
                else if (hov_t)                         gs_gui_dock_ex_cnt(ctx, child, parent, GS_GUI_SPLIT_TOP, 0.5f);
                else if (hov_b)                         gs_gui_dock_ex_cnt(ctx, child, parent, GS_GUI_SPLIT_BOTTOM, 0.5f);
            } 
        } 
    } 
}

GS_API_DECL void gs_gui_end(gs_gui_context_t *ctx) 
{
	int32_t i, n; 

    // Check for docking, draw overlays
    gs_gui_docking(ctx);

    for (uint32_t r = 0; r < gs_dyn_array_size(ctx->requests); ++r)
    { 
        const gs_gui_request_t* req = &ctx->requests[r];

        // If split moved, update position for next frame
        switch (req->type)
        {
            case GS_GUI_CNT_MOVE:
            {
                if (req->cnt)
                {
                    req->cnt->rect.x += ctx->mouse_delta.x;
                    req->cnt->rect.y += ctx->mouse_delta.y;

                    if (req->cnt->tab_bar)
                    {
                        gs_gui_tab_bar_t* tb = gs_slot_array_getp(ctx->tab_bars, req->cnt->tab_bar);
                        gs_assert(tb);
                        tb->rect.x += ctx->mouse_delta.x;
                        tb->rect.y += ctx->mouse_delta.y;
                    }
                } 
            } break;

            case GS_GUI_CNT_FOCUS:
            {
                if (!req->cnt) break;

                gs_gui_container_t* cnt = (gs_gui_container_t*)req->cnt;
                gs_assert(cnt);

                gs_gui_split_t* rs = gs_gui_get_root_split(ctx, cnt);

                if (cnt->tab_bar)
                {
                    if (rs)
                    {
                        gs_gui_bring_split_to_front(ctx, rs);
                    }

                    gs_gui_tab_bar_t* tab_bar = gs_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
                    gs_gui_tab_item_t* tab_item = &tab_bar->items[cnt->tab_item];
                    gs_gui_container_t* fcnt = (gs_gui_container_t*)tab_bar->items[tab_bar->focus].data;
                    fcnt->opt |= GS_GUI_OPT_NOHOVER;
                    fcnt->opt |= GS_GUI_OPT_NOINTERACT; 
                    fcnt->flags &= ~GS_GUI_WINDOW_FLAGS_VISIBLE;
					tab_bar->focus = tab_item->idx;
                    cnt->flags |= GS_GUI_WINDOW_FLAGS_VISIBLE;

                    // Bring all tab items to front
                    for (uint32_t i = 0; i < tab_bar->size; ++i)
                    {
                        gs_gui_bring_to_front(ctx, tab_bar->items[i].data); 
                    }
                    gs_gui_bring_to_front(ctx, cnt); 
                } 

            } break;

            case GS_GUI_SPLIT_MOVE:
            {
                if (req->split)
                {
                    req->split->rect.x += ctx->mouse_delta.x;
                    req->split->rect.y += ctx->mouse_delta.y; 
                    gs_gui_update_split(ctx, req->split);
                }

            } break;

            case GS_GUI_SPLIT_RESIZE_SE: 
            {
                if (req->split)
                {
                    gs_gui_rect_t* r = &req->split->rect;
                    r->w = gs_max(r->w + ctx->mouse_delta.x, 40);
                    r->h = gs_max(r->h + ctx->mouse_delta.y, 40);
                    gs_gui_update_split(ctx, req->split);
                }
            } break; 

            case GS_GUI_SPLIT_RESIZE_W: 
            {
                if (req->split)
                {
                    gs_gui_rect_t* r = &req->split->rect;
                    float w = r->w;
                    float max_x = r->x + r->w;
                    r->w = gs_max(r->w - ctx->mouse_delta.x, 40); 
                    if (fabsf(r->w - w) > 0.f)
                    {
                        r->x = gs_min(r->x + ctx->mouse_delta.x, max_x);
                    }
                    gs_gui_update_split(ctx, req->split);
                }
            } break; 

            case GS_GUI_SPLIT_RESIZE_E: 
            {
                if (req->split)
                {
                    gs_gui_rect_t* r = &req->split->rect;
                    r->w = gs_max(r->w + ctx->mouse_delta.x, 40); 
                    gs_gui_update_split(ctx, req->split);
                }
            } break; 

            case GS_GUI_SPLIT_RESIZE_N: 
            {
                if (req->split)
                {
                    gs_gui_rect_t* r = &req->split->rect;
                    float h = r->h;
                    float max_y = h + r->y;
                    r->h = gs_max(r->h - ctx->mouse_delta.y, 40); 
                    if (fabsf(r->h - h) > 0.f)
                    {
                        r->y = gs_min(r->y + ctx->mouse_delta.y, max_y);
                    }
                    gs_gui_update_split(ctx, req->split);
                }
            } break; 

            case GS_GUI_SPLIT_RESIZE_NE: 
            {
                if (req->split)
                {
                    gs_gui_rect_t* r = &req->split->rect; 
                    r->w = gs_max(r->w + ctx->mouse_delta.x, 40); 
                    float h = r->h;
                    float max_y = h + r->y;
                    r->h = gs_max(r->h - ctx->mouse_delta.y, 40); 
                    if (fabsf(r->h - h) > 0.f)
                    {
                        r->y = gs_min(r->y + ctx->mouse_delta.y, max_y);
                    }
                    gs_gui_update_split(ctx, req->split);
                }
            } break; 

            case GS_GUI_SPLIT_RESIZE_NW: 
            {
                if (req->split)
                {
                    gs_gui_rect_t* r = &req->split->rect; 
                    float h = r->h;
                    float max_y = h + r->y;
                    r->h = gs_max(r->h - ctx->mouse_delta.y, 40); 
                    if (fabsf(r->h - h) > 0.f)
                    {
                        r->y = gs_min(r->y + ctx->mouse_delta.y, max_y);
                    }

                    float w = r->w;
                    float max_x = r->x + r->w;
                    r->w = gs_max(r->w - ctx->mouse_delta.x, 40); 
                    if (fabsf(r->w - w) > 0.f)
                    {
                        r->x = gs_min(r->x + ctx->mouse_delta.x, max_x);
                    }
                    gs_gui_update_split(ctx, req->split);
                }
            } break; 

            case GS_GUI_SPLIT_RESIZE_S: 
            {
                if (req->split)
                {
                    gs_gui_rect_t* r = &req->split->rect;
                    r->h = gs_max(r->h + ctx->mouse_delta.y, 40); 
                    gs_gui_update_split(ctx, req->split);
                }
            } break; 

            case GS_GUI_SPLIT_RESIZE_SW: 
            {
                if (req->split)
                {
                    gs_gui_rect_t* r = &req->split->rect;
                    float h = r->h;
                    float max_y = h + r->y;
                    r->h = gs_max(r->h + ctx->mouse_delta.y, 40); 

                    float w = r->w;
                    float max_x = r->x + r->w;
                    r->w = gs_max(r->w - ctx->mouse_delta.x, 40); 
                    if (fabsf(r->w - w) > 0.f)
                    {
                        r->x = gs_min(r->x + ctx->mouse_delta.x, max_x);
                    }
                    gs_gui_update_split(ctx, req->split);
                }
            } break; 


            case GS_GUI_SPLIT_RATIO:
            {
                const float smin = 0.05f; 
                const float smax = 1.f - smin; 
                gs_gui_split_t* split = req->split;

                switch (split->type)
                {
                    case GS_GUI_SPLIT_LEFT:
                    {
                        split->ratio = gs_clamp(split->ratio + ctx->mouse_delta.x / split->rect.w, smin, smax);
                        gs_gui_update_split(ctx, split);
                    } break;

                    case GS_GUI_SPLIT_RIGHT:
                    {
                        split->ratio = gs_clamp(split->ratio - ctx->mouse_delta.x / split->rect.w, smin, smax);
                        gs_gui_update_split(ctx, split);
                    } break; 

                    case GS_GUI_SPLIT_TOP:
                    {
                        split->ratio = gs_clamp(split->ratio + ctx->mouse_delta.y / split->rect.h, smin, smax);
                        gs_gui_update_split(ctx, split);
                    } break; 

                    case GS_GUI_SPLIT_BOTTOM:
                    {
                        split->ratio = gs_clamp(split->ratio - ctx->mouse_delta.y / split->rect.h, smin, smax);
                        gs_gui_update_split(ctx, split);
                    } break; 
                }

                // Bring to font
                gs_gui_bring_split_to_front(ctx, gs_gui_get_root_split_from_split(ctx, split));

            } break;

			case GS_GUI_TAB_SWAP_LEFT:
			{
				gs_gui_tab_item_swap(ctx, req->cnt, -1);
			} break;
			
			case GS_GUI_TAB_SWAP_RIGHT:
			{
				gs_gui_tab_item_swap(ctx, req->cnt, +1);
			} break;
        } 
    }

    // Clear reqests
    gs_dyn_array_clear(ctx->requests);

	// Check stacks	
    gs_gui_expect(ctx->container_stack.idx == 0);
	gs_gui_expect(ctx->clip_stack.idx == 0);
	gs_gui_expect(ctx->id_stack.idx == 0);
	gs_gui_expect(ctx->layout_stack.idx	== 0);

	// Handle scroll input
	if (ctx->scroll_target) 
    {
		ctx->scroll_target->scroll.x += ctx->scroll_delta.x;
		ctx->scroll_target->scroll.y += ctx->scroll_delta.y;
	}

	// Unset focus if focus id was not touched this frame
	if (!ctx->updated_focus) { ctx->focus = 0; }
	ctx->updated_focus = 0;

	// Bring hover root to front if mouse was pressed 
	if (ctx->mouse_pressed && ctx->next_hover_root &&
			ctx->next_hover_root->zindex < ctx->last_zindex &&
			ctx->next_hover_root->zindex >= 0
	) 
    {
        // Root split
        gs_gui_split_t* split = gs_gui_get_root_split(ctx, ctx->next_hover_root);

        // Need to bring entire dockspace to front
        if (split)
        {
            gs_gui_bring_split_to_front(ctx, split);
        } 
        else if (~ctx->next_hover_root->opt & GS_GUI_OPT_NOFOCUS)
        {
            gs_gui_bring_to_front(ctx, ctx->next_hover_root);
        } 
	} 

	// Reset state
	ctx->key_pressed = 0;
	ctx->input_text[0] = '\0';
	ctx->mouse_pressed = 0;
	ctx->scroll_delta = gs_v2(0, 0);
	ctx->last_mouse_pos = ctx->mouse_pos; 
    ctx->undock_root = NULL;

    if (ctx->mouse_down != GS_GUI_MOUSE_LEFT)
    {
        gs_platform_set_cursor(ctx->window_hndl, GS_PLATFORM_CURSOR_ARROW);
    }

	// Sort root containers by zindex 
	n = ctx->root_list.idx;
	qsort(ctx->root_list.items, n, sizeof(gs_gui_container_t*), gs_gui_compare_zindex);

	// Set root container jump commands
	for (i = 0; i < n; i++) 
    {
		gs_gui_container_t *cnt = ctx->root_list.items[i];

		// If this is the first container then make the first command jump to it.  
		// otherwise set the previous container's tail to jump to this one 
		if (i == 0) 
        {
			gs_gui_command_t *cmd = (gs_gui_command_t*) ctx->command_list.items;
			cmd->jump.dst = (char*) cnt->head + sizeof(gs_gui_jumpcommand_t);
		} 
        else 
        {
			gs_gui_container_t *prev = ctx->root_list.items[i - 1];
			prev->tail->jump.dst = (char*) cnt->head + sizeof(gs_gui_jumpcommand_t);
		}

		// Make the last container's tail jump to the end of command list
		if (i == n - 1) 
        {
			cnt->tail->jump.dst = ctx->command_list.items + ctx->command_list.idx;
		}
	}
} 

GS_API_DECL void gs_gui_render(gs_gui_context_t* ctx, gs_command_buffer_t* cb)
{
    const gs_vec2 fb = gs_platform_framebuffer_sizev(ctx->window_hndl);

    gsi_camera2D(&ctx->gsi);
    gsi_blend_enabled(&ctx->gsi, true);

    gs_gui_command_t* cmd = NULL; 
    while (gs_gui_next_command(ctx, &cmd)) 
    {
      switch (cmd->type) 
      {
        case GS_GUI_COMMAND_TEXT:
        {
            const gs_vec2* tp = &cmd->text.pos;
            const char* ts = &cmd->text.str;
            const gs_color_t* tc = &cmd->text.color; 
            const gs_asset_font_t* tf = cmd->text.font;
            gsi_text(&ctx->gsi, tp->x, tp->y, ts, tf, false, tc->r, tc->g, tc->b, tc->a);
        } break;

        case GS_GUI_COMMAND_SHAPE:
        {
            gsi_texture(&ctx->gsi, gs_handle_invalid(gs_graphics_texture_t));
            gs_color_t* c = &cmd->shape.color;

            switch (cmd->shape.type)
            {
                case GS_GUI_SHAPE_RECT:
                {
                    gs_gui_rect_t* r = &cmd->shape.rect; 
                    gsi_rectvd(&ctx->gsi, gs_v2(r->x, r->y), gs_v2(r->w, r->h), gs_v2s(0.f), gs_v2s(1.f), *c, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
                } break;

                case GS_GUI_SHAPE_CIRCLE:
                {
                    // gs_gui_rect_t cr = gs_gui_get_clip_rect(ctx);
                    // gs_immediate_draw_t* dl = &ctx->overlay_draw_list;
                    // gsi_rectvd(dl, gs_v2(cr.x, cr.y), gs_v2(cr.w, cr.h), gs_v2s(0.f), gs_v2s(1.f), GS_COLOR_GREEN, GS_GRAPHICS_PRIMITIVE_LINES);
                    gs_vec2* cp = &cmd->shape.circle.center;
                    float* r = &cmd->shape.circle.radius;
                    gsi_circle(&ctx->gsi, cp->x, cp->y, *r, 16, c->r, c->g, c->b, c->a, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
                } break;

                case GS_GUI_SHAPE_TRIANGLE:
                {
                    gs_vec2* pa = &cmd->shape.triangle.points[0];
                    gs_vec2* pb = &cmd->shape.triangle.points[1];
                    gs_vec2* pc = &cmd->shape.triangle.points[2];
                    gsi_trianglev(&ctx->gsi, *pa, *pb, *pc, *c, GS_GRAPHICS_PRIMITIVE_TRIANGLES);

                } break;
            }
            
        } break;

        case GS_GUI_COMMAND_ICON:
        {
        } break;

        case GS_GUI_COMMAND_IMAGE:
        {
            gsi_texture(&ctx->gsi, cmd->image.hndl);
            gs_color_t* c = &cmd->image.color;
            gs_gui_rect_t* r = &cmd->image.rect; 
            gs_vec4* uvs = &cmd->image.uvs;
            gsi_rectvd(&ctx->gsi, gs_v2(r->x, r->y), gs_v2(r->w, r->h), gs_v2(uvs->x, uvs->y), gs_v2(uvs->z, uvs->w), *c, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
        } break;

        case GS_GUI_COMMAND_CLIP:
        { 
            // Will project scissor/clipping rectangles into framebuffer space
            gs_vec2 clip_off = gs_v2s(0.f);     // (0,0) unless using multi-viewports
            gs_vec2 clip_scale = gs_v2s(1.f);   // (1,1) unless using retina display which are often (2,2) 

            gs_gui_rect_t clip_rect;
            clip_rect.x = (cmd->clip.rect.x - clip_off.x) * clip_scale.x;
            clip_rect.y = (cmd->clip.rect.y - clip_off.y) * clip_scale.y;
            clip_rect.w = (cmd->clip.rect.w - clip_off.x) * clip_scale.x;
            clip_rect.h = (cmd->clip.rect.h - clip_off.y) * clip_scale.y;

            clip_rect.x = gs_max(clip_rect.x, 0.f);
            clip_rect.y = gs_max(clip_rect.y, 0.f);
            clip_rect.w = gs_max(clip_rect.w, 0.f);
            clip_rect.h = gs_max(clip_rect.h, 0.f); 

            // gs_println("clip: <%.2f, %>2f, %.2f, %.2f>", clip_rect.x, fb.y - clip_rect.h - clip_rect.y, clip_rect.w, clip_rect.h);

            gsi_set_view_scissor(&ctx->gsi, 
                (int32_t)(clip_rect.x), 
                (int32_t)(fb.y - clip_rect.h - clip_rect.y), 
                (int32_t)(clip_rect.w), 
                (int32_t)(clip_rect.h));

        } break;
      }
    }

    // Draw main list
    gsi_draw(&ctx->gsi, cb); 

    // Draw overlay list
    gsi_draw(&ctx->overlay_draw_list, cb);
}

GS_API_DECL void gs_gui_set_focus(gs_gui_context_t* ctx, gs_gui_id id) 
{
	ctx->focus = id;
	ctx->updated_focus = 1;
}

GS_API_DECL gs_gui_id gs_gui_get_id(gs_gui_context_t* ctx, const void* data, int32_t size) 
{
	int32_t idx = ctx->id_stack.idx;
	gs_gui_id res = (idx > 0) ? ctx->id_stack.items[idx - 1] : GS_GUI_HASH_INITIAL;
	gs_gui_hash(&res, data, size);
	ctx->last_id = res;
	return res;
}

GS_API_DECL void gs_gui_push_id(gs_gui_context_t* ctx, const void* data, int32_t size) 
{
	gs_gui_stack_push(ctx->id_stack, gs_gui_get_id(ctx, data, size));
}

GS_API_DECL void gs_gui_pop_id(gs_gui_context_t* ctx) 
{
	gs_gui_stack_pop(ctx->id_stack);
} 

GS_API_DECL void gs_gui_push_clip_rect(gs_gui_context_t* ctx, gs_gui_rect_t rect) 
{
	gs_gui_rect_t last = gs_gui_get_clip_rect(ctx);
	gs_gui_stack_push(ctx->clip_stack, gs_gui_intersect_rects(rect, last));
} 

GS_API_DECL void gs_gui_pop_clip_rect(gs_gui_context_t* ctx) 
{
	gs_gui_stack_pop(ctx->clip_stack);
} 

GS_API_DECL gs_gui_rect_t gs_gui_get_clip_rect(gs_gui_context_t* ctx) 
{
	gs_gui_expect(ctx->clip_stack.idx > 0);
	return ctx->clip_stack.items[ctx->clip_stack.idx - 1];
} 

GS_API_DECL int32_t gs_gui_check_clip(gs_gui_context_t* ctx, gs_gui_rect_t r) 
{
	gs_gui_rect_t cr = gs_gui_get_clip_rect(ctx);

	if (r.x > cr.x + cr.w || r.x + r.w < cr.x ||
			r.y > cr.y + cr.h || r.y + r.h < cr.y) 
    { 
        return GS_GUI_CLIP_ALL; 
    }

	if (r.x >= cr.x && r.x + r.w <= cr.x + cr.w &&
			r.y >= cr.y && r.y + r.h <= cr.y + cr.h ) 
    { 
        return 0; 
    }

	return GS_GUI_CLIP_PART;
}

GS_API_DECL gs_gui_container_t* gs_gui_get_current_container(gs_gui_context_t* ctx) 
{
	gs_gui_expect(ctx->container_stack.idx > 0);
	return ctx->container_stack.items[ ctx->container_stack.idx - 1 ];
} 

GS_API_DECL gs_gui_container_t* gs_gui_get_container(gs_gui_context_t* ctx, const char* name) 
{
	gs_gui_id id = gs_gui_get_id(ctx, name, strlen(name));
	return gs_gui_get_container_ex(ctx, id, 0);
}

GS_API_DECL void gs_gui_bring_to_front(gs_gui_context_t* ctx, gs_gui_container_t* cnt) 
{
    gs_gui_container_t* root = gs_gui_get_root_container(ctx, cnt);
    if (root->opt & GS_GUI_OPT_NOBRINGTOFRONT)
    {
        if (cnt->opt & GS_GUI_OPT_DOCKSPACE) cnt->zindex = 0;
        else cnt->zindex = 2;
        if (cnt->tab_bar)
        {
            gs_gui_tab_bar_t* tab_bar = gs_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
            for (uint32_t i = 0; i < tab_bar->size; ++i)
            {
                ((gs_gui_container_t*)tab_bar->items[i].data)->zindex = cnt->zindex + i;
            }
        }
    } 
    else
    {
        cnt->zindex = ++ctx->last_zindex;

        // If container is part of a tab item, then iterate and bring to front as well
        if (cnt->tab_bar)
        {
            gs_gui_tab_bar_t* tab_bar = gs_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
            for (uint32_t i = 0; i < tab_bar->size; ++i)
            {
                ((gs_gui_container_t*)tab_bar->items[i].data)->zindex = ++ctx->last_zindex;
            }
        }
    }
} 

/*============================================================================
** Pool
**============================================================================*/

GS_API_DECL int32_t gs_gui_pool_init(gs_gui_context_t* ctx, gs_gui_pool_item_t* items, int32_t len, gs_gui_id id) 
{
	int32_t i, n = -1, f = ctx->frame;
	for (i = 0; i < len; i++) 
    {
		if (items[i].last_update < f) 
        {
			f = items[i].last_update;
			n = i;
		}
	}

	gs_gui_expect(n > -1);
	items[n].id = id;
	gs_gui_pool_update(ctx, items, n);

	return n;
} 

GS_API_DECL int32_t gs_gui_pool_get(gs_gui_context_t* ctx, gs_gui_pool_item_t* items, int32_t len, gs_gui_id id) 
{
    // Note(john): This is a linear hash lookup. Could speed this up with a quadratic lookup. 
	int32_t i;
	gs_gui_unused(ctx);
	for (i = 0; i < len; i++) 
    {
		if (items[i].id == id) 
        { 
            return i; 
        }
	}
	return -1;
}

GS_API_DECL void gs_gui_pool_update(gs_gui_context_t* ctx, gs_gui_pool_item_t* items, int32_t idx) 
{
	items[idx].last_update = ctx->frame;
} 

/*============================================================================
** input handlers
**============================================================================*/

GS_API_DECL void gs_gui_input_mousemove(gs_gui_context_t* ctx, int32_t x, int32_t y) 
{
	ctx->mouse_pos = gs_v2(x, y);
}

GS_API_DECL void gs_gui_input_mousedown(gs_gui_context_t* ctx, int32_t x, int32_t y, int32_t btn) 
{
	gs_gui_input_mousemove(ctx, x, y);
	ctx->mouse_down |= btn;
	ctx->mouse_pressed |= btn;
}

GS_API_DECL void gs_gui_input_mouseup(gs_gui_context_t* ctx, int32_t x, int32_t y, int32_t btn) 
{
	gs_gui_input_mousemove(ctx, x, y);
	ctx->mouse_down &= ~btn;
} 

GS_API_DECL void gs_gui_input_scroll(gs_gui_context_t* ctx, int32_t x, int32_t y) 
{
	ctx->scroll_delta.x += x;
	ctx->scroll_delta.y += y;
} 

GS_API_DECL void gs_gui_input_keydown(gs_gui_context_t* ctx, int32_t key) 
{
	ctx->key_pressed |= key;
	ctx->key_down |= key;
} 

GS_API_DECL void gs_gui_input_keyup(gs_gui_context_t* ctx, int32_t key) 
{
	ctx->key_down &= ~key;
} 

GS_API_DECL void gs_gui_input_text(gs_gui_context_t* ctx, const char* text) 
{
	int32_t len = strlen(ctx->input_text);
	int32_t size = strlen(text) + 1;
	if (len + size > (int32_t)sizeof(ctx->input_text)) return;
	memcpy(ctx->input_text + len, text, size);
} 

/*============================================================================
** commandlist
**============================================================================*/

GS_API_DECL gs_gui_command_t* gs_gui_push_command(gs_gui_context_t* ctx, int32_t type, int32_t size) 
{
	gs_gui_command_t* cmd = (gs_gui_command_t*) (ctx->command_list.items + ctx->command_list.idx);
	gs_gui_expect(ctx->command_list.idx + size < GS_GUI_COMMANDLIST_SIZE);
	cmd->base.type = type;
	cmd->base.size = size;
	ctx->command_list.idx += size;
	return cmd;
} 

GS_API_DECL int32_t gs_gui_next_command(gs_gui_context_t* ctx, gs_gui_command_t** cmd) 
{
	if (*cmd) 
    {
		*cmd = (gs_gui_command_t*) (((char*) *cmd) + (*cmd)->base.size);
	} 
    else 
    {
		*cmd = (gs_gui_command_t*) ctx->command_list.items;
	}

	while ((char*) *cmd != ctx->command_list.items + ctx->command_list.idx) 
    {
		if ((*cmd)->type != GS_GUI_COMMAND_JUMP) 
        { 
            return 1; 
        }
		*cmd = (*cmd)->jump.dst;
	}
	return 0;
} 

GS_API_DECL void gs_gui_set_clip(gs_gui_context_t* ctx, gs_gui_rect_t rect) 
{
	gs_gui_command_t* cmd;
	cmd = gs_gui_push_command(ctx, GS_GUI_COMMAND_CLIP, sizeof(gs_gui_clipcommand_t));
	cmd->clip.rect = rect;
} 

GS_API_DECL void gs_gui_draw_rect(gs_gui_context_t* ctx, gs_gui_rect_t rect, gs_color_t color) 
{
	gs_gui_command_t* cmd;
	rect = gs_gui_intersect_rects(rect, gs_gui_get_clip_rect(ctx));
	if (rect.w > 0 && rect.h > 0) 
    {
		cmd = gs_gui_push_command(ctx, GS_GUI_COMMAND_SHAPE, sizeof(gs_gui_shapecommand_t));
        cmd->shape.type = GS_GUI_SHAPE_RECT;
		cmd->shape.rect = rect;
		cmd->shape.color = color;
	}
} 

GS_API_DECL void gs_gui_draw_circle(gs_gui_context_t* ctx, gs_vec2 position, float radius, gs_color_t color)
{
	gs_gui_command_t* cmd;
    gs_gui_rect_t rect = gs_gui_rect(position.x - radius, position.y - radius, 2.f * radius, 2.f * radius);

	// do clip command if the rect isn't fully contained within the cliprect
	int32_t clipped = gs_gui_check_clip(ctx, rect);
	if (clipped == GS_GUI_CLIP_ALL ) {return;}
	if (clipped == GS_GUI_CLIP_PART) {gs_gui_set_clip(ctx, gs_gui_get_clip_rect(ctx));}

	// do shape command
	cmd = gs_gui_push_command(ctx, GS_GUI_COMMAND_SHAPE, sizeof(gs_gui_shapecommand_t));
	cmd->shape.type = GS_GUI_SHAPE_CIRCLE;
	cmd->shape.circle.center = position;
	cmd->shape.circle.radius = radius;
	cmd->shape.color = color;

	// reset clipping if it was set
	if (clipped) {gs_gui_set_clip(ctx, gs_gui_unclipped_rect);}
}

GS_API_DECL void gs_gui_draw_triangle(gs_gui_context_t* ctx, gs_vec2 a, gs_vec2 b, gs_vec2 c, gs_color_t color)
{
	gs_gui_command_t* cmd;

	// Check each point against rect (if partially clipped, then good
	int32_t clipped = 0x00; 
	gs_gui_rect_t clip = gs_gui_get_clip_rect(ctx);
	int32_t ca = gs_gui_rect_overlaps_vec2(clip, a);
	int32_t cb = gs_gui_rect_overlaps_vec2(clip, b);
	int32_t cc = gs_gui_rect_overlaps_vec2(clip, c);

    if (ca && cb && cc) clipped = 0x00; // No clip
    else if (!ca && !cb && !cc) clipped = GS_GUI_CLIP_ALL;
	else if (ca || cb || cc) clipped = GS_GUI_CLIP_PART;

    if (clipped == GS_GUI_CLIP_ALL) {return;}
    if (clipped == GS_GUI_CLIP_PART) {gs_gui_set_clip(ctx, clip);} 

    cmd = gs_gui_push_command(ctx, GS_GUI_COMMAND_SHAPE, sizeof(gs_gui_shapecommand_t));
    cmd->shape.type = GS_GUI_SHAPE_TRIANGLE;
    cmd->shape.triangle.points[0] = a;
    cmd->shape.triangle.points[1] = b;
    cmd->shape.triangle.points[2] = c;
    cmd->shape.color = color;

    // Reset clipping if set
    if (clipped) {gs_gui_set_clip(ctx, gs_gui_unclipped_rect);}
}

GS_API_DECL void gs_gui_draw_box(gs_gui_context_t* ctx, gs_gui_rect_t rect, gs_color_t color) 
{
	gs_gui_draw_rect(ctx, gs_gui_rect(rect.x + 1, rect.y, rect.w - 2, 1), color);
	gs_gui_draw_rect(ctx, gs_gui_rect(rect.x + 1, rect.y + rect.h - 1, rect.w - 2, 1), color);
	gs_gui_draw_rect(ctx, gs_gui_rect(rect.x, rect.y, 1, rect.h), color);
	gs_gui_draw_rect(ctx, gs_gui_rect(rect.x + rect.w - 1, rect.y, 1, rect.h), color);
}

GS_API_DECL void gs_gui_draw_text(gs_gui_context_t* ctx, gs_asset_font_t* font, const char* str, int32_t len,
	gs_vec2 pos, gs_color_t color)
{
    // Set to default font
    if (!font)
    {
        font = gsi_default_font(); 
    } 

#define DRAW_TEXT(TEXT, RECT, COLOR)\
    do\
    {\
        gs_gui_command_t* cmd;\
        gs_vec2 td = ctx->text_dimensions(font, TEXT, -1);\
        gs_gui_rect_t rect = (RECT);\
        int32_t clipped = gs_gui_check_clip(ctx, rect);\
\
        if (clipped == GS_GUI_CLIP_ALL)\
        {\
            return;\
        }\
\
        if (clipped == GS_GUI_CLIP_PART)\
        {\
            gs_gui_rect_t crect = gs_gui_get_clip_rect(ctx);\
            gs_gui_set_clip(ctx, crect);\
        }\
\
        /* add command */\
        if (len < 0)\
        {\
            len = strlen(TEXT);\
        }\
\
        cmd = gs_gui_push_command(ctx, GS_GUI_COMMAND_TEXT, sizeof(gs_gui_textcommand_t) + len);\
        memcpy(cmd->text.str, TEXT, len);\
        cmd->text.str[len] = '\0';\
        cmd->text.pos = gs_v2(rect.x, rect.y);\
        cmd->text.color = COLOR;\
        cmd->text.font = font;\
\
        if (clipped)\
        {\
            gs_gui_set_clip(ctx, gs_gui_unclipped_rect);\
        }\
    } while (0)

    // Draw shadow
    {
        DRAW_TEXT(str, gs_gui_rect(pos.x + 1.f, pos.y + 1.f, td.x, td.y), gs_color_alpha(GS_COLOR_BLACK, (uint8_t)((float)color.a / 2.f)));
    }
    
    // Draw text
    {
        DRAW_TEXT(str, gs_gui_rect(pos.x, pos.y, td.x, td.y), color);
    } 
} 

GS_API_DECL void gs_gui_draw_icon(gs_gui_context_t* ctx, int32_t id, gs_gui_rect_t rect, gs_color_t color) 
{
	gs_gui_command_t* cmd;

	/* do clip command if the rect isn't fully contained within the cliprect */
	int32_t clipped = gs_gui_check_clip(ctx, rect);
	if (clipped == GS_GUI_CLIP_ALL ) {return;}
	if (clipped == GS_GUI_CLIP_PART) {gs_gui_set_clip(ctx, gs_gui_get_clip_rect(ctx));}

	/* do icon command */
	cmd = gs_gui_push_command(ctx, GS_GUI_COMMAND_ICON, sizeof(gs_gui_iconcommand_t));
	cmd->icon.id = id;
	cmd->icon.rect = rect;
	cmd->icon.color = color;

	/* reset clipping if it was set */
	if (clipped) { gs_gui_set_clip(ctx, gs_gui_unclipped_rect); }
} 

GS_API_DECL void gs_gui_draw_image(gs_gui_context_t *ctx, gs_handle(gs_graphics_texture_t) hndl, gs_gui_rect_t rect, gs_vec2 uv0, gs_vec2 uv1, gs_color_t color)
{
	gs_gui_command_t* cmd;

	/* do clip command if the rect isn't fully contained within the cliprect */
	int32_t clipped = gs_gui_check_clip(ctx, rect);
	if (clipped == GS_GUI_CLIP_ALL ) {return;}
	if (clipped == GS_GUI_CLIP_PART) {gs_gui_set_clip(ctx, gs_gui_get_clip_rect(ctx));}

	/* do icon command */
	cmd = gs_gui_push_command(ctx, GS_GUI_COMMAND_IMAGE, sizeof(gs_gui_imagecommand_t));
	cmd->image.hndl = hndl;
	cmd->image.rect = rect;
    cmd->image.uvs = gs_v4(uv0.x, uv0.y, uv1.x, uv1.y);
	cmd->image.color = color;

	/* reset clipping if it was set */
	if (clipped) { gs_gui_set_clip(ctx, gs_gui_unclipped_rect); }
} 

GS_API_DECL void gs_gui_draw_nine_rect(gs_gui_context_t* ctx, gs_handle(gs_graphics_texture_t) hndl, gs_gui_rect_t rect, gs_vec2 uv0, gs_vec2 uv1, uint32_t left, uint32_t right, uint32_t top, uint32_t bottom, gs_color_t color)
{
    // Draw images based on rect, slice image based on uvs (uv0, uv1), original texture dimensions (width, height) and control margins (left, right, top, bottom) 
    gs_graphics_texture_desc_t desc = gs_default_val();
    gs_graphics_texture_desc_query(hndl, &desc);
    uint32_t width = desc.width;
    uint32_t height = desc.height;

    // tl
    {
        uint32_t w = left;
        uint32_t h = top;
        gs_gui_rect_t r = gs_gui_rect(rect.x, rect.y, w, h);
        gs_vec2 st0 = gs_v2(uv0.x, uv0.y);
        gs_vec2 st1 = gs_v2(uv0.x + ((float)left / (float)width), uv0.y + ((float)top / (float)height));
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // tr
    {
        uint32_t w = right;
        uint32_t h = top;
        gs_gui_rect_t r = gs_gui_rect(rect.x + rect.w - w, rect.y, w, h);
        gs_vec2 st0 = gs_v2(uv1.x - ((float)right / (float)width), uv0.y);
        gs_vec2 st1 = gs_v2(uv1.x, uv0.y + ((float)top / (float)height));
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // br
    {
        uint32_t w = right;
        uint32_t h = bottom;
        gs_gui_rect_t r = gs_gui_rect(rect.x + rect.w - w, rect.y + rect.h - h, w, h);
        gs_vec2 st0 = gs_v2(uv1.x - ((float)right / (float)width), uv1.y - ((float)bottom / (float)height));
        gs_vec2 st1 = gs_v2(uv1.x, uv1.y);
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // bl
    {
        uint32_t w = left;
        uint32_t h = bottom;
        gs_gui_rect_t r = gs_gui_rect(rect.x, rect.y + rect.h - h, w, h);
        gs_vec2 st0 = gs_v2(uv0.x, uv1.y - ((float)bottom / (float)height));
        gs_vec2 st1 = gs_v2(uv0.x + ((float)left / (float)width), uv1.y);
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // top
    {
        uint32_t w = rect.w - left - right;
        uint32_t h = top;
        gs_gui_rect_t r = gs_gui_rect(rect.x + left, rect.y, w, h);
        gs_vec2 st0 = gs_v2(uv0.x + ((float)left / (float)width), uv0.y);
        gs_vec2 st1 = gs_v2(uv1.x - ((float)right / (float)width), uv0.y + ((float)top / (float)height));
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // bottom
    {
        uint32_t w = rect.w - left - right;
        uint32_t h = bottom;
        gs_gui_rect_t r = gs_gui_rect(rect.x + left, rect.y + rect.h - h, w, h);
        gs_vec2 st0 = gs_v2(uv0.x + ((float)left / (float)width), uv1.y - ((float)bottom / (float)height));
        gs_vec2 st1 = gs_v2(uv1.x - ((float)right / (float)width), uv1.y);
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // left
    {
        uint32_t w = left;
        uint32_t h = rect.h - top - bottom;
        gs_gui_rect_t r = gs_gui_rect(rect.x, rect.y + top, w, h);
        gs_vec2 st0 = gs_v2(uv0.x, uv0.y + ((float)top / (float)height));
        gs_vec2 st1 = gs_v2(uv0.x + ((float)left / (float)width), uv1.y - ((float)bottom / (float)height));
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // right
    {
        uint32_t w = right;
        uint32_t h = rect.h - top - bottom;
        gs_gui_rect_t r = gs_gui_rect(rect.x + rect.w - w, rect.y + top, w, h);
        gs_vec2 st0 = gs_v2(uv1.x - ((float)right / (float)width), uv0.y + ((float)top / (float)height));
        gs_vec2 st1 = gs_v2(uv1.x, uv1.y - ((float)bottom / (float)height));
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // center
    {
        uint32_t w = rect.w - right - left;
        uint32_t h = rect.h - top - bottom;
        gs_gui_rect_t r = gs_gui_rect(rect.x + left, rect.y + top, w, h);
        gs_vec2 st0 = gs_v2(uv0.x + ((float)left / (float)width), uv0.y + ((float)top / (float)height));
        gs_vec2 st1 = gs_v2(uv1.x - ((float)right / (float)width), uv1.y - ((float)bottom / (float)height));
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }
}

/*============================================================================
** layout
**============================================================================*/
enum {
    GS_GUI_RELATIVE = 1,
    GS_GUI_ABSOLUTE = 2
}; 

GS_API_DECL void gs_gui_layout_begin_column(gs_gui_context_t* ctx) 
{
	gs_gui_push_layout(ctx, gs_gui_layout_next(ctx), gs_v2(0, 0));
} 

GS_API_DECL void gs_gui_layout_end_column(gs_gui_context_t* ctx) 
{
	gs_gui_layout_t *a, *b;
	b = gs_gui_get_layout(ctx);
	gs_gui_stack_pop(ctx->layout_stack);

	/* inherit position/next_row/max from child layout if they are greater */
	a = gs_gui_get_layout(ctx);
	a->position.x = gs_max(a->position.x, b->position.x + b->body.x - a->body.x);
	a->next_row = gs_max(a->next_row, b->next_row + b->body.y - a->body.y);
	a->max.x = gs_max(a->max.x, b->max.x);
	a->max.y = gs_max(a->max.y, b->max.y);
} 

GS_API_DECL void gs_gui_layout_row(gs_gui_context_t* ctx, int32_t items, const int32_t* widths, int32_t height) 
{
	gs_gui_layout_t* layout = gs_gui_get_layout(ctx);
	if (widths) 
    {
		gs_gui_expect(items <= GS_GUI_MAX_WIDTHS);
		memcpy(layout->widths, widths, items * sizeof(widths[0]));
	}
	layout->items = items;
	layout->position = gs_v2(layout->indent, layout->next_row);
	layout->size.y = height;
	layout->item_index = 0;
} 

GS_API_DECL void gs_gui_layout_width(gs_gui_context_t *ctx, int32_t width) 
{
	gs_gui_get_layout(ctx)->size.x = width;
} 

GS_API_DECL void gs_gui_layout_height(gs_gui_context_t *ctx, int32_t height) 
{
	gs_gui_get_layout(ctx)->size.y = height;
} 

GS_API_DECL void gs_gui_layout_set_next(gs_gui_context_t *ctx, gs_gui_rect_t r, int32_t relative) 
{
	gs_gui_layout_t *layout = gs_gui_get_layout(ctx);
	layout->next = r;
	layout->next_type = relative ? GS_GUI_RELATIVE : GS_GUI_ABSOLUTE;
} 

GS_API_DECL gs_gui_rect_t gs_gui_layout_next(gs_gui_context_t *ctx) 
{
	gs_gui_layout_t* layout = gs_gui_get_layout(ctx);
	gs_gui_style_t* style = ctx->style;
	gs_gui_rect_t res;

	if (layout->next_type) 
    {
		/* handle rect set by `gs_gui_layout_set_next` */
		int32_t type = layout->next_type;
		layout->next_type = 0;
		res = layout->next;
		if (type == GS_GUI_ABSOLUTE) 
        { 
            return (ctx->last_rect = res); 
        }

	} 
    else 
    {
		/* handle next row */
		if (layout->item_index == layout->items) 
        {
			gs_gui_layout_row(ctx, layout->items, NULL, layout->size.y);
		}

		/* position */
		res.x = layout->position.x;
		res.y = layout->position.y;

		/* size */
		res.w = layout->items > 0 ? layout->widths[layout->item_index] : layout->size.x;
		res.h = layout->size.y;

		if (res.w == 0) { res.w = style->size.x + style->padding * 2; }
		if (res.h == 0) { res.h = style->size.y + style->padding * 2; }
		if (res.w <	0) { res.w += layout->body.w - res.x + 1; }
		if (res.h <	0) { res.h += layout->body.h - res.y + 1; }

		layout->item_index++;
	}

	/* update position */
	layout->position.x += res.w + style->spacing;
	layout->next_row = gs_max(layout->next_row, res.y + res.h + style->spacing);

	/* apply body offset */
	res.x += layout->body.x;
	res.y += layout->body.y;

	/* update max position */
	layout->max.x = gs_max(layout->max.x, res.x + res.w);
	layout->max.y = gs_max(layout->max.y, res.y + res.h);

	return (ctx->last_rect = res);
} 

/*============================================================================
** controls
**============================================================================*/

static int32_t gs_gui_in_hover_root(gs_gui_context_t *ctx) 
{
	int32_t i = ctx->container_stack.idx;
	while (i--) 
    {
		if (ctx->container_stack.items[i] == ctx->hover_root) { return 1; }

		/* only root containers have their `head` field set; stop searching if we've
		** reached the current root container */
		if (ctx->container_stack.items[i]->head) { break; }
	}
	return 0;
} 

GS_API_DECL void gs_gui_draw_control_frame(gs_gui_context_t *ctx, gs_gui_id id, gs_gui_rect_t rect,
	int32_t colorid, int32_t opt)
{
	if (opt & GS_GUI_OPT_NOFRAME) { return; }
	colorid += (ctx->focus == id) ? 2 : (ctx->hover == id) ? 1 : 0;
	ctx->draw_frame(ctx, rect, colorid);
} 

GS_API_DECL void gs_gui_draw_control_text(gs_gui_context_t *ctx, const char *str, gs_gui_rect_t rect,
	int32_t colorid, int32_t opt)
{
	gs_vec2 pos;
	gs_asset_font_t* font = ctx->style->font; 
    gs_vec2 td = ctx->text_dimensions(font, str, -1);
	int32_t tw = (int32_t)td.x;
    int32_t th = (int32_t)td.y;

	gs_gui_push_clip_rect(ctx, rect);
	pos.y = rect.y + (rect.h - th) / 2;

	if (opt & GS_GUI_OPT_ALIGNCENTER) 
    {
		pos.x = rect.x + (rect.w - tw) / 2;
	} 
    else if (opt & GS_GUI_OPT_ALIGNRIGHT) 
    {
		pos.x = rect.x + rect.w - tw - ctx->style->padding;
	} 
    else 
    {
		pos.x = rect.x + ctx->style->padding;
	} 

	gs_gui_draw_text(ctx, font, str, -1, pos, ctx->style->colors[colorid]);
	gs_gui_pop_clip_rect(ctx);
} 

GS_API_DECL int32_t gs_gui_mouse_over(gs_gui_context_t *ctx, gs_gui_rect_t rect) 
{
	return gs_gui_rect_overlaps_vec2(rect, ctx->mouse_pos) && !ctx->hover_split && !ctx->lock_hover_id && 
		gs_gui_rect_overlaps_vec2(gs_gui_get_clip_rect(ctx), ctx->mouse_pos) &&
		gs_gui_in_hover_root(ctx);
} 

GS_API_DECL void gs_gui_update_control(gs_gui_context_t *ctx, gs_gui_id id, gs_gui_rect_t rect, int32_t opt) 
{ 
    int32_t mouseover = 0;
    gs_immediate_draw_t* dl = &ctx->overlay_draw_list; 

    if (opt & GS_GUI_OPT_FORCEFOCUS)
    {
        mouseover = gs_gui_rect_overlaps_vec2(gs_gui_get_clip_rect(ctx), ctx->mouse_pos);
    }
    else
    {
	    mouseover = gs_gui_mouse_over(ctx, rect);
    } 

	if (ctx->focus == id) { ctx->updated_focus = 1; }
	if (opt & GS_GUI_OPT_NOINTERACT) { return; }
	if (mouseover && !ctx->mouse_down) { ctx->hover = id; }

	if (ctx->focus == id) 
    {
		if (ctx->mouse_pressed && !mouseover) { gs_gui_set_focus(ctx, 0); }
		if (!ctx->mouse_down && ~opt & GS_GUI_OPT_HOLDFOCUS) { gs_gui_set_focus(ctx, 0); }
	}

	if (ctx->hover == id) 
    {
        // gsi_rectvd(dl, gs_v2(rect.x, rect.y), gs_v2(rect.w, rect.h), gs_v2s(0.f), gs_v2s(1.f), GS_COLOR_RED, GS_GRAPHICS_PRIMITIVE_LINES);

		if (ctx->mouse_pressed) 
        {
			gs_gui_set_focus(ctx, id);
		} 
        else if (!mouseover) 
        {
			ctx->hover = 0;
		}
	}

    if (ctx->focus == id)
    {
        // gsi_rectvd(dl, gs_v2(rect.x, rect.y), gs_v2(rect.w, rect.h), gs_v2s(0.f), gs_v2s(1.f), GS_COLOR_GREEN, GS_GRAPHICS_PRIMITIVE_LINES);
    }
} 

GS_API_DECL void gs_gui_text(gs_gui_context_t *ctx, const char *text, int32_t wrap)
{
	const char *start, *end, *p = text;
	int32_t width = -1;
	gs_asset_font_t* font = ctx->style->font;
	gs_color_t color = ctx->style->colors[GS_GUI_COLOR_TEXT];
    int32_t th = ctx->font_height(font);
	gs_gui_layout_begin_column(ctx);
	gs_gui_layout_row(ctx, 1, &width, th);

	do 
    {
	    gs_gui_rect_t r = gs_gui_layout_next(ctx);
		int32_t w = 0;
		start = end = p;
		do 
        {
			const char* word = p;
			while (*p && *p != ' ' && *p != '\n') 
            { 
                p++; 
            }

            if (wrap) w += ctx->text_width(font, word, p - word);
			if (w > r.w && end != start) 
            { 
                break; 
            }

			if (wrap) w += ctx->text_width(font, p, 1);
			end = p++;

		} while (*end && *end != '\n');

        // Draw text
		gs_gui_draw_text(ctx, font, start, end - start, gs_v2(r.x, r.y), color);
		p = end + 1;

	} while (*end);

	gs_gui_layout_end_column(ctx);
}

GS_API_DECL void gs_gui_label(gs_gui_context_t *ctx, const char *text) 
{
	gs_gui_draw_control_text(ctx, text, gs_gui_layout_next(ctx), GS_GUI_COLOR_TEXT, 0);
} 

GS_API_DECL int32_t gs_gui_button_ex(gs_gui_context_t *ctx, const char *label, int32_t icon, int32_t opt) 
{
	int32_t res = 0;
	gs_gui_id id = label ? gs_gui_get_id(ctx, label, strlen(label))
									 : gs_gui_get_id(ctx, &icon, sizeof(icon));
	gs_gui_rect_t r = gs_gui_layout_next(ctx);
	gs_gui_update_control(ctx, id, r, opt);

	/* handle click */
	if (ctx->mouse_pressed == GS_GUI_MOUSE_LEFT && ctx->focus == id) 
    {
		res |= GS_GUI_RES_SUBMIT;
	}

	// draw with callback
    if (ctx->callbacks.button)
    {
        ctx->callbacks.button(ctx, r, id, ctx->hover == id, ctx->focus == id, opt, label, icon);
    }
    else
    {
        gs_gui_draw_control_frame(ctx, id, r, GS_GUI_COLOR_BUTTON, opt);
        if (label) {gs_gui_draw_control_text(ctx, label, r, GS_GUI_COLOR_TEXT, opt);}
        if (icon) {gs_gui_draw_icon(ctx, icon, r, ctx->style->colors[GS_GUI_COLOR_TEXT]);}
    }

	return res;
}

GS_API_DECL int32_t gs_gui_checkbox(gs_gui_context_t *ctx, const char *label, int32_t *state) 
{
	int32_t res = 0;
	gs_gui_id id = gs_gui_get_id(ctx, &state, sizeof(state));
	gs_gui_rect_t r = gs_gui_layout_next(ctx);
	gs_gui_rect_t box = gs_gui_rect(r.x, r.y, r.h, r.h);
	gs_gui_update_control(ctx, id, r, 0);

	/* handle click */
	if (ctx->mouse_pressed == GS_GUI_MOUSE_LEFT && ctx->focus == id) 
    {
		res |= GS_GUI_RES_CHANGE;
		*state = !*state;
	}

	/* draw */
	gs_gui_draw_control_frame(ctx, id, box, GS_GUI_COLOR_BASE, 0);
	if (*state) 
    {
		gs_gui_draw_icon(ctx, GS_GUI_ICON_CHECK, box, ctx->style->colors[GS_GUI_COLOR_TEXT]);
	}

	r = gs_gui_rect(r.x + box.w, r.y, r.w - box.w, r.h);
	gs_gui_draw_control_text(ctx, label, r, GS_GUI_COLOR_TEXT, 0);
	return res;
}

GS_API_DECL int32_t gs_gui_textbox_raw(gs_gui_context_t *ctx, char *buf, int32_t bufsz, gs_gui_id id, gs_gui_rect_t r,
	int32_t opt)
{
	int32_t res = 0;
	gs_gui_update_control(ctx, id, r, opt | GS_GUI_OPT_HOLDFOCUS);

	if (ctx->focus == id) 
    {
		/* handle text input */
		int32_t len = strlen(buf);
		int32_t n = gs_min(bufsz - len - 1, (int32_t) strlen(ctx->input_text)); 
		if (n > 0) 
        {
			memcpy(buf + len, ctx->input_text, n);
			len += n;
			buf[len] = '\0';
			res |= GS_GUI_RES_CHANGE;
		}

		/* handle backspace */
		if (ctx->key_pressed & GS_GUI_KEY_BACKSPACE && len > 0) 
        {
			/* skip utf-8 continuation bytes */
			while ((buf[--len] & 0xc0) == 0x80 && len > 0);
			buf[len] = '\0';
			res |= GS_GUI_RES_CHANGE;
		}

		/* handle return */
		if (ctx->key_pressed & GS_GUI_KEY_RETURN) 
        {
			gs_gui_set_focus(ctx, 0);
			res |= GS_GUI_RES_SUBMIT;
		}
	}

	/* draw */

    // Textbox border
    gs_gui_draw_box(ctx, gs_gui_expand_rect(r, 1), ctx->focus == id ? ctx->style->colors[GS_GUI_COLOR_BUTTONFOCUS] : ctx->style->colors[GS_GUI_COLOR_BUTTON]); 

    // Textbox bg 
	gs_gui_draw_control_frame(ctx, id, r, GS_GUI_COLOR_BASE, opt);

    // Text and carret
	if (ctx->focus == id) 
    {
		gs_color_t color = ctx->style->colors[GS_GUI_COLOR_TEXT];
		gs_asset_font_t* font = ctx->style->font; 
		int32_t textw = ctx->text_width(font, buf, -1);
        int32_t texth = ctx->font_height(font);
		int32_t ofx = r.w - ctx->style->padding - textw - 1;
		int32_t textx = r.x + gs_min(ofx, ctx->style->padding);
		int32_t texty = r.y + (r.h - texth) / 2;
		int32_t cary = r.y + 1; 
		// gs_gui_draw_box(ctx, gs_gui_rect(textx, texty, textw, texth), GS_COLOR_RED); 
		gs_gui_push_clip_rect(ctx, r); 

        // Draw text
		gs_gui_draw_text(ctx, font, buf, -1, gs_v2(textx, texty), color); 

        // Draw carret (want it to blink somehow)
		gs_gui_draw_rect(ctx, gs_gui_rect(textx + textw + 1, r.y + 5, 1, r.h - 10), color); 

		gs_gui_pop_clip_rect(ctx);
	} 
    else 
    {
		gs_color_t color = ctx->style->colors[GS_GUI_COLOR_TEXT];
		gs_asset_font_t* font = ctx->style->font; 
		int32_t textw = ctx->text_width(font, buf, -1);
		int32_t texth = ctx->text_height(font, buf, -1);
		int32_t textx = r.x + ctx->style->padding;
		int32_t texty = r.y + (r.h - texth) / 2;
		gs_gui_push_clip_rect(ctx, r); 
		gs_gui_draw_text(ctx, font, buf, -1, gs_v2(textx, texty), color);
		gs_gui_pop_clip_rect(ctx);
	}

	return res;
}

static int32_t gs_gui_number_textbox(gs_gui_context_t *ctx, gs_gui_real *value, gs_gui_rect_t r, gs_gui_id id) 
{
	if (ctx->mouse_pressed == GS_GUI_MOUSE_LEFT && ctx->key_down & GS_GUI_KEY_SHIFT &&
			ctx->hover == id
	) 
    {
		ctx->number_edit = id;
		gs_snprintf(ctx->number_edit_buf, GS_GUI_MAX_FMT, GS_GUI_REAL_FMT, *value);
	}
	if (ctx->number_edit == id) 
    {
		int32_t res = gs_gui_textbox_raw(
			ctx, ctx->number_edit_buf, sizeof(ctx->number_edit_buf), id, r, 0);
		if (res & GS_GUI_RES_SUBMIT || ctx->focus != id) 
        {
			*value = strtod(ctx->number_edit_buf, NULL);
			ctx->number_edit = 0;
		} 
        else 
        {
			return 1;
		}
	}
	return 0;
} 

GS_API_DECL int32_t gs_gui_textbox_ex(gs_gui_context_t *ctx, char *buf, int32_t bufsz, int32_t opt) 
{
	gs_gui_id id = gs_gui_get_id(ctx, &buf, sizeof(buf));
	gs_gui_rect_t r = gs_gui_layout_next(ctx);
	return gs_gui_textbox_raw(ctx, buf, bufsz, id, r, opt);
} 

GS_API_DECL int32_t gs_gui_slider_ex(gs_gui_context_t *ctx, gs_gui_real *value, gs_gui_real low, gs_gui_real high,
	gs_gui_real step, const char *fmt, int32_t opt)
{
	char buf[GS_GUI_MAX_FMT + 1];
	gs_gui_rect_t thumb;
	int32_t x, w, res = 0;
	gs_gui_real last = *value, v = last;
	gs_gui_id id = gs_gui_get_id(ctx, &value, sizeof(value));
	gs_gui_rect_t base = gs_gui_layout_next(ctx);

	/* handle text input mode */
	if (gs_gui_number_textbox(ctx, &v, base, id)) { return res; }

	/* handle normal mode */
	gs_gui_update_control(ctx, id, base, opt);

	/* handle input */
	if (ctx->focus == id &&
			(ctx->mouse_down | ctx->mouse_pressed) == GS_GUI_MOUSE_LEFT)
	{
		v = low + (ctx->mouse_pos.x - base.x) * (high - low) / base.w;
		if (step) { v = (((v + step / 2) / step)) * step; }
	}

	/* clamp and store value, update res */
	*value = v = gs_clamp(v, low, high);
	if (last != v) { res |= GS_GUI_RES_CHANGE; }

	/* draw base */
	gs_gui_draw_control_frame(ctx, id, base, GS_GUI_COLOR_BASE, opt);

	/* draw thumb */
	w = ctx->style->thumb_size;
	x = (v - low) * (base.w - w) / (high - low);
	thumb = gs_gui_rect(base.x + x, base.y, w, base.h);
	gs_gui_draw_control_frame(ctx, id, thumb, GS_GUI_COLOR_BUTTON, opt);

	/* draw text	*/
	gs_snprintf(buf, GS_GUI_MAX_FMT, fmt, v);
	gs_gui_draw_control_text(ctx, buf, base, GS_GUI_COLOR_TEXT, opt);

	return res;
} 

GS_API_DECL int32_t gs_gui_number_ex(gs_gui_context_t *ctx, gs_gui_real *value, gs_gui_real step,
	const char *fmt, int32_t opt)
{
	char buf[GS_GUI_MAX_FMT + 1];
	int32_t res = 0;
	gs_gui_id id = gs_gui_get_id(ctx, &value, sizeof(value));
	gs_gui_rect_t base = gs_gui_layout_next(ctx);
	gs_gui_real last = *value;

	/* handle text input mode */
	if (gs_gui_number_textbox(ctx, value, base, id)) { return res; }

	/* handle normal mode */
	gs_gui_update_control(ctx, id, base, opt);

	/* handle input */
	if (ctx->focus == id && ctx->mouse_down == GS_GUI_MOUSE_LEFT) 
    {
		*value += ctx->mouse_delta.x * step;
	}

	/* set flag if value changed */
	if (*value != last) { res |= GS_GUI_RES_CHANGE; }

	/* draw base */
	gs_gui_draw_control_frame(ctx, id, base, GS_GUI_COLOR_BASE, opt);

	/* draw text	*/
	gs_snprintf(buf, GS_GUI_MAX_FMT, fmt, *value);
	gs_gui_draw_control_text(ctx, buf, base, GS_GUI_COLOR_TEXT, opt);

	return res;
} 

static int32_t _gs_gui_header(gs_gui_context_t *ctx, const char *label, int32_t istreenode, int32_t opt) 
{
	gs_gui_rect_t r;
	int32_t active, expanded;
	gs_gui_id id = gs_gui_get_id(ctx, label, strlen(label));
	int32_t idx = gs_gui_pool_get(ctx, ctx->treenode_pool, GS_GUI_TREENODEPOOL_SIZE, id);
	int32_t width = -1;
	gs_gui_layout_row(ctx, 1, &width, 0);

	active = (idx >= 0);
	expanded = (opt & GS_GUI_OPT_EXPANDED) ? !active : active;
	r = gs_gui_layout_next(ctx);
	gs_gui_update_control(ctx, id, r, 0);

	/* handle click */
	active ^= (ctx->mouse_pressed == GS_GUI_MOUSE_LEFT && ctx->focus == id);

	/* update pool ref */
	if (idx >= 0) 
    {
		if (active) 
        { gs_gui_pool_update(ctx, ctx->treenode_pool, idx); 
        } 
		else 
        { 
            memset(&ctx->treenode_pool[idx], 0, sizeof(gs_gui_pool_item_t)); 
        }

	} 
    else if (active) 
    {
		gs_gui_pool_init(ctx, ctx->treenode_pool, GS_GUI_TREENODEPOOL_SIZE, id);
	}

	/* draw */
	if (istreenode) 
    {
		if (ctx->hover == id) 
        { 
            ctx->draw_frame(ctx, r, GS_GUI_COLOR_BUTTONHOVER); 
        } 
	} 
    else 
    {
		gs_gui_draw_control_frame(ctx, id, r, GS_GUI_COLOR_BUTTON, 0);
	}

    // Draw icon for tree node (make this a callback)
    /*
	gs_gui_draw_icon(
		ctx, expanded ? GS_GUI_ICON_EXPANDED : GS_GUI_ICON_COLLAPSED,
		gs_gui_rect(r.x, r.y, r.h, r.h), ctx->style->colors[GS_GUI_COLOR_TEXT]);
    */
    const float sz = 6.f;
    if (expanded)
    {
        gs_vec2 a = {r.x + sz / 2.f, r.y + (r.h - sz) / 2.f};
        gs_vec2 b = gs_vec2_add(a, gs_v2(sz, 0.f));
        gs_vec2 c = gs_vec2_add(a, gs_v2(sz / 2.f, sz));
        gs_gui_draw_triangle(ctx, a, b, c, ctx->style->colors[GS_GUI_COLOR_TEXT]);
    }
    else
    {
        gs_vec2 a = {r.x + sz / 2.f, r.y + (r.h - sz) / 2.f};
        gs_vec2 b = gs_vec2_add(a, gs_v2(sz, sz / 2.f));
        gs_vec2 c = gs_vec2_add(a, gs_v2(0.f, sz));
        gs_gui_draw_triangle(ctx, a, b, c, ctx->style->colors[GS_GUI_COLOR_TEXT]);
    }

    // Draw text for treenode
	r.x += r.h - ctx->style->padding;
	r.w -= r.h - ctx->style->padding; 
	gs_gui_draw_control_text(ctx, label, r, GS_GUI_COLOR_TEXT, 0);

	return expanded ? GS_GUI_RES_ACTIVE : 0;
} 

GS_API_DECL int32_t gs_gui_header_ex(gs_gui_context_t *ctx, const char *label, int32_t opt) 
{
	return _gs_gui_header(ctx, label, 0, opt);
}

GS_API_DECL int32_t gs_gui_begin_treenode_ex(gs_gui_context_t *ctx, const char *label, int32_t opt) 
{
	int32_t res = _gs_gui_header(ctx, label, 1, opt);
	if (res & GS_GUI_RES_ACTIVE) 
    {
		gs_gui_get_layout(ctx)->indent += ctx->style->indent;
		gs_gui_stack_push(ctx->id_stack, ctx->last_id);
	}

	return res;
} 

GS_API_DECL void gs_gui_end_treenode(gs_gui_context_t *ctx) 
{
	gs_gui_get_layout(ctx)->indent -= ctx->style->indent;
	gs_gui_pop_id(ctx);
} 

// -1 for left, + 1 for right
GS_API_DECL void gs_gui_tab_item_swap(gs_gui_context_t* ctx, gs_gui_container_t* cnt, int32_t direction)
{
    gs_gui_tab_bar_t* tab_bar = gs_gui_get_tab_bar(ctx, cnt);
    if (!tab_bar) return; 
	
	int32_t item = (int32_t)cnt->tab_item;
    int32_t idx = gs_clamp(item + direction, 0, (int32_t)tab_bar->size - 1);

    gs_gui_container_t* scnt = tab_bar->items[idx].data;

    gs_gui_tab_item_t* cti = &tab_bar->items[cnt->tab_item]; 
    gs_gui_tab_item_t* sti = &tab_bar->items[idx];
    gs_gui_tab_item_t tmp = *cti;

    // Swap cti
    sti->data = cnt;
    cnt->tab_item = sti->idx; 

    // Swap sti
    cti->data = scnt;
    scnt->tab_item = cti->idx; 

    tab_bar->focus = sti->idx;
}

int32_t gs_gui_begin_window_ex(gs_gui_context_t* ctx, const char* title, gs_gui_rect_t rect, int32_t opt) 
{ 
	gs_gui_rect_t body;
	gs_gui_id id = gs_gui_get_id(ctx, title, strlen(title)); 
	gs_gui_container_t* cnt = gs_gui_get_container_ex(ctx, id, opt); 

	if (!cnt || !cnt->open) 
    {
        return 0;
    } 

	memcpy(cnt->name, title, 256);

    const int32_t title_max_size = 100;

    bool new_frame = cnt->frame != ctx->frame;

    const float split_size = GS_GUI_SPLIT_SIZE;

	gs_gui_stack_push(ctx->id_stack, id); 

    // Get splits
    gs_gui_split_t* split = gs_gui_get_split(ctx, cnt); 
    gs_gui_split_t* root_split = gs_gui_get_root_split(ctx, cnt); 

    // Cache rect
	if ((cnt->rect.w == 0.f || opt & GS_GUI_OPT_FORCESETRECT) && new_frame) 
    {
        if (opt & GS_GUI_OPT_FULLSCREEN)
        {
            gs_vec2 fb = gs_platform_framebuffer_sizev(ctx->window_hndl);
            cnt->rect = gs_gui_rect(0, 0, (uint32_t)fb.x, (uint32_t)fb.y);

            // Set root split rect size
            if (root_split)
            {
                root_split->rect = cnt->rect;
                gs_gui_update_split(ctx, root_split);
            } 
        }
        else
        {
            cnt->rect = rect;
        }
    }
	gs_gui_begin_root_container(ctx, cnt, opt);
	rect = body = cnt->rect;
    cnt->opt = opt;

    if (opt & GS_GUI_OPT_DOCKSPACE)
    {
        cnt->zindex = 0;
    }

    // Get root container
    gs_gui_container_t* root_cnt = gs_gui_get_root_container(ctx, cnt);

    // If parent cannot move/resize, set to this opt as well
    if (root_cnt->opt & GS_GUI_OPT_NOMOVE)
    {
        cnt->opt |= GS_GUI_OPT_NOMOVE;
    }

    if (root_cnt->opt & GS_GUI_OPT_NORESIZE)
    {
        cnt->opt |= GS_GUI_OPT_NORESIZE;
    } 

    // If in a tab view, then title has to be handled differently...
    gs_gui_tab_bar_t* tab_bar = cnt->tab_bar ? gs_slot_array_getp(ctx->tab_bars, cnt->tab_bar) : NULL; 
    gs_gui_tab_item_t* tab_item = tab_bar ? &tab_bar->items[cnt->tab_item] : NULL;

    if (tab_item && tab_item)
    {
        if (tab_bar->focus == tab_item->idx) 
        {
            cnt->flags |= GS_GUI_WINDOW_FLAGS_VISIBLE;
            cnt->opt &= !GS_GUI_OPT_NOINTERACT; 
            cnt->opt &= !GS_GUI_OPT_NOHOVER; 
        }
        else
        {
            cnt->flags &= ~GS_GUI_WINDOW_FLAGS_VISIBLE;
            cnt->opt |= GS_GUI_OPT_NOINTERACT; 
            cnt->opt |= GS_GUI_OPT_NOHOVER; 
        }
    } 

    bool in_root = false;

    // If hovered root is in the tab group and moused over, then is hovered 
    if (tab_bar)
    {
        for (uint32_t i = 0; i < tab_bar->size; ++i)
        {
            if (ctx->hover_root == (gs_gui_container_t*)tab_bar->items[i].data)
            {
                in_root = true;
                break;
            }
        }
    } 

    gs_gui_container_t* s_cnt = cnt;  
    if (tab_bar && split)
    {
        for (uint32_t i = 0; i < tab_bar->size; ++i)
        {
            if (((gs_gui_container_t*)tab_bar->items[i].data)->split)
            {
                s_cnt = tab_bar->items[i].data;
            }
        }
    } 

    // Do split size/position
    if (split)
    { 
        const gs_gui_rect_t* sr = &split->rect;
        const float ratio = split->ratio;
        const float shsz = split_size;
        const float omr = (1.f - ratio);

        switch (split->type)
        {
            case GS_GUI_SPLIT_LEFT:
            {
                if (split->children[GS_GUI_SPLIT_NODE_CHILD].container == s_cnt)
                { 
                    cnt->rect = gs_gui_rect(sr->x + shsz, sr->y + shsz, sr->w * ratio - 2.f * shsz, sr->h - 2.f * shsz); 
                }
                else                
                {
                    cnt->rect = gs_gui_rect(sr->x + sr->w * ratio + shsz, sr->y + shsz, sr->w * (1.f - ratio) - 2.f * shsz, sr->h - 2.f * shsz); 
                }

            } break;

            case GS_GUI_SPLIT_RIGHT:
            {
                if (split->children[GS_GUI_SPLIT_NODE_PARENT].container == s_cnt)
                { 
                    cnt->rect = gs_gui_rect(sr->x + shsz, sr->y + shsz, sr->w * (1.f - ratio) - 2.f * shsz, sr->h - 2.f * shsz); 
                }
                else                
                { 
                    cnt->rect = gs_gui_rect(sr->x + sr->w * (1.f - ratio) + shsz, sr->y + shsz, sr->w * ratio - 2.f * shsz, sr->h - 2.f * shsz); 
                }
            } break;

            case GS_GUI_SPLIT_TOP:
            {
                if (split->children[GS_GUI_SPLIT_NODE_CHILD].container == s_cnt)
                {
                    cnt->rect = gs_gui_rect(sr->x + shsz, sr->y + shsz, sr->w - 2.f * shsz, sr->h * ratio - 2.f * shsz); 
                }
                else                
                {
                    cnt->rect = gs_gui_rect(sr->x + shsz, sr->y + sr->h * ratio + shsz, sr->w - 2.f * shsz, sr->h * (1.f - ratio) - 2.f * shsz);
                } 
            } break;

            case GS_GUI_SPLIT_BOTTOM:
            { 
                if (split->children[GS_GUI_SPLIT_NODE_CHILD].container == s_cnt)
                {
                    cnt->rect = gs_gui_rect(sr->x + shsz, sr->y + sr->h * (1.f - ratio) + shsz, sr->w - 2.f * shsz, sr->h * (ratio) - 2.f * shsz);
                }
                else                
                {
                    cnt->rect = gs_gui_rect(sr->x + shsz, sr->y + shsz, sr->w - 2.f * shsz, sr->h * (1.f - ratio) - 2.f * shsz); 
                } 
            } break;
        }
    } 

    // Calculate movement
    if (~cnt->opt & GS_GUI_OPT_NOTITLE && new_frame)
    {
        gs_gui_rect_t* rp = root_split ? &root_split->rect : &cnt->rect; 

        // Cache rect
		gs_gui_rect_t tr = cnt->rect;
		tr.h = ctx->style->title_height;
        tr.x += split_size;
        gs_gui_id id = gs_gui_get_id(ctx, "!title", 6); 
        gs_gui_update_control(ctx, id, tr, opt); 

        // Need to move the entire thing
        if ((id == ctx->focus || id == ctx->hover) && ctx->mouse_down == GS_GUI_MOUSE_LEFT) 
        { 
            if (tab_bar)
            {
                ctx->next_focus_root = (gs_gui_container_t*)(tab_bar->items[tab_bar->focus].data);
                gs_gui_bring_to_front(ctx, tab_bar->items[tab_bar->focus].data);
            }
            else
            {
                ctx->next_focus_root = cnt; 
            }

            if (root_split)
            {
                gs_gui_request_t req = gs_default_val();
                req.type = GS_GUI_SPLIT_MOVE;
                req.split = root_split;
                gs_dyn_array_push(ctx->requests, req);
            }
			// Figure this out...
            else if (!tab_bar)
            {
                gs_gui_request_t req = gs_default_val();
                req.type = GS_GUI_CNT_MOVE;
                req.cnt = cnt;
                gs_dyn_array_push(ctx->requests, req);
            }
        }

        // Tab view
        int32_t tw = title_max_size;
        id = gs_gui_get_id(ctx, "!split_tab", 10);
        const float hp = 0.8f;
        tr.x += split_size;
        float h = tr.h * hp;
        float y = tr.y + tr.h * (1.f - hp); 

        // Will update tab bar rect size with parent window rect
        if (tab_item)
        {
            // Get tab bar
            gs_gui_rect_t* r = &tab_bar->rect;

            // Determine width
            int32_t tab_width = gs_min(r->w / (float)tab_bar->size, title_max_size); 
            tw = tab_item->zindex ? tab_width : tab_width + 1.f; 

            // Determine position (based on zindex and total width)
            float xoff = 0.f; //tab_item->zindex ? 2.f : 0.f;  
            tr.x = tab_bar->rect.x + tab_width * tab_item->zindex + xoff; 
        }

        gs_gui_rect_t r = gs_gui_rect(tr.x + split_size, y, tw, h); 

        gs_gui_update_control(ctx, id, r, opt); 

        // Need to move the entire thing
        if ((id == ctx->hover || id == ctx->focus) && ctx->mouse_down == GS_GUI_MOUSE_LEFT) 
        { 
            gs_gui_set_focus(ctx, id); 
            ctx->next_focus_root = cnt; 

            // Don't move from tab bar
            if (tab_item)
            { 
                // Handle break out
                if (ctx->mouse_pos.y < tr.y || ctx->mouse_pos.y > tr.y + tr.h)
                { 
                    ctx->undock_root = cnt;
                } 

                if (tab_bar->focus != tab_item->idx)
                {
                    gs_gui_request_t req = gs_default_val();
                    req.type = GS_GUI_CNT_FOCUS;
                    req.cnt = cnt;
                    gs_dyn_array_push(ctx->requests, req);
                } 
            } 

            else if (root_split)
            {
                // Handle break out
                if (ctx->mouse_pos.y < tr.y || ctx->mouse_pos.y > tr.y + tr.h)
                { 
                    ctx->undock_root = cnt;
                } 
            }
            else
            {
                gs_gui_request_t req = gs_default_val();
                req.type = GS_GUI_CNT_MOVE;
                req.cnt = cnt;
                gs_dyn_array_push(ctx->requests, req);
            }
        }
    } 

    // Control frame for body movement
    if (
        ~root_cnt->opt & GS_GUI_OPT_NOMOVE &&
        ~cnt->opt & GS_GUI_OPT_NOMOVE && 
        ~cnt->opt & GS_GUI_OPT_NOINTERACT && 
        ~cnt->opt & GS_GUI_OPT_NOHOVER && 
        new_frame && 
        cnt->flags & GS_GUI_WINDOW_FLAGS_VISIBLE
    )
    { 
        // Cache rect
		gs_gui_rect_t br = cnt->rect;

        if (~cnt->opt & GS_GUI_OPT_NOTITLE) 
        {
            br.y += ctx->style->title_height;
            br.h -= ctx->style->title_height;
        }
        gs_gui_id id = gs_gui_get_id(ctx, "!body", 5);
        gs_gui_update_control(ctx, id, br, opt); 

        // Need to move the entire thing
        if ((id == ctx->focus) && ctx->mouse_down == GS_GUI_MOUSE_LEFT) 
        {
            ctx->next_focus_root = cnt; 
            if (root_split)
            {
                gs_gui_request_t req = gs_default_val();
                req.type = GS_GUI_SPLIT_MOVE;
                req.split = root_split;
                gs_dyn_array_push(ctx->requests, req);
            }
            else if (tab_bar) 
            { 
                gs_gui_request_t req = gs_default_val();
                req.type = GS_GUI_CNT_FOCUS;
                req.cnt = cnt;
                gs_dyn_array_push(ctx->requests, req);

                req.type = GS_GUI_CNT_MOVE;
                req.cnt = cnt;
                gs_dyn_array_push(ctx->requests, req);
            } 
            else
            {
                gs_gui_request_t req = gs_default_val();
                req.type = GS_GUI_CNT_MOVE;
                req.cnt = cnt;
                gs_dyn_array_push(ctx->requests, req);
            }
        }
    } 

    // Get parent window if in tab view, then set rect to it (will be a frame off though...)
    if (tab_item && tab_bar)
    {
        if (tab_bar->focus == tab_item->idx || split) 
        {
            tab_bar->rect = cnt->rect;
        }
        else
        {
            cnt->rect = tab_bar->rect;
        }
    }

    // Cache body 
    body = cnt->rect; 

    if (split)
    {
        const float sh = split_size * 0.5f; 
        // body.y += sh;
        // body.h -= split_size;
    }

    if (~opt & GS_GUI_OPT_NOTITLE)
    {
		gs_gui_rect_t tr = cnt->rect;
		tr.h = ctx->style->title_height;
        if (split)
        {
            const float sh = split_size * 0.5f; 
            // tr.x += sh;
            // tr.w -= split_size;
            // tr.y += sh;
        }
        body.y += tr.h;
        body.h -= tr.h; 
    } 

	// draw body frame
	if (~opt & GS_GUI_OPT_NOFRAME && cnt->flags & GS_GUI_WINDOW_FLAGS_VISIBLE) 
    {
		ctx->draw_frame(ctx, body, GS_GUI_COLOR_WINDOWBG);
	} 

    if (split && ~opt & GS_GUI_OPT_NOCLIP && cnt->flags & GS_GUI_WINDOW_FLAGS_VISIBLE)
    {
        gs_gui_push_clip_rect(ctx, gs_gui_expand_rect(cnt->rect, 1));
    } 

    if (split)
    {
        const float sh = split_size * 0.5f; 
        body.x += sh;
        body.w -= split_size;
    }

	// do title bar
	if (~opt & GS_GUI_OPT_NOTITLE) 
    {
		gs_gui_rect_t tr = cnt->rect;
		tr.h = ctx->style->title_height;
        if (split)
        {
            const float sh = split_size * 0.5f; 
            // tr.y += sh;
        }

        // Don't draw this unless you're the bottom window or first frame in a tab group (if in dockspace)
        if (tab_bar)
        {
            bool lowest = true;
            {
                for (uint32_t i = 0; i < tab_bar->size; ++i)
                { 
                    if (cnt->zindex > ((gs_gui_container_t*)(tab_bar->items[i].data))->zindex) 
                    {
                        lowest = false;
                        break;
                    }
                }
                if (lowest) 
                {
                    ctx->draw_frame(ctx, tr, GS_GUI_COLOR_TITLEBG);
		            // gs_gui_draw_box(ctx, gs_gui_expand_rect(tr, 1), ctx->style->colors[GS_GUI_COLOR_BORDER]);
                }
            }
        }

        else
        {
            ctx->draw_frame(ctx, tr, GS_GUI_COLOR_TITLEBG); 
		    // gs_gui_draw_box(ctx, gs_gui_expand_rect(tr, 1), ctx->style->colors[GS_GUI_COLOR_BORDER]);
        }

        // Draw tab control
        { 

            // Tab view
            int32_t tw = title_max_size;
            id = gs_gui_get_id(ctx, "!split_tab", 10);
            const float hp = 0.8f;
            tr.x += split_size;
            float h = tr.h * hp;
            float y = tr.y + tr.h * (1.f - hp); 

            // Will update tab bar rect size with parent window rect
            if (tab_item)
            {
                // Get tab bar
                gs_gui_rect_t* r = &tab_bar->rect;

                // Determine width
                int32_t tab_width = gs_min(r->w / (float)tab_bar->size, title_max_size); 
                tw = tab_width - 2.f;

                // Determine position (based on zindex and total width)
                float xoff = !tab_item->zindex ? split_size : 2.f; //tab_item->zindex ? 2.f : 0.f;  
                tr.x = tab_bar->rect.x + tab_width * tab_item->zindex + xoff;
            }

			gs_gui_rect_t r = gs_gui_rect(tr.x + split_size, y, tw, h);

            bool hovered = false;

            if (in_root && gs_gui_rect_overlaps_vec2(r, ctx->mouse_pos))
            {
                gs_immediate_draw_t* dl = &ctx->overlay_draw_list;
                // gsi_rectvd(dl, gs_v2(r.x, r.y), gs_v2(r.w, r.h), gs_v2s(0.f), gs_v2s(1.f), GS_COLOR_WHITE, GS_GRAPHICS_PRIMITIVE_LINES);
                hovered = true;
            }

			bool other_root_active = ctx->focus_root != cnt;
			if (tab_bar)
			{
				for (uint32_t i = 0; i < tab_bar->size; ++i)
				{
					if (tab_bar->items[i].data == ctx->focus_root)
					{ 
						other_root_active = false;
					}
				}
			}

            if (!other_root_active && hovered && ctx->mouse_down == GS_GUI_MOUSE_LEFT && !ctx->lock_focus)
            { 
				// This is an issue...
                gs_gui_set_focus(ctx, id);
				ctx->lock_focus = id;

                if (tab_item && tab_bar->focus != tab_item->idx)
                { 
                    gs_gui_request_t req = gs_default_val();
                    req.type = GS_GUI_CNT_FOCUS;
                    req.cnt = cnt;
                    gs_dyn_array_push(ctx->requests, req);
                } 
            }

			if (!other_root_active && ctx->mouse_down == GS_GUI_MOUSE_LEFT && ctx->focus == id)
			{
				if (ctx->mouse_pos.x < r.x)
				{ 
					gs_gui_request_t req = gs_default_val();
					req.type = GS_GUI_TAB_SWAP_LEFT;
					req.cnt = cnt;
					gs_dyn_array_push(ctx->requests, req);
				}
				if (ctx->mouse_pos.x > r.x + r.w)
				{ 
					gs_gui_request_t req = gs_default_val();
					req.type = GS_GUI_TAB_SWAP_RIGHT;
					req.cnt = cnt;
					gs_dyn_array_push(ctx->requests, req);
				}
			}

            bool tab_focus = (!tab_bar || (tab_bar && tab_item && tab_bar->focus == tab_item->idx));

            gs_color_t def = ctx->style->colors[GS_GUI_COLOR_BUTTON]; // gs_color(50, 50, 50, 255);
            gs_color_t hov = ctx->style->colors[GS_GUI_COLOR_BUTTONHOVER]; // gs_color(70, 70, 70, 255);
            gs_color_t act = ctx->style->colors[GS_GUI_COLOR_BUTTONFOCUS]; // GS_COLOR_GREEN; // gs_color(90, 90, 90, 255); 
            gs_color_t inactive = gs_color(10, 10, 10, 50);

			gs_gui_push_clip_rect(ctx, gs_gui_expand_rect(cnt->rect, 1));

            gs_gui_push_clip_rect(ctx, r);

            gs_gui_draw_rect(ctx, r, id == ctx->focus ? act : hovered ? hov : tab_focus ? def : inactive); 
            gs_gui_draw_control_text(ctx, title, tr, tab_focus ? GS_GUI_COLOR_TITLETEXT : GS_GUI_COLOR_TEXT_INACTIVE, opt); 

            gs_gui_pop_clip_rect(ctx); 
			gs_gui_pop_clip_rect(ctx);
        } 

		// do `close` button
        /*
		if (~opt & GS_GUI_OPT_NOCLOSE && false) 
        {
			gs_gui_id id = gs_gui_get_id(ctx, "!close", 6);
			gs_gui_rect_t r = gs_gui_rect(tr.x + tr.w - tr.h, tr.y, tr.h, tr.h);
			tr.w -= r.w;
			gs_gui_draw_icon(ctx, GS_GUI_ICON_CLOSE, r, ctx->style->colors[GS_GUI_COLOR_TITLETEXT]);
			gs_gui_update_control(ctx, id, r, opt);
			if (ctx->mouse_pressed == GS_GUI_MOUSE_LEFT && id == ctx->focus) 
            {
				cnt->open = 0;
			}
		} 
        */
	} 

	// resize to content size
	if (opt & GS_GUI_OPT_AUTOSIZE && !split) 
    {
		gs_gui_rect_t r = gs_gui_get_layout(ctx)->body;
		cnt->rect.w = cnt->content_size.x + (cnt->rect.w - r.w);
		cnt->rect.h = cnt->content_size.y + (cnt->rect.h - r.h);
	} 

    if (split && ~opt & GS_GUI_OPT_NOCLIP && cnt->flags & GS_GUI_WINDOW_FLAGS_VISIBLE)
    {
        gs_gui_pop_clip_rect(ctx); 
    }

    // Draw border
	if (~opt & GS_GUI_OPT_NOFRAME && cnt->flags & GS_GUI_WINDOW_FLAGS_VISIBLE) 
    {
		gs_gui_draw_box(ctx, gs_gui_expand_rect(cnt->rect, 1), ctx->style->colors[GS_GUI_COLOR_BORDER]); 
	}

    gs_gui_push_container_body(ctx, cnt, body, opt);

	/* close if this is a popup window and elsewhere was clicked */
	if (opt & GS_GUI_OPT_POPUP && ctx->mouse_pressed && ctx->hover_root != cnt) 
    {
		cnt->open = 0;
	} 

    if (~opt & GS_GUI_OPT_NOCLIP)
    {
        if (cnt->flags & GS_GUI_WINDOW_FLAGS_VISIBLE)
        {
	        gs_gui_push_clip_rect(ctx, cnt->body);
        }
        else 
        {
	        gs_gui_push_clip_rect(ctx, gs_gui_rect(0, 0, 0, 0));
        }
    }

	return GS_GUI_RES_ACTIVE;
} 

GS_API_DECL void gs_gui_end_window(gs_gui_context_t *ctx) 
{ 
    gs_gui_container_t* cnt = gs_gui_get_current_container(ctx); 

    // Get root container
    gs_gui_container_t* root_cnt = gs_gui_get_root_container(ctx, cnt);

    // Get splits
    gs_gui_split_t* split = gs_gui_get_split(ctx, cnt);
    gs_gui_split_t* root_split = gs_gui_get_root_split(ctx, cnt); 

    const bool new_frame = cnt->frame != ctx->frame;

    // Cache opt
    const int32_t opt = cnt->opt; 

    // Pop clip for rect
    if (~cnt->opt & GS_GUI_OPT_NOCLIP) 
    {
        gs_gui_pop_clip_rect(ctx); 
    } 

    if (~cnt->opt & GS_GUI_OPT_NOCLIP) 
    {
        gs_gui_push_clip_rect(ctx, cnt->rect); 
    } 

    // do `resize` handle
    if (~cnt->opt & GS_GUI_OPT_NORESIZE && ~root_cnt->opt & GS_GUI_OPT_NORESIZE && new_frame && ~cnt->opt & GS_GUI_OPT_DOCKSPACE) 
    {
        int32_t sz = ctx->style->title_height;
        gs_gui_id id = gs_gui_get_id(ctx, "!resize", 7);
        gs_gui_rect_t r = gs_gui_rect(cnt->rect.x + cnt->rect.w - sz, cnt->rect.y + cnt->rect.h - sz, sz, sz);
        gs_gui_update_control(ctx, id, r, opt);
        if (id == ctx->focus && ctx->mouse_down == GS_GUI_MOUSE_LEFT) 
        { 
            if (root_split)
            {
                gs_gui_request_t req = gs_default_val();
                req.type = GS_GUI_SPLIT_RESIZE_SE;
                req.split = root_split;
                gs_dyn_array_push(ctx->requests, req);
            }
            else
            {
                cnt->rect.w = gs_max(96, cnt->rect.w + ctx->mouse_delta.x);
                cnt->rect.h = gs_max(64, cnt->rect.h + ctx->mouse_delta.y);
            }
        }

        // Draw resize icon (this will also be a callback)
        const uint32_t grid = 8;
        const float w = r.w / (float)grid;
        const float h = r.h / (float)grid;
        const float m = 1.f;
        const float o = 5.f;

        gs_color_t col = ctx->focus == id ? ctx->style->colors[GS_GUI_COLOR_BUTTONFOCUS] : ctx->hover == id ? ctx->style->colors[GS_GUI_COLOR_BUTTONHOVER] : ctx->style->colors[GS_GUI_COLOR_BUTTON]; 

        gs_gui_draw_rect(ctx, gs_gui_rect(r.x + w * grid - o, r.y + h * (grid - 2) - o, w - m, h - m), col);
        gs_gui_draw_rect(ctx, gs_gui_rect(r.x + w * grid - o, r.y + h * (grid - 1) - o, w - m, h - m), col);
        gs_gui_draw_rect(ctx, gs_gui_rect(r.x + w * (grid - 1) - o, r.y + h * (grid - 1) - o, w - m, h - m), col);
        gs_gui_draw_rect(ctx, gs_gui_rect(r.x + w * grid - o, r.y + h * grid - o, w - m, h - m), col);
        gs_gui_draw_rect(ctx, gs_gui_rect(r.x + w * (grid - 1) - o, r.y + h * grid - o, w - m, h - m), col);
        gs_gui_draw_rect(ctx, gs_gui_rect(r.x + w * (grid - 2) - o, r.y + h * grid - o, w - m, h - m), col);
    } 
    
    if (~cnt->opt & GS_GUI_OPT_NOCLIP) 
    {
        gs_gui_pop_clip_rect(ctx); 
    } 

    // draw shadow
	if (~opt & GS_GUI_OPT_NOFRAME && cnt->flags & GS_GUI_WINDOW_FLAGS_VISIBLE) 
    {
        gs_gui_rect_t* r = &cnt->rect; 
        uint32_t ssz = split ? GS_GUI_SPLIT_SIZE : 5;

        gs_gui_draw_rect(ctx, gs_gui_rect(r->x, r->y + r->h, r->w + 1, 1), ctx->style->colors[GS_GUI_COLOR_SHADOW]);
        gs_gui_draw_rect(ctx, gs_gui_rect(r->x, r->y + r->h, r->w + ssz, ssz), ctx->style->colors[GS_GUI_COLOR_SHADOW]);
        gs_gui_draw_rect(ctx, gs_gui_rect(r->x + r->w, r->y, 1, r->h), ctx->style->colors[GS_GUI_COLOR_SHADOW]);
        gs_gui_draw_rect(ctx, gs_gui_rect(r->x + r->w, r->y, ssz, r->h), ctx->style->colors[GS_GUI_COLOR_SHADOW]);
	} 

    #define _gui_window_resize_ctrl(ID, RECT, MOUSE, SPLIT_TYPE, MOD_KEY, ...)\
        do {\
            if (ctx->key_down == (MOD_KEY))\
            {\
                gs_gui_id _ID = (ID);\
                gs_gui_rect_t _R = (RECT);\
                gs_gui_update_control(ctx, (ID), _R, opt);\
        \
                if (_ID == ctx->hover|| _ID == ctx->focus)\
                {\
                    gs_gui_draw_rect(ctx, _R, GS_COLOR_WHITE);\
                }\
        \
                if (_ID == ctx->focus && ctx->mouse_down == (MOUSE))\
                {\
                    gs_gui_draw_rect(ctx, _R, GS_COLOR_WHITE);\
                    if (root_split)\
                    {\
                        gs_gui_request_t req = gs_default_val();\
                        req.type = (SPLIT_TYPE);\
                        req.split = root_split;\
                        gs_dyn_array_push(ctx->requests, req);\
                    }\
                    else if (new_frame)\
                    {\
                        __VA_ARGS__\
                    }\
                }\
            }\
        } while (0)\

    // Control frame for body resize
    if (~opt & GS_GUI_OPT_NORESIZE && cnt->flags & GS_GUI_WINDOW_FLAGS_VISIBLE)
    { 
        // Cache main rect
        gs_gui_rect_t* r = root_split ? &root_split->rect : &cnt->rect;
        gs_gui_rect_t* cr = &cnt->rect;

        const float border_ratio = 0.333f;

        if (split) 
        {
            _gui_window_resize_ctrl(
                gs_gui_get_id(ctx, "!split_w", 8), 
                gs_gui_rect(cr->x, cr->y + cr->h * border_ratio, cr->w * border_ratio, cr->h * (1.f - 2.f * border_ratio)), 
                GS_GUI_MOUSE_RIGHT,
                GS_GUI_SPLIT_RESIZE_INVALID, 
                GS_GUI_KEY_CTRL,
                { 
                }); 

            _gui_window_resize_ctrl(
                gs_gui_get_id(ctx, "!split_e", 8), 
                gs_gui_rect(cr->x + cr->w * (1.f - border_ratio), cr->y + cr->h * border_ratio, cr->w * border_ratio, cr->h * (1.f - 2.f * border_ratio)), 
                GS_GUI_MOUSE_LEFT,
                GS_GUI_SPLIT_RESIZE_INVALID, 
                GS_GUI_KEY_CTRL,
                { 
                }); 

            _gui_window_resize_ctrl(
                gs_gui_get_id(ctx, "!split_n", 8), 
                gs_gui_rect(cr->x + cr->w * border_ratio, cr->y, cr->w * (1.f - 2.f * border_ratio), cr->h * border_ratio), 
                GS_GUI_MOUSE_LEFT,
                GS_GUI_SPLIT_RESIZE_INVALID, 
                GS_GUI_KEY_CTRL,
                { 
                }); 

            _gui_window_resize_ctrl(
                gs_gui_get_id(ctx, "!split_s", 8), 
                gs_gui_rect(cr->x + cr->w * border_ratio, cr->y + cr->h * (1.f - border_ratio), cr->w * (1.f - 2.f * border_ratio), cr->h * border_ratio), 
                GS_GUI_MOUSE_LEFT,
                GS_GUI_SPLIT_RESIZE_INVALID, 
                GS_GUI_KEY_CTRL,
                { 
                }); 

            _gui_window_resize_ctrl(
                gs_gui_get_id(ctx, "!split_se", 9), 
                gs_gui_rect(cr->x + cr->w - cr->w * border_ratio, cr->y + cr->h * (1.f - border_ratio), cr->w * border_ratio, cr->h * border_ratio), 
                GS_GUI_MOUSE_LEFT,
                GS_GUI_SPLIT_RESIZE_INVALID, 
                GS_GUI_KEY_CTRL,
                { 
                }); 

            _gui_window_resize_ctrl(
                gs_gui_get_id(ctx, "!split_ne", 9), 
                gs_gui_rect(cr->x + cr->w - cr->w * border_ratio, cr->y, cr->w * border_ratio, cr->h * border_ratio), 
                GS_GUI_MOUSE_LEFT,
                GS_GUI_SPLIT_RESIZE_INVALID, 
                GS_GUI_KEY_CTRL,
                { 
                }); 

            _gui_window_resize_ctrl(
                gs_gui_get_id(ctx, "!split_nw", 9), 
                gs_gui_rect(cr->x, cr->y, cr->w * border_ratio, cr->h * border_ratio), 
                GS_GUI_MOUSE_LEFT,
                GS_GUI_SPLIT_RESIZE_INVALID, 
                GS_GUI_KEY_CTRL,
                { 
                }); 

            _gui_window_resize_ctrl(
                gs_gui_get_id(ctx, "!split_sw", 9), 
                gs_gui_rect(cr->x, cr->y + cr->h - cr->h * border_ratio, cr->w * border_ratio, cr->h * border_ratio), 
                GS_GUI_MOUSE_LEFT,
                GS_GUI_SPLIT_RESIZE_INVALID, 
                GS_GUI_KEY_CTRL,
                { 
                }); 
        } 

        _gui_window_resize_ctrl(
            gs_gui_get_id(ctx, "!res_w", 6), 
            gs_gui_rect(r->x, r->y + r->h * border_ratio, r->w * border_ratio, r->h * (1.f - 2.f * border_ratio)), 
            GS_GUI_MOUSE_LEFT,
            GS_GUI_SPLIT_RESIZE_W, 
            GS_GUI_KEY_ALT,
            { 
                float w = r->w;
                float max_x = r->x + r->w;
                r->w = gs_max(r->w - ctx->mouse_delta.x, 40); 
                if (fabsf(r->w - w) > 0.f)
                {
                    r->x = gs_min(r->x + ctx->mouse_delta.x, max_x);
                }
            }); 

        _gui_window_resize_ctrl(
            gs_gui_get_id(ctx, "!res_e", 6), 
            gs_gui_rect(r->x + r->w * (1.f - border_ratio), r->y + r->h * border_ratio, r->w * border_ratio, r->h * (1.f - 2.f * border_ratio)), 
            GS_GUI_MOUSE_LEFT,
            GS_GUI_SPLIT_RESIZE_E, 
            GS_GUI_KEY_ALT,
            { 
                r->w = gs_max(r->w + ctx->mouse_delta.x, 40);
            }); 

        _gui_window_resize_ctrl(
            gs_gui_get_id(ctx, "!res_n", 6), 
            gs_gui_rect(r->x + r->w * border_ratio, r->y, r->w * (1.f - 2.f * border_ratio), r->h * border_ratio), 
            GS_GUI_MOUSE_LEFT,
            GS_GUI_SPLIT_RESIZE_N, 
            GS_GUI_KEY_ALT,
            { 
                float h = r->h;
                float max_y = h + r->y;
                r->h = gs_max(r->h - ctx->mouse_delta.y, 40); 
                if (fabsf(r->h - h) > 0.f)
                {
                    r->y = gs_min(r->y + ctx->mouse_delta.y, max_y);
                }
            }); 

        _gui_window_resize_ctrl(
            gs_gui_get_id(ctx, "!res_s", 6), 
            gs_gui_rect(r->x + r->w * border_ratio, r->y + r->h * (1.f - border_ratio), r->w * (1.f - 2.f * border_ratio), r->h * border_ratio), 
            GS_GUI_MOUSE_LEFT,
            GS_GUI_SPLIT_RESIZE_S, 
            GS_GUI_KEY_ALT,
            { 
                r->h = gs_max(r->h + ctx->mouse_delta.y, 40);
            }); 

        _gui_window_resize_ctrl(
            gs_gui_get_id(ctx, "!res_se", 7), 
            gs_gui_rect(r->x + r->w - r->w * border_ratio, r->y + r->h * (1.f - border_ratio), r->w * border_ratio, r->h * border_ratio), 
            GS_GUI_MOUSE_LEFT,
            GS_GUI_SPLIT_RESIZE_SE, 
            GS_GUI_KEY_ALT,
            { 
                r->w = gs_max(r->w + ctx->mouse_delta.x, 40);
                r->h = gs_max(r->h + ctx->mouse_delta.y, 40);
            }); 

        _gui_window_resize_ctrl(
            gs_gui_get_id(ctx, "!res_ne", 7), 
            gs_gui_rect(r->x + r->w - r->w * border_ratio, r->y, r->w * border_ratio, r->h * border_ratio), 
            GS_GUI_MOUSE_LEFT,
            GS_GUI_SPLIT_RESIZE_NE, 
            GS_GUI_KEY_ALT,
            { 
                r->w = gs_max(r->w + ctx->mouse_delta.x, 40); 
                float h = r->h;
                float max_y = h + r->y;
                r->h = gs_max(r->h - ctx->mouse_delta.y, 40); 
                if (fabsf(r->h - h) > 0.f)
                {
                    r->y = gs_min(r->y + ctx->mouse_delta.y, max_y);
                }
            }); 

        _gui_window_resize_ctrl(
            gs_gui_get_id(ctx, "!res_nw", 7), 
            gs_gui_rect(r->x, r->y, r->w * border_ratio, r->h * border_ratio), 
            GS_GUI_MOUSE_LEFT,
            GS_GUI_SPLIT_RESIZE_NW, 
            GS_GUI_KEY_ALT,
            { 
                float h = r->h;
                float max_y = h + r->y;
                r->h = gs_max(r->h - ctx->mouse_delta.y, 40); 
                if (fabsf(r->h - h) > 0.f)
                {
                    r->y = gs_min(r->y + ctx->mouse_delta.y, max_y);
                }

                float w = r->w;
                float max_x = r->x + r->w;
                r->w = gs_max(r->w - ctx->mouse_delta.x, 40); 
                if (fabsf(r->w - w) > 0.f)
                {
                    r->x = gs_min(r->x + ctx->mouse_delta.x, max_x);
                }
            }); 

        _gui_window_resize_ctrl(
            gs_gui_get_id(ctx, "!res_sw", 7), 
            gs_gui_rect(r->x, r->y + r->h - r->h * border_ratio, r->w * border_ratio, r->h * border_ratio), 
            GS_GUI_MOUSE_LEFT,
            GS_GUI_SPLIT_RESIZE_SW, 
            GS_GUI_KEY_ALT,
            { 
                float h = r->h;
                float max_y = h + r->y;
                r->h = gs_max(r->h + ctx->mouse_delta.y, 40); 

                float w = r->w;
                float max_x = r->x + r->w;
                r->w = gs_max(r->w - ctx->mouse_delta.x, 40); 
                if (fabsf(r->w - w) > 0.f)
                {
                    r->x = gs_min(r->x + ctx->mouse_delta.x, max_x);
                }
            }); 

        static bool capture = false;
        static gs_vec2 mp = {0};
        static gs_gui_rect_t _rect = {0};

        /*
        _gui_window_resize_ctrl(
            gs_gui_get_id(ctx, "!res_c", 5), 
            gs_gui_rect(r->x + r->w * border_ratio, r->y + r->h * border_ratio, r->w * border_ratio, r->h * border_ratio), 
            GS_GUI_SPLIT_RESIZE_CENTER, 
            { 
                if (!capture)
                {
                    capture = true;
                    mp = ctx->mouse_pos;
                    _rect = *r;
                }

                // Grow based on dist from center 
                gs_vec2 c = gs_v2(r->x + r->w * 0.5f, r->y + r->h * 0.5f); 
                gs_vec2 a = gs_vec2_sub(c, mp); 
                gs_vec2 b = gs_vec2_sub(c, ctx->mouse_pos); 
                gs_vec2 na = gs_vec2_norm(a);
                gs_vec2 nb = gs_vec2_norm(b);
                float dist = gs_vec2_len(gs_vec2_sub(b, a)); 
                float dot = gs_vec2_dot(na, nb);
                gs_println("len: %.2f, dot: %.2f", dist, dot);

                // Grow rect by dot product (scale dimensions) 
                float sign = dot >= 0.f ? 1.f : -1.f;
                float factor = 1.f - dist / 1000.f;
                r->w = _rect.w * factor * sign; 
                r->h = _rect.h * factor * sign; 

                // Equidistant resize from middle (grow rect based on delta)
                float h = r->h;
                float max_y = h + r->y;
                r->h = gs_max(r->h - ctx->mouse_delta.y, 40); 
                if (fabsf(r->h - h) > 0.f)
                {
                    r->y = gs_min(r->y - ctx->mouse_delta.y, max_y);
                }

                float w = r->w;
                float max_x = r->x + r->w;
                r->w = gs_max(r->w - ctx->mouse_delta.x, 40); 
                if (fabsf(r->w - w) > 0.f)
                {
                    r->x = gs_min(r->x - ctx->mouse_delta.x, max_x);
                }
            }); 
        */

        if (ctx->mouse_down != GS_GUI_MOUSE_LEFT)
        {
            capture = false;
            mp = gs_v2s(0.f);
        } 
    }

    // Determine if focus root in same tab group as current window for docking
    bool can_dock = true;
    if (cnt->tab_bar)
    {
        gs_gui_tab_bar_t* tab_bar = gs_slot_array_getp(ctx->tab_bars, cnt->tab_bar);
        for (uint32_t t = 0; t < tab_bar->size; ++t)
        {
            if (tab_bar->items[t].data == ctx->focus_root) 
            {
                can_dock = false;
            }
        }
    }

    // Do docking overlay (if enabled) 
    if (
        can_dock && 
        ~cnt->opt & GS_GUI_OPT_NODOCK &&
        ctx->focus_root && 
        ctx->focus_root != cnt &&  
        gs_gui_rect_overlaps_vec2(cnt->rect, ctx->mouse_pos) && // This is the incorrect part - need to check if this container isn't being overlapped by another
        ctx->mouse_down == GS_GUI_MOUSE_LEFT && 
		~cnt->opt & GS_GUI_OPT_NOHOVER && 
        cnt->flags & GS_GUI_WINDOW_FLAGS_VISIBLE) 
    { 
        gs_gui_split_t* focus_split = gs_gui_get_root_split(ctx, ctx->focus_root);
        gs_gui_split_t* cnt_split   = gs_gui_get_root_split(ctx, cnt); 

        // NOTE(john): this is incorrect...
        if ((!focus_split && !cnt_split) || ((focus_split || cnt_split) && (focus_split != cnt_split)))
        {
            // Set dockable root container
            ctx->dockable_root = ctx->dockable_root && cnt->zindex > ctx->dockable_root->zindex ? cnt : ctx->dockable_root? ctx->dockable_root : cnt;
        } 
    } 

    // Set current frame
    cnt->frame = ctx->frame;

    // Pop root container
	gs_gui_end_root_container(ctx); 
} 

GS_API_DECL void gs_gui_open_popup(gs_gui_context_t* ctx, const char* name) 
{
	gs_gui_container_t *cnt = gs_gui_get_container(ctx, name);

	// Set as hover root so popup isn't closed in begin_window_ex()
	ctx->hover_root = ctx->next_hover_root = cnt;

	// position at mouse cursor, open and bring-to-front
	cnt->rect = gs_gui_rect(ctx->mouse_pos.x, ctx->mouse_pos.y, 100, 100);
	cnt->open = 1;
	gs_gui_bring_to_front(ctx, cnt);
} 

GS_API_DECL int32_t gs_gui_begin_popup_ex(gs_gui_context_t* ctx, const char* name, gs_gui_rect_t r, int32_t opt)
{
	opt |= (GS_GUI_OPT_POPUP | GS_GUI_OPT_NODOCK | GS_GUI_OPT_CLOSED); 
	return gs_gui_begin_window_ex(ctx, name, r, opt);
} 

GS_API_DECL void gs_gui_end_popup(gs_gui_context_t *ctx) 
{
	gs_gui_end_window(ctx);
} 

GS_API_DECL void gs_gui_begin_panel_ex(gs_gui_context_t* ctx, const char* name, int32_t opt) 
{
	gs_gui_container_t *cnt;
	gs_gui_push_id(ctx, name, strlen(name));
	cnt = gs_gui_get_container_ex(ctx, ctx->last_id, opt);
	cnt->rect = gs_gui_layout_next(ctx);
	
    if (~opt & GS_GUI_OPT_NOFRAME) 
    {
		ctx->draw_frame(ctx, cnt->rect, GS_GUI_COLOR_PANELBG);
	}

	gs_gui_stack_push(ctx->container_stack, cnt);
	gs_gui_push_container_body(ctx, cnt, cnt->rect, opt);
	gs_gui_push_clip_rect(ctx, cnt->body);
} 

GS_API_DECL void gs_gui_end_panel(gs_gui_context_t *ctx) 
{
	gs_gui_pop_clip_rect(ctx);
	gs_gui_pop_container(ctx);
}

#endif // GS_GUI_IMPL 
#endif // GS_GUI_H















