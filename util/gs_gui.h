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

	Before including, define the gunslinger immediate gui implementation like this:

	    #define GS_GUI_IMPL

    in EXACTLY ONE C or C++ file that includes this header, BEFORE the
    include, like this:

		#define GS_GUI_IMPL
		#include "gs_gui.h"

    All other files should just #include "gs_gui.h" without the #define.

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

#define GS_GUI_SPLIT_SIZE           2.f 
#define GS_GUI_MAX_CNT              48 
#define GS_GUI_COMMANDLIST_SIZE		(256 * 1024)
#define GS_GUI_ROOTLIST_SIZE		32
#define GS_GUI_CONTAINERSTACK_SIZE	32
#define GS_GUI_CLIPSTACK_SIZE		32
#define GS_GUI_IDSTACK_SIZE			32
#define GS_GUI_LAYOUTSTACK_SIZE		16
#define GS_GUI_CONTAINERPOOL_SIZE	48
#define GS_GUI_TREENODEPOOL_SIZE	48
#define GS_GUI_GS_GUI_SPLIT_SIZE	32
#define GS_GUI_GS_GUI_TAB_SIZE		32
#define GS_GUI_MAX_WIDTHS			16
#define GS_GUI_REAL					float
#define GS_GUI_REAL_FMT				"%.3g"
#define GS_GUI_SLIDER_FMT			"%.2f"
#define GS_GUI_MAX_FMT				127
#define GS_GUI_TAB_ITEM_MAX         24 
#define GS_GUI_CLS_SELECTOR_MAX     4

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
	GS_GUI_COMMAND_CUSTOM,
	GS_GUI_COMMAND_MAX
};

enum {
    GS_GUI_SHAPE_RECT = 1,
    GS_GUI_SHAPE_CIRCLE, 
    GS_GUI_SHAPE_TRIANGLE,
    GS_GUI_SHAPE_LINE
};

enum {
    GS_GUI_TRANSFORM_WORLD = 0x00,
    GS_GUI_TRANSFORM_LOCAL
};

enum {
    GS_GUI_GIZMO_TRANSLATE = 0x00,
    GS_GUI_GIZMO_ROTATE, 
    GS_GUI_GIZMO_SCALE
};

enum { 
    GS_GUI_COLOR_BACKGROUND = 0x00,
    GS_GUI_COLOR_CONTENT,
	GS_GUI_COLOR_BORDER,
	GS_GUI_COLOR_SHADOW,
    GS_GUI_COLOR_CONTENT_BACKGROUND,
    GS_GUI_COLOR_CONTENT_SHADOW,
    GS_GUI_COLOR_CONTENT_BORDER,

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
    GS_GUI_OPT_NOBRINGTOFRONT = (1 << 22), 
    GS_GUI_OPT_LEFTCLICKONLY = (1 << 23),
    GS_GUI_OPT_NOSWITCHSTATE = (1 << 24),
    GS_GUI_OPT_NOBORDER      = (1 << 25),
    GS_GUI_OPT_ISCONTENT     = (1 << 26),
    GS_GUI_OPT_NOCARET       = (1 << 27),
    GS_GUI_OPT_NOSCROLLHORIZONTAL = (1 << 28),
    GS_GUI_OPT_NOSCROLLVERTICAL = (1 << 29)
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
typedef struct {gs_vec2 start; gs_vec2 end;} gs_gui_line_t;

typedef struct {gs_gui_id id; int32_t last_update;} gs_gui_pool_item_t; 

typedef struct 
{
    int32_t type, size;
} gs_gui_basecommand_t;

typedef struct 
{
    gs_gui_basecommand_t base; 
    void *dst;
} gs_gui_jumpcommand_t;

typedef struct 
{
    gs_gui_basecommand_t base; 
    gs_gui_rect_t rect;
} gs_gui_clipcommand_t;

typedef struct 
{
    gs_gui_basecommand_t base; 
    gs_asset_font_t* font; 
    gs_vec2 pos; 
    gs_color_t color; 
    char str[1];
} gs_gui_textcommand_t;

typedef struct 
{
    gs_gui_basecommand_t base; 
    gs_gui_rect_t rect; 
    gs_handle(gs_graphics_texture_t) hndl; 
    gs_vec4 uvs; 
    gs_color_t color;
} gs_gui_imagecommand_t;

struct gs_gui_customcommand_t;

// Draw Callback
typedef void (* gs_gui_draw_callback_t)(gs_gui_context_t* ctx, struct gs_gui_customcommand_t* cmd);

typedef struct gs_gui_customcommand_t
{
    gs_gui_basecommand_t base; 
    gs_gui_rect_t clip; 
    gs_gui_rect_t viewport;
    gs_gui_id hash;
    gs_gui_id hover;
    gs_gui_id focus;
    gs_gui_draw_callback_t cb;
    void* data;
    size_t sz;
} gs_gui_customcommand_t;

typedef struct 
{
    gs_gui_basecommand_t base; 
    uint32_t type;
    union
    {
        gs_gui_rect_t rect;
        gs_gui_circle_t circle;
        gs_gui_triangle_t triangle;
        gs_gui_line_t line;
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
	gs_gui_imagecommand_t image;
    gs_gui_customcommand_t custom;
} gs_gui_command_t; 

struct gs_gui_context_t;

typedef void (* gs_gui_on_draw_button_callback)(struct gs_gui_context_t* ctx, gs_gui_rect_t rect, gs_gui_id id, bool hovered, bool focused, int32_t opt, const char* label, int32_t icon);

typedef enum {
    GS_GUI_LAYOUT_ANCHOR_TOPLEFT = 0x00,
    GS_GUI_LAYOUT_ANCHOR_TOPCENTER,
    GS_GUI_LAYOUT_ANCHOR_TOPRIGHT,
    GS_GUI_LAYOUT_ANCHOR_LEFT,
    GS_GUI_LAYOUT_ANCHOR_CENTER,
    GS_GUI_LAYOUT_ANCHOR_RIGHT,
    GS_GUI_LAYOUT_ANCHOR_BOTTOMLEFT,
    GS_GUI_LAYOUT_ANCHOR_BOTTOMCENTER,
    GS_GUI_LAYOUT_ANCHOR_BOTTOMRIGHT
} gs_gui_layout_anchor_type; 

typedef enum {
    GS_GUI_ALIGN_START = 0x00,
    GS_GUI_ALIGN_CENTER,
    GS_GUI_ALIGN_END
} gs_gui_align_type;

typedef enum {
    GS_GUI_JUSTIFY_START = 0x00,
    GS_GUI_JUSTIFY_CENTER, 
    GS_GUI_JUSTIFY_END
} gs_gui_justification_type;

typedef enum {
    GS_GUI_DIRECTION_COLUMN = 0x00,
    GS_GUI_DIRECTION_ROW,
    GS_GUI_DIRECTION_COLUMN_REVERSE, 
    GS_GUI_DIRECTION_ROW_REVERSE
} gs_gui_direction;

typedef struct gs_gui_layout_t
{
	gs_gui_rect_t body;
	gs_gui_rect_t next;
	gs_vec2 position;
	gs_vec2 size;
	gs_vec2 max;
    int32_t padding[4];
	int32_t widths[GS_GUI_MAX_WIDTHS];
	int32_t items;
	int32_t item_index;
	int32_t next_row;
	int32_t next_type;
	int32_t indent; 

    // flex direction / justification / alignment
    int32_t direction; 
    int32_t justify_content;
    int32_t align_content;

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
    int32_t frame;
} gs_gui_split_t;

typedef enum gs_gui_window_flags { 
	GS_GUI_WINDOW_FLAGS_VISIBLE	    = (1 << 0),
	GS_GUI_WINDOW_FLAGS_FIRST_INIT	= (1 << 1)
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

typedef enum {
    GS_GUI_ELEMENT_STATE_NEG = -1,
    GS_GUI_ELEMENT_STATE_DEFAULT = 0x00,
    GS_GUI_ELEMENT_STATE_HOVER,
    GS_GUI_ELEMENT_STATE_FOCUS,
    GS_GUI_ELEMENT_STATE_COUNT,
    GS_GUI_ELEMENT_STATE_ON_HOVER,
    GS_GUI_ELEMENT_STATE_ON_FOCUS,
    GS_GUI_ELEMENT_STATE_OFF_HOVER,
    GS_GUI_ELEMENT_STATE_OFF_FOCUS
} gs_gui_element_state;

typedef enum gs_gui_element_type
{
    GS_GUI_ELEMENT_CONTAINER = 0x00,
    GS_GUI_ELEMENT_LABEL,
    GS_GUI_ELEMENT_TEXT,
    GS_GUI_ELEMENT_PANEL,
    GS_GUI_ELEMENT_INPUT,
    GS_GUI_ELEMENT_BUTTON,
    GS_GUI_ELEMENT_SCROLL,
    GS_GUI_ELEMENT_IMAGE,
    GS_GUI_ELEMENT_COUNT
} gs_gui_element_type; 

typedef enum {
    GS_GUI_PADDING_LEFT = 0x00,
    GS_GUI_PADDING_RIGHT, 
    GS_GUI_PADDING_TOP,
    GS_GUI_PADDING_BOTTOM
} gs_gui_padding_type;

typedef enum {
    GS_GUI_MARGIN_LEFT = 0x00,
    GS_GUI_MARGIN_RIGHT, 
    GS_GUI_MARGIN_TOP,
    GS_GUI_MARGIN_BOTTOM
} gs_gui_margin_type;

typedef enum {

    // Width/Height
    GS_GUI_STYLE_WIDTH = 0x00,
    GS_GUI_STYLE_HEIGHT,

    // Padding
    GS_GUI_STYLE_PADDING, 
    GS_GUI_STYLE_PADDING_LEFT, 
    GS_GUI_STYLE_PADDING_RIGHT, 
    GS_GUI_STYLE_PADDING_TOP, 
    GS_GUI_STYLE_PADDING_BOTTOM, 

    GS_GUI_STYLE_MARGIN,            // Can set margin for all at once, if -1.f then will assume 'auto' to simulate standard css
    GS_GUI_STYLE_MARGIN_LEFT, 
    GS_GUI_STYLE_MARGIN_RIGHT,
    GS_GUI_STYLE_MARGIN_TOP,
    GS_GUI_STYLE_MARGIN_BOTTOM,

    // Border Radius
    GS_GUI_STYLE_BORDER_RADIUS,
    GS_GUI_STYLE_BORDER_RADIUS_LEFT,
    GS_GUI_STYLE_BORDER_RADIUS_RIGHT,
    GS_GUI_STYLE_BORDER_RADIUS_TOP,
    GS_GUI_STYLE_BORDER_RADIUS_BOTTOM,

    // Border Width
    GS_GUI_STYLE_BORDER_WIDTH,
    GS_GUI_STYLE_BORDER_WIDTH_LEFT,
    GS_GUI_STYLE_BORDER_WIDTH_RIGHT,
    GS_GUI_STYLE_BORDER_WIDTH_TOP,
    GS_GUI_STYLE_BORDER_WIDTH_BOTTOM,

    // Text
    GS_GUI_STYLE_TEXT_ALIGN,

    // Flex
    GS_GUI_STYLE_DIRECTION,
    GS_GUI_STYLE_ALIGN_CONTENT,
    GS_GUI_STYLE_JUSTIFY_CONTENT,       // Justify runs parallel to direction (ex. for row, left to right)

    // Shadow
    GS_GUI_STYLE_SHADOW_X,
    GS_GUI_STYLE_SHADOW_Y,

    // Colors
    GS_GUI_STYLE_COLOR_BACKGROUND,
    GS_GUI_STYLE_COLOR_BORDER,
    GS_GUI_STYLE_COLOR_SHADOW,
    GS_GUI_STYLE_COLOR_CONTENT,
    GS_GUI_STYLE_COLOR_CONTENT_BACKGROUND,
    GS_GUI_STYLE_COLOR_CONTENT_BORDER,
    GS_GUI_STYLE_COLOR_CONTENT_SHADOW,

    // Font
    GS_GUI_STYLE_FONT,

	GS_GUI_STYLE_COUNT

} gs_gui_style_element_type;

enum {
    GS_GUI_ANIMATION_DIRECTION_FORWARD = 0x00,
    GS_GUI_ANIMATION_DIRECTION_BACKWARD
}; 
 
typedef struct {
    gs_gui_style_element_type type;
    union { 
        int32_t value;
        gs_color_t color;
        gs_asset_font_t* font;
    }; 
} gs_gui_style_element_t; 

typedef struct gs_gui_animation_t
{
    int16_t max;            // max time
    int16_t time;           // current time
    int16_t delay;          // delay
    int16_t curve;          // curve type
    int16_t direction;      // current direction
    int16_t playing;        // whether or not active
    int16_t iterations;     // number of iterations to play the animation
    int16_t focus_state;    // cached focus_state from frame (want to delete this somehow)
    int16_t hover_state;    // cached hover_state from frame (want to delete this somehow)
    int16_t start_state;    // starting state for animation blend
    int16_t end_state;      // ending state for animation blend
    int32_t frame;          // current frame (to match)
} gs_gui_animation_t; 

typedef struct gs_gui_style_t
{
    // font
    gs_asset_font_t* font;

    // dimensions
	float size[2];
	int16_t spacing;            // get rid of    (use padding)
	int16_t indent;             // get rid of    (use margin)
	int16_t title_height;       // get rid of    (use title_bar style)
	int16_t scrollbar_size;     // get rid of    (use scroll style)
	int16_t thumb_size;         // get rid of    (use various styles)

    // colors
	gs_color_t colors[GS_GUI_COLOR_MAX]; 

    // padding/margin
    int32_t padding[4];
    int16_t margin[4];

    // border
    int16_t border_width[4];
    int16_t border_radius[4];

    // flex direction / justification / alignment
    int16_t direction; 
    int16_t justify_content;
    int16_t align_content;

    // shadow amount each direction
    int16_t shadow_x;
    int16_t shadow_y;

} gs_gui_style_t;

// Keep animation properties lists within style sheet to look up

typedef struct gs_gui_animation_property_t
{
    gs_gui_style_element_type type;
    int16_t time;
    int16_t delay;
} gs_gui_animation_property_t;

typedef struct gs_gui_animation_property_list_t
{
    gs_dyn_array(gs_gui_animation_property_t) properties[3];
} gs_gui_animation_property_list_t; 

/* 
   element type
   classes
   id

   gs_gui_button(gui, "Text##.cls#id");
   gs_gui_label(gui, "Title###title");

    button .class #id : hover {         // All of these styles get concat into one?
    }
*/

typedef struct {
    gs_dyn_array(gs_gui_style_element_t) styles[3];
} gs_gui_style_list_t;

typedef struct gs_gui_style_sheet_t {
    gs_gui_style_t styles[GS_GUI_ELEMENT_COUNT][3]; // default | hovered | focused
    gs_hash_table(gs_gui_element_type, gs_gui_animation_property_list_t) animations;
    
    gs_hash_table(uint64_t, gs_gui_style_list_t) cid_styles;
    gs_hash_table(uint64_t, gs_gui_animation_property_list_t) cid_animations;
} gs_gui_style_sheet_t;

typedef struct gs_gui_style_sheet_element_desc_t {

    struct { 

        struct {
            gs_gui_style_element_t* data;
            size_t size;
        } style;

        struct {
            gs_gui_animation_property_t* data;
            size_t size;
        } animation;

    } all; 

    struct {

        struct {
            gs_gui_style_element_t* data;    
            size_t size; 
        } style;

        struct {
            gs_gui_animation_property_t* data;
            size_t size;
        } animation;

    } def;

    struct {
        struct {
            gs_gui_style_element_t* data;    
            size_t size; 
        } style;

        struct {
            gs_gui_animation_property_t* data;
            size_t size;
        } animation;
    } hover;

    struct {
        struct {
            gs_gui_style_element_t* data;    
            size_t size; 
        } style;

        struct {
            gs_gui_animation_property_t* data;
            size_t size;
        } animation;
    } focus; 

} gs_gui_style_sheet_element_desc_t;

typedef gs_gui_style_sheet_element_desc_t gs_gui_inline_style_desc_t;

typedef struct gs_gui_style_sheet_desc_t { 
    gs_gui_style_sheet_element_desc_t container;
    gs_gui_style_sheet_element_desc_t button;
    gs_gui_style_sheet_element_desc_t panel;
    gs_gui_style_sheet_element_desc_t input;
    gs_gui_style_sheet_element_desc_t text;
    gs_gui_style_sheet_element_desc_t label;
    gs_gui_style_sheet_element_desc_t scroll;
    gs_gui_style_sheet_element_desc_t tab;
    gs_gui_style_sheet_element_desc_t menu;
    gs_gui_style_sheet_element_desc_t title;
    gs_gui_style_sheet_element_desc_t image;
} gs_gui_style_sheet_desc_t; 

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

typedef struct gs_gui_inline_style_stack_t
{
    gs_dyn_array(gs_gui_style_element_t) styles[3];
    gs_dyn_array(gs_gui_animation_property_t) animations[3];
    gs_dyn_array(uint32_t) style_counts;                      // amount of styles to pop off at "top of stack" for each state
    gs_dyn_array(uint32_t) animation_counts;                  // amount of animations to pop off at "top of stack" for each state
} gs_gui_inline_style_stack_t;

typedef struct gs_gui_context_t 
{ 
	// Core state
	gs_gui_style_t* style;              // Active style
    gs_gui_style_sheet_t* style_sheet;  // Active style sheet
	gs_gui_id hover;
	gs_gui_id focus;
	gs_gui_id last_id;
    gs_gui_id state_switch_id;          // Id that had a state switch
    int32_t switch_state;
    gs_gui_id lock_focus;
    int32_t last_hover_state;
    int32_t last_focus_state;
    gs_gui_id prev_hover;
    gs_gui_id prev_focus;
	gs_gui_rect_t last_rect;
	int32_t last_zindex;
	int32_t updated_focus;
	int32_t frame;
    gs_vec2 framebuffer_size;
    gs_gui_container_t* active_root;
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
    gs_gui_alt_drag_mode_type alt_drag_mode;
    gs_dyn_array(gs_gui_request_t) requests;

	// Stacks
	gs_gui_stack(uint8_t, GS_GUI_COMMANDLIST_SIZE) command_list;
	gs_gui_stack(gs_gui_container_t*, GS_GUI_ROOTLIST_SIZE) root_list;
	gs_gui_stack(gs_gui_container_t*, GS_GUI_CONTAINERSTACK_SIZE) container_stack;
	gs_gui_stack(gs_gui_rect_t, GS_GUI_CLIPSTACK_SIZE) clip_stack;
	gs_gui_stack(gs_gui_id, GS_GUI_IDSTACK_SIZE) id_stack;
	gs_gui_stack(gs_gui_layout_t, GS_GUI_LAYOUTSTACK_SIZE) layout_stack; 

    // Style sheet element stacks
    gs_hash_table(gs_gui_element_type, gs_gui_inline_style_stack_t) inline_styles;

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

    // Active Transitions
    gs_hash_table(gs_gui_id, gs_gui_animation_t) animations;

    // Font stash
    gs_hash_table(uint64_t, gs_asset_font_t*) font_stash;

    // Callbacks
    struct {
        gs_gui_on_draw_button_callback button;
    } callbacks;

} gs_gui_context_t; 

typedef struct 
{ 
	// Core state
	gs_gui_style_t* style;              // Active style
    gs_gui_style_sheet_t* style_sheet;  // Active style sheet   
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
    gs_gui_alt_drag_mode_type alt_drag_mode;
    gs_dyn_array(gs_gui_request_t) requests;

	// Stacks
	gs_gui_stack(gs_gui_container_t*, GS_GUI_CONTAINERSTACK_SIZE) container_stack;

    gs_dyn_array(uint8_t) command_list;
    gs_dyn_array(gs_gui_container_t*) root_list;
    gs_dyn_array(gs_gui_rect_t) clip_stack;
    gs_dyn_array(gs_gui_id) id_stack;
    gs_dyn_array(gs_gui_layout_t) layout_stack;

	// Retained state pools
	gs_gui_pool_item_t container_pool[GS_GUI_CONTAINERPOOL_SIZE];
	gs_gui_pool_item_t treenode_pool[GS_GUI_TREENODEPOOL_SIZE];

    gs_slot_array(gs_gui_split_t) splits;
    gs_slot_array(gs_gui_tab_bar_t) tab_bars;

	// Input state
	gs_vec2 mouse_pos;
	gs_vec2 last_mouse_pos;
	gs_vec2 mouse_delta;
	gs_vec2 scroll_delta;
	int16_t mouse_down;
	int16_t mouse_pressed;
	int16_t key_down;
	int16_t key_pressed;
	char input_text[32]; 

    // Backend resources
    uint32_t window_hndl;
    gs_immediate_draw_t gsi;
    gs_immediate_draw_t overlay_draw_list;                                  

    // Active Transitions
    gs_hash_table(gs_gui_id, gs_gui_animation_t) animations;

    // Callbacks
    struct {
        gs_gui_on_draw_button_callback button;
    } callbacks;

} gs_gui_context_pruned_t; 

typedef struct {
    const char* key;
    gs_asset_font_t* font;
} gs_gui_font_desc_t;

typedef struct {
    gs_gui_font_desc_t* fonts;
    size_t size;
} gs_gui_font_stash_desc_t;

typedef struct
{
    const char* id;                                // Id selector
    const char* classes[GS_GUI_CLS_SELECTOR_MAX];  // Class selectors
} gs_gui_selector_desc_t;

GS_API_DECL gs_gui_rect_t gs_gui_rect(float x, float y, float w, float h);

//=== Context ===//

GS_API_DECL gs_gui_context_t gs_gui_new(uint32_t window_hndl);
GS_API_DECL void gs_gui_init(gs_gui_context_t *ctx, uint32_t window_hndl);
GS_API_DECL void gs_gui_init_font_stash(gs_gui_context_t *ctx, gs_gui_font_stash_desc_t* desc);
GS_API_DECL gs_gui_context_t gs_gui_context_new(uint32_t window_hndl);
GS_API_DECL void gs_gui_free(gs_gui_context_t* ctx); 
GS_API_DECL void gs_gui_begin(gs_gui_context_t *ctx, gs_vec2 framebuffer_size);
GS_API_DECL void gs_gui_end(gs_gui_context_t *ctx); 
GS_API_DECL void gs_gui_render(gs_gui_context_t* ctx, gs_command_buffer_t* cb);

//=== Util ===//
GS_API_DECL void gs_gui_renderpass_submit(gs_gui_context_t* ctx, gs_command_buffer_t* cb, gs_color_t clear); 
GS_API_DECL void gs_gui_renderpass_submit_ex(gs_gui_context_t* ctx, gs_command_buffer_t* cb, gs_graphics_clear_action_t* action);
GS_API_DECL void gs_gui_parse_id_tag(gs_gui_context_t* ctx, const char* str, char* buffer, size_t sz);
GS_API_DECL void gs_gui_parse_label_tag(gs_gui_context_t* ctx, const char* str, char* buffer, size_t sz);

//=== Main API ===//

GS_API_DECL void gs_gui_set_focus(gs_gui_context_t *ctx, gs_gui_id id);
GS_API_DECL void gs_gui_set_hover(gs_gui_context_t *ctx, gs_gui_id id);
GS_API_DECL gs_gui_id gs_gui_get_id(gs_gui_context_t *ctx, const void *data, int32_t size);
GS_API_DECL void gs_gui_push_id(gs_gui_context_t *ctx, const void *data, int32_t size);
GS_API_DECL void gs_gui_pop_id(gs_gui_context_t *ctx);
GS_API_DECL void gs_gui_push_clip_rect(gs_gui_context_t *ctx, gs_gui_rect_t rect);
GS_API_DECL void gs_gui_pop_clip_rect(gs_gui_context_t *ctx);
GS_API_DECL gs_gui_rect_t gs_gui_get_clip_rect(gs_gui_context_t *ctx);
GS_API_DECL int32_t gs_gui_check_clip(gs_gui_context_t *ctx, gs_gui_rect_t r);
GS_API_DECL int32_t gs_gui_mouse_over(gs_gui_context_t* ctx, gs_gui_rect_t rect);
GS_API_DECL void gs_gui_update_control(gs_gui_context_t* ctx, gs_gui_id id, gs_gui_rect_t rect, int32_t opt);

//=== Conatiner ===//

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

//=== Animation ===//

GS_API_DECL gs_gui_animation_t* gs_gui_get_animation(gs_gui_context_t* ctx, gs_gui_id id, 
        const gs_gui_selector_desc_t* desc, int32_t elementid); 

GS_API_DECL gs_gui_style_t gs_gui_animation_get_blend_style(gs_gui_context_t* ctx, gs_gui_animation_t* anim, 
        const gs_gui_selector_desc_t* desc, int32_t elementid); 

//=== Style Sheet ===//

GS_API_DECL gs_gui_style_sheet_t gs_gui_style_sheet_create(gs_gui_context_t* ctx, gs_gui_style_sheet_desc_t* desc);
GS_API_DECL void gs_gui_style_sheet_destroy(gs_gui_context_t* ctx, gs_gui_style_sheet_t* ss);
GS_API_DECL void gs_gui_set_element_style(gs_gui_context_t* ctx, gs_gui_element_type element, gs_gui_element_state state, gs_gui_style_element_t* style, size_t size);
GS_API_DECL void gs_gui_style_sheet_set_element_styles(gs_gui_style_sheet_t* style_sheet, gs_gui_element_type element, gs_gui_element_state state, gs_gui_style_element_t* styles, size_t size);
GS_API_DECL void gs_gui_set_style_sheet(gs_gui_context_t* ctx, gs_gui_style_sheet_t* style_sheet);

//=== Resource Loading ===//

GS_API_DECL gs_gui_style_sheet_t gs_gui_style_sheet_load_from_file(gs_gui_context_t* ctx, const char* file_path);
GS_API_DECL gs_gui_style_sheet_t gs_gui_style_sheet_load_from_memory(gs_gui_context_t* ctx, const char* memory, size_t sz, bool* success);

//=== Pools ===//

GS_API_DECL int32_t gs_gui_pool_init(gs_gui_context_t *ctx, gs_gui_pool_item_t *items, int32_t len, gs_gui_id id);
GS_API_DECL int32_t gs_gui_pool_get(gs_gui_context_t *ctx, gs_gui_pool_item_t *items, int32_t len, gs_gui_id id);
GS_API_DECL void gs_gui_pool_update(gs_gui_context_t *ctx, gs_gui_pool_item_t *items, int32_t idx);

//=== Input ===//

GS_API_DECL void gs_gui_input_mousemove(gs_gui_context_t *ctx, int32_t x, int32_t y);
GS_API_DECL void gs_gui_input_mousedown(gs_gui_context_t *ctx, int32_t x, int32_t y, int32_t btn);
GS_API_DECL void gs_gui_input_mouseup(gs_gui_context_t *ctx, int32_t x, int32_t y, int32_t btn);
GS_API_DECL void gs_gui_input_scroll(gs_gui_context_t *ctx, int32_t x, int32_t y);
GS_API_DECL void gs_gui_input_keydown(gs_gui_context_t *ctx, int32_t key);
GS_API_DECL void gs_gui_input_keyup(gs_gui_context_t *ctx, int32_t key);
GS_API_DECL void gs_gui_input_text(gs_gui_context_t *ctx, const char *text); 

//=== Commands ===//

GS_API_DECL gs_gui_command_t* gs_gui_push_command(gs_gui_context_t* ctx, int32_t type, int32_t size);
GS_API_DECL int32_t gs_gui_next_command(gs_gui_context_t* ctx, gs_gui_command_t** cmd); 
GS_API_DECL void gs_gui_set_clip(gs_gui_context_t* ctx, gs_gui_rect_t rect);

//=== Drawing ===//

GS_API_DECL void gs_gui_draw_rect(gs_gui_context_t* ctx, gs_gui_rect_t rect, gs_color_t color);
GS_API_DECL void gs_gui_draw_circle(gs_gui_context_t* ctx, gs_vec2 position, float radius, gs_color_t color);
GS_API_DECL void gs_gui_draw_triangle(gs_gui_context_t* ctx, gs_vec2 a, gs_vec2 b, gs_vec2 c, gs_color_t color);
GS_API_DECL void gs_gui_draw_box(gs_gui_context_t* ctx, gs_gui_rect_t rect, int16_t* width, gs_color_t color);
GS_API_DECL void gs_gui_draw_line(gs_gui_context_t* ctx, gs_vec2 start, gs_vec2 end, gs_color_t color);
GS_API_DECL void gs_gui_draw_text(gs_gui_context_t* ctx, gs_asset_font_t* font, const char *str, int32_t len, gs_vec2 pos, gs_color_t color, int32_t shadow_x, int32_t shadow_y, gs_color_t shadow_color);
GS_API_DECL void gs_gui_draw_image(gs_gui_context_t* ctx, gs_handle(gs_graphics_texture_t) hndl, gs_gui_rect_t rect, gs_vec2 uv0, gs_vec2 uv1, gs_color_t color);
GS_API_DECL void gs_gui_draw_nine_rect(gs_gui_context_t* ctx, gs_handle(gs_graphics_texture_t) hndl, gs_gui_rect_t rect, gs_vec2 uv0, gs_vec2 uv1, uint32_t left, uint32_t right, uint32_t top, uint32_t bottom, gs_color_t color);
GS_API_DECL void gs_gui_draw_control_frame(gs_gui_context_t* ctx, gs_gui_id id, gs_gui_rect_t rect, int32_t elementid, int32_t opt);
GS_API_DECL void gs_gui_draw_control_text(gs_gui_context_t* ctx, const char *str, gs_gui_rect_t rect, const gs_gui_style_t* style, int32_t opt);
GS_API_DECL void gs_gui_draw_custom(gs_gui_context_t* ctx, gs_gui_rect_t rect, gs_gui_draw_callback_t cb, void* data, size_t sz);

//=== Layout ===//

GS_API_DECL void gs_gui_layout_row(gs_gui_context_t *ctx, int32_t items, const int32_t *widths, int32_t height);
GS_API_DECL void gs_gui_layout_row_ex(gs_gui_context_t *ctx, int32_t items, const int32_t *widths, int32_t height, int32_t justification);
GS_API_DECL void gs_gui_layout_width(gs_gui_context_t *ctx, int32_t width);
GS_API_DECL void gs_gui_layout_height(gs_gui_context_t *ctx, int32_t height);
GS_API_DECL void gs_gui_layout_column_begin(gs_gui_context_t *ctx);
GS_API_DECL void gs_gui_layout_column_end(gs_gui_context_t *ctx);
GS_API_DECL void gs_gui_layout_set_next(gs_gui_context_t *ctx, gs_gui_rect_t r, int32_t relative);
GS_API_DECL gs_gui_rect_t gs_gui_layout_next(gs_gui_context_t *ctx); 
GS_API_DECL gs_gui_rect_t gs_gui_layout_anchor(const gs_gui_rect_t* parent, int32_t width, int32_t height, int32_t xoff, int32_t yoff, gs_gui_layout_anchor_type type);

//=== Elements ===//

#define gs_gui_button(_CTX, _LABEL)				    gs_gui_button_ex((_CTX), (_LABEL), NULL, GS_GUI_OPT_ALIGNCENTER | GS_GUI_OPT_LEFTCLICKONLY)
#define gs_gui_text(_CTX, _TXT)                     gs_gui_text_ex((_CTX), (_TXT), 1, NULL, 0x00)
#define gs_gui_textbox(_CTX, _BUF, _BUFSZ)			gs_gui_textbox_ex((_CTX), (_BUF), (_BUFSZ), NULL, 0x00)
#define gs_gui_slider(_CTX, _VALUE, _LO, _HI)		gs_gui_slider_ex((_CTX), (_VALUE), (_LO), (_HI), 0, GS_GUI_SLIDER_FMT, NULL, GS_GUI_OPT_ALIGNCENTER)
#define gs_gui_number(_CTX, _VALUE, _STEP)			gs_gui_number_ex((_CTX), (_VALUE), (_STEP), GS_GUI_SLIDER_FMT, NULL, GS_GUI_OPT_ALIGNCENTER)
#define gs_gui_header(_CTX, _LABEL)		            gs_gui_header_ex((_CTX), (_LABEL), NULL, 0x00)
#define gs_gui_checkbox(_CTX, _LABEL, _STATE)       gs_gui_checkbox_ex((_CTX), (_LABEL), (_STATE), NULL, 0x00)
#define gs_gui_treenode_begin(_CTX, _LABEL)	        gs_gui_treenode_begin_ex((_CTX), (_LABEL), NULL, 0x00)
#define gs_gui_window_begin(_CTX, _TITLE, _RECT)    gs_gui_window_begin_ex((_CTX), (_TITLE), (_RECT), 0, NULL, 0x00)
#define gs_gui_popup_begin(_CTX, _TITLE, _RECT)     gs_gui_popup_begin_ex((_CTX), (_TITLE), (_RECT), NULL, 0x00)
#define gs_gui_panel_begin(_CTX, _NAME)			    gs_gui_panel_begin_ex((_CTX), (_NAME), NULL, 0x00)
#define gs_gui_image(_CTX, _HNDL)                   gs_gui_image_ex((_CTX), (_HNDL), gs_v2s(0.f), gs_v2s(1.f), NULL, 0x00)
#define gs_gui_combo_begin(_CTX, _ID, _ITEM, _MAX)  gs_gui_combo_begin_ex((_CTX), (_ID), (_ITEM), (_MAX), NULL, 0x00)
#define gs_gui_dock(_CTX, _DST, _SRC, _TYPE)        gs_gui_dock_ex((_CTX), (_DST), (_SRC), (_TYPE), 0.5f)
#define gs_gui_undock(_CTX, _NAME)                  gs_gui_undock_ex((_CTX), (_NAME)) 
#define gs_gui_label(_CTX, _FMT, ...)\
    (\
     gs_snprintf((_CTX)->number_edit_buf, sizeof((_CTX)->number_edit_buf), _FMT, ## __VA_ARGS__),\
     gs_gui_label_ex((_CTX), (_CTX)->number_edit_buf, NULL, 0x00)\
    )

//=== Elements (Extended) ===//

GS_API_DECL int32_t gs_gui_image_ex(gs_gui_context_t* ctx, gs_handle(gs_graphics_texture_t) hndl, gs_vec2 uv0, gs_vec2 uv1, const gs_gui_selector_desc_t* desc, int32_t opt);
GS_API_DECL int32_t gs_gui_text_ex(gs_gui_context_t* ctx, const char* text, int32_t text_wrap, const gs_gui_selector_desc_t* desc, int32_t opt);
GS_API_DECL int32_t gs_gui_label_ex(gs_gui_context_t* ctx, const char* text, const gs_gui_selector_desc_t* desc, int32_t opt);
GS_API_DECL int32_t gs_gui_button_ex(gs_gui_context_t* ctx, const char* label, const gs_gui_selector_desc_t* desc, int32_t opt);
GS_API_DECL int32_t gs_gui_checkbox_ex(gs_gui_context_t* ctx, const char* label, int32_t* state, const gs_gui_selector_desc_t* desc, int32_t opt);
GS_API_DECL int32_t gs_gui_textbox_raw(gs_gui_context_t* ctx, char* buf, int32_t bufsz, gs_gui_id id, gs_gui_rect_t r, const gs_gui_selector_desc_t* desc, int32_t opt);
GS_API_DECL int32_t gs_gui_textbox_ex(gs_gui_context_t* ctx, char* buf, int32_t bufsz, const gs_gui_selector_desc_t* desc, int32_t opt);
GS_API_DECL int32_t gs_gui_slider_ex(gs_gui_context_t* ctx, gs_gui_real* value, gs_gui_real low, gs_gui_real high, gs_gui_real step, 
        const char* fmt, const gs_gui_selector_desc_t* desc, int32_t opt);
GS_API_DECL int32_t gs_gui_number_ex(gs_gui_context_t* ctx, gs_gui_real* value, gs_gui_real step, const char* fmt, const gs_gui_selector_desc_t* desc, int32_t opt);
GS_API_DECL int32_t gs_gui_header_ex(gs_gui_context_t* ctx, const char* label, const gs_gui_selector_desc_t* desc, int32_t opt);
GS_API_DECL int32_t gs_gui_treenode_begin_ex(gs_gui_context_t * ctx, const char* label, const gs_gui_selector_desc_t* desc, int32_t opt);
GS_API_DECL void gs_gui_treenode_end(gs_gui_context_t* ctx);
GS_API_DECL int32_t gs_gui_window_begin_ex(gs_gui_context_t * ctx, const char* title, gs_gui_rect_t rect, bool* open, const gs_gui_selector_desc_t* desc, int32_t opt);
GS_API_DECL void gs_gui_window_end(gs_gui_context_t* ctx);
GS_API_DECL void gs_gui_popup_open(gs_gui_context_t* ctx, const char* name);
GS_API_DECL int32_t gs_gui_popup_begin_ex(gs_gui_context_t* ctx, const char* name, gs_gui_rect_t r, const gs_gui_selector_desc_t* desc, int32_t opt);
GS_API_DECL void gs_gui_popup_end(gs_gui_context_t* ctx);
GS_API_DECL void gs_gui_panel_begin_ex(gs_gui_context_t* ctx, const char* name, const gs_gui_selector_desc_t* desc, int32_t opt);
GS_API_DECL void gs_gui_panel_end(gs_gui_context_t* ctx); 
GS_API_DECL int32_t gs_gui_combo_begin_ex(gs_gui_context_t* ctx, const char* id, const char* current_item, int32_t max_items, gs_gui_selector_desc_t* desc, int32_t opt);
GS_API_DECL void gs_gui_combo_end(gs_gui_context_t* ctx);

//=== Demos ===//

GS_API_DECL int32_t gs_gui_style_editor(gs_gui_context_t* ctx, gs_gui_style_sheet_t* style_sheet, gs_gui_rect_t rect, bool* open);
GS_API_DECL int32_t gs_gui_demo_window(gs_gui_context_t* ctx, gs_gui_rect_t rect, bool* open);

//=== Docking ===//

GS_API_DECL void gs_gui_dock_ex(gs_gui_context_t* ctx, const char* dst, const char* src, int32_t split_type, float ratio);
GS_API_DECL void gs_gui_undock_ex(gs_gui_context_t* ctx, const char* name);
GS_API_DECL void gs_gui_dock_ex_cnt(gs_gui_context_t* ctx, gs_gui_container_t* dst, gs_gui_container_t* src, int32_t split_type, float ratio);
GS_API_DECL void gs_gui_undock_ex_cnt(gs_gui_context_t* ctx, gs_gui_container_t* cnt);

//=== Gizmo ===//

GS_API_DECL int32_t gs_gui_gizmo(gs_gui_context_t* ctx, gs_camera_t* camera, gs_vqs* model, float snap, int32_t op, int32_t mode, int32_t opt);

//=== Implementation ===//

#ifdef GS_GUI_IMPL 

#ifndef GS_PHYSICS_IMPL
    #define GS_PHYSICS_IMPL
    #include "gs_physics.h"
#endif

#define gs_gui_unused(x) ((void) (x))

#define gs_gui_expect(x)\
    do {																                    \
		if (!(x)) {																			\
			gs_log_error("Fatal error: assertion '%s' failed\n",               \
				#x);												    \
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
	const unsigned char *p = (const unsigned char*)data;
	while (size--) 
    {
		*hash = (*hash ^ *p++) * 16777619;
	}
} 

static gs_gui_rect_t gs_gui_unclipped_rect = { 0, 0, 0x1000000, 0x1000000 };

// Default styles
static gs_gui_style_t gs_gui_default_container_style[3] = gs_default_val();
static gs_gui_style_t gs_gui_default_button_style[3] = gs_default_val();
static gs_gui_style_t gs_gui_default_text_style[3] = gs_default_val();
static gs_gui_style_t gs_gui_default_label_style[3] = gs_default_val();
static gs_gui_style_t gs_gui_default_panel_style[3] = gs_default_val(); 
static gs_gui_style_t gs_gui_default_input_style[3] = gs_default_val();
static gs_gui_style_t gs_gui_default_scroll_style[3] = gs_default_val(); 
static gs_gui_style_t gs_gui_default_image_style[3] = gs_default_val();

static gs_gui_style_t gs_gui_default_style = 
{
	// font | size | spacing | indent | title_height | scroll_width | thumb_width 
	NULL, {68, 18}, 2, 10, 20, 5, 5, 

    // colors
    {
        {25, 25, 25, 255},    // GS_GUI_COLOR_BACKGROUND
        {255, 255, 255, 255}, // GS_GUI_COLOR_CONTENT
        {29, 29, 29, 76},     // GS_GUI_COLOR_BORDER
        {0, 0, 0, 31},        // GS_GUI_COLOR_SHADOW
        {0, 0, 0, 0},         // GS_GUI_COLOR_CONTENT_BACKGROUND
        {0, 0, 0, 0},         // GS_GUI_COLOR_CONTENT_SHADOW
        {0, 0, 0, 0}          // GS_GUI_COLOR_CONTENT_BORDER
    },

    // padding (left, right, top, bottom)
    {2, 2, 2, 2},

    // margin (left, right, top, bottom)
    {2, 2, 2, 2},

    // border width (left, right, top, bottom)
    {1, 1, 1, 1},

    // border radius (left, right, top, bottom)
    {0, 0, 0, 0}, 

    // flex direction / justification / alignment / shrink / grow
    GS_GUI_DIRECTION_COLUMN, 
    GS_GUI_JUSTIFY_START, 
    GS_GUI_ALIGN_CENTER, 

    // shadow x, y
    1, 1
}; 

static gs_gui_style_sheet_t gs_gui_default_style_sheet = gs_default_val(); 

static gs_gui_style_t gs_gui_get_current_element_style(gs_gui_context_t* ctx, const gs_gui_selector_desc_t* desc, 
        int32_t elementid, int32_t state)
{
  
#define GS_GUI_APPLY_STYLE(SE)\
    do {\
        switch ((SE)->type)\
        {\
            case GS_GUI_STYLE_WIDTH:  style.size[0] = (float)(SE)->value; break;\
            case GS_GUI_STYLE_HEIGHT: style.size[1] = (float)(SE)->value; break;\
\
            case GS_GUI_STYLE_PADDING: {\
                style.padding[GS_GUI_PADDING_LEFT] = (int32_t)(SE)->value;\
                style.padding[GS_GUI_PADDING_TOP] = (int32_t)(SE)->value;\
                style.padding[GS_GUI_PADDING_RIGHT] = (int32_t)(SE)->value;\
                style.padding[GS_GUI_PADDING_BOTTOM] = (int32_t)(SE)->value;\
            }\
\
            case GS_GUI_STYLE_PADDING_LEFT:     style.padding[GS_GUI_PADDING_LEFT] = (int32_t)(SE)->value; break;\
            case GS_GUI_STYLE_PADDING_TOP:      style.padding[GS_GUI_PADDING_TOP] = (int32_t)(SE)->value; break;\
            case GS_GUI_STYLE_PADDING_RIGHT:    style.padding[GS_GUI_PADDING_RIGHT] = (int32_t)(SE)->value; break;\
            case GS_GUI_STYLE_PADDING_BOTTOM:   style.padding[GS_GUI_PADDING_BOTTOM] = (int32_t)(SE)->value; break;\
\
            case GS_GUI_STYLE_MARGIN: {\
                style.margin[GS_GUI_MARGIN_LEFT] = (int32_t)(SE)->value;\
                style.margin[GS_GUI_MARGIN_TOP] = (int32_t)(SE)->value;\
                style.margin[GS_GUI_MARGIN_RIGHT] = (int32_t)(SE)->value;\
                style.margin[GS_GUI_MARGIN_BOTTOM] = (int32_t)(SE)->value;\
            } break;\
\
            case GS_GUI_STYLE_MARGIN_LEFT:      style.margin[GS_GUI_MARGIN_LEFT] = (int32_t)(SE)->value; break;\
            case GS_GUI_STYLE_MARGIN_TOP:       style.margin[GS_GUI_MARGIN_TOP] = (int32_t)(SE)->value; break;\
            case GS_GUI_STYLE_MARGIN_RIGHT:     style.margin[GS_GUI_MARGIN_RIGHT] = (int32_t)(SE)->value; break;\
            case GS_GUI_STYLE_MARGIN_BOTTOM:    style.margin[GS_GUI_MARGIN_BOTTOM] = (int32_t)(SE)->value; break;\
\
            case GS_GUI_STYLE_BORDER_RADIUS: {\
                style.border_radius[0] = (SE)->value;\
                style.border_radius[1] = (SE)->value;\
                style.border_radius[2] = (SE)->value;\
                style.border_radius[3] = (SE)->value;\
            } break;\
\
            case GS_GUI_STYLE_BORDER_RADIUS_LEFT:   style.border_radius[0] = (SE)->value; break;\
            case GS_GUI_STYLE_BORDER_RADIUS_RIGHT:  style.border_radius[1] = (SE)->value; break;\
            case GS_GUI_STYLE_BORDER_RADIUS_TOP:    style.border_radius[2] = (SE)->value; break;\
            case GS_GUI_STYLE_BORDER_RADIUS_BOTTOM: style.border_radius[3] = (SE)->value; break;\
\
            case GS_GUI_STYLE_BORDER_WIDTH: {\
                style.border_width[0] = (SE)->value;\
                style.border_width[1] = (SE)->value;\
                style.border_width[2] = (SE)->value;\
                style.border_width[3] = (SE)->value;\
            } break;\
\
            case GS_GUI_STYLE_BORDER_WIDTH_LEFT:    style.border_width[0] = (SE)->value; break;\
            case GS_GUI_STYLE_BORDER_WIDTH_RIGHT:   style.border_width[1] = (SE)->value; break;\
            case GS_GUI_STYLE_BORDER_WIDTH_TOP:     style.border_width[2] = (SE)->value; break;\
            case GS_GUI_STYLE_BORDER_WIDTH_BOTTOM:  style.border_width[3] = (SE)->value; break;\
\
            case GS_GUI_STYLE_DIRECTION:        style.direction = (int32_t)(SE)->value; break;\
            case GS_GUI_STYLE_ALIGN_CONTENT:    style.align_content = (int32_t)(SE)->value; break;\
            case GS_GUI_STYLE_JUSTIFY_CONTENT:  style.justify_content = (int32_t)(SE)->value; break;\
\
            case GS_GUI_STYLE_SHADOW_X:         style.shadow_x = (int32_t)(SE)->value; break;\
            case GS_GUI_STYLE_SHADOW_Y:         style.shadow_y = (int32_t)(SE)->value; break;\
\
            case GS_GUI_STYLE_COLOR_BACKGROUND:         style.colors[GS_GUI_COLOR_BACKGROUND] = (SE)->color; break;\
            case GS_GUI_STYLE_COLOR_BORDER:             style.colors[GS_GUI_COLOR_BORDER] = (SE)->color; break;\
            case GS_GUI_STYLE_COLOR_SHADOW:             style.colors[GS_GUI_COLOR_SHADOW] = (SE)->color; break;\
            case GS_GUI_STYLE_COLOR_CONTENT:            style.colors[GS_GUI_COLOR_CONTENT] = (SE)->color; break;\
            case GS_GUI_STYLE_COLOR_CONTENT_BACKGROUND: style.colors[GS_GUI_COLOR_CONTENT_BACKGROUND] = (SE)->color; break;\
            case GS_GUI_STYLE_COLOR_CONTENT_BORDER:     style.colors[GS_GUI_COLOR_CONTENT_BORDER] = (SE)->color; break;\
            case GS_GUI_STYLE_COLOR_CONTENT_SHADOW:     style.colors[GS_GUI_COLOR_CONTENT_SHADOW] = (SE)->color; break;\
\
            case GS_GUI_STYLE_FONT:             style.font = (SE)->font; break;\
        }\
    } while (0)

    gs_gui_style_t style = ctx->style_sheet->styles[elementid][state];

    // Look for id tag style
    gs_gui_style_list_t* id_styles = NULL;
    gs_gui_style_list_t* cls_styles[GS_GUI_CLS_SELECTOR_MAX] = gs_default_val(); 

    if (desc)
    {
        char TMP[256] = gs_default_val();

        // ID selector
        gs_snprintf(TMP, sizeof(TMP), "#%s", desc->id);
        const uint64_t id_hash = gs_hash_str64(TMP);
        id_styles = gs_hash_table_exists(ctx->style_sheet->cid_styles, id_hash) ? 
            gs_hash_table_getp(ctx->style_sheet->cid_styles, id_hash) : NULL;

        // Class selectors
        for (uint32_t i = 0; i < GS_GUI_CLS_SELECTOR_MAX; ++i)
        {
            if (desc->classes[i]) {
                gs_snprintf(TMP, sizeof(TMP), ".%s", desc->classes[i]);
                const uint64_t cls_hash = gs_hash_str64(TMP);
                cls_styles[i] = gs_hash_table_exists(ctx->style_sheet->cid_styles, cls_hash) ? 
                    gs_hash_table_getp(ctx->style_sheet->cid_styles, cls_hash) : NULL;
            }
            else break;
        }
    }

    // Override with class styles
    if (*cls_styles)
    {
        for (uint32_t i = 0; i < GS_GUI_CLS_SELECTOR_MAX; ++i)
        {
			if (!cls_styles[i]) break;
            for (uint32_t s = 0; s < gs_dyn_array_size(cls_styles[i]->styles[state]); ++s) {
                gs_gui_style_element_t* se = &cls_styles[i]->styles[state][s];
                GS_GUI_APPLY_STYLE(se);
            }
        }
    }

    // Override with id styles
    if (id_styles)
    { 
        for (uint32_t i = 0; i < gs_dyn_array_size(id_styles->styles[state]); ++i) {
            gs_gui_style_element_t* se = &id_styles->styles[state][i];
            GS_GUI_APPLY_STYLE(se);
        }
    } 

    if (gs_hash_table_exists(ctx->inline_styles, (gs_gui_element_type)elementid))
    {
        gs_gui_inline_style_stack_t* iss = gs_hash_table_getp(ctx->inline_styles, 
                (gs_gui_element_type)elementid);
        if (gs_dyn_array_size(iss->styles[state]))
        { 
            // Get last size to apply for styles for this state
            const uint32_t scz = gs_dyn_array_size(iss->style_counts);
            const uint32_t ct = state == 0x00 ? iss->style_counts[scz - 3] : 
                                state == 0x01 ? iss->style_counts[scz - 2] : 
                                                iss->style_counts[scz - 1];
            const uint32_t ssz = gs_dyn_array_size(iss->styles[state]);

            for (uint32_t i = 0; i < ct; ++i) {
                uint32_t idx = (ssz - ct + i);
                gs_gui_style_element_t* se = &iss->styles[state][idx];
                GS_GUI_APPLY_STYLE(se);
            }
        } 
    }

    return style;
} 

GS_API_DECL gs_gui_style_t gs_gui_animation_get_blend_style(gs_gui_context_t* ctx, gs_gui_animation_t* anim, 
	const gs_gui_selector_desc_t* desc, int32_t elementid)
{
    gs_gui_style_t ret = gs_default_val(); 

    int32_t focus_state = anim->focus_state;
    int32_t hover_state = anim->hover_state; 

    gs_gui_style_t s0 = gs_gui_get_current_element_style(ctx, desc, elementid, anim->start_state);
    gs_gui_style_t s1 = gs_gui_get_current_element_style(ctx, desc, elementid, anim->end_state);

    gs_gui_inline_style_stack_t* iss = NULL;
    if (gs_hash_table_exists(ctx->inline_styles, (gs_gui_element_type)elementid)) {
        iss = gs_hash_table_getp(ctx->inline_styles, (gs_gui_element_type)elementid);
    }

    if (anim->direction == GS_GUI_ANIMATION_DIRECTION_FORWARD) {ret = s1;}
    else {ret = s0;} 

    const gs_gui_animation_property_list_t* list = NULL; 
    if (gs_hash_table_exists(ctx->style_sheet->animations, (gs_gui_element_type)elementid)) {
        list = gs_hash_table_getp(ctx->style_sheet->animations, (gs_gui_element_type)elementid);
    } 

	const gs_gui_animation_property_list_t* id_list = NULL; 
    const gs_gui_animation_property_list_t* cls_list[GS_GUI_CLS_SELECTOR_MAX] = gs_default_val();
	bool has_class_animations = false;

    if (desc)
    {
        char TMP[256] = gs_default_val();

        // ID animations
        if (desc->id)
        {
            gs_snprintf(TMP, sizeof(TMP), "#%s", desc->id);
            const uint64_t id_hash = gs_hash_str64(TMP);
            if (gs_hash_table_exists(ctx->style_sheet->cid_animations, id_hash)) { 
                id_list = gs_hash_table_getp(ctx->style_sheet->cid_animations, id_hash);
            }
        }

        // Class animations 
        if (*desc->classes)
        {
            for (uint32_t i = 0; i < GS_GUI_CLS_SELECTOR_MAX; ++i)
            {
                if (!desc->classes[i]) break; 
                gs_snprintf(TMP, sizeof(TMP), ".%s", desc->classes[i]);
                const uint64_t cls_hash = gs_hash_str64(TMP);
                if (cls_hash && gs_hash_table_exists(ctx->style_sheet->cid_animations, cls_hash)) { 
                    cls_list[i] = gs_hash_table_getp(ctx->style_sheet->cid_animations, cls_hash);
					has_class_animations = true;
                }
            }
        }
    }

#define GS_GUI_BLEND_COLOR(TYPE)\
        do {\
            gs_color_t* c0 = &s0.colors[TYPE];\
            gs_color_t* c1 = &s1.colors[TYPE];\
            float r = 255.f * gs_interp_smoothstep((float)c0->r / 255.f, (float)c1->r / 255.f, t);\
            float g = 255.f * gs_interp_smoothstep((float)c0->g / 255.f, (float)c1->g / 255.f, t);\
            float b = 255.f * gs_interp_smoothstep((float)c0->b / 255.f, (float)c1->b / 255.f, t);\
            float a = 255.f * gs_interp_smoothstep((float)c0->a / 255.f, (float)c1->a / 255.f, t);\
            ret.colors[TYPE] = gs_color((u8)r, (u8)g, (u8)b, (u8)a);\
        } while (0)

#define GS_GUI_BLEND_VALUE(FIELD, TYPE)\
        do {\
            float v0 = (float)s0.FIELD;\
            float v1 = (float)s1.FIELD;\
            ret.FIELD = (TYPE)gs_interp_smoothstep(v0, v1, t);\
        } while (0) 

#define GS_GUI_BLEND_PROPERTIES(LIST)\
    do {\
        for (uint32_t i = 0; i < gs_dyn_array_size(LIST); ++i)\
        {\
            const gs_gui_animation_property_t* prop = &LIST[i];\
            float t = 0.f;\
            switch (anim->direction)\
            {\
                default:\
                case GS_GUI_ANIMATION_DIRECTION_FORWARD:\
                {\
                    t = gs_clamp(gs_map_range((float)prop->delay, (float)prop->time + (float)prop->delay, 0.f, 1.f, (float)anim->time), 0.f, 1.f);\
                } break;\
                case GS_GUI_ANIMATION_DIRECTION_BACKWARD:\
                {\
                    if (prop->time <= 0.f)\
                        t = 1.f;\
                    else\
                        t = gs_clamp(gs_map_range((float)0.f, (float)anim->max - (float)prop->delay, 0.f, 1.f, (float)anim->time), 0.f, 1.f);\
                } break;\
            }\
\
            switch (prop->type)\
            {\
                case GS_GUI_STYLE_COLOR_BACKGROUND:         {GS_GUI_BLEND_COLOR(GS_GUI_COLOR_BACKGROUND);} break;\
                case GS_GUI_STYLE_COLOR_SHADOW:             {GS_GUI_BLEND_COLOR(GS_GUI_COLOR_SHADOW);} break;\
                case GS_GUI_STYLE_COLOR_BORDER:             {GS_GUI_BLEND_COLOR(GS_GUI_COLOR_BORDER);} break;\
                case GS_GUI_STYLE_COLOR_CONTENT:            {GS_GUI_BLEND_COLOR(GS_GUI_COLOR_CONTENT);} break;\
                case GS_GUI_STYLE_COLOR_CONTENT_BACKGROUND: {GS_GUI_BLEND_COLOR(GS_GUI_COLOR_CONTENT_BACKGROUND);} break;\
                case GS_GUI_STYLE_COLOR_CONTENT_SHADOW:     {GS_GUI_BLEND_COLOR(GS_GUI_COLOR_CONTENT_SHADOW);} break;\
                case GS_GUI_STYLE_COLOR_CONTENT_BORDER:     {GS_GUI_BLEND_COLOR(GS_GUI_COLOR_CONTENT_BORDER);} break;\
                case GS_GUI_STYLE_WIDTH:            {GS_GUI_BLEND_VALUE(size[0], float);} break;\
                case GS_GUI_STYLE_HEIGHT:           {GS_GUI_BLEND_VALUE(size[1], float);} break;\
                case GS_GUI_STYLE_BORDER_WIDTH: {\
                    GS_GUI_BLEND_VALUE(border_width[0], int16_t);\
                    GS_GUI_BLEND_VALUE(border_width[1], int16_t);\
                    GS_GUI_BLEND_VALUE(border_width[2], int16_t);\
                    GS_GUI_BLEND_VALUE(border_width[3], int16_t);\
                } break;\
                case GS_GUI_STYLE_BORDER_WIDTH_LEFT:   {GS_GUI_BLEND_VALUE(border_width[0], int16_t);} break;\
                case GS_GUI_STYLE_BORDER_WIDTH_RIGHT:  {GS_GUI_BLEND_VALUE(border_width[1], int16_t);} break;\
                case GS_GUI_STYLE_BORDER_WIDTH_TOP:    {GS_GUI_BLEND_VALUE(border_width[2], int16_t);} break;\
                case GS_GUI_STYLE_BORDER_WIDTH_BOTTOM: {GS_GUI_BLEND_VALUE(border_width[3], int16_t);} break;\
                case GS_GUI_STYLE_BORDER_RADIUS: {\
                    GS_GUI_BLEND_VALUE(border_radius[0], int16_t);\
                    GS_GUI_BLEND_VALUE(border_radius[1], int16_t);\
                    GS_GUI_BLEND_VALUE(border_radius[2], int16_t);\
                    GS_GUI_BLEND_VALUE(border_radius[3], int16_t);\
                } break;\
                case GS_GUI_STYLE_BORDER_RADIUS_LEFT:   {GS_GUI_BLEND_VALUE(border_radius[0], int16_t);} break;\
                case GS_GUI_STYLE_BORDER_RADIUS_RIGHT:  {GS_GUI_BLEND_VALUE(border_radius[1], int16_t);} break;\
                case GS_GUI_STYLE_BORDER_RADIUS_TOP:    {GS_GUI_BLEND_VALUE(border_radius[2], int16_t);} break;\
                case GS_GUI_STYLE_BORDER_RADIUS_BOTTOM: {GS_GUI_BLEND_VALUE(border_radius[3], int16_t);} break;\
                case GS_GUI_STYLE_MARGIN_BOTTOM:    {GS_GUI_BLEND_VALUE(margin[GS_GUI_MARGIN_BOTTOM], int16_t);} break;\
                case GS_GUI_STYLE_MARGIN_TOP:       {GS_GUI_BLEND_VALUE(margin[GS_GUI_MARGIN_TOP], int16_t);} break;\
                case GS_GUI_STYLE_MARGIN_LEFT:      {GS_GUI_BLEND_VALUE(margin[GS_GUI_MARGIN_LEFT], int16_t);} break;\
                case GS_GUI_STYLE_MARGIN_RIGHT:     {GS_GUI_BLEND_VALUE(margin[GS_GUI_MARGIN_RIGHT], int16_t);} break;\
                case GS_GUI_STYLE_MARGIN: {\
                    GS_GUI_BLEND_VALUE(margin[0], int16_t);\
                    GS_GUI_BLEND_VALUE(margin[1], int16_t);\
                    GS_GUI_BLEND_VALUE(margin[2], int16_t);\
                    GS_GUI_BLEND_VALUE(margin[3], int16_t);\
                } break;\
                case GS_GUI_STYLE_PADDING_BOTTOM:    {GS_GUI_BLEND_VALUE(padding[GS_GUI_PADDING_BOTTOM], int32_t);} break;\
                case GS_GUI_STYLE_PADDING_TOP:       {GS_GUI_BLEND_VALUE(padding[GS_GUI_PADDING_TOP], int32_t);} break;\
                case GS_GUI_STYLE_PADDING_LEFT:      {GS_GUI_BLEND_VALUE(padding[GS_GUI_PADDING_LEFT], int32_t);} break;\
                case GS_GUI_STYLE_PADDING_RIGHT:     {GS_GUI_BLEND_VALUE(padding[GS_GUI_PADDING_RIGHT], int32_t);} break;\
                case GS_GUI_STYLE_PADDING: {\
                    GS_GUI_BLEND_VALUE(padding[0], int32_t);\
                    GS_GUI_BLEND_VALUE(padding[1], int32_t);\
                    GS_GUI_BLEND_VALUE(padding[2], int32_t);\
                    GS_GUI_BLEND_VALUE(padding[3], int32_t);\
                } break;\
                case GS_GUI_STYLE_SHADOW_X:         {GS_GUI_BLEND_VALUE(shadow_x, int16_t);} break;\
                case GS_GUI_STYLE_SHADOW_Y:         {GS_GUI_BLEND_VALUE(shadow_y, int16_t);} break;\
            }\
        }\
    } while (0)

    // Get final blends
    if (list && !gs_dyn_array_empty(list->properties[anim->end_state])) { 
        GS_GUI_BLEND_PROPERTIES(list->properties[anim->end_state]);
    } 

    // Class list
    if (has_class_animations)
    {
        for (uint32_t c = 0; c < GS_GUI_CLS_SELECTOR_MAX; ++c)
        {
            if (!cls_list[c]) continue;
            if (!gs_dyn_array_empty(cls_list[c]->properties[anim->end_state])) {
                GS_GUI_BLEND_PROPERTIES(cls_list[c]->properties[anim->end_state]);
            }
        }
    }

	// Id list
	if (id_list && !gs_dyn_array_empty(id_list->properties[anim->end_state])) { 
        GS_GUI_BLEND_PROPERTIES(id_list->properties[anim->end_state]);
	}

    if (iss) {
        GS_GUI_BLEND_PROPERTIES(iss->animations[anim->end_state]);
    }

    return ret;
}

static void _gs_gui_animation_get_time(gs_gui_context_t* ctx, gs_gui_id id, int32_t elementid,
	const gs_gui_selector_desc_t* desc, gs_gui_inline_style_stack_t* iss, int32_t state, gs_gui_animation_t* anim) 
{ 
	uint32_t act = 0, ssz = 0;
	if (iss && gs_dyn_array_size(iss->animations[state])) {
		const uint32_t scz = gs_dyn_array_size(iss->animation_counts);
		act = state == 0x00 ? iss->animation_counts[scz - 3] :
			  state == 0x01 ? iss->animation_counts[scz - 2] :
							  iss->animation_counts[scz - 1];
		ssz = gs_dyn_array_size(iss->animations[state]);
	}
    gs_gui_animation_property_list_t* cls_list[GS_GUI_CLS_SELECTOR_MAX] = gs_default_val();
	const gs_gui_animation_property_list_t* id_list = NULL;
	const gs_gui_animation_property_list_t* list = NULL;
	bool has_class_animations = false;

    if (desc)
    {
        char TMP[256] = gs_default_val();
        
        // Id animations
        gs_snprintf(TMP, sizeof(TMP), "#%s", desc->id);
        const uint64_t id_hash = gs_hash_str64(TMP);
        if (gs_hash_table_exists(ctx->style_sheet->cid_animations, id_hash)) {
            id_list = gs_hash_table_getp(ctx->style_sheet->cid_animations, id_hash);
        }

        // Class animations
        for (uint32_t i = 0; i < GS_GUI_CLS_SELECTOR_MAX; ++i)
        {
            if (!desc->classes[i]) break;
            gs_snprintf(TMP, sizeof(TMP), ".%s", desc->classes[i]);
            const uint64_t cls_hash = gs_hash_str64(TMP);
            if (gs_hash_table_exists(ctx->style_sheet->cid_animations, cls_hash)) {
                cls_list[i] = gs_hash_table_getp(ctx->style_sheet->cid_animations, cls_hash);
				has_class_animations = true;
            }
        }
    }

    // Element type animations
	if (gs_hash_table_exists(ctx->style_sheet->animations, (gs_gui_element_type)elementid)) {
		list = gs_hash_table_getp(ctx->style_sheet->animations, (gs_gui_element_type)elementid);
	} 

	// Fill properties in order of specificity
	gs_gui_animation_property_t properties[GS_GUI_STYLE_COUNT] = gs_default_val();
	for (uint32_t i = 0; i < GS_GUI_STYLE_COUNT; ++i) { 
		properties[i].type = (gs_gui_style_element_type)i;
	}

#define GUI_SET_PROPERTY_TIMES(PROP_LIST)\
	do {\
		for (uint32_t p = 0; p < gs_dyn_array_size((PROP_LIST)); ++p)\
		{\
			gs_gui_animation_property_t* prop = &(PROP_LIST)[p];\
			properties[prop->type].time = prop->time;\
			properties[prop->type].delay = prop->delay;\
		}\
	} while (0)

	// Element type list
	if (list)
	{ 
		gs_dyn_array(gs_gui_animation_property_t) props = list->properties[state];
		GUI_SET_PROPERTY_TIMES(props);
	}

	// Class list
	if (has_class_animations)
	{
		for (uint32_t c = 0; c < GS_GUI_CLS_SELECTOR_MAX; ++c)
		{
			if (!cls_list[c]) continue;
			gs_dyn_array(gs_gui_animation_property_t) props = cls_list[c]->properties[state];
			GUI_SET_PROPERTY_TIMES(props);
		}
	} 

	// Id list
	if (id_list)
	{
		gs_dyn_array(gs_gui_animation_property_t) props = id_list->properties[state];
		GUI_SET_PROPERTY_TIMES(props);
	}

	// Inline style list 
	if (act && iss)
	{ 
		for ( uint32_t a = 0; a < act; ++a )
		{
			uint32_t idx = ssz - act + a;
			gs_gui_animation_property_t* ap = &iss->animations[state][idx];
			properties[ap->type].time = ap->time;
			properties[ap->type].delay = ap->delay;
		}
	}

	// Set max times
	for (uint32_t i = 0; i < GS_GUI_STYLE_COUNT; ++i)
	{
		if (properties[i].time > anim->max) anim->max = properties[i].time;
		if (properties[i].delay > anim->delay) anim->delay = properties[i].delay;
	} 

	// Finalize time
	anim->max += anim->delay;
	anim->max = gs_max(anim->max, 5); 
}

GS_API_DECL gs_gui_animation_t* gs_gui_get_animation(gs_gui_context_t* ctx, gs_gui_id id, const gs_gui_selector_desc_t* desc, int32_t elementid)
{ 
    gs_gui_animation_t* anim = NULL; 

    const bool32 valid_eid = (elementid >= 0 && elementid < GS_GUI_ELEMENT_COUNT); 

    // Construct new animation if necessary to insert
    if (ctx->state_switch_id == id)
    { 
        if (!gs_hash_table_exists(ctx->animations, id))
        {
            gs_gui_animation_t val = gs_default_val();
            gs_hash_table_insert(ctx->animations, id, val);
        } 

		gs_gui_inline_style_stack_t* iss = NULL;
		if (gs_hash_table_exists(ctx->inline_styles, (gs_gui_element_type)elementid))
		{
			iss = gs_hash_table_getp(ctx->inline_styles, (gs_gui_element_type)elementid);
		}

	#define ANIM_GET_TIME(STATE)\

        anim = gs_hash_table_getp(ctx->animations, id); 
        anim->playing = true;

        int16_t focus_state = 0x00; 
        int16_t hover_state = 0x00;
        int16_t direction = 0x00;
        int16_t start_state = 0x00; 
        int16_t end_state = 0x00;
        int16_t time_state = 0x00;

        switch (ctx->switch_state)
        {
            case GS_GUI_ELEMENT_STATE_OFF_FOCUS:
            {
                if (ctx->hover == id) 
                {
                    anim->direction = GS_GUI_ANIMATION_DIRECTION_BACKWARD;
                    anim->start_state = GS_GUI_ELEMENT_STATE_HOVER;
                    anim->end_state = GS_GUI_ELEMENT_STATE_FOCUS;
                    time_state = GS_GUI_ELEMENT_STATE_HOVER;
					if (valid_eid) _gs_gui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                    anim->time = anim->max;
                } 
                else 
                {
                    anim->direction = GS_GUI_ANIMATION_DIRECTION_BACKWARD;
                    anim->start_state = GS_GUI_ELEMENT_STATE_DEFAULT;
                    anim->end_state = GS_GUI_ELEMENT_STATE_FOCUS;
                    time_state = GS_GUI_ELEMENT_STATE_DEFAULT;
					if (valid_eid) _gs_gui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                    anim->time = anim->max;
                }
            } break;

            case GS_GUI_ELEMENT_STATE_ON_FOCUS:
            {
                anim->direction = GS_GUI_ANIMATION_DIRECTION_FORWARD;
                anim->start_state = GS_GUI_ELEMENT_STATE_HOVER;
                anim->end_state = GS_GUI_ELEMENT_STATE_FOCUS;
                time_state = GS_GUI_ELEMENT_STATE_FOCUS;
				if (valid_eid) _gs_gui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                anim->time = 0;
            } break;

            case GS_GUI_ELEMENT_STATE_OFF_HOVER:
            {
                anim->direction = GS_GUI_ANIMATION_DIRECTION_BACKWARD;
                anim->start_state = GS_GUI_ELEMENT_STATE_DEFAULT;
                anim->end_state = GS_GUI_ELEMENT_STATE_HOVER;
                time_state = GS_GUI_ELEMENT_STATE_DEFAULT;
				if (valid_eid) _gs_gui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                anim->time = anim->max;
            } break;

            case GS_GUI_ELEMENT_STATE_ON_HOVER:
            {
                anim->direction = GS_GUI_ANIMATION_DIRECTION_FORWARD;
                anim->start_state = GS_GUI_ELEMENT_STATE_DEFAULT;
                anim->end_state = GS_GUI_ELEMENT_STATE_HOVER;
                time_state = GS_GUI_ELEMENT_STATE_HOVER;
				if (valid_eid) _gs_gui_animation_get_time(ctx, id, elementid, desc, iss, time_state, anim);
                anim->time = 0;
            } break;
        } 

        // Reset state switches and id
        ctx->state_switch_id = 0; 
        ctx->switch_state = 0;

        return anim;
    } 

    // Return if found
    if (gs_hash_table_exists(ctx->animations, id)) {
        anim = gs_hash_table_getp(ctx->animations, id);
    } 

    if (anim && !anim->playing)
    {
        // This is causing a crash...
        gs_hash_table_erase(ctx->animations, id);
        anim = NULL;
    } 

    return anim;
}

GS_API_DECL void gs_gui_animation_update(gs_gui_context_t* ctx, gs_gui_animation_t* anim)
{
    if (ctx->frame == anim->frame) return;

    const int16_t dt = (int16_t)(gs_platform_delta_time() * 1000.f);

    if (anim->playing)
    { 
        // Forward
        switch (anim->direction)
        {
            default: 
            case (GS_GUI_ANIMATION_DIRECTION_FORWARD):
            {
                anim->time += dt; 
                if (anim->time >= anim->max)
                {
                    anim->time = anim->max;
                    anim->playing = false;
                }
            } break;

            case (GS_GUI_ANIMATION_DIRECTION_BACKWARD):
            {
                anim->time -= dt; 
                if (anim->time <= 0)
                {
                    anim->time = 0;
                    anim->playing = false;
                }
            } break;
        } 
    }

    anim->frame = ctx->frame;
}

GS_API_DECL gs_gui_rect_t gs_gui_rect(float x, float y, float w, float h) 
{
	gs_gui_rect_t res;
	res.x = x; res.y = y; res.w = w; res.h = h;
	return res;
} 

static gs_gui_rect_t gs_gui_expand_rect(gs_gui_rect_t rect, int16_t v[4]) 
{
	return gs_gui_rect(rect.x - v[0], 
			   rect.y - v[2], 
			   rect.w + v[0] + v[1], 
			   rect.h + v[2] + v[3]);
} 

static gs_gui_rect_t gs_gui_intersect_rects(gs_gui_rect_t r1, gs_gui_rect_t r2) 
{
	int32_t x1 = (int32_t)gs_max(r1.x, r2.x);
	int32_t y1 = (int32_t)gs_max(r1.y, r2.y);
	int32_t x2 = (int32_t)gs_min(r1.x + r1.w, r2.x + r2.w);
	int32_t y2 = (int32_t)gs_min(r1.y + r1.h, r2.y + r2.h);
	if (x2 < x1) {x2 = x1;}
	if (y2 < y1) {y2 = y1;}
	return gs_gui_rect((float)x1, (float)y1, (float)x2 - (float)x1, (float)y2 - (float)y1);
}

static int32_t gs_gui_rect_overlaps_vec2(gs_gui_rect_t r, gs_vec2 p) 
{
	return p.x >= r.x && p.x < r.x + r.w && p.y >= r.y && p.y < r.y + r.h;
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
        // ctx->hover = c0;
    }
    else
    {
        gs_gui_split_t* s = gs_slot_array_getp(ctx->splits, c0->split);
        gs_gui_bring_split_to_front(ctx, s);
    }

    if (c1->type == GS_GUI_SPLIT_NODE_CONTAINER) 
    {
        gs_gui_bring_to_front(ctx, c1->container); 
        // ctx->hover = c1;
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

static void gs_gui_draw_frame(gs_gui_context_t* ctx, gs_gui_rect_t rect, gs_gui_style_t* style) 
{ 
	gs_gui_draw_rect(ctx, rect, style->colors[GS_GUI_COLOR_BACKGROUND]);

	// draw border
	if (style->colors[GS_GUI_COLOR_BORDER].a) 
    {
		gs_gui_draw_box(ctx, gs_gui_expand_rect(rect, (int16_t*)style->border_width), (int16_t*)style->border_width, style->colors[GS_GUI_COLOR_BORDER]);
	}
}

static int32_t gs_gui_compare_zindex(const void *a, const void *b) 
{
	return (*(gs_gui_container_t**) a)->zindex - (*(gs_gui_container_t**) b)->zindex;
} 

static gs_gui_style_t* gs_gui_push_style(gs_gui_context_t* ctx, gs_gui_style_t* style)
{ 
    gs_gui_style_t* save = ctx->style;
    ctx->style = style;
    return save;
}

static void gs_gui_push_inline_style(gs_gui_context_t* ctx, gs_gui_element_type elementid, gs_gui_inline_style_desc_t* desc)
{ 
    if (elementid >= GS_GUI_ELEMENT_COUNT || !desc)
    {
        return;
    } 

    if (!gs_hash_table_exists(ctx->inline_styles, elementid)) 
    {
        gs_gui_inline_style_stack_t v = gs_default_val();
        gs_hash_table_insert(ctx->inline_styles, elementid, v);
    }

    gs_gui_inline_style_stack_t* iss = gs_hash_table_getp(ctx->inline_styles, elementid);
    gs_assert(iss); 

    // Counts to keep for popping off
    uint32_t style_ct[3] = gs_default_val(), anim_ct[3] = gs_default_val(); 

    if (desc->all.style.data && desc->all.style.size)
    {
        // Total amount to write for each section
        uint32_t ct = desc->all.style.size / sizeof(gs_gui_style_element_t);
        style_ct[0] += ct;
        style_ct[1] += ct;
        style_ct[2] += ct;

        // Iterate through all properties, then just push them back into style element list
        for (uint32_t i = 0; i < ct; ++i)
        {
            gs_dyn_array_push(iss->styles[0], desc->all.style.data[i]);
            gs_dyn_array_push(iss->styles[1], desc->all.style.data[i]);
            gs_dyn_array_push(iss->styles[2], desc->all.style.data[i]);
        }
    }
    if (desc->all.animation.data && desc->all.animation.size)
    {
        // Total amount to write for each section
        uint32_t ct = desc->all.animation.size / sizeof(gs_gui_animation_property_t);
        anim_ct[0] += ct;
        anim_ct[1] += ct;
        anim_ct[2] += ct;

        for (uint32_t i = 0; i < ct; ++i)
        {
            gs_dyn_array_push(iss->animations[0], desc->all.animation.data[i]);
            gs_dyn_array_push(iss->animations[1], desc->all.animation.data[i]);
            gs_dyn_array_push(iss->animations[2], desc->all.animation.data[i]);
        }
    }

#define GS_GUI_COPY_INLINE_STYLE(TYPE, INDEX)\
    do {\
        if (desc->TYPE.style.data && desc->TYPE.style.size)\
        {\
            uint32_t ct = desc->TYPE.style.size / sizeof(gs_gui_style_element_t);\
            style_ct[INDEX] += ct;\
            for (uint32_t i = 0; i < ct; ++i)\
            {\
                gs_dyn_array_push(iss->styles[INDEX], desc->TYPE.style.data[i]);\
            }\
        }\
        if (desc->TYPE.animation.data && desc->TYPE.animation.size)\
        {\
            uint32_t ct = desc->TYPE.animation.size / sizeof(gs_gui_animation_property_t);\
            anim_ct[INDEX] += ct;\
\
            for (uint32_t i = 0; i < ct; ++i)\
            {\
                gs_dyn_array_push(iss->animations[INDEX], desc->TYPE.animation.data[i]);\
            }\
        }\
    } while (0)

    // Copy remaining individual styles
    GS_GUI_COPY_INLINE_STYLE(def, 0);
    GS_GUI_COPY_INLINE_STYLE(hover, 1);
    GS_GUI_COPY_INLINE_STYLE(focus, 2); 

    // Add final counts
    gs_dyn_array_push(iss->style_counts, style_ct[0]);
    gs_dyn_array_push(iss->style_counts, style_ct[1]);
    gs_dyn_array_push(iss->style_counts, style_ct[2]);

    gs_dyn_array_push(iss->animation_counts, anim_ct[0]);
    gs_dyn_array_push(iss->animation_counts, anim_ct[1]);
    gs_dyn_array_push(iss->animation_counts, anim_ct[2]);
}

static void gs_gui_pop_inline_style(gs_gui_context_t* ctx, gs_gui_element_type elementid)
{
    if (elementid >= GS_GUI_ELEMENT_COUNT)
    {
        return;
    } 

    if (!gs_hash_table_exists(ctx->inline_styles, elementid)) 
    {
        return;
    }

    gs_gui_inline_style_stack_t* iss = gs_hash_table_getp(ctx->inline_styles, elementid);
    gs_assert(iss); 

    if (gs_dyn_array_size(iss->style_counts) >= 3)
    {
        const uint32_t sz = gs_dyn_array_size(iss->style_counts);
        uint32_t c0 = iss->style_counts[sz - 3];   // default
        uint32_t c1 = iss->style_counts[sz - 2];   // hover
        uint32_t c2 = iss->style_counts[sz - 1];   // focus
        
        // Pop off elements
        if (iss->styles[0]) gs_dyn_array_head(iss->styles[0])->size -= c0;
        if (iss->styles[1]) gs_dyn_array_head(iss->styles[1])->size -= c1;
        if (iss->styles[2]) gs_dyn_array_head(iss->styles[2])->size -= c2;
    }
    
    if (gs_dyn_array_size(iss->animation_counts) >= 3)
    {
        const uint32_t sz = gs_dyn_array_size(iss->animation_counts);
        uint32_t c0 = iss->animation_counts[sz - 3];   // default
        uint32_t c1 = iss->animation_counts[sz - 2];   // hover
        uint32_t c2 = iss->animation_counts[sz - 1];   // focus
        
        // Pop off elements
        if (iss->animations[0]) gs_dyn_array_head(iss->animations[0])->size -= c0;
        if (iss->animations[1]) gs_dyn_array_head(iss->animations[1])->size -= c1;
        if (iss->animations[2]) gs_dyn_array_head(iss->animations[2])->size -= c2;
    }
}

static void gs_gui_pop_style(gs_gui_context_t* ctx, gs_gui_style_t* style)
{
    ctx->style = style;
} 

static void gs_gui_push_layout(gs_gui_context_t *ctx, gs_gui_rect_t body, gs_vec2 scroll) 
{
	gs_gui_layout_t layout;
	int32_t width = 0;
	memset(&layout, 0, sizeof(layout));
	layout.body = gs_gui_rect(body.x - scroll.x, body.y - scroll.y, body.w, body.h);
	layout.max = gs_v2(-0x1000000, -0x1000000);
    layout.direction = ctx->style->direction; 
    layout.justify_content = ctx->style->justify_content;
    layout.align_content = ctx->style->align_content;
    memcpy(layout.padding, ctx->style->padding, sizeof(int32_t) * 4);
	gs_gui_stack_push(ctx->layout_stack, layout);
	gs_gui_layout_row(ctx, 1, &width, 0);
} 

static void gs_gui_pop_layout(gs_gui_context_t* ctx)
{
	gs_gui_stack_pop(ctx->layout_stack);
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
		int32_t maxscroll = (int32_t)(cs.y - b->h);										    \
																						    \
		if (maxscroll > 0 && b->h > 0) {													\
			gs_gui_rect_t base, thumb;														\
			gs_gui_id id = gs_gui_get_id(ctx, "!scrollbar" #y, 11);							\
			const int32_t elementid = GS_GUI_ELEMENT_SCROLL;                                \
            gs_gui_style_t style = gs_default_val();                                        \
            gs_gui_animation_t* anim = gs_gui_get_animation(ctx, id, desc, elementid);      \
                                                                                            \
            /* Update anim (keep states locally within animation, only way to do this)*/    \
            if (anim)                                                                       \
            {                                                                               \
                gs_gui_animation_update(ctx, anim);                                         \
                                                                                            \
                /* Get blended style based on animation*/                                   \
                style = gs_gui_animation_get_blend_style(ctx, anim, desc, elementid);       \
            }                                                                               \
            else                                                                            \
            {                                                                               \
                style = ctx->focus == id ?                                                  \
                            gs_gui_get_current_element_style(ctx, desc, elementid, 0x02) :  \
                        ctx->hover == id ?                                                  \
                            gs_gui_get_current_element_style(ctx, desc, elementid, 0x01) :  \
                            gs_gui_get_current_element_style(ctx, desc, elementid, 0x00);   \
            }                                                                               \
                                                                                            \
	        int32_t sz = (int32_t)style.size[0];                                            \
            if (cs.y > cnt->body.h) {body->w -= sz;}                                        \
            if (cs.x > cnt->body.w) {body->h -= sz;}                                        \
                                                                                            \
			/* get sizing / positioning */													\
			base = *b;																	    \
			base.x = b->x + b->w;															\
			base.w = style.size[0];											                \
																							\
			/* handle input */																\
			gs_gui_update_control(ctx, id, base, 0);										\
			if (ctx->focus == id && ctx->mouse_down == GS_GUI_MOUSE_LEFT) {					\
				cnt->scroll.y += ctx->mouse_delta.y * cs.y / base.h;					    \
			}																				\
			/* clamp scroll to limits */													\
			cnt->scroll.y = gs_clamp(cnt->scroll.y, 0, maxscroll);					        \
            int32_t state = ctx->focus == id ? GS_GUI_ELEMENT_STATE_FOCUS :                 \
                            ctx->hover == id ? GS_GUI_ELEMENT_STATE_HOVER : 0x00;           \
																							\
			/* draw base and thumb */														\
            gs_gui_draw_rect(ctx, base, style.colors[GS_GUI_COLOR_BACKGROUND]);             \
            /* draw border*/                                                                \
            if (style.colors[GS_GUI_COLOR_BORDER].a)                                        \
            {                                                                               \
                gs_gui_draw_box(ctx, gs_gui_expand_rect(base, (int16_t*)style.border_width),          \
                        (int16_t*)style.border_width, style.colors[GS_GUI_COLOR_BORDER]);             \
            }                                                                               \
            float pl = ((float)style.padding[GS_GUI_PADDING_LEFT]);                         \
            float pr = ((float)style.padding[GS_GUI_PADDING_RIGHT]);                        \
            float pt = ((float)style.padding[GS_GUI_PADDING_TOP]);                          \
            float pb = ((float)style.padding[GS_GUI_PADDING_BOTTOM]);                       \
            float w = ((float)base.w - pr);                                                 \
            float x = (float)(base.x + pl);                                                 \
			thumb = base;																    \
			thumb.x = x; thumb.w = w;													    \
			thumb.h = gs_max(style.thumb_size, base.h * b->h / cs.y) - pb;			        \
			thumb.y += cnt->scroll.y * (base.h - thumb.h) / maxscroll + pt;				    \
            gs_gui_draw_rect(ctx, thumb, style.colors[GS_GUI_COLOR_CONTENT]);               \
            /* draw border*/                                                                \
            if (style.colors[GS_GUI_COLOR_BORDER].a)                                        \
            {                                                                               \
                gs_gui_draw_box(ctx, gs_gui_expand_rect(thumb, (int16_t*)style.border_width),         \
                        (int16_t*)style.border_width, style.colors[GS_GUI_COLOR_BORDER]);             \
            }                                                                               \
																							\
			/* set this as the scroll_target (will get scrolled on mousewheel) */           \
			/* if the mouse is over it */													\
			if (                                                                            \
                gs_gui_mouse_over(ctx, *b) ||                                               \
                gs_gui_mouse_over(ctx, base) ||                                             \
                gs_gui_mouse_over(ctx, thumb)                                               \
            )                                                                               \
            {                                                                               \
                ctx->scroll_target = cnt;                                                   \
            }				                                                                \
        }                                                                                   \
	} while (0) 

static void gs_gui_scrollbars(gs_gui_context_t* ctx, gs_gui_container_t* cnt, gs_gui_rect_t* body, const gs_gui_selector_desc_t* desc, int32_t opt) 
{
	int32_t sz = (int32_t)ctx->style_sheet->styles[GS_GUI_ELEMENT_SCROLL][0x00].size[0];
	gs_vec2 cs = cnt->content_size;
	cs.x += ctx->style->padding[GS_GUI_PADDING_LEFT] * 2;
	cs.y += ctx->style->padding[GS_GUI_PADDING_TOP] * 2;
	gs_gui_push_clip_rect(ctx, *body);

	/* resize body to make room for scrollbars */
	if (cs.y > cnt->body.h) { body->w -= sz; }
	if (cs.x > cnt->body.w) { body->h -= sz; } 

	/* to create a horizontal or vertical scrollbar almost-identical code is
	** used; only the references to `x|y` `w|h` need to be switched */
	gs_gui_scrollbar(ctx, cnt, body, cs, x, y, w, h);

    if (~opt & GS_GUI_OPT_NOSCROLLHORIZONTAL)
    {
	    gs_gui_scrollbar(ctx, cnt, body, cs, y, x, h, w);
    }

    if (cs.y <= cnt->body.h) {cnt->scroll.y = 0;}
    if (cs.x <= cnt->body.w) {cnt->scroll.x = 0;}

	gs_gui_pop_clip_rect(ctx);
}


static void gs_gui_push_container_body(gs_gui_context_t *ctx, gs_gui_container_t *cnt, gs_gui_rect_t body, const gs_gui_selector_desc_t* desc, int32_t opt) 
{
	if (~opt & GS_GUI_OPT_NOSCROLL) {gs_gui_scrollbars(ctx, cnt, &body, desc, opt);}
    int32_t* padding = ctx->style->padding;
    float l = body.x + padding[GS_GUI_PADDING_LEFT];
    float t = body.y + padding[GS_GUI_PADDING_TOP];
    float r = body.x + body.w - padding[GS_GUI_PADDING_RIGHT];
    float b = body.y + body.h - padding[GS_GUI_PADDING_BOTTOM];

    gs_gui_rect_t rect = gs_gui_rect(l, t, r - l, b - t);
	gs_gui_push_layout(ctx, rect, cnt->scroll);
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

static void gs_gui_root_container_end(gs_gui_context_t *ctx) 
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

#define GS_GUI_COPY_STYLES(DST, SRC, ELEM)\
    do {\
        DST[ELEM][0x00] = SRC[ELEM][0x00];\
        DST[ELEM][0x01] = SRC[ELEM][0x01];\
        DST[ELEM][0x02] = SRC[ELEM][0x02];\
    } while (0)

GS_API_DECL gs_gui_style_sheet_t gs_gui_style_sheet_create(gs_gui_context_t* ctx, gs_gui_style_sheet_desc_t* desc)
{
    // Generate new style sheet based on default element styles
    gs_gui_style_sheet_t style_sheet = gs_default_val(); 

    // Copy all default styles
    GS_GUI_COPY_STYLES(style_sheet.styles, gs_gui_default_style_sheet.styles, GS_GUI_ELEMENT_CONTAINER);
    GS_GUI_COPY_STYLES(style_sheet.styles, gs_gui_default_style_sheet.styles, GS_GUI_ELEMENT_LABEL);
    GS_GUI_COPY_STYLES(style_sheet.styles, gs_gui_default_style_sheet.styles, GS_GUI_ELEMENT_TEXT);
    GS_GUI_COPY_STYLES(style_sheet.styles, gs_gui_default_style_sheet.styles, GS_GUI_ELEMENT_PANEL);
    GS_GUI_COPY_STYLES(style_sheet.styles, gs_gui_default_style_sheet.styles, GS_GUI_ELEMENT_INPUT);
    GS_GUI_COPY_STYLES(style_sheet.styles, gs_gui_default_style_sheet.styles, GS_GUI_ELEMENT_BUTTON);
    GS_GUI_COPY_STYLES(style_sheet.styles, gs_gui_default_style_sheet.styles, GS_GUI_ELEMENT_SCROLL); 
    GS_GUI_COPY_STYLES(style_sheet.styles, gs_gui_default_style_sheet.styles, GS_GUI_ELEMENT_IMAGE); 

// GS_API_DECL void gs_gui_style_sheet_set_element_styles(gs_gui_style_sheet_t* style_sheet, gs_gui_element_type element, gs_gui_element_state state, gs_gui_style_element_t* styles, size_t size);

#define GS_GUI_APPLY_STYLE_ELEMENT(ELEMENT, TYPE)\
    do {\
        if ((ELEMENT).all.style.data)\
        {\
            gs_gui_style_sheet_set_element_styles(&style_sheet, TYPE, GS_GUI_ELEMENT_STATE_NEG, (ELEMENT).all.style.data, (ELEMENT).all.style.size);\
        }\
        else if ((ELEMENT).def.style.data)\
        {\
            gs_gui_style_sheet_set_element_styles(&style_sheet, TYPE, GS_GUI_ELEMENT_STATE_DEFAULT, (ELEMENT).def.style.data, (ELEMENT).def.style.size);\
        }\
        if ((ELEMENT).hover.style.data)\
        {\
            gs_gui_style_sheet_set_element_styles(&style_sheet, TYPE, GS_GUI_ELEMENT_STATE_HOVER, (ELEMENT).hover.style.data, (ELEMENT).hover.style.size);\
        }\
        if ((ELEMENT).focus.style.data)\
        {\
            gs_gui_style_sheet_set_element_styles(&style_sheet, TYPE, GS_GUI_ELEMENT_STATE_FOCUS, (ELEMENT).focus.style.data, (ELEMENT).focus.style.size);\
        }\
    } while (0)

    // Iterate through descriptor
    if (desc)
    { 
        GS_GUI_APPLY_STYLE_ELEMENT(desc->button, GS_GUI_ELEMENT_BUTTON);
        GS_GUI_APPLY_STYLE_ELEMENT(desc->container, GS_GUI_ELEMENT_CONTAINER);
        GS_GUI_APPLY_STYLE_ELEMENT(desc->panel, GS_GUI_ELEMENT_PANEL);
        GS_GUI_APPLY_STYLE_ELEMENT(desc->scroll, GS_GUI_ELEMENT_SCROLL);
        GS_GUI_APPLY_STYLE_ELEMENT(desc->image, GS_GUI_ELEMENT_IMAGE);
        GS_GUI_APPLY_STYLE_ELEMENT(desc->label, GS_GUI_ELEMENT_LABEL);
        GS_GUI_APPLY_STYLE_ELEMENT(desc->text, GS_GUI_ELEMENT_TEXT);
    }

#define COPY_ANIM_DATA(TYPE, ELEMENT)\
    do {\
        /* Apply animations */\
        if (desc->TYPE.all.animation.data)\
        {\
            int32_t cnt = desc->TYPE.all.animation.size / sizeof(gs_gui_animation_property_t);\
            if (!gs_hash_table_exists(style_sheet.animations, ELEMENT)) {\
                gs_gui_animation_property_list_t v = gs_default_val();\
                gs_hash_table_insert(style_sheet.animations, ELEMENT, v);\
            }\
            gs_gui_animation_property_list_t* list = gs_hash_table_getp(style_sheet.animations, ELEMENT);\
            gs_assert(list);\
            /* Register animation properties for all */\
            for (uint32_t i = 0; i < 3; ++i)\
            {\
                for (uint32_t c = 0; c < cnt; ++c)\
                {\
                    gs_dyn_array_push(list->properties[i], desc->TYPE.all.animation.data[c]);\
                }\
            }\
        }\
    } while (0)

    // Copy animations
    COPY_ANIM_DATA(button, GS_GUI_ELEMENT_BUTTON);
    COPY_ANIM_DATA(label, GS_GUI_ELEMENT_LABEL);
    COPY_ANIM_DATA(scroll, GS_GUI_ELEMENT_SCROLL);
    COPY_ANIM_DATA(image, GS_GUI_ELEMENT_IMAGE);
    COPY_ANIM_DATA(panel, GS_GUI_ELEMENT_PANEL);
    COPY_ANIM_DATA(text, GS_GUI_ELEMENT_TEXT);
    COPY_ANIM_DATA(container, GS_GUI_ELEMENT_CONTAINER);

    return style_sheet;
} 

GS_API_DECL void gs_gui_style_sheet_destroy(gs_gui_context_t* ctx, gs_gui_style_sheet_t* ss)
{
    // Need to free all animations
    if (!ss || !ss->animations) {
        gs_log_warning("Trying to destroy invalid style sheet");
        return;
    }

    for (
        gs_hash_table_iter it = gs_hash_table_iter_new(ss->animations);
        gs_hash_table_iter_valid(ss->animations, it);
        gs_hash_table_iter_advance(ss->animations, it)
    )
    {
        gs_gui_animation_property_list_t* list = gs_hash_table_iter_getp(ss->animations, it);
        for (uint32_t i = 0; i < 3; ++i)
        {
            gs_dyn_array_free(list->properties[i]);
        }
    }
    gs_hash_table_free(ss->animations);
}

GS_API_DECL void gs_gui_set_style_sheet(gs_gui_context_t* ctx, gs_gui_style_sheet_t* style_sheet)
{
    ctx->style_sheet = style_sheet ? style_sheet : &gs_gui_default_style_sheet;
}

GS_API_DECL void gs_gui_style_sheet_set_element_styles(gs_gui_style_sheet_t* ss, gs_gui_element_type element, gs_gui_element_state state, gs_gui_style_element_t* styles, size_t size)
{
    const uint32_t count = size / sizeof(gs_gui_style_element_t);
    uint32_t idx_cnt = 1;
    uint32_t idx = 0;

    // Switch on state
    switch (state)
    {
        // Do all
        default:                            idx_cnt = 3; break;
        case GS_GUI_ELEMENT_STATE_DEFAULT:  idx = 0; break;
        case GS_GUI_ELEMENT_STATE_HOVER:    idx = 1; break;
        case GS_GUI_ELEMENT_STATE_FOCUS:    idx = 2; break;
    }

    for (uint32_t s = idx, c = 0; c < idx_cnt; ++s, ++c)
    {
        gs_gui_style_t* cs = &ss->styles[element][s];
        for (uint32_t i = 0; i < count; ++i)
        {
            gs_gui_style_element_t* se = &styles[i];

            switch (se->type)
            {
                // Width/Height
                case GS_GUI_STYLE_WIDTH:            cs->size[0] = (float)se->value; break;
                case GS_GUI_STYLE_HEIGHT:           cs->size[1] = (float)se->value; break;

                // Padding
                case GS_GUI_STYLE_PADDING: {
                    cs->padding[GS_GUI_PADDING_LEFT] = (int32_t)se->value;
                    cs->padding[GS_GUI_PADDING_TOP] = (int32_t)se->value;
                    cs->padding[GS_GUI_PADDING_RIGHT] = (int32_t)se->value;
                    cs->padding[GS_GUI_PADDING_BOTTOM] = (int32_t)se->value;
                } 
                case GS_GUI_STYLE_PADDING_LEFT:     cs->padding[GS_GUI_PADDING_LEFT] = (int32_t)se->value; break;     
                case GS_GUI_STYLE_PADDING_TOP:      cs->padding[GS_GUI_PADDING_TOP] = (int32_t)se->value; break;
                case GS_GUI_STYLE_PADDING_RIGHT:    cs->padding[GS_GUI_PADDING_RIGHT] = (int32_t)se->value; break;
                case GS_GUI_STYLE_PADDING_BOTTOM:   cs->padding[GS_GUI_PADDING_BOTTOM] = (int32_t)se->value; break;

                case GS_GUI_STYLE_MARGIN: {
                    cs->margin[GS_GUI_MARGIN_LEFT] = (int32_t)se->value;
                    cs->margin[GS_GUI_MARGIN_TOP] = (int32_t)se->value;
                    cs->margin[GS_GUI_MARGIN_RIGHT] = (int32_t)se->value;
                    cs->margin[GS_GUI_MARGIN_BOTTOM] = (int32_t)se->value;
                } break;           

                case GS_GUI_STYLE_MARGIN_LEFT:      cs->margin[GS_GUI_MARGIN_LEFT] = (int32_t)se->value; break;
                case GS_GUI_STYLE_MARGIN_TOP:       cs->margin[GS_GUI_MARGIN_TOP] = (int32_t)se->value; break;
                case GS_GUI_STYLE_MARGIN_RIGHT:     cs->margin[GS_GUI_MARGIN_RIGHT] = (int32_t)se->value; break;
                case GS_GUI_STYLE_MARGIN_BOTTOM:    cs->margin[GS_GUI_MARGIN_BOTTOM] = (int32_t)se->value; break;

                // Border
                case GS_GUI_STYLE_BORDER_RADIUS: {
                    cs->border_radius[0] = se->value;
                    cs->border_radius[1] = se->value;
                    cs->border_radius[2] = se->value;
                    cs->border_radius[3] = se->value;
                } break;

                case GS_GUI_STYLE_BORDER_RADIUS_LEFT:   cs->border_radius[0] = se->value; break;
                case GS_GUI_STYLE_BORDER_RADIUS_RIGHT:  cs->border_radius[1] = se->value; break;
                case GS_GUI_STYLE_BORDER_RADIUS_TOP:    cs->border_radius[2] = se->value; break;
                case GS_GUI_STYLE_BORDER_RADIUS_BOTTOM: cs->border_radius[3] = se->value; break;

                case GS_GUI_STYLE_BORDER_WIDTH: {     
                    cs->border_width[0] = se->value;
                    cs->border_width[1] = se->value;
                    cs->border_width[2] = se->value;
                    cs->border_width[3] = se->value;
                } break;

                case GS_GUI_STYLE_BORDER_WIDTH_LEFT:    cs->border_width[0] = se->value; break;     
                case GS_GUI_STYLE_BORDER_WIDTH_RIGHT:   cs->border_width[1] = se->value; break;     
                case GS_GUI_STYLE_BORDER_WIDTH_TOP:     cs->border_width[2] = se->value; break;     
                case GS_GUI_STYLE_BORDER_WIDTH_BOTTOM:  cs->border_width[3] = se->value; break;     

                // Flex
                case GS_GUI_STYLE_DIRECTION:        cs->direction = (int32_t)se->value; break;
                case GS_GUI_STYLE_ALIGN_CONTENT:    cs->align_content = (int32_t)se->value; break;
                case GS_GUI_STYLE_JUSTIFY_CONTENT:  cs->justify_content = (int32_t)se->value; break;

                // Shadow
                case GS_GUI_STYLE_SHADOW_X:         cs->shadow_x = (int32_t)se->value; break;
                case GS_GUI_STYLE_SHADOW_Y:         cs->shadow_y = (int32_t)se->value; break;

                // Colors
                case GS_GUI_STYLE_COLOR_BACKGROUND:         cs->colors[GS_GUI_COLOR_BACKGROUND] = se->color; break;
                case GS_GUI_STYLE_COLOR_BORDER:             cs->colors[GS_GUI_COLOR_BORDER] = se->color; break;
                case GS_GUI_STYLE_COLOR_SHADOW:             cs->colors[GS_GUI_COLOR_SHADOW] = se->color; break;
                case GS_GUI_STYLE_COLOR_CONTENT:            cs->colors[GS_GUI_COLOR_CONTENT] = se->color; break;
                case GS_GUI_STYLE_COLOR_CONTENT_BACKGROUND: cs->colors[GS_GUI_COLOR_CONTENT_BACKGROUND] = se->color; break;
                case GS_GUI_STYLE_COLOR_CONTENT_BORDER:     cs->colors[GS_GUI_COLOR_CONTENT_BORDER] = se->color; break;
                case GS_GUI_STYLE_COLOR_CONTENT_SHADOW:     cs->colors[GS_GUI_COLOR_CONTENT_SHADOW] = se->color; break;

                // Font
                case GS_GUI_STYLE_FONT:             cs->font = se->font; break;
            }
        }
    }
}

GS_API_DECL void gs_gui_set_element_style(gs_gui_context_t* ctx, gs_gui_element_type element, gs_gui_element_state state, gs_gui_style_element_t* style, size_t size)
{
    const uint32_t count = size / sizeof(gs_gui_style_element_t);
    uint32_t idx_cnt = 1;
    uint32_t idx = 0;

    // Switch on state
    switch (state)
    {
        // Do all
        default:                            idx_cnt = 3; break;
        case GS_GUI_ELEMENT_STATE_DEFAULT:  idx = 0; break;
        case GS_GUI_ELEMENT_STATE_HOVER:    idx = 1; break;
        case GS_GUI_ELEMENT_STATE_FOCUS:    idx = 2; break;
    }

    for (uint32_t s = idx, c = 0; c < idx_cnt; ++s, ++c)
    {
        gs_gui_style_t* cs = &ctx->style_sheet->styles[element][s];
        for (uint32_t i = 0; i < count; ++i)
        {
            gs_gui_style_element_t* se = &style[i];

            switch (se->type)
            {
                // Width/Height
                case GS_GUI_STYLE_WIDTH:            cs->size[0] = (float)se->value; break;
                case GS_GUI_STYLE_HEIGHT:           cs->size[1] = (float)se->value; break;

                // Padding
                case GS_GUI_STYLE_PADDING: {
                    cs->padding[GS_GUI_PADDING_LEFT] = (int32_t)se->value;
                    cs->padding[GS_GUI_PADDING_TOP] = (int32_t)se->value;
                    cs->padding[GS_GUI_PADDING_RIGHT] = (int32_t)se->value;
                    cs->padding[GS_GUI_PADDING_BOTTOM] = (int32_t)se->value;
                } 
                case GS_GUI_STYLE_PADDING_LEFT:     cs->padding[GS_GUI_PADDING_LEFT] = (int32_t)se->value; break;     
                case GS_GUI_STYLE_PADDING_TOP:      cs->padding[GS_GUI_PADDING_TOP] = (int32_t)se->value; break;
                case GS_GUI_STYLE_PADDING_RIGHT:    cs->padding[GS_GUI_PADDING_RIGHT] = (int32_t)se->value; break;
                case GS_GUI_STYLE_PADDING_BOTTOM:   cs->padding[GS_GUI_PADDING_BOTTOM] = (int32_t)se->value; break;

                case GS_GUI_STYLE_MARGIN: {
                    cs->margin[GS_GUI_MARGIN_LEFT] = (int32_t)se->value;
                    cs->margin[GS_GUI_MARGIN_TOP] = (int32_t)se->value;
                    cs->margin[GS_GUI_MARGIN_RIGHT] = (int32_t)se->value;
                    cs->margin[GS_GUI_MARGIN_BOTTOM] = (int32_t)se->value;
                } break;           

                case GS_GUI_STYLE_MARGIN_LEFT:      cs->margin[GS_GUI_MARGIN_LEFT] = (int32_t)se->value; break;
                case GS_GUI_STYLE_MARGIN_TOP:       cs->margin[GS_GUI_MARGIN_TOP] = (int32_t)se->value; break;
                case GS_GUI_STYLE_MARGIN_RIGHT:     cs->margin[GS_GUI_MARGIN_RIGHT] = (int32_t)se->value; break;
                case GS_GUI_STYLE_MARGIN_BOTTOM:    cs->margin[GS_GUI_MARGIN_BOTTOM] = (int32_t)se->value; break;

                // Border
                case GS_GUI_STYLE_BORDER_RADIUS: {
                    cs->border_radius[0] = se->value;
                    cs->border_radius[1] = se->value;
                    cs->border_radius[2] = se->value;
                    cs->border_radius[3] = se->value;
                } break;

                case GS_GUI_STYLE_BORDER_RADIUS_LEFT:   cs->border_radius[0] = se->value; break;
                case GS_GUI_STYLE_BORDER_RADIUS_RIGHT:  cs->border_radius[1] = se->value; break;
                case GS_GUI_STYLE_BORDER_RADIUS_TOP:    cs->border_radius[2] = se->value; break;
                case GS_GUI_STYLE_BORDER_RADIUS_BOTTOM: cs->border_radius[3] = se->value; break;

                case GS_GUI_STYLE_BORDER_WIDTH: {     
                    cs->border_width[0] = se->value;
                    cs->border_width[1] = se->value;
                    cs->border_width[2] = se->value;
                    cs->border_width[3] = se->value;
                } break;

                case GS_GUI_STYLE_BORDER_WIDTH_LEFT:    cs->border_width[0] = se->value; break;     
                case GS_GUI_STYLE_BORDER_WIDTH_RIGHT:   cs->border_width[1] = se->value; break;     
                case GS_GUI_STYLE_BORDER_WIDTH_TOP:     cs->border_width[2] = se->value; break;     
                case GS_GUI_STYLE_BORDER_WIDTH_BOTTOM:  cs->border_width[3] = se->value; break;     

                // Flex
                case GS_GUI_STYLE_DIRECTION:        cs->direction = (int32_t)se->value; break;
                case GS_GUI_STYLE_ALIGN_CONTENT:    cs->align_content = (int32_t)se->value; break;
                case GS_GUI_STYLE_JUSTIFY_CONTENT:  cs->justify_content = (int32_t)se->value; break;

                // Shadow
                case GS_GUI_STYLE_SHADOW_X:         cs->shadow_x = (int32_t)se->value; break;
                case GS_GUI_STYLE_SHADOW_Y:         cs->shadow_y = (int32_t)se->value; break;

                // Colors
                case GS_GUI_STYLE_COLOR_BACKGROUND:         cs->colors[GS_GUI_COLOR_BACKGROUND] = se->color; break;
                case GS_GUI_STYLE_COLOR_BORDER:             cs->colors[GS_GUI_COLOR_BORDER] = se->color; break;
                case GS_GUI_STYLE_COLOR_SHADOW:             cs->colors[GS_GUI_COLOR_SHADOW] = se->color; break;
                case GS_GUI_STYLE_COLOR_CONTENT:            cs->colors[GS_GUI_COLOR_CONTENT] = se->color; break;
                case GS_GUI_STYLE_COLOR_CONTENT_BACKGROUND: cs->colors[GS_GUI_COLOR_CONTENT_BACKGROUND] = se->color; break;
                case GS_GUI_STYLE_COLOR_CONTENT_BORDER:     cs->colors[GS_GUI_COLOR_CONTENT_BORDER] = se->color; break;
                case GS_GUI_STYLE_COLOR_CONTENT_SHADOW:     cs->colors[GS_GUI_COLOR_CONTENT_SHADOW] = se->color; break;

                // Font
                case GS_GUI_STYLE_FONT:             cs->font = se->font; break;
            }
        }
    }
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
    cnt->flags |= GS_GUI_WINDOW_FLAGS_VISIBLE | GS_GUI_WINDOW_FLAGS_FIRST_INIT;
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
    uint32_t f = ctx->frame;
    if (!f) gs_gui_begin(ctx, gs_platform_framebuffer_sizev(ctx->window_hndl));
    gs_gui_container_t* dst_cnt = gs_gui_get_container(ctx, dst);
    gs_gui_container_t* src_cnt = gs_gui_get_container(ctx, src); 
    gs_gui_dock_ex_cnt(ctx, dst_cnt, src_cnt, split_type, ratio); 
    if (f != ctx->frame) gs_gui_end(ctx);
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
        split.type = (gs_gui_split_type)split_type;       
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
					gs_gui_tab_item_swap(ctx, (gs_gui_container_t*)tab_bar->items[i].data, +1);
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
						gs_gui_container_t* c = (gs_gui_container_t*)tab_bar->items[i].data;
						c->parent = (gs_gui_container_t*)tab_bar->items[tab_bar->focus].data;
                        tab_bar->items[i].idx = i;
                        tab_bar->items[i].zindex = i;
					}

                    gs_gui_container_t* fcnt = (gs_gui_container_t*)tab_bar->items[tab_bar->focus].data;
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
					gs_gui_tab_item_swap(ctx, (gs_gui_container_t*)tab_bar->items[i].data, +1);
				} 

                for (uint32_t i = 0; i < tab_bar->size; ++i)
                {
                    gs_gui_container_t* fcnt = (gs_gui_container_t*)tab_bar->items[i].data;
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

                    gs_assert((gs_gui_container_t*)tab_bar->items[tab_bar->focus].data != cnt);

                    gs_gui_container_t* fcnt = (gs_gui_container_t*)tab_bar->items[tab_bar->focus].data;
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
					gs_gui_tab_item_swap(ctx, (gs_gui_container_t*)tab_bar->items[i].data, +1);
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
						gs_gui_container_t* c = (gs_gui_container_t*)tab_bar->items[i].data;
						c->parent = (gs_gui_container_t*)tab_bar->items[tab_bar->focus].data;
					} 
				}
            }
            // Only 2 windows left in tab bar
            else
            { 
                for (uint32_t i = 0; i < tab_bar->size; ++i)
                {
                    gs_gui_container_t* fcnt = (gs_gui_container_t*)tab_bar->items[i].data;
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

#define GS_GUI_COPY_STYLE(DST, SRC)\
    do {\
        DST[GS_GUI_ELEMENT_STATE_DEFAULT] = SRC;\
        DST[GS_GUI_ELEMENT_STATE_HOVER] = SRC;\
        DST[GS_GUI_ELEMENT_STATE_FOCUS] = SRC;\
    } while (0)

static void gs_gui_init_default_styles(gs_gui_context_t* ctx)
{ 
    // Set up main default style
    gs_gui_default_style.font = gsi_default_font();
	gs_gui_default_style.size[0] = 68.f;
	gs_gui_default_style.size[1] = 18.f;
	gs_gui_default_style.spacing = 2;
	gs_gui_default_style.indent = 10;
	gs_gui_default_style.title_height = 20;
	gs_gui_default_style.scrollbar_size = 5;
	gs_gui_default_style.thumb_size = 5;

    // Initialize all default styles
    GS_GUI_COPY_STYLE(gs_gui_default_container_style, gs_gui_default_style);
    GS_GUI_COPY_STYLE(gs_gui_default_button_style, gs_gui_default_style);
    GS_GUI_COPY_STYLE(gs_gui_default_text_style, gs_gui_default_style);
    GS_GUI_COPY_STYLE(gs_gui_default_label_style, gs_gui_default_style);
    GS_GUI_COPY_STYLE(gs_gui_default_panel_style, gs_gui_default_style);
    GS_GUI_COPY_STYLE(gs_gui_default_input_style, gs_gui_default_style);
    GS_GUI_COPY_STYLE(gs_gui_default_scroll_style, gs_gui_default_style);
    GS_GUI_COPY_STYLE(gs_gui_default_image_style, gs_gui_default_style);

    gs_gui_style_t* style = NULL;

    // button
    for (uint32_t i = 0; i < 3; ++i) {
        style = &gs_gui_default_button_style[i];
        style->justify_content = GS_GUI_JUSTIFY_CENTER;
    } 
    gs_gui_default_button_style[GS_GUI_ELEMENT_STATE_DEFAULT].colors[GS_GUI_COLOR_BACKGROUND] = gs_color(35, 35, 35, 255); 
    gs_gui_default_button_style[GS_GUI_ELEMENT_STATE_HOVER].colors[GS_GUI_COLOR_BACKGROUND] = gs_color(40, 40, 40, 255); 
    gs_gui_default_button_style[GS_GUI_ELEMENT_STATE_FOCUS].colors[GS_GUI_COLOR_BACKGROUND] = gs_color(0, 214, 121, 255); 

    // panel
    for (uint32_t i = 0; i < 3; ++i) {
        style = &gs_gui_default_panel_style[i];
        style->colors[GS_GUI_COLOR_BACKGROUND] = gs_color(30, 30, 30, 255); 
        style->size[1] = 19; 
    } 

    // input
    for (uint32_t i = 0; i < 3; ++i) {
        style = &gs_gui_default_input_style[i];
        style->colors[GS_GUI_COLOR_BACKGROUND] = gs_color(20, 20, 20, 255); 
        style->size[1] = 19; 
    } 

    // text
    for (uint32_t i = 0; i < 3; ++i) {
        style = &gs_gui_default_text_style[i];
        style->colors[GS_GUI_COLOR_BACKGROUND] = gs_color(0, 0, 0, 0);
        style->colors[GS_GUI_COLOR_CONTENT] = GS_COLOR_WHITE;
    } 

    // label
    for (uint32_t i = 0; i < 3; ++i) {
        style = &gs_gui_default_label_style[i];
        style->colors[GS_GUI_COLOR_BACKGROUND] = gs_color(0, 0, 0, 0);
        style->colors[GS_GUI_COLOR_CONTENT] = GS_COLOR_WHITE; 
        style->size[1] = 19; 
    } 

    // scroll 
    for (uint32_t i = 0; i < 3; ++i) {
        style = &gs_gui_default_scroll_style[i];
        style->size[0] = 10;
        style->padding[GS_GUI_PADDING_RIGHT] = 4;
    } 

    style = &gs_gui_default_scroll_style[GS_GUI_ELEMENT_STATE_DEFAULT];
    style->colors[GS_GUI_COLOR_BACKGROUND] = gs_color(22, 22, 22, 255);
    style->colors[GS_GUI_COLOR_CONTENT] = gs_color(255, 255, 255, 100); 

#define GS_GUI_COPY(DST, SRC)\
    do {\
        DST[0x00] = SRC[0x00];\
        DST[0x01] = SRC[0x01];\
        DST[0x02] = SRC[0x02];\
    } while (0)

    GS_GUI_COPY(gs_gui_default_style_sheet.styles[GS_GUI_ELEMENT_CONTAINER], gs_gui_default_container_style);
    GS_GUI_COPY(gs_gui_default_style_sheet.styles[GS_GUI_ELEMENT_LABEL], gs_gui_default_label_style);
    GS_GUI_COPY(gs_gui_default_style_sheet.styles[GS_GUI_ELEMENT_TEXT], gs_gui_default_text_style);
    GS_GUI_COPY(gs_gui_default_style_sheet.styles[GS_GUI_ELEMENT_PANEL], gs_gui_default_panel_style);
    GS_GUI_COPY(gs_gui_default_style_sheet.styles[GS_GUI_ELEMENT_INPUT], gs_gui_default_input_style);
    GS_GUI_COPY(gs_gui_default_style_sheet.styles[GS_GUI_ELEMENT_BUTTON], gs_gui_default_button_style);
    GS_GUI_COPY(gs_gui_default_style_sheet.styles[GS_GUI_ELEMENT_SCROLL], gs_gui_default_scroll_style);
    GS_GUI_COPY(gs_gui_default_style_sheet.styles[GS_GUI_ELEMENT_IMAGE], gs_gui_default_image_style);

    ctx->style_sheet = &gs_gui_default_style_sheet;
	ctx->style = &ctx->style_sheet->styles[GS_GUI_ELEMENT_CONTAINER][0x00]; 
} 

static char button_map[256] = gs_default_val(); 
static char key_map[512] = gs_default_val();

GS_API_DECL gs_gui_context_t gs_gui_new(uint32_t window_hndl)
{
    gs_gui_context_t ctx = gs_default_val();
    gs_gui_init(&ctx, window_hndl);
    return ctx;
}

GS_API_DECL void gs_gui_init(gs_gui_context_t *ctx, uint32_t window_hndl)
{ 
	memset(ctx, 0, sizeof(*ctx));
    ctx->gsi = gs_immediate_draw_new(); 
    ctx->overlay_draw_list = gs_immediate_draw_new();
    gs_gui_init_default_styles(ctx);
    ctx->window_hndl = window_hndl;
    ctx->last_zindex = 1000;
    gs_slot_array_reserve(ctx->splits, GS_GUI_GS_GUI_SPLIT_SIZE);
    gs_gui_split_t split = gs_default_val();
    gs_slot_array_insert(ctx->splits, split); // First item is set for 0x00 invalid
    gs_slot_array_reserve(ctx->tab_bars, GS_GUI_GS_GUI_TAB_SIZE);
    gs_gui_tab_bar_t tb = gs_default_val();
    gs_slot_array_insert(ctx->tab_bars, tb);

    button_map[GS_MOUSE_LBUTTON & 0xff] = GS_GUI_MOUSE_LEFT;
    button_map[GS_MOUSE_RBUTTON & 0xff] = GS_GUI_MOUSE_RIGHT;
    button_map[GS_MOUSE_MBUTTON & 0xff] = GS_GUI_MOUSE_MIDDLE;

    key_map[GS_KEYCODE_LEFT_SHIFT    & 0xff] = GS_GUI_KEY_SHIFT;
    key_map[GS_KEYCODE_RIGHT_SHIFT   & 0xff] = GS_GUI_KEY_SHIFT;
    key_map[GS_KEYCODE_LEFT_CONTROL  & 0xff] = GS_GUI_KEY_CTRL;
    key_map[GS_KEYCODE_RIGHT_CONTROL & 0xff] = GS_GUI_KEY_CTRL;
    key_map[GS_KEYCODE_LEFT_ALT      & 0xff] = GS_GUI_KEY_ALT;
    key_map[GS_KEYCODE_RIGHT_ALT     & 0xff] = GS_GUI_KEY_ALT;
    key_map[GS_KEYCODE_ENTER         & 0xff] = GS_GUI_KEY_RETURN;
    key_map[GS_KEYCODE_BACKSPACE     & 0xff] = GS_GUI_KEY_BACKSPACE;
} 

GS_API_DECL void gs_gui_init_font_stash(gs_gui_context_t* ctx, gs_gui_font_stash_desc_t* desc)
{
    gs_hash_table_clear(ctx->font_stash);
    uint32_t ct = sizeof(gs_gui_font_desc_t) / desc->size;
    for (uint32_t i = 0; i < ct; ++i) {
        gs_hash_table_insert(ctx->font_stash, gs_hash_str64(desc->fonts[i].key), desc->fonts[i].font);
    }
}

GS_API_DECL gs_gui_context_t gs_gui_context_new(uint32_t window_hndl)
{
    gs_gui_context_t gui = gs_default_val();
    gs_gui_init(&gui, window_hndl);
    return gui;
}

GS_API_DECL void gs_gui_free(gs_gui_context_t* ctx)
{
    // lulz...
}

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
    gs_gui_container_t* hover_cnt = ctx->hover_root ? ctx->hover_root :
	    			    ctx->next_hover_root ? ctx->next_hover_root : 
				    NULL;
    bool valid_hover = hover_cnt && hover_cnt->zindex > top->zindex;
    valid_hover = false;
    bool valid = true;

    split->frame = ctx->frame;
    root_split->frame = ctx->frame; 

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

            int16_t exp[4] = {1, 1, 1, 1};
			gs_gui_rect_t expand = gs_gui_expand_rect(r, exp);
            bool hover = !valid_hover && !ctx->focus && !ctx->prev_dockable_root && gs_gui_rect_overlaps_vec2(expand, ctx->mouse_pos) && !ctx->lock_hover_id; 
            if (hover) ctx->next_hover_split = split;
            if (hover && ctx->mouse_down == GS_GUI_MOUSE_LEFT)
            {
                if (!ctx->focus_split) ctx->focus_split = split;
            } 
            bool active = ctx->focus_split == split;
            if (active && valid)
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
                gs_color_t bc = ctx->focus_split == split ? 
                    ctx->style_sheet->styles[GS_GUI_ELEMENT_BUTTON][GS_GUI_ELEMENT_STATE_FOCUS].colors[GS_GUI_COLOR_BACKGROUND] : 
                    ctx->style_sheet->styles[GS_GUI_ELEMENT_BUTTON][GS_GUI_ELEMENT_STATE_HOVER].colors[GS_GUI_COLOR_BACKGROUND];
                gs_gui_draw_rect(ctx, r, bc);
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

                int16_t exp[] = {1, 1, 1, 1};
				gs_gui_rect_t expand = gs_gui_expand_rect(r, exp);
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
                    gs_color_t bc = ctx->focus_split == split ? 
                        ctx->style_sheet->styles[GS_GUI_ELEMENT_BUTTON][GS_GUI_ELEMENT_STATE_FOCUS].colors[GS_GUI_COLOR_BACKGROUND] : 
                        ctx->style_sheet->styles[GS_GUI_ELEMENT_BUTTON][GS_GUI_ELEMENT_STATE_HOVER].colors[GS_GUI_COLOR_BACKGROUND];
                    gs_gui_draw_rect(ctx, r, bc);
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

GS_API_DECL void gs_gui_begin(gs_gui_context_t* ctx, gs_vec2 framebuffer_size)
{ 
    // Capture event information
    gs_vec2 platform_m_pos = gs_platform_mouse_positionv();
	gs_vec2 window_size = gs_platform_framebuffer_sizev(ctx->window_hndl);
	float percent_x = platform_m_pos.x / window_size.x;
	float percent_y = platform_m_pos.y / window_size.y;
	gs_vec2 mouse_pos = gs_v2(framebuffer_size.x * percent_x, framebuffer_size.y * percent_y);

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
                        // ctx->mouse_pos = evt.mouse.move;
                    } break;

                    case GS_PLATFORM_MOUSE_WHEEL:
                    {
                        gs_gui_input_scroll(ctx, 0, (int32_t)(-evt.mouse.wheel.y * 30.f));
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

    ctx->mouse_pos = mouse_pos;
    ctx->framebuffer_size = framebuffer_size;
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

    // Set up overlay draw list
    gs_vec2 fbs = framebuffer_size;
    gsi_camera2D(&ctx->overlay_draw_list, (uint32_t)fbs.x, (uint32_t)fbs.y);
    gsi_defaults(&ctx->overlay_draw_list);

    /*
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
            int32_t opt = GS_GUI_OPT_NOFRAME | 
                          GS_GUI_OPT_FORCESETRECT | 
                          GS_GUI_OPT_NOMOVE | 
                          GS_GUI_OPT_NOTITLE | 
                          GS_GUI_OPT_NOSCROLL | 
                          GS_GUI_OPT_NOCLIP | 
                          GS_GUI_OPT_NODOCK | 
                          GS_GUI_OPT_DOCKSPACE | 
                          GS_GUI_OPT_NOBORDER;
            gs_gui_window_begin_ex(ctx, TMP, r, NULL, opt); 
            {
                // Set zindex for sorting (always below the bottom most window in this split tree)
                gs_gui_container_t* ds = gs_gui_get_current_container(ctx);
                int32_t zindex = INT32_MAX - 1;
                gs_gui_get_split_lowest_zindex(ctx, split, &zindex);
                if (root_cnt->opt & GS_GUI_OPT_DOCKSPACE) ds->zindex = 0;
                else ds->zindex = gs_clamp((int32_t)zindex - 1, 0, INT32_MAX);

                gs_gui_rect_t fr = split->rect;
                fr.x += GS_GUI_SPLIT_SIZE; fr.y += GS_GUI_SPLIT_SIZE; fr.w -= 2.f * GS_GUI_SPLIT_SIZE; fr.h -= 2.f * GS_GUI_SPLIT_SIZE;
		        // gs_gui_draw_frame(ctx, fr, &ctx->style_sheet->styles[GS_GUI_ELEMENT_CONTAINER][0x00]);

                // Draw splits
                gs_gui_draw_splits(ctx, split);

                // Do resize controls for dockspace
                gs_gui_container_t* top = gs_gui_get_top_most_container(ctx, split);
                const gs_gui_rect_t* sr = &split->rect;
                gs_gui_container_t* hover_cnt = ctx->hover_root ? ctx->hover_root : 
						ctx->next_hover_root ? ctx->next_hover_root : 
						NULL;
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
                        gs_gui_draw_control_frame(ctx, id, lr, GS_GUI_ELEMENT_BUTTON, 0x00);
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
                        gs_gui_draw_control_frame(ctx, id, rr, GS_GUI_ELEMENT_BUTTON, 0x00);
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
                        gs_gui_draw_control_frame(ctx, id, tr, GS_GUI_ELEMENT_BUTTON, 0x00);
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
                        gs_gui_draw_control_frame(ctx, id, br, GS_GUI_ELEMENT_BUTTON, 0x00);
                        ctx->next_hover_root = top;
                        gs_gui_request_t req = gs_default_val();
                        req.type = GS_GUI_SPLIT_RESIZE_S;
                        req.split = split;
                        gs_dyn_array_push(ctx->requests, req);
                    } 
                }
            }
            gs_gui_window_end(ctx);
        } 
    }
    */

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
        gs_color_t def_col = gs_color_alpha(ctx->style_sheet->styles[GS_GUI_ELEMENT_BUTTON][GS_GUI_ELEMENT_STATE_FOCUS].colors[GS_GUI_COLOR_BACKGROUND], 100);
        gs_color_t hov_col = gs_color_alpha(ctx->style_sheet->styles[GS_GUI_ELEMENT_BUTTON][GS_GUI_ELEMENT_STATE_FOCUS].colors[GS_GUI_COLOR_BACKGROUND], 150);

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
                        gs_gui_bring_to_front(ctx, (gs_gui_container_t*)tab_bar->items[i].data); 
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

    // Have to clear style stacks

    // Set previous frame ids 
    // ctx->prev_hover = 0;
    // ctx->prev_focus = ctx->focus; 

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

    if (ctx->mouse_pressed && (!ctx->next_hover_root || ctx->next_hover_root->opt & GS_GUI_OPT_NOFOCUS))
    {
        ctx->active_root = NULL;
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
    const gs_vec2 fb = ctx->framebuffer_size;

    gsi_defaults(&ctx->gsi);
    gsi_camera2D(&ctx->gsi, (uint32_t)fb.x, (uint32_t)fb.y);
    gsi_blend_enabled(&ctx->gsi, true);

    gs_gui_rect_t clip = gs_gui_unclipped_rect;

    gs_gui_command_t* cmd = NULL; 
    while (gs_gui_next_command(ctx, &cmd)) 
    {
      switch (cmd->type) 
      {
        case GS_GUI_COMMAND_CUSTOM:
        { 
            gsi_defaults(&ctx->gsi);
            gsi_set_view_scissor(&ctx->gsi, 
                (int32_t)(cmd->custom.clip.x), 
                (int32_t)(fb.y - cmd->custom.clip.h - cmd->custom.clip.y), 
                (int32_t)(cmd->custom.clip.w), 
                (int32_t)(cmd->custom.clip.h));

            if (cmd->custom.cb) {
                cmd->custom.cb(ctx, &cmd->custom);
            }

            gsi_defaults(&ctx->gsi);
            gsi_camera2D(&ctx->gsi, (uint32_t)fb.x, (uint32_t)fb.y);
            gsi_blend_enabled(&ctx->gsi, true);
            gs_graphics_set_viewport(&ctx->gsi.commands, 0, 0, (uint32_t)fb.x, (uint32_t)fb.y);

            gsi_set_view_scissor(&ctx->gsi, 
                (int32_t)(clip.x), 
                (int32_t)(fb.y - clip.h - clip.y), 
                (int32_t)(clip.w), 
                (int32_t)(clip.h));

        } break;

        case GS_GUI_COMMAND_TEXT:
        {
            const gs_vec2* tp = &cmd->text.pos;
            const char* ts = cmd->text.str;
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

                case GS_GUI_SHAPE_LINE:
                {
                    gs_vec2* s = &cmd->shape.line.start;
                    gs_vec2* e = &cmd->shape.line.end;
                    gsi_linev(&ctx->gsi, *s, *e, *c);
                } break;
            }
            
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

            clip = clip_rect;

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

GS_API_DECL void gs_gui_renderpass_submit(gs_gui_context_t* ctx, gs_command_buffer_t* cb, gs_color_t c)
{
    gs_vec2 fbs = ctx->framebuffer_size;
	gs_graphics_clear_action_t action = gs_default_val();
	action.color[0] = (float)c.r / 255.f; 
	action.color[1] = (float)c.g / 255.f; 
	action.color[2] = (float)c.b / 255.f; 
	action.color[3] = (float)c.a / 255.f;
	gs_graphics_clear_desc_t clear = gs_default_val();
	clear.actions = &action;
    gs_graphics_renderpass_begin(cb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
    {
        gs_graphics_clear(cb, &clear);
        gs_graphics_set_viewport(cb, 0, 0, (int)fbs.x,(int)fbs.y);
        gs_gui_render(ctx, cb);
    }
    gs_graphics_renderpass_end(cb);
}

GS_API_DECL void gs_gui_renderpass_submit_ex(gs_gui_context_t* ctx, gs_command_buffer_t* cb, gs_graphics_clear_action_t* action)
{
    gs_vec2 fbs = ctx->framebuffer_size;
    gs_graphics_clear_desc_t clear = gs_default_val();
    clear.actions = action;
	gs_renderpass pass = gs_default_val();
	gs_graphics_renderpass_begin(cb, pass);
	gs_graphics_set_viewport(cb, 0, 0, (int32_t)fbs.x, (int32_t)fbs.y);
	gs_graphics_clear(cb, &clear);
	gs_gui_render(ctx, cb);
	gs_graphics_renderpass_end(cb);
}

GS_API_DECL void gs_gui_set_hover(gs_gui_context_t *ctx, gs_gui_id id)
{
    ctx->prev_hover = ctx->hover;
    ctx->hover = id;
}

GS_API_DECL void gs_gui_set_focus(gs_gui_context_t* ctx, gs_gui_id id) 
{
    ctx->prev_focus = ctx->focus;
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

GS_API_DECL gs_gui_id gs_gui_get_id_hash(gs_gui_context_t* ctx, const void* data, int32_t size, gs_gui_id hash)
{
	gs_gui_id res = hash;
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
	ctx->mouse_pos = gs_v2((f32)x, (f32)y);
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

	while ((uint8_t*) *cmd != (uint8_t*)(ctx->command_list.items + ctx->command_list.idx)) 
    {
		if ((*cmd)->type != GS_GUI_COMMAND_JUMP) 
        { 
            return 1; 
        }
		*cmd = (gs_gui_command_t*)((*cmd)->jump.dst);
	}
	return 0;
} 

GS_API_DECL void gs_gui_set_clip(gs_gui_context_t* ctx, gs_gui_rect_t rect) 
{
	gs_gui_command_t* cmd;
	cmd = gs_gui_push_command(ctx, GS_GUI_COMMAND_CLIP, sizeof(gs_gui_clipcommand_t));
	cmd->clip.rect = rect;
} 

GS_API_DECL void gs_gui_draw_line(gs_gui_context_t* ctx, gs_vec2 start, gs_vec2 end, gs_color_t color)
{
	gs_gui_command_t* cmd;
	gs_gui_rect_t rect = gs_default_val();
	gs_vec2 s = start.x < end.x ? start : end;
	gs_vec2 e = start.x < end.x ? end : start;
	gs_gui_rect(s.x, s.y, e.x - s.x, e.y - s.y);
	rect = gs_gui_intersect_rects(rect, gs_gui_get_clip_rect(ctx));

	// do clip command if the rect isn't fully contained within the cliprect
	int32_t clipped = gs_gui_check_clip(ctx, rect);
	if (clipped == GS_GUI_CLIP_ALL ) {return;}
	if (clipped == GS_GUI_CLIP_PART) {gs_gui_set_clip(ctx, gs_gui_get_clip_rect(ctx));}

	cmd = gs_gui_push_command(ctx, GS_GUI_COMMAND_SHAPE, sizeof(gs_gui_shapecommand_t));
	cmd->shape.type = GS_GUI_SHAPE_LINE;
	cmd->shape.line.start = s;
	cmd->shape.line.end = e;
	cmd->shape.color = color;

	// reset clipping if it was set
	if (clipped) {gs_gui_set_clip(ctx, gs_gui_unclipped_rect);}
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

GS_API_DECL void gs_gui_draw_box(gs_gui_context_t* ctx, gs_gui_rect_t rect, int16_t* w, gs_color_t color)
{
    gs_immediate_draw_t* dl = &ctx->overlay_draw_list;
    // gsi_rectvd(dl, gs_v2(rect.x, rect.y), gs_v2(rect.w, rect.h), gs_v2s(0.f), gs_v2s(1.f), GS_COLOR_RED, GS_GRAPHICS_PRIMITIVE_LINES);

    const float l = (float)w[0], r = (float)w[1], t = (float)w[2], b = (float)w[3];
	gs_gui_draw_rect(ctx, gs_gui_rect(rect.x + l, rect.y, rect.w - r - l, t), color);               // top
	gs_gui_draw_rect(ctx, gs_gui_rect(rect.x + l, rect.y + rect.h - b, rect.w - r - l, b), color);  // bottom 
	gs_gui_draw_rect(ctx, gs_gui_rect(rect.x, rect.y, l, rect.h), color);                           // left
	gs_gui_draw_rect(ctx, gs_gui_rect(rect.x + rect.w - r, rect.y, r, rect.h), color);              // right
}

GS_API_DECL void gs_gui_draw_text(gs_gui_context_t* ctx, gs_asset_font_t* font, const char *str, int32_t len, gs_vec2 pos, gs_color_t color, int32_t shadow_x, int32_t shadow_y, gs_color_t shadow_color)
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
        gs_vec2 td = gs_gui_text_dimensions(font, TEXT, -1);\
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
    if (shadow_x || shadow_y && shadow_color.a)
    {
        DRAW_TEXT(str, gs_gui_rect(pos.x + (float)shadow_x, pos.y + (float)shadow_y, td.x, td.y), shadow_color);
    }
    
    // Draw text
    {
        DRAW_TEXT(str, gs_gui_rect(pos.x, pos.y, td.x, td.y), color);
    } 
} 

GS_API_DECL void gs_gui_draw_image(gs_gui_context_t *ctx, gs_handle(gs_graphics_texture_t) hndl, gs_gui_rect_t rect, gs_vec2 uv0, gs_vec2 uv1, gs_color_t color)
{
	gs_gui_command_t* cmd;

	/* do clip command if the rect isn't fully contained within the cliprect */
	int32_t clipped = gs_gui_check_clip(ctx, rect);
	if (clipped == GS_GUI_CLIP_ALL ) {return;}
	if (clipped == GS_GUI_CLIP_PART) {gs_gui_set_clip(ctx, gs_gui_get_clip_rect(ctx));}

	/* do image command */
	cmd = gs_gui_push_command(ctx, GS_GUI_COMMAND_IMAGE, sizeof(gs_gui_imagecommand_t));
	cmd->image.hndl = hndl;
	cmd->image.rect = rect;
    cmd->image.uvs = gs_v4(uv0.x, uv0.y, uv1.x, uv1.y);
	cmd->image.color = color;

	/* reset clipping if it was set */
	if (clipped) {gs_gui_set_clip(ctx, gs_gui_unclipped_rect);}
} 

GS_API_DECL void gs_gui_draw_custom(gs_gui_context_t* ctx, gs_gui_rect_t rect, gs_gui_draw_callback_t cb, void* data, size_t sz)
{
	gs_gui_command_t* cmd;

    gs_gui_rect_t viewport = rect;

	rect = gs_gui_intersect_rects(rect, gs_gui_get_clip_rect(ctx));

	/* do clip command if the rect isn't fully contained within the cliprect */
	int32_t clipped = gs_gui_check_clip(ctx, rect);
	if (clipped == GS_GUI_CLIP_ALL ) {return;}
	if (clipped == GS_GUI_CLIP_PART) {gs_gui_set_clip(ctx, gs_gui_get_clip_rect(ctx));}

	int32_t idx = ctx->id_stack.idx;
	gs_gui_id res = (idx > 0) ? ctx->id_stack.items[idx - 1] : GS_GUI_HASH_INITIAL;

	/* do custom command */
	cmd = gs_gui_push_command(ctx, GS_GUI_COMMAND_CUSTOM, sizeof(gs_gui_customcommand_t));
	cmd->custom.clip = rect;
    cmd->custom.viewport = viewport;
	cmd->custom.cb = cb;
    cmd->custom.hover = ctx->hover;
    cmd->custom.focus = ctx->focus; 
    cmd->custom.hash = res;
    cmd->custom.data = ctx->command_list.items + ctx->command_list.idx;
    cmd->custom.sz = sz;
    cmd->base.size += sz;
    
    // Copy data and move list forward
    memcpy(ctx->command_list.items + ctx->command_list.idx, data, sz);
    ctx->command_list.idx += sz;

	/* reset clipping if it was set */
	if (clipped) {gs_gui_set_clip(ctx, gs_gui_unclipped_rect);}
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
        gs_gui_rect_t r = gs_gui_rect(rect.x, rect.y, (f32)w, (f32)h);
        gs_vec2 st0 = gs_v2(uv0.x, uv0.y);
        gs_vec2 st1 = gs_v2(uv0.x + ((float)left / (float)width), uv0.y + ((float)top / (float)height));
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // tr
    {
        uint32_t w = right;
        uint32_t h = top;
        gs_gui_rect_t r = gs_gui_rect(rect.x + rect.w - w, rect.y, (f32)w, (f32)h);
        gs_vec2 st0 = gs_v2(uv1.x - ((float)right / (float)width), uv0.y);
        gs_vec2 st1 = gs_v2(uv1.x, uv0.y + ((float)top / (float)height));
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // br
    {
        uint32_t w = right;
        uint32_t h = bottom;
        gs_gui_rect_t r = gs_gui_rect(rect.x + rect.w - (f32)w, rect.y + rect.h - (f32)h, (f32)w, (f32)h);
        gs_vec2 st0 = gs_v2(uv1.x - ((float)right / (float)width), uv1.y - ((float)bottom / (float)height));
        gs_vec2 st1 = gs_v2(uv1.x, uv1.y);
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // bl
    {
        uint32_t w = left;
        uint32_t h = bottom;
        gs_gui_rect_t r = gs_gui_rect(rect.x, rect.y + rect.h - (f32)h, (f32)w, (f32)h);
        gs_vec2 st0 = gs_v2(uv0.x, uv1.y - ((float)bottom / (float)height));
        gs_vec2 st1 = gs_v2(uv0.x + ((float)left / (float)width), uv1.y);
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // top
    {
        uint32_t w = (u32)rect.w - left - right;
        uint32_t h = top;
        gs_gui_rect_t r = gs_gui_rect(rect.x + left, rect.y, (f32)w, (f32)h);
        gs_vec2 st0 = gs_v2(uv0.x + ((float)left / (float)width), uv0.y);
        gs_vec2 st1 = gs_v2(uv1.x - ((float)right / (float)width), uv0.y + ((float)top / (float)height));
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // bottom
    {
        uint32_t w = (u32)rect.w - left - right;
        uint32_t h = bottom;
        gs_gui_rect_t r = gs_gui_rect(rect.x + left, rect.y + rect.h - (f32)h, (f32)w, (f32)h);
        gs_vec2 st0 = gs_v2(uv0.x + ((float)left / (float)width), uv1.y - ((float)bottom / (float)height));
        gs_vec2 st1 = gs_v2(uv1.x - ((float)right / (float)width), uv1.y);
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // left
    {
        uint32_t w = left;
        uint32_t h = (u32)rect.h - top - bottom;
        gs_gui_rect_t r = gs_gui_rect(rect.x, rect.y + top, (f32)w, (f32)h);
        gs_vec2 st0 = gs_v2(uv0.x, uv0.y + ((float)top / (float)height));
        gs_vec2 st1 = gs_v2(uv0.x + ((float)left / (float)width), uv1.y - ((float)bottom / (float)height));
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // right
    {
        uint32_t w = right;
        uint32_t h = (u32)rect.h - top - bottom;
        gs_gui_rect_t r = gs_gui_rect(rect.x + rect.w - (f32)w, rect.y + top, (f32)w, (f32)h);
        gs_vec2 st0 = gs_v2(uv1.x - ((float)right / (float)width), uv0.y + ((float)top / (float)height));
        gs_vec2 st1 = gs_v2(uv1.x, uv1.y - ((float)bottom / (float)height));
        gs_gui_draw_image(ctx, hndl, r, st0, st1, color);
    }

    // center
    {
        uint32_t w = (u32)rect.w - right - left;
        uint32_t h = (u32)rect.h - top - bottom;
        gs_gui_rect_t r = gs_gui_rect(rect.x + left, rect.y + top, (f32)w, (f32)h);
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

GS_API_DECL gs_gui_rect_t gs_gui_layout_anchor(const gs_gui_rect_t* p, int32_t width, int32_t height, int32_t xoff, int32_t yoff, gs_gui_layout_anchor_type type)
{ 
    float w = (float)width;
    float h = (float)height;
    gs_gui_rect_t r = gs_gui_rect(p->x, p->y, w, h);

    switch (type)
    {
        default:
        case GS_GUI_LAYOUT_ANCHOR_TOPLEFT:
        {
        } break;

        case GS_GUI_LAYOUT_ANCHOR_TOPCENTER:
        {
            r.x = p->x + (p->w - w) * 0.5f;
        } break;

        case GS_GUI_LAYOUT_ANCHOR_TOPRIGHT:
        {
            r.x = p->x + (p->w - w);
        } break;

        case GS_GUI_LAYOUT_ANCHOR_LEFT:
        {
            r.y = p->y + (p->h - h) * 0.5f;
        } break;

        case GS_GUI_LAYOUT_ANCHOR_CENTER:
        {
            r.x = p->x + (p->w - w) * 0.5f;
            r.y = p->y + (p->h - h) * 0.5f;
        } break;

        case GS_GUI_LAYOUT_ANCHOR_RIGHT:
        {
            r.x = p->x + (p->w - w);
            r.y = p->y + (p->h - h) * 0.5f;
        } break;

        case GS_GUI_LAYOUT_ANCHOR_BOTTOMLEFT:
        {
            r.y = p->y + (p->h - h);
        } break;

        case GS_GUI_LAYOUT_ANCHOR_BOTTOMCENTER:
        {
            r.x = p->x + (p->w - w) * 0.5f;
            r.y = p->y + (p->h - h);
        } break;

        case GS_GUI_LAYOUT_ANCHOR_BOTTOMRIGHT:
        {
            r.x = p->x + (p->w - w);
            r.y = p->y + (p->h - h);
        } break;
    }

    // Apply offset
    r.x += xoff;
    r.y += yoff;

    return r;
}

GS_API_DECL void gs_gui_layout_column_begin(gs_gui_context_t* ctx) 
{
	gs_gui_push_layout(ctx, gs_gui_layout_next(ctx), gs_v2(0, 0));
} 

GS_API_DECL void gs_gui_layout_column_end(gs_gui_context_t* ctx) 
{
	gs_gui_layout_t *a, *b;
	b = gs_gui_get_layout(ctx);
	gs_gui_stack_pop(ctx->layout_stack);

	/* inherit position/next_row/max from child layout if they are greater */
	a = gs_gui_get_layout(ctx);
	a->position.x = gs_max(a->position.x, b->position.x + b->body.x - a->body.x);
	a->next_row = (int32_t)gs_max((f32)a->next_row, (f32)b->next_row + (f32)b->body.y - (f32)a->body.y);
	a->max.x = gs_max(a->max.x, b->max.x);
	a->max.y = gs_max(a->max.y, b->max.y);
} 

GS_API_DECL void gs_gui_layout_row(gs_gui_context_t *ctx, int32_t items, const int32_t *widths, int32_t height)
{
    gs_gui_style_t* style = ctx->style;
	gs_gui_layout_t* layout = gs_gui_get_layout(ctx); 

	if (widths) 
    {
		gs_gui_expect(items <= GS_GUI_MAX_WIDTHS);
		memcpy(layout->widths, widths, items * sizeof(widths[0]));
	}
	layout->items = items;
	layout->position = gs_v2((f32)layout->indent, (f32)layout->next_row);
	layout->size.y = (f32)height;
	layout->item_index = 0;
} 

GS_API_DECL void gs_gui_layout_row_ex(gs_gui_context_t *ctx, int32_t items, const int32_t *widths, int32_t height, int32_t justification)
{
    gs_gui_layout_row(ctx, items, widths, height);
	gs_gui_layout_t* layout = gs_gui_get_layout(ctx); 

    switch (justification)
    {
        default: break;

        case GS_GUI_JUSTIFY_CENTER:
        {
            // Iterate through all widths, calculate total
            // X is center - tw/2
            float w = 0.f;
            for (uint32_t i = 0; i < items; ++i)
            {
                w += widths[i] > 0.f ? widths[i] : widths[i] == 0.f? layout->size.x : layout->body.w - widths[i];
            }
            layout->position.x = (layout->body.w - w) * 0.5f + layout->indent;
        } break; 

        case GS_GUI_JUSTIFY_END:
        {
            float w = 0.f; 
            for (uint32_t i = 0; i < items; ++i) {
                w += widths[i] > 0.f ? widths[i] : widths[i] == 0.f? layout->size.x : layout->body.w - widths[i];
            }
            layout->position.x = (layout->body.w - w);
        } break;
    }
}

GS_API_DECL void gs_gui_layout_width(gs_gui_context_t *ctx, int32_t width) 
{
	gs_gui_get_layout(ctx)->size.x = (f32)width;
} 

GS_API_DECL void gs_gui_layout_height(gs_gui_context_t *ctx, int32_t height) 
{
	gs_gui_get_layout(ctx)->size.y = (f32)height;
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
		// handle next row
		if (layout->item_index == layout->items) 
        {
			gs_gui_layout_row(ctx, layout->items, NULL, (s32)layout->size.y);
		} 

        const int32_t items = layout->items;
        const int32_t idx = layout->item_index; 

        int32_t ml = style->margin[GS_GUI_MARGIN_LEFT];
        int32_t mr = style->margin[GS_GUI_MARGIN_RIGHT];
        int32_t mt = style->margin[GS_GUI_MARGIN_TOP];
        int32_t mb = style->margin[GS_GUI_MARGIN_BOTTOM]; 

		// position
		res.x = layout->position.x + ml;
		res.y = layout->position.y + mt;

		// size
		res.w = layout->items > 0 ? layout->widths[layout->item_index] : layout->size.x;
		res.h = layout->size.y;

        // default fallbacks
        if (res.w == 0) { res.w = style->size[0]; }
        if (res.h == 0) { res.h = style->size[1]; }

        // Not sure about this... should probably iterate through the rest, figure out what's left, then 
        // determine how much to divide up
        if (res.w < 0) { res.w += layout->body.w - res.x + 1; }
        if (res.h < 0) { res.h += layout->body.h - res.y + 1; }

		layout->item_index++;
	} 

	/* update position */
	layout->position.x += res.w + style->margin[GS_GUI_MARGIN_RIGHT];
	layout->next_row = (s32)gs_max(layout->next_row, res.y + res.h + style->margin[GS_GUI_MARGIN_BOTTOM]);//  + style->margin[GS_GUI_MARGIN_TOP] * 0.5f);

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
	int32_t elementid, int32_t opt)
{
	if (opt & GS_GUI_OPT_NOFRAME) {return;}
    int32_t state = ctx->focus == id ? GS_GUI_ELEMENT_STATE_FOCUS : 
                    ctx->hover == id ? GS_GUI_ELEMENT_STATE_HOVER : 
                    0x00;
	gs_gui_draw_frame(ctx, rect, &ctx->style_sheet->styles[elementid][state]);
} 

GS_API_DECL void gs_gui_draw_control_text(gs_gui_context_t *ctx, const char *str, gs_gui_rect_t rect,
	const gs_gui_style_t* style, int32_t opt)
{ 
	gs_vec2 pos = gs_v2(rect.x, rect.y);
	gs_asset_font_t* font = style->font; 
    gs_vec2 td = gs_gui_text_dimensions(font, str, -1);
	int32_t tw = (int32_t)td.x;
    int32_t th = (int32_t)td.y;

	gs_gui_push_clip_rect(ctx, rect);

    // Grab stylings
    const int32_t padding_left = style->padding[GS_GUI_PADDING_LEFT];
    const int32_t padding_top = style->padding[GS_GUI_PADDING_TOP];
    const int32_t padding_right = style->padding[GS_GUI_PADDING_RIGHT];
    const int32_t padding_bottom = style->padding[GS_GUI_PADDING_BOTTOM];
    const int32_t align = style->align_content;
    const int32_t justify = style->justify_content; 

    // Determine x placement based on justification
    switch (justify)
    { 
        default:
        case GS_GUI_JUSTIFY_START:
        { 
            pos.x = rect.x + padding_left;
        } break;

        case GS_GUI_JUSTIFY_CENTER:
        {
            pos.x = rect.x + (rect.w - tw) * 0.5f;
        } break;

        case GS_GUI_JUSTIFY_END:
        {
            pos.x = rect.x + (rect.w - tw) - padding_right;
        } break;
    }

    // Determine y placement based on alignment within rect
    switch (align)
    {
        default:
        case GS_GUI_ALIGN_START:
        {
            pos.y = rect.y + padding_top;
        } break;

        case GS_GUI_ALIGN_CENTER:
        {
            pos.y = rect.y + (rect.h - th) * 0.5f;
        } break;

        case GS_GUI_ALIGN_END:
        {
            pos.y = rect.y + (rect.h - th) - padding_bottom;
        } break;
    } 

    bool is_content = (opt & GS_GUI_OPT_ISCONTENT);
    int32_t bg_color = is_content ? GS_GUI_COLOR_CONTENT_BACKGROUND : 
                                    GS_GUI_COLOR_BACKGROUND;
    int32_t sh_color = is_content ? GS_GUI_COLOR_CONTENT_SHADOW : 
                                    GS_GUI_COLOR_SHADOW;
    int32_t bd_color = is_content ? GS_GUI_COLOR_CONTENT_BORDER : 
                                    GS_GUI_COLOR_BORDER;

    int32_t sx = style->shadow_x;
    int32_t sy = style->shadow_y;
    const gs_color_t* sc = &style->colors[sh_color]; 

    // draw border
    const gs_color_t* bc = &style->colors[bd_color]; 
    if (bc->a) 
    {
        gs_gui_pop_clip_rect(ctx);
        gs_gui_rect_t border_rect = gs_gui_expand_rect(rect, (int16_t*)style->border_width);
	    gs_gui_push_clip_rect(ctx, border_rect);
        gs_gui_draw_box(ctx, border_rect, (int16_t*)style->border_width, *bc);
    }

    // Draw background rect
    gs_gui_draw_rect(ctx, rect, style->colors[bg_color]); 
	gs_gui_draw_text(ctx, font, str, -1, pos, style->colors[GS_GUI_COLOR_CONTENT], sx, sy, *sc);
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

    gs_gui_id prev_hov = ctx->prev_hover;
    gs_gui_id prev_foc = ctx->prev_focus;

    // I should do updates in here

    if (opt & GS_GUI_OPT_FORCEFOCUS)
    {
        mouseover = gs_gui_rect_overlaps_vec2(gs_gui_get_clip_rect(ctx), ctx->mouse_pos);
    }
    else
    {
	    mouseover = gs_gui_mouse_over(ctx, rect);
    } 

    // Check for 'mouse-over' with id selection here

	if (ctx->focus == id) { ctx->updated_focus = 1; }
	if (opt & GS_GUI_OPT_NOINTERACT) { return; }

    // Check for hold focus here
	if (mouseover && !ctx->mouse_down) { 
        gs_gui_set_hover(ctx, id);
    }

	if (ctx->focus == id) 
    { 
        gs_gui_set_focus(ctx, id);
		if (ctx->mouse_pressed && !mouseover) { gs_gui_set_focus(ctx, 0); }
		if (!ctx->mouse_down && ~opt & GS_GUI_OPT_HOLDFOCUS) { gs_gui_set_focus(ctx, 0); }
	} 

    if (ctx->prev_hover == id && !mouseover) {ctx->prev_hover = ctx->hover;}

	if (ctx->hover == id) 
    { 
		if (ctx->mouse_pressed) 
        {
            if ((opt & GS_GUI_OPT_LEFTCLICKONLY && ctx->mouse_pressed == GS_GUI_MOUSE_LEFT) || (~opt & GS_GUI_OPT_LEFTCLICKONLY))
            {
			    gs_gui_set_focus(ctx, id);
            }
		} 
        else if (!mouseover) 
        {
            gs_gui_set_hover(ctx, 0);
		}
	} 

    // Do state check
    if (~opt & GS_GUI_OPT_NOSWITCHSTATE)
    {
        if (ctx->focus == id)
        {
            if (ctx->prev_focus != id)  ctx->last_focus_state = GS_GUI_ELEMENT_STATE_ON_FOCUS; 
            else                        ctx->last_focus_state = GS_GUI_ELEMENT_STATE_FOCUS; 
        }
        else
        {
            if (ctx->prev_focus == id)  ctx->last_focus_state = GS_GUI_ELEMENT_STATE_OFF_FOCUS; 
            else                        ctx->last_focus_state = GS_GUI_ELEMENT_STATE_DEFAULT; 
        }

        if (ctx->hover == id)
        {
            if (ctx->prev_hover != id)  ctx->last_hover_state = GS_GUI_ELEMENT_STATE_ON_HOVER;
            else                        ctx->last_hover_state = GS_GUI_ELEMENT_STATE_HOVER;
        }
        else
        {
            if (ctx->prev_hover == id) ctx->last_hover_state = GS_GUI_ELEMENT_STATE_OFF_HOVER;
            else                        ctx->last_hover_state = GS_GUI_ELEMENT_STATE_DEFAULT;
        }

        if (ctx->prev_focus == id && !ctx->mouse_down && ~opt & GS_GUI_OPT_HOLDFOCUS) {
            ctx->prev_focus = ctx->focus;
        }

        if (
            ctx->last_hover_state == GS_GUI_ELEMENT_STATE_ON_HOVER  || 
            ctx->last_hover_state == GS_GUI_ELEMENT_STATE_OFF_HOVER || 
            ctx->last_focus_state == GS_GUI_ELEMENT_STATE_ON_FOCUS  || 
            ctx->last_focus_state == GS_GUI_ELEMENT_STATE_OFF_FOCUS  
        )
        {
            // Don't have a hover state switch if focused
            ctx->switch_state = ctx->last_focus_state ? ctx->last_focus_state : ctx->focus != id ? ctx->last_hover_state : GS_GUI_ELEMENT_STATE_DEFAULT; 
            switch (ctx->switch_state)
            {
                case GS_GUI_ELEMENT_STATE_OFF_HOVER:
                case GS_GUI_ELEMENT_STATE_ON_HOVER:
                {
                    if (ctx->focus == id || ctx->prev_focus == id) 
                    {
                        ctx->switch_state = 0x00;
                    }
                } break;
            }
            if (ctx->switch_state && ctx->prev_focus != id) ctx->state_switch_id = id;
        } 
    }
    else
    {
        ctx->prev_focus = prev_foc;
        ctx->prev_hover = prev_hov;
    }

} 

GS_API_DECL int32_t gs_gui_text_ex(gs_gui_context_t* ctx, const char* text, int32_t wrap, const gs_gui_selector_desc_t* desc, int32_t opt)
{ 
	int32_t res = 0, elementid = GS_GUI_ELEMENT_TEXT;
	gs_gui_id id = gs_gui_get_id(ctx, text, strlen(text)); 
    gs_immediate_draw_t* dl = &ctx->overlay_draw_list; 
    gs_gui_style_t style = gs_default_val();
    gs_gui_animation_t* anim = gs_gui_get_animation(ctx, id, desc, elementid); 

	const char *start, *end, *p = text;
	int32_t width = -1; 

    // Update anim (keep states locally within animation, only way to do this) 
    if (anim)
    {
        gs_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = gs_gui_animation_get_blend_style(ctx, anim, desc, elementid); 
    }
    else
    { 
        style = ctx->focus == id ? gs_gui_get_current_element_style(ctx, desc, elementid, 0x02) : 
                ctx->hover == id ? gs_gui_get_current_element_style(ctx, desc, elementid, 0x01) : 
                                   gs_gui_get_current_element_style(ctx, desc, elementid, 0x00);
    } 

    gs_gui_style_t* save = gs_gui_push_style(ctx, &style);
	gs_asset_font_t* font = ctx->style->font;
	gs_color_t* color = &ctx->style->colors[GS_GUI_COLOR_CONTENT];
    int32_t sx = ctx->style->shadow_x;
    int32_t sy = ctx->style->shadow_y;
    gs_color_t* sc = &ctx->style->colors[GS_GUI_COLOR_SHADOW];
    int32_t th = gs_gui_font_height(font);
	gs_gui_layout_column_begin(ctx);
	gs_gui_layout_row(ctx, 1, &width, th); 

    gs_gui_rect_t tr = gs_gui_layout_next(ctx);
    gs_gui_layout_set_next(ctx, tr, 0);
    gs_gui_rect_t r = gs_gui_layout_next(ctx); 
    gs_gui_rect_t bg = r;
	do 
    {
		int32_t w = 0;
		start = end = p;
		do 
        {
			const char* word = p;
			while (*p && *p != ' ' && *p != '\n') 
            { 
                p++; 
            }

            if (wrap) w += gs_gui_text_width(font, word, p - word);
			if (w > r.w && end != start) 
            { 
                break; 
            }

			if (wrap) w += gs_gui_text_width(font, p, 1);
			end = p++;

		} while (*end && *end != '\n'); 

        if (r.w > tr.w) tr.w = r.w;
        tr.h = (r.y + r.h) - tr.y;

        gs_gui_rect_t txtrct = r;
        bg = r;
        if (*end)
        { 
            r = gs_gui_layout_next(ctx);
            bg.h = r.y - bg.y;
        }
        else
        {
            int32_t th = gs_gui_text_height(font, start, end - start); 
            bg.h = r.h + (float)th / 2.f;
        }
        
        // Draw frame here for background if applicable (need to do this to account for space between wrap)
        if (ctx->style->colors[GS_GUI_COLOR_BACKGROUND].a)
        {
            gs_gui_draw_rect(ctx, bg, style.colors[GS_GUI_COLOR_BACKGROUND]);
        } 

        // Draw text
		gs_gui_draw_text(ctx, font, start, end - start, gs_v2(txtrct.x, txtrct.y), *color, sx, sy, *sc);
		p = end + 1;

	} while (*end);

    // draw border
    if (style.colors[GS_GUI_COLOR_BORDER].a) 
    {
        gs_gui_draw_box(ctx, gs_gui_expand_rect(tr, (int16_t*)style.border_width), (int16_t*)style.border_width, style.colors[GS_GUI_COLOR_BORDER]);
    }

	gs_gui_update_control(ctx, id, tr, 0x00); 

	// handle click
    if (ctx->mouse_down != GS_GUI_MOUSE_LEFT && ctx->hover == id && ctx->last_focus_state == GS_GUI_ELEMENT_STATE_OFF_FOCUS)
    { 
		res |= GS_GUI_RES_SUBMIT;
	}

	gs_gui_layout_column_end(ctx); 
    gs_gui_pop_style(ctx, save);

    return res;
} 

GS_API_DECL int32_t gs_gui_label_ex(gs_gui_context_t* ctx, const char* label, const gs_gui_selector_desc_t* desc, int32_t opt)
{
    // Want to push animations here for styles
	int32_t res = 0;
    int32_t elementid = GS_GUI_ELEMENT_LABEL;
    gs_gui_id id = gs_gui_get_id(ctx, label, gs_strlen(label)); 

    char id_tag[256] = gs_default_val(); 
    char label_tag[256] = gs_default_val(); 
    gs_gui_parse_id_tag(ctx, label, id_tag, sizeof(id_tag));
    gs_gui_parse_label_tag(ctx, label, label_tag, sizeof(label_tag));

	if (id_tag) gs_gui_push_id(ctx, id_tag, strlen(id_tag));

    gs_gui_style_t style = gs_default_val();
    gs_gui_animation_t* anim = gs_gui_get_animation(ctx, id, desc, elementid); 

    if (anim)
    {
        gs_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = gs_gui_animation_get_blend_style(ctx, anim, desc, elementid); 
    }
    else
    { 
        style = ctx->focus == id ? gs_gui_get_current_element_style(ctx, desc, elementid, 0x02) : 
                ctx->hover == id ? gs_gui_get_current_element_style(ctx, desc, elementid, 0x01) : 
                                   gs_gui_get_current_element_style(ctx, desc, elementid, 0x00);
    } 

    gs_gui_style_t* save = gs_gui_push_style(ctx, &style); 
	gs_gui_rect_t r = gs_gui_layout_next(ctx);
	gs_gui_update_control(ctx, id, r, 0x00); 
	gs_gui_draw_control_text(ctx, label_tag, r, &style, 0x00); 
    gs_gui_pop_style(ctx, save);
	if (id_tag) gs_gui_pop_id(ctx);

	/* handle click */
    if (
            ctx->mouse_down != GS_GUI_MOUSE_LEFT && 
            ctx->hover == id && 
            ctx->last_focus_state == GS_GUI_ELEMENT_STATE_OFF_FOCUS
    )
    { 
		res |= GS_GUI_RES_SUBMIT;
	}

    return res;
} 

GS_API_DECL int32_t gs_gui_image_ex(gs_gui_context_t* ctx, gs_handle(gs_graphics_texture_t) hndl, gs_vec2 uv0, gs_vec2 uv1, const gs_gui_selector_desc_t* desc, int32_t opt)
{
	int32_t res = 0;
	gs_gui_id id = gs_gui_get_id(ctx, &hndl, sizeof(hndl));
    const int32_t elementid = GS_GUI_ELEMENT_IMAGE;

    gs_gui_style_t style = gs_default_val();
    gs_gui_animation_t* anim = gs_gui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this) 
    if (anim)
    {
        gs_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = gs_gui_animation_get_blend_style(ctx, anim, desc, elementid); 
    }
    else
    { 
        style = ctx->focus == id ? gs_gui_get_current_element_style(ctx, desc, elementid, 0x02) : 
                ctx->hover == id ? gs_gui_get_current_element_style(ctx, desc, elementid, 0x01) : 
                                   gs_gui_get_current_element_style(ctx, desc, elementid, 0x00);
    } 

    // Temporary copy of style
    gs_gui_style_t* save = gs_gui_push_style(ctx, &style); 
	gs_gui_rect_t r = gs_gui_layout_next(ctx);
	gs_gui_update_control(ctx, id, r, opt);

	/* handle click */
    if (ctx->mouse_down != GS_GUI_MOUSE_LEFT && ctx->hover == id && ctx->last_focus_state == GS_GUI_ELEMENT_STATE_OFF_FOCUS)
    { 
		res |= GS_GUI_RES_SUBMIT;
	}

    // draw border
    if (style.colors[GS_GUI_COLOR_BORDER].a) 
    {
        gs_gui_draw_box(ctx, gs_gui_expand_rect(r, (int16_t*)style.border_width), (int16_t*)style.border_width, style.colors[GS_GUI_COLOR_BORDER]);
    }

    gs_gui_draw_image(ctx, hndl, r, uv0, uv1, style.colors[GS_GUI_COLOR_CONTENT]);

    gs_gui_pop_style(ctx, save);

	return res;
}

GS_API_DECL int32_t gs_gui_combo_begin_ex(gs_gui_context_t* ctx, const char* id, const char* current_item, int32_t max_items, gs_gui_selector_desc_t* desc, int32_t opt)
{
    int32_t res = 0;
    opt = GS_GUI_OPT_NOMOVE | 
                  GS_GUI_OPT_NORESIZE | 
                  GS_GUI_OPT_NOTITLE | 
                  GS_GUI_OPT_FORCESETRECT; 

    if (gs_gui_button(ctx, current_item)) {
        gs_gui_popup_open(ctx, id);
    }

    int32_t ct = max_items > 0 ? max_items : 0;
    gs_gui_rect_t rect = ctx->last_rect;
    rect.y += rect.h;
    rect.h = ct ? (ct + 1) * ctx->style_sheet->styles[GS_GUI_ELEMENT_BUTTON][0x00].size[1] : rect.h;
    return gs_gui_popup_begin_ex(ctx, id, rect, NULL, opt);
}

GS_API_DECL void gs_gui_combo_end(gs_gui_context_t* ctx) 
{
    gs_gui_popup_end(ctx);
}

GS_API_DECL void gs_gui_parse_label_tag(gs_gui_context_t* ctx, const char* str, char* buffer, size_t sz)
{
    gs_lexer_t lex = gs_lexer_c_ctor(str);
    while (gs_lexer_can_lex(&lex))
    {
        gs_token_t token = gs_lexer_next_token(&lex);
        switch (token.type)
        {
            case GS_TOKEN_HASH:
            {
                if (gs_lexer_peek(&lex).type == GS_TOKEN_HASH)
                {
                    gs_token_t end = gs_lexer_current_token(&lex);

                    // Determine len
                    size_t len = gs_min(end.text - str, sz);

                    memcpy(buffer, str, len);
                    return;
                }
            } break;
        }
    } 

    // Reached end, so just memcpy
    memcpy(buffer, str, sz);
}

GS_API_DECL void gs_gui_parse_id_tag(gs_gui_context_t* ctx, const char* str, char* buffer, size_t sz)
{
    gs_lexer_t lex = gs_lexer_c_ctor(str);
    while (gs_lexer_can_lex(&lex))
    {
        gs_token_t token = gs_lexer_next_token(&lex);
        switch (token.type)
        {
            case GS_TOKEN_HASH:
            {
                if (gs_lexer_peek(&lex).type == GS_TOKEN_HASH)
                {
                    gs_token_t end = gs_lexer_next_token(&lex);
                    end = gs_lexer_next_token(&lex);

                    // Determine len
                    size_t len = gs_min((str + strlen(str)) - end.text, sz);

                    memcpy(buffer, end.text, len);
                    return;
                }
            } break;
        }
    } 
}

GS_API_DECL int32_t gs_gui_button_ex(gs_gui_context_t* ctx, const char* label, const gs_gui_selector_desc_t* desc, int32_t opt)
{ 
    // Note(john): clip out early here for performance

	int32_t res = 0;
	gs_gui_id id = gs_gui_get_id(ctx, label, strlen(label)); 
    gs_immediate_draw_t* dl = &ctx->overlay_draw_list;

    char id_tag[256] = gs_default_val(); 
    char label_tag[256] = gs_default_val(); 
    gs_gui_parse_id_tag(ctx, label, id_tag, sizeof(id_tag));
    gs_gui_parse_label_tag(ctx, label, label_tag, sizeof(label_tag));

    gs_gui_style_t style = gs_default_val();
    gs_gui_animation_t* anim = gs_gui_get_animation(ctx, id, desc, GS_GUI_ELEMENT_BUTTON);

	// Push id if tag available
	if (id_tag) {
		gs_gui_push_id(ctx, id_tag, strlen(id_tag));
	}

    // Update anim (keep states locally within animation, only way to do this) 
    if (anim)
    {
        gs_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = gs_gui_animation_get_blend_style(ctx, anim, desc, GS_GUI_ELEMENT_BUTTON); 
    }
    else
    { 
        style = ctx->focus == id ? gs_gui_get_current_element_style(ctx, desc, GS_GUI_ELEMENT_BUTTON, 0x02) : 
                ctx->hover == id ? gs_gui_get_current_element_style(ctx, desc, GS_GUI_ELEMENT_BUTTON, 0x01) : 
                                   gs_gui_get_current_element_style(ctx, desc, GS_GUI_ELEMENT_BUTTON, 0x00);
    } 

    // Temporary copy of style
    gs_gui_style_t* save = gs_gui_push_style(ctx, &style); 
	gs_gui_rect_t r = gs_gui_layout_next(ctx);
	gs_gui_update_control(ctx, id, r, opt); 

	/* handle click or button press for submission */
    if (ctx->mouse_down != GS_GUI_MOUSE_LEFT && ctx->hover == id && ctx->last_focus_state == GS_GUI_ELEMENT_STATE_OFF_FOCUS)
    { 
		res |= GS_GUI_RES_SUBMIT;
	}

    // draw border
    if (style.colors[GS_GUI_COLOR_BORDER].a) 
    {
        gs_gui_draw_box(ctx, gs_gui_expand_rect(r, (int16_t*)style.border_width), (int16_t*)style.border_width, style.colors[GS_GUI_COLOR_BORDER]);
    }

    opt |= GS_GUI_OPT_ISCONTENT;
    gs_gui_draw_rect(ctx, r, style.colors[GS_GUI_COLOR_BACKGROUND]);
    if (label) {gs_gui_draw_control_text(ctx, label_tag, r, &style, opt);}

    gs_gui_pop_style(ctx, save);

	if (id_tag) gs_gui_pop_id(ctx);

	return res;
}

GS_API_DECL int32_t gs_gui_checkbox_ex(gs_gui_context_t* ctx, const char* label, int32_t* state, const gs_gui_selector_desc_t* desc, int32_t opt)
{
	int32_t res = 0;
	gs_gui_id id = gs_gui_get_id(ctx, &state, sizeof(state));
	gs_gui_rect_t r = gs_gui_layout_next(ctx);
	gs_gui_rect_t box = gs_gui_rect(r.x, r.y, r.h, r.h);
    int32_t ox = (int32_t)(box.w * 0.2f), oy = (int32_t)(box.h * 0.2f);
    gs_gui_rect_t inner_box = gs_gui_rect(box.x + ox, box.y + oy, box.w - 2 * ox, box.h - 2 * oy);
	gs_gui_update_control(ctx, id, r, 0);

    int32_t elementid = GS_GUI_ELEMENT_BUTTON;
    gs_gui_style_t style = gs_default_val();
    style = ctx->focus == id ? gs_gui_get_current_element_style(ctx, desc, elementid, 0x02) : 
            ctx->hover == id ? gs_gui_get_current_element_style(ctx, desc, elementid, 0x01) : 
                               gs_gui_get_current_element_style(ctx, desc, elementid, 0x00);

	/* handle click */
	if (ctx->mouse_pressed == GS_GUI_MOUSE_LEFT && ctx->focus == id) 
    {
		res |= GS_GUI_RES_CHANGE;
		*state = !*state;
	}

	/* draw */
	gs_gui_draw_control_frame(ctx, id, box, GS_GUI_ELEMENT_INPUT, 0);
	if (*state) 
    {
        // Draw in a filled rect
        gs_gui_draw_rect(ctx, inner_box, style.colors[GS_GUI_COLOR_BACKGROUND]);
	}

	r = gs_gui_rect(r.x + box.w, r.y, r.w - box.w, r.h);
	gs_gui_draw_control_text(ctx, label, r, &ctx->style_sheet->styles[GS_GUI_ELEMENT_TEXT][0], 0);
	return res;
}

GS_API_DECL int32_t gs_gui_textbox_raw(gs_gui_context_t* ctx, char* buf, int32_t bufsz, gs_gui_id id, gs_gui_rect_t rect, 
        const gs_gui_selector_desc_t* desc, int32_t opt)
{
	int32_t res = 0;

    int32_t elementid = GS_GUI_ELEMENT_INPUT; 
    gs_gui_style_t style = gs_default_val();
    gs_gui_animation_t* anim = gs_gui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this) 
    if (anim)
    {
        // Need to check that I haven't updated more than once this frame
        gs_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = gs_gui_animation_get_blend_style(ctx, anim, desc, elementid); 
    }
    else
    { 
        style = ctx->focus == id ? gs_gui_get_current_element_style(ctx, desc, elementid, 0x02) : 
                ctx->hover == id ? gs_gui_get_current_element_style(ctx, desc, elementid, 0x01) : 
                                   gs_gui_get_current_element_style(ctx, desc, elementid, 0x00);
    } 

    // Push temp style
    gs_gui_style_t* save = gs_gui_push_style(ctx, &style); 

	gs_gui_update_control(ctx, id, rect, opt | GS_GUI_OPT_HOLDFOCUS);

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
    gs_gui_draw_box(ctx, gs_gui_expand_rect(rect, (int16_t*)style.border_width), (int16_t*)style.border_width, style.colors[GS_GUI_COLOR_BORDER]);

    // Textbox bg 
	gs_gui_draw_control_frame(ctx, id, rect, GS_GUI_ELEMENT_INPUT, opt);

    // Text and carret
	if (ctx->focus == id) 
    {
        gs_gui_style_t* sp = &style;
		gs_color_t* color = &sp->colors[GS_GUI_COLOR_CONTENT];
        int32_t sx = sp->shadow_x;
        int32_t sy = sp->shadow_y;
		gs_color_t* sc = &sp->colors[GS_GUI_COLOR_SHADOW];
		gs_asset_font_t* font = sp->font; 
		int32_t textw = gs_gui_text_width(font, buf, -1);
        int32_t texth = gs_gui_font_height(font);
		int32_t ofx = (int32_t)(rect.w - sp->padding[GS_GUI_PADDING_RIGHT] - textw - 1);
		int32_t textx = (int32_t)(rect.x + gs_min(ofx, sp->padding[GS_GUI_PADDING_LEFT]));
		int32_t texty = (int32_t)(rect.y + (rect.h - texth) / 2);
		int32_t cary = (int32_t)(rect.y + 1); 
		gs_gui_push_clip_rect(ctx, rect); 

        // Draw text
        gs_gui_draw_control_text(ctx, buf, rect, &style, opt);

        // Draw caret (control alpha based on frame)
        static bool on = true;
        static float ct = 0.f;
        if (~opt & GS_GUI_OPT_NOCARET)
        { 
            gs_vec2 pos = gs_v2(rect.x, rect.y);

            // Grab stylings
            const int32_t padding_left = sp->padding[GS_GUI_PADDING_LEFT];
            const int32_t padding_top = sp->padding[GS_GUI_PADDING_TOP];
            const int32_t padding_right = sp->padding[GS_GUI_PADDING_RIGHT];
            const int32_t padding_bottom = sp->padding[GS_GUI_PADDING_BOTTOM];
            const int32_t align = sp->align_content;
            const int32_t justify = sp->justify_content; 

            // Determine x placement based on justification
            switch (justify)
            { 
                default:
                case GS_GUI_JUSTIFY_START:
                { 
                    pos.x = rect.x + padding_left;
                } break;

                case GS_GUI_JUSTIFY_CENTER:
                {
                    pos.x = rect.x + (rect.w - textw) * 0.5f;
                } break;

                case GS_GUI_JUSTIFY_END:
                {
                    pos.x = rect.x + (rect.w - textw) - padding_right;
                } break;
            }

            // Determine caret position based on style justification
            gs_gui_rect_t cr = gs_gui_rect(pos.x + textw + padding_right, 
				(f32)rect.y + 5.f, 1.f, (f32)rect.h - 10.f); 

            if (ctx->last_focus_state == GS_GUI_ELEMENT_STATE_ON_FOCUS) {on = true; ct = 0.f;}
            ct += 0.1f;
            if (ct >= 3.f) {on = !on; ct = 0.f;}
            gs_color_t col = *color;
            col.a = on ? col.a : 0;
            gs_gui_draw_rect(ctx, cr, col); 
        }

		gs_gui_pop_clip_rect(ctx);
	} 
    else 
    {
        gs_gui_style_t* sp = &style;
		gs_color_t* color = &sp->colors[GS_GUI_COLOR_CONTENT];
		gs_asset_font_t* font = sp->font; 
        int32_t sx = sp->shadow_x;
        int32_t sy = sp->shadow_y;
		gs_color_t* sc = &sp->colors[GS_GUI_COLOR_SHADOW];
		int32_t textw = gs_gui_text_width(font, buf, -1);
		int32_t texth = gs_gui_text_height(font, buf, -1);
		int32_t textx = (int32_t)(rect.x + sp->padding[GS_GUI_PADDING_LEFT]);
		int32_t texty = (int32_t)(rect.y + (rect.h - texth) / 2);
		gs_gui_push_clip_rect(ctx, rect); 
        gs_gui_draw_control_text(ctx, buf, rect, &style, opt); 
		gs_gui_pop_clip_rect(ctx);
	}

    gs_gui_pop_style(ctx, save);

	return res;
}

static int32_t gs_gui_number_textbox(gs_gui_context_t *ctx, gs_gui_real *value, gs_gui_rect_t r, gs_gui_id id, const gs_gui_selector_desc_t* desc) 
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
        // This is broken for some reason...
		int32_t res = gs_gui_textbox_raw(ctx, ctx->number_edit_buf, 
			sizeof(ctx->number_edit_buf), id, r, desc, 0);

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

GS_API_DECL int32_t gs_gui_textbox_ex(gs_gui_context_t* ctx, char* buf, int32_t bufsz, const gs_gui_selector_desc_t* desc, int32_t opt)
{
    // Handle animation here...
    int32_t res = 0;
	gs_gui_id id = gs_gui_get_id(ctx, &buf, sizeof(buf)); 
    int32_t elementid = GS_GUI_ELEMENT_INPUT; 
    gs_gui_style_t style = gs_default_val();
    gs_gui_animation_t* anim = gs_gui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this) 
    if (anim)
    {
        // Need to check that I haven't updated more than once this frame
        gs_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = gs_gui_animation_get_blend_style(ctx, anim, desc, elementid); 
    }
    else
    { 
        style = ctx->focus == id ? gs_gui_get_current_element_style(ctx, desc, elementid, 0x02) : 
                ctx->hover == id ? gs_gui_get_current_element_style(ctx, desc, elementid, 0x01) : 
                                   gs_gui_get_current_element_style(ctx, desc, elementid, 0x00);
    } 

    // Push temp style
    gs_gui_style_t* save = gs_gui_push_style(ctx, &style); 
	gs_gui_rect_t r = gs_gui_layout_next(ctx);
	gs_gui_update_control(ctx, id, r, opt | GS_GUI_OPT_HOLDFOCUS);
    res |= gs_gui_textbox_raw(ctx, buf, bufsz, id, r, desc, opt);
    gs_gui_pop_style(ctx, save);

    return res;
} 

GS_API_DECL int32_t gs_gui_slider_ex(gs_gui_context_t* ctx, gs_gui_real* value, gs_gui_real low, gs_gui_real high, gs_gui_real step, 
        const char* fmt, const gs_gui_selector_desc_t* desc, int32_t opt)
{
	char buf[GS_GUI_MAX_FMT + 1];
	gs_gui_rect_t thumb;
	int32_t x, w, res = 0;
	gs_gui_real last = *value, v = last;
	gs_gui_id id = gs_gui_get_id(ctx, &value, sizeof(value));
    int32_t elementid = GS_GUI_ELEMENT_INPUT; 
    gs_gui_style_t style = gs_default_val();
    gs_gui_animation_t* anim = gs_gui_get_animation(ctx, id, desc, elementid);
    int32_t state = ctx->focus == id ? GS_GUI_ELEMENT_STATE_FOCUS : 
                    ctx->hover == id ? GS_GUI_ELEMENT_STATE_HOVER : 
                                       GS_GUI_ELEMENT_STATE_DEFAULT; 

    // Update anim (keep states locally within animation, only way to do this) 
    if (anim)
    {
        gs_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = gs_gui_animation_get_blend_style(ctx, anim, desc, elementid); 
    }
    else
    { 
        style = gs_gui_get_current_element_style(ctx, desc, elementid, state); 
    } 

    // Temporary copy of style
    gs_gui_style_t* save = gs_gui_push_style(ctx, &style); 
	gs_gui_rect_t base = gs_gui_layout_next(ctx);

	/* handle text input mode */
	if (gs_gui_number_textbox(ctx, &v, base, id, desc)) {return res;}

	/* handle normal mode */
	gs_gui_update_control(ctx, id, base, opt);

	/* handle input */
	if (ctx->focus == id &&
			(ctx->mouse_down | ctx->mouse_pressed) == GS_GUI_MOUSE_LEFT)
	{
		v = low + (ctx->mouse_pos.x - base.x) * (high - low) / base.w;
		if (step) {v = (((v + step / 2) / step)) * step;}
	}

	/* clamp and store value, update res */
	*value = v = gs_clamp(v, low, high);
	if (last != v) {res |= GS_GUI_RES_CHANGE;} 

	/* draw base */
	gs_gui_draw_control_frame(ctx, id, base, GS_GUI_ELEMENT_INPUT, opt);

	/* draw control */
	w = style.thumb_size; // Don't like this...
	x = (int32_t)((v - low) * (base.w - w) / (high - low));
	thumb = gs_gui_rect((f32)base.x + (f32)x, base.y, (f32)w, base.h);
	gs_gui_draw_control_frame(ctx, id, thumb, GS_GUI_ELEMENT_BUTTON, opt); 

	/* draw text	*/
    style.colors[GS_GUI_COLOR_BACKGROUND] = ctx->style_sheet->styles[GS_GUI_ELEMENT_TEXT][state].colors[GS_GUI_COLOR_BACKGROUND];
	gs_snprintf(buf, GS_GUI_MAX_FMT, fmt, v);
	gs_gui_draw_control_text(ctx, buf, base, &style, opt);  // oh...bg

    // Pop style
    gs_gui_pop_style(ctx, save);

	return res;
} 

GS_API_DECL int32_t gs_gui_number_ex(gs_gui_context_t* ctx, gs_gui_real* value, gs_gui_real step, const char* fmt, 
        const gs_gui_selector_desc_t* desc, int32_t opt)
{
	char buf[GS_GUI_MAX_FMT + 1];
	int32_t res = 0; 
	gs_gui_id id = gs_gui_get_id(ctx, &value, sizeof(value));
    int32_t elementid = GS_GUI_ELEMENT_INPUT; 
    gs_gui_style_t style = gs_default_val();
    gs_gui_animation_t* anim = gs_gui_get_animation(ctx, id, desc, elementid);

    // Update anim (keep states locally within animation, only way to do this) 
    if (anim)
    {
        gs_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = gs_gui_animation_get_blend_style(ctx, anim, desc, elementid); 
    }
    else
    { 
        style = ctx->focus == id ? gs_gui_get_current_element_style(ctx, desc, elementid, 0x02) : 
                ctx->hover == id ? gs_gui_get_current_element_style(ctx, desc, elementid, 0x01) : 
                                   gs_gui_get_current_element_style(ctx, desc, elementid, 0x00);
    } 

    // Temporary copy of style
    gs_gui_style_t* save = gs_gui_push_style(ctx, &style); 
	gs_gui_rect_t base = gs_gui_layout_next(ctx); 
	gs_gui_real last = *value;

	/* handle text input mode */
	if (gs_gui_number_textbox(ctx, value, base, id, desc)) { 
		gs_gui_pop_style(ctx, save);
		return res; 
	}

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
	gs_gui_draw_control_frame(ctx, id, base, GS_GUI_ELEMENT_INPUT, opt);

	/* draw text	*/
	gs_snprintf(buf, GS_GUI_MAX_FMT, fmt, *value);
	gs_gui_draw_control_text(ctx, buf, base, &ctx->style_sheet->styles[GS_GUI_ELEMENT_TEXT][0], opt);

    gs_gui_pop_style(ctx, save);

	return res;
} 

static int32_t _gs_gui_header(gs_gui_context_t *ctx, const char *label, int32_t istreenode, 
        const gs_gui_selector_desc_t* desc, int32_t opt) 
{
	gs_gui_rect_t r;
	int32_t active, expanded;
	gs_gui_id id = gs_gui_get_id(ctx, label, strlen(label));
	int32_t idx = gs_gui_pool_get(ctx, ctx->treenode_pool, GS_GUI_TREENODEPOOL_SIZE, id);
	int32_t width = -1;
	gs_gui_layout_row(ctx, 1, &width, 0);

    char id_tag[256] = gs_default_val(); 
    char label_tag[256] = gs_default_val(); 
    gs_gui_parse_id_tag(ctx, label, id_tag, sizeof(id_tag));
    gs_gui_parse_label_tag(ctx, label, label_tag, sizeof(label_tag));

    if (id_tag) gs_gui_push_id(ctx, id_tag, strlen(id_tag));

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
            gs_gui_draw_frame(ctx, r, &ctx->style_sheet->styles[GS_GUI_ELEMENT_BUTTON][GS_GUI_ELEMENT_STATE_HOVER]); 
        } 
	} 
    else 
    {
		gs_gui_draw_control_frame(ctx, id, r, GS_GUI_ELEMENT_BUTTON, 0);
	}

    const float sz = 6.f;
    if (expanded)
    {
        gs_vec2 a = {r.x + sz / 2.f, r.y + (r.h - sz) / 2.f};
        gs_vec2 b = gs_vec2_add(a, gs_v2(sz, 0.f));
        gs_vec2 c = gs_vec2_add(a, gs_v2(sz / 2.f, sz));
        gs_gui_draw_triangle(ctx, a, b, c, ctx->style_sheet->styles[GS_GUI_ELEMENT_TEXT][0x00].colors[GS_GUI_COLOR_CONTENT]);
    }
    else
    {
        gs_vec2 a = {r.x + sz / 2.f, r.y + (r.h - sz) / 2.f};
        gs_vec2 b = gs_vec2_add(a, gs_v2(sz, sz / 2.f));
        gs_vec2 c = gs_vec2_add(a, gs_v2(0.f, sz));
        gs_gui_draw_triangle(ctx, a, b, c, ctx->style_sheet->styles[GS_GUI_ELEMENT_TEXT][0x00].colors[GS_GUI_COLOR_CONTENT]);
    }

    // Draw text for treenode
	r.x += r.h - ctx->style->padding[GS_GUI_PADDING_TOP];
	r.w -= r.h - ctx->style->padding[GS_GUI_PADDING_BOTTOM]; 
	gs_gui_draw_control_text(ctx, label_tag, r, &ctx->style_sheet->styles[GS_GUI_ELEMENT_TEXT][0x00], 0);

    if (id_tag) gs_gui_pop_id(ctx);

	return expanded ? GS_GUI_RES_ACTIVE : 0;
} 

GS_API_DECL int32_t gs_gui_header_ex(gs_gui_context_t* ctx, const char* label, const gs_gui_selector_desc_t* desc, int32_t opt)
{
	return _gs_gui_header(ctx, label, 0, desc, opt);
}

GS_API_DECL int32_t gs_gui_treenode_begin_ex(gs_gui_context_t * ctx, const char* label, const gs_gui_selector_desc_t* desc, int32_t opt)
{
	int32_t res = _gs_gui_header(ctx, label, 1, desc, opt);
	if (res & GS_GUI_RES_ACTIVE) 
    {
		gs_gui_get_layout(ctx)->indent += ctx->style->indent;
		gs_gui_stack_push(ctx->id_stack, ctx->last_id);
	}

	return res;
} 

GS_API_DECL void gs_gui_treenode_end(gs_gui_context_t *ctx) 
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

    gs_gui_container_t* scnt = (gs_gui_container_t*)tab_bar->items[idx].data;

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

GS_API_DECL int32_t gs_gui_window_begin_ex(gs_gui_context_t * ctx, const char* title, gs_gui_rect_t rect, bool* open, 
        const gs_gui_selector_desc_t* desc, int32_t opt)
{ 
	gs_gui_rect_t body;
	gs_gui_id id = gs_gui_get_id(ctx, title, strlen(title)); 
	gs_gui_container_t* cnt = gs_gui_get_container_ex(ctx, id, opt); 

    char id_tag[256] = gs_default_val(); 
    char label_tag[256] = gs_default_val(); 
    gs_gui_parse_id_tag(ctx, title, id_tag, sizeof(id_tag));
    gs_gui_parse_label_tag(ctx, title, label_tag, sizeof(label_tag));

    if (cnt && open) 
    {
        cnt->open = *open;
    }

	if (!cnt || !cnt->open) 
    {
        return 0;
    } 

	memcpy(cnt->name, label_tag, 256);

    const int32_t title_max_size = 100;

    bool new_frame = cnt->frame != ctx->frame;

    int32_t state = ctx->active_root == cnt ? GS_GUI_ELEMENT_STATE_FOCUS : 
                    ctx->hover_root == cnt ? GS_GUI_ELEMENT_STATE_HOVER : 
                                             GS_GUI_ELEMENT_STATE_DEFAULT; 

    const float split_size = GS_GUI_SPLIT_SIZE;

	gs_gui_stack_push(ctx->id_stack, id); 

    // Get splits
    gs_gui_split_t* split = gs_gui_get_split(ctx, cnt); 
    gs_gui_split_t* root_split = gs_gui_get_root_split(ctx, cnt); 

    // Get root container
    gs_gui_container_t* root_cnt = gs_gui_get_root_container(ctx, cnt);

    // Cache rect
	if ((cnt->rect.w == 0.f || opt & GS_GUI_OPT_FORCESETRECT || opt & GS_GUI_OPT_FULLSCREEN || cnt->flags & GS_GUI_WINDOW_FLAGS_FIRST_INIT) && new_frame) 
    {
        if (opt & GS_GUI_OPT_FULLSCREEN)
        {
            gs_vec2 fb = ctx->framebuffer_size;
            cnt->rect = gs_gui_rect(0, 0, fb.x, fb.y);

            // Set root split rect size
            if (root_split)
            {
                root_split->rect = cnt->rect;
                gs_gui_update_split(ctx, root_split);
            } 
        }
        else
        { 
            // Set root split rect size
            if (root_split && root_cnt == cnt)
            {
                root_split->rect = rect;
                gs_gui_update_split(ctx, root_split);
            } 
            else
            {
                cnt->rect = rect;
            }
        }
        cnt->flags = cnt->flags & ~GS_GUI_WINDOW_FLAGS_FIRST_INIT;
    }
	gs_gui_begin_root_container(ctx, cnt, opt);
	rect = body = cnt->rect;
    cnt->opt = opt;

    if (opt & GS_GUI_OPT_DOCKSPACE)
    {
        cnt->zindex = 0;
    }

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
                s_cnt = (gs_gui_container_t*)tab_bar->items[i].data;
            }
        }
    } 

    // Do split size/position
    if (split)
    { 
        const gs_gui_style_t* cstyle = &ctx->style_sheet->styles[GS_GUI_ELEMENT_CONTAINER][state];
        const gs_gui_rect_t* sr = &split->rect;
        const float ratio = split->ratio;
        float shsz = split_size;
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
            // This lock id is what I need...

            ctx->active_root = cnt;

            if (tab_bar)
            {
                ctx->next_focus_root = (gs_gui_container_t*)(tab_bar->items[tab_bar->focus].data);
                gs_gui_bring_to_front(ctx, (gs_gui_container_t*)tab_bar->items[tab_bar->focus].data);
                if (id == ctx->focus && tab_bar->focus != tab_item->idx) ctx->lock_focus = id;
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
            else
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
            int32_t tab_width = (s32)gs_min(r->w / (float)tab_bar->size, title_max_size); 
            tw = tab_item->zindex ? (s32)tab_width : (s32)(tab_width + 1.f); 

            // Determine position (based on zindex and total width)
            float xoff = 0.f; //tab_item->zindex ? 2.f : 0.f;  
            tr.x = tab_bar->rect.x + tab_width * tab_item->zindex + xoff; 
        }

        gs_gui_rect_t r = gs_gui_rect(tr.x + split_size, y, (f32)tw, h); 

        gs_gui_update_control(ctx, id, r, opt); 

        // Need to move the entire thing
        if ((id == ctx->hover || id == ctx->focus) && ctx->mouse_down == GS_GUI_MOUSE_LEFT) 
        { 
            gs_gui_set_focus(ctx, id); 
            ctx->next_focus_root = cnt; 
            ctx->active_root = cnt;

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
        // gs_gui_update_control(ctx, id, br, (opt | GS_GUI_OPT_NOSWITCHSTATE)); 

        // Need to move the entire thing
        if (ctx->hover_root == cnt && !ctx->focus_split && !ctx->focus && !ctx->lock_focus && !ctx->hover && ctx->mouse_down == GS_GUI_MOUSE_LEFT) 
        {
            ctx->active_root = cnt;
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
    }

    if (~opt & GS_GUI_OPT_NOTITLE)
    {
		gs_gui_rect_t tr = cnt->rect;
		tr.h = ctx->style->title_height;
        if (split)
        {
            const float sh = split_size * 0.5f; 
        }
        body.y += tr.h;
        body.h -= tr.h; 
    } 

    int32_t zindex = INT32_MAX - 1;
    if (root_split) {
        gs_gui_get_split_lowest_zindex(ctx, root_split, &zindex);
        if (zindex == cnt->zindex) {
            gs_gui_style_t* style = &ctx->style_sheet->styles[GS_GUI_ELEMENT_CONTAINER][state];
            gs_gui_draw_rect(ctx, root_split->rect, style->colors[GS_GUI_COLOR_BACKGROUND]); 
            gs_gui_draw_splits(ctx, root_split);
        } 
    }

	// draw body frame
	if (~opt & GS_GUI_OPT_NOFRAME && cnt->flags & GS_GUI_WINDOW_FLAGS_VISIBLE) 
    { 
        gs_gui_style_t* style = &ctx->style_sheet->styles[GS_GUI_ELEMENT_CONTAINER][state];

		if (ctx->active_root == root_cnt)
		{ 
			int32_t f = 0;
		}

        gs_gui_draw_rect(ctx, body, style->colors[GS_GUI_COLOR_BACKGROUND]); 

        // draw border (get root cnt and check state of that)
        if (split)
        { 
            int32_t root_state = ctx->active_root == root_cnt ? GS_GUI_ELEMENT_STATE_FOCUS : 
                                 ctx->hover_root == root_cnt ?  GS_GUI_ELEMENT_STATE_HOVER : 
                                                                GS_GUI_ELEMENT_STATE_DEFAULT;

            bool share_split = ctx->active_root && gs_gui_get_root_container(ctx, ctx->active_root) == root_cnt ? true : 
                               false;

            // Have to look and see if hovered root shares split...
            gs_gui_style_t* root_style = style;
            if (share_split)
            {
                root_style = &ctx->style_sheet->styles[GS_GUI_ELEMENT_CONTAINER][GS_GUI_ELEMENT_STATE_FOCUS];  
            }
            else
            {
                root_style =
                    state      == GS_GUI_ELEMENT_STATE_FOCUS ? style : 
                    root_state == GS_GUI_ELEMENT_STATE_FOCUS ? &ctx->style_sheet->styles[GS_GUI_ELEMENT_CONTAINER][root_state] : 
                    root_state == GS_GUI_ELEMENT_STATE_HOVER ? &ctx->style_sheet->styles[GS_GUI_ELEMENT_CONTAINER][root_state] : 
                                                               style;
            }
            if (~opt & GS_GUI_OPT_NOBORDER && root_style->colors[GS_GUI_COLOR_BORDER].a) { 
                gs_gui_draw_box(ctx, gs_gui_expand_rect(split->rect, (int16_t*)root_style->border_width), (int16_t*)root_style->border_width, root_style->colors[GS_GUI_COLOR_BORDER]); 
            }
        }
        else
        {
            if (~opt & GS_GUI_OPT_NOBORDER && style->colors[GS_GUI_COLOR_BORDER].a) { 
                gs_gui_draw_box(ctx, gs_gui_expand_rect(cnt->rect, (int16_t*)style->border_width), (int16_t*)style->border_width, style->colors[GS_GUI_COLOR_BORDER]); 
            }
        }
	} 

    if (split && ~opt & GS_GUI_OPT_NOCLIP && cnt->flags & GS_GUI_WINDOW_FLAGS_VISIBLE)
    {
        int16_t exp[] = {1, 1, 1, 1};
		gs_gui_push_clip_rect(ctx, gs_gui_expand_rect(cnt->rect, exp));
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
        gs_gui_style_t* cstyle = &ctx->style_sheet->styles[GS_GUI_ELEMENT_CONTAINER][state];
		gs_gui_rect_t tr = cnt->rect;
		tr.h = ctx->style->title_height;
        if (split)
        {
            const float sh = split_size * 0.5f; 
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
                    gs_gui_draw_frame(ctx, tr, &ctx->style_sheet->styles[GS_GUI_ELEMENT_PANEL][0x00]);
		            // gs_gui_draw_box(ctx, gs_gui_expand_rect(tr, (int16_t*)cstyle->border_width), (int16_t*)cstyle->border_width, cstyle->colors[GS_GUI_COLOR_BORDER]);
                }
            }
        }

        else
        {
            gs_gui_draw_frame(ctx, tr, &ctx->style_sheet->styles[GS_GUI_ELEMENT_PANEL][0x00]); 
		    // gs_gui_draw_box(ctx, gs_gui_expand_rect(tr, (int16_t*)cstyle->border_width), cstyle->border_width, cstyle->colors[GS_GUI_COLOR_BORDER]);
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
                int32_t tab_width = (s32)gs_min(r->w / (float)tab_bar->size, title_max_size); 
                tw = (s32)(tab_width - 2.f);

                // Determine position (based on zindex and total width)
                float xoff = !tab_item->zindex ? split_size : 2.f; //tab_item->zindex ? 2.f : 0.f;  
                tr.x = tab_bar->rect.x + tab_width * tab_item->zindex + xoff;
            }

			gs_gui_rect_t r = gs_gui_rect(tr.x + split_size, y, (f32)tw, h);

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

            gs_color_t def = ctx->style_sheet->styles[GS_GUI_ELEMENT_BUTTON][0x00].colors[GS_GUI_COLOR_BACKGROUND];
            gs_color_t hov = ctx->style_sheet->styles[GS_GUI_ELEMENT_BUTTON][0x01].colors[GS_GUI_COLOR_BACKGROUND];
            gs_color_t act = ctx->style_sheet->styles[GS_GUI_ELEMENT_BUTTON][0x02].colors[GS_GUI_COLOR_BACKGROUND]; 
            gs_color_t inactive = gs_color(10, 10, 10, 50);

            int16_t exp[] = {1, 1, 1, 1};
			gs_gui_push_clip_rect(ctx, gs_gui_expand_rect(cnt->rect, exp));

            gs_gui_push_clip_rect(ctx, r);

            gs_gui_draw_rect(ctx, r, id == ctx->focus ? act : hovered ? hov : tab_focus ? def : inactive); 
            gs_gui_draw_control_text(ctx, label_tag, r, &ctx->style_sheet->styles[GS_GUI_ELEMENT_CONTAINER][state], opt); 

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
        /*
		gs_gui_rect_t r = gs_gui_get_layout(ctx)->body;
        cnt->rect.w = cnt->content_size.x + (cnt->rect.w - r.w);
		cnt->rect.h = cnt->content_size.y + (cnt->rect.h - r.h);
        */
	} 

    if (split && ~opt & GS_GUI_OPT_NOCLIP && cnt->flags & GS_GUI_WINDOW_FLAGS_VISIBLE)
    {
        gs_gui_pop_clip_rect(ctx); 
    }

    // Draw border
	if (~opt & GS_GUI_OPT_NOFRAME && cnt->flags & GS_GUI_WINDOW_FLAGS_VISIBLE) 
    {
        const int* w = (int*)ctx->style_sheet->styles[GS_GUI_ELEMENT_CONTAINER][0x00].border_width;
        const gs_color_t* bc = &ctx->style_sheet->styles[GS_GUI_ELEMENT_CONTAINER][0x00].colors[GS_GUI_COLOR_BORDER]; 
		// gs_gui_draw_box(ctx, gs_gui_expand_rect(cnt->rect, w), w, *bc); 
	}

    gs_gui_push_container_body(ctx, cnt, body, desc, opt);

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

GS_API_DECL void gs_gui_window_end(gs_gui_context_t *ctx) 
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
        gs_gui_rect_t r = gs_gui_rect(cnt->rect.x + cnt->rect.w - (f32)sz, cnt->rect.y + cnt->rect.h - (f32)sz, (f32)sz, (f32)sz);
        gs_gui_update_control(ctx, id, r, opt);
        if (id == ctx->focus && ctx->mouse_down == GS_GUI_MOUSE_LEFT) 
        { 
            ctx->active_root = cnt;
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
        const uint32_t grid = 5;
        const float w = r.w / (float)grid;
        const float h = r.h / (float)grid;
        const float m = 2.f;
        const float o = 5.f;

        gs_color_t col = 
            ctx->focus == id ? ctx->style_sheet->styles[GS_GUI_ELEMENT_BUTTON][0x02].colors[GS_GUI_COLOR_BACKGROUND] : 
            ctx->hover == id ? ctx->style_sheet->styles[GS_GUI_ELEMENT_BUTTON][0x01].colors[GS_GUI_COLOR_BACKGROUND] : 
            ctx->style_sheet->styles[GS_GUI_ELEMENT_BUTTON][0x00].colors[GS_GUI_COLOR_BACKGROUND];  

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
        uint32_t ssz = (u32)(split ? GS_GUI_SPLIT_SIZE : 5);

        gs_gui_draw_rect(ctx, gs_gui_rect(r->x, r->y + r->h, r->w + 1, 1), ctx->style->colors[GS_GUI_COLOR_SHADOW]);
        gs_gui_draw_rect(ctx, gs_gui_rect(r->x, r->y + r->h, r->w + (f32)ssz, (f32)ssz), ctx->style->colors[GS_GUI_COLOR_SHADOW]);
        gs_gui_draw_rect(ctx, gs_gui_rect(r->x + r->w, r->y, 1, r->h), ctx->style->colors[GS_GUI_COLOR_SHADOW]);
        gs_gui_draw_rect(ctx, gs_gui_rect(r->x + r->w, r->y, (f32)ssz, r->h), ctx->style->colors[GS_GUI_COLOR_SHADOW]);
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
	gs_gui_root_container_end(ctx); 
} 

GS_API_DECL void gs_gui_popup_open(gs_gui_context_t* ctx, const char* name) 
{
	gs_gui_container_t *cnt = gs_gui_get_container(ctx, name);

	// Set as hover root so popup isn't closed in window_begin_ex()
	ctx->hover_root = ctx->next_hover_root = cnt;

	// position at mouse cursor, open and bring-to-front
	cnt->rect = gs_gui_rect(ctx->mouse_pos.x, ctx->mouse_pos.y, 100, 100);
	cnt->open = 1;
	gs_gui_bring_to_front(ctx, cnt);
} 

GS_API_DECL int32_t gs_gui_popup_begin_ex(gs_gui_context_t* ctx, const char* name, gs_gui_rect_t r, const gs_gui_selector_desc_t* desc, int32_t opt)
{
	opt |= (GS_GUI_OPT_POPUP | GS_GUI_OPT_NODOCK | GS_GUI_OPT_CLOSED); 
	return gs_gui_window_begin_ex(ctx, name, r, NULL, NULL, opt);
} 

GS_API_DECL void gs_gui_popup_end(gs_gui_context_t *ctx) 
{
	gs_gui_window_end(ctx);
} 

GS_API_DECL void gs_gui_panel_begin_ex(gs_gui_context_t* ctx, const char* name, const gs_gui_selector_desc_t* desc, int32_t opt)
{
	gs_gui_container_t *cnt; 
    const int32_t elementid = GS_GUI_ELEMENT_PANEL;
    char id_tag[256] = gs_default_val(); 
    gs_gui_parse_id_tag(ctx, name, id_tag, sizeof(id_tag));

	// if (id_tag) gs_gui_push_id(ctx, id_tag, strlen(id_tag));
    // else gs_gui_push_id(ctx, name, strlen(name));
    gs_gui_push_id(ctx, name, strlen(name));
	cnt = gs_gui_get_container_ex(ctx, ctx->last_id, opt);
	cnt->rect = gs_gui_layout_next(ctx);

    const gs_gui_id id = gs_gui_get_id(ctx, name, strlen(name));

    gs_gui_style_t style = gs_default_val();
    gs_gui_animation_t* anim = gs_gui_get_animation(ctx, id, desc, elementid); 

    // Update anim (keep states locally within animation, only way to do this) 
    if (anim)
    {
        gs_gui_animation_update(ctx, anim);

        // Get blended style based on animation
        style = gs_gui_animation_get_blend_style(ctx, anim, desc, elementid); 
    }
    else
    { 
        style = ctx->focus == id ? gs_gui_get_current_element_style(ctx, desc, elementid, 0x02) : 
                ctx->hover == id ? gs_gui_get_current_element_style(ctx, desc, elementid, 0x01) : 
                                   gs_gui_get_current_element_style(ctx, desc, elementid, 0x00);
    } 
	
    if (~opt & GS_GUI_OPT_NOFRAME) {
		gs_gui_draw_frame(ctx, cnt->rect, &style);
	} 

    // Need a way to push/pop syles temp styles
	gs_gui_stack_push(ctx->container_stack, cnt);
	gs_gui_push_container_body(ctx, cnt, cnt->rect, desc, opt);
	gs_gui_push_clip_rect(ctx, cnt->body);
} 

GS_API_DECL void gs_gui_panel_end(gs_gui_context_t *ctx) 
{
	gs_gui_pop_clip_rect(ctx);
	gs_gui_pop_container(ctx);
} 

static uint8_t uint8_slider(gs_gui_context_t *ctx, unsigned char *value, int low, int high, const gs_gui_selector_desc_t* desc, int32_t opt) 
{
    static float tmp;
    gs_gui_push_id(ctx, &value, sizeof(value));
    tmp = (float)*value;
    int res = gs_gui_slider_ex(ctx, &tmp, (gs_gui_real)low, (gs_gui_real)high, 0, "%.0f", desc, opt);
    *value = (u8)tmp;
    gs_gui_pop_id(ctx);
    return res;
}

static int32_t int32_slider(gs_gui_context_t *ctx, int32_t* value, int32_t low, int32_t high, const gs_gui_selector_desc_t* desc, int32_t opt) 
{
    static float tmp;
    gs_gui_push_id(ctx, &value, sizeof(value));
    tmp = (float)*value;
    int res = gs_gui_slider_ex(ctx, &tmp, (gs_gui_real)low, (gs_gui_real)high, 0, "%.0f", desc, opt);
    *value = (int32_t)tmp;
    gs_gui_pop_id(ctx);
    return res;
}

static int16_t int16_slider(gs_gui_context_t *ctx, int16_t* value, int32_t low, int32_t high, const gs_gui_selector_desc_t* desc, int32_t opt) 
{
    static float tmp;
    gs_gui_push_id(ctx, &value, sizeof(value));
    tmp = (float)*value;
    int res = gs_gui_slider_ex(ctx, &tmp, (gs_gui_real)low, (gs_gui_real)high, 0, "%.0f", desc, opt);
    *value = (int16_t)tmp;
    gs_gui_pop_id(ctx);
    return res;
} 

//=== Gizmo ===// 

typedef struct
{
    b32 hit;
    gs_vec3 point;
} gs_gui_gizmo_line_intersection_result_t;

enum {
    GS_GUI_AXIS_RIGHT = 0x01, 
    GS_GUI_AXIS_UP, 
    GS_GUI_AXIS_FORWARD
};

typedef struct {
    struct {
        gs_vqs model;
        union {
            gs_cylinder_t cylinder;
            gs_plane_t plane;
        } shape;
    } axis;
    struct {
        gs_vqs model;
        union {
            gs_cone_t cone;
            gs_aabb_t aabb;
        } shape;
    } cap;
} gs_gui_axis_t;

typedef struct { 
    gs_gui_axis_t right;
    gs_gui_axis_t up;
    gs_gui_axis_t forward;
} gs_gizmo_translate_t;

typedef struct {
    gs_gui_axis_t right;
    gs_gui_axis_t up;
    gs_gui_axis_t forward;
} gs_gizmo_scale_t;

typedef struct {
    gs_gui_axis_t right;
    gs_gui_axis_t up;
    gs_gui_axis_t forward;
} gs_gizmo_rotate_t;

static gs_gizmo_scale_t gs_gizmo_scale(const gs_vqs* parent)
{
    gs_gizmo_scale_t gizmo = gs_default_val();
	const gs_vec3 ax_scl = gs_v3(0.03f, 1.f, 0.03f);
	const gs_vec3 cap_scl = gs_vec3_scale(gs_v3s(0.05f), 2.f * gs_vec3_len(parent->scale));
	gs_vqs local = gs_vqs_default();
	gs_vqs abs = gs_vqs_default(); 

#define GS_GUI_GIZMO_AXIS_DEFINE_SCALE(MEMBER, OFFSET, DEG, AXIS)\
    do {\
		/* Axis */\
		{\
			local = gs_vqs_ctor(\
				OFFSET,\
				gs_quat_angle_axis(gs_deg2rad(DEG), AXIS),\
				ax_scl\
			);\
			gizmo.MEMBER.axis.model = gs_vqs_absolute_transform(&local, parent);\
			gs_cylinder_t axis = gs_default_val();\
			axis.r = 1.f;\
			axis.base = gs_v3(0.f, 0.0f, 0.f);\
			axis.height = 1.f;\
			gizmo.MEMBER.axis.shape.cylinder = axis;\
		}\
\
		/* Cap */\
		{\
			local = gs_vqs_ctor(\
				gs_v3(0.f, 0.5f, 0.f),\
				gs_quat_default(),\
				gs_v3s(1.f)\
			);\
\
			gizmo.MEMBER.cap.model = gs_vqs_absolute_transform(&local, &gizmo.MEMBER.axis.model);\
			gizmo.MEMBER.cap.model.scale = cap_scl;\
			gs_aabb_t aabb = gs_default_val();\
			aabb.min = gs_v3s(-0.5f);\
			aabb.max = gs_v3s(0.5f);\
			gizmo.MEMBER.cap.shape.aabb = aabb;\
		}\
	} while (0) 

    const float off = 0.6f;
    GS_GUI_GIZMO_AXIS_DEFINE_SCALE(right, gs_v3(-off, 0.f, 0.f), 90.f, GS_ZAXIS);
    GS_GUI_GIZMO_AXIS_DEFINE_SCALE(up, gs_v3(0.f, off, 0.f), 0.f, GS_YAXIS);
    GS_GUI_GIZMO_AXIS_DEFINE_SCALE(forward, gs_v3(0.f, 0.f, off), 90.f, GS_XAXIS);

	return gizmo;
}

static gs_gizmo_translate_t gs_gizmo_translate(const gs_vqs* parent)
{
    gs_gizmo_translate_t trans = gs_default_val();
	const gs_vec3 ax_scl = gs_v3(0.03f, 1.f, 0.03f);
	const gs_vec3 cap_scl = gs_vec3_scale(gs_v3(0.02f, 0.05f, 0.02f), 2.f * gs_vec3_len(parent->scale));
	gs_vqs local = gs_vqs_default();
	gs_vqs abs = gs_vqs_default(); 

#define GS_GUI_GIZMO_AXIS_DEFINE_TRANSLATE(MEMBER, OFFSET, DEG, AXIS)\
    do {\
		/* Axis */\
		{\
			local = gs_vqs_ctor(\
				OFFSET,\
				gs_quat_angle_axis(gs_deg2rad(DEG), AXIS),\
				ax_scl\
			);\
			trans.MEMBER.axis.model = gs_vqs_absolute_transform(&local, parent);\
			gs_cylinder_t axis = gs_default_val();\
			axis.r = 1.f;\
			axis.base = gs_v3(0.f, 0.0f, 0.f);\
			axis.height = 1.f;\
			trans.MEMBER.axis.shape.cylinder = axis;\
		}\
\
		/* Cap */\
		{\
			local = gs_vqs_ctor(\
				gs_v3(0.f, 0.5f, 0.f),\
				gs_quat_default(),\
				gs_v3s(1.f)\
			);\
\
			trans.MEMBER.cap.model = gs_vqs_absolute_transform(&local, &trans.MEMBER.axis.model);\
			trans.MEMBER.cap.model.scale = cap_scl;\
			gs_cone_t cap = gs_default_val();\
			cap.r = 1.f;\
			cap.base = gs_v3(0.f, 0.0f, 0.f);\
			cap.height = 1.0f;\
			trans.MEMBER.cap.shape.cone = cap;\
		}\
	} while (0)

    const float off = 0.6f;
    GS_GUI_GIZMO_AXIS_DEFINE_TRANSLATE(right, gs_v3(-off, 0.f, 0.f), 90.f, GS_ZAXIS);
    GS_GUI_GIZMO_AXIS_DEFINE_TRANSLATE(up, gs_v3(0.f, off, 0.f), 0.f, GS_YAXIS);
    GS_GUI_GIZMO_AXIS_DEFINE_TRANSLATE(forward, gs_v3(0.f, 0.f, off), 90.f, GS_XAXIS);

	return trans;
}

static gs_gizmo_rotate_t gs_gizmo_rotate(const gs_vqs* parent)
{
    gs_gizmo_rotate_t gizmo = gs_default_val();
	const gs_vec3 ax_scl = gs_v3(1.f, 1.f, 1.f);
	gs_vqs local = gs_vqs_default();
	gs_vqs abs = gs_vqs_default(); 

#define GS_GUI_GIZMO_AXIS_DEFINE_ROTATE(MEMBER, OFFSET, DEG, AXIS)\
    do {\
		/* Axis */\
		{\
			local = gs_vqs_ctor(\
				OFFSET,\
				gs_quat_angle_axis(gs_deg2rad(DEG), AXIS),\
				ax_scl\
			);\
			gizmo.MEMBER.axis.model = gs_vqs_absolute_transform(&local, parent);\
			gs_plane_t axis = gs_plane_from_pt_normal(gs_v3s(0.f), GS_ZAXIS);\
			gizmo.MEMBER.axis.shape.plane = axis;\
		}\
	} while (0)

    GS_GUI_GIZMO_AXIS_DEFINE_ROTATE(right, gs_v3(0.f, 0.f, 0.f), 90.f, GS_YAXIS);
    GS_GUI_GIZMO_AXIS_DEFINE_ROTATE(up, gs_v3(0.f, 0.f, 0.f), -90.f, GS_XAXIS);
    GS_GUI_GIZMO_AXIS_DEFINE_ROTATE(forward, gs_v3(0.f, 0.f, 0.f), 0.f, GS_ZAXIS);

	return gizmo;
}

typedef struct {
    int32_t op;
    int32_t mode;
    gs_vqs xform;
    gs_contact_info_t info;
    union {
        gs_gizmo_translate_t translate;
        gs_gizmo_scale_t scale;
        gs_gizmo_rotate_t rotate;
    } gizmo;
    int16_t hover;
	gs_camera_t camera;
    gs_gui_rect_t viewport;
    gs_gui_gizmo_line_intersection_result_t li;
} gs_gizmo_desc_t;

static void gs_gui_gizmo_render(gs_gui_context_t* ctx, gs_gui_customcommand_t* cmd)
{ 
	const gs_vec2 fbs = ctx->framebuffer_size;
	const float t = gs_platform_elapsed_time();
    gs_immediate_draw_t* gsi = &ctx->gsi;
    gs_gizmo_desc_t* desc = (gs_gizmo_desc_t*)cmd->data;
    gs_gui_rect_t clip = cmd->clip;
    gs_gui_rect_t viewport = desc->viewport;
	gs_camera_t cam = desc->camera;
    const uint16_t segments = 4;
    const gs_gui_gizmo_line_intersection_result_t* li = &desc->li;
    const gs_contact_info_t* info = &desc->info;

	gsi_defaults(gsi);
	gsi_depth_enabled(gsi, false);
    gsi_camera(gsi, &cam, (uint32_t)viewport.w, (uint32_t)viewport.h);
    gs_graphics_set_viewport(&gsi->commands, (u32)viewport.x, (u32)(fbs.y - viewport.h - viewport.y), 
            (u32)viewport.w, (u32)viewport.h); 
	gs_graphics_primitive_type primitive = GS_GRAPHICS_PRIMITIVE_TRIANGLES; 

#define GS_GUI_GIZMO_AXIS_TRANSLATE(ID, AXIS, COLOR)\
    do {\
        gs_gui_id id = gs_gui_get_id_hash(ctx, ID, strlen(ID), cmd->hash);\
        bool hover = cmd->hover == id;\
        bool focus = cmd->focus == id;\
        gs_color_t color = hover || focus ? GS_COLOR_YELLOW : COLOR;\
        /* Axis */\
        {\
            gsi_push_matrix(gsi, GSI_MATRIX_MODELVIEW);\
            gsi_mul_matrix(gsi, gs_vqs_to_mat4(&desc->gizmo.translate.AXIS.axis.model));\
            {\
                gs_cylinder_t* axis = &desc->gizmo.translate.AXIS.axis.shape.cylinder;\
                gsi_cylinder(gsi, axis->base.x, axis->base.y, axis->base.z, axis->r,\
                    axis->r, axis->height, segments, color.r, color.g, color.b, 100, primitive);\
            }\
            gsi_pop_matrix(gsi);\
        }\
\
        /* Cap */\
        {\
            gsi_push_matrix(gsi, GSI_MATRIX_MODELVIEW);\
            gsi_mul_matrix(gsi, gs_vqs_to_mat4(&desc->gizmo.translate.AXIS.cap.model));\
            {\
                gs_cone_t* cap = &desc->gizmo.translate.AXIS.cap.shape.cone;\
                gsi_cone(gsi, cap->base.x, cap->base.y, cap->base.z, cap->r, cap->height, segments, color.r, color.g, color.b, 100, primitive);\
            }\
            gsi_pop_matrix(gsi);\
        }\
    } while (0)

#define GS_GUI_GIZMO_AXIS_SCALE(ID, AXIS, COLOR)\
    do {\
        gs_gui_id id = gs_gui_get_id_hash(ctx, ID, strlen(ID), cmd->hash);\
        bool hover = cmd->hover == id;\
        bool focus = cmd->focus == id;\
        gs_color_t color = hover || focus ? GS_COLOR_YELLOW : COLOR;\
        /* Axis */\
        {\
            gsi_push_matrix(gsi, GSI_MATRIX_MODELVIEW);\
            gsi_mul_matrix(gsi, gs_vqs_to_mat4(&desc->gizmo.scale.AXIS.axis.model));\
            {\
                gs_cylinder_t* axis = &desc->gizmo.scale.AXIS.axis.shape.cylinder;\
                gsi_cylinder(gsi, axis->base.x, axis->base.y, axis->base.z, axis->r,\
                    axis->r, axis->height, segments, color.r, color.g, color.b, 100, primitive);\
            }\
            gsi_pop_matrix(gsi);\
        }\
\
        /* Cap */\
        {\
            gsi_push_matrix(gsi, GSI_MATRIX_MODELVIEW);\
            gsi_mul_matrix(gsi, gs_vqs_to_mat4(&desc->gizmo.scale.AXIS.cap.model));\
            {\
                gs_aabb_t* cap = &desc->gizmo.scale.AXIS.cap.shape.aabb;\
                gs_vec3 hd = gs_vec3_scale(gs_vec3_sub(cap->max, cap->min), 0.5f);\
                gs_vec3 c = gs_vec3_add(cap->min, hd);\
                gsi_box(gsi, c.x, c.y, c.z, hd.x, hd.y, hd.z, color.r, color.g, color.b, 100, primitive);\
            }\
            gsi_pop_matrix(gsi);\
        }\
    } while (0)

#define GS_GUI_GIZMO_AXIS_ROTATE(ID, AXIS, COLOR)\
    do {\
        gs_color_t def_color = (COLOR);\
        gs_gui_id id = gs_gui_get_id_hash(ctx, ID, strlen(ID), cmd->hash);\
        bool hover = cmd->hover == id;\
        bool focus = cmd->focus == id;\
        gs_color_t color = hover || focus ? GS_COLOR_YELLOW : def_color;\
        /* Axis */\
        {\
            gsi_push_matrix(gsi, GSI_MATRIX_MODELVIEW);\
            gsi_mul_matrix(gsi, gs_vqs_to_mat4(&desc->gizmo.rotate.AXIS.axis.model));\
            {\
                gs_plane_t* axis = &desc->gizmo.rotate.AXIS.axis.shape.plane;\
                gsi_arc(gsi, 0.f, 0.f, 0.92f, 1.f, 0.f, 360.f, 48, color.r, color.g, color.b, color.a, GS_GRAPHICS_PRIMITIVE_TRIANGLES);\
            }\
            gsi_pop_matrix(gsi);\
            if (focus) {\
                gs_vec3 ls = desc->xform.translation;\
                gs_vec3 le = gs_vec3_add(ls, gs_vec3_scale(info->normal, 0.5f));\
                gsi_line3Dv(gsi, ls, le, GS_COLOR_BLUE);\
            }\
        }\
    } while (0)

    switch (desc->op)
    {
        case GS_GUI_GIZMO_TRANSLATE:
        { 
            GS_GUI_GIZMO_AXIS_TRANSLATE("#gizmo_trans_right", right, GS_COLOR_RED);
            GS_GUI_GIZMO_AXIS_TRANSLATE("#gizmo_trans_up", up, GS_COLOR_GREEN);
            GS_GUI_GIZMO_AXIS_TRANSLATE("#gizmo_trans_forward", forward, GS_COLOR_BLUE); 
        } break;

        case GS_GUI_GIZMO_SCALE:
        {
            GS_GUI_GIZMO_AXIS_SCALE("#gizmo_scale_right", right, GS_COLOR_RED);
            GS_GUI_GIZMO_AXIS_SCALE("#gizmo_scale_up", up, GS_COLOR_GREEN);
            GS_GUI_GIZMO_AXIS_SCALE("#gizmo_scale_forward", forward, GS_COLOR_BLUE); 
        } break;

        case GS_GUI_GIZMO_ROTATE:
        {
            gs_gui_id id_r = gs_gui_get_id_hash(ctx, "#gizmo_rotate_right", strlen("#gizmo_rotate_right"), cmd->hash);
            gs_gui_id id_u = gs_gui_get_id_hash(ctx, "#gizmo_rotate_up", strlen("#gizmo_rotate_up"), cmd->hash);
            gs_gui_id id_f = gs_gui_get_id_hash(ctx, "#gizmo_rotate_forward", strlen("#gizmo_rotate_forward"), cmd->hash);

            GS_GUI_GIZMO_AXIS_ROTATE("#gizmo_rotate_right", right, 
                    (cmd->focus == id_u || cmd->focus == id_f) ? gs_color_alpha(GS_COLOR_RED, 25) : 
                    gs_color_alpha(GS_COLOR_RED, 100));
            GS_GUI_GIZMO_AXIS_ROTATE("#gizmo_rotate_up", up, 
                    (cmd->focus == id_r || cmd->focus == id_f) ?  gs_color_alpha(GS_COLOR_GREEN, 25) : 
                    gs_color_alpha(GS_COLOR_GREEN, 100));
            GS_GUI_GIZMO_AXIS_ROTATE("#gizmo_rotate_forward", forward, 
                    (cmd->focus == id_r || cmd->focus == id_u) ? gs_color_alpha(GS_COLOR_BLUE, 25) : 
                    gs_color_alpha(GS_COLOR_BLUE, 100));
        } break;
    } 

    if (li->hit)
    {
        gsi_sphere(gsi, li->point.x, li->point.y, li->point.z, 0.005f, 255, 0, 0, 255, GS_GRAPHICS_PRIMITIVE_LINES); 
        gsi_line3Dv(gsi, li->point, desc->xform.translation, gs_color(255, 0, 0, 255));
    }
} 

static gs_gui_gizmo_line_intersection_result_t 
gs_gui_gizmo_get_line_intersection(const gs_vqs* model, gs_vec3 axis_a, gs_vec3 axis_b, gs_vec3 axis_c, 
        const gs_camera_t* camera, const gs_ray_t* ray, gs_vec3 plane_normal_axis, bool compare_supporting_axes, bool override_axis)
{
    gs_gui_gizmo_line_intersection_result_t res = gs_default_val();

    // Find absolute dot between cam forward and right axis
    gs_vec3 cf = gs_vec3_norm(gs_camera_forward(camera));
    gs_vec3 ta = gs_vec3_norm(gs_quat_rotate(model->rotation, axis_a));
    float cfdta = fabsf(gs_vec3_dot(cf, ta));

    // This doesn't really make sense. I want to project along the x/y or x/z planes.

    gs_plane_t intersection_plane = gs_default_val();
    gs_vec3 op = model->translation;

    if (compare_supporting_axes)
    {
        // Now determine appropriate axis to move along
        gs_vec3 tb = gs_vec3_norm(gs_quat_rotate(model->rotation, axis_b));
        gs_vec3 tc = gs_vec3_norm(gs_quat_rotate(model->rotation, axis_c));

        float cfdtb = fabsf(gs_vec3_dot(cf, tb));
        float cfdtc = fabsf(gs_vec3_dot(cf, tc));

        intersection_plane = cfdtb < cfdtc ? gs_plane_from_pt_normal(op, tc) : 
            gs_plane_from_pt_normal(op, tb);
    }
    else
    {
        if (override_axis)
        { 
            intersection_plane = gs_plane_from_pt_normal(op, plane_normal_axis);
        }
        else
        {
            intersection_plane = gs_plane_from_pt_normal(op, ta);
        }
    } 

    // Get line intersection from ray and chosen intersection plane
    gs_plane_t* ip = &intersection_plane;
    float denom = gs_vec3_dot(gs_v3(ip->a, ip->b, ip->c), ray->d);
    if (fabsf(denom) >= GS_EPSILON)
    {
        float t =  -(ip->a * ray->p.x + ip->b * ray->p.y + ip->c * ray->p.z + ip->d) / denom;
        res.hit = t >= 0.f ? true : false;
        res.point = gs_vec3_add(ray->p, gs_vec3_scale(ray->d, t));
    } 

    return res;
} 

static gs_vec3 s_intersection_start = gs_default_val();
static gs_vqs s_delta = gs_default_val();
static bool just_set_focus = false;

GS_API_DECL int32_t gs_gui_gizmo(gs_gui_context_t* ctx, gs_camera_t* camera, gs_vqs* model, float snap, int32_t op, int32_t mode, int32_t opt)
{
    int32_t res = 0;
    if (model->rotation.w == 0.f) model->rotation = gs_quat_default();
    if (gs_vec3_len(model->scale) == 0.f) model->scale = gs_v3s(1.f);

	const gs_vec2 fbs = ctx->framebuffer_size;
	const float t = gs_platform_elapsed_time(); 
    const bool in_hover_root = gs_gui_in_hover_root(ctx); 

    gs_immediate_draw_t* dl = &ctx->overlay_draw_list;

    // This doesn't actually work for the clip...
	gs_gui_rect_t clip = gs_gui_layout_next(ctx);

    // Transform mouse into viewport space
    gs_vec2 mc = gs_platform_mouse_positionv();
    mc = gs_vec2_sub(mc, gs_v2(clip.x, clip.y));

    // Project ray to world
    const float ray_len = 1000.f;
    gs_vec3 ms = gs_v3(mc.x, mc.y, 0.f); 
    gs_vec3 me = gs_v3(mc.x, mc.y, -ray_len);
    gs_vec3 ro = gs_camera_screen_to_world(camera, ms, 0, 0, (int32_t)clip.w, (int32_t)clip.h);
    gs_vec3 rd = gs_camera_screen_to_world(camera, me, 0, 0, (int32_t)clip.w, (int32_t)clip.h); 
    rd = gs_vec3_norm(gs_vec3_sub(ro, rd));

    gs_ray_t ray = gs_default_val();
    ray.p = ro;
    ray.d = rd;
    ray.len = ray_len;

    // Check for nan
    if (gs_vec3_nan(ray.p)) ray.p = gs_v3s(0.f);
    if (gs_vec3_nan(ray.d)) ray.d = gs_v3s(0.f);

    gs_gizmo_desc_t desc = gs_default_val();
    desc.op = op;
    desc.mode = mode;
	desc.camera = *camera;
    desc.info.depth = FLT_MAX;
    desc.viewport = clip;

    desc.xform = gs_vqs_default();
    desc.xform.translation = model->translation;
    desc.xform.rotation = (mode == GS_GUI_TRANSFORM_LOCAL || op == GS_GUI_GIZMO_SCALE) ? model->rotation : gs_quat_default();       // This depends on the mode (local/world)

    switch (camera->proj_type)
    {
        case GS_PROJECTION_TYPE_ORTHOGRAPHIC:
        {
            desc.xform.scale = gs_v3s(camera->ortho_scale * 0.2f);
        } break;

        case GS_PROJECTION_TYPE_PERSPECTIVE:
        {
            float dist_from_cam = gs_vec3_dist(desc.xform.translation, camera->transform.translation);
            desc.xform.scale = gs_v3s(dist_from_cam * 0.3f);
        } break;
    }

    #define UPDATE_GIZMO_CONTROL(ID, RAY, SHAPE, MODEL, FUNC, CAP_SHAPE, CAP_MODEL, CAP_FUNC, INFO)\
        do {\
            int32_t mouseover = 0;\
            gs_gui_id id = (ID);\
            gs_contact_info_t info0 = gs_default_val();\
            gs_contact_info_t info1 = gs_default_val();\
            if (in_hover_root) {\
                FUNC(&(SHAPE), &(MODEL), &(RAY), NULL, &info0);\
                info0.depth = gs_vec3_dist(info0.point, (RAY).p);\
                CAP_FUNC(&(CAP_SHAPE), &(CAP_MODEL), &(RAY), NULL, &info1);\
                info1.depth = gs_vec3_dist(info1.point, (RAY).p);\
            }\
            gs_contact_info_t* info = info0.depth < info1.depth ? &info0 : &info1;\
            mouseover = info->hit && info->depth <= INFO.depth && in_hover_root && !ctx->hover_split && !ctx->lock_hover_id;\
            if (ctx->focus == id) {ctx->updated_focus = 1;}\
            if (~opt & GS_GUI_OPT_NOINTERACT) {\
                /* Check for hold focus here */\
                if (mouseover && !ctx->mouse_down) {\
                    gs_gui_set_hover(ctx, id);\
                    INFO = *info;\
                }\
\
                if (ctx->focus == id)\
                {\
                    res |= GS_GUI_RES_ACTIVE;\
                    just_set_focus = false;\
                    gs_gui_set_focus(ctx, id);\
                    if (ctx->mouse_pressed && !mouseover) {gs_gui_set_focus(ctx, 0);}\
                    if (!ctx->mouse_down && ~opt & GS_GUI_OPT_HOLDFOCUS) {gs_gui_set_focus(ctx, 0);}\
                }\
\
                if (ctx->prev_hover == id && !mouseover) {ctx->prev_hover = ctx->hover;}\
\
                if (ctx->hover == id)\
                {\
                    if (ctx->mouse_pressed)\
                    {\
                        if ((opt & GS_GUI_OPT_LEFTCLICKONLY && ctx->mouse_pressed == GS_GUI_MOUSE_LEFT) || (~opt & GS_GUI_OPT_LEFTCLICKONLY))\
                        {\
                            gs_gui_set_focus(ctx, id);\
                            just_set_focus = true;\
                        }\
                    }\
                    else if (!mouseover)\
                    {\
                        gs_gui_set_hover(ctx, 0);\
                    }\
                }\
            }\
        } while (0) 


	switch (op)
	{ 
		case GS_GUI_GIZMO_TRANSLATE:
		{ 
            // Construct translate gizmo for this frame based on given parent transform
			desc.gizmo.translate = gs_gizmo_translate(&desc.xform); 

            gs_gui_id id_r = gs_gui_get_id(ctx, "#gizmo_trans_right", strlen("#gizmo_trans_right"));
            gs_gui_id id_u = gs_gui_get_id(ctx, "#gizmo_trans_up", strlen("#gizmo_trans_up"));
            gs_gui_id id_f = gs_gui_get_id(ctx, "#gizmo_trans_forward", strlen("#gizmo_trans_forward")); 

            // Right 
            UPDATE_GIZMO_CONTROL(id_r, ray, 
                    desc.gizmo.translate.right.axis.shape.cylinder, desc.gizmo.translate.right.axis.model, gs_cylinder_vs_ray, 
                    desc.gizmo.translate.right.cap.shape.cone, desc.gizmo.translate.right.cap.model, gs_cone_vs_ray, 
                    desc.info);

            // Up
            UPDATE_GIZMO_CONTROL(id_u, ray, 
                    desc.gizmo.translate.up.axis.shape.cylinder, desc.gizmo.translate.up.axis.model, gs_cylinder_vs_ray, 
                    desc.gizmo.translate.up.cap.shape.cone, desc.gizmo.translate.up.cap.model, gs_cone_vs_ray, 
                    desc.info);

            // Forward 
            UPDATE_GIZMO_CONTROL(id_f, ray, 
                    desc.gizmo.translate.forward.axis.shape.cylinder, desc.gizmo.translate.forward.axis.model, gs_cylinder_vs_ray, 
                    desc.gizmo.translate.forward.cap.shape.cone, desc.gizmo.translate.forward.cap.model, gs_cone_vs_ray, 
                    desc.info); 
            
            // Control 
            if (ctx->focus == id_r)
            { 
                desc.li = gs_gui_gizmo_get_line_intersection(&desc.xform, GS_XAXIS, GS_YAXIS, GS_ZAXIS, 
                    camera, &ray, gs_v3s(0.f), true, false);

                if (just_set_focus) {
                    s_intersection_start = desc.li.point;
                    memset(&s_delta, 0, sizeof(s_delta));
                    s_delta.rotation = gs_quat(
                        model->translation.x, 
                        model->translation.y,
                        model->translation.z, 
                        0.f
                    );
                }

                if (desc.li.hit)
                {
                    gs_vec3 axis = gs_vec3_norm(gs_quat_rotate(desc.xform.rotation, GS_XAXIS)); 
                    gs_vec3 u = gs_vec3_sub(desc.li.point, s_intersection_start); 
                    float udotn = gs_vec3_dot(u, axis); 
                    s_delta.translation = gs_vec3_scale(axis, udotn); 
                    s_intersection_start = gs_vec3_add(s_intersection_start, s_delta.translation); 
                    if (gs_vec3_eq(axis, GS_XAXIS)) {
                        s_delta.translation.y = 0.f;
                        s_delta.translation.z = 0.f;
                    } 
                    if (snap > 0.f)
                    {
                        s_delta.scale = gs_vec3_add(s_delta.scale, s_delta.translation);     // Store total delta since interaction began 
                        float snap_len = round(gs_vec3_len(s_delta.scale) / snap) * snap;
                        gs_vec3 norm = gs_vec3_norm(s_delta.scale);
                        gs_vec3 delta = gs_vec3_scale(gs_vec3_norm(s_delta.scale), snap_len);
                        gs_vec3 op = gs_v3(s_delta.rotation.x, s_delta.rotation.y, s_delta.rotation.z);
                        model->translation = gs_vec3_add(op, delta);
                    }
                    else
                    {
                        // Set final translation
                        model->translation = gs_vec3_add(desc.xform.translation, s_delta.translation);
                    }

                } 
            }
            else if (ctx->focus == id_u)
            {
                desc.li = gs_gui_gizmo_get_line_intersection(&desc.xform, GS_YAXIS, GS_XAXIS, GS_ZAXIS, 
                    camera, &ray, gs_v3s(0.f), true, false);

                if (just_set_focus) {
                    s_intersection_start = desc.li.point;
                    memset(&s_delta, 0, sizeof(s_delta));
                    s_delta.rotation = gs_quat(
                        model->translation.x, 
                        model->translation.y,
                        model->translation.z, 
                        0.f
                    );
                }

                if (desc.li.hit)
                {
                    gs_vec3 axis = gs_vec3_norm(gs_quat_rotate(desc.xform.rotation, GS_YAXIS)); 
                    gs_vec3 u = gs_vec3_sub(desc.li.point, s_intersection_start); 
                    float udotn = gs_vec3_dot(u, axis); 
                    s_delta.translation = gs_vec3_scale(axis, udotn); 
                    s_intersection_start = gs_vec3_add(s_intersection_start, s_delta.translation); 
                    if (gs_vec3_eq(axis, GS_YAXIS)) {
                        s_delta.translation.x = 0.f;
                        s_delta.translation.z = 0.f;
                    } 
                    if (snap > 0.f)
                    {
                        s_delta.scale = gs_vec3_add(s_delta.scale, s_delta.translation);     // Store total delta since interaction began 
                        float snap_len = round(gs_vec3_len(s_delta.scale) / snap) * snap;
                        gs_vec3 norm = gs_vec3_norm(s_delta.scale);
                        gs_vec3 delta = gs_vec3_scale(gs_vec3_norm(s_delta.scale), snap_len);
                        gs_vec3 op = gs_v3(s_delta.rotation.x, s_delta.rotation.y, s_delta.rotation.z);
                        model->translation = gs_vec3_add(op, delta);
                    }
                    else
                    {
                        // Set final translation
                        model->translation = gs_vec3_add(desc.xform.translation, s_delta.translation);
                    }
                } 
            }
            else if (ctx->focus == id_f)
            {
                desc.li = gs_gui_gizmo_get_line_intersection(&desc.xform, GS_ZAXIS, GS_XAXIS, GS_YAXIS, 
                    camera, &ray, gs_v3s(0.f), true, false);

                if (just_set_focus) {
                    s_intersection_start = desc.li.point;
                    memset(&s_delta, 0, sizeof(s_delta));
                    s_delta.rotation = gs_quat(
                        model->translation.x, 
                        model->translation.y,
                        model->translation.z, 
                        0.f
                    );
                }

                if (desc.li.hit)
                {
                    gs_vec3 axis = gs_vec3_norm(gs_quat_rotate(desc.xform.rotation, GS_ZAXIS)); 
                    gs_vec3 u = gs_vec3_sub(desc.li.point, s_intersection_start); 
                    float udotn = gs_vec3_dot(u, axis); 
                    s_delta.translation = gs_vec3_scale(axis, udotn); 
                    s_intersection_start = gs_vec3_add(s_intersection_start, s_delta.translation); 
                    if (gs_vec3_eq(axis, GS_ZAXIS)) {
                        s_delta.translation.x = 0.f;
                        s_delta.translation.y = 0.f;
                    } 
                    if (snap > 0.f)
                    {
                        s_delta.scale = gs_vec3_add(s_delta.scale, s_delta.translation);     // Store total delta since interaction began 
                        float snap_len = round(gs_vec3_len(s_delta.scale) / snap) * snap;
                        gs_vec3 norm = gs_vec3_norm(s_delta.scale);
                        gs_vec3 delta = gs_vec3_scale(gs_vec3_norm(s_delta.scale), snap_len);
                        gs_vec3 op = gs_v3(s_delta.rotation.x, s_delta.rotation.y, s_delta.rotation.z);
                        model->translation = gs_vec3_add(op, delta);
                    }
                    else
                    {
                        // Set final translation
                        model->translation = gs_vec3_add(desc.xform.translation, s_delta.translation);
                    }
                } 
            } 

		} break;

		case GS_GUI_GIZMO_SCALE:
		{
            // Construct translate gizmo for this frame based on given parent transform
			desc.gizmo.scale = gs_gizmo_scale(&desc.xform); 

            gs_gui_id id_r = gs_gui_get_id(ctx, "#gizmo_scale_right", strlen("#gizmo_scale_right"));
            gs_gui_id id_u = gs_gui_get_id(ctx, "#gizmo_scale_up", strlen("#gizmo_scale_up"));
            gs_gui_id id_f = gs_gui_get_id(ctx, "#gizmo_scale_forward", strlen("#gizmo_scale_forward")); 

            // Right
            UPDATE_GIZMO_CONTROL(id_r, ray, 
                    desc.gizmo.scale.right.axis.shape.cylinder, desc.gizmo.scale.right.axis.model, gs_cylinder_vs_ray, 
                    desc.gizmo.scale.right.cap.shape.aabb, desc.gizmo.scale.right.cap.model, gs_aabb_vs_ray, 
                    desc.info);

            // Up
            UPDATE_GIZMO_CONTROL(id_u, ray, 
                    desc.gizmo.scale.up.axis.shape.cylinder, desc.gizmo.scale.up.axis.model, gs_cylinder_vs_ray, 
                    desc.gizmo.scale.up.cap.shape.aabb, desc.gizmo.scale.up.cap.model, gs_aabb_vs_ray, 
                    desc.info);

            // Forward 
            UPDATE_GIZMO_CONTROL(id_f, ray, 
                    desc.gizmo.scale.forward.axis.shape.cylinder, desc.gizmo.scale.forward.axis.model, gs_cylinder_vs_ray, 
                    desc.gizmo.scale.forward.cap.shape.aabb, desc.gizmo.scale.forward.cap.model, gs_aabb_vs_ray, 
                    desc.info);

            // Control 
            if (ctx->focus == id_r)
            { 
                desc.li = gs_gui_gizmo_get_line_intersection(&desc.xform, GS_XAXIS, GS_YAXIS, GS_ZAXIS, 
                    camera, &ray, gs_v3s(0.f), true, false);

                if (desc.li.hit)
                {
                    if (just_set_focus) {
                        s_intersection_start = desc.li.point;
                        memset(&s_delta, 0, sizeof(s_delta));
                        s_delta.rotation = gs_quat(
                            model->scale.x, 
                            model->scale.y, 
                            model->scale.z, 
                            0.f
                        );
                    } 

                    gs_vec3 axis = gs_vec3_norm(gs_quat_rotate(desc.xform.rotation, GS_XAXIS));
                    gs_vec3 u = gs_vec3_sub(desc.li.point, s_intersection_start); 
                    float udotn = gs_vec3_dot(u, axis); 
                    float neg = gs_vec3_dot(axis, GS_XAXIS) < 0.f ? 1.f : -1.f;
                    s_delta.translation = gs_vec3_scale(axis, udotn);
                    s_intersection_start = gs_vec3_add(s_intersection_start, s_delta.translation);
                    s_delta.translation = gs_vec3_scale(s_delta.translation, neg); 
                    s_delta.translation.z = 0.f;
                    s_delta.translation.y = 0.f; 
                    if (snap > 0.f)
                    {
                        s_delta.scale = gs_vec3_add(s_delta.scale, s_delta.translation);     // Store total delta since interaction began 
                        float snap_len = round(gs_vec3_len(s_delta.scale) / snap) * snap;
                        gs_vec3 norm = gs_vec3_norm(s_delta.scale);
                        gs_vec3 delta = gs_vec3_scale(gs_vec3_norm(s_delta.scale), snap_len);
                        gs_vec3 os = gs_v3(s_delta.rotation.x, s_delta.rotation.y, s_delta.rotation.z);
                        model->scale = gs_vec3_add(os, delta);
                    }
                    else
                    {
                        model->scale = gs_vec3_add(model->scale, s_delta.translation);
                    }
                } 
            }
            else if (ctx->focus == id_u)
            {
                desc.li = gs_gui_gizmo_get_line_intersection(&desc.xform, GS_YAXIS, GS_XAXIS, GS_ZAXIS, 
                    camera, &ray, gs_v3s(0.f), true, false);

                if (desc.li.hit)
                {
                    if (just_set_focus) {
                        s_intersection_start = desc.li.point;
                        memset(&s_delta, 0, sizeof(s_delta));
                        s_delta.rotation = gs_quat(
                            model->scale.x, 
                            model->scale.y, 
                            model->scale.z, 
                            0.f
                        );
                    } 

                    gs_vec3 axis = gs_vec3_norm(gs_quat_rotate(desc.xform.rotation, GS_YAXIS));
                    gs_vec3 u = gs_vec3_sub(desc.li.point, s_intersection_start); 
                    float udotn = gs_vec3_dot(u, axis); 
                    s_delta.translation = gs_vec3_scale(axis, udotn);
                    float neg = gs_vec3_dot(axis, GS_YAXIS) < 0.f ? -1.f : 1.f;
                    s_intersection_start = gs_vec3_add(s_intersection_start, s_delta.translation);
                    s_delta.translation = gs_vec3_scale(s_delta.translation, neg); 
                    s_delta.translation.z = 0.f;
                    s_delta.translation.x = 0.f;
                    if (snap > 0.f)
                    {
                        s_delta.scale = gs_vec3_add(s_delta.scale, s_delta.translation);     // Store total delta since interaction began 
                        float snap_len = round(gs_vec3_len(s_delta.scale) / snap) * snap;
                        gs_vec3 norm = gs_vec3_norm(s_delta.scale);
                        gs_vec3 delta = gs_vec3_scale(gs_vec3_norm(s_delta.scale), snap_len);
                        gs_vec3 os = gs_v3(s_delta.rotation.x, s_delta.rotation.y, s_delta.rotation.z);
                        model->scale = gs_vec3_add(os, delta);
                    }
                    else
                    {
                        model->scale = gs_vec3_add(model->scale, s_delta.translation);
                    }
                } 
            }
            else if (ctx->focus == id_f)
            {
                desc.li = gs_gui_gizmo_get_line_intersection(&desc.xform, GS_ZAXIS, GS_XAXIS, GS_YAXIS, 
                    camera, &ray, gs_v3s(0.f), true, false);

                if (desc.li.hit)
                {
                    if (just_set_focus) {
                        s_intersection_start = desc.li.point;
                        memset(&s_delta, 0, sizeof(s_delta));
                        s_delta.rotation = gs_quat(
                            model->scale.x, 
                            model->scale.y, 
                            model->scale.z, 
                            0.f
                        );
                    } 

                    gs_vec3 axis = gs_vec3_norm(gs_quat_rotate(desc.xform.rotation, GS_ZAXIS));
                    gs_vec3 u = gs_vec3_sub(desc.li.point, s_intersection_start); 
                    float udotn = gs_vec3_dot(u, axis); 
                    float neg = gs_vec3_dot(axis, GS_ZAXIS) < 0.f ? -1.f : 1.f;
                    s_delta.translation = gs_vec3_scale(axis, udotn);
                    s_intersection_start = gs_vec3_add(s_intersection_start, s_delta.translation);
                    s_delta.translation = gs_vec3_scale(s_delta.translation, neg); 
                    s_delta.translation.y = 0.f;
                    s_delta.translation.x = 0.f;
                    if (snap > 0.f)
                    {
                        s_delta.scale = gs_vec3_add(s_delta.scale, s_delta.translation);     // Store total delta since interaction began 
                        float snap_len = round(gs_vec3_len(s_delta.scale) / snap) * snap;
                        gs_vec3 norm = gs_vec3_norm(s_delta.scale);
                        gs_vec3 delta = gs_vec3_scale(gs_vec3_norm(s_delta.scale), snap_len);
                        gs_vec3 os = gs_v3(s_delta.rotation.x, s_delta.rotation.y, s_delta.rotation.z);
                        model->scale = gs_vec3_add(os, delta);
                    }
                    else
                    {
                        model->scale = gs_vec3_add(model->scale, s_delta.translation);
                    }
                }
            } 

		} break; 

    #define UPDATE_GIZMO_CONTROL_ROTATE(ID, RAY, SHAPE, MODEL, AXIS, INFO)\
        do {\
            int32_t mouseover = 0;\
            gs_gui_id id = (ID);\
            gs_contact_info_t info = gs_default_val();\
            gs_vec3 axis = gs_quat_rotate(desc.xform.rotation, AXIS);\
            info.normal = axis;\
            if (in_hover_root) {\
                gs_plane_t ip = gs_plane_from_pt_normal(desc.xform.translation, axis);\
                float denom = gs_vec3_dot(gs_v3(ip.a, ip.b, ip.c), ray.d);\
                denom =  fabsf(denom) >= GS_EPSILON ? denom : 0.00001f;\
                info.depth = -(ip.a * ray.p.x + ip.b * ray.p.y + ip.c * ray.p.z + ip.d) / denom;\
                gs_gui_gizmo_line_intersection_result_t res = gs_default_val();\
                res.point = gs_vec3_add(ray.p, gs_vec3_scale(ray.d, info.depth));\
                float dist = gs_vec3_dist(res.point, model->translation);\
                float scl = gs_vec3_len(desc.xform.scale);\
                if (dist <= 0.6f * scl && dist >= 0.45f * scl) {\
                    info.hit = true;\
                }\
            }\
            mouseover = info.hit && info.depth <= INFO.depth && in_hover_root && !ctx->hover_split && !ctx->lock_hover_id;\
            if (ctx->focus == id) {ctx->updated_focus = 1; INFO = info;}\
            if (~opt & GS_GUI_OPT_NOINTERACT) {\
                /* Check for hold focus here */\
                if (mouseover && !ctx->mouse_down) {\
                    gs_gui_set_hover(ctx, id);\
                    INFO = info;\
                }\
\
                if (ctx->focus == id)\
                {\
                    res |= GS_GUI_RES_ACTIVE;\
                    just_set_focus = false;\
                    gs_gui_set_focus(ctx, id);\
                    if (ctx->mouse_pressed && !mouseover) {gs_gui_set_focus(ctx, 0);}\
                    if (!ctx->mouse_down && ~opt & GS_GUI_OPT_HOLDFOCUS) {gs_gui_set_focus(ctx, 0);}\
                }\
\
                if (ctx->prev_hover == id && !mouseover) {ctx->prev_hover = ctx->hover;}\
\
                if (ctx->hover == id)\
                {\
                    if (ctx->mouse_pressed)\
                    {\
                        if ((opt & GS_GUI_OPT_LEFTCLICKONLY && ctx->mouse_pressed == GS_GUI_MOUSE_LEFT) || (~opt & GS_GUI_OPT_LEFTCLICKONLY))\
                        {\
                            gs_gui_set_focus(ctx, id);\
                            just_set_focus = true;\
                        }\
                    }\
                    else if (!mouseover)\
                    {\
                        gs_gui_set_hover(ctx, 0);\
                    }\
                }\
            }\
        } while (0) 

		case GS_GUI_GIZMO_ROTATE:
		{
            // Construct translate gizmo for this frame based on given parent transform
			desc.gizmo.rotate = gs_gizmo_rotate(&desc.xform); 

            gs_gui_id id_r = gs_gui_get_id(ctx, "#gizmo_rotate_right", strlen("#gizmo_rotate_right"));
            gs_gui_id id_u = gs_gui_get_id(ctx, "#gizmo_rotate_up", strlen("#gizmo_rotate_up"));
            gs_gui_id id_f = gs_gui_get_id(ctx, "#gizmo_rotate_forward", strlen("#gizmo_rotate_forward")); 

            // Right
            UPDATE_GIZMO_CONTROL_ROTATE(id_r, ray, desc.gizmo.rotate.right.axis.shape.plane, 
                    desc.gizmo.rotate.right.axis.model, GS_XAXIS, desc.info); 

            // Up
            UPDATE_GIZMO_CONTROL_ROTATE(id_u, ray, desc.gizmo.rotate.up.axis.shape.plane, 
                    desc.gizmo.rotate.up.axis.model, GS_YAXIS, desc.info); 

            // Forward
            UPDATE_GIZMO_CONTROL_ROTATE(id_f, ray, desc.gizmo.rotate.forward.axis.shape.plane, 
                    desc.gizmo.rotate.forward.axis.model, GS_ZAXIS, desc.info);

            if (ctx->focus == id_r)
            {
                desc.li = op == GS_GUI_TRANSFORM_LOCAL ? 
                    gs_gui_gizmo_get_line_intersection(&desc.xform, GS_XAXIS, GS_YAXIS, GS_ZAXIS, camera, &ray, gs_v3s(0.f), false, false) : 
                    gs_gui_gizmo_get_line_intersection(&desc.xform, GS_XAXIS, GS_YAXIS, GS_ZAXIS, camera, &ray, gs_v3s(0.f), false, true); 


                if (desc.li.hit)
                {
                    if (just_set_focus) {
                        s_intersection_start = desc.li.point;
                        memset(&s_delta, 0, sizeof(s_delta));
                        s_delta.translation = gs_v3(
                            model->rotation.x, 
                            model->rotation.y, 
                            model->rotation.z
                        );
                        s_delta.scale.y = model->rotation.w;
                    } 

                    float dist_from_cam = gs_vec3_dist(desc.xform.translation, camera->transform.translation); 
                    const float denom = dist_from_cam != 0.f ? dist_from_cam : 2.f;
                    const gs_vec3 end_vector = gs_vec3_sub(desc.li.point, desc.xform.translation);
                    const gs_vec3 start_norm = gs_vec3_norm(gs_vec3_sub(s_intersection_start, desc.xform.translation));
                    const gs_vec3 end_norm = gs_vec3_norm(end_vector);
                    const gs_vec3 rot_local = gs_quat_rotate(desc.xform.rotation, GS_XAXIS); 

                    float len = gs_vec3_len(end_vector) / denom;
                    float angle = gs_vec3_angle_between_signed(start_norm, end_norm); 

                    if (len > 1.f) {
                        angle *= len;
                    } 

                    gs_vec3 cross = gs_vec3_cross(start_norm, end_norm);
                    if (gs_vec3_dot(rot_local, cross) < 0.f) {
                        angle *= -1.f;
                    }

                    s_intersection_start = desc.li.point;
                    float delta = gs_rad2deg(angle); 
                    s_delta.scale.x += delta;
                    s_delta.rotation = gs_quat_angle_axis(gs_deg2rad(delta), GS_XAXIS);

                    if (snap > 0.f)
                    {
                        float snap_delta = round(s_delta.scale.x / snap) * snap;
                        s_delta.rotation = gs_quat_angle_axis(gs_deg2rad(snap_delta), GS_XAXIS);
                        gs_quat orot = gs_quat(s_delta.translation.x, s_delta.translation.y, s_delta.translation.z, s_delta.scale.y);
                        switch (mode) {
                            case GS_GUI_TRANSFORM_WORLD: model->rotation = gs_quat_mul(s_delta.rotation, orot); break;
                            case GS_GUI_TRANSFORM_LOCAL: model->rotation = gs_quat_mul(orot, s_delta.rotation); break;
                        }
                    }
                    else
                    {
                        switch (mode) {
                            case GS_GUI_TRANSFORM_WORLD: model->rotation = gs_quat_mul(s_delta.rotation, model->rotation); break;
                            case GS_GUI_TRANSFORM_LOCAL: model->rotation = gs_quat_mul(model->rotation, s_delta.rotation); break;
                        }
                    }

                }
            } 
            else if (ctx->focus == id_u)
            {
                desc.li = op == GS_GUI_TRANSFORM_LOCAL ? 
                    gs_gui_gizmo_get_line_intersection(&desc.xform, GS_YAXIS, GS_XAXIS, GS_ZAXIS, camera, &ray, gs_v3s(0.f), false, false) : 
                    gs_gui_gizmo_get_line_intersection(&desc.xform, GS_YAXIS, GS_XAXIS, GS_ZAXIS, camera, &ray, gs_v3s(0.f), false, true); 

                if (desc.li.hit)
                {
                    if (just_set_focus) {
                        s_intersection_start = desc.li.point;
                        memset(&s_delta, 0, sizeof(s_delta)); 
                        s_delta.translation = gs_v3(
                            model->rotation.x, 
                            model->rotation.y, 
                            model->rotation.z
                        );
                        s_delta.scale.y = model->rotation.w;
                    } 

                    float dist_from_cam = gs_vec3_dist(desc.xform.translation, camera->transform.translation); 
                    const float denom = dist_from_cam != 0.f ? dist_from_cam : 2.f;
                    const gs_vec3 end_vector = gs_vec3_sub(desc.li.point, desc.xform.translation);
                    const gs_vec3 start_norm = gs_vec3_norm(gs_vec3_sub(s_intersection_start, desc.xform.translation));
                    const gs_vec3 end_norm = gs_vec3_norm(end_vector);
                    const gs_vec3 rot_local = gs_quat_rotate(desc.xform.rotation, GS_YAXIS); 

                    float len = gs_vec3_len(end_vector) / denom;
                    float angle = gs_vec3_angle_between_signed(start_norm, end_norm); 

                    if (len > 1.f) {
                        angle *= len;
                    } 

                    gs_vec3 cross = gs_vec3_cross(start_norm, end_norm);
                    if (gs_vec3_dot(rot_local, cross) < 0.f) {
                        angle *= -1.f;
                    }

                    s_intersection_start = desc.li.point;
                    float delta = gs_rad2deg(angle); 
                    s_delta.scale.x += delta;
                    s_delta.rotation = gs_quat_angle_axis(gs_deg2rad(delta), GS_YAXIS);

                    if (snap > 0.f)
                    {
                        float snap_delta = round(s_delta.scale.x / snap) * snap;
                        s_delta.rotation = gs_quat_angle_axis(gs_deg2rad(snap_delta), GS_YAXIS);
                        gs_quat orot = gs_quat(s_delta.translation.x, s_delta.translation.y, s_delta.translation.z, s_delta.scale.y);
                        switch (mode) {
                            case GS_GUI_TRANSFORM_WORLD: model->rotation = gs_quat_mul(s_delta.rotation, orot); break;
                            case GS_GUI_TRANSFORM_LOCAL: model->rotation = gs_quat_mul(orot, s_delta.rotation); break;
                        }
                    }
                    else
                    {
                        switch (mode) {
                            case GS_GUI_TRANSFORM_WORLD: model->rotation = gs_quat_mul(s_delta.rotation, model->rotation); break;
                            case GS_GUI_TRANSFORM_LOCAL: model->rotation = gs_quat_mul(model->rotation, s_delta.rotation); break;
                        }
                    }
                }
            } 
            else if (ctx->focus == id_f)
            {
                desc.li = op == GS_GUI_TRANSFORM_LOCAL ? 
                    gs_gui_gizmo_get_line_intersection(&desc.xform, GS_ZAXIS, GS_XAXIS, GS_YAXIS, camera, &ray, gs_v3s(0.f), false, false) : 
                    gs_gui_gizmo_get_line_intersection(&desc.xform, GS_ZAXIS, GS_XAXIS, GS_YAXIS, camera, &ray, gs_v3s(0.f), false, true); 

                if (desc.li.hit)
                {
                    if (just_set_focus) {
                        s_intersection_start = desc.li.point;
                        memset(&s_delta, 0, sizeof(s_delta));
                        s_delta.translation = gs_v3(
                            model->rotation.x, 
                            model->rotation.y, 
                            model->rotation.z
                        );
                        s_delta.scale.y = model->rotation.w;
                    } 

                    float dist_from_cam = gs_vec3_dist(desc.xform.translation, camera->transform.translation); 
                    const float denom = dist_from_cam != 0.f ? dist_from_cam : 2.f;
                    const gs_vec3 end_vector = gs_vec3_sub(desc.li.point, desc.xform.translation);
                    const gs_vec3 start_norm = gs_vec3_norm(gs_vec3_sub(s_intersection_start, desc.xform.translation));
                    const gs_vec3 end_norm = gs_vec3_norm(end_vector);
                    const gs_vec3 rot_local = gs_quat_rotate(desc.xform.rotation, GS_ZAXIS); 

                    float len = gs_vec3_len(end_vector) / denom;
                    float angle = gs_vec3_angle_between_signed(start_norm, end_norm); 

                    if (len > 1.f) {
                        angle *= len;
                    } 

                    gs_vec3 cross = gs_vec3_cross(start_norm, end_norm);
                    if (gs_vec3_dot(rot_local, cross) < 0.f) {
                        angle *= -1.f;
                    }

                    s_intersection_start = desc.li.point;
                    float delta = gs_rad2deg(angle); 
                    s_delta.scale.x += delta;
                    s_delta.rotation = gs_quat_angle_axis(gs_deg2rad(delta), GS_ZAXIS);

                    if (snap > 0.f)
                    {
                        float snap_delta = round(s_delta.scale.x / snap) * snap;
                        s_delta.rotation = gs_quat_angle_axis(gs_deg2rad(snap_delta), GS_ZAXIS);
                        gs_quat orot = gs_quat(s_delta.translation.x, s_delta.translation.y, s_delta.translation.z, s_delta.scale.y);
                        switch (mode) {
                            case GS_GUI_TRANSFORM_WORLD: model->rotation = gs_quat_mul(s_delta.rotation, orot); break;
                            case GS_GUI_TRANSFORM_LOCAL: model->rotation = gs_quat_mul(orot, s_delta.rotation); break;
                        }
                    }
                    else
                    {
                        switch (mode) {
                            case GS_GUI_TRANSFORM_WORLD: model->rotation = gs_quat_mul(s_delta.rotation, model->rotation); break;
                            case GS_GUI_TRANSFORM_LOCAL: model->rotation = gs_quat_mul(model->rotation, s_delta.rotation); break;
                        }
                    }
                }
            } 

		} break;
	} 

    // Have to render the gizmo using view/projection (so a custom render command) 
    gs_gui_draw_custom(ctx, clip, gs_gui_gizmo_render, &desc, sizeof(desc));

    return res;
}


//=== Demos ===//

GS_API_DECL int32_t gs_gui_style_editor(gs_gui_context_t *ctx, gs_gui_style_sheet_t* style_sheet, gs_gui_rect_t rect, bool* open) 
{
    if (!style_sheet)
    {
        style_sheet = &gs_gui_default_style_sheet;
    }

    static struct {const char* label; int32_t idx;} elements[] = {
        {"container",  GS_GUI_ELEMENT_CONTAINER},
        {"button",  GS_GUI_ELEMENT_BUTTON} ,
        {"panel",  GS_GUI_ELEMENT_PANEL},
        {"input",  GS_GUI_ELEMENT_INPUT},
        {"label",  GS_GUI_ELEMENT_LABEL},
        {"text",  GS_GUI_ELEMENT_TEXT},
        {"scroll",  GS_GUI_ELEMENT_SCROLL},
        {"image",  GS_GUI_ELEMENT_IMAGE},
        {NULL}
    }; 

    static char* states[] = {
      "default", 
      "hover", 
      "focus"
    };

    static struct {const char* label; int32_t idx;} colors[] = {
        {"background",  GS_GUI_COLOR_BACKGROUND},
        {"border",  GS_GUI_COLOR_BORDER},
        {"shadow",  GS_GUI_COLOR_SHADOW},
        {"content",  GS_GUI_COLOR_CONTENT} ,
        {"content_shadow",  GS_GUI_COLOR_CONTENT_SHADOW},
        {"content_background",  GS_GUI_COLOR_CONTENT_BACKGROUND},
        {"content_border",  GS_GUI_COLOR_CONTENT_BORDER},
        {NULL}
    }; 

  if (gs_gui_window_begin_ex(ctx, "Style_Editor", rect, open, NULL, 0x00)) 
  { 
    for (uint32_t i = 0; elements[i].label; ++i)
    {
        int32_t idx = elements[i].idx; 

        if (gs_gui_treenode_begin_ex(ctx, elements[i].label, NULL, 0x00))
        {
            for (uint32_t j = 0; j < GS_GUI_ELEMENT_STATE_COUNT; ++j)
            {
                gs_gui_push_id(ctx, &j, sizeof(j));
                gs_gui_style_t* s = &style_sheet->styles[idx][j];
                if (gs_gui_treenode_begin_ex(ctx, states[j], NULL, 0x00))
                {
                    gs_gui_style_t* save = gs_gui_push_style(ctx, &ctx->style_sheet->styles[GS_GUI_ELEMENT_PANEL][0x00]);
                    int32_t row[] = {-1};
                    gs_gui_layout_row(ctx, 1, row, 300);
                    gs_gui_panel_begin(ctx, states[j]);
                    {
                        gs_gui_layout_t* l = gs_gui_get_layout(ctx);
                        gs_gui_rect_t* r = &l->body; 

                        const int32_t ls = 80;

                        // size
                        int32_t w = (int32_t)((l->body.w - ls) * 0.35f);
                        {
                            int32_t row[] = {ls, w, w};
                            gs_gui_layout_row(ctx, 3, row, 0); 
                        }

                        gs_gui_label(ctx, "size:");
                        gs_gui_slider(ctx, &s->size[0], 0.f, 500.f);
                        gs_gui_slider(ctx, &s->size[1], 0.f, 500.f); 

                        w = (int32_t)((l->body.w - ls) * 0.2f); 

                        {
                            int32_t row[] = {ls, w, w, w, w};
                            gs_gui_layout_row(ctx, 5, row, 0); 
                        }

                        gs_gui_label(ctx, "border_width:");
                        int16_slider(ctx, &s->border_width[0], 0, 100, NULL, GS_GUI_OPT_ALIGNCENTER); 
                        int16_slider(ctx, &s->border_width[1], 0, 100, NULL, GS_GUI_OPT_ALIGNCENTER); 
                        int16_slider(ctx, &s->border_width[2], 0, 100, NULL, GS_GUI_OPT_ALIGNCENTER); 
                        int16_slider(ctx, &s->border_width[3], 0, 100, NULL, GS_GUI_OPT_ALIGNCENTER); 

                        gs_gui_label(ctx, "border_radius:");
                        int16_slider(ctx, &s->border_radius[0], 0, 100, NULL, GS_GUI_OPT_ALIGNCENTER); 
                        int16_slider(ctx, &s->border_radius[1], 0, 100, NULL, GS_GUI_OPT_ALIGNCENTER); 
                        int16_slider(ctx, &s->border_radius[2], 0, 100, NULL, GS_GUI_OPT_ALIGNCENTER); 
                        int16_slider(ctx, &s->border_radius[3], 0, 100, NULL, GS_GUI_OPT_ALIGNCENTER); 

                        // padding/margin
                        gs_gui_label(ctx, "padding:");
                        int32_slider(ctx, &s->padding[0], 0, 100, NULL, GS_GUI_OPT_ALIGNCENTER); 
                        int32_slider(ctx, &s->padding[1], 0, 100, NULL, GS_GUI_OPT_ALIGNCENTER); 
                        int32_slider(ctx, &s->padding[2], 0, 100, NULL, GS_GUI_OPT_ALIGNCENTER); 
                        int32_slider(ctx, &s->padding[3], 0, 100, NULL, GS_GUI_OPT_ALIGNCENTER); 

                        gs_gui_label(ctx, "margin:");
                        int16_slider(ctx, &s->margin[0], 0, 100, NULL, GS_GUI_OPT_ALIGNCENTER); 
                        int16_slider(ctx, &s->margin[1], 0, 100, NULL, GS_GUI_OPT_ALIGNCENTER); 
                        int16_slider(ctx, &s->margin[2], 0, 100, NULL, GS_GUI_OPT_ALIGNCENTER); 
                        int16_slider(ctx, &s->margin[3], 0, 100, NULL, GS_GUI_OPT_ALIGNCENTER); 

                        // Colors
                        int sw = (int32_t)(l->body.w * 0.14);
                        {
                            int32_t row[] = {80, sw, sw, sw, sw, -1};
                            gs_gui_layout_row(ctx, 6, row, 0);
                        }

                        for (uint32_t c = 0; colors[c].label; ++c)
                        {
                            gs_gui_label(ctx, colors[c].label);
                            uint8_slider(ctx, &s->colors[c].r, 0, 255, NULL, GS_GUI_OPT_ALIGNCENTER);
                            uint8_slider(ctx, &s->colors[c].g, 0, 255, NULL, GS_GUI_OPT_ALIGNCENTER);
                            uint8_slider(ctx, &s->colors[c].b, 0, 255, NULL, GS_GUI_OPT_ALIGNCENTER);
                            uint8_slider(ctx, &s->colors[c].a, 0, 255, NULL, GS_GUI_OPT_ALIGNCENTER);
                            gs_gui_draw_rect(ctx, gs_gui_layout_next(ctx), s->colors[c]);
                        }
                    }
                    gs_gui_panel_end(ctx); 
                    gs_gui_pop_style(ctx, save);

                    gs_gui_treenode_end(ctx);
                }
                gs_gui_pop_id(ctx);
            }
            gs_gui_treenode_end(ctx);
        }
    } 
    gs_gui_window_end(ctx);
  }

  return 0x01;
} 

GS_API_DECL int32_t gs_gui_demo_window(gs_gui_context_t* ctx, gs_gui_rect_t rect, bool* open) 
{
    /* 
       Window for all of the available features for gs_gui: 

        - Opts: 

        - Widgets: 
            - Label
            - Panel
            - Button
            - Textbox
            - Text 
            Todo: 
                - Menu / MenuItem
                - ColorPicker 
                - Lists (with images / various style enums for item types (circle, rect, box)

        - Window Navigation
            - VIM controls eventually?

        - Docking

        - Menus

        - Tabs 

        - Styles: 
            - Inline styles
            - Style sheets 
    */

    if (gs_gui_window_begin_ex(ctx, "Demo_Window", rect, open, NULL, 0x00)) 
    { 
        gs_gui_container_t* win = gs_gui_get_current_container(ctx);

        if (gs_gui_treenode_begin(ctx, "Help"))
        {
            {
                int32_t row[] = {-10};
                gs_gui_layout_row(ctx, 1, row, 170);
            }
            
            gs_gui_panel_begin(ctx, "#!window_info");
            {
                {
                    int32_t row[] = {-1};
                    gs_gui_layout_row(ctx, 1, row, 0);
                }
                gs_gui_label(ctx, "ABOUT THIS DEMO:");
                gs_gui_text(ctx, "  - Sections below are demonstrating many aspects of the util."); 
            }
            gs_gui_panel_end(ctx);
            gs_gui_treenode_end(ctx);
        }

        if (gs_gui_treenode_begin(ctx, "Window Info"))
        {
            {
                int32_t row[] = {-10};
                gs_gui_layout_row(ctx, 1, row, 170);
            }
            gs_gui_panel_begin(ctx, "#!window_info");
            {
                char buf[64];
                {
                    int32_t row[] = {65, -1};
                    gs_gui_layout_row(ctx, 2, row, 0);
                }

                gs_gui_label(ctx,"Position:");
                gs_snprintf(buf, 64, "%.2f, %.2f", win->rect.x, win->rect.y); gs_gui_label(ctx, buf);

                gs_gui_label(ctx, "Size:");
                gs_snprintf(buf, 64, "%.2f, %.2f", win->rect.w, win->rect.h); gs_gui_label(ctx, buf);

                gs_gui_label(ctx, "Title:");
                gs_gui_label(ctx, win->name);

                gs_gui_label(ctx, "ID:");
                gs_snprintf(buf, 64, "%zu", win->id); gs_gui_label(ctx, buf);

                gs_gui_label(ctx, "Open:");
                gs_snprintf(buf, 64, "%s", win->open ? "true" : "close"); gs_gui_label(ctx, buf);
            }
            gs_gui_panel_end(ctx);

            gs_gui_treenode_end(ctx);
        }

        if (gs_gui_treenode_begin(ctx, "Context State"))
        {
            {
                int32_t row[] = {-10};
                gs_gui_layout_row(ctx, 1, row, 170);
            }
            gs_gui_panel_begin(ctx, "#!context_state");
            {
                char buf[64];
                {
                    int32_t row[] = {80, -1};
                    gs_gui_layout_row(ctx, 2, row, 0);
                }

                gs_gui_label(ctx,"Hovered:");
                gs_snprintf(buf, 64, "%s", ctx->hover_root ? ctx->hover_root->name : "NULL"); gs_gui_label(ctx, buf);

                gs_gui_label(ctx, "Focused:");
                gs_snprintf(buf, 64, "%s", ctx->focus_root ? ctx->focus_root->name : "NULL"); gs_gui_label(ctx, buf);

                gs_gui_label(ctx, "Active:");
                gs_snprintf(buf, 64, "%s", ctx->active_root ? ctx->active_root->name : "NULL"); gs_gui_label(ctx, buf);

                gs_gui_label(ctx, "Lock Focus:");
                gs_snprintf(buf, 64, "%zu", ctx->lock_focus); gs_gui_label(ctx, buf);
            }
            gs_gui_panel_end(ctx);

            gs_gui_treenode_end(ctx);
        } 

        if (gs_gui_treenode_begin(ctx, "Widgets"))
        {
            {
                int32_t row[] = {-10};
                gs_gui_layout_row(ctx, 1, row, 170);
            }
            gs_gui_panel_begin(ctx, "#!widgets");
            {
                {
                    int32_t row[] = {150, 50};
                    gs_gui_layout_row(ctx, 2, row, 0);
                }
                gs_gui_layout_column_begin(ctx);
                {
                    {
                        int32_t row[] = {0};
                        gs_gui_layout_row(ctx, 1, row, 0);
                    }
                    gs_gui_button(ctx, "Button"); 

                    // Label
                    gs_gui_label(ctx, "Label");

                    // Text
                    {
                        int32_t row[] = {150};
                        gs_gui_layout_row(ctx, 1, row, 0);
                    }
                    gs_gui_text(ctx, "This is some text");

                    static char buf[64] = {0}; 
                    gs_gui_textbox(ctx, buf, 64);                
                }
                gs_gui_layout_column_end(ctx);

                gs_gui_layout_column_begin(ctx);
                {
                    gs_gui_label(ctx, "(?)");
                    if (ctx->hover == ctx->last_id) gs_println("HOVERED");
                }
                gs_gui_layout_column_end(ctx); 
            }
            gs_gui_panel_end(ctx); 
            gs_gui_treenode_end(ctx);
        }

        gs_gui_window_end(ctx); 
    }
    return 0x01;
}

//==== Resource Loading ===// 

typedef enum 
{
    GS_GUI_SS_DEF_NUMBER = 0x00, 
    GS_GUI_SS_DEF_ENUM, 
    GS_GUI_SS_DEF_COLOR,
    GS_GUI_SS_DEF_STR
} gs_gui_ss_var_def_type;

typedef struct
{
    gs_gui_ss_var_def_type type;
    union {
        int32_t number;
        gs_color_t color;
        char str[64]; 
    } val;
} gs_gui_ss_var_def_t;

typedef struct
{
	gs_hash_table(uint64_t, gs_gui_ss_var_def_t) variables;
} gs_gui_ss_variables_t;

#define _GS_GUI_SS_GET_TO_VALUES(T0, T1)\
    do {\
        bool ret = gs_lexer_require_token_type(lex, T0);\
        ret &= gs_lexer_require_token_type(lex, T1);\
        if (!ret) {\
            gs_log_warning("Unidentified token.");\
            return false;\
        }\
        token = gs_lexer_current_token(lex);\
    } while (0) 

bool _gs_gui_style_sheet_parse_attribute_transition(gs_gui_context_t* ctx, gs_lexer_t* lex, 
	gs_gui_style_sheet_t* ss, const uint64_t id_tag, int32_t elementid, int32_t state, gs_gui_ss_variables_t* variables)
{
    // Name of enum attribute 
    gs_token_t token = gs_lexer_current_token(lex);
    // gs_token_debug_print(&token); 

    if (id_tag) 
    { 
        if (!gs_hash_table_exists(ss->cid_animations, id_tag))
        {
            gs_gui_animation_property_list_t sl = gs_default_val();
            gs_hash_table_insert(ss->cid_animations, id_tag, sl);
        }
    }

#define PARSE_TRANSITION(T)\
    do {\
        uint32_t time_v = 0;\
        uint32_t delay_v = 0;\
        bool ret = gs_lexer_require_token_type(lex, GS_TOKEN_COLON);\
        ret &= (gs_lexer_require_token_type(lex, GS_TOKEN_DOLLAR) || gs_lexer_require_token_type(lex, GS_TOKEN_NUMBER));\
        if (!ret) {\
            gs_log_warning("Transition: Unidentified token.");\
            return false;\
        }\
        gs_token_t time = gs_lexer_current_token(lex);\
        switch (time.type)\
        { \
            case GS_TOKEN_NUMBER:\
            {\
                gs_snprintfc(TIME, 32, "%.*s", time.len, time.text);\
                time_v = (uint32_t)atoi(TIME);\
            } break;\
\
            case GS_TOKEN_DOLLAR:\
            {\
                ret &= (gs_lexer_require_token_type(lex, GS_TOKEN_IDENTIFIER));\
                if (!ret) {\
                    gs_log_warning("Transition: Variable missing identifier token.");\
                    return false;\
                }\
                token = gs_lexer_current_token(lex);\
                gs_snprintfc(VAR, 64, "%.*s", token.len, token.text);\
                uint64_t hash = gs_hash_str64(VAR);\
                if (gs_hash_table_exists(variables->variables, hash))\
                {\
                    time_v = (uint32_t)(gs_hash_table_getp(variables->variables, hash))->val.number;\
                }\
                else\
                {\
                    gs_log_warning("Transition: Variable not found: %s.", VAR);\
                    return false;\
                }\
            } break;\
        }\
        ret &= (gs_lexer_require_token_type(lex, GS_TOKEN_DOLLAR) || gs_lexer_require_token_type(lex, GS_TOKEN_NUMBER));\
        if (!ret) {\
            gs_log_warning("Transition: Unidentified token.");\
            return false;\
        }\
        gs_token_t delay = gs_lexer_current_token(lex);\
        switch (delay.type)\
        {\
            case GS_TOKEN_NUMBER:\
            {\
                gs_snprintfc(DELAY, 32, "%.*s", delay.len, delay.text);\
                uint32_t delay_v = (uint32_t)atoi(DELAY);\
            } break;\
\
            case GS_TOKEN_DOLLAR:\
            {\
                ret &= (gs_lexer_require_token_type(lex, GS_TOKEN_IDENTIFIER));\
                if (!ret) {\
                    gs_log_warning("Transition: Variable missing identifier token.");\
                    return false;\
                }\
                token = gs_lexer_current_token(lex);\
                gs_snprintfc(VAR, 64, "%.*s", token.len, token.text);\
                uint64_t hash = gs_hash_str64(VAR);\
                if (gs_hash_table_exists(variables->variables, hash))\
                {\
                    delay_v = (uint32_t)(gs_hash_table_getp(variables->variables, hash))->val.number;\
                }\
                else\
                {\
                    gs_log_warning("Transition: Variable not found: %s.", VAR);\
                    return false;\
                }\
            } break;\
        }\
        gs_gui_animation_property_t prop = gs_default_val();\
        prop.type = T;\
        prop.time = (int16_t)time_v;\
        prop.delay = (int16_t)delay_v;\
        switch (state)\
        {\
            case GS_GUI_ELEMENT_STATE_DEFAULT:\
            {\
                for (uint32_t s = 0; s < 3; ++s)\
                {\
                    gs_dyn_array_push(list->properties[s], prop);\
                }\
            } break;\
            default:\
            {\
                gs_dyn_array_push(list->properties[state], prop);\
            } break;\
        }\
        ret &= (gs_lexer_require_token_type(lex, GS_TOKEN_SEMICOLON));\
        if (!ret) {\
            gs_log_warning("Transition: Missing semicolon.");\
            return false;\
        }\
    } while (0)

    _GS_GUI_SS_GET_TO_VALUES(GS_TOKEN_COLON, GS_TOKEN_LBRACE);

    if (!gs_hash_table_exists(ss->animations, (gs_gui_element_type)elementid)) 
    {
        gs_gui_animation_property_list_t list = gs_default_val();
        gs_hash_table_insert(ss->animations, (gs_gui_element_type)elementid, list);
    }
    gs_gui_animation_property_list_t* list = id_tag ? gs_hash_table_getp(ss->cid_animations, id_tag) : 
                 gs_hash_table_getp(ss->animations, (gs_gui_element_type)elementid);

    int32_t bc = 1;
    while (gs_lexer_can_lex(lex) && bc)
    {
        token = gs_lexer_next_token(lex);
        switch (token.type) 
        {
            case GS_TOKEN_LBRACE: bc++; break;
            case GS_TOKEN_RBRACE: bc--; break;
            case GS_TOKEN_IDENTIFIER:
            {
                if (gs_token_compare_text(&token, "color_background"))      PARSE_TRANSITION(GS_GUI_STYLE_COLOR_BACKGROUND);
                else if (gs_token_compare_text(&token, "color_border"))     PARSE_TRANSITION(GS_GUI_STYLE_COLOR_BORDER);
                else if (gs_token_compare_text(&token, "color_shadow"))     PARSE_TRANSITION(GS_GUI_STYLE_COLOR_SHADOW);
                else if (gs_token_compare_text(&token, "color_content"))    PARSE_TRANSITION(GS_GUI_STYLE_COLOR_CONTENT);
                else if (gs_token_compare_text(&token, "width"))            PARSE_TRANSITION(GS_GUI_STYLE_WIDTH);
                else if (gs_token_compare_text(&token, "height"))           PARSE_TRANSITION(GS_GUI_STYLE_HEIGHT);
                else if (gs_token_compare_text(&token, "padding"))          PARSE_TRANSITION(GS_GUI_STYLE_PADDING);
                else if (gs_token_compare_text(&token, "padding_left"))     PARSE_TRANSITION(GS_GUI_STYLE_PADDING_LEFT);
                else if (gs_token_compare_text(&token, "padding_right"))    PARSE_TRANSITION(GS_GUI_STYLE_PADDING_RIGHT);
                else if (gs_token_compare_text(&token, "padding_top"))      PARSE_TRANSITION(GS_GUI_STYLE_PADDING_TOP);
                else if (gs_token_compare_text(&token, "padding_bottom"))   PARSE_TRANSITION(GS_GUI_STYLE_PADDING_BOTTOM);
                else if (gs_token_compare_text(&token, "margin"))           PARSE_TRANSITION(GS_GUI_STYLE_PADDING);
                else if (gs_token_compare_text(&token, "margin_left"))      PARSE_TRANSITION(GS_GUI_STYLE_MARGIN_LEFT);
                else if (gs_token_compare_text(&token, "margin_right"))     PARSE_TRANSITION(GS_GUI_STYLE_MARGIN_RIGHT);
                else if (gs_token_compare_text(&token, "margin_top"))       PARSE_TRANSITION(GS_GUI_STYLE_MARGIN_TOP);
                else if (gs_token_compare_text(&token, "margin_bottom"))    PARSE_TRANSITION(GS_GUI_STYLE_MARGIN_BOTTOM);
                else if (gs_token_compare_text(&token, "shadow_x"))         PARSE_TRANSITION(GS_GUI_STYLE_SHADOW_X);
                else if (gs_token_compare_text(&token, "shadow_y"))         PARSE_TRANSITION(GS_GUI_STYLE_SHADOW_Y);
                else if (gs_token_compare_text(&token, "color_content_background")) PARSE_TRANSITION(GS_GUI_STYLE_COLOR_CONTENT_BACKGROUND);
                else if (gs_token_compare_text(&token, "color_content_border"))     PARSE_TRANSITION(GS_GUI_STYLE_COLOR_CONTENT_BORDER);
                else if (gs_token_compare_text(&token, "color_content_shadow"))     PARSE_TRANSITION(GS_GUI_STYLE_COLOR_CONTENT_SHADOW);
                else
                {
                    gs_log_warning("Unidentified attribute: %.*s.", token.len, token.text);
                    return false;
                }
            } break;
        }
    }

    return true;
}

bool _gs_gui_style_sheet_parse_attribute_font(gs_gui_context_t* ctx, gs_lexer_t* lex, gs_gui_style_sheet_t* ss, 
	uint64_t id_tag, int32_t elementid, int32_t state, gs_gui_ss_variables_t* variables)
{
    // Name of enum attribute 
    gs_token_t token = gs_lexer_current_token(lex);
    // gs_token_debug_print(&token); 
    //
    gs_gui_style_list_t* idsl = NULL;
    if (id_tag) 
    { 
        const uint64_t idhash = id_tag;
        if (!gs_hash_table_exists(ss->cid_styles, idhash))
        {
            gs_gui_style_list_t sl = gs_default_val();
            gs_hash_table_insert(ss->cid_styles, idhash, sl);
        }
        idsl = gs_hash_table_getp(ss->cid_styles, idhash);
    }

#define SET_FONT(FONT)\
    do {\
        switch (state)\
        {\
            case GS_GUI_ELEMENT_STATE_DEFAULT:\
            {\
                se.font = FONT;\
                for (uint32_t s = 0; s < 3; ++s)\
                {\
                    if (idsl) gs_dyn_array_push(idsl->styles[s], se);\
                    else ss->styles[elementid][s].font = FONT;\
                }\
            } break;\
            default:\
            {\
                se.font = FONT;\
                if (idsl) gs_dyn_array_push(idsl->styles[state], se);\
                ss->styles[elementid][state].font = FONT;\
            } break;\
        }\
    } while (0)

    if (gs_token_compare_text(&token, "font"))
    {
        bool ret = gs_lexer_require_token_type(lex, GS_TOKEN_COLON);
        ret &= (gs_lexer_require_token_type(lex, GS_TOKEN_STRING) || 
            gs_lexer_require_token_type(lex, GS_TOKEN_DOLLAR));
        if (!ret)
        {
            gs_log_warning("Missing either string value or variable.");
            return false;
        }
        gs_gui_style_element_t se = gs_default_val();
        se.type = GS_GUI_STYLE_FONT;
        token = gs_lexer_current_token(lex); 
        char FONT[64] = gs_default_val();
        switch (token.type)
        {
            case GS_TOKEN_STRING:
            {
                gs_snprintf(FONT, sizeof(FONT), "%.*s", token.len - 2, token.text + 1);
            } break;

            case GS_TOKEN_DOLLAR:
            {
                bool ret = gs_lexer_require_token_type(lex, GS_TOKEN_IDENTIFIER);
                if (!ret) {
                    gs_log_warning("Unidentified token.");
                    return false;
                }\
                token = gs_lexer_current_token(lex);
                gs_snprintfc(TMP, 256, "%.*s", token.len, token.text);
                uint64_t hash = gs_hash_str64(TMP);
                gs_gui_ss_var_def_t* var = gs_hash_table_exists(variables->variables, hash) ?
                    gs_hash_table_getp(variables->variables, hash) : NULL;
                if (!var) {
                    gs_log_warning("Variable not found: %s", TMP);
                    return false;
                }
                memcpy(FONT, var->val.str, sizeof(FONT));
                ret = gs_lexer_require_token_type(lex, GS_TOKEN_SEMICOLON);
                if (!ret) {
                    gs_log_warning("Missing semicolon.");
                    return false;
                }
                token = gs_lexer_current_token(lex);
            } break; 
        }

        uint64_t hash = gs_hash_str64(FONT);
        bool found = false;
        for (
                gs_hash_table_iter it = gs_hash_table_iter_new(ctx->font_stash);
                gs_hash_table_iter_valid(ctx->font_stash, it);
                gs_hash_table_iter_advance(ctx->font_stash, it)
        )
        {
            uint64_t key = gs_hash_table_getk(ctx->font_stash, it);
            if (hash == key)
            {
                gs_asset_font_t* font = gs_hash_table_iter_get(ctx->font_stash, it);
                SET_FONT(font); 
                found = true;
                break;
            } 
        }
        if (!found)
        {
            gs_log_warning("Font not found in gui font stash: %s", FONT);
        }
    }
    else
    {
        gs_log_warning("Unidentified token.");
        return false;
    }

    return true; 
}

bool _gs_gui_style_sheet_parse_attribute_enum(gs_gui_context_t* ctx, gs_lexer_t* lex, gs_gui_style_sheet_t* ss, 
	uint64_t id_tag, int32_t elementid, int32_t state, gs_gui_ss_variables_t* variables)
{
    // Name of enum attribute 
    gs_token_t token = gs_lexer_current_token(lex);
    // gs_token_debug_print(&token); 

    gs_gui_style_list_t* idsl = NULL;
    if (id_tag) 
    { 
        const uint64_t idhash = id_tag;
        if (!gs_hash_table_exists(ss->cid_styles, idhash))
        {
            gs_gui_style_list_t sl = gs_default_val();
            gs_hash_table_insert(ss->cid_styles, idhash, sl);
        }
        idsl = gs_hash_table_getp(ss->cid_styles, idhash);
    }

#define SET_ENUM(COMP, VAL)\
    do {\
        se.value = VAL;\
        switch (state)\
        {\
            case GS_GUI_ELEMENT_STATE_DEFAULT:\
            {\
                if (idsl)\
                {\
                    for (uint32_t s = 0; s < 3; ++s) {\
                        gs_dyn_array_push(idsl->styles[s], se);\
                    }\
                }\
                else\
                {\
                    for (uint32_t s = 0; s < 3; ++s) {\
                        ss->styles[elementid][s].COMP = VAL;\
                    }\
                }\
            } break;\
            default:\
            {\
                if (idsl) gs_dyn_array_push(idsl->styles[state], se);\
                else ss->styles[elementid][state].COMP = VAL;\
            } break;\
        }\
    } while (0)

    if (gs_token_compare_text(&token, "justify_content"))
    {
        gs_gui_style_element_t se = gs_default_val();
        se.type = GS_GUI_STYLE_JUSTIFY_CONTENT;
        bool ret = gs_lexer_require_token_type(lex, GS_TOKEN_COLON);
        ret &= (gs_lexer_require_token_type(lex, GS_TOKEN_IDENTIFIER) || 
            gs_lexer_require_token_type(lex, GS_TOKEN_DOLLAR));
        if (!ret)
        {
            gs_log_warning("Missing either identifier value or variable.");
            return false;
        }
        token = gs_lexer_current_token(lex); 
        switch (token.type)
        {
            case GS_TOKEN_IDENTIFIER: 
            {
                if (gs_token_compare_text(&token, "start")) SET_ENUM(justify_content, GS_GUI_JUSTIFY_START);
                else if (gs_token_compare_text(&token, "end")) SET_ENUM(justify_content, GS_GUI_JUSTIFY_END);
                else if (gs_token_compare_text(&token, "center")) SET_ENUM(justify_content, GS_GUI_JUSTIFY_CENTER);
            } break;

            case GS_TOKEN_DOLLAR:
            {
                bool ret = gs_lexer_require_token_type(lex, GS_TOKEN_IDENTIFIER);
                if (!ret) {
                    gs_log_warning("Unidentified token.");
                    return false;
                }\
                token = gs_lexer_current_token(lex);
                gs_snprintfc(TMP, 256, "%.*s", token.len, token.text);
                uint64_t hash = gs_hash_str64(TMP);
                gs_gui_ss_var_def_t* var = gs_hash_table_exists(variables->variables, hash) ?
                    gs_hash_table_getp(variables->variables, hash) : NULL;
                if (!var) {
                    gs_log_warning("Variable not found: %s", TMP);
                    return false;
                }
                SET_ENUM(justify_content, var->val.number);
                ret = gs_lexer_require_token_type(lex, GS_TOKEN_SEMICOLON);
                if (!ret) {
                    gs_log_warning("Missing semicolon.");
                    return false;
                }
                token = gs_lexer_current_token(lex);
            } break;
        }
    }
    else if (gs_token_compare_text(&token, "align_content"))
    { 
        gs_gui_style_element_t se = gs_default_val();
        se.type = GS_GUI_STYLE_ALIGN_CONTENT;
        bool ret = gs_lexer_require_token_type(lex, GS_TOKEN_COLON);
        ret &= (gs_lexer_require_token_type(lex, GS_TOKEN_IDENTIFIER) || 
            gs_lexer_require_token_type(lex, GS_TOKEN_DOLLAR));
        if (!ret)
        {
            gs_log_warning("Missing either identifier value or variable.");
            return false;
        }
        token = gs_lexer_current_token(lex); 
        switch (token.type)
        {
            case GS_TOKEN_IDENTIFIER: 
            {
                if (gs_token_compare_text(&token, "start")) SET_ENUM(align_content, GS_GUI_ALIGN_START);
                else if (gs_token_compare_text(&token, "end")) SET_ENUM(align_content, GS_GUI_ALIGN_END);
                else if (gs_token_compare_text(&token, "center")) SET_ENUM(align_content, GS_GUI_ALIGN_CENTER);
            } break;

            case GS_TOKEN_DOLLAR:
            {
                bool ret = gs_lexer_require_token_type(lex, GS_TOKEN_IDENTIFIER);
                if (!ret) {
                    gs_log_warning("Unidentified token.");
                    return false;
                }\
                token = gs_lexer_current_token(lex);
                gs_snprintfc(TMP, 256, "%.*s", token.len, token.text);
                uint64_t hash = gs_hash_str64(TMP);
                gs_gui_ss_var_def_t* var = gs_hash_table_exists(variables->variables, hash) ?
                    gs_hash_table_getp(variables->variables, hash) : NULL;
                if (!var) {
                    gs_log_warning("Variable not found: %s", TMP);
                    return false;
                }
                SET_ENUM(align_content, var->val.number);
                ret = gs_lexer_require_token_type(lex, GS_TOKEN_SEMICOLON);
                if (!ret) {
                    gs_log_warning("Missing semicolon.");
                    return false;
                }
                token = gs_lexer_current_token(lex);
            } break;
        }
    }

    return true;
}

bool _gs_gui_style_sheet_parse_attribute_val(gs_gui_context_t* ctx, gs_lexer_t* lex, gs_gui_style_sheet_t* ss, 
	uint64_t id_tag, int32_t elementid, int32_t state, gs_gui_ss_variables_t* variables)
{
    // Name of value attribute 
    gs_token_t token = gs_lexer_current_token(lex);
    // gs_token_debug_print(&token);

    gs_gui_style_list_t* idsl = NULL;
    if (id_tag) 
    { 
        const uint64_t idhash = id_tag;
        if (!gs_hash_table_exists(ss->cid_styles, idhash))
        {
            gs_gui_style_list_t sl = gs_default_val();
            gs_hash_table_insert(ss->cid_styles, idhash, sl);
        }
        idsl = gs_hash_table_getp(ss->cid_styles, idhash);
    }

    #define SET_VAL4(COMP, T, SE)\
        do {\
            gs_gui_style_element_t se = gs_default_val();\
            se.type = SE;\
            bool ret = gs_lexer_require_token_type(lex, GS_TOKEN_COLON);\
            ret &= (gs_lexer_require_token_type(lex, GS_TOKEN_DOLLAR) || gs_lexer_require_token_type(lex, GS_TOKEN_NUMBER));\
            token = gs_lexer_current_token(lex);\
            gs_token_debug_print(&token);\
            if (!ret) {\
                gs_log_warning("Unidentified token.");\
                return false;\
            }\
            switch (token.type)\
            {\
                case GS_TOKEN_NUMBER:\
                {\
                    gs_snprintfc(TMP, 10, "%.*s", token.len, token.text);\
                    uint32_t val = (uint32_t)atoi(TMP);\
                    se.value = (T)val;\
                    switch (state) {\
                        case GS_GUI_ELEMENT_STATE_DEFAULT:\
                        {\
                            if (idsl) {\
                                gs_dyn_array_push(idsl->styles[GS_GUI_ELEMENT_STATE_DEFAULT], se);\
                                gs_dyn_array_push(idsl->styles[GS_GUI_ELEMENT_STATE_HOVER], se);\
                                gs_dyn_array_push(idsl->styles[GS_GUI_ELEMENT_STATE_FOCUS], se);\
                            }\
                            else {\
                                for (uint32_t p = 0; p < 4; ++p) {\
                                    ss->styles[elementid][GS_GUI_ELEMENT_STATE_DEFAULT].COMP[p] = (T)val;\
                                    ss->styles[elementid][GS_GUI_ELEMENT_STATE_HOVER].COMP[p] = (T)val;\
                                    ss->styles[elementid][GS_GUI_ELEMENT_STATE_FOCUS].COMP[p] = (T)val;\
                                }\
                            }\
                        } break;\
                        default:\
                        {\
                            if (idsl) gs_dyn_array_push(idsl->styles[state], se);\
                            else {\
                                for (uint32_t p = 0; p < 4; ++p) {\
                                    ss->styles[elementid][state].COMP[p] = (T)val;\
                                }\
                            }\
                        } break;\
                    }\
                } break;\
                case GS_TOKEN_DOLLAR:\
                {\
                    bool ret = gs_lexer_require_token_type(lex, GS_TOKEN_IDENTIFIER);\
                    if (!ret) {\
                        gs_log_warning("Unidentified token.");\
                        return false;\
                    }\
                    token = gs_lexer_current_token(lex);\
                    gs_snprintfc(TMP, 256, "%.*s", token.len, token.text);\
                    uint64_t hash = gs_hash_str64(TMP);\
                    gs_gui_ss_var_def_t* var = gs_hash_table_exists(variables->variables, hash) ?\
                        gs_hash_table_getp(variables->variables, hash) : NULL;\
                    if (!var) {\
                        gs_log_warning("Variable not found: %s", TMP);\
                        return false;\
                    }\
                    T val = var->val.number;\
                    se.value = val;\
                    switch (state) {\
                        case GS_GUI_ELEMENT_STATE_DEFAULT:\
                        {\
                            if (idsl) {\
                                gs_dyn_array_push(idsl->styles[GS_GUI_ELEMENT_STATE_DEFAULT], se);\
                                gs_dyn_array_push(idsl->styles[GS_GUI_ELEMENT_STATE_HOVER], se);\
                                gs_dyn_array_push(idsl->styles[GS_GUI_ELEMENT_STATE_FOCUS], se);\
                            }\
                            else {\
                                for (uint32_t p = 0; p < 4; ++p) {\
                                    ss->styles[elementid][GS_GUI_ELEMENT_STATE_DEFAULT].COMP[p] = val;\
                                    ss->styles[elementid][GS_GUI_ELEMENT_STATE_HOVER].COMP[p] = val;\
                                    ss->styles[elementid][GS_GUI_ELEMENT_STATE_FOCUS].COMP[p] = val;\
                                }\
                            }\
                        } break;\
                        default:\
                        {\
                            if (idsl) gs_dyn_array_push(idsl->styles[state], se);\
                            else {\
                                for (uint32_t p = 0; p < 4; ++p) {\
                                    ss->styles[elementid][state].COMP[p] = val;\
                                }\
                            }\
                        } break;\
                    }\
                    ret = gs_lexer_require_token_type(lex, GS_TOKEN_SEMICOLON);\
                    if (!ret) {\
                        gs_log_warning("Missing semicolon.");\
                        return false;\
                    }\
                    token = gs_lexer_current_token(lex);\
                } break;\
            }\
        } while (0)

    #define SET_VAL2(COMP0, COMP1, T, SE0, SE1)\
        do {\
            gs_gui_style_element_t se0 = gs_default_val();\
            gs_gui_style_element_t se1 = gs_default_val();\
            se0.type = SE0;\
            se1.type = SE1;\
            bool ret = gs_lexer_require_token_type(lex, GS_TOKEN_COLON);\
            ret &= (gs_lexer_require_token_type(lex, GS_TOKEN_DOLLAR) || gs_lexer_require_token_type(lex, GS_TOKEN_NUMBER));\
            if (!ret) {\
                gs_log_warning("Unidentified token.");\
                return false;\
            }\
            token = gs_lexer_current_token(lex);\
            T val = 0;\
            switch (token.type)\
            {\
                case GS_TOKEN_NUMBER:\
                {\
                    gs_snprintfc(TMP, 10, "%.*s", token.len, token.text);\
                    val = (T)atoi(TMP);\
                } break;\
                case GS_TOKEN_DOLLAR:\
                {\
                    bool ret = gs_lexer_require_token_type(lex, GS_TOKEN_IDENTIFIER);\
                    if (!ret) {\
                        gs_log_warning("Unidentified token.");\
                        return false;\
                    }\
                    token = gs_lexer_current_token(lex);\
                    gs_snprintfc(TMP, 256, "%.*s", token.len, token.text);\
                    uint64_t hash = gs_hash_str64(TMP);\
                    gs_gui_ss_var_def_t* var = gs_hash_table_exists(variables->variables, hash) ?\
                        gs_hash_table_getp(variables->variables, hash) : NULL;\
                    if (!var) {\
                        gs_log_warning("Variable not found: %s", TMP);\
                        return false;\
                    }\
                    val = (T)var->val.number;\
                    ret = gs_lexer_require_token_type(lex, GS_TOKEN_SEMICOLON);\
                    if (!ret) {\
                        gs_log_warning("Missing semicolon.");\
                        return false;\
                    }\
                    token = gs_lexer_current_token(lex);\
                } break;\
            }\
            switch (state)\
            {\
                case GS_GUI_ELEMENT_STATE_DEFAULT:\
                {\
                    if (idsl)\
                    {\
                        se0.value = val;\
                        se1.value = val;\
                        gs_dyn_array_push(idsl->styles[GS_GUI_ELEMENT_STATE_DEFAULT], se0);\
                        gs_dyn_array_push(idsl->styles[GS_GUI_ELEMENT_STATE_HOVER], se0);\
                        gs_dyn_array_push(idsl->styles[GS_GUI_ELEMENT_STATE_FOCUS], se0);\
                        gs_dyn_array_push(idsl->styles[GS_GUI_ELEMENT_STATE_DEFAULT], se1);\
                        gs_dyn_array_push(idsl->styles[GS_GUI_ELEMENT_STATE_HOVER], se1);\
                        gs_dyn_array_push(idsl->styles[GS_GUI_ELEMENT_STATE_FOCUS], se1);\
                    }\
                    else\
                    {\
                        ss->styles[elementid][GS_GUI_ELEMENT_STATE_DEFAULT].COMP0 = val;\
                        ss->styles[elementid][GS_GUI_ELEMENT_STATE_HOVER].COMP0 = val;\
                        ss->styles[elementid][GS_GUI_ELEMENT_STATE_FOCUS].COMP0 = val;\
                        ss->styles[elementid][GS_GUI_ELEMENT_STATE_DEFAULT].COMP1 = val;\
                        ss->styles[elementid][GS_GUI_ELEMENT_STATE_HOVER].COMP1 = val;\
                        ss->styles[elementid][GS_GUI_ELEMENT_STATE_FOCUS].COMP1 = val;\
                    }\
                } break;\
                default:\
                {\
                    if (idsl)\
                    {\
                        se0.value = val;\
                        se1.value = val;\
                        gs_dyn_array_push(idsl->styles[state], se0);\
                        gs_dyn_array_push(idsl->styles[state], se1);\
                    }\
                    else\
                    {\
                        ss->styles[elementid][state].COMP0 = val;\
                        ss->styles[elementid][state].COMP1 = val;\
                    }\
                } break;\
            }\
        } while (0)

    #define SET_VAL(COMP, T, SE)\
        do {\
            bool ret = gs_lexer_require_token_type(lex, GS_TOKEN_COLON);\
            token = gs_lexer_current_token(lex);\
            ret &= (gs_lexer_require_token_type(lex, GS_TOKEN_DOLLAR) || gs_lexer_require_token_type(lex, GS_TOKEN_NUMBER));\
            token = gs_lexer_current_token(lex);\
            if (!ret) {\
                gs_log_warning("Unidentified token: %.*s", token.len, token.text);\
                return false;\
            }\
            gs_gui_style_element_t se = gs_default_val();\
            se.type = SE;\
            T val = 0;\
            switch (token.type)\
            {\
                case GS_TOKEN_NUMBER:\
                {\
                    gs_snprintfc(TMP, 10, "%.*s", token.len, token.text);\
                    val = (T)atoi(TMP);\
                } break;\
\
                case GS_TOKEN_DOLLAR:\
                {\
                    bool ret = gs_lexer_require_token_type(lex, GS_TOKEN_IDENTIFIER);\
                    if (!ret) {\
                        gs_log_warning("Unidentified token.");\
                        return false;\
                    }\
                    token = gs_lexer_current_token(lex);\
                    gs_snprintfc(TMP, 256, "%.*s", token.len, token.text);\
                    uint64_t hash = gs_hash_str64(TMP);\
                    gs_gui_ss_var_def_t* var = gs_hash_table_exists(variables->variables, hash) ?\
                        gs_hash_table_getp(variables->variables, hash) : NULL;\
                    if (!var) {\
                        gs_log_warning("Variable not found: %s", TMP);\
                        return false;\
                    }\
                    val = (T)var->val.number;\
                    ret = gs_lexer_require_token_type(lex, GS_TOKEN_SEMICOLON);\
                    if (!ret) {\
                        gs_log_warning("Missing semicolon.");\
                        return false;\
                    }\
                    token = gs_lexer_current_token(lex);\
                } break;\
            }\
            switch (state)\
            {\
                case GS_GUI_ELEMENT_STATE_DEFAULT:\
                {\
                    if (idsl)\
                    {\
                        se.value = (int32_t)val;\
                        gs_dyn_array_push(idsl->styles[GS_GUI_ELEMENT_STATE_DEFAULT], se);\
                        gs_dyn_array_push(idsl->styles[GS_GUI_ELEMENT_STATE_HOVER], se);\
                        gs_dyn_array_push(idsl->styles[GS_GUI_ELEMENT_STATE_FOCUS], se);\
                    }\
                    else\
                    {\
                        ss->styles[elementid][GS_GUI_ELEMENT_STATE_DEFAULT].COMP = val;\
                        ss->styles[elementid][GS_GUI_ELEMENT_STATE_HOVER].COMP = val;\
                        ss->styles[elementid][GS_GUI_ELEMENT_STATE_FOCUS].COMP = val;\
                    }\
                } break;\
                default:\
                {\
                    if (idsl)\
                    {\
                        se.value = (int32_t)val;\
                        gs_dyn_array_push(idsl->styles[state], se);\
                    }\
                    else\
                    {\
                        ss->styles[elementid][state].COMP = val;\
                    }\
                } break;\
                }\
        } while (0)

    if (gs_token_compare_text(&token, "width"))               SET_VAL(size[0], float, GS_GUI_STYLE_WIDTH);
    else if (gs_token_compare_text(&token, "height"))         SET_VAL(size[1], float, GS_GUI_STYLE_HEIGHT);
    else if (gs_token_compare_text(&token, "padding"))        SET_VAL4(padding, int16_t, GS_GUI_STYLE_PADDING); 
    else if (gs_token_compare_text(&token, "padding_left"))   SET_VAL(padding[GS_GUI_PADDING_LEFT], int16_t, GS_GUI_STYLE_PADDING_LEFT);
    else if (gs_token_compare_text(&token, "padding_right"))  SET_VAL(padding[GS_GUI_PADDING_RIGHT], int16_t, GS_GUI_STYLE_PADDING_RIGHT);
    else if (gs_token_compare_text(&token, "padding_top"))    SET_VAL(padding[GS_GUI_PADDING_TOP], int16_t, GS_GUI_STYLE_PADDING_TOP);
    else if (gs_token_compare_text(&token, "padding_bottom")) SET_VAL(padding[GS_GUI_PADDING_BOTTOM], int16_t, GS_GUI_STYLE_PADDING_BOTTOM);
    else if (gs_token_compare_text(&token, "margin"))         SET_VAL4(margin, int16_t, GS_GUI_STYLE_MARGIN);
    else if (gs_token_compare_text(&token, "margin_left"))    SET_VAL(margin[GS_GUI_MARGIN_LEFT], int16_t, GS_GUI_STYLE_MARGIN_LEFT);
    else if (gs_token_compare_text(&token, "margin_right"))   SET_VAL(margin[GS_GUI_MARGIN_RIGHT], int16_t, GS_GUI_STYLE_MARGIN_RIGHT);
    else if (gs_token_compare_text(&token, "margin_top"))     SET_VAL(margin[GS_GUI_MARGIN_TOP], int16_t, GS_GUI_STYLE_MARGIN_TOP);
    else if (gs_token_compare_text(&token, "margin_bottom"))  SET_VAL(margin[GS_GUI_MARGIN_BOTTOM], int16_t, GS_GUI_STYLE_MARGIN_BOTTOM);
    else if (gs_token_compare_text(&token, "border"))         SET_VAL4(border_width, int16_t, GS_GUI_STYLE_BORDER_WIDTH);
    else if (gs_token_compare_text(&token, "border_left"))    SET_VAL(border_width[0], int16_t, GS_GUI_STYLE_BORDER_WIDTH_LEFT);
    else if (gs_token_compare_text(&token, "border_right"))   SET_VAL(border_width[1], int16_t, GS_GUI_STYLE_BORDER_WIDTH_RIGHT);
    else if (gs_token_compare_text(&token, "border_top"))     SET_VAL(border_width[2], int16_t, GS_GUI_STYLE_BORDER_WIDTH_TOP);
    else if (gs_token_compare_text(&token, "border_bottom"))  SET_VAL(border_width[3], int16_t, GS_GUI_STYLE_BORDER_WIDTH_BOTTOM);
    else if (gs_token_compare_text(&token, "shadow"))         SET_VAL2(shadow_x, shadow_y, int16_t, GS_GUI_STYLE_SHADOW_X, GS_GUI_STYLE_SHADOW_Y);

    return true;
}

bool _gs_gui_style_sheet_parse_attribute_color(gs_gui_context_t* ctx, gs_lexer_t* lex, gs_gui_style_sheet_t* ss, 
	uint64_t id_tag, int32_t elementid, int32_t state, gs_gui_ss_variables_t* variables)
{ 
    // Name of color attribute 
    gs_token_t token = gs_lexer_current_token(lex);
    // gs_token_debug_print(&token);

    gs_gui_style_list_t* idsl = NULL;
    if (id_tag) 
    { 
        const uint64_t idhash = id_tag;
        if (!gs_hash_table_exists(ss->cid_styles, idhash))
        {
            gs_gui_style_list_t sl = gs_default_val();
            gs_hash_table_insert(ss->cid_styles, idhash, sl);
        }
        idsl = gs_hash_table_getp(ss->cid_styles, idhash);
    }

    gs_gui_style_element_type type = (gs_gui_style_element_type)0x00;
    int32_t color = GS_GUI_COLOR_BACKGROUND; 
    if (gs_token_compare_text(&token, "color_background"))   {color = GS_GUI_COLOR_BACKGROUND; type = GS_GUI_STYLE_COLOR_BACKGROUND;}
    else if (gs_token_compare_text(&token, "color_border"))  {color = GS_GUI_COLOR_BORDER; type = GS_GUI_STYLE_COLOR_BORDER;}
    else if (gs_token_compare_text(&token, "color_shadow"))  {color = GS_GUI_COLOR_SHADOW; type = GS_GUI_STYLE_COLOR_SHADOW;}
    else if (gs_token_compare_text(&token, "color_content")) {color = GS_GUI_COLOR_CONTENT; type = GS_GUI_STYLE_COLOR_CONTENT;}
    else if (gs_token_compare_text(&token, "color_content_background")) {color = GS_GUI_COLOR_CONTENT_BACKGROUND; type = GS_GUI_STYLE_COLOR_CONTENT_BACKGROUND;}
    else if (gs_token_compare_text(&token, "color_content_border")) {color = GS_GUI_COLOR_CONTENT_BORDER; type = GS_GUI_STYLE_COLOR_CONTENT_BORDER;}
    else if (gs_token_compare_text(&token, "color_content_shadow")) {color = GS_GUI_COLOR_CONTENT_SHADOW; type = GS_GUI_STYLE_COLOR_CONTENT_SHADOW;}
    else
    {
        gs_log_warning("Unidentified attribute: %.*s.", token.len, token.text);
        return false;
    }

    token = gs_lexer_next_token(lex);
    if (token.type != GS_TOKEN_COLON)
    {
        gs_log_warning("Unidentified token. (Expected colon after attribute type).");
        gs_token_debug_print(&token);
        return false;
    }

    gs_gui_style_element_t se = gs_default_val();
    se.type = type; 

    token = gs_lexer_next_token(lex); 
    // gs_println("Parsing color: %.*s", token.len, token.text);

    if (gs_token_compare_text(&token, "rgba") ||
        gs_token_compare_text(&token, "rgb")
    )
    {
        int32_t i = 0;
        while (gs_lexer_can_lex(lex) && token.type != GS_TOKEN_RPAREN)
        {
            token = gs_lexer_next_token(lex);
            switch (token.type)
            { 
                case GS_TOKEN_NUMBER:
                {
                    if (i < 4)
                    {
                        gs_snprintfc(TMP, 10, "%.*s", token.len, token.text);
                        uint8_t val = (uint8_t)atoi(TMP);

                        if (idsl)
                        {
                            se.color.rgba[i] = val;
                        }
                        else
                        { 
                            #define SET_COLOR(STATE)\
                                do {\
                                    switch (i)\
                                    {\
                                        case 0: ss->styles[elementid][STATE].colors[color].r = val; break;\
                                        case 1: ss->styles[elementid][STATE].colors[color].g = val; break;\
                                        case 2: ss->styles[elementid][STATE].colors[color].b = val; break;\
                                        case 3: ss->styles[elementid][STATE].colors[color].a = val; break;\
                                    }\
                                } while (0)

                            switch (state)
                            {
                                case GS_GUI_ELEMENT_STATE_HOVER:
                                case GS_GUI_ELEMENT_STATE_FOCUS:
                                {
                                    SET_COLOR(state);
                                } break;

                                case GS_GUI_ELEMENT_STATE_DEFAULT:
                                {
                                    for (uint32_t s = 0; s < 3; ++s) {
                                        SET_COLOR(s);
                                    }
                                } break;
                            }
                        }
                        i++;
                        // gs_token_debug_print(&token);
                    }
                } break;
            }
        }

        // Set alpha, if not provided
		if (i < 4)
		{
			ss->styles[elementid][state].colors[color].a = 255;
			se.color.a = 255;
		}

        // Push style element
        if (idsl)
        {
			switch (state) 
			{ 
				case GS_GUI_ELEMENT_STATE_DEFAULT: 
				{
					for (uint32_t s = 0; s < 3; ++s) {
						gs_dyn_array_push(idsl->styles[s], se);
					}
				} break;
				default:
				{
					gs_dyn_array_push(idsl->styles[state], se);
				} break;
			}
        }
    }
    else if (gs_token_compare_text(&token, "$"))
    { 
        token = gs_lexer_next_token(lex);
        if (token.type != GS_TOKEN_IDENTIFIER)
        {
            gs_log_warning("Unidentified symbol found: %.*s. Expecting identifier for variable name.", token.len, token.text);
            return false;
        }

        gs_snprintfc(TMP, 256, "%.*s", token.len, token.text);
        uint64_t hash = gs_hash_str64(TMP);
        gs_gui_ss_var_def_t* var = gs_hash_table_exists(variables->variables, hash) ? gs_hash_table_getp(variables->variables, hash) : NULL; 
        if (var) { 

            se.color = var->val.color;

            switch (state) 
            {
                default: 
                {
                    if (idsl) gs_dyn_array_push(idsl->styles[state], se);
                    else ss->styles[elementid][state].colors[color] = var->val.color;
                } break;

                case GS_GUI_ELEMENT_STATE_DEFAULT:
                {
					for (uint32_t s = 0; s < 3; ++s) {
						if (idsl) gs_dyn_array_push(idsl->styles[s], se);
                        else ss->styles[elementid][s].colors[color] = var->val.color;
					}
                } break;
            }
        }
        else
        {
            gs_log_warning("Variable not found: %.*s.", token.len, token.text);
        }
        token = gs_lexer_next_token(lex);
        if (token.type != GS_TOKEN_SEMICOLON) 
        {
            gs_log_warning("Syntax error. Expecting semicolon, found: %.*s.", token.len, token.text);
            return false; 
        }
    }
    else
    {
        gs_log_warning("Unidentified color type found: %.*s. (Expect either 'rgba' or 'rgb').", token.len, token.text);
        return false;
    }

    
    return true;
}

bool _gs_gui_style_sheet_parse_attribute(gs_gui_context_t* ctx, gs_lexer_t* lex, gs_gui_style_sheet_t* ss, 
	uint64_t id_tag, int32_t elementid, int32_t state, gs_gui_ss_variables_t* variables)
{
    // Name of attribute
    gs_token_t token = gs_lexer_current_token(lex); 

    if (gs_token_compare_text(&token, "color_background")         || 
        gs_token_compare_text(&token, "color_border")             || 
        gs_token_compare_text(&token, "color_shadow")             ||
        gs_token_compare_text(&token, "color_content")            ||  
        gs_token_compare_text(&token, "color_content_background") ||
        gs_token_compare_text(&token, "color_content_border")     ||
        gs_token_compare_text(&token, "color_content_shadow")    
    )
    {
        if (!_gs_gui_style_sheet_parse_attribute_color(ctx, lex, ss, id_tag, elementid, state, variables))
        {
            gs_log_warning("Failed to parse color attribute.");
            return false;
        }
    }
    else if (gs_token_compare_text(&token, "width") || 
             gs_token_compare_text(&token, "height") || 
             gs_token_compare_text(&token, "padding") ||
             gs_token_compare_text(&token, "padding_left") ||
             gs_token_compare_text(&token, "padding_right") ||
             gs_token_compare_text(&token, "padding_top") ||    
             gs_token_compare_text(&token, "padding_bottom") ||
             gs_token_compare_text(&token, "margin") ||
             gs_token_compare_text(&token, "margin_left") ||
             gs_token_compare_text(&token, "margin_right") ||
             gs_token_compare_text(&token, "margin_top") ||
             gs_token_compare_text(&token, "margin_bottom") ||
             gs_token_compare_text(&token, "border") ||
             gs_token_compare_text(&token, "border_left") ||
             gs_token_compare_text(&token, "border_right") ||
             gs_token_compare_text(&token, "border_top") ||
             gs_token_compare_text(&token, "border_bottom") ||
             gs_token_compare_text(&token, "shadow")
    )
    {
        if (!_gs_gui_style_sheet_parse_attribute_val(ctx, lex, ss, id_tag, elementid, state, variables))
        {
            gs_log_warning("Failed to parse value attribute.");
            return false;
        }
    }
    else if (gs_token_compare_text(&token, "justify_content") || 
             gs_token_compare_text(&token, "align_content")
    )
    {
        if (!_gs_gui_style_sheet_parse_attribute_enum(ctx, lex, ss, id_tag, elementid, state, variables))
        {
            gs_log_warning("Failed to parse enum attribute.");
            return false;
        }
    }

    else if (gs_token_compare_text(&token, "font"))
    {
        if (!_gs_gui_style_sheet_parse_attribute_font(ctx, lex, ss, id_tag, elementid, state, variables))
        {
            gs_log_warning("Failed to parse font attribute.");
            return false;
        }
    }

    else if (gs_token_compare_text(&token, "transition"))
    {
        if (!_gs_gui_style_sheet_parse_attribute_transition(ctx, lex, ss, id_tag, elementid, state, variables))
        {
            gs_log_warning("Failed to parse transition attribute.");
            return false;
        }
    }
    else
    {
        gs_log_warning("Unidentified attribute: %.*s.", token.len, token.text);
        return false;
    }
    return true;
}

bool _gs_gui_style_sheet_parse_element(gs_gui_context_t* ctx, gs_lexer_t* lex, gs_gui_style_sheet_t* ss, 
	int32_t elementid, gs_gui_ss_variables_t* variables)
{
    int32_t state = 0x00;
    int32_t bc = 0;
    gs_token_t token = gs_lexer_next_token(lex);
    if (token.type == GS_TOKEN_COLON) 
    { 
        token = gs_lexer_next_token(lex);
        if (token.type != GS_TOKEN_IDENTIFIER) 
        {
            gs_log_warning("Unidentified Token. (Expected identifier after colon).");
            gs_token_debug_print(&token);
            return false;
        }

        if (gs_token_compare_text(&token, "focus"))      state = GS_GUI_ELEMENT_STATE_FOCUS;
        else if (gs_token_compare_text(&token, "hover")) state = GS_GUI_ELEMENT_STATE_HOVER;
        else
        {
            gs_log_warning("Unidentified element state provided: %.*s", token.len, token.text);
            return false;
        }

        // Get rbrace
        token = gs_lexer_next_token(lex);
    }

    if (token.type != GS_TOKEN_LBRACE) 
    {
        gs_log_warning("Unidentified token. (Expected brace after element declaration).");
        gs_token_debug_print(&token);
        return false;
    } 

    bc = 1;
    while (gs_lexer_can_lex(lex) && bc)
    {
        token = gs_lexer_next_token(lex);
        switch (token.type)
        {
            case GS_TOKEN_LBRACE: bc++; break;
            case GS_TOKEN_RBRACE: bc--; break;
            case GS_TOKEN_IDENTIFIER:
            { 
                // gs_println("Parsing attribute: %.*s", token.len, token.text);
                if (!_gs_gui_style_sheet_parse_attribute(ctx, lex, ss, 0, elementid, state, variables))
                {
                    gs_log_warning("Unable to parse attribute");
                    return false;
                }
            } break;
        }
    }

    return true;
}

bool _gs_gui_style_sheet_parse_cid_tag(gs_gui_context_t* ctx, gs_lexer_t* lex, gs_gui_style_sheet_t* ss, 
	const uint64_t cid_tag, gs_gui_ss_variables_t* variables)
{
    int32_t state = 0x00;
    int32_t bc = 0;
    gs_token_t token = gs_lexer_next_token(lex);
    if (token.type == GS_TOKEN_COLON) 
    { 
        token = gs_lexer_next_token(lex);
        if (token.type != GS_TOKEN_IDENTIFIER) 
        {
            gs_log_warning("Unidentified Token. (Expected identifier after colon).");
            gs_token_debug_print(&token);
            return false;
        }

        if (gs_token_compare_text(&token, "focus"))      state = GS_GUI_ELEMENT_STATE_FOCUS;
        else if (gs_token_compare_text(&token, "hover")) state = GS_GUI_ELEMENT_STATE_HOVER;
        else
        {
            gs_log_warning("Unidentified element state provided: %.*s", token.len, token.text);
            return false;
        }

        // Get rbrace
        token = gs_lexer_next_token(lex);
    }

    if (token.type != GS_TOKEN_LBRACE) 
    {
        gs_log_warning("Unidentified token. (Expected brace after element declaration).");
        gs_token_debug_print(&token);
        return false;
    } 

    bc = 1;
    while (gs_lexer_can_lex(lex) && bc)
    {
        token = gs_lexer_next_token(lex);
        switch (token.type)
        {
            case GS_TOKEN_LBRACE: bc++; break;
            case GS_TOKEN_RBRACE: bc--; break;
            case GS_TOKEN_IDENTIFIER:
            { 
                // gs_println("Parsing attribute: %.*s", token.len, token.text);
                if (!_gs_gui_style_sheet_parse_attribute(ctx, lex, ss, cid_tag, GS_GUI_ELEMENT_COUNT, state, variables))
                {
                    gs_log_warning("Unable to parse attribute");
                    return false;
                }
            } break;
        }
    }

    return true;
}

GS_API_DECL gs_gui_style_sheet_t gs_gui_style_sheet_load_from_file(gs_gui_context_t* ctx, const char* file_path)
{
    // Generate new style sheet based on default element styles
    gs_gui_style_sheet_t ss = gs_default_val(); 
    bool success = true;

    size_t sz = 0;
    char* fd = gs_platform_read_file_contents(file_path, "rb", &sz); 

    if (!fd) {
        gs_log_warning("Cannot load file: %s", file_path);
        return ss;
    }

    ss = gs_gui_style_sheet_load_from_memory(ctx, fd, sz, &success);

    if (success) {
        gs_log_success("Successfully loaded style sheet %s.", file_path);
    }
    else {
        gs_log_warning("Failed to loaded style sheet %s.", file_path);
    }

    gs_free(fd);
    return ss;
}

static bool _gs_gui_style_sheet_parse_variable(gs_gui_context_t* ctx, gs_lexer_t* lex, gs_gui_style_sheet_t* ss, 
        char* name_buf, size_t sz, gs_gui_ss_var_def_t* out)
{
    // Get next token, needs to be identifier 
    gs_token_t token = gs_lexer_next_token(lex);
    if (token.type != GS_TOKEN_IDENTIFIER)
    {
        gs_log_warning("Unidentified token. (Expected variable name after percent sign).");
        gs_token_debug_print(&token);
        return false;
    }

    // Copy name of variable
    memcpy(name_buf, token.text, gs_min(token.len, sz));

    // Expect colon
    token = gs_lexer_next_token(lex);
    if (token.type != GS_TOKEN_COLON)
    {
        gs_log_warning("Syntax error. (Expected colon name after variable name).");
        gs_token_debug_print(&token);
        return false;
    }

    // Now to get variable
    token = gs_lexer_next_token(lex);
    while (gs_lexer_can_lex(lex) && token.type != GS_TOKEN_SEMICOLON)
    {
        switch (token.type)
        {
            case GS_TOKEN_IDENTIFIER:
            {
                if (gs_token_compare_text(&token, "rgb"))
                {
                    token = gs_lexer_next_token(lex);
                    if (token.type != GS_TOKEN_LPAREN)
                    {
                        gs_log_warning("rgb: missing paren (", token.len, token.text);
                        gs_token_debug_print(&token);
                        return false;
                    }

                    out->type = GS_GUI_SS_DEF_COLOR;

                    for (uint32_t i = 0; i < 3; ++i)
                    {
                        token = gs_lexer_next_token(lex);
                        if (token.type != GS_TOKEN_NUMBER)
                        {
                            gs_log_warning("rgb expects numbers", token.len, token.text);
                            gs_token_debug_print(&token);
                            return false;
                        }
                        gs_snprintfc(VAL, 32, "%.*s", token.len, token.text);\
                        uint8_t v = (uint8_t)atoi(VAL);\
                        out->val.color.rgba[i] = v;
                    }
                    out->val.color.rgba[3] = 255;
                }
                else if (gs_token_compare_text(&token, "rgba"))
                {
                    token = gs_lexer_next_token(lex);
                    if (token.type != GS_TOKEN_LPAREN)
                    {
                        gs_log_warning("rgb: missing paren (", token.len, token.text);
                        gs_token_debug_print(&token);
                        return false;
                    }
                    out->type = GS_GUI_SS_DEF_COLOR;
                    for (uint32_t i = 0; i < 4; ++i)
                    {
                        token = gs_lexer_next_token(lex);
                        if (token.type != GS_TOKEN_NUMBER)
                        {
                            gs_log_warning("rgb expects numbers", token.len, token.text);
                            gs_token_debug_print(&token);
                            return false;
                        }
                        gs_snprintfc(VAL, 32, "%.*s", token.len, token.text);\
                        uint8_t v = (uint8_t)atoi(VAL);\
                        out->val.color.rgba[i] = v;
                    }
                }
                else if (gs_token_compare_text(&token, "center"))
                {
                    out->type = GS_GUI_SS_DEF_ENUM;
                    out->val.number = (int32_t)GS_GUI_JUSTIFY_CENTER;
                }
                else if (gs_token_compare_text(&token, "start"))
                {
                    out->type = GS_GUI_SS_DEF_ENUM;
                    out->val.number = (int32_t)GS_GUI_JUSTIFY_START;
                }
                else if (gs_token_compare_text(&token, "end"))
                {
                    out->type = GS_GUI_SS_DEF_ENUM;
                    out->val.number = (int32_t)GS_GUI_JUSTIFY_END;
                }
                else
                {
                    gs_log_warning("Variable value unknown: %.*s", token.len, token.text);
                    gs_token_debug_print(&token);
                    return false;
                }
            } break; 

            case GS_TOKEN_NUMBER:
            {
                gs_snprintfc(VAL, 32, "%.*s", token.len, token.text);\
                int32_t v = (int32_t)atoi(VAL);\
                out->type = GS_GUI_SS_DEF_NUMBER; 
                out->val.number = v;
            } break;

            case GS_TOKEN_STRING:
            {
                gs_snprintfc(VAL, 64, "%.*s", token.len - 2, token.text + 1);\
                out->type = GS_GUI_SS_DEF_STR; 
                memcpy(out->val.str, VAL, 64);
            }
        }

        token = gs_lexer_next_token(lex);
    }

    return true;
}

// Going to require a lot of parsing
GS_API_DECL gs_gui_style_sheet_t gs_gui_style_sheet_load_from_memory(gs_gui_context_t* ctx, const char* fd, size_t sz, bool* sp)
{ 
    // Generate new style sheet based on default element styles
    gs_gui_style_sheet_t ss = gs_default_val(); 
    bool success = true;

	gs_gui_ss_variables_t variables = gs_default_val();

    // Copy all default styles
    GS_GUI_COPY_STYLES(ss.styles, gs_gui_default_style_sheet.styles, GS_GUI_ELEMENT_CONTAINER);
    GS_GUI_COPY_STYLES(ss.styles, gs_gui_default_style_sheet.styles, GS_GUI_ELEMENT_LABEL);
    GS_GUI_COPY_STYLES(ss.styles, gs_gui_default_style_sheet.styles, GS_GUI_ELEMENT_TEXT);
    GS_GUI_COPY_STYLES(ss.styles, gs_gui_default_style_sheet.styles, GS_GUI_ELEMENT_PANEL);
    GS_GUI_COPY_STYLES(ss.styles, gs_gui_default_style_sheet.styles, GS_GUI_ELEMENT_INPUT);
    GS_GUI_COPY_STYLES(ss.styles, gs_gui_default_style_sheet.styles, GS_GUI_ELEMENT_BUTTON);
    GS_GUI_COPY_STYLES(ss.styles, gs_gui_default_style_sheet.styles, GS_GUI_ELEMENT_SCROLL); 
    GS_GUI_COPY_STYLES(ss.styles, gs_gui_default_style_sheet.styles, GS_GUI_ELEMENT_IMAGE); 

#define PARSE_ELEMENT(TYPE, TYPESTR)\
    do {\
        if (!_gs_gui_style_sheet_parse_element(ctx, &lex, &ss, TYPE, &variables))\
        {\
            gs_log_warning("Failed to parse element: %s", TYPESTR);\
            success = false;\
            break;\
        }\
    } while (0)

    // Parse style sheet for styles
    gs_lexer_t lex = gs_lexer_c_ctor(fd);
    while (success && gs_lexer_c_can_lex(&lex))
    {
        gs_token_t token = lex.next_token(&lex);
        switch (token.type)
        {
            case GS_TOKEN_IDENTIFIER:
            {
                if (gs_token_compare_text(&token, "button")) PARSE_ELEMENT(GS_GUI_ELEMENT_BUTTON, "button");
                else if (gs_token_compare_text(&token, "text")) PARSE_ELEMENT(GS_GUI_ELEMENT_TEXT, "text");
                else if (gs_token_compare_text(&token, "label")) PARSE_ELEMENT(GS_GUI_ELEMENT_LABEL, "label");
                else if (gs_token_compare_text(&token, "image")) PARSE_ELEMENT(GS_GUI_ELEMENT_IMAGE, "image");
                else if (gs_token_compare_text(&token, "scroll")) PARSE_ELEMENT(GS_GUI_ELEMENT_SCROLL, "scroll");
                else if (gs_token_compare_text(&token, "panel")) PARSE_ELEMENT(GS_GUI_ELEMENT_PANEL, "panel");
                else if (gs_token_compare_text(&token, "container")) PARSE_ELEMENT(GS_GUI_ELEMENT_CONTAINER, "container");
                else if (gs_token_compare_text(&token, "input")) PARSE_ELEMENT(GS_GUI_ELEMENT_INPUT, "input");
                else
                {
                    gs_log_warning("Unidentified token. (Invalid element type found).");
                    gs_token_debug_print(&token);
                    break;
                }

            } break;

            case GS_TOKEN_PERIOD:
            {
                // Do single class for now
                gs_token_t cls_tag = gs_lexer_next_token(&lex);
                char CLS_TAG[256] = gs_default_val();
                CLS_TAG[0] = '.';
                memcpy(CLS_TAG + 1, cls_tag.text, cls_tag.len);
                uint64_t cls_hash = gs_hash_str64(CLS_TAG);
                if (!_gs_gui_style_sheet_parse_cid_tag(ctx, &lex, &ss, cls_hash, &variables))
                {
                    gs_log_warning("Failed to parse id tag: %s", CLS_TAG);
                    success = false;
                    break;
                }

            } break;

            case GS_TOKEN_HASH:
            {
                gs_token_t id_tag = gs_lexer_next_token(&lex);
                char ID_TAG[256] = gs_default_val();
                ID_TAG[0] = '#';
                memcpy(ID_TAG + 1, id_tag.text, id_tag.len);
                uint64_t id_hash = gs_hash_str64(ID_TAG);
                if (!_gs_gui_style_sheet_parse_cid_tag(ctx, &lex, &ss, id_hash, &variables))
                {
                    gs_log_warning("Failed to parse id tag: %s", ID_TAG);
                    success = false;
                    break;
                } 
            } break;

            case GS_TOKEN_ASTERISK:
            {
                // Save token
                gs_token_t token = gs_lexer_next_token(&lex); gs_lexer_set_token(&lex, token); 
                PARSE_ELEMENT(GS_GUI_ELEMENT_CONTAINER, "* (container)"); gs_lexer_set_token(&lex, token);
                PARSE_ELEMENT(GS_GUI_ELEMENT_TEXT, "* (text)"); gs_lexer_set_token(&lex, token);
                PARSE_ELEMENT(GS_GUI_ELEMENT_LABEL, "* (label)"); gs_lexer_set_token(&lex, token);
                PARSE_ELEMENT(GS_GUI_ELEMENT_IMAGE, "* (image)"); gs_lexer_set_token(&lex, token);
                PARSE_ELEMENT(GS_GUI_ELEMENT_BUTTON, "* (button)"); gs_lexer_set_token(&lex, token);
                PARSE_ELEMENT(GS_GUI_ELEMENT_PANEL, "* (panel)"); gs_lexer_set_token(&lex, token);
                PARSE_ELEMENT(GS_GUI_ELEMENT_INPUT, "* (input)"); gs_lexer_set_token(&lex, token);
                PARSE_ELEMENT(GS_GUI_ELEMENT_SCROLL, "* (scroll)"); 
            } break;

            case GS_TOKEN_DOLLAR:
            { 
                gs_gui_ss_var_def_t variable = gs_default_val(); 
                char variable_name[256] = gs_default_val();
                if (!_gs_gui_style_sheet_parse_variable(ctx, &lex, &ss, variable_name, sizeof(variable_name), &variable))
                {
                    gs_log_warning("Failed to parse variable: %s", variable_name);
                    success = false;
                    break;
                }
                else
                {
                    gs_hash_table_insert(variables.variables, gs_hash_str64(variable_name), variable);
                }

                /*
                typedef enum 
                {
                    GS_GUI_SS_DEF_VALUE = 0x00, 
                    GS_GUI_SS_DEF_COLOR,
                    GS_GUI_SS_DEF_STR
                } gs_gui_ss_var_def_type;

                typedef struct
                {
                    gs_gui_ss_var_def_type type;
                    union {
                        int32_t value;
                        gs_color_t color;
                        char str[64];
                    } val;
                } gs_gui_ss_var_def_t;
                */

            } break; 
        }
    } 

    if (!sp)
    {
        if (success) {
            gs_log_success("Successfully loaded style sheet from memory.");
        }
        else {
            gs_log_warning("Failed to loaded style sheet from memory.");
        }
    }
    else
    {
        *sp = success;
    }

    if (variables.variables) gs_hash_table_free(variables.variables);

    return ss;
}

#endif // GS_GUI_IMPL 
#endif // GS_GUI_H















