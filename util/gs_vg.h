
/*================================================================
    * Copyright: 2022 John Jackson
    * GSPhysics: Simple vector graphics rendering
    * File: gs_vg.h
    All Rights Reserved
=================================================================*/

#ifndef GS_VG_H
#define GS_VG_H

/*
    USAGE: (IMPORTANT)

    =================================================================================================================

    Before including, define the gunslinger physics implementation like this:

        #define GS_VG_IMPL

    in EXACTLY ONE C or C++ file that includes this header, BEFORE the
    include, like this:

        #define GS_VG_IMPL
        #include "gs_vg.h"

    All other files should just #include "gs_vg.h" without the #define.

    MUST include "gs.h" and declare GS_IMPL BEFORE this file, since this file relies on that:

        #define GS_IMPL
        #include "gs.h"

        #define GS_VG_IMPL
        #include "gs_vg.h"
 + 
    ================================================================================================================
*/

//==== Interface ====//

#define GS_VG_MITER_ANGLE_MIN  10.f
#define GS_VG_ROUND_ANGLE_MIN  10.f
#define GS_VG_SEGMENT_MAX      24

enum 
{
    GS_VG_JOINT_MITER = 0x00,
    GS_VG_JOINT_BEVEL, 
    GS_VG_JOINT_ROUND 
};

enum 
{
    GS_VG_END_BUTT = 0x00,
    GS_VG_END_JOINT,
    GS_VG_END_SQUARE,
    GS_VG_END_ROUND 
};

enum 
{
    GS_VG_WINDING_CW = 0x00,
    GS_VG_WINDING_CCW
};

enum 
{
    GS_VG_STROKE = 0x00,
    GS_VG_FILL
}; 

typedef struct
{
    gs_vec2 position;
    gs_color_t color;
    float thickness;
} gs_vg_point_t;

GS_API_DECL gs_vg_point_t gs_vg_point_create(gs_vec2 position, gs_color_t color, float thickness);

// Subpaths
typedef struct
{
    uint16_t start; 
    uint16_t count;
} gs_vg_path_t;

typedef struct 
{
    int16_t mode;    // Stroke/Fill
    int16_t joint;   // Joint style
    int16_t end;     // End style
    float thickness;
    gs_color_t color;
    uint16_t anti_alias;
    float aa_scale;
} gs_vg_paint_t; 

typedef struct
{
    gs_vg_point_t a;
    gs_vg_point_t b;
} gs_vg_line_seg_t;

typedef struct
{
	b32 intersected;
	gs_vg_point_t point;
} gs_vg_line_seg_intersection_t;

typedef struct gs_vg_poly_seg_t 
{
	gs_vg_line_seg_t center; 
	gs_vg_line_seg_t edge1; 
	gs_vg_line_seg_t edge2;
} gs_vg_poly_seg_t;

typedef struct
{
    gs_vec2 xform;
    gs_dyn_array(gs_vg_point_t) points;
    gs_dyn_array(gs_vg_path_t) paths;
    gs_dyn_array(gs_vg_poly_seg_t) segments;
    gs_vg_paint_t paint;
} gs_vg_state_t;

typedef struct
{ 
    gs_immediate_draw_t gsi;
    gs_vg_state_t state;
} gs_vg_ctx_t;

GS_API_DECL gs_vg_ctx_t gs_vg_ctx_new();
GS_API_DECL void gsvg_frame_begin(gs_vg_ctx_t* ctx, uint32_t ws, uint32_t wy);
GS_API_DECL void gsvg_frame_end(gs_vg_ctx_t* ctx);
GS_API_DECL void gsvg_path_begin(gs_vg_ctx_t* ctx);
GS_API_DECL void gsvg_path_stroke(gs_vg_ctx_t* ctx); 
GS_API_DECL void gsvg_path_moveto(gs_vg_ctx_t* ctx, float x, float y);
GS_API_DECL void gsvg_path_lineto(gs_vg_ctx_t* ctx, float x, float y);
GS_API_DECL void gsvg_path_cbezierto(gs_vg_ctx_t* ctx, float x0, float y0, 
        float x1, float y1, float x2, float y2);
GS_API_DECL void gsvg_path_qbezierto(gs_vg_ctx_t* ctx, float x0, float y0, float x1, float y1);
GS_API_DECL void gsvg_path_arcto(gs_vg_ctx_t* ctx, float x0, float y0, float x1, float y1, float radius);
GS_API_DECL void gsvg_path_arc(gs_vg_ctx_t* ctx, float cx, float cy, float radius, float angle_start, float angle_end);
GS_API_DECL void gsvg_paint(gs_vg_ctx_t* ctx, gs_vg_paint_t paint);
GS_API_DECL void gsvg_render(gs_vg_ctx_t* ctx, gs_command_buffer_t* cb, uint32_t vw, uint32_t vh); 
GS_API_DECL void gsvg_renderpass_submit(gs_vg_ctx_t* ctx, gs_command_buffer_t* cb, gs_vec2 fbs, gs_color_t c); 

//==== Implementation ====//

#ifdef GS_VG_IMPL 

// Forward decls
GS_API_DECL void _gsvg_path_stroke_impl(gs_vg_ctx_t* ctx, gs_vg_path_t* path); 
GS_API_DECL void _gsvg_path_fill_impl(gs_vg_ctx_t* ctx);

// Utils
static bool gs_vg_point_equals(gs_vg_point_t* p0, gs_vg_point_t* p1)
{
    return gs_vec2_equals(p0->position, p1->position);
}

static void gsvg_add_point(gs_vg_ctx_t* ctx, gs_vg_point_t point)
{
    gs_vg_state_t* state = &ctx->state;
    gs_vg_path_t* path = &state->paths[gs_dyn_array_size(state->paths) - 1];

    if (gs_dyn_array_empty(state->points))
    {
        gs_dyn_array_push(state->points, point);
        path->count++;
    }
    // Determine whether or not previous point equals this one
    else
    {
        gs_vg_point_t* p = &((state->points)[gs_dyn_array_size(state->points) - 1]);
        if (!gs_vg_point_equals(p, &point))
        {
            gs_dyn_array_push(state->points, point);
            path->count++;
        }
    } 
}

GS_API_DECL gs_vg_ctx_t gs_vg_ctx_new()
{
    gs_vg_ctx_t ctx = gs_default_val();
    ctx.gsi = gs_immediate_draw_new();
    return ctx;
}

GS_API_DECL void gsvg_frame_begin(gs_vg_ctx_t* ctx, uint32_t vw, uint32_t vh)
{
    gs_dyn_array_clear(ctx->state.points);
    gs_dyn_array_clear(ctx->state.paths);
	gsi_camera2D(&ctx->gsi, vw, vh);
	gsi_blend_enabled(&ctx->gsi, true);
    gsi_begin(&ctx->gsi, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    gsi_tc2fv(&ctx->gsi, gs_v2s(0.f)); 
    ctx->state.paint.color = GS_COLOR_WHITE;
    ctx->state.paint.thickness = 1.f;
    ctx->state.paint.end = GS_VG_END_BUTT;
    ctx->state.paint.joint = GS_VG_JOINT_MITER;
    ctx->state.paint.anti_alias = true;
    ctx->state.paint.aa_scale = 0.5f;
}

GS_API_DECL void gsvg_frame_end(gs_vg_ctx_t* ctx)
{
    // Not sure what to do here...
} 

GS_API_DECL void gsvg_paint(gs_vg_ctx_t* ctx, gs_vg_paint_t paint)
{
    ctx->state.paint = paint;
}

static bool _gsvg_path_end_impl(gs_vg_ctx_t* ctx)
{
    gs_vg_state_t* state = &ctx->state;
    gs_vg_path_t* path = &state->paths[gs_dyn_array_size(state->paths) - 1];
    return path->count;
}

GS_API_DECL void gsvg_path_begin(gs_vg_ctx_t* ctx)
{
    gs_vg_state_t* state = &ctx->state;

    if (gs_dyn_array_empty(ctx->state.paths))
    {
        gs_vg_path_t path = gs_default_val();
        gs_dyn_array_push(state->paths, path);
    }
    else
    {
        // End previous path, if points were written
        if ( _gsvg_path_end_impl(ctx))
        {
            gs_vg_path_t path = gs_default_val();
            path.start = gs_dyn_array_size(state->points);
            gs_dyn_array_push(state->paths, path);
        }
    } 
}

GS_API_DECL void gsvg_path_fill(gs_vg_ctx_t* ctx)
{
    gs_vg_state_t* state = &ctx->state;

    // End previous path
    _gsvg_path_end_impl(ctx); 

    _gsvg_path_fill_impl(ctx);

    // Just use naive ear-clipping for now, don't respect holes

    gs_dyn_array_clear(state->points);
    gs_dyn_array_clear(state->paths);
    gs_dyn_array_clear(state->segments);
}

GS_API_DECL void gsvg_path_stroke(gs_vg_ctx_t* ctx) 
{
    // End previous path
    // Iterate through all sub-paths and stroke
    gs_vg_state_t* state = &ctx->state;

    // End previous path
    _gsvg_path_end_impl(ctx);

    // For each subpath, stroke
    for (uint32_t i = 0; i < gs_dyn_array_size(state->paths); ++i)
    {
        gs_vg_path_t* path = &state->paths[i]; 
        _gsvg_path_stroke_impl(ctx, path);
        gs_dyn_array_clear(state->segments);
    } 

    // Clear previous paths, segments, and points
    gs_dyn_array_clear(state->points);
    gs_dyn_array_clear(state->paths);
    gs_dyn_array_clear(state->segments);
}

GS_API_DECL void gsvg_path_close(gs_vg_ctx_t* ctx)
{
    ctx->state.paint.end = GS_VG_END_JOINT;
}

GS_API_DECL void gsvg_path_moveto(gs_vg_ctx_t* ctx, float x, float y)
{
    gsvg_path_begin(ctx);
    ctx->state.xform = gs_v2(x, y);
}

GS_API_DECL void gsvg_path_lineto(gs_vg_ctx_t* ctx, float x, float y)
{
    gs_vg_state_t* state = &ctx->state;
    gs_vg_paint_t* paint = &state->paint;
    gs_vg_point_t p0 = gs_vg_point_create(state->xform, paint->color, paint->thickness);
    gs_vg_point_t p1 = gs_vg_point_create(gs_v2(x, y), paint->color, paint->thickness);
    gsvg_add_point(ctx, p0);
    gsvg_add_point(ctx, p1);
    state->xform = p1.position;
}

GS_API_DECL void gsvg_path_qbezierto(gs_vg_ctx_t* ctx, float x1, float y1, float x2, float y2)
{
    gs_vg_state_t* state = &ctx->state;
    gs_vg_paint_t* paint = &state->paint;
    float x0 = state->xform.x;
    float y0 = state->xform.y; 
    gs_vg_point_t p = gs_default_val();

    uint32_t num_segs = GS_VG_SEGMENT_MAX;
    float step = 1.0f / (float)num_segs;
    for (float s = 0.f; s < 1.f; s += step)
    {
        float xa = gs_interp_linear(x0, x1, s); 
        float ya = gs_interp_linear(y0, y1, s); 
        float xb = gs_interp_linear(x1, x2, s); 
        float yb = gs_interp_linear(y1, y2, s); 

        float x = gs_interp_linear(xa, xb, s);
        float y = gs_interp_linear(ya, yb, s);
        p = gs_vg_point_create(gs_v2(x, y), paint->color, paint->thickness);
        gsvg_add_point(ctx, p);
    } 

    state->xform = p.position;
}

GS_API_DECL void gsvg_path_cbezierto(gs_vg_ctx_t* ctx, float x1, float y1, 
        float x2, float y2, float x3, float y3)
{
    gs_vg_state_t* state = &ctx->state;
    gs_vg_paint_t* paint = &state->paint;
    float x0 = state->xform.x;
    float y0 = state->xform.y;

    gs_vg_point_t p = gs_default_val();

    uint32_t num_segs = GS_VG_SEGMENT_MAX;
    float step = 1.0f / (float)num_segs;
    for (float s = 0.f; s < 1.f; s += step)
    {
        float xa = gs_interp_linear(x0, x1, s); 
        float ya = gs_interp_linear(y0, y1, s); 
        float xb = gs_interp_linear(x1, x2, s); 
        float yb = gs_interp_linear(y1, y2, s); 
        float xc = gs_interp_linear(x2, x3, s); 
        float yc = gs_interp_linear(y2, y3, s); 
        
        float xm = gs_interp_linear(xa, xb, s);
        float ym = gs_interp_linear(ya, yb, s);
        float xn = gs_interp_linear(xb, xc, s);
        float yn = gs_interp_linear(yb, yc, s);

        float x = gs_interp_linear(xm, xn, s);
        float y = gs_interp_linear(ym, yn, s);
        p = gs_vg_point_create(gs_v2(x, y), paint->color, paint->thickness);
        gsvg_add_point(ctx, p);
    } 

    state->xform = p.position;
}

void _gsvg_path_arc_impl(gs_vg_ctx_t* ctx, float cx, float cy, float r, float sa, float ea)
{
    gs_vg_state_t* state = &ctx->state;
    gs_vg_paint_t* paint = &state->paint;

	if (sa > ea) {
		f32 tmp = sa;
		sa = ea;
		ea = tmp;
	}
	f32 diff = ea - sa;
	if (fabsf(diff) <= 0.001f) {
		return;
	} 

    gs_vec2 origin = gs_v2(cx, cy);
    gs_vg_point_t p = gs_default_val();
    uint32_t num_segments = GS_VG_SEGMENT_MAX;
	f32 step = num_segments < 5 ? diff / 5.f : diff / (f32)num_segments;

	// Need CW vs. CCW here...

    for (f32 i = sa; i <= ea; i += step) 
    {
        f32 a = gs_deg2rad(i);
        gs_vec2 pos = gs_vec2_add(origin, gs_vec2_scale(gs_v2(cos(a), sin(a)), r));
        p = gs_vg_point_create(pos, paint->color, paint->thickness);
        gsvg_add_point(ctx, p);
    }

    // Push last angle on as well
    {
        f32 a = gs_deg2rad(ea);
        gs_vec2 pos = gs_vec2_add(origin, gs_vec2_scale(gs_v2(cos(a), sin(a)), r));
        p = gs_vg_point_create(pos, paint->color, paint->thickness);
        gsvg_add_point(ctx, p);
    }

}

GS_API_DECL void gsvg_path_arcto(gs_vg_ctx_t* ctx, float x1, float y1, float x2, float y2, float radius)
{ 
    gs_vg_state_t* state = &ctx->state;
    gs_vg_paint_t* paint = &state->paint;
    float x0 = state->xform.x, y0 = state->xform.y;
    gs_vec2 d0 = gs_v2s(0.f);
    gs_vec2 d1 = gs_v2s(0.f);
    gs_vec2 c = gs_v2s(0.f);
    float a = 0.f, d = 0.f, a0 = 0.f, a1 = 0.f;
    int16_t dir = GS_VG_WINDING_CW; 

    gs_vg_point_t p = gs_vg_point_create(state->xform, paint->color, paint->thickness);
    gsvg_add_point(ctx, p);

    // CW vs. CCW depending on points
    d0 = gs_vec2_norm(gs_vec2_sub(gs_v2(x0, y0), gs_v2(x1, y1)));
    d1 = gs_vec2_norm(gs_vec2_sub(gs_v2(x2, y2), gs_v2(x1, y1)));
    a = acosf(gs_vec2_dot(d0, d1)); 
    d = radius / tanf(a * 0.5f);

    if (d > 10000.f)
    {
        gsvg_path_lineto(ctx, x1, y1);
        return;
    } 

    if (gs_vec2_cross(d0, d1) > 0.f)
    {
        c.x = x1 + d0.x * d + d0.y * radius;
        c.y = y1 + d0.y * d - d0.x * radius;
        a0 = atan2f(d0.x, -d0.y);
        a1 = atan2f(-d1.x, d1.y);
        dir = GS_VG_WINDING_CW;
    }
    else
    {
        c.x = x1 + d0.x * d - d0.y * radius;
        c.y = y1 + d0.y * d + d0.x * radius;
        a0 = atan2f(-d0.x, d0.y);
        a1 = atan2f(d1.x, -d1.y);
        dir = GS_VG_WINDING_CCW;
    }

    _gsvg_path_arc_impl(ctx, c.x, c.y, radius, gs_rad2deg(a0), gs_rad2deg(a1));
    const gs_vg_point_t* last = &state->points[gs_dyn_array_size(state->points) - 1];
    state->xform = last->position;
} 

GS_API_DECL void gsvg_path_arc(gs_vg_ctx_t* ctx, float cx, float cy, float r, float sa, float ea)
{
    gsvg_path_moveto(ctx, cx, cy); 
    _gsvg_path_arc_impl(ctx, cx, cy, r, sa, ea);
    const gs_vg_point_t* last = &ctx->state.points[gs_dyn_array_size(ctx->state.points) - 1];
    gsvg_path_moveto(ctx, last->position.x, last->position.y); 
}

GS_API_DECL void gsvg_render(gs_vg_ctx_t* ctx, gs_command_buffer_t* cb, uint32_t vw, uint32_t vh)
{ 
    gsi_draw(&ctx->gsi, cb);
} 

GS_API_DECL void gsvg_renderpass_submit(gs_vg_ctx_t* ctx, gs_command_buffer_t* cb, gs_vec2 fbs, gs_color_t c)
{
	gs_graphics_clear_action_t action = gs_default_val();
	action.color[0] = (float)c.r / 255.f; 
	action.color[1] = (float)c.g / 255.f; 
	action.color[2] = (float)c.b / 255.f; 
	action.color[3] = (float)c.a / 255.f;
	gs_graphics_clear_desc_t clear = gs_default_val();
	clear.actions = &action;
    gs_graphics_renderpass_begin(cb, (gs_handle(gs_graphics_renderpass_t)){0});
    {
        gs_graphics_clear(cb, &clear);
        gs_graphics_set_viewport(cb, 0, 0, (uint32_t)fbs.x,(uint32_t)fbs.y);
        gsvg_render(ctx, cb, (uint32_t)fbs.x, (uint32_t)fbs.y);
    }
    gs_graphics_renderpass_end(cb);
}

#define miter_min_angle gs_deg2rad(GS_VG_MITER_ANGLE_MIN)
#define round_min_angle gs_deg2rad(GS_VG_ROUND_ANGLE_MIN)

gs_vg_point_t gs_vg_point_create(gs_vec2 position, gs_color_t color, float thickness)
{
    gs_vg_point_t pt = gs_default_val();
    pt.position = position;
    pt.color = color;
    pt.thickness = thickness;
    return pt;
}

gs_vg_point_t gs_vg_point_add(gs_vg_point_t p, gs_vec2 os)
{
	gs_vg_point_t pp = p;
	pp.position = gs_vec2_add(pp.position, os);
	return pp;
}

gs_vg_point_t gs_vg_point_sub(gs_vg_point_t p, gs_vec2 os)
{
	return gs_vg_point_add(p, gs_vec2_scale(os, -1.f));
}

gs_vg_line_seg_t 
gs_vg_line_seg_add(gs_vg_line_seg_t l, gs_vec2 os) 
{
    gs_vg_line_seg_t ls = gs_default_val();
    ls.a = gs_vg_point_add(l.a, os);
    ls.b = gs_vg_point_add(l.b, os);
    return ls;
}

gs_vg_line_seg_t 
gs_vg_line_seg_sub(gs_vg_line_seg_t l, gs_vec2 os) 
{
    gs_vg_line_seg_t ls = gs_default_val();
    ls.a = gs_vg_point_sub(l.a, os);
    ls.b = gs_vg_point_sub(l.b, os);
    return ls;
}

gs_vec2 gs_vg_line_seg_dir(gs_vg_line_seg_t l, b32 normalized) 
{
	gs_vec2 vec = gs_vec2_sub(l.b.position, l.a.position);
	return normalized ? gs_vec2_norm(vec) : vec;
}

// Not sure about this one...
gs_vec2 gs_vg_line_seg_normal(gs_vg_line_seg_t l) 
{
	gs_vec2 dir = gs_vg_line_seg_dir(l, true);

	// return the direction vector
	// rotated by 90 degrees counter-clockwise
	return (gs_vec2){-dir.y, dir.x};
}

gs_vg_line_seg_intersection_t null_intersection()
{
    gs_vg_line_seg_intersection_t ls = gs_default_val();
    return ls;
}

gs_vg_line_seg_intersection_t gs_vg_line_seg_intersection_create(gs_vg_point_t pt, b32 intersected)
{
    gs_vg_line_seg_intersection_t ls = gs_default_val();
    ls.intersected = intersected;
    ls.point = pt;
    return ls;
} 

gs_vg_line_seg_intersection_t 
gs_vg_line_seg_intersection(gs_vg_line_seg_t a, gs_vg_line_seg_t b, b32 infiniteLines) 
{
	// calculate un-normalized direction vectors
	gs_vec2 r = gs_vg_line_seg_dir(a, false);
	gs_vec2 s = gs_vg_line_seg_dir(b, false);

	gs_vec2 origin_dist = gs_vec2_sub(b.a.position, a.a.position);

	f32 u_numerator = gs_vec2_cross(origin_dist, r);
	f32 denominator = gs_vec2_cross(r, s);

	// The lines are parallel
	if (fabsf(denominator) < 0.0001f) {
		return null_intersection();
	}

	// solve the intersection positions
	f32 u = u_numerator / denominator;
	f32 t = gs_vec2_cross(origin_dist, s) / denominator;

	if (!infiniteLines && (t < 0 || t > 1 || u < 0 || u > 1)) {
		// the intersection lies outside of the line segments
		return null_intersection();
	}

	// calculate the intersection point
	// a.a + r * t;
	return gs_vg_line_seg_intersection_create(gs_vg_point_add(a.a, gs_vec2_scale(r, t)), true);
}

gs_vg_poly_seg_t gs_vg_poly_seg_create(gs_vg_line_seg_t center) 
{
	gs_vg_poly_seg_t p = {0};
	p.center = center;

	// Need to create these manually
	gs_vec2 n = gs_vg_line_seg_normal(center);

	// Edge1.a will be a line segment from center + norm * a;
	p.edge1.a = gs_vg_point_add(center.a, gs_vec2_scale(n, center.a.thickness));
	p.edge1.b = gs_vg_point_add(center.b, gs_vec2_scale(n, center.b.thickness));

	// Edge1.b will be a line segment from center - norm * a;
	p.edge2.a = gs_vg_point_sub(center.a, gs_vec2_scale(n, center.a.thickness));
	p.edge2.b = gs_vg_point_sub(center.b, gs_vec2_scale(n, center.b.thickness));

	return p;
}

void _gsvg_path_stroke_joint_impl(gs_vg_ctx_t* ctx, gs_vg_poly_seg_t seg1, gs_vg_poly_seg_t seg2,
	                                  int16_t joint_style, gs_vg_point_t* end1, gs_vg_point_t* end2,
	                                  gs_vg_point_t* next_start1, gs_vg_point_t* next_start2,
	                                  b32 allow_overlap); 
GS_API_DECL void _gsvg_path_stroke_triangle_fan_impl(gs_vg_ctx_t* ctx, gs_vg_point_t conenct_to, 
        gs_vg_point_t origin, gs_vg_point_t start, gs_vg_point_t end, bool clockwise);

GS_API_DECL void _gsvg_path_stroke_impl(gs_vg_ctx_t* ctx, gs_vg_path_t* path)
{ 
    gs_vg_state_t* state = &ctx->state;
    gs_vg_paint_t* paint = &state->paint;
    int16_t end_cap_style = paint->end;
    int16_t joint_style = paint->joint;
    bool allow_overlap = false;

	for (u32 i = path->start; i + 1 < path->start + path->count; ++i) 
	{
		gs_vg_point_t point1 = state->points[i];
		gs_vg_point_t point2 = state->points[i + 1];

		// to avoid division-by-zero errors,
		// only create a line segment for non-identical points
		if (!gs_vec2_equal(point1.position, point2.position)) 
		{
            gs_vg_line_seg_t center = gs_default_val(); 
            center.a = point1;
            center.b = point2;
			gs_vg_poly_seg_t ps = gs_vg_poly_seg_create(center); 
			gs_dyn_array_push(state->segments, ps);
		}
	}

	if (end_cap_style == GS_VG_END_JOINT) {
		// create a connecting segment from the last to the first point

		gs_vg_point_t point1 = state->points[path->start + path->count - 1];
		gs_vg_point_t point2 = state->points[path->start];

		// to avoid division-by-zero errors,
		// only create a line segment for non-identical points
		if (!gs_vec2_equal(point1.position, point2.position)) 
		{
            gs_vg_line_seg_t center = gs_default_val(); 
            center.a = point1;
            center.b = point2;
			gs_vg_poly_seg_t ps = gs_vg_poly_seg_create(center); 
			gs_dyn_array_push(state->segments, ps);
		}
	}

	if (gs_dyn_array_empty(state->segments)) {
		// handle the case of insufficient input points
		return;
	}

	gs_vg_point_t next_start1 = {0};
	gs_vg_point_t next_start2 = {0};
	gs_vg_point_t start1 = {0};
	gs_vg_point_t start2 = {0};
	gs_vg_point_t end1 = {0};
	gs_vg_point_t end2 = {0};

	// calculate the path's global start and end points
	gs_vg_poly_seg_t first_segment = state->segments[0];
	gs_vg_poly_seg_t last_segment = state->segments[gs_dyn_array_size(state->segments) - 1];

	gs_vg_point_t path_start1 = first_segment.edge1.a;
	gs_vg_point_t path_start2 = first_segment.edge2.a;
	gs_vg_point_t path_end1 = last_segment.edge1.b;
	gs_vg_point_t path_end2 = last_segment.edge2.b;

	// handle different end cap styles
	if (end_cap_style == GS_VG_END_SQUARE) 
    {
		path_start1 = gs_vg_point_sub(path_start1, 
                gs_vec2_scale(gs_vg_line_seg_dir(first_segment.edge1, true), first_segment.edge1.a.thickness));

		path_start2 = gs_vg_point_sub(path_start2, 
                gs_vec2_scale(gs_vg_line_seg_dir(first_segment.edge2, true), first_segment.edge1.a.thickness));

		path_end1 = gs_vg_point_add(path_end1, 
                gs_vec2_scale(gs_vg_line_seg_dir(last_segment.edge1, true), last_segment.edge1.b.thickness));

		path_end2 = gs_vg_point_add(path_end2, 
                gs_vec2_scale(gs_vg_line_seg_dir(last_segment.edge2, true), last_segment.edge1.b.thickness));

	} 
    else if (end_cap_style == GS_VG_END_ROUND) 
    { 
		// draw half circle end caps
		_gsvg_path_stroke_triangle_fan_impl(ctx, first_segment.center.a, first_segment.center.a,
		                  first_segment.edge1.a, first_segment.edge2.a, false);

		_gsvg_path_stroke_triangle_fan_impl(ctx, last_segment.center.b, last_segment.center.b,
		                  last_segment.edge1.b, last_segment.edge2.b, true);

	} 
    else if (end_cap_style == GS_VG_END_JOINT) 
    {
		// join the last (connecting) segment and the first segment
		_gsvg_path_stroke_joint_impl(ctx, last_segment, first_segment, joint_style,
		            &path_end1, &path_end2, &path_start1, &path_start2, false);
	}

	// generate mesh data for path segments
	for (u32 i = 0; i < gs_dyn_array_size(state->segments); ++i) {

		gs_vg_poly_seg_t segment = state->segments[i];

		// calculate start
		if (i == 0) 
        {
			// this is the first segment
			start1 = path_start1;
			start2 = path_start2;
		}

		if (i + 1 == gs_dyn_array_size(state->segments)) 
        {
			// this is the last segment
			end1 = path_end1;
			end2 = path_end2;
		} 
        else 
        {
			_gsvg_path_stroke_joint_impl(ctx, segment, state->segments[i + 1], joint_style,
			            &end1, &end2, &next_start1, &next_start2, allow_overlap);
		} 

        float anti_alias_scl = paint->aa_scale;
		if (paint->anti_alias) 
        { 
			f32 s_thick = gs_max(start1.thickness, start2.thickness); 
			f32 e_thick = gs_max(end1.thickness, end2.thickness); 

			// Push back verts for anti-aliasing as well...somehow
			gs_vec2 sn = gs_vec2_norm(gs_vec2_sub(start2.position, start1.position));
			gs_vec2 en = gs_vec2_norm(gs_vec2_sub(end2.position, end1.position));
			const f32 sl = (gs_vec2_len(gs_vec2_sub(start2.position, start1.position)) / (s_thick)) * anti_alias_scl;
			const f32 el = (gs_vec2_len(gs_vec2_sub(end2.position, end1.position)) / (e_thick)) * anti_alias_scl;

            gs_color_t s1_col = gs_color_alpha(start1.color, 0);
            gs_color_t s2_col = gs_color_alpha(start2.color, 0);
            gs_color_t e1_col = gs_color_alpha(end1.color, 0);
            gs_color_t e2_col = gs_color_alpha(end2.color, 0);

			gs_vg_point_t s1 = gs_vg_point_sub(start1, gs_vec2_scale(sn, sl));
			gs_vg_point_t s2 = gs_vg_point_add(start2, gs_vec2_scale(sn, sl));
			gs_vg_point_t e1 = gs_vg_point_sub(end1, gs_vec2_scale(en, el));
			gs_vg_point_t e2 = gs_vg_point_add(end2, gs_vec2_scale(en, el)); 

            gsi_c4ubv(&ctx->gsi, start1.color); gsi_v2fv(&ctx->gsi, start1.position);
            gsi_c4ubv(&ctx->gsi, s1_col); gsi_v2fv(&ctx->gsi, s1.position);
            gsi_c4ubv(&ctx->gsi, e1_col); gsi_v2fv(&ctx->gsi, e1.position);

            gsi_c4ubv(&ctx->gsi, e1_col); gsi_v2fv(&ctx->gsi, e1.position);
            gsi_c4ubv(&ctx->gsi, end1.color); gsi_v2fv(&ctx->gsi, end1.position);
            gsi_c4ubv(&ctx->gsi, start1.color); gsi_v2fv(&ctx->gsi, start1.position);

            gsi_c4ubv(&ctx->gsi, s2_col); gsi_v2fv(&ctx->gsi, s2.position);
            gsi_c4ubv(&ctx->gsi, start2.color); gsi_v2fv(&ctx->gsi, start2.position);
            gsi_c4ubv(&ctx->gsi, e2_col); gsi_v2fv(&ctx->gsi, e2.position);

            gsi_c4ubv(&ctx->gsi, e2_col); gsi_v2fv(&ctx->gsi, e2.position);
            gsi_c4ubv(&ctx->gsi, end2.color); gsi_v2fv(&ctx->gsi, end2.position);
            gsi_c4ubv(&ctx->gsi, start2.color); gsi_v2fv(&ctx->gsi, start2.position);

			// If we're at beginning and not end_cap_joint, then we need to anti-alias edge
			if (i == 0 && (end_cap_style == GS_VG_END_SQUARE 
				|| end_cap_style == GS_VG_END_BUTT) )
			{
				gs_vg_point_t s1s = gs_vg_point_sub(start1, gs_vec2_scale(sn, sl * 0.5f));
				gs_vg_point_t s2s = gs_vg_point_add(start2, gs_vec2_scale(sn, sl * 0.5f));

				gs_vec2 snc = gs_v2(-sn.y, sn.x); 
				gs_vg_point_t s1c = gs_vg_point_add(s1s, gs_vec2_scale(snc, sl));
				gs_vg_point_t s2c = gs_vg_point_add(s2s, gs_vec2_scale(snc, sl));

				gsi_c4ubv(&ctx->gsi, s1_col); gsi_v2fv(&ctx->gsi, s1c.position);
				gsi_c4ubv(&ctx->gsi, s1s.color); gsi_v2fv(&ctx->gsi, s1s.position);
				gsi_c4ubv(&ctx->gsi, s2_col); gsi_v2fv(&ctx->gsi, s2c.position);

				gsi_c4ubv(&ctx->gsi, s2_col); gsi_v2fv(&ctx->gsi, s2c.position);
				gsi_c4ubv(&ctx->gsi, s2s.color); gsi_v2fv(&ctx->gsi, s2s.position);
				gsi_c4ubv(&ctx->gsi, s1s.color); gsi_v2fv(&ctx->gsi, s1s.position);
			}

			// If we're at end and not end_cap_joint, then we need to anti-alias edge
			if ((i + 1) == gs_dyn_array_size(state->segments) 
				&& (end_cap_style == GS_VG_END_SQUARE 
				|| end_cap_style == GS_VG_END_BUTT) )
			{ 
				gs_vg_point_t e1s = gs_vg_point_sub(end1, gs_vec2_scale(en, el * 0.5f));
				gs_vg_point_t e2s = gs_vg_point_add(end2, gs_vec2_scale(en, el * 0.5f));

				gs_vec2 enc = gs_v2(-en.y, en.x); 
				gs_vg_point_t e1c = gs_vg_point_add(e1s, gs_vec2_scale(enc, el));
				gs_vg_point_t e2c = gs_vg_point_add(e2s, gs_vec2_scale(enc, el));

				gsi_c4ubv(&ctx->gsi, e1_col); gsi_v2fv(&ctx->gsi, e1c.position);
				gsi_c4ubv(&ctx->gsi, e1s.color); gsi_v2fv(&ctx->gsi, e1s.position);
				gsi_c4ubv(&ctx->gsi, e2_col); gsi_v2fv(&ctx->gsi, e2c.position);

				gsi_c4ubv(&ctx->gsi, e2_col); gsi_v2fv(&ctx->gsi, e2c.position);
				gsi_c4ubv(&ctx->gsi, e2s.color); gsi_v2fv(&ctx->gsi, e2s.position);
				gsi_c4ubv(&ctx->gsi, e1s.color); gsi_v2fv(&ctx->gsi, e1s.position);
			}
		} 

		// Push back verts
        gsi_c4ubv(&ctx->gsi, start1.color); gsi_v2fv(&ctx->gsi, start1.position); 
        gsi_c4ubv(&ctx->gsi, start2.color); gsi_v2fv(&ctx->gsi, start2.position); 
        gsi_c4ubv(&ctx->gsi, end1.color); gsi_v2fv(&ctx->gsi, end1.position);

        gsi_c4ubv(&ctx->gsi, end1.color); gsi_v2fv(&ctx->gsi, end1.position); 
        gsi_c4ubv(&ctx->gsi, start2.color); gsi_v2fv(&ctx->gsi, start2.position); 
        gsi_c4ubv(&ctx->gsi, end2.color); gsi_v2fv(&ctx->gsi, end2.position);

		start1 = next_start1;
		start2 = next_start2;
	} 
}

GS_API_DECL void _gsvg_path_stroke_triangle_fan_impl(gs_vg_ctx_t* ctx, gs_vg_point_t connect_to, 
        gs_vg_point_t origin, gs_vg_point_t start, gs_vg_point_t end, bool clockwise)
{
    gs_vg_paint_t* paint = &ctx->state.paint;
	gs_vg_point_t point1 = gs_vg_point_sub(start, origin.position);
	gs_vg_point_t point2 = gs_vg_point_sub(end, origin.position);

	// calculate the angle between the two points
	f32 angle1 = atan2(point1.position.y, point1.position.x);
	f32 angle2 = atan2(point2.position.y, point2.position.x);

	// ensure the outer angle is calculated
	if (clockwise) 
    {
		if (angle2 > angle1) {
			angle2 = angle2 - 2.0 * GS_PI;
		}
	} 
    else 
    {
		if (angle1 > angle2) {
			angle1 = angle1 - 2.0 * GS_PI;
		}
	}

	f32 joint_angle = angle2 - angle1;

	// calculate the amount of triangles to use for the joint
	s32 num_triangles = (s32)gs_max(1, (float)floor(fabsf(joint_angle) / round_min_angle));

	// calculate the angle of each triangle
	f32 tri_angle = joint_angle / (f32)num_triangles;

	gs_vg_point_t start_point = start;
	gs_vg_point_t end_point = end;

	for (s32 t = 0; t < num_triangles; t++) 
	{
		if (t + 1 == num_triangles) 
        {
			// it's the last triangle - ensure it perfectly
			// connects to the next line
			end_point = end;
		} else 
        {
			f32 rot = (t + 1) * tri_angle;

			// rotate the original point around the origin
			end_point.position.x = cos(rot) * point1.position.x - sin(rot) * point1.position.y;
			end_point.position.y = sin(rot) * point1.position.x + cos(rot) * point1.position.y;

			// re-add the rotation origin to the target point
			end_point = gs_vg_point_add(end_point, origin.position);
		}

        // Emit verts directly into gsi
        gsi_c4ubv(&ctx->gsi, start_point.color); gsi_v2fv(&ctx->gsi, start_point.position); 
        gsi_c4ubv(&ctx->gsi, end_point.color); gsi_v2fv(&ctx->gsi, end_point.position); 
        gsi_c4ubv(&ctx->gsi, connect_to.color); gsi_v2fv(&ctx->gsi, connect_to.position);

        float anti_alias_scl = paint->aa_scale;
		if (paint->anti_alias) 
        {
			gs_vec2 ns = gs_vec2_norm(gs_vec2_sub(start_point.position, connect_to.position));
			gs_vec2 ne = gs_vec2_norm(gs_vec2_sub(end_point.position, connect_to.position));

            gs_color_t s_col = gs_color_alpha(start_point.color, 0);
            gs_color_t e_col = gs_color_alpha(end_point.color, 0);
            
			const f32 sl = (gs_vec2_len(gs_vec2_sub(start_point.position, connect_to.position)) / (start_point.thickness)) * anti_alias_scl;
			const f32 el = (gs_vec2_len(gs_vec2_sub(end_point.position, connect_to.position)) / (end_point.thickness)) * anti_alias_scl;
			gs_vg_point_t s = gs_vg_point_add(start_point, gs_vec2_scale(ns, sl));
			gs_vg_point_t e = gs_vg_point_add(end_point, gs_vec2_scale(ne, el));
            
            gsi_c4ubv(&ctx->gsi, s_col); gsi_v2fv(&ctx->gsi, s.position);
            gsi_c4ubv(&ctx->gsi, start_point.color); gsi_v2fv(&ctx->gsi, start_point.position);
            gsi_c4ubv(&ctx->gsi, e_col); gsi_v2fv(&ctx->gsi, e.position);

            gsi_c4ubv(&ctx->gsi, e_col); gsi_v2fv(&ctx->gsi, e.position);
            gsi_c4ubv(&ctx->gsi, end_point.color); gsi_v2fv(&ctx->gsi, end_point.position);
            gsi_c4ubv(&ctx->gsi, start_point.color); gsi_v2fv(&ctx->gsi, start_point.position); 
		} 

		start_point = end_point;
	}
}

void _gsvg_path_stroke_joint_impl(gs_vg_ctx_t* ctx, 
        gs_vg_poly_seg_t seg1, gs_vg_poly_seg_t seg2,
	    int16_t joint_style, gs_vg_point_t* end1, gs_vg_point_t* end2,
	    gs_vg_point_t* next_start1, gs_vg_point_t* next_start2, b32 allow_overlap) 
{
    gs_vg_paint_t* paint = &ctx->state.paint;

	// calculate the angle between the two line segments
	gs_vec2 dir1 = gs_vg_line_seg_dir(seg1.center, true);
	gs_vec2 dir2 = gs_vg_line_seg_dir(seg2.center, true);

	f32 angle = gs_vec2_angle(dir1, dir2);

	// wrap the angle around the 180° mark if it exceeds 90°
	// for minimum angle detection
	f32 wrapped_angle = angle;
	if (wrapped_angle > GS_PI / 2) {
		wrapped_angle = GS_PI - wrapped_angle;
	}

	if (joint_style == GS_VG_JOINT_MITER && wrapped_angle < miter_min_angle) {
		joint_style = GS_VG_JOINT_BEVEL;
	}

	if (joint_style == GS_VG_JOINT_MITER) {
		// calculate each edge's intersection point
		// with the next segment's central line
		gs_vg_line_seg_intersection_t sec1 = gs_vg_line_seg_intersection(seg1.edge1, seg2.edge1, true);
		gs_vg_line_seg_intersection_t sec2 = gs_vg_line_seg_intersection(seg1.edge2, seg2.edge2, true);

		*end1 = sec1.intersected ? sec1.point : seg1.edge1.b;
		*end2 = sec2.intersected ? sec2.point : seg1.edge2.b;

		*next_start1 = *end1;
		*next_start2 = *end2; 
	} 
    else 
    {
		// joint style is either BEVEL or ROUND

		// find out which are the inner edges for this joint
		f32 x1 = dir1.x;
		f32 x2 = dir2.x;
		f32 y1 = dir1.y;
		f32 y2 = dir2.y;

		b32 clockwise = (x1 * y2 - x2 * y1) < 0;

		gs_vg_line_seg_t *inner1, *inner2, *outer1, *outer2;

		// as the normal vector is rotated counter-clockwise,
		// the first edge lies to the left
		// from the central line's perspective,
		// and the second one to the right.
		if (clockwise) {
			outer1 = &seg1.edge1;
			outer2 = &seg2.edge1;
			inner1 = &seg1.edge2;
			inner2 = &seg2.edge2;
		} else {
			outer1 = &seg1.edge2;
			outer2 = &seg2.edge2;
			inner1 = &seg1.edge1;
			inner2 = &seg2.edge1;
		}

		// calculate the intersection point of the inner edges
		gs_vg_line_seg_intersection_t inner_sec_opt = gs_vg_line_seg_intersection(*inner1, *inner2, allow_overlap);

		gs_vg_point_t inner_sec = inner_sec_opt.intersected 
		                ? inner_sec_opt.point
		                // for parallel lines, simply connect them directly
		                : inner1->b;

		// if there's no inner intersection, flip
		// the next start position for near-180° turns 
		gs_vg_point_t inner_start;

		if (inner_sec_opt.intersected) 
        {
			inner_start = inner_sec;
		} 
        else if (angle > GS_PI / 2.0) 
        {
			inner_start = outer1->b;
		} 
        else 
        {
			inner_start = inner1->b;
		}

		if (clockwise) 
        {
			*end1 = outer1->b;
			*end2 = inner_sec;

			*next_start1 = outer2->a;
			*next_start2 = inner_start; 
		} 
        else 
        {
			*end1 = inner_sec;
			*end2 = outer1->b;

			*next_start1 = inner_start;
			*next_start2 = outer2->a;
		}

		// connect the intersection points according to the joint style

		if (joint_style == GS_VG_JOINT_BEVEL) 
		{
            gsi_c4ubv(&ctx->gsi, outer1->b.color);
            gsi_v2fv(&ctx->gsi, outer1->b.position);

            gsi_c4ubv(&ctx->gsi, outer2->a.color);
            gsi_v2fv(&ctx->gsi, outer2->a.position);

            gsi_c4ubv(&ctx->gsi, inner_sec.color);
            gsi_v2fv(&ctx->gsi, inner_sec.position);
            
            float anti_alias_scl = paint->aa_scale;
			if (paint->anti_alias) 
			{
				gs_vec2 ns = gs_vec2_norm(gs_vec2_sub(outer1->b.position, inner_sec.position));
				gs_vec2 ne = gs_vec2_norm(gs_vec2_sub(outer2->a.position, inner_sec.position));

				// gs_vec4 s_col = (gs_vec4){outer1->b.color.x, outer1->b.color.y, outer1->b.color.z, 0.f};
				// gs_vec4 e_col = (gs_vec4){outer2->a.color.x, outer2->a.color.y, outer2->a.color.z, 0.f};
                gs_color_t s_col = gs_color_alpha(outer1->b.color, 0);
                gs_color_t e_col = gs_color_alpha(outer2->a.color, 0);
				const f32 sl = (gs_vec2_len(gs_vec2_sub(outer1->b.position, inner_sec.position)) / (outer1->b.thickness)) * anti_alias_scl;
				const f32 el = (gs_vec2_len(gs_vec2_sub(outer2->a.position, inner_sec.position)) / (outer2->a.thickness)) * anti_alias_scl;
				gs_vg_point_t s = gs_vg_point_add(outer1->b, gs_vec2_scale(ns, sl));
				gs_vg_point_t e = gs_vg_point_add(outer2->a, gs_vec2_scale(ns, el));
                
                gsi_c4ubv(&ctx->gsi, s_col); gsi_v2fv(&ctx->gsi, s.position);
                gsi_c4ubv(&ctx->gsi, outer1->b.color); gsi_v2fv(&ctx->gsi, outer1->b.position);
                gsi_c4ubv(&ctx->gsi, e_col); gsi_v2fv(&ctx->gsi, e.position); 

                gsi_c4ubv(&ctx->gsi, e_col); gsi_v2fv(&ctx->gsi, e.position); 
                gsi_c4ubv(&ctx->gsi, outer2->a.color); gsi_v2fv(&ctx->gsi, outer2->a.position);
                gsi_c4ubv(&ctx->gsi, outer1->b.color); gsi_v2fv(&ctx->gsi, outer1->b.position); 
			}

		} 
		else if (joint_style == GS_VG_JOINT_ROUND) 
		{
			// draw a circle between the ends of the outer edges,
			// centered at the actual point
			// with half the line thickness as the radius
			_gsvg_path_stroke_triangle_fan_impl(ctx, inner_sec, seg1.center.b, outer1->b, outer2->a, clockwise);
		} 
		else 
		{
			gs_assert(false);
		}
	}
} 

typedef struct
{
    uint16_t indices[3];
} triangle_t;

GS_API_DECL void _gsvg_path_fill_impl(gs_vg_ctx_t* ctx)
{
    // Ear clipping, naive, no holes filled

    // Generate index and triangle lists to fill
    // All indices will just be entire point list

    gs_dyn_array(uint16_t) points = NULL;
    gs_dyn_array(triangle_t) triangles = NULL;
}

#endif // GS_VG_IMPL
#endif // GS_VG_H


























