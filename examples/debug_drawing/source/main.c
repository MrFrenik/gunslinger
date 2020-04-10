#include <gs.h>

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

// Forward Decls.
gs_result app_init();		// Use to init your application
gs_result app_update();		// Use to update your application
gs_result app_shutdown();	// Use to shutdown your appliaction

// Ported to a c99 impl from: https://github.com/CrushedPixel/Polyline2D/

// Need a collection of glyphs, each with bearing/advance/etc.

// Colors
#define white (gs_vec4){1.f, 1.f, 1.f, 1.f}
#define red (gs_vec4){1.f, 0.f, 0.f, 1.f}
#define green (gs_vec4){0.f, 1.f, 0.f, 1.f}
#define blue (gs_vec4){0.f, 0.f, 1.f, 1.f}
#define purple (gs_vec4){1.f, 0.f, 1.f, 1.f}

_global b32 anti_alias = true;
_global f32 anti_alias_scl = 2.f;
_global b32 is_playing = true;

_global NSVGimage *image = NULL;

typedef struct vg_ctx_t
{
	gs_dyn_array(gs_vec2) points;
	b32 anti_alias;
} vg_ctx_t;

vg_ctx_t vg_ctx_create()
{
	vg_ctx_t ctx = {0};
	ctx.points = gs_dyn_array_new(gs_vec2);
	ctx.anti_alias = true;
	return ctx;
}

void vg_ctx_reset( vg_ctx_t* ctx )
{
	gs_dyn_array_clear(ctx->points);
	ctx->anti_alias = true;
}

// Don't know how to keep track of state for connecting joint style...

 // ~20 degrees
#define miter_min_angle gs_deg_to_rad(20)
 // ~10 degrees
#define round_min_angle gs_deg_to_rad(20)

typedef struct poly_point_t
{
	gs_vec2 position;
	gs_vec4 color;
	f32 thickness;
} poly_point_t;

poly_point_t poly_point_create(gs_vec2 position, gs_vec4 color, f32 thickness)
{
	poly_point_t p = {0};
	p.position = position;
	p.color = color;
	p.thickness = thickness;
	return p;
}

poly_point_t poly_point_add(poly_point_t p, gs_vec2 os)
{
	poly_point_t pp = p;
	pp.position = gs_vec2_add(pp.position, os);
	return pp;
}

poly_point_t poly_point_sub(poly_point_t p, gs_vec2 os)
{
	return poly_point_add(p, gs_vec2_scale(os, -1.f));
}


// Need a way to do complex poly shapes

typedef struct vert_t
{
	gs_vec2 position;
	gs_vec4 color;
} vert_t;

vert_t vert_t_create(gs_vec2 position, gs_vec4 color)
{
	return (vert_t){ position, color };
}

typedef struct line_segment_t
{
	poly_point_t a;
	poly_point_t b;
} line_segment_t;

typedef struct line_segment_intersection_t
{
	b32 intersected;
	poly_point_t point;
} line_segment_intersection_t;

/**
 * @return A copy of the line segment, offset by the given vector.
 */
line_segment_t line_seg_add( line_segment_t l, gs_vec2 os ) 
{
	poly_point_t a = poly_point_create(gs_vec2_add(l.a.position, os), l.a.color, l.a.thickness);
	poly_point_t b = poly_point_create(gs_vec2_add(l.b.position, os), l.b.color, l.b.thickness);
	return (line_segment_t){ a, b };
	// return (line_segment_t){ gs_vec2_add(l.a, os), gs_vec2_add(l.b, os) };
}

/**
 * @return A copy of the line segment, offset by the given vector.
 */
line_segment_t line_seg_sub( line_segment_t l, gs_vec2 os ) 
{
	return line_seg_add( l, (gs_vec2){ -os.x, -os.y } );
}

/**
 * @return The line segment's direction vector.
 */
gs_vec2 line_seg_dir( line_segment_t l, b32 normalized ) 
{
	// gs_vec2 vec = gs_vec2_sub(l.b, l.a);
	gs_vec2 vec = gs_vec2_sub(l.b.position, l.a.position);
	return normalized ? gs_vec2_norm(vec) : vec;
}

/**
 * @return The line segment's normal vector.
 */
gs_vec2 line_seg_normal( line_segment_t l ) 
{
	gs_vec2 dir = line_seg_dir( l, true );

	// return the direction vector
	// rotated by 90 degrees counter-clockwise
	return (gs_vec2){-dir.y, dir.x};
}

#define null_intersection()\
	(line_segment_intersection_t){ false, (poly_point_t){0} }

line_segment_intersection_t line_seg_intersection( line_segment_t a, line_segment_t b, b32 infiniteLines ) 
{
	// calculate un-normalized direction vectors
	gs_vec2 r = line_seg_dir( a, false );
	gs_vec2 s = line_seg_dir( b, false );

	// gs_vec2 origin_dist = gs_vec2_sub(b.a, a.a);
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
	// return (line_segment_intersection_t){ true, gs_vec2_add(a.a, gs_vec2_scale(r, t)) };
	// return (line_segment_intersection_t){ true, gs_vec2_add(a.a.position, gs_vec2_scale(r, t)) };
	return (line_segment_intersection_t){ true, poly_point_add(a.a, gs_vec2_scale(r, t)) };
}

typedef enum joint_style_t
{
	/**
	 * Corners are drawn with sharp joints.
	 * If the joint's outer angle is too large,
	 * the joint is drawn as beveled instead,
	 * to avoid the miter extending too far out.
	 */
	joint_style_miter,
	/**
	 * Corners are flattened.
	 */
	joint_style_bevel,
	/**
	 * Corners are rounded off.
	 */
	joint_style_round

} joint_style_t;

typedef enum end_cap_style_t 
{
	/**
	 * Path ends are drawn flat,
	 * and don't exceed the actual end point.
	 */
	end_cap_style_butt,
	/**
	 * Path ends are drawn flat,
	 * but extended beyond the end point
	 * by half the line thickness.
	 */
	end_cap_style_square,
	/**
	 * Path ends are rounded off.
	 */
	end_cap_style_round,
	/**
	 * Path ends are connected according to the JointStyle.
	 * When using this EndCapStyle, don't specify the common start/end point twice,
	 * as Polyline2D connects the first and last input point itself.
	 */
	end_cap_style_joint

} end_cap_style_t;

typedef struct poly_segment_t 
{
	line_segment_t center; 
	line_segment_t edge1; 
	line_segment_t edge2;
} poly_segment_t;

// Forward Decl.
void poly_line_create_joint( gs_dyn_array(vert_t)* vertices,
	                                  poly_segment_t seg1, poly_segment_t seg2,
	                                  joint_style_t joint_style, poly_point_t* end1, poly_point_t* end2,
	                                  poly_point_t* next_start1, poly_point_t* next_start2,
	                                  b32 allow_overlap);

void poly_line_create_triangle_fan( gs_dyn_array(vert_t)* vertices, poly_point_t connect_to, poly_point_t origin,
                                        poly_point_t start, poly_point_t end, b32 clockwise );

poly_segment_t poly_seg_create( line_segment_t center ) 
{
	poly_segment_t p = {0};
	p.center = center;
	// calculate the segment's outer edges by offsetting
	// the central line by the normal vector
	// multiplied with the thickness

	// center + center.normal() * thickness
	// This is for growing along center
	// p.edge1 = line_seg_add(center, gs_vec2_scale(line_seg_normal(center), thickness));
	// p.edge2 = line_seg_sub(center, gs_vec2_scale(line_seg_normal(center), thickness));

	// Need to create these manually
	gs_vec2 n = line_seg_normal(center);

	// typedef struct poly_segment_t 
	// {
	// 	line_segment_t center; 
	// 	line_segment_t edge1; 
	// 	line_segment_t edge2;
	// } poly_segment_t;

	// Edge1.a will be a line segment from center + norm * a;
	p.edge1.a = poly_point_add(center.a, gs_vec2_scale(n, center.a.thickness));
	p.edge1.b = poly_point_add(center.b, gs_vec2_scale(n, center.b.thickness));
	// Edge1.b will be a line segment from center - norm * a;
	p.edge2.a = poly_point_sub(center.a, gs_vec2_scale(n, center.a.thickness));
	p.edge2.b = poly_point_sub(center.b, gs_vec2_scale(n, center.b.thickness));



	// This is for growing 'outwards' away from center
	// p.edge1 = center;
	// p.edge2 = line_seg_sub(center, gs_vec2_scale(line_seg_normal(center), thickness * 2.f));

	// This is for growing 'inwards' away from center
	// p.edge1 = center;
	// p.edge2 = line_seg_add(center, gs_vec2_scale(line_seg_normal(center), thickness * 2.f));

	return p;
}

void poly_line_create( gs_dyn_array(vert_t)* vertices, poly_point_t* points, u32 count, 
				joint_style_t joint_style, end_cap_style_t end_cap_style,
                b32 allow_overlap ) 
{
	gs_dyn_array(poly_segment_t) segments = gs_dyn_array_new(poly_segment_t);
	for (u32 i = 0; i + 1 < count; ++i) 
	{
		poly_point_t point1 = points[i];
		poly_point_t point2 = points[i + 1];

		// to avoid division-by-zero errors,
		// only create a line segment for non-identical points
		if (!gs_vec2_equal(point1.position, point2.position)) 
		{
			poly_segment_t ps = poly_seg_create( (line_segment_t){point1, point2} ); 
			gs_dyn_array_push(segments, ps);
		}
	}

	if (end_cap_style == end_cap_style_joint) {
		// create a connecting segment from the last to the first point

		poly_point_t point1 = points[count - 1];
		poly_point_t point2 = points[0];

		// to avoid division-by-zero errors,
		// only create a line segment for non-identical points
		if (!gs_vec2_equal(point1.position, point2.position)) 
		{
			poly_segment_t ps = poly_seg_create( (line_segment_t){point1, point2} ); 
			gs_dyn_array_push(segments, ps);
		}
	}

	if (gs_dyn_array_empty(segments)) {
		// handle the case of insufficient input points
		return;
	}

	poly_point_t next_start1 = {0};
	poly_point_t next_start2 = {0};
	poly_point_t start1 = {0};
	poly_point_t start2 = {0};
	poly_point_t end1 = {0};
	poly_point_t end2 = {0};

	// calculate the path's global start and end points
	poly_segment_t first_segment = segments[0];
	poly_segment_t last_segment = segments[gs_dyn_array_size(segments) - 1];

	poly_point_t path_start1 = first_segment.edge1.a;
	poly_point_t path_start2 = first_segment.edge2.a;
	poly_point_t path_end1 = last_segment.edge1.b;
	poly_point_t path_end2 = last_segment.edge2.b;

	// handle different end cap styles
	if (end_cap_style == end_cap_style_square) {
		// extend the start/end points by half the thickness
		// path_start1 = gs_vec2_sub(path_start1, gs_vec2_scale(line_seg_dir(first_segment.edge1, true), thickness));
		// path_start2 = gs_vec2_sub(path_start2, gs_vec2_scale( line_seg_dir(first_segment.edge2, true), thickness));
		// path_end1 = gs_vec2_add(path_end1, gs_vec2_scale( line_seg_dir(last_segment.edge1, true), thickness));
		// path_end2 = gs_vec2_add(path_end2, gs_vec2_scale( line_seg_dir(last_segment.edge2, true), thickness));

		path_start1 = poly_point_sub(path_start1, gs_vec2_scale(line_seg_dir(first_segment.edge1, true), first_segment.edge1.a.thickness));
		path_start2 = poly_point_sub(path_start2, gs_vec2_scale( line_seg_dir(first_segment.edge2, true), first_segment.edge1.a.thickness));
		path_end1 = poly_point_add(path_end1, gs_vec2_scale( line_seg_dir(last_segment.edge1, true), last_segment.edge1.b.thickness));
		path_end2 = poly_point_add(path_end2, gs_vec2_scale( line_seg_dir(last_segment.edge2, true), last_segment.edge1.b.thickness));

	} else if (end_cap_style == end_cap_style_round) {
		// draw half circle end caps
		poly_line_create_triangle_fan(vertices, first_segment.center.a, first_segment.center.a,
		                  first_segment.edge1.a, first_segment.edge2.a, false);
		poly_line_create_triangle_fan(vertices, last_segment.center.b, last_segment.center.b,
		                  last_segment.edge1.b, last_segment.edge2.b, true);

	} else if (end_cap_style == end_cap_style_joint) {
		// join the last (connecting) segment and the first segment
		poly_line_create_joint(vertices, last_segment, first_segment, joint_style,
		            &path_end1, &path_end2, &path_start1, &path_start2, allow_overlap);
	}

	// generate mesh data for path segments
	for (u32 i = 0; i < gs_dyn_array_size(segments); ++i) {

		poly_segment_t segment = segments[i];

		// calculate start
		if (i == 0) {
			// this is the first segment
			start1 = path_start1;
			start2 = path_start2;
		}

		if (i + 1 == gs_dyn_array_size(segments)) {
			// this is the last segment
			end1 = path_end1;
			end2 = path_end2;
		} else {
			poly_line_create_joint(vertices, segment, segments[i + 1], joint_style,
			            &end1, &end2, &next_start1, &next_start2, allow_overlap);
		}

		if ( anti_alias) {

			f32 s_thick = gs_max(start1.thickness, start2.thickness); 
			f32 e_thick = gs_max(end1.thickness, end2.thickness); 

			// Push back verts for anti-aliasing as well...somehow
			gs_vec2 sn = gs_vec2_norm(gs_vec2_sub(start2.position, start1.position));
			gs_vec2 en = gs_vec2_norm(gs_vec2_sub(end2.position, end1.position));
			const f32 sl = gs_vec2_len(gs_vec2_sub(start2.position, start1.position)) / (s_thick) / anti_alias_scl;
			const f32 el = gs_vec2_len(gs_vec2_sub(end2.position, end1.position)) / (e_thick) / anti_alias_scl;

			gs_vec4 s1_col = (gs_vec4){start1.color.x, start1.color.y, start1.color.z, 0.f};
			gs_vec4 s2_col = (gs_vec4){start2.color.x, start2.color.y, start2.color.z, 0.f};
			gs_vec4 e1_col = (gs_vec4){end1.color.x, end1.color.y, end1.color.z, 0.f};
			gs_vec4 e2_col = (gs_vec4){end2.color.x, end2.color.y, end2.color.z, 0.f};

			poly_point_t s1 = poly_point_sub(start1, gs_vec2_scale(sn, sl));
			poly_point_t s2 = poly_point_add(start2, gs_vec2_scale(sn, sl));
			poly_point_t e1 = poly_point_sub(end1, gs_vec2_scale(en, el));
			poly_point_t e2 = poly_point_add(end2, gs_vec2_scale(en, el));

			gs_dyn_array_push(*vertices, vert_t_create(s1.position, s1_col));
			gs_dyn_array_push(*vertices, vert_t_create(start1.position, start1.color));
			gs_dyn_array_push(*vertices, vert_t_create(e1.position, e1_col));

			gs_dyn_array_push(*vertices, vert_t_create(e1.position, e1_col));
			gs_dyn_array_push(*vertices, vert_t_create(end1.position, end1.color));
			gs_dyn_array_push(*vertices, vert_t_create(start1.position, start1.color));

			gs_dyn_array_push(*vertices, vert_t_create(s2.position, s2_col));
			gs_dyn_array_push(*vertices, vert_t_create(start2.position, start2.color));
			gs_dyn_array_push(*vertices, vert_t_create(e2.position, e2_col));

			gs_dyn_array_push(*vertices, vert_t_create(e2.position, e2_col));
			gs_dyn_array_push(*vertices, vert_t_create(end2.position, end2.color));
			gs_dyn_array_push(*vertices, vert_t_create(start2.position, start2.color));

			// If we're at beginning and not end_cap_joint, then we need to anti-alias edge
			if ( i == 0 && (end_cap_style == end_cap_style_square 
				|| end_cap_style == end_cap_style_butt) )
			{
				gs_vec2 snc = (gs_vec2){-sn.y, sn.x}; 
				poly_point_t s1c = poly_point_sub(s1, gs_vec2_scale(snc, sl));
				poly_point_t s2c = poly_point_sub(s2, gs_vec2_scale(snc, sl));

				// gs_dyn_array_push(*vertices, vert_t_create(s1c.position, s1_col));
				// gs_dyn_array_push(*vertices, vert_t_create(s1.position, s1_col));
				// gs_dyn_array_push(*vertices, vert_t_create(s2.position, s2_col));

				// gs_dyn_array_push(*vertices, vert_t_create(s2.position, s2_col));
				// gs_dyn_array_push(*vertices, vert_t_create(s2c.position, s2_col));
				// gs_dyn_array_push(*vertices, vert_t_create(s1c.position, s1_col));
			}

			if ( (i + 1) == gs_dyn_array_size(segments) 
				&& (end_cap_style == end_cap_style_square 
				|| end_cap_style == end_cap_style_butt) )
			{
				gs_vec2 enc = (gs_vec2){-en.y, en.x}; 
				poly_point_t e1c = poly_point_add(e1, gs_vec2_scale(enc, el));
				poly_point_t e2c = poly_point_add(e2, gs_vec2_scale(enc, el));

				// gs_dyn_array_push(*vertices, vert_t_create(e1c.position, e1_col));
				// gs_dyn_array_push(*vertices, vert_t_create(e1.position, e1_col));
				// gs_dyn_array_push(*vertices, vert_t_create(e2.position, e2_col));

				// gs_dyn_array_push(*vertices, vert_t_create(e2.position, e2_col));
				// gs_dyn_array_push(*vertices, vert_t_create(e2c.position, e2_col));
				// gs_dyn_array_push(*vertices, vert_t_create(e1c.position, e1_col));
			}
		}

		// Push back verts
		gs_dyn_array_push(*vertices, vert_t_create(start1.position, start1.color));
		gs_dyn_array_push(*vertices, vert_t_create(start2.position, start2.color));
		gs_dyn_array_push(*vertices, vert_t_create(end1.position, end1.color));

		gs_dyn_array_push(*vertices, vert_t_create(end1.position, end1.color));
		gs_dyn_array_push(*vertices, vert_t_create(start2.position, start2.color));
		gs_dyn_array_push(*vertices, vert_t_create(end2.position, end2.color));

		start1 = next_start1;
		start2 = next_start2;
	}

	gs_dyn_array_free(segments);
}

void poly_line_create_joint( gs_dyn_array(vert_t)* vertices,
	                                  poly_segment_t seg1, poly_segment_t seg2,
	                                  joint_style_t joint_style, poly_point_t* end1, poly_point_t* end2,
	                                  poly_point_t* next_start1, poly_point_t* next_start2,
	                                  b32 allow_overlap) 
{
	// calculate the angle between the two line segments
	gs_vec2 dir1 = line_seg_dir(seg1.center, true);
	gs_vec2 dir2 = line_seg_dir(seg2.center, true);

	f32 angle = gs_vec2_angle(dir1, dir2);

	// wrap the angle around the 180° mark if it exceeds 90°
	// for minimum angle detection
	f32 wrapped_angle = angle;
	if (wrapped_angle > gs_pi / 2) {
		wrapped_angle = gs_pi - wrapped_angle;
	}

	if (joint_style == joint_style_miter && wrapped_angle < miter_min_angle) {
		// the minimum angle for mitered joints wasn't exceeded.
		// to avoid the intersection point being extremely far out,
		// thus producing an enormous joint like a rasta on 4/20,
		// we render the joint beveled instead.
		joint_style = joint_style_bevel;
	}

	if (joint_style == joint_style_miter) {
		// calculate each edge's intersection point
		// with the next segment's central line
		line_segment_intersection_t sec1 = line_seg_intersection(seg1.edge1, seg2.edge1, true);
		line_segment_intersection_t sec2 = line_seg_intersection(seg1.edge2, seg2.edge2, true);

		*end1 = sec1.intersected ? sec1.point : seg1.edge1.b;
		*end2 = sec2.intersected ? sec2.point : seg1.edge2.b;

		*next_start1 = *end1;
		*next_start2 = *end2;

	} else {
		// joint style is either BEVEL or ROUND

		// find out which are the inner edges for this joint
		f32 x1 = dir1.x;
		f32 x2 = dir2.x;
		f32 y1 = dir1.y;
		f32 y2 = dir2.y;

		b32 clockwise = (x1 * y2 - x2 * y1) < 0;

		line_segment_t *inner1, *inner2, *outer1, *outer2;

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
		line_segment_intersection_t inner_sec_opt = line_seg_intersection(*inner1, *inner2, allow_overlap);

		poly_point_t inner_sec = inner_sec_opt.intersected 
		                ? inner_sec_opt.point
		                // for parallel lines, simply connect them directly
		                : inner1->b;

		// if there's no inner intersection, flip
		// the next start position for near-180° turns
		poly_point_t inner_start;
		if (inner_sec_opt.intersected) {
			inner_start = inner_sec;
		} else if (angle > gs_pi / 2.0) {
			inner_start = outer1->b;
		} else {
			inner_start = inner1->b;
		}

		if (clockwise) {
			*end1 = outer1->b;
			*end2 = inner_sec;

			*next_start1 = outer2->a;
			*next_start2 = inner_start;

		} else {
			*end1 = inner_sec;
			*end2 = outer1->b;

			*next_start1 = inner_start;
			*next_start2 = outer2->a;
		}

		// connect the intersection points according to the joint style

		if (joint_style == joint_style_bevel) 
		{
			// simply connect the intersection points
			// gs_dyn_array_push(*vertices, vert_t_create(outer1->b.position, outer1->a.color));
			// gs_dyn_array_push(*vertices, vert_t_create(outer2->a.position, outer2->b.color));
			// gs_dyn_array_push(*vertices, vert_t_create(inner_sec.position, inner_sec.color));

			// gs_dyn_array_push(*vertices, vert_t_create(outer1->b.position, red));
			// gs_dyn_array_push(*vertices, vert_t_create(outer2->a.position, red));
			// gs_dyn_array_push(*vertices, vert_t_create(inner_sec.position, red));

			gs_dyn_array_push(*vertices, vert_t_create(outer1->b.position, outer1->b.color)); // right
			gs_dyn_array_push(*vertices, vert_t_create(outer2->a.position, outer2->a.color));	// left
			gs_dyn_array_push(*vertices, vert_t_create(inner_sec.position, inner_sec.color));	// center

			if ( anti_alias ) 
			{
				gs_vec2 ns = gs_vec2_norm(gs_vec2_sub(outer1->b.position, inner_sec.position));
				gs_vec2 ne = gs_vec2_norm(gs_vec2_sub(outer2->a.position, inner_sec.position));

				gs_vec4 s_col = (gs_vec4){outer1->b.color.x, outer1->b.color.y, outer1->b.color.z, 0.f};
				gs_vec4 e_col = (gs_vec4){outer2->a.color.x, outer2->a.color.y, outer2->a.color.z, 0.f};
				const f32 sl = gs_vec2_len(gs_vec2_sub(outer1->b.position, inner_sec.position)) / (outer1->b.thickness) / anti_alias_scl;
				const f32 el = gs_vec2_len(gs_vec2_sub(outer2->a.position, inner_sec.position)) / (outer2->a.thickness) / anti_alias_scl;
				poly_point_t s = poly_point_add(outer1->b, gs_vec2_scale(ns, sl));
				poly_point_t e = poly_point_add(outer2->a, gs_vec2_scale(ns, el));

				gs_dyn_array_push(*vertices, vert_t_create(s.position, s_col));
				gs_dyn_array_push(*vertices, vert_t_create(outer1->b.position, outer1->b.color));
				gs_dyn_array_push(*vertices, vert_t_create(e.position, e_col));

				gs_dyn_array_push(*vertices, vert_t_create(e.position, e_col));
				gs_dyn_array_push(*vertices, vert_t_create(outer2->a.position, outer2->a.color));
				gs_dyn_array_push(*vertices, vert_t_create(outer1->b.position, outer1->b.color));

			}

		} 
		else if (joint_style == joint_style_round) 
		{
			// draw a circle between the ends of the outer edges,
			// centered at the actual point
			// with half the line thickness as the radius
			poly_line_create_triangle_fan(vertices, inner_sec, seg1.center.b, outer1->b, outer2->a, clockwise);
		} 
		else 
		{
			gs_assert(false);
		}
	}
}

void poly_line_create_triangle_fan( gs_dyn_array(vert_t)* vertices, poly_point_t connect_to, poly_point_t origin,
                                        poly_point_t start, poly_point_t end, b32 clockwise ) 
{
	poly_point_t point1 = poly_point_sub(start, origin.position);
	poly_point_t point2 = poly_point_sub(end, origin.position);

	// calculate the angle between the two points
	f32 angle1 = atan2(point1.position.y, point1.position.x);
	f32 angle2 = atan2(point2.position.y, point2.position.x);

	// ensure the outer angle is calculated
	if (clockwise) {
		if (angle2 > angle1) {
			angle2 = angle2 - 2.0 * gs_pi;
		}
	} else {
		if (angle1 > angle2) {
			angle1 = angle1 - 2.0 * gs_pi;
		}
	}

	f32 joint_angle = angle2 - angle1;

	// calculate the amount of triangles to use for the joint
	s32 num_triangles = gs_max(1, floor(fabsf(joint_angle) / round_min_angle));

	// calculate the angle of each triangle
	f32 tri_angle = joint_angle / (f32)num_triangles;

	poly_point_t start_point = start;
	poly_point_t end_point;

	for (s32 t = 0; t < num_triangles; t++) 
	{
		if (t + 1 == num_triangles) {
			// it's the last triangle - ensure it perfectly
			// connects to the next line
			end_point = end;
		} else {
			f32 rot = (t + 1) * tri_angle;

			// rotate the original point around the origin
			end_point.position.x = cos(rot) * point1.position.x - sin(rot) * point1.position.y;
			end_point.position.y = sin(rot) * point1.position.x + cos(rot) * point1.position.y;

			// re-add the rotation origin to the target point
			end_point = poly_point_add(end_point, origin.position);
		}

		// emit the triangle
		gs_dyn_array_push( *vertices, vert_t_create(start_point.position, start_point.color));
		gs_dyn_array_push( *vertices, vert_t_create(end_point.position, end_point.color));
		gs_dyn_array_push( *vertices, vert_t_create(connect_to.position, connect_to.color));

		if ( anti_alias ) {
			gs_vec2 ns = gs_vec2_norm(gs_vec2_sub(start_point.position, connect_to.position));
			gs_vec2 ne = gs_vec2_norm(gs_vec2_sub(end_point.position, connect_to.position));

			gs_vec4 s_col = (gs_vec4){start_point.color.x, start_point.color.y, start_point.color.z, 0.f};
			gs_vec4 e_col = (gs_vec4){end_point.color.x, end_point.color.y, end_point.color.z, 0.f};
			const f32 sl = gs_vec2_len(gs_vec2_sub(start_point.position, connect_to.position)) / (start_point.thickness) / anti_alias_scl;
			const f32 el = gs_vec2_len(gs_vec2_sub(end_point.position, connect_to.position)) / (end_point.thickness) / anti_alias_scl;
			poly_point_t s = poly_point_add(start_point, gs_vec2_scale(ns, sl));
			poly_point_t e = poly_point_add(end_point, gs_vec2_scale(ns, el));

			gs_dyn_array_push(*vertices, vert_t_create(s.position, s_col));
			gs_dyn_array_push(*vertices, vert_t_create(start_point.position, start_point.color));
			gs_dyn_array_push(*vertices, vert_t_create(e.position, e_col));

			gs_dyn_array_push(*vertices, vert_t_create(e.position, e_col));
			gs_dyn_array_push(*vertices, vert_t_create(end_point.position, end_point.color));
			gs_dyn_array_push(*vertices, vert_t_create(start_point.position, start_point.color));
		}

		// Need to emit information for anti-alising as well


		start_point = end_point;
	}
}

// Does a shape contain paths? Or does a path contain a shape?

typedef struct path_t
{
	gs_dyn_array(poly_point_t) points;
	end_cap_style_t end_cap_style;
	joint_style_t joint_style;
} path_t;

path_t path_create_new()
{
	path_t p = {0};
	p.points = gs_dyn_array_new(poly_point_t);
	p.end_cap_style = end_cap_style_butt;
	return p;
}

void path_free( path_t* p )
{
	gs_dyn_array_free(p->points);
}

void path_begin( path_t* p )
{
	// Clear all previous points
	gs_dyn_array_clear(p->points);
}

void path_clear(path_t* p)
{
	path_begin(p);
}

path_t path_deep_copy(path_t* p)
{
	path_t path = path_create_new();
	path.end_cap_style = p->end_cap_style;
	path.joint_style = p->joint_style;
	for ( u32 i = 0; i < gs_dyn_array_size(p->points); ++i )
	{
		gs_dyn_array_push(path.points, p->points[i]);
	}
	return path;
}

void path_submit( path_t* p, gs_dyn_array(vert_t)* verts )
{
	// Submit all points to renderer
	poly_line_create(verts, p->points, gs_dyn_array_size(p->points), p->joint_style, p->end_cap_style, false);
}

void path_draw_line( path_t* p, gs_vec2 start, gs_vec2 end, f32 thickness, gs_vec4 color )
{
	gs_dyn_array_push(p->points, poly_point_create(start, color, thickness));
	gs_dyn_array_push(p->points, poly_point_create(end, color, thickness));
}

void path_draw_triangle( path_t* p, gs_vec2 a, gs_vec2 b, gs_vec2 c, f32 thickness, gs_vec4 color )
{
	gs_dyn_array_push(p->points, poly_point_create(a, color, thickness)); 
	gs_dyn_array_push(p->points, poly_point_create(b, color, thickness)); 
	gs_dyn_array_push(p->points, poly_point_create(c, color, thickness)); 
}

void path_draw_circle( path_t* p, gs_vec2 origin, f32 r, s32 num_segments, f32 thickness, gs_vec4 color )
{
	f32 step = num_segments < 5 ? 360.f / 5.f : 360.f / (f32)num_segments;
	for ( f32 i = 0; i < 360.f; i += step ) {
		f32 a = gs_deg_to_rad(i);
		poly_point_t pt = poly_point_create(gs_vec2_add(origin, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								color, thickness);
		gs_dyn_array_push(p->points, pt);
	}
	// Push last angle on as well
	{
		f32 a = gs_deg_to_rad(360.f);
		poly_point_t pt = poly_point_create(gs_vec2_add(origin, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								color, thickness);
		gs_dyn_array_push(p->points, pt);
	}
}

void path_draw_arc( path_t* p, gs_vec2 origin, f32 r, f32 start_angle, f32 end_angle, s32 num_segments, f32 thickness, gs_vec4 color )
{
	if ( start_angle > end_angle ) {
		f32 tmp = start_angle;
		start_angle = end_angle;
		end_angle = tmp;
	}
	f32 diff = end_angle - start_angle;
	if ( fabsf(diff) <= 0.001f ) {
		return;
	}

	f32 step = num_segments < 5 ? diff / 5.f : diff / (f32)num_segments;
	for ( f32 i = start_angle; i <= end_angle; i += step ) {
		f32 a = gs_deg_to_rad(i);
		poly_point_t pt = poly_point_create(gs_vec2_add(origin, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								color, thickness);
		gs_dyn_array_push(p->points, pt);
	}
	// Push last angle on as well
	{
		f32 a = gs_deg_to_rad(end_angle);
		poly_point_t pt = poly_point_create(gs_vec2_add(origin, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								color, thickness);
		gs_dyn_array_push(p->points, pt);
	}
}

void path_draw_square( path_t* p, gs_vec2 origin, gs_vec2 half_extents, f32 thickness, gs_vec4 color )
{

	gs_dyn_array_push(p->points, poly_point_create((gs_vec2){origin.x - half_extents.x, origin.y - half_extents.y}, color, thickness));
	gs_dyn_array_push(p->points, poly_point_create((gs_vec2){origin.x + half_extents.x, origin.y - half_extents.y}, color, thickness));
	gs_dyn_array_push(p->points, poly_point_create((gs_vec2){origin.x + half_extents.x, origin.y + half_extents.y}, color, thickness));
	gs_dyn_array_push(p->points, poly_point_create((gs_vec2){origin.x - half_extents.x, origin.y + half_extents.y}, color, thickness));
}

void path_draw_G(path_t* p, gs_vec2 position, f32 thickness, gs_vec4 color)
{
	const f32 r = 30.f;
	const f32 start_angle = 0.f;
	const f32 end_angle = 320.f;
	const s32 num_segments = 40;
	f32 diff = end_angle - start_angle;

	// Draw line first (from position to outside radius)
	gs_dyn_array_push(p->points, poly_point_create((gs_vec2){position.x, position.y}, color, thickness));
	gs_dyn_array_push(p->points, poly_point_create((gs_vec2){position.x + r, position.y}, color, thickness));

	// Create arc
	f32 step = diff / (f32)num_segments;
	for ( f32 i = start_angle; i <= end_angle; i += step ) {
		f32 a = gs_deg_to_rad(i);
		poly_point_t pt = poly_point_create(gs_vec2_add(position, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								color, thickness);
		gs_dyn_array_push(p->points, pt);
	}
	// Push last angle on as well
	{
		f32 a = gs_deg_to_rad(end_angle);
		poly_point_t pt = poly_point_create(gs_vec2_add(position, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								color, thickness);
		gs_dyn_array_push(p->points, pt);
	}
}

typedef struct shape_t
{
	gs_dyn_array(path_t) paths;
	u32 current_path_idx;
	gs_vqs xform;
} shape_t;

shape_t shape_create_new()
{
	shape_t s = {0};
	s.paths = gs_dyn_array_new(path_t);
	s.current_path_idx = 0;
	s.xform = gs_vqs_default();
	return s;
}

void shape_begin(shape_t* s)
{
	// Clear all path memory
	gs_for_range_i(gs_dyn_array_size(s->paths)) {
		path_free(&s->paths[i]);
	}
	gs_dyn_array_clear(s->paths);
}

path_t* shape_begin_path(shape_t* s)
{
	gs_dyn_array_push(s->paths, path_create_new());
	return &s->paths[gs_dyn_array_size(s->paths) - 1];
}

void shape_free(shape_t* s)
{
	gs_for_range_i(gs_dyn_array_size(s->paths)) {
		gs_dyn_array_clear(&s->paths[i]);
		path_free(&s->paths[i]);
	}
	gs_dyn_array_clear(s->paths);
	gs_dyn_array_free(s->paths);
}

void shape_submit(shape_t *s, gs_dyn_array(vert_t)* verts)
{
	gs_for_range_i(gs_dyn_array_size(s->paths))
	{
		path_submit(&s->paths[i], verts);
	}
}

shape_t shape_deep_copy(shape_t* s)
{
	shape_t shape = shape_create_new();

	for ( u32 i = 0; i < gs_dyn_array_size(s->paths); ++i ) {
		gs_dyn_array_push(shape.paths, path_deep_copy(&s->paths[i]));
	}

	shape.xform = s->xform;

	return shape;
}

void shape_draw_line( shape_t* s, gs_vec2 start, gs_vec2 end, f32 thickness, gs_vec4 color )
{
	path_t* p = shape_begin_path(s);
	path_draw_line(p, start, end, thickness, color);
}

void shape_draw_square( shape_t* s, gs_vec2 origin, gs_vec2 half_extents, f32 thickness, gs_vec4 color, b32 fill )
{
	if ( fill ) 
	{
		// Calculate thickness needed to fill square
		gs_vec2 sp = (gs_vec2){origin.x, origin.y - half_extents.y - 2.f * thickness};
		gs_vec2 ep = (gs_vec2){origin.x, origin.y + half_extents.y + 2.f * thickness};
		shape_draw_line(s, sp, ep, half_extents.x / 2.f + 2.f * thickness, color);	
	}

	path_t* p = shape_begin_path(s);
	path_draw_square(p, origin, half_extents, thickness, color);
	p->end_cap_style = end_cap_style_joint;
}

void shape_draw_grid(shape_t* s, gs_vec2 origin, f32 width, f32 height, u32 num_cols, u32 num_rows, f32 thickness, gs_vec4 color )
{
	f32 cell_width = width / (f32)num_cols;
	f32 cell_height = height / (f32)num_rows;

	gs_vec2 tl = (gs_vec2){ origin.x - width / 2.f, origin.y - height / 2.f };

	// Draw grid across entire screen for shiggles
	for ( u32 r = 0; r <= num_rows; ++r )
	{
		path_t* p = shape_begin_path(s);
		gs_vec2 sp = (gs_vec2){tl.x, tl.y + r * cell_height};
		gs_vec2 ep = (gs_vec2){tl.x + width, tl.y + r * cell_height};
		path_draw_line(p, sp, ep, thickness, color);
	}

	for ( u32 c = 0; c <= num_cols; ++c )
	{
		path_t* p = shape_begin_path(s);
		gs_vec2 sp = (gs_vec2){tl.x + c * cell_width, tl.y};
		gs_vec2 ep = (gs_vec2){tl.x + c * cell_width, tl.y + height};
		path_draw_line(p, sp, ep, thickness, color);
	}
}


typedef enum animation_action_type_t
{
	animation_action_type_walk_path = 0,
	animation_action_type_transform,		// Takes in vqs transform and will 
	animation_action_type_wait,
	animation_action_type_disable_shape,
	animation_action_percentage_alpha
} animation_action_type_t;

typedef enum animation_ease_type_t
{
	animation_ease_type_lerp = 0,
	animation_ease_type_smooth_step,
	animation_ease_type_ease_in,
	animation_ease_type_ease_out,
	animation_ease_type_cosine
} animation_ease_type_t;

typedef struct animation_path_desc_t {
	u32 start;
	u32 end;
	joint_style_t joint_style;
	end_cap_style_t end_cap_style;
} animation_path_desc_t;

typedef struct animation_action_desc_t
{
	u32 read_position;
	f32 total_time;
	f32 current_time;
	animation_action_type_t action_type;
	animation_ease_type_t ease_type;
} animation_action_desc_t;

typedef struct animation_t
{
	shape_t shape;
	shape_t shape_mod;
	gs_byte_buffer action_data_buffer;
	gs_dyn_array(animation_action_desc_t) actions;
	u32 current_action_idx;
	f32 total_play_time;
	f32 time;
	f32 animation_speed;
	b32 playing;
} animation_t;

animation_t animation_create_new( f32 total_play_time, f32 speed )
{
	animation_t a = {0};
	a.total_play_time = total_play_time;
	a.animation_speed = speed;
	a.time = 0.f;
	a.action_data_buffer = gs_byte_buffer_new();
	a.actions = gs_dyn_array_new(animation_action_desc_t);
	a.current_action_idx = 0;
	a.shape = shape_create_new();
	a.shape_mod = shape_create_new();
	a.playing = true;
	return a;
}

void animation_free(animation_t* a)
{
	gs_byte_buffer_free(&a->action_data_buffer);
	gs_dyn_array_free(a->actions);
}

void animation_clear(animation_t* a)
{
	gs_byte_buffer_clear(&a->action_data_buffer);
	shape_free(&a->shape);
	shape_free(&a->shape_mod);
	a->current_action_idx = 0;
	a->total_play_time = 0.f;
}

void animation_reset(animation_t* a)
{
	a->time = 0.f;
	a->playing = true;
	animation_clear(a);
}

void animation_tick(animation_t* a, f32 dt)
{
	if ( a->playing ) 
	{
		animation_action_desc_t* action = &a->actions[a->current_action_idx];
		if (action->current_time > action->total_time) 
		{
			a->current_action_idx++;

			// Resetting shape state in animation (this will not scale...)
			shape_free(&a->shape);
			a->shape = shape_deep_copy(&a->shape_mod);
		}
		if (a->current_action_idx >= gs_dyn_array_size(a->actions)) {
			a->playing = false;
		}

		// Increment animation times
		a->time = gs_clamp(a->time + dt * a->animation_speed, 0.f, a->total_play_time);
		action->current_time += dt * a->animation_speed;
	}
}

void animation_set_shape(animation_t* a, shape_t* s)
{
	animation_clear(a);
	a->shape = shape_deep_copy(s);
	a->shape_mod = shape_deep_copy(s);
}

void animation_shape(animation_t* a, shape_t* s, f32 dt)
{
	if ( !a->playing ) {
		return;
	}

	animation_action_desc_t* action = &a->actions[a->current_action_idx];

	// Set action data buffer to be back to position
	a->action_data_buffer.position = action->read_position;
	animation_action_type_t action_type = action->action_type;
	animation_ease_type_t ease_type = action->ease_type;
	f32 t = 0.f;

	switch ( ease_type )
	{
		default:
		case animation_ease_type_lerp: {
			t = gs_interp_linear(0.f, 1.f, gs_clamp(action->current_time / action->total_time, 0.f, 1.f));
		} break;

		case animation_ease_type_smooth_step: {
			t = gs_interp_smooth_step(0.f, 1.f, gs_clamp(action->current_time / action->total_time, 0.f, 1.f));
		} break;
	}

	switch ( action_type )
	{
		case animation_action_type_transform: 
		{
			// Grab transform from buffer
			gs_vqs xform = gs_byte_buffer_read(&a->action_data_buffer, gs_vqs);
			gs_vqs* sxform = &s->xform;

			switch ( ease_type )
			{
				case animation_ease_type_smooth_step: 
				{
					// Translate position over time
					sxform->position.x = gs_interp_smooth_step(a->shape.xform.position.x, xform.position.x, t);
					sxform->position.y = gs_interp_smooth_step(a->shape.xform.position.y, xform.position.y, t);
					sxform->position.z = gs_interp_smooth_step(a->shape.xform.position.z, xform.position.z, t);

					// Scale over time
					sxform->scale.x = gs_interp_smooth_step(a->shape.xform.scale.x, xform.scale.x, t);
					sxform->scale.y = gs_interp_smooth_step(a->shape.xform.scale.y, xform.scale.y, t);
					sxform->scale.z = gs_interp_smooth_step(a->shape.xform.scale.z, xform.scale.z, t);

					// Rotate over time
					// sxform->rotation = gs_quat_slerp(a->shape.xform.rotation, xform.rotation, t);
					sxform->rotation.x = gs_interp_smooth_step(a->shape.xform.rotation.x, xform.rotation.x, t);
					sxform->rotation.y = gs_interp_smooth_step(a->shape.xform.rotation.y, xform.rotation.y, t);
					sxform->rotation.z = gs_interp_smooth_step(a->shape.xform.rotation.z, xform.rotation.z, t);
					sxform->rotation.w = gs_interp_smooth_step(a->shape.xform.rotation.w, xform.rotation.w, t);

				} break;

				default:
				case animation_ease_type_lerp: 
				{
					// Translate position over time
					sxform->position.x = gs_interp_linear(a->shape.xform.position.x, xform.position.x, t);
					sxform->position.y = gs_interp_linear(a->shape.xform.position.y, xform.position.y, t);
					sxform->position.z = gs_interp_linear(a->shape.xform.position.z, xform.position.z, t);

					// Scale over time
					sxform->scale.x = gs_interp_linear(a->shape.xform.scale.x, xform.scale.x, t);
					sxform->scale.y = gs_interp_linear(a->shape.xform.scale.y, xform.scale.y, t);
					sxform->scale.z = gs_interp_linear(a->shape.xform.scale.z, xform.scale.z, t);

					sxform->rotation.x = gs_interp_linear(a->shape.xform.rotation.x, xform.rotation.x, t);
					sxform->rotation.y = gs_interp_linear(a->shape.xform.rotation.y, xform.rotation.y, t);
					sxform->rotation.z = gs_interp_linear(a->shape.xform.rotation.z, xform.rotation.z, t);
					sxform->rotation.w = gs_interp_linear(a->shape.xform.rotation.w, xform.rotation.w, t);
				} break;
			}

		} break;	

		case animation_action_type_walk_path:
		{
			// Free our shape mod, then reset here
			shape_free(&a->shape_mod);
			a->shape_mod = shape_deep_copy(&a->shape);

			// Will modify mod shape point data here
			// For each point in path, calculate total length of path
			for (u32 pi = 0; pi < gs_dyn_array_size(a->shape.paths); ++pi)
			{
				path_t* pm = &a->shape_mod.paths[pi];
				path_t* p = &a->shape.paths[pi];

				end_cap_style_t end_cap_style = p->end_cap_style;
				joint_style_t joint_style = p->joint_style;
				u32 pt_count = gs_dyn_array_size(p->points);

				f32 total_length = 0.f;

				// Have to walk each path.
				for ( u32 i = 0; i + 1 < gs_dyn_array_size(p->points); ++i )
				{
					// Calculate length from this vert to the next
					poly_point_t p0 = p->points[i];
					poly_point_t p1 = p->points[i + 1];
					total_length += gs_vec2_dist(p0.position, p1.position);
				}

				// If the end cap is joined, then need to also check against the last and first point
				if (pt_count > 2 && end_cap_style == end_cap_style_joint)
				{
					poly_point_t p0 = p->points[gs_dyn_array_size(p->points) - 1];
					poly_point_t p1 = p->points[0];
					total_length += gs_vec2_dist(p0.position, p1.position);
				}

				// Total animation length of path will be based on anim.time / anim.total_time;
				f32 anim_len = total_length * t;

				f32 cur_len = 0.f;
				s32 push_idx = -1;
				poly_point_t* push_point = NULL;
				gs_vec2 push_pos;

				for ( u32 i = 1; i < gs_dyn_array_size(p->points) + (end_cap_style == end_cap_style_joint ? 1 : 0); ++i ) {

					// Calculate length from this vert to the next
					poly_point_t* p0 = &p->points[i - 1];
					poly_point_t* p1 = &p->points[i % gs_dyn_array_size(p->points)];
					cur_len += gs_vec2_dist(p0->position, p1->position);

					if ( cur_len > anim_len) {
						// Calculate closest point we can, and submit that instead
						f32 diff = cur_len - anim_len;
						gs_vec2 n = gs_vec2_norm(gs_vec2_sub(p1->position, p0->position));
						push_pos = gs_vec2_sub(p1->position, gs_vec2_scale(n, diff));

						// We've wrapped back around
						if ( (i % gs_dyn_array_size(p->points)) == 0 ) 
						{
							// Push new point to end
							push_point = &p->points[gs_dyn_array_size(p->points) - 1];
						} 
						else 
						{
							// Set our current point to this position
							pm->points[i % gs_dyn_array_size(pm->points)].position = push_pos;
							pt_count = i + 1;
						}

						pm->end_cap_style = end_cap_style_butt;

						break;
					}
				}
				if ( push_point != NULL ) {
					gs_dyn_array_push(pm->points, poly_point_create(push_pos, push_point->color, push_point->thickness));
				}
				else {
					// Need to limit the amount of points drawn
					gs_dyn_array_size(pm->points) = pt_count;
				}
			}

		} break;

		case animation_action_percentage_alpha:
		{
			// Grab transform from buffer
			f32 fade_amt = gs_byte_buffer_read(&a->action_data_buffer, f32);

			// For each path
			for (u32 pi = 0; pi < gs_dyn_array_size(a->shape.paths); ++pi)
			{
				path_t* pm = &a->shape_mod.paths[pi];
				path_t* p = &a->shape.paths[pi];

				// For each point
				for ( u32 i = 0; i < gs_dyn_array_size(p->points); ++i ) 
				{
					// Each pixel's opacity gets faded by a certain amount over time
					poly_point_t* pt = &p->points[i];
					poly_point_t* pt_m = &pm->points[i];

					switch ( ease_type )
					{
						case animation_ease_type_smooth_step:
						{
							pt_m->color.w = gs_interp_smooth_step(pt->color.w, pt->color.w * fade_amt, t);
						} break;	

						default:
						case animation_ease_type_lerp:
						{
							pt_m->color.w = gs_interp_linear(pt->color.w, pt->color.w * fade_amt, t);
						} break;
					}
				}
			}
		} break;

		default: {
			// Do nothing...
		} break;
	}

}

poly_point_t transform_point(poly_point_t p, gs_vqs* xform)
{
	// Transform point position by model matrix
	poly_point_t pt = p;
	gs_mat4 model = gs_vqs_to_mat4(xform);
	gs_vec4 np = gs_mat4_mul_vec4(model, (gs_vec4){pt.position.x, pt.position.y, 0.f, 1.f});
	pt.position = (gs_vec2){np.x, np.y};
	pt.thickness *= gs_vec3_len(xform->scale);

	return pt;
}

// Do I actually ned to store any data then? Why couldn't I just animation a shape based on the animation actions?...
void animation_play(gs_dyn_array(vert_t)* verts, animation_t* a, f32 dt)
{
	if ( is_playing ) {
		animation_tick(a, dt);
	}

	// We dont' show if we've disabled the shape
	animation_action_desc_t* action = &a->actions[a->current_action_idx];
	animation_action_type_t action_type = action->action_type;
	if ( action_type == animation_action_type_disable_shape ) {
		return;
	}

	// Animate shape by current action
	animation_shape(a, &a->shape_mod, dt);

	gs_dyn_array(poly_point_t) points = gs_dyn_array_new(poly_point_t);

	// Transform all points via shape's model matrix
	for (u32 i = 0; i < gs_dyn_array_size(a->shape_mod.paths); ++i)
	{
		path_t* p = &a->shape_mod.paths[i];

		for (u32 j = 0; j < gs_dyn_array_size(p->points); ++j) {
			// Transform point
			poly_point_t pt = p->points[j];
			gs_dyn_array_push(points, transform_point(pt, &a->shape_mod.xform));
		}

		// Apply transform to points (this shouldn't be destructive, of course)
		// Treat points as if they are an object of read-only verts 
		poly_line_create(verts, points, gs_dyn_array_size(points), p->joint_style, p->end_cap_style, false);

		gs_dyn_array_clear(points);
	}

	gs_dyn_array_free(points);
}

/*
	f32 time: total time that this animation will run
*/
void animation_add_action(animation_t* a, animation_action_type_t action_type, 
	animation_ease_type_t ease_type, f32 time, void* data)
{
	animation_action_desc_t desc = {0};
	desc.read_position = a->action_data_buffer.position;
	desc.total_time = time;
	desc.current_time = 0.f;
	desc.action_type = action_type;
	desc.ease_type = ease_type;

	// Write beginning index for this action into action list
	gs_dyn_array_push(a->actions, desc);

	// Add to total run time
	a->total_play_time += time;

	// Then write data
	switch ( action_type )
	{
		case animation_action_type_transform: 
		{
			gs_vqs* xform = (gs_vqs*)data;
			gs_byte_buffer_write(&a->action_data_buffer, gs_vqs, *xform);
		} break;

		case animation_action_percentage_alpha: 
		{
			f32* amt = (f32*)data;
			gs_byte_buffer_write(&a->action_data_buffer, f32, *amt);
		} break;

		// No information yet for this or don't write anything by default
		default:
		{
		} break;
	}
}

void animation_begin_shape(animation_t* a, shape_t* s)
{
	/*
	animation_clear(a);

	u32 start_idx = 0;

	// Copy all point data over from shape in this call into points buffer
	for ( u32 i = 0; i < gs_dyn_array_size(s->paths); ++i )
	{
		animation_path_desc_t p_desc = { 0 };
		path_t* p 				= &s->paths[i];
		p_desc.start 			= start_idx;
		p_desc.end 				= start_idx + gs_dyn_array_size(p->points);
		p_desc.joint_style 		= p->joint_style;
		p_desc.end_cap_style 	= p->end_cap_style;

		for ( u32 pt = 0; pt < gs_dyn_array_size(p->points); ++pt ) {
			// gs_dyn_array_push(a->points, p->points[pt]);	
		}

		start_idx = p_desc.end;

		gs_dyn_array_push(a->path_descs, p_desc);
	}
	*/
}

// Animate path will submit to verts to be rendered
/*
void animation_path(gs_dyn_array(vert_t)* verts, animation_t* a, path_t* p)
{
	// For each point in path, calculate total length of path
	f32 total_length = 0.f;
	for ( u32 i = 0; i + 1 < gs_dyn_array_size(p->points); ++i )
	{
		// Calculate length from this vert to the next
		poly_point_t* p0 = &p->points[i];
		poly_point_t* p1 = &p->points[i + 1];
		total_length += gs_vec2_dist(p0->position, p1->position);
	}

	// If the end cap is joined, then need to also check against the last and first point
	if (gs_dyn_array_size(p->points) > 2 && p->end_cap_style == end_cap_style_joint)
	{
		poly_point_t* p0 = &p->points[gs_dyn_array_size(p->points) - 1];
		poly_point_t* p1 = &p->points[0];
		total_length += gs_vec2_dist(p0->position, p1->position);
	}

	// Total animation length of path will be based on anim.time / anim.total_time;
	f32 anim_len = total_length * (a->time / a->total_play_time);

	// Push on first point
	gs_dyn_array_push(a->points, p->points[0]);

	f32 cur_len = 0.f;
	for ( u32 i = 1; i < gs_dyn_array_size(p->points) + (p->end_cap_style == end_cap_style_joint ? 1 : 0); ++i ) {

		// Calculate length from this vert to the next
		poly_point_t* p0 = &p->points[i - 1];
		poly_point_t* p1 = &p->points[i % gs_dyn_array_size(p->points)];
		cur_len += gs_vec2_dist(p0->position, p1->position);
		if ( cur_len <= anim_len ) {
			// Add our point
			gs_dyn_array_push(a->points, *p1);
		}
		// Otherwise, we're done
		else {
			// Calculate closest point we can, and submit that instead
			f32 diff = cur_len - anim_len;	 // This value isn't correct...
			gs_vec2 n = gs_vec2_norm(gs_vec2_sub(p1->position, p0->position));
			gs_vec2 pos = gs_vec2_sub(p1->position, gs_vec2_scale(n, diff));
			gs_dyn_array_push(a->points, poly_point_create(pos, p1->color, p1->thickness));
			break;
		}
	}

	// Submit path to render
	poly_line_create(verts, a->points, gs_dyn_array_size(a->points), p->joint_style, 
		anim_len >= total_length ? p->end_cap_style : end_cap_style_butt, false);
}
*/

// void animation_shape(gs_dyn_array(vert_t)* verts, animation_t* a, shape_t* s)
// {
// 	// Loop through all paths in shape and submit for animation
// 	gs_for_range_i(gs_dyn_array_size(s->paths)) {
// 		animation_path(verts, a, &s->paths[i]);
// 	}
// }

// Does this submit points? Or just map the path's points to whatever the time is? Think that makes more sense.
// void animate_shape(animation_t* a, shape_t* s, f32 dt)
// {
// 	// For each path in shape, we're going to animate separately...

// 	// Incrememnt animation time
// 	a->time = gs_clamp(a->time + dt * a->animation_speed, 0.f, a->total_play_time);

// 	// For animating shapes, just iterate through path?
// 	// u32 pt_sz = (u32)gs_dyn_array_size(p->points);

// 	// Map this number from 0 -> pt_size based on time -> total_play_time;
// 	f32 scl = gs_interp_smooth_step(0.f, 1.f, a->time / a->total_play_time);

// 	// This doesn't work correctly...

// 	// Need to chop up path into animation steps, depending on animation.
// 	// Then need to 

// 	gs_mat4 scl_mtx = gs_mat4_scale((gs_vec3){scl, scl, 1.f});
// 	gs_for_range_i(gs_dyn_array_size(a->path.points)) {
// 		poly_point_t* pt = &a->path.points[i];
// 		gs_vec3 new_pt = gs_mat4_mul_vec3(scl_mtx, (gs_vec3){pt->position.x, pt->position.y, 0.f});
// 		pt->position.x = new_pt.x;
// 		pt->position.y = new_pt.y;
// 	}

// 	// This is destructive, of course, so I'm not entirely thrilled with this. Would be nice to be 
// 	// able to just animate them non-destructively.

// 	// So, obviously this won't work...Need to introduce new points to "grow" the path over time. yeesh.
// 	// Not sure how to translate this to work with paths, actually...

// 	// Set the path's point count to this value
// 	// gs_dyn_array_size(p->points) = ct;
// }

// Command buffer global var
_global gs_resource( gs_command_buffer ) g_cb = {0};
_global gs_resource( gs_shader ) g_shader = {0};
_global gs_resource( gs_vertex_buffer ) g_vb = {0};
_global gs_resource( gs_index_buffer ) g_ib = {0};
_global gs_resource( gs_uniform ) u_proj = {0};
_global gs_resource( gs_uniform ) u_view = {0};

_global gs_dyn_array( vert_t ) g_verts;
_global gs_dyn_array(animation_t) g_animations;
_global shape_t g_shape = {0};

_global const char* debug_shader_v_src = "\n"
"#version 330 core\n"
"layout (location = 0) in vec2 a_position;\n"
"layout (location = 1) in vec4 a_color;\n"
"out DATA {\n"
"	vec4 color;\n"
"} fs_out;\n"
"uniform mat4 u_proj;\n"
"uniform mat4 u_view;\n"
"void main() {\n"
" 	gl_Position = u_proj * u_view * vec4(a_position.xy, 0.0, 1.0);\n"
" 	fs_out.color = a_color;\n"
"}";

_global const char* debug_shader_f_src = "\n"
"#version 330 core\n"
"in DATA {\n"
"	vec4 color;\n"
"} fs_in;\n"
"out vec4 frag_color;\n"
"void main() {\n"
"	frag_color = fs_in.color;\n"
"}";

int main( int argc, char** argv )
{
	gs_application_desc app = {0};
	app.window_title 		= "Debug Drawing";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.init 				= &app_init;
	app.update 				= &app_update;
	app.shutdown 			= &app_shutdown;
	app.frame_rate 			= 60;

	// Construct internal instance of our engine
	gs_engine* engine = gs_engine_construct( app );

	// Run the internal engine loop until completion
	gs_result res = engine->run();

	// Check result of engine after exiting loop
	if ( res != gs_result_success ) 
	{
		gs_println( "Error: Engine did not successfully finish running." );
		return -1;
	}

	gs_println( "Gunslinger exited successfully." );

	return 0;	
}

// Here, we'll initialize all of our application data, which in this case is our graphics resources
gs_result app_init()
{
	// Cache instance of graphics api from engine
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Viewport
	const gs_vec2 ws = platform->window_size(platform->main_window());

	// Construct command buffer ( the command buffer is used to allow for immediate drawing at any point in our program )
	g_cb = gfx->construct_command_buffer();

	g_shader = gfx->construct_shader( debug_shader_v_src, debug_shader_f_src );

	// We're gonna draw quads...so...
	// Construct uniforms
	u_proj = gfx->construct_uniform( g_shader, "u_proj", gs_uniform_type_mat4 );
	u_view = gfx->construct_uniform( g_shader, "u_view", gs_uniform_type_mat4 );

	// Vertex data layout
	gs_vertex_attribute_type vertex_layout[] = {
		gs_vertex_attribute_float2,	// Position
		gs_vertex_attribute_float4  // Color
	};
	u32 layout_count = sizeof(vertex_layout) / sizeof(gs_vertex_attribute_type);

	// Construct vertex buffer objects
	g_vb = gfx->construct_vertex_buffer( vertex_layout, layout_count, NULL, 0 );

	g_verts = gs_dyn_array_new( vert_t );
	g_animations = gs_dyn_array_new(animation_t);
	g_shape = shape_create_new();

	// Let's get two animations for now
	gs_for_range_i(10) {
		gs_dyn_array_push(g_animations, animation_create_new(1.f, 0.5f));
	}

	animation_t* anim = NULL;
	shape_t* shape = &g_shape;

	const u32 num_cols = 10;
	const u32 num_rows = 10;
	const f32 grid_size = 500.f;
	const f32 ct = 1.0f;
	f32 cw = grid_size / (f32)num_cols;
	f32 ch = grid_size / (f32)num_rows;
	gs_vec2 chext = (gs_vec2){cw / 2.f - ct * 2.f, ch / 2.f - ct * 2.f};
	gs_vec2 ocp = (gs_vec2){cw / 2.f, ch / 2.f};

	gs_vqs trans = gs_vqs_default();

	gs_vec4 grid_col = (gs_vec4){0.7f, 0.5f, 0.2f, 1.f};
	gs_vec4 highlight_col = (gs_vec4){0.9f, 0.8f, 0.1f, 0.1f};

	f32 w = (cw - ct);
	f32 h = (ch - ct);


	// Initialize shape and animation for grid
	// Want each of these squares to animate in as well...
	shape_begin(shape);
	shape->xform.position = (gs_vec3){ws.x / 2.f, ws.y / 2.f, 0.f};
	shape_draw_square(shape, (gs_vec2){ocp.x - cw, ocp.y + ch}, chext, ct, grid_col, true);
	shape_draw_square(shape, (gs_vec2){ocp.x, ocp.y + ch}, chext, ct, grid_col, true);
	shape_draw_square(shape, (gs_vec2){ocp.x, ocp.y + ch}, chext, ct, grid_col, true);
	shape_draw_square(shape, (gs_vec2){ocp.x - cw * 2, ocp.y + ch}, chext, ct, grid_col, true);
	shape_draw_square(shape, (gs_vec2){ocp.x + cw * 2, ocp.y + ch * 2}, chext, ct, grid_col, true);
	shape_draw_grid(shape, (gs_vec2){0.f, 0.f}, grid_size, grid_size, num_rows, num_cols, 0.5f, (gs_vec4){1.f, 1.f, 1.f, 1.f});

	// Total play time should be determined by total number of tracks...
	// Animate one of the paths
	trans.scale = (gs_vec3){0.4f, 0.4f, 0.f};
	trans.position = (gs_vec3){grid_size * trans.scale.x + 20.f, grid_size * trans.scale.y + 20.f, 0.f};
	f32 fade_amt = 0.2f;
	anim = &g_animations[1];
	animation_set_shape(anim, shape);
	animation_add_action(anim, animation_action_type_walk_path, animation_ease_type_smooth_step, 5.f, NULL);
	animation_add_action(anim, animation_action_type_wait, animation_ease_type_lerp, 7.f, NULL);
	animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, 2.f, &fade_amt);
	animation_add_action(anim, animation_action_type_wait, animation_ease_type_lerp, 4.f, NULL);
	fade_amt = 0.0f;
	animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, 1.f, &fade_amt);
	animation_add_action(anim, animation_action_type_wait, animation_ease_type_lerp, 10.f, NULL);

	/*
		Center Cell Animation
	*/
	// Set up shape
	shape_begin(shape);
	shape->xform.position = (gs_vec3){ws.x / 2.f, ws.y / 2.f, 0.f};
	shape_draw_square(shape, ocp, chext, ct, (gs_vec4){grid_col.x, grid_col.y, grid_col.z, 0.1f}, true);

	fade_amt = 10.f;
	anim = &g_animations[0];
	animation_set_shape(anim, shape);
	animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_lerp, 5.f, NULL);
	animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, 3.f, &fade_amt);
	animation_add_action(anim, animation_action_type_wait, animation_ease_type_smooth_step, 10.f, NULL);

	trans = shape->xform;
	trans.scale = (gs_vec3){ 1.5f, 1.5f, 1.f };
	trans.position = (gs_vec3){ ws.x * 0.25f / trans.scale.x, (ws.y * 0.5f - ch) / trans.scale.y, 0.f };
	animation_add_action(anim, animation_action_type_transform, animation_ease_type_smooth_step, 3.f, &trans);

	/*
		// Highlight center cell animation
	*/
	shape_begin(shape);
	shape->xform.position = (gs_vec3){ws.x / 2.f, ws.y / 2.f, 0.f};
	shape_draw_square(shape, ocp, chext, 2.f, highlight_col, false);

	anim = &g_animations[2];
	fade_amt = 10.0f;
	animation_set_shape(anim, shape);
	animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_lerp, 15.f, NULL);
	animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, 0.5f, &fade_amt);
	fade_amt = 0.1f;
	animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, 0.5f, &fade_amt);
	fade_amt = 10.0f;
	animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, 0.5f, &fade_amt);
	fade_amt = 0.0f;
	animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, 3.0f, &fade_amt);


	return gs_result_success;
}

/*
void draw_G(gs_dyn_array(vert_t)* verts, gs_vec2 position, f32 thickness, gs_vec4 color)
{
	// Arc for G
	gs_dyn_array(poly_point_t) points = gs_dyn_array_new(poly_point_t);

	const f32 r = 30.f;
	const f32 start_angle = 0.f;
	const f32 end_angle = 320.f;
	const s32 num_segments = 30;
	f32 diff = end_angle - start_angle;

	// Draw line first (from position to outside radius)
	gs_dyn_array_push(points, poly_point_create((gs_vec2){position.x, position.y}, color, thickness));
	gs_dyn_array_push(points, poly_point_create((gs_vec2){position.x + r, position.y}, color, thickness));

	// Create arc
	f32 step = diff / (f32)num_segments;
	for ( f32 i = start_angle; i <= end_angle; i += step ) {
		f32 a = gs_deg_to_rad(i);
		poly_point_t p = poly_point_create(gs_vec2_add(position, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								color, thickness);
		gs_dyn_array_push(points, p);
	}
	// Push last angle on as well
	{
		f32 a = gs_deg_to_rad(end_angle);
		poly_point_t p = poly_point_create(gs_vec2_add(position, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								color, thickness);
		gs_dyn_array_push(points, p);
	}
	u32 pt_count = gs_dyn_array_size(points);

	// Pass in verts for creating poly line
	poly_line_create(verts, points, pt_count, joint_style_miter, end_cap_style_butt, false);

	gs_dyn_array_free(points);
}

void draw_o(gs_dyn_array(vert_t)* verts, gs_vec2 position, f32 thickness, gs_vec4 color)
{
	const f32 r = 20.f;
	draw_circle(verts, position, r, 30, thickness, color);
}

void draw_g(gs_dyn_array(vert_t)* verts, gs_vec2 position, f32 thickness, gs_vec4 color)
{
	const f32 start_angle = 0.f;
	const f32 end_angle = 170.f;
	const s32 num_segments = 20;
	const f32 r = 20.f;
	f32 diff = end_angle - start_angle;

	draw_circle(verts, position, r, num_segments, thickness, color);

	gs_dyn_array(poly_point_t) points = gs_dyn_array_new(poly_point_t);

	gs_vec2 ao = (gs_vec2){position.x, position.y + r};

	// Draw connecting shape
	gs_dyn_array_push(points, poly_point_create((gs_vec2){position.x + r, position.y - r - 5.f}, color, thickness));
	gs_dyn_array_push(points, poly_point_create((gs_vec2){position.x + r, ao.y - 1.f}, color, thickness));

	// Create arc
	f32 step = diff / (f32)num_segments;
	for ( f32 i = start_angle; i <= end_angle; i += step ) {
		f32 a = gs_deg_to_rad(i);
		poly_point_t p = poly_point_create(gs_vec2_add(ao, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								color, thickness);
		gs_dyn_array_push(points, p);
	}
	// Push last angle on as well
	{
		f32 a = gs_deg_to_rad(end_angle);
		poly_point_t p = poly_point_create(gs_vec2_add(ao, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								color, thickness);
		gs_dyn_array_push(points, p);
	}
	u32 pt_count = gs_dyn_array_size(points);

	// Pass in verts for creating poly line
	poly_line_create(verts, points, pt_count, joint_style_miter, end_cap_style_butt, false);

	gs_dyn_array_free(points);
}

void draw_l(gs_dyn_array(vert_t)* verts, gs_vec2 position, f32 thickness, gs_vec4 color)
{
	draw_line(verts, (gs_vec2){position.x, position.y - 30.f}, (gs_vec2){position.x, position.y + 30.f}, thickness, color);
}

void draw_e(gs_dyn_array(vert_t)* verts, gs_vec2 position, f32 thickness, gs_vec4 color)
{
	// Arc for e
	gs_dyn_array(poly_point_t) points = gs_dyn_array_new(poly_point_t);

	const f32 r = 20.f;
	const f32 start_angle = 40.f;
	const f32 end_angle = 341.f;
	const s32 num_segments = 30;
	f32 diff = end_angle - start_angle;

	// Create arc
	f32 step = diff / (f32)num_segments;
	for ( f32 i = start_angle; i <= end_angle; i += step ) {
		f32 a = gs_deg_to_rad(i);
		poly_point_t p = poly_point_create(gs_vec2_add(position, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								color, thickness);
		gs_dyn_array_push(points, p);
	}
	// Push last angle on as well
	{
		f32 a = gs_deg_to_rad(end_angle);
		poly_point_t p = poly_point_create(gs_vec2_add(position, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								color, thickness);
		gs_dyn_array_push(points, p);
	}

	// Draw line (from position to outside radius)
	gs_dyn_array_push(points, poly_point_create((gs_vec2){position.x - r, position.y + sin(gs_deg_to_rad(20.f)) * r}, color, thickness));
	gs_dyn_array_push(points, poly_point_create((gs_vec2){position.x + r, position.y - sin(gs_deg_to_rad(20.f)) * r}, color, thickness));

	u32 pt_count = gs_dyn_array_size(points);

	// Pass in verts for creating poly line
	poly_line_create(verts, points, pt_count, joint_style_miter, end_cap_style_butt, false);

	gs_dyn_array_free(points);
}

void draw_grid(gs_dyn_array(vert_t)* verts, f32 t)
{
	// Viewport
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	const gs_vec2 ws = platform->window_size(platform->main_window());

	f32 zoom = gs_clamp(100.f * (sin(t * 0.2f) * 0.5f + 0.5f), 20.f, 100.f);

	// Based on zoom, determine cell size, number of rows, number of columns

	// Need grid square size...

	// Draw grid across entire screen for shiggles
	for ( u32 r = 0; r < 40; ++r )
	{
		gs_vec2 sp = (gs_vec2){-10.f, r * zoom + 10.f};
		gs_vec2 ep = (gs_vec2){ws.x + 10.f, r * zoom + 10.f};
		draw_line(verts, sp, ep, 0.1f, (gs_vec4){1.f, 1.f, 1.f, 0.3f});
	}

	for ( u32 c = 0; c < 40; ++c )
	{
		gs_vec2 sp = (gs_vec2){c * zoom + 10.f, 0.f};
		gs_vec2 ep = (gs_vec2){c * zoom + 10.f, ws.y + 10.f};
		draw_line(verts, sp, ep, 0.1f, (gs_vec4){1.f, 1.f, 1.f, 0.3f});
	}

	// Put at idx 10, 10

	// Draw a bright yellow square up in dere
	gs_vec2 sqo = (gs_vec2){100.f, 100.f};
	gs_vec2 sqhe = (gs_vec2){45.f, 45.f};
	draw_square(verts, sqo, sqhe, 1.f, (gs_vec4){0.8f, 0.9f, 0.3f, 1.f});
}
*/

gs_result app_update()
{
	// Grab global instance of engine
	gs_engine* engine = gs_engine_instance();

	gs_timed_action( 10, 
	{
		gs_println( "frame: %.5f", engine->ctx.platform->time.frame );
	});

	// Api instances
	gs_graphics_i* gfx = engine->ctx.graphics;
	gs_platform_i* platform = engine->ctx.platform;

	// const f32 t = platform->elapsed_time() / 1000.f;
	const f32 dt = platform->time.delta;
	static f32 t = 0.f;
	t += 0.1f;

	// Viewport
	const gs_vec2 ws = platform->window_size(platform->main_window());

	// If we press the escape key, exit the application
	if ( platform->key_pressed( gs_keycode_esc ) )
	{
		return gs_result_success;
	}

	if ( platform->key_pressed( gs_keycode_a ) ) {
		anti_alias = !anti_alias;
	}

	if ( platform->key_pressed(gs_keycode_p)) {
		is_playing = !is_playing;
	}

	/*===============
	// Render scene
	================*/

	gs_for_range_i(gs_dyn_array_size(g_animations)) {
		animation_play(&g_verts, &g_animations[i], 0.1f);
	}

	// Update vertex data for frame
	gfx->update_vertex_buffer_data( g_vb, g_verts, gs_dyn_array_size(g_verts) * sizeof(vert_t) );

	// Set clear color and clear screen
	f32 clear_color[4] = { 0.05f, 0.05f, 0.05f, 1.f };
	gfx->set_view_clear( g_cb, clear_color );

#if (defined GS_PLATFORM_APPLE)
	gfx->set_view_port( g_cb, (u32)ws.x * 2, (u32)ws.y * 2 );
#else
	gfx->set_view_port( g_cb, (u32)ws.x, (u32)ws.y );
#endif

	// Bind shader
	gfx->bind_shader( g_cb, g_shader );

	// Bind uniforms
	gs_mat4 view_mtx = gs_mat4_translate((gs_vec3){0.f, 0.f, -3.f});
	gs_mat4 proj_mtx = gs_mat4_ortho(0.f, ws.x, ws.y, 0, 0.01f, 1000.f);

	gfx->bind_uniform( g_cb, u_view, &view_mtx );
	gfx->bind_uniform( g_cb, u_proj, &proj_mtx );

	// Bind vertex buffer
	gfx->bind_vertex_buffer( g_cb, g_vb );

	// Draw vertices
	gfx->draw( g_cb, 0, gs_dyn_array_size( g_verts ) );

	// Submit command buffer for rendering
	gfx->submit_command_buffer( g_cb ); 	// I suppose this COULD flush the command buffer altogether? ...

	 // Clear line data from array
	gs_dyn_array_clear( g_verts );

	return gs_result_in_progress;
}

gs_result app_shutdown()
{
	// Free all da things
	gs_dyn_array_free( g_verts );

	gs_println( "Goodbye, Gunslinger." );
	return gs_result_success;
}

/*
	path_start(ctx);
		path_set_end_cap_style(ctx, end_cap_style);
		path_add_points(ctx, line_pts, joint_style);
		path_set_connecting_joint_style(ctx, joint_style)
		path_add_points(ctx, bezier_curve_pts, joint_style);
	path_end(ctx);

	// Just need a simple way to connect two polylines with different joint styles with single end_cap style
	// Test is to do letters, like Google 'G'

	Want a way to animate arbitrary points, whether they be allowed to shrink, fade in, grow, animate the points over time, etc.

	// Animate a path

	typedef struct path
	{
		gs_dyn_array(point_t) points;
		end_cap_style_t end_cap_style;
	} path;

	path_begin(path);
		path_draw_line(path, start, end, thickness, color);
	path_end(path);

	animate_path(animation_type);

	animation_t anim0;

	// Does this mean a timeline will only operate on a single shape?
	// Want them to operate on shared data but transfer?

	// Animation can maintain its own timeline
	animation_set_shape(animation0, shape);
		animation_add_action(animation0, action_type0, start_time0);
		animation_add_action(animation0, action_type1, start_time1);
	animation_play(animation0);

	time_line_set_shape(timeline, shape);
	time_line_add_animation(timeline, anim0, start_time);
	time_line_add_animation(timeline, anim1, start_time);
	time_line_play(timeline);

	pts = time_line_get_data(timeline);
*/

