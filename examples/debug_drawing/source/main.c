#include <gs.h>

// Forward Decls.
gs_result app_init();		// Use to init your application
gs_result app_update();		// Use to update your application
gs_result app_shutdown();	// Use to shutdown your appliaction
void init_font();
void play_scene_one();		
void play_scene_two();
void play_scene_three();	
void play_scene_four();

// Ported to a c99 impl from: https://github.com/CrushedPixel/Polyline2D/

// Colors
#define white (gs_vec4){1.f, 1.f, 1.f, 1.f}
#define red (gs_vec4){1.f, 0.f, 0.f, 1.f}
#define green (gs_vec4){0.f, 1.f, 0.f, 1.f}
#define blue (gs_vec4){0.f, 0.f, 1.f, 1.f}
#define purple (gs_vec4){1.f, 0.f, 1.f, 1.f}

_global b32 anti_alias = true;
_global f32 anti_alias_scl = 2.f;
_global b32 is_playing = true;

#define frame_buffer_size (gs_vec2){1920, 1080}

_global f32 anim_mult = 1.f;

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

	// Need to create these manually
	gs_vec2 n = line_seg_normal(center);

	// Edge1.a will be a line segment from center + norm * a;
	p.edge1.a = poly_point_add(center.a, gs_vec2_scale(n, center.a.thickness));
	p.edge1.b = poly_point_add(center.b, gs_vec2_scale(n, center.b.thickness));
	// Edge1.b will be a line segment from center - norm * a;
	p.edge2.a = poly_point_sub(center.a, gs_vec2_scale(n, center.a.thickness));
	p.edge2.b = poly_point_sub(center.b, gs_vec2_scale(n, center.b.thickness));

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
			}

			if ( (i + 1) == gs_dyn_array_size(segments) 
				&& (end_cap_style == end_cap_style_square 
				|| end_cap_style == end_cap_style_butt) )
			{
				gs_vec2 enc = (gs_vec2){-en.y, en.x}; 
				poly_point_t e1c = poly_point_add(e1, gs_vec2_scale(enc, el));
				poly_point_t e2c = poly_point_add(e2, gs_vec2_scale(enc, el));
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

		start_point = end_point;
	}
}

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
	for ( f32 i = 0; i <= 360.f; i += step ) {
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

void path_draw_bezier_curve(path_t* p, gs_vec2 cp0, gs_vec2 cp1, gs_vec2 cp2, gs_vec2 cp3, u32 num_segments, f32 thickness, gs_vec4 color)
{
	f64 xu = 0.0, yu = 0.0; 	
	f64 step = num_segments < 3 ? 3 : 1.0 / num_segments;
	for ( f64 u = 0.0; u <= 1.0; u += step )
	{
		xu = pow(1.0 - u, 3) * cp0.x + 3 * u * pow(1.0 - u, 2) * cp1.x + pow(u, 2) * (1.0 - u) * cp2.x + 
			pow(u, 3) * cp3.x;
		yu = pow(1.0 - u, 3) * cp0.y + 3 * u * pow(1.0 - u, 2) * cp1.y + 3 * pow(u, 2) * (1.0 - u) * cp2.y + 
			pow(u, 3) * cp3.y;
		gs_dyn_array_push(p->points, poly_point_create(((gs_vec2){xu, yu}), color, thickness));
	}
}

void path_draw_arc(path_t* p, gs_vec2 origin, f32 r, f32 start_angle, f32 end_angle, s32 num_segments, f32 thickness, gs_vec4 color)
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

typedef struct vg_glyph_t
{
	gs_dyn_array(path_t) paths;
	f32 bearing_x;
	f32 advance_x;
	f32 bearing_y;
	f32 advance_y;
} vg_glyph_t;

typedef vg_glyph_t* vg_glyph_ptr;

// Hash table := key: u32, val: u32
gs_hash_table_decl( u32, vg_glyph_ptr, gs_hash_u32, gs_hash_key_comp_std_type );

typedef struct vg_font_t
{
	gs_hash_table(u32, vg_glyph_ptr) glyphs;
} vg_font_t;

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
		path_free(&s->paths[i]);
	}
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

void shape_draw_glyph(shape_t* s, gs_vec2 position, vg_glyph_t* glyph, f32 font_point, gs_vec4 color)
{
	f32 scale = font_point / 96.f;

	for ( u32 i = 0; i < gs_dyn_array_size(glyph->paths); ++i )
	{
		path_t* gp = &glyph->paths[i];
		path_t* p = shape_begin_path(s);

		// For each point in glyph path, need to add to da ting
		for ( u32 k = 0; k < gs_dyn_array_size(gp->points); ++k )
		{
			poly_point_t pt = gp->points[k];
			pt.position = gs_vec2_add(pt.position, position);
			pt.position = gs_vec2_add(pt.position, (gs_vec2){0.f, glyph->bearing_y});
			pt.position = gs_vec2_add(pt.position, (gs_vec2){glyph->bearing_x, glyph->bearing_y});

			// Scale font by transform
			gs_mat4 scl_mat = gs_mat4_scale((gs_vec3){scale, scale, scale});
			gs_vec4 np = gs_mat4_mul_vec4(scl_mat, (gs_vec4){pt.position.x, pt.position.y, 0.f, 1.f});
			pt.position = (gs_vec2){np.x, np.y};
			pt.color = color;

			gs_dyn_array_push(p->points, pt);
		}

		p->end_cap_style = gp->end_cap_style;
		p->joint_style = gp->joint_style;
	}
}

f32 shape_draw_text( shape_t* s, gs_vec2 origin, vg_font_t* font, const char* txt, f32 font_point, gs_vec4 color )
{
	gs_vec2 og = origin;

	u32 len = gs_string_length( txt );

	const f32 glyph_height = 80.f;
	f32 total_advance = 0.f;

	for ( u32 i = 0; i < len; ++i )
	{
		if (txt[i] == '\n') {
			origin.y += glyph_height;
			origin.x = og.x;
		}
		if (txt[i] == '\t') {
			vg_glyph_t* glyph = gs_hash_table_get(font->glyphs, ' ');
			if (glyph) {
				origin.x += glyph->advance_x * 2;
				total_advance += 45.f;
			}
		}

		vg_glyph_t* glyph = gs_hash_table_get(font->glyphs, txt[i]);
		if ( glyph )
		{
			shape_draw_glyph(s, origin, glyph, font_point, color );

			// Move x forward
			// origin.x += glyph->advance_x;

			// Monospaced glyph width
			origin.x += 45.f;
			total_advance += 45.f;
		}
	}

	return total_advance;
}

void shape_draw_triangle( shape_t* s, gs_vec2 a, gs_vec2 b, gs_vec2 c, f32 thickness, gs_vec4 color )
{
	path_t* p = shape_begin_path(s);
	path_draw_triangle(p, a, b, c, thickness, color);
	p->joint_style = joint_style_miter;
	p->end_cap_style = end_cap_style_joint;
}

void shape_draw_line( shape_t* s, gs_vec2 start, gs_vec2 end, f32 thickness, gs_vec4 color )
{
	path_t* p = shape_begin_path(s);
	path_draw_line(p, start, end, thickness, color);
}

void shape_draw_arrow(shape_t* s, gs_vec2 start, gs_vec2 end, f32 arrow_thickness, f32 arrow_length, f32 thickness, gs_vec4 color)
{
	shape_draw_line(s, start, end, thickness, color);	
	gs_vec2 norm = gs_vec2_norm(gs_vec2_sub(end, start));
	gs_vec2 tangent = (gs_vec2){-norm.y, norm.x};

	gs_vec2 ta = gs_vec2_add(end, gs_vec2_scale(tangent, arrow_thickness / 2.f));
	gs_vec2 tb = gs_vec2_add(end, gs_vec2_scale(tangent, -arrow_thickness / 2.f));
	gs_vec2 tc = gs_vec2_add(end, gs_vec2_scale(norm, arrow_length));
	shape_draw_triangle(s, ta, tb, tc, thickness, color);
}

void shape_draw_square( shape_t* s, gs_vec2 origin, gs_vec2 half_extents, f32 thickness, gs_vec4 color, b32 fill )
{
	if ( fill ) 
	{
		// Calculate thickness needed to fill square
		gs_vec2 sp = (gs_vec2){origin.x, origin.y - half_extents.y - thickness};
		gs_vec2 ep = (gs_vec2){origin.x, origin.y + half_extents.y + thickness};
		shape_draw_line(s, sp, ep, half_extents.x / 2.f + 4.f * thickness, color);	
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

	// For outside border, draw square
	shape_draw_square( s, origin, (gs_vec2){width/2.f, height/2.f}, thickness, color, false );

	const f32 t2 = thickness;

	// Draw grid across entire screen for shiggles
	for ( u32 r = 1; r < num_rows; ++r )
	{
		path_t* p = shape_begin_path(s);
		gs_vec2 sp = (gs_vec2){tl.x + t2, tl.y + r * cell_height + t2};
		gs_vec2 ep = (gs_vec2){tl.x + width - t2, tl.y + r * cell_height - t2};
		path_draw_line(p, sp, ep, thickness, color);
	}

	for ( u32 c = 1; c < num_cols; ++c )
	{
		path_t* p = shape_begin_path(s);
		gs_vec2 sp = (gs_vec2){tl.x + c * cell_width + t2, tl.y + t2};
		gs_vec2 ep = (gs_vec2){tl.x + c * cell_width - t2, tl.y + height - t2};
		path_draw_line(p, sp, ep, thickness, color);
	}
}

// Need a collection of glyphs, each with bearing/advance/etc.

// A vgFont will be a collection of shapes...I think?
/*
	shape_draw_text(shape_t* shape, vg_font_t*, const char* txt)
	{
		for (characters in txt) 
		{
			glyph_t g = vg_font_get_glyph(c);

			shape_draw_glyph(shape_t* shape, glyph_t* glyph);
		}
	}
*/


typedef enum animation_action_type_t
{
	animation_action_type_walk_path = 0,
	animation_action_type_transform,		// Takes in vqs transform and will 
	animation_action_type_wait,
	animation_action_type_disable_shape,
	animation_action_percentage_alpha,
	animation_action_set_alpha
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

animation_t animation_create_new()
{
	animation_t a = {0};
	a.animation_speed = 1.f;
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
	gs_dyn_array_clear(a->actions);
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
		a->time = gs_clamp(a->time + dt * a->animation_speed * anim_mult, 0.f, a->total_play_time);
		action->current_time += dt * a->animation_speed * anim_mult;
	}
}

void animation_set_shape(animation_t* a, shape_t* s)
{
	animation_reset(a);
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

		case animation_action_set_alpha:
		{
			// Grab transform from buffer
			f32 alpha = gs_byte_buffer_read(&a->action_data_buffer, f32);

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
							pt_m->color.w = gs_interp_smooth_step(pt->color.w, alpha, t);
						} break;	

						default:
						case animation_ease_type_lerp:
						{
							pt_m->color.w = gs_interp_linear(pt->color.w, alpha, t);
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
			if (!data) {
				gs_assert(false);
			}

			gs_vqs* xform = (gs_vqs*)data;
			gs_byte_buffer_write(&a->action_data_buffer, gs_vqs, *xform);
		} break;

		case animation_action_percentage_alpha: 
		{
			if (!data) {
				gs_assert(false);
			}

			f32* amt = (f32*)data;
			gs_byte_buffer_write(&a->action_data_buffer, f32, *amt);
		} break;

		case animation_action_set_alpha: 
		{
			if (!data) {
				gs_assert(false);
			}

			f32* amt = (f32*)data;
			gs_byte_buffer_write(&a->action_data_buffer, f32, *amt);
		} break;

		// No information yet for this or don't write anything by default
		default:
		{
		} break;
	}
}

// Command buffer global var
_global gs_resource( gs_command_buffer ) g_cb = {0};
_global gs_resource( gs_shader ) g_shader = {0};
_global gs_resource( gs_shader ) g_bb_shader = {0};
_global gs_resource( gs_vertex_buffer ) g_vb = {0};
_global gs_resource( gs_index_buffer ) g_ib = {0};
_global gs_resource( gs_uniform ) u_proj = {0};
_global gs_resource( gs_uniform ) u_view = {0};
_global gs_resource( gs_uniform ) u_tex = {0};
_global gs_resource( gs_texture ) 			g_rt = {0};
_global gs_resource( gs_frame_buffer ) 		g_fb = {0};
_global gs_resource( gs_vertex_buffer ) g_vbo = {0};
_global gs_resource( gs_index_buffer ) g_ibo = {0};

_global gs_dyn_array( vert_t ) g_verts;
_global gs_dyn_array(animation_t) g_animations;
_global shape_t g_shape = {0};
_global vg_font_t g_font = {0};

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

_global const char* bb_v_src = "\n"
"#version 330 core\n"
"layout (location = 0) in vec2 a_position;\n"
"layout (location = 1) in vec2 a_uv;\n"
"out DATA {\n"
"	vec2 uv;\n"
"} fs_out;\n"
"void main() {\n"
" 	gl_Position = vec4(a_position.xy, 0.0, 1.0);\n"
" 	fs_out.uv = a_uv;\n"
"}";

_global const char* bb_f_src = "\n"
"#version 330 core\n"
"in DATA {\n"
"	vec2 uv;\n"
"} fs_in;\n"
"uniform sampler2D u_tex;\n"
"out vec4 frag_color;\n"
"void main() {\n"
"	frag_color = texture(u_tex, fs_in.uv);\n"
"}";

_global b32 app_running = true;

void window_close_callback( void* win )
{
	app_running = false;
}

int main( int argc, char** argv )
{
	gs_application_desc app = {0};
	app.window_title 		= "Debug Drawing";
	app.window_width 		= 1920;
	app.window_height 		= 1080;
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

	// Set callback for window closing
	platform->set_window_close_callback(platform->main_window(), &window_close_callback);

	// Viewport
	const gs_vec2 ws = frame_buffer_size;

	// Construct command buffer ( the command buffer is used to allow for immediate drawing at any point in our program )
	g_cb = gfx->construct_command_buffer();

	g_shader = gfx->construct_shader( debug_shader_v_src, debug_shader_f_src );
	g_bb_shader = gfx->construct_shader(bb_v_src, bb_f_src);

	// We're gonna draw quads...so...
	// Construct uniforms
	u_proj = gfx->construct_uniform( g_shader, "u_proj", gs_uniform_type_mat4 );
	u_view = gfx->construct_uniform( g_shader, "u_view", gs_uniform_type_mat4 );
	u_tex = gfx->construct_uniform( g_bb_shader, "u_tex", gs_uniform_type_sampler2d);

	// Vertex data layout
	{
		gs_vertex_attribute_type vertex_layout[] = {
			gs_vertex_attribute_float2,	// Position
			gs_vertex_attribute_float4  // Color
		};
		u32 layout_count = sizeof(vertex_layout) / sizeof(gs_vertex_attribute_type);

		// Construct vertex buffer objects
		g_vb = gfx->construct_vertex_buffer( vertex_layout, layout_count, NULL, 0 );
	}

	// Vertex data layout for our mesh (for this shader, it's a single float2 attribute for position)
	{
		gs_vertex_attribute_type layout[] = 
		{
			gs_vertex_attribute_float2,
			gs_vertex_attribute_float2
		};
		// Count of our vertex attribute array
		u32 layout_count = sizeof( layout ) / sizeof( gs_vertex_attribute_type ); 

		// Vertex data for triangle
		f32 v_data[] = 
		{
			// Positions  UVs
			-1.0f, -1.0f,  0.0f, 0.0f,	// Top Left
			 1.0f, -1.0f,  1.0f, 0.0f,	// Top Right 
			-1.0f,  1.0f,  0.0f, 1.0f,  // Bottom Left
			 1.0f,  1.0f,  1.0f, 1.0f   // Bottom Right
		};

		u32 i_data[] = 
		{
			0, 2, 3,
			3, 1, 0
		};

		// Construct vertex buffer
		g_vbo = gfx->construct_vertex_buffer( layout, layout_count, v_data, sizeof(v_data) );
		// Construct index buffer
		g_ibo = gfx->construct_index_buffer( i_data, sizeof(i_data) );
	}

	g_verts = gs_dyn_array_new( vert_t );
	g_animations = gs_dyn_array_new(animation_t);
	g_shape = shape_create_new();

	init_font();

	// Let's get two animations for now

	play_scene_one();

	// Construct texture resource from GPU
	gs_texture_parameter_desc t_desc = gs_texture_parameter_desc_default();
	t_desc.texture_format = gs_texture_format_rgba8;
	t_desc.mag_filter = gs_linear;
	t_desc.min_filter = gs_linear;
	t_desc.generate_mips = true;
	t_desc.width = frame_buffer_size.x;
	t_desc.height = frame_buffer_size.y;
	t_desc.num_comps = 4;
	t_desc.data = NULL;

	// Construct target for offscreen rendering
	g_rt = gfx->construct_texture( t_desc );

	// Construct frame buffer
	g_fb = gfx->construct_frame_buffer( g_rt );


	return gs_result_success;
}

gs_result app_update()
{
	if (!app_running) {
		return gs_result_success;
	}
	
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

	if ( platform->key_pressed(gs_keycode_one)) {
		play_scene_one();
	}

	if ( platform->key_pressed(gs_keycode_two)) {
		play_scene_two();
	}

	if ( platform->key_pressed(gs_keycode_three)) {
		play_scene_three();
	}

	if ( platform->key_pressed(gs_keycode_four)) {
		play_scene_four();
	}

	/*===============
	// Render scene
	================*/

	gs_for_range_i(gs_dyn_array_size(g_animations)) {
		animation_play(&g_verts, &g_animations[i], 0.1f);
	}

	// 
	// Bind our render target and render offscreen
	gfx->bind_frame_buffer( g_cb, g_fb );
	{
		// Bind frame buffer attachment for rendering
		gfx->set_frame_buffer_attachment( g_cb, g_rt, 0 );

		// Update vertex data for frame
		gfx->update_vertex_buffer_data( g_vb, g_verts, gs_dyn_array_size(g_verts) * sizeof(vert_t) );

		// Set clear color and clear screen
		f32 clear_color[4] = { 0.05f, 0.05f, 0.05f, 1.f };
		gfx->set_view_clear( g_cb, clear_color );
		gfx->set_view_port( g_cb, frame_buffer_size.x, frame_buffer_size.y );

		// Bind shader
		gfx->bind_shader( g_cb, g_shader );

		// Bind uniforms
		gs_mat4 view_mtx = gs_mat4_translate((gs_vec3){0.f, 0.f, -3.f});
		gs_mat4 proj_mtx = gs_mat4_ortho(0.f, frame_buffer_size.x, frame_buffer_size.y, 0.f, 0.01f, 1000.f);

		gfx->bind_uniform( g_cb, u_view, &view_mtx );
		gfx->bind_uniform( g_cb, u_proj, &proj_mtx );

		// Bind vertex buffer
		gfx->bind_vertex_buffer( g_cb, g_vb );

		// Draw vertices
		gfx->draw( g_cb, 0, gs_dyn_array_size( g_verts ) );
	}
	gfx->unbind_frame_buffer( g_cb );

	// Backbuffer pres.
	{
			// This is to handle mac's retina high dpi for now until I fix that internally.
		#if (defined GS_PLATFORM_APPLE)
			gfx->set_view_port( g_cb, (s32)ws.x * 2.f, (s32)ws.y * 2.f );
		#else
			gfx->set_view_port( g_cb, (s32)ws.x, (s32)ws.y );
		#endif

		// Set clear color and clear screen
		f32 clear_color[4] = { 0.2f, 0.2f, 0.2f, 1.f };
		gfx->set_view_clear( g_cb, clear_color );

		// Bind shader
		gfx->bind_shader( g_cb, g_bb_shader );
		gfx->bind_texture( g_cb, u_tex, g_rt, 0 );

		// Bind vertex buffer
		gfx->bind_vertex_buffer( g_cb, g_vbo );
		gfx->bind_index_buffer( g_cb, g_ibo );

		// Draw vertices
		// gfx->draw( g_cb, 0, gs_dyn_array_size( g_verts ) );
		gfx->draw_indexed( g_cb, 6 );
	}

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

vg_glyph_t* glyph_create_new_ptr()
{
	vg_glyph_t* glyph = gs_malloc(sizeof(vg_glyph_t));
	memset(glyph, 0, sizeof(vg_glyph_t));
	glyph->paths = gs_dyn_array_new(path_t);
	glyph->advance_x = 50.f;						// This is determined based on code point...so...
	return glyph;
}

#define glyph_thickness 0.1f

vg_glyph_t* __glyph_create_A()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 53.f;
	glyph->bearing_y = 15.f;
	glyph->bearing_x = -5.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	p->joint_style = joint_style_miter;
	path_draw_line(p, (gs_vec2){-25.f, 0.f}, (gs_vec2){0.f, -55.f}, glyph_thickness, white);
	path_draw_line(p, (gs_vec2){0.f, -55.f}, (gs_vec2){25.f, 0.f}, glyph_thickness, white);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	path_draw_line(p, (gs_vec2){-15.f, -15.f}, (gs_vec2){15.f, -15.f}, glyph_thickness, white);

	return glyph;
}

vg_glyph_t* __glyph_create_B()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 55.f;
	glyph->bearing_y = -15.f;
	glyph->bearing_x = -18.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	p->joint_style = joint_style_miter;
	p->end_cap_style = end_cap_style_joint;
	f32 scl = 5.f;

	path_draw_line(p, (gs_vec2){0.f, 0.f}, gs_vec2_scale((gs_vec2){0.f, 6.f}, scl), glyph_thickness, white);

	path_draw_bezier_curve(p, gs_vec2_scale((gs_vec2){0.f, 0.f}, scl), gs_vec2_scale((gs_vec2){7.f, 0.f}, scl), 
		gs_vec2_scale((gs_vec2){22.f, 6.f}, scl), gs_vec2_scale((gs_vec2){0.f, 6.f}, scl), 30, glyph_thickness, white);

	gs_vec2 offset = (gs_vec2){0.f, 6.f * scl};

	path_draw_bezier_curve(p, 
		gs_vec2_add(gs_vec2_scale((gs_vec2){0.f, 0.f}, scl), offset), 
		gs_vec2_add(gs_vec2_scale((gs_vec2){10.5f, -1.f}, scl), offset), 
		gs_vec2_add(gs_vec2_scale((gs_vec2){22.5f, 8.f}, scl), offset), 
		gs_vec2_add(gs_vec2_scale((gs_vec2){0.f, 6.f}, scl), offset), 
		30, glyph_thickness, white);

	return glyph;
}

vg_glyph_t* __glyph_create_C()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 59.f;
	glyph->bearing_y = 0.f;
	glyph->bearing_x = 10.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	const f32 thickness = glyph_thickness;
	const gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	p->joint_style = joint_style_miter;
	p->end_cap_style = end_cap_style_butt;
	f32 scl = 40.f;
	f32 r = 0.8f;

	path_draw_arc(p, position, r * scl, 40.f, 320.f, 50, thickness, color);

	// path_draw_bezier_curve(p, gs_vec2_scale((gs_vec2){1.f, 0.f}, scl), gs_vec2_scale((gs_vec2){-6.f, 0.f}, scl), 
	// 	gs_vec2_scale((gs_vec2){-15.f, 6.f}, scl), gs_vec2_scale((gs_vec2){1.f, 6.f}, scl), 30, 2.f, white);

	return glyph;
}

vg_glyph_t* __glyph_create_D()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 70.f;
	glyph->bearing_y = -15.f;
	glyph->bearing_x = -15.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	p->joint_style = joint_style_miter;
	p->end_cap_style = end_cap_style_joint;
	f32 scl = 10.f;

	path_draw_line(p, (gs_vec2){0.f, 0.f}, gs_vec2_scale((gs_vec2){0.f, 6.f}, scl), glyph_thickness, white);

	path_draw_bezier_curve(p, gs_vec2_scale((gs_vec2){0.f, 0.f}, scl), gs_vec2_scale((gs_vec2){7.f, 0.f}, scl), 
		gs_vec2_scale((gs_vec2){20.f, 6.f}, scl), gs_vec2_scale((gs_vec2){0.f, 6.f}, scl), 30, glyph_thickness, white);

	return glyph;
}

vg_glyph_t* __glyph_create_E()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 60.f;
	glyph->bearing_y = -13.f;
	glyph->bearing_x = -24.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	f32 scl = 55.f;

	path_draw_line(p, gs_vec2_scale((gs_vec2){0.7f, 0.f}, scl), 
					  gs_vec2_scale((gs_vec2){0.0f, 0.f}, scl), glyph_thickness, white);

	path_draw_line(p, gs_vec2_scale((gs_vec2){0.0f, 0.f}, scl), 
					  gs_vec2_scale((gs_vec2){0.0f, 1.f}, scl), glyph_thickness, white);

	path_draw_line(p, gs_vec2_scale((gs_vec2){0.0f, 1.f}, scl), 
					  gs_vec2_scale((gs_vec2){0.7f, 1.f}, scl), glyph_thickness, white);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	path_draw_line(p, gs_vec2_scale((gs_vec2){0.0f, 0.4f}, scl), 
					  gs_vec2_scale((gs_vec2){0.6f, 0.4f}, scl), glyph_thickness, white);

	return glyph;
}

vg_glyph_t* __glyph_create_F()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 60.f;
	glyph->bearing_y = 16.5f;
	glyph->bearing_x = -24.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	f32 scl = 60.f;

	path_draw_line(p, gs_vec2_scale((gs_vec2){0.0f, 0.f}, scl), 
					  gs_vec2_scale((gs_vec2){0.0f, -1.f}, scl), glyph_thickness, white);

	path_draw_line(p, gs_vec2_scale((gs_vec2){0.0f, -1.f}, scl), 
					  gs_vec2_scale((gs_vec2){0.7f, -1.f}, scl), glyph_thickness, white);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	path_draw_line(p, gs_vec2_scale((gs_vec2){0.0f, -0.6f}, scl), 
					  gs_vec2_scale((gs_vec2){0.6f, -0.6f}, scl), glyph_thickness, white);

	return glyph;
}

vg_glyph_t* __glyph_create_G()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 65.f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const f32 r = 30.f;
	const f32 start_angle = 0.f;
	const f32 end_angle = 320.f;
	const s32 num_segments = 40;
	f32 diff = end_angle - start_angle;

	gs_vec2 position = {0.f, 0.f};
	f32 thickness = glyph_thickness;

	// Draw line first (from position to outside radius)
	gs_dyn_array_push(p->points, poly_point_create((gs_vec2){position.x, position.y}, white, thickness));
	gs_dyn_array_push(p->points, poly_point_create((gs_vec2){position.x + r, position.y}, white, thickness));

	// Create arc
	f32 step = diff / (f32)num_segments;
	for ( f32 i = start_angle; i <= end_angle; i += step ) {
		f32 a = gs_deg_to_rad(i);
		poly_point_t pt = poly_point_create(gs_vec2_add(position, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								white, thickness);
		gs_dyn_array_push(p->points, pt);
	}
	// Push last angle on as well
	{
		f32 a = gs_deg_to_rad(end_angle);
		poly_point_t pt = poly_point_create(gs_vec2_add(position, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								white, thickness);
		gs_dyn_array_push(p->points, pt);
	}

	return glyph;
}

vg_glyph_t* __glyph_create_H()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 60.f;
	glyph->bearing_y = 16.5f;
	glyph->bearing_x = -24.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	f32 scl = 60.f;

	path_draw_line(p, gs_vec2_scale((gs_vec2){0.0f, 0.f}, scl), 
					  gs_vec2_scale((gs_vec2){0.0f, -1.f}, scl), glyph_thickness, white);

	path_draw_line(p, gs_vec2_scale((gs_vec2){0.0f, -1.f}, scl), 
					  gs_vec2_scale((gs_vec2){0.7f, -1.f}, scl), glyph_thickness, white);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	path_draw_line(p, gs_vec2_scale((gs_vec2){0.0f, -0.6f}, scl), 
					  gs_vec2_scale((gs_vec2){0.6f, -0.6f}, scl), glyph_thickness, white);

	return glyph;
}

vg_glyph_t* __glyph_create_P()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 30.f;
	glyph->bearing_y = -14.f;
	glyph->bearing_x = -20.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	p->joint_style = joint_style_miter;
	p->end_cap_style = end_cap_style_joint;
	f32 scl = 5.f;

	path_draw_line(p, (gs_vec2){0.f, 0.f}, gs_vec2_scale((gs_vec2){0.f, 12.f}, scl), glyph_thickness, white);

	path_draw_bezier_curve(p, gs_vec2_scale((gs_vec2){0.f, 0.f}, scl), gs_vec2_scale((gs_vec2){10.f, 0.f}, scl), 
		gs_vec2_scale((gs_vec2){22.f, 6.f}, scl), gs_vec2_scale((gs_vec2){0.f, 6.f}, scl), 30, glyph_thickness, white);

	return glyph;
}

vg_glyph_t* __glyph_create_a()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	glyph->bearing_y = -12.f;
	glyph->bearing_x = 0.f;
	glyph->advance_x = 25.f;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const gs_vec4 color = white;
	const f32 thickness = glyph_thickness;
	const f32 scl = 23.f;
	const f32 r = 0.5f;

	path_draw_arc(p, gs_vec2_add(position, (gs_vec2){0.05f * scl, 0.05f * scl}), r * 0.9f * scl, 
		220.f, 360.f, 40, thickness, white);

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){r * scl, 0.05f * scl});
	gs_vec2 elp = gs_vec2_add(slp, (gs_vec2){0.f, 0.9f * scl});
	path_draw_line(p, slp, elp, thickness, color);
	// ae
	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 n1 = gs_vec2_norm((gs_vec2){-150.7f, 600.5f});
	gs_vec2 n2 = gs_vec2_norm((gs_vec2){-150.1f, -250.9f});

	slp = gs_vec2_sub(elp, (gs_vec2){0.f, 0.28f * scl});
	elp = gs_vec2_sub(slp, (gs_vec2){0.f, 0.4f * scl});
	// path_draw_bezier_curve(p, 
	// 	slp, 
	// 	(gs_vec2){n1.x * scl * 5.f * 1.2f, n1.y * scl * 1.2f * 1.2f},
	// 	gs_vec2_scale(n2, scl * 1.2f),
	// 	elp, 
	// 	30, glyph_thickness, white);
	 path_draw_arc(p, gs_vec2_add(position, (gs_vec2){0.f, 0.5f * scl}), r  * scl,
		0.f, 360.f, 40, thickness, white);

	return glyph;
}

vg_glyph_t* __glyph_create_b()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	glyph->bearing_y = -15.f;
	glyph->bearing_x = -14.f;
	glyph->advance_x = 50.f;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const gs_vec4 color = white;
	const f32 thickness = glyph_thickness;
	const f32 scl = 30.f;
	const f32 r = 0.5f;

	// b

	gs_vec2 slp = gs_vec2_sub(position, (gs_vec2){0.f, 0.7f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.f, 1.f * scl});
	path_draw_line(p, slp, elp, thickness, color);
	// ae
	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 n1 = gs_vec2_norm((gs_vec2){150.7f, 400.5f});
	gs_vec2 n2 = gs_vec2_norm((gs_vec2){200.1f, -400.9f});

	slp = gs_vec2_sub(elp, (gs_vec2){0.f, 0.5f * scl});
	elp = gs_vec2_sub(slp, (gs_vec2){0.f, 0.4f * scl});
	path_draw_circle(p, gs_vec2_add(slp, (gs_vec2){r * scl, 0.f}), r * scl, 75, thickness, color);

	// path_draw_bezier_curve(p, 
	// 	slp, 
	// 	(gs_vec2){n1.x * scl * 5.f * 1.2f, n1.y * scl * 1.2f * 1.2f},
	// 	gs_vec2_scale(n2, scl * 1.4f),
	// 	elp, 
	// 	30, 2.f, white);

	return glyph;
}

vg_glyph_t* __glyph_create_c()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	glyph->bearing_y = -8.f;
	glyph->bearing_x = 0.f;
	glyph->advance_x = 35.f;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const gs_vec4 color = white;
	const f32 thickness = glyph_thickness;
	const f32 scl = 30.f;
	const f32 r = 0.5f;

	// c
	path_draw_arc(p, position, r * scl, 40.f, 320.f, 50, thickness, color);

	return glyph;
}

vg_glyph_t* __glyph_create_d()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	glyph->bearing_y = -15.f;
	glyph->bearing_x = 14.f;
	glyph->advance_x = 35.f;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const gs_vec4 color = white;
	const f32 thickness = glyph_thickness;
	const f32 scl = 30.f;
	const f32 r = 0.5f;

	// b

	gs_vec2 slp = gs_vec2_sub(position, (gs_vec2){0.f, 0.81f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.f, 1.f * scl});
	path_draw_line(p, slp, elp, thickness, color);
	// ae
	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 n1 = gs_vec2_norm((gs_vec2){-200.7f, 500.5f});
	gs_vec2 n2 = gs_vec2_norm((gs_vec2){-200.1f, -600.9f});

	slp = gs_vec2_sub(elp, (gs_vec2){0.f, 0.5f * scl});
	elp = gs_vec2_sub(slp, (gs_vec2){0.f, 0.4f * scl});
	path_draw_circle(p, gs_vec2_sub(slp, (gs_vec2){r * scl, 0.f}), r * scl, 75, thickness, color);

	// slp = gs_vec2_sub(elp, (gs_vec2){0.f, 0.2f * scl});
	// elp = gs_vec2_sub(slp, (gs_vec2){0.f, 0.6f * scl});
	// path_draw_bezier_curve(p, 
	// 	slp, 
	// 	(gs_vec2){n1.x * scl * 5.f * 1.2f, n1.y * scl * 1.2f * 1.2f},
	// 	gs_vec2_scale(n2, scl * 1.2f),
	// 	elp, 
	// 	30, glyph_thickness, white);

	// path_draw_bezier_curve(p, 
	// 	slp, 
	// 	(gs_vec2){n1.x * scl * 5.f * 1.2f, n1.y * scl * 1.2f * 1.2f},
	// 	gs_vec2_scale(n2, scl * 1.4f),
	// 	elp, 
	// 	30, 2.f, white);

	return glyph;
}

vg_glyph_t* __glyph_create_e()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	glyph->bearing_y = -8.f;
	glyph->bearing_x = 5.f;
	glyph->advance_x = 40.f;

	const f32 scl = 30.f;
	const f32 r = 0.5f;
	const f32 start_angle = 40.f;
	const f32 end_angle = 360.f;
	const s32 num_segments = 30;
	const f32 thickness = glyph_thickness;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	gs_vec4 color = white;
	f32 diff = end_angle - start_angle;

	// Create arc
	f32 step = diff / (f32)num_segments;
	for ( f32 i = start_angle; i <= end_angle; i += step ) {
		f32 a = gs_deg_to_rad(i);
		poly_point_t pt = poly_point_create(gs_vec2_add(position, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r * scl)), 
								color, thickness);
		gs_dyn_array_push(p->points, pt);
	}
	// Push last angle on as well
	{
		f32 a = gs_deg_to_rad(end_angle);
		poly_point_t pt = poly_point_create(gs_vec2_add(position, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r * scl)), 
								color, thickness);
		gs_dyn_array_push(p->points, pt);
	}

	// Draw line (from position to outside radius)
	gs_dyn_array_push(p->points, poly_point_create((gs_vec2){position.x - r * scl, position.y}, color, thickness));
	gs_dyn_array_push(p->points, poly_point_create((gs_vec2){position.x + r * scl, position.y}, color, thickness));

	p->joint_style = joint_style_miter;
	p->end_cap_style = end_cap_style_butt;

	return glyph;
}

vg_glyph_t* __glyph_create_f()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	glyph->bearing_y = 0.f;
	glyph->bearing_x = -2.f;
	glyph->advance_x = 35.f;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const gs_vec4 color = white;
	const f32 thickness = glyph_thickness;
	const f32 scl = 30.f;
	const f32 r = 0.5f;

	// f

	gs_vec2 slp = position;
	gs_vec2 elp = gs_vec2_sub(position, (gs_vec2){0.f, 1.f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	// Draw arc at top
	path_draw_arc(p, gs_vec2_add(elp, (gs_vec2){r * scl, 0.f}), r * scl, 
		180.f, 270.f, 40, thickness, white);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	// slp = gs_vec2_sub(elp, (gs_vec2){0.2f * scl, 0.65f * scl});
	slp = gs_vec2_sub(position, (gs_vec2){0.3f * scl, 0.95f * scl});
	elp = gs_vec2_add(slp, (gs_vec2){0.6f * scl, 0.0f * scl});
	// path_draw_circle(p, gs_vec2_sub(slp, (gs_vec2){r * scl, 0.f}), r * scl, 75, thickness, color);
	path_draw_line(p, slp, elp, thickness, color);

	// slp = gs_vec2_sub(elp, (gs_vec2){0.f, 0.2f * scl});
	// elp = gs_vec2_sub(slp, (gs_vec2){0.f, 0.6f * scl});
	// path_draw_bezier_curve(p, 
	// 	slp, 
	// 	(gs_vec2){n1.x * scl * 5.f * 1.2f, n1.y * scl * 1.2f * 1.2f},
	// 	gs_vec2_scale(n2, scl * 1.2f),
	// 	elp, 
	// 	30, glyph_thickness, white);

	// path_draw_bezier_curve(p, 
	// 	slp, 
	// 	(gs_vec2){n1.x * scl * 5.f * 1.2f, n1.y * scl * 1.2f * 1.2f},
	// 	gs_vec2_scale(n2, scl * 1.4f),
	// 	elp, 
	// 	30, 2.f, white);

	return glyph;
}

vg_glyph_t* __glyph_create_g()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 45.f;
	glyph->bearing_y = -8.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const f32 start_angle = 0.f;
	const f32 end_angle = 170.f;
	const s32 num_segments = 20;
	const f32 scl = 30.f;
	const f32 r = 0.5f;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	f32 diff = end_angle - start_angle;
	gs_vec2 position = (gs_vec2){0.f, 0.f};

	path_draw_circle(p, position, r * scl, num_segments, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 ao = (gs_vec2){position.x, position.y + r * scl};

	// Draw connecting shape
	gs_dyn_array_push(p->points, poly_point_create((gs_vec2){position.x + r * scl, position.y - r * scl + 3.f}, color, thickness));
	gs_dyn_array_push(p->points, poly_point_create((gs_vec2){position.x + r * scl, ao.y - 1.f}, color, thickness));

	// Create arc
	f32 step = diff / (f32)num_segments;
	for ( f32 i = start_angle; i <= end_angle; i += step ) {
		f32 a = gs_deg_to_rad(i);
		poly_point_t pt = poly_point_create(gs_vec2_add(ao, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r * scl)), 
								color, thickness);
		gs_dyn_array_push(p->points, pt);
	}
	// Push last angle on as well
	{
		f32 a = gs_deg_to_rad(end_angle);
		poly_point_t pt = poly_point_create(gs_vec2_add(ao, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r * scl)), 
								color, thickness);
		gs_dyn_array_push(p->points, pt);
	}

	return glyph;
}

vg_glyph_t* __glyph_create_h()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = 0.f;
	glyph->bearing_x = -10.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 40.f;
	const f32 r = 0.3f;

	// h

	// Draw line
	gs_vec2 slp = position;
	gs_vec2 elp = gs_vec2_sub(position, (gs_vec2){0.f, 1.2f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	slp = gs_vec2_add(position, (gs_vec2){2.f * r * scl, 0.f});
	elp = gs_vec2_sub(slp, (gs_vec2){0.f, 0.5f * scl});

	// // Draw arc at top
	path_draw_arc(p, gs_vec2_sub(elp, (gs_vec2){r * scl, 0.f }), r * scl, 180.f, 360.f, 50, thickness, white);

	// // Draw separate line from bottom
	path_draw_line(p, slp, elp, thickness, color);


	return glyph;
}

vg_glyph_t* __glyph_create_i()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = -1.f;
	glyph->bearing_x = 0.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 70.f;
	const f32 r = 0.2f;

	// ih

	// Draw line
	gs_vec2 slp = gs_vec2_sub(position, (gs_vec2){0.1f * scl, 0.4f * scl});
	gs_vec2 elp = gs_vec2_sub(position, (gs_vec2){0.f, 0.4f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = position;
	path_draw_line(p, slp, elp, thickness, color);

	p->joint_style = joint_style_miter;

	slp = gs_vec2_sub(position, (gs_vec2){0.12f * scl, 0.f});
	elp = gs_vec2_add(position, (gs_vec2){0.12f * scl, 0.f});
	path_draw_line(p, slp, elp, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	slp = gs_vec2_sub(position, (gs_vec2){0.f * scl, 0.55f * scl});
	elp = gs_vec2_sub(position, (gs_vec2){0.f, 0.45f * scl});

	// Draw dot
	slp = gs_vec2_sub(position, (gs_vec2){0.f * scl, 0.65f * scl});
	elp = gs_vec2_sub(position, (gs_vec2){0.f, 0.55f * scl});
	path_draw_line(p, slp, elp, thickness * 3.f, color);

	return glyph;
}

vg_glyph_t* __glyph_create_j()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 15.f;
	glyph->bearing_y = 0.f;
	glyph->bearing_x = 2.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 70.f;
	const f32 r = 0.1f;

	// hij

	gs_vec2 slp = gs_vec2_sub(position, (gs_vec2){0.1f * scl, 0.4f * scl});
	gs_vec2 elp = gs_vec2_sub(position, (gs_vec2){0.f, 0.4f * scl});
	// Draw line
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_add(position, (gs_vec2){0.f, 0.05f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	// Draw arc at bottom
	path_draw_arc(p, gs_vec2_sub(elp, (gs_vec2){r * scl, 0.f }), r * scl, 0.f, 180.f, 50, thickness, white);

	p->joint_style = joint_style_miter;

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	// Draw dot
	slp = gs_vec2_sub(position, (gs_vec2){0.f * scl, 0.55f * scl});
	elp = gs_vec2_sub(position, (gs_vec2){0.f, 0.45f * scl});
	path_draw_line(p, slp, elp, thickness * 3.f, color);

	return glyph;
}

vg_glyph_t* __glyph_create_k()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = 0.f;
	glyph->bearing_x = -5.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 55.f;
	const f32 r = 0.1f;

	// hijk

	gs_vec2 slp = position;
	gs_vec2 elp = gs_vec2_sub(position, (gs_vec2){0.f, 0.8f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	// Draw connect
	slp = gs_vec2_sub(position, (gs_vec2){-0.25f * scl, 0.6f * scl});
	elp = gs_vec2_sub(position, (gs_vec2){-0.02f * scl, 0.4f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_add(position, (gs_vec2){0.3f * scl, 0.f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	p->joint_style = joint_style_bevel;

	return glyph;
}

vg_glyph_t* __glyph_create_l()
{
	// jl
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 30.f;
	glyph->bearing_y = 0.f;
	glyph->bearing_x = -5.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 90.f;
	const f32 r = 0.1f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_sub(position, (gs_vec2){0.1f * scl, 0.4f * scl});
	gs_vec2 elp = gs_vec2_sub(position, (gs_vec2){0.f, 0.4f * scl});
	// Draw line
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_sub(position, (gs_vec2){0.f, 0.1f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	p->joint_style = joint_style_miter;

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	slp = elp;
	elp = position;
	path_draw_line(p, slp, elp, thickness, color);

	// Draw arc at bottom
	slp = elp;
	elp = gs_vec2_add(slp, (gs_vec2){0.1f * scl, 0.f});
	path_draw_line(p, slp, elp, thickness, color);

	p->joint_style = joint_style_round;

	return glyph;
}

vg_glyph_t* __glyph_create_m()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = 0.f;
	glyph->bearing_x = -12.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 65.f;
	const f32 r = 0.15f;

	// hijm

	gs_vec2 slp = gs_vec2_sub(position, (gs_vec2){0.f, 0.5f * scl});
	gs_vec2 elp = position;
	// Draw line
	path_draw_line(p, slp, elp, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	slp = gs_vec2_add(slp, (gs_vec2){r * scl, 0.2f * scl});
	path_draw_arc(p, slp, r * scl, 180.f, 360.f, 50, thickness, white);

	slp = gs_vec2_add(slp, (gs_vec2){r * scl, 0.f});
	elp = (gs_vec2){slp.x, position.y};
	path_draw_line(p, slp, elp, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	slp = gs_vec2_add(slp, (gs_vec2){r * scl, 0.f});
	path_draw_arc(p, slp, r * scl, 180.f, 360.f, 50, thickness, white);

	slp = gs_vec2_add(slp, (gs_vec2){r * scl, 0.f});
	elp = (gs_vec2){slp.x, position.y};
	path_draw_line(p, slp, elp, thickness, color);


	return glyph;
}

vg_glyph_t* __glyph_create_n()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 28.f;
	glyph->bearing_y = 0.f;
	glyph->bearing_x = -3.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 65.f;
	const f32 r = 0.15f;

	// hijm

	gs_vec2 slp = gs_vec2_sub(position, (gs_vec2){0.f, 0.5f * scl});
	gs_vec2 elp = position;
	// Draw line
	path_draw_line(p, slp, elp, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	slp = gs_vec2_add(slp, (gs_vec2){r * scl, 0.2f * scl});
	path_draw_arc(p, slp, r * scl, 180.f, 360.f, 50, thickness, white);

	slp = gs_vec2_add(slp, (gs_vec2){r * scl, 0.f});
	elp = (gs_vec2){slp.x, position.y};
	path_draw_line(p, slp, elp, thickness, color);

	return glyph;
}

vg_glyph_t* __glyph_create_o()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = -8.f;
	glyph->advance_x = 40.f;
	glyph->bearing_x = 0.f;
	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 90.f;
	const f32 r = 0.18f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	path_draw_circle(p, position, r * scl, num_segments, glyph_thickness, white);
	return glyph;
}

vg_glyph_t* __glyph_create_p()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = -8.f;
	glyph->bearing_x = 1.f;
	glyph->advance_x = 35.f;
	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 30.f;
	const f32 r = 0.5f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	path_draw_circle(p, position, r * scl, num_segments, glyph_thickness, white);

	// p 

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	gs_vec2 slp = gs_vec2_sub(position, (gs_vec2){r * scl, 0.4f * scl});
	gs_vec2 elp = gs_vec2_add(slp, (gs_vec2){0.f, 1.4f * scl});

	path_draw_line(p, slp, elp, glyph_thickness, white);
	return glyph;
}

vg_glyph_t* __glyph_create_q()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = -8.f;
	glyph->bearing_x = -1.f;
	glyph->advance_x = 30.f;
	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 30.f;
	const f32 r = 0.5f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	path_draw_circle(p, position, r * scl, num_segments, glyph_thickness, white);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){r * scl, -0.4f * scl});
	gs_vec2 elp = gs_vec2_add(slp, (gs_vec2){0.f, 1.2f * scl});

	path_draw_line(p, slp, elp, glyph_thickness, white);
	return glyph;
}

vg_glyph_t* __glyph_create_r()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = 0.f;
	glyph->bearing_x = -9.f;
	glyph->advance_x = 30.f;
	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 30.f;
	const f32 r = 0.4f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){r * scl, -0.65f * scl});

	path_draw_arc(p, slp, r * scl, 180.f, 340.f, num_segments, glyph_thickness, white);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	slp = gs_vec2_add(position, (gs_vec2){0.f, -1.f * scl});
	gs_vec2 elp = position;

	path_draw_line(p, slp, elp, glyph_thickness, white);
	return glyph;
}

vg_glyph_t* __glyph_create_s()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = -8.f;
	glyph->bearing_x = -0.f;
	glyph->advance_x = 35.f;
	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 30.f;
	const f32 r = 0.25f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){0.1f * scl, -0.5f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.f, -0.4f * scl});

	gs_vec2 n1 = gs_vec2_norm((gs_vec2){-1.66f, -4.38f});
	gs_vec2 n2 = gs_vec2_norm((gs_vec2){-3.64f, -9.52f});

	// path_draw_bezier_curve(p, 
	// 	slp,
	// 	gs_vec2_scale(n1, scl), 
	// 	gs_vec2_scale(n2, scl), 
	// 	elp,
	// 	30, glyph_thickness, white);

	path_draw_arc(p, gs_vec2_add(position, (gs_vec2){0.f, -r * scl}), r * scl, -30.f, -270.f, 70, glyph_thickness, white );

	// slp = elp;
	// elp = gs_vec2_add(position, (gs_vec2){-0.2f * scl, -0.3f * scl});
	// n1 = gs_vec2_norm((gs_vec2){20.66f, -40.38f});
	// n2 = gs_vec2_norm((gs_vec2){20.64f, -2.52f});

	// path_draw_bezier_curve(p, 
	// 	slp,
	// 	gs_vec2_scale(n1, scl * 0.5f), 
	// 	gs_vec2_scale(n2, scl * 0.5f), 
	// 	elp,
	// 	30, glyph_thickness, white);


	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	path_draw_arc(p, gs_vec2_add(position, (gs_vec2){0.f, r * scl}), r * scl, -95.f, 160.f, 70, glyph_thickness, white );

	p->joint_style = joint_style_round;

	// // s
	// path_draw_arc(p, slp, r * scl, 180.f, 340.f, num_segments, glyph_thickness, white);
	// path_draw_arc(p, slp, r * scl, 180.f, 340.f, num_segments, glyph_thickness, white);

	// gs_dyn_array_push(glyph->paths, path_create_new());
	// p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	// slp = gs_vec2_add(position, (gs_vec2){0.f, -0.24f * scl});
	// gs_vec2 elp = position;

	// path_draw_line(p, slp, elp, glyph_thickness, white);
	return glyph;
}

vg_glyph_t* __glyph_create_t()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 32.f;
	glyph->bearing_y = -1.f;
	glyph->bearing_x = -1.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 70.f;
	const f32 r = 0.1f;

	gs_vec2 slp = gs_vec2_sub(position, (gs_vec2){0.f, 0.6f * scl});
	gs_vec2 elp = gs_vec2_sub(position, (gs_vec2){0.f, 0.1f * scl});
	path_draw_arc(p, gs_vec2_add(elp, (gs_vec2){r * scl, 0.f }), r * scl, 0.f, 180.f, 50, thickness, white);
	path_draw_line(p, slp, elp, thickness, color);
	p->joint_style = joint_style_round;

	// st

	// Draw cross
	slp = gs_vec2_sub(position, (gs_vec2){0.15f * scl, 0.35f * scl});
	elp = gs_vec2_sub(position, (gs_vec2){-0.15f * scl, 0.35f * scl});
	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	path_draw_line(p, slp, elp, thickness, color);

	return glyph;
}

vg_glyph_t* __glyph_create_u()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 38.f;
	glyph->bearing_y = 0.f;
	glyph->bearing_x = -10.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 70.f;
	const f32 r = 0.15f;

	gs_vec2 slp = gs_vec2_sub(position, (gs_vec2){0.f, 0.45f * scl});
	gs_vec2 elp = gs_vec2_sub(position, (gs_vec2){0.f, 0.15f * scl});
	path_draw_arc(p, gs_vec2_add(elp, (gs_vec2){r * scl, 0.f }), r * scl, 0.f, 180.f, 50, thickness, white);
	path_draw_line(p, slp, elp, thickness, color);
	p->joint_style = joint_style_round;

	// stu

	slp = gs_vec2_sub(position, (gs_vec2){-r * scl * 2.f, 0.f * scl});
	elp = gs_vec2_sub(position, (gs_vec2){-r * scl * 2.f, 0.45f * scl});
	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	path_draw_line(p, slp, elp, thickness, color);

	return glyph;
}

vg_glyph_t* __glyph_create_x()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = -3.f;
	glyph->bearing_x = 0.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){0.35f * scl, -0.8f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){-0.35f * scl, 0.2f * scl});
	path_draw_line(p, slp, elp, thickness, color);
	p->joint_style = joint_style_round;

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	slp = gs_vec2_add(position, (gs_vec2){-0.35f * scl, -0.8f * scl});
	elp = gs_vec2_add(position, (gs_vec2){0.35f * scl, 0.2f * scl});
	path_draw_line(p, slp, elp, thickness, color);
	p->joint_style = joint_style_round;

	return glyph;
}

vg_glyph_t* __glyph_create_y()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = 0.f;
	glyph->bearing_x = 0.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){0.35f * scl, -0.9f * scl});
	gs_vec2 elp = gs_vec2_sub(position, (gs_vec2){0.0f * scl, -0.0f * scl});
	path_draw_line(p, slp, elp, thickness, color);
	slp = elp;
	elp = gs_vec2_sub(position, (gs_vec2){0.2f * scl, -0.2f * scl});
	path_draw_line(p, slp, elp, thickness, color);
	path_draw_line(p, elp, gs_vec2_sub(elp, (gs_vec2){0.2f * scl, 0.f}), thickness, color);
	p->joint_style = joint_style_round;

	// stuy

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	slp = gs_vec2_add(position, (gs_vec2){-0.35f * scl, -0.9f * scl});
	elp = gs_vec2_sub(position, (gs_vec2){0.0f * scl, -0.0f * scl});
	path_draw_line(p, slp, elp, thickness, color);
	p->joint_style = joint_style_round;

	return glyph;
}

vg_glyph_t* __glyph_create_v()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = 0.f;
	glyph->bearing_x = 0.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){0.35f * scl, -0.8f * scl});
	gs_vec2 elp = gs_vec2_sub(position, (gs_vec2){0.0f * scl, -0.0f * scl});
	path_draw_line(p, slp, elp, thickness, color);
	p->joint_style = joint_style_round;

	slp = gs_vec2_add(position, (gs_vec2){-0.35f * scl, -0.8f * scl});
	elp = gs_vec2_sub(position, (gs_vec2){0.0f * scl, -0.0f * scl});
	path_draw_line(p, slp, elp, thickness, color);
	p->joint_style = joint_style_round;

	return glyph;
}

vg_glyph_t* __glyph_create_w()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = -1.f;
	glyph->bearing_x = 0.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	p->joint_style = joint_style_miter;

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){-0.35f * scl, -0.8f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){-0.2f * scl, -0.0f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	slp = gs_vec2_add(position, (gs_vec2){-0.2f * scl, 0.0f * scl});
	elp = gs_vec2_add(position, (gs_vec2){0.0f * scl, -0.4f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	slp = gs_vec2_add(position, (gs_vec2){0.0f * scl, -0.4f * scl});
	elp = gs_vec2_add(position, (gs_vec2){0.2f * scl, 0.0f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	slp = gs_vec2_add(position, (gs_vec2){0.2f * scl, -0.0f * scl});
	elp = gs_vec2_add(position, (gs_vec2){0.35f * scl, -0.8f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	return glyph;
}

vg_glyph_t* __glyph_create_space()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = 0.f;
	glyph->bearing_x = 0.f;
	glyph->advance_x = 25.f;
	return glyph;
}

vg_glyph_t* __glyph_create_forward_slash()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = -3.f;
	glyph->bearing_x = 0.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){0.35f * scl, -0.8f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){-0.35f * scl, 0.2f * scl});
	path_draw_line(p, slp, elp, thickness, color);
	p->joint_style = joint_style_round;

	return glyph;
}

vg_glyph_t* __glyph_create_backward_slash()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = -3.f;
	glyph->bearing_x = 0.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){-0.35f * scl, -0.8f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.35f * scl, 0.2f * scl});
	path_draw_line(p, slp, elp, thickness, color);
	p->joint_style = joint_style_round;

	return glyph;
}

vg_glyph_t* __glyph_create_asterisk()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = -15.f;
	glyph->bearing_x = 0.f;
	glyph->advance_x = 25.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	const f32 deg0 = gs_deg_to_rad(45.f); 
	const f32 deg1 = gs_deg_to_rad(45.f * 2.f);
	const f32 deg2 = gs_deg_to_rad(45.f * 3.f);
	const f32 deg3 = gs_deg_to_rad(45.f * 4.f);
	const f32 deg4 = gs_deg_to_rad(45.f * 5.f);
	const f32 deg5 = gs_deg_to_rad(45.f * 6.f);
	const f32 deg6 = gs_deg_to_rad(45.f * 7.f);

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){cos(deg0) * r * scl, sin(deg0) * r * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){cos(deg4) * r * scl, sin(deg4) * r * scl});
	path_draw_line(p, slp, elp, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	slp = gs_vec2_add(position, (gs_vec2){cos(deg2) * r * scl, sin(deg2) * r * scl});
	elp = gs_vec2_add(position, (gs_vec2){cos(deg6) * r * scl, sin(deg6) * r * scl});
	path_draw_line(p, slp, elp, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	slp = gs_vec2_add(position, (gs_vec2){cos(deg1) * r * scl, sin(deg1) * r * scl});
	elp = gs_vec2_add(position, (gs_vec2){cos(deg5) * r * scl, sin(deg5) * r * scl});
	path_draw_line(p, slp, elp, thickness, color);

	return glyph;
}

vg_glyph_t* __glyph_create_underscore()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = 0.f;
	glyph->bearing_x = 0.f;
	glyph->advance_x = 25.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){-0.5f * scl, 0.0f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.5f * scl, 0.f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	return glyph;
}

vg_glyph_t* __glyph_create_less_than()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = -8.f;
	glyph->bearing_x = -8.f;
	glyph->advance_x = 25.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){0.5f * scl, -0.5f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.f * scl, 0.f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_add(slp, (gs_vec2){0.5f * scl, 0.5f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	return glyph;
}

vg_glyph_t* __glyph_create_greater_than()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = -8.f;
	glyph->bearing_x = 8.f;
	glyph->advance_x = 25.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){-0.5f * scl, -0.5f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.f * scl, 0.f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_add(slp, (gs_vec2){-0.5f * scl, 0.5f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	return glyph;
}

vg_glyph_t* __glyph_create_minus()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = -18.f;
	glyph->bearing_x = 5.f;
	glyph->advance_x = 25.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){-0.5f * scl, 0.5f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.5f * scl, 0.5f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	return glyph;
}

vg_glyph_t* __glyph_create_plus()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = -10.f;
	glyph->bearing_x = -5.f;
	glyph->advance_x = 25.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){-0.5f * scl, 0.0f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.5f * scl, 0.0f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	slp = gs_vec2_add(position, (gs_vec2){ 0.f * scl, -0.5f * scl});
	elp = gs_vec2_add(position, (gs_vec2){ 0.f * scl, 0.5f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	return glyph;
}

vg_glyph_t* __glyph_create_left_paren()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = -10.f;
	glyph->bearing_x = 5.f;
	glyph->advance_x = 25.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 40.f;
	const f32 r = 0.5f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){-0.5f * scl, 0.0f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.5f * scl, 0.f * scl});
	path_draw_arc(p, position, r * scl, 90.f, 270.f, num_segments, thickness, color);

	return glyph;
}

vg_glyph_t* __glyph_create_right_paren()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = -10.f;
	glyph->bearing_x = -5.f;
	glyph->advance_x = 25.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 40.f;
	const f32 r = 0.5f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){-0.5f * scl, 0.0f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.5f * scl, 0.f * scl});
	path_draw_arc(p, position, r * scl, 270.f, 360.f + 90.f, num_segments, thickness, color);

	return glyph;
}

vg_glyph_t* __glyph_create_apostrophe()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = -20.f;
	glyph->bearing_x = 0.f;
	glyph->advance_x = 25.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	// '

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){0.f, -0.2f * scl});
	gs_vec2 elp = position;
	path_draw_line(p, slp, elp, thickness * 2.f, color);
	slp = gs_vec2_add(elp, (gs_vec2){-r * scl, 0.f});
	path_draw_arc(p, slp, r * scl, 0.f, 20.f, num_segments, thickness, color);

	p->joint_style = joint_style_round;

	return glyph;
}

vg_glyph_t* __glyph_create_comma()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = 0.f;
	glyph->bearing_x = 0.f;
	glyph->advance_x = 25.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	// ,

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){0.f, -0.2f * scl});
	gs_vec2 elp = position;
	path_draw_line(p, slp, elp, thickness * 2.f, color);
	slp = gs_vec2_add(elp, (gs_vec2){-r * scl, 0.f});
	path_draw_arc(p, slp, r * scl, 0.f, 100.f, num_segments, thickness, color);

	p->joint_style = joint_style_round;

	return glyph;
}

vg_glyph_t* __glyph_create_dot()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = 0.f;
	glyph->bearing_x = 0.f;
	glyph->advance_x = 25.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	// .

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){0.f, -0.1f * scl});
	gs_vec2 elp = position;
	path_draw_line(p, slp, elp, thickness * 2.f, color);
	
	p->joint_style = joint_style_round;

	return glyph;
}

vg_glyph_t* __glyph_create_colon()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = -8.f;
	glyph->bearing_x = 0.f;
	glyph->advance_x = 25.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	// :

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){0.f, -0.5f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.f, -0.2f * scl});
	path_draw_line(p, slp, elp, thickness * 2.f, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];
	slp = gs_vec2_add(position, (gs_vec2){0.f, 0.5f * scl});
	elp = gs_vec2_add(position, (gs_vec2){0.f, 0.2f * scl});
	path_draw_line(p, slp, elp, thickness * 2.f, color);

	return glyph;
}

vg_glyph_t* __glyph_create_zero()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = -8.f;
	glyph->bearing_x = 0.f;
	glyph->advance_x = 25.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.4f;

	// 0 

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	path_draw_arc(p, position, r * scl, 0.f, 180.f, num_segments, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	path_draw_arc(p, gs_vec2_sub(position, (gs_vec2){0.f, 0.3f * scl}), r * scl, 180.f, 360.f, num_segments, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){-r * scl, 0.05f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){-r * scl, -0.3f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	slp = gs_vec2_add(position, (gs_vec2){r * scl, 0.05f * scl});
	elp = gs_vec2_add(position, (gs_vec2){r * scl, -0.3f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	slp = gs_vec2_add(position, (gs_vec2){-r * scl, 0.05f * scl});
	elp = gs_vec2_add(position, (gs_vec2){r * scl, -0.3f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	return glyph;
}

vg_glyph_t* __glyph_create_one()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = -1.f;
	glyph->bearing_x = 0.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 90.f;
	const f32 r = 0.2f;

	// Draw line
	gs_vec2 slp = gs_vec2_sub(position, (gs_vec2){0.1f * scl, 0.3f * scl});
	gs_vec2 elp = gs_vec2_sub(position, (gs_vec2){0.f, 0.4f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = position;
	path_draw_line(p, slp, elp, thickness, color);

	p->joint_style = joint_style_miter;

	slp = gs_vec2_sub(position, (gs_vec2){0.12f * scl, 0.f});
	elp = gs_vec2_add(position, (gs_vec2){0.12f * scl, 0.f});
	path_draw_line(p, slp, elp, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	slp = gs_vec2_sub(position, (gs_vec2){0.f * scl, 0.55f * scl});
	elp = gs_vec2_sub(position, (gs_vec2){0.f, 0.45f * scl});

	return glyph;
}

vg_glyph_t* __glyph_create_two()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = -1.f;
	glyph->bearing_x = 0.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 90.f;
	const f32 r = 0.1f;

	// Draw line
	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){0.f * scl, -0.3f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){-0.1f * scl, 0.f * scl});

	path_draw_arc(p, slp, r * scl, 190.f, 360.f, num_segments, thickness, color);
	slp = gs_vec2_add(slp, (gs_vec2){r * scl, -sin(0) * r * scl});
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_add(position, (gs_vec2){0.15f * scl, 0.f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	p->joint_style = joint_style_round;

	return glyph;
}

vg_glyph_t* __glyph_create_three()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = -15.f;
	glyph->bearing_x = 0.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	// 3

	const s32 num_segments = 70;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 93.f;
	const f32 r = 0.1f;

	// Draw line
	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){0.f * scl, -0.3f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){-0.1f * scl, 0.f * scl});

	path_draw_arc(p, position, r * scl, 190.f, 360.f + 100.f, num_segments, thickness, color);
	path_draw_arc(p, gs_vec2_add(position, (gs_vec2){0.f, 2.f * r * scl}), r * scl, 270.f, 360.f + 150.f, num_segments, thickness, color);

	p->joint_style = joint_style_round;

	return glyph;
}

vg_glyph_t* __glyph_create_four()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = 0.f;
	glyph->bearing_x = 4.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	// 4

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 55.f;
	const f32 r = 0.1f;

	// Draw line
	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){0.f * scl, 0.0f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.0f * scl, -0.7f * scl});
	path_draw_line(p, slp, elp, thickness,color);

	slp = elp;
	elp = gs_vec2_add(position, (gs_vec2){-0.3f * scl, -0.3f * scl});
	path_draw_line(p, slp, elp, thickness,color);

	slp = elp;
	elp = gs_vec2_add(position, (gs_vec2){0.1f * scl, -0.3f * scl});
	path_draw_line(p, slp, elp, thickness,color);

	p->joint_style = joint_style_miter;

	return glyph;
}

vg_glyph_t* __glyph_create_five()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = -17.f;
	glyph->bearing_x = 0.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	// 5

	const s32 num_segments = 70;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 60.f;
	const f32 r = 0.18f;

	// Draw two circles
	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){r * scl, 0.f});
	gs_vec2 elp = gs_vec2_add(slp, (gs_vec2){2.f * -r * scl, 0.0f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_add(slp, (gs_vec2){0.f, 0.2f * scl});
	path_draw_line(p, slp, elp, thickness, color);


	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	path_draw_arc(p, gs_vec2_add(elp, (gs_vec2){r * scl, r * scl}), r * scl, 180.f, 360.f + 150.f, num_segments, thickness, color);

	p->joint_style = joint_style_miter;

	return glyph;
}

vg_glyph_t* __glyph_create_six()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = -17.f;
	glyph->bearing_x = 0.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	// 6

	const s32 num_segments = 70;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 60.f;
	const f32 r = 0.18f;

	// Draw two circles
	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){-r * scl, 0.3f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){-r * scl, 0.1f * scl});
	path_draw_line(p, slp, elp, thickness, color);
	path_draw_arc(p, gs_vec2_add(elp, (gs_vec2){r * scl, 0.f * scl}), r * scl, 180.f, 300.f, num_segments, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	path_draw_circle(p, gs_vec2_add(position, (gs_vec2){0.f, 2.f * r * scl - 1.f}), r * scl, num_segments, thickness, color);

	p->joint_style = joint_style_miter;

	return glyph;
}

vg_glyph_t* __glyph_create_seven()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = 0.f;
	glyph->bearing_x = 6.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	// 7

	const s32 num_segments = 70;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 60.f;
	const f32 r = 0.18f;

	// Draw two circles
	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){2.f * -r * scl, -0.6f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.f, -0.6f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_add(slp, (gs_vec2){2.f * -r * scl, 0.6f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	p->joint_style = joint_style_miter;

	return glyph;
}

vg_glyph_t* __glyph_create_eight()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = -15.f;
	glyph->bearing_x = 0.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	// 8

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 55.f;
	const f32 r = 0.18f;

	// Draw two circles
	path_draw_circle(p, position, r * scl, num_segments, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	path_draw_circle(p, gs_vec2_add(position, (gs_vec2){0.f, 2.f * r * scl - 1.f}), r * scl, num_segments, thickness, color);

	p->joint_style = joint_style_miter;

	return glyph;
}

vg_glyph_t* __glyph_create_nine()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = -15.f;
	glyph->bearing_x = 0.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	// 9

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 55.f;
	const f32 r = 0.18f;

	// Draw two circles
	path_draw_circle(p, position, r * scl, num_segments, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){r * scl, 0.f});
	gs_vec2 elp = gs_vec2_add(slp, (gs_vec2){-r * scl, 0.55f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	p->joint_style = joint_style_miter;

	return glyph;
}

vg_glyph_t* __glyph_create_semi_colon()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 35.f;
	glyph->bearing_y = -5.f;
	glyph->bearing_x = 0.f;
	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	// ;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 70.f;
	const f32 r = 0.18f;

	// i;

	// Draw two circles
	path_draw_line(p, position, gs_vec2_add(position, (gs_vec2){-0.1f * scl, 0.2f * scl}), thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	path_draw_line(p, gs_vec2_add(position, (gs_vec2){0.f, -0.2f * scl}), 
		gs_vec2_add(position, (gs_vec2){0.0f, -0.1f * scl}), thickness * 2.f, color);

	p->joint_style = joint_style_miter;

	return glyph;
}

vg_glyph_t* __glyph_create_equal()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->bearing_y = -10.f;
	glyph->bearing_x = 0.f;
	glyph->advance_x = 25.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 35.f;
	const f32 r = 0.5f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){-0.5f * scl, 0.0f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.5f * scl, 0.f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	slp = gs_vec2_add(position, (gs_vec2){-0.5f * scl, -0.3f * scl});
		elp = gs_vec2_add(position, (gs_vec2){0.5f * scl, -0.3f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	return glyph;
}

vg_glyph_t* __glyph_create_right_brace()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 30.f;
	glyph->bearing_y = -5.f;
	glyph->bearing_x = 0.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 75.f;
	const f32 r = 0.1f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_sub(position, (gs_vec2){0.1f * scl, 0.4f * scl});
	gs_vec2 elp = gs_vec2_sub(position, (gs_vec2){0.f, 0.4f * scl});
	// Draw line
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_sub(position, (gs_vec2){0.f, 0.2f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_sub(elp, (gs_vec2){-0.1f * scl, -0.05f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	p->joint_style = joint_style_miter;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	slp = elp;
	elp = gs_vec2_sub(elp, (gs_vec2){0.1f * scl, -0.05f * scl});
	// Draw line
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_sub(position, (gs_vec2){0.f, -0.1f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_sub(elp, (gs_vec2){0.1f * scl, 0.f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	p->joint_style = joint_style_miter;

	return glyph;
}

vg_glyph_t* __glyph_create_left_bracket()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 30.f;
	glyph->bearing_y = -0.f;
	glyph->bearing_x = 0.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 75.f;
	const f32 r = 0.1f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){0.2f * scl, -0.6f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.f, -0.6f * scl});
	// Draw line
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = position;
	// Draw line
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_add(slp, (gs_vec2){0.2f * scl, 0.f});;
	// Draw line
	path_draw_line(p, slp, elp, thickness, color);

	p->joint_style = joint_style_miter;

	return glyph;
}

vg_glyph_t* __glyph_create_right_bracket()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 30.f;
	glyph->bearing_y = -0.f;
	glyph->bearing_x = 0.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 75.f;
	const f32 r = 0.1f;

	// ]

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_add(position, (gs_vec2){-0.2f * scl, -0.6f * scl});
	gs_vec2 elp = gs_vec2_add(position, (gs_vec2){0.f, -0.6f * scl});
	// Draw line
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = position;
	// Draw line
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_add(slp, (gs_vec2){-0.2f * scl, 0.f});;
	// Draw line
	path_draw_line(p, slp, elp, thickness, color);

	p->joint_style = joint_style_miter;

	return glyph;
}

vg_glyph_t* __glyph_create_left_brace()
{
	vg_glyph_t* glyph = glyph_create_new_ptr();
	glyph->advance_x = 30.f;
	glyph->bearing_y = -5.f;
	glyph->bearing_x = 0.f;

	const s32 num_segments = 20;
	const f32 thickness = glyph_thickness;
	gs_vec4 color = white;
	gs_vec2 position = (gs_vec2){0.f, 0.f};
	const f32 scl = 75.f;
	const f32 r = 0.1f;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	path_t* p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	gs_vec2 slp = gs_vec2_sub(position, (gs_vec2){-0.1f * scl, 0.4f * scl});
	gs_vec2 elp = gs_vec2_sub(position, (gs_vec2){0.f, 0.4f * scl});
	// Draw line
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_sub(position, (gs_vec2){0.f, 0.2f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_sub(elp, (gs_vec2){0.1f * scl, -0.05f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	p->joint_style = joint_style_miter;

	// Construct a new path for the glyph
	gs_dyn_array_push(glyph->paths, path_create_new());
	p = &glyph->paths[gs_dyn_array_size(glyph->paths) - 1];

	slp = elp;
	elp = gs_vec2_sub(elp, (gs_vec2){-0.1f * scl, -0.05f * scl});
	// Draw line
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_sub(position, (gs_vec2){0.f, -0.1f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	slp = elp;
	elp = gs_vec2_sub(elp, (gs_vec2){-0.1f * scl, 0.f * scl});
	path_draw_line(p, slp, elp, thickness, color);

	p->joint_style = joint_style_miter;

	return glyph;
}

void init_font()
{
	// Allocate data for glyphs
	g_font.glyphs = gs_hash_table_new(u32, vg_glyph_ptr);

	// Add glyph into font table
	gs_hash_table_insert(g_font.glyphs, 'A', __glyph_create_A());
	gs_hash_table_insert(g_font.glyphs, 'B', __glyph_create_B());
	gs_hash_table_insert(g_font.glyphs, 'C', __glyph_create_C());
	gs_hash_table_insert(g_font.glyphs, 'D', __glyph_create_D());
	gs_hash_table_insert(g_font.glyphs, 'E', __glyph_create_E());
	gs_hash_table_insert(g_font.glyphs, 'F', __glyph_create_F());
	gs_hash_table_insert(g_font.glyphs, 'G', __glyph_create_G());
	gs_hash_table_insert(g_font.glyphs, 'P', __glyph_create_P());
	gs_hash_table_insert(g_font.glyphs, 'a', __glyph_create_a());
	gs_hash_table_insert(g_font.glyphs, 'b', __glyph_create_b());
	gs_hash_table_insert(g_font.glyphs, 'c', __glyph_create_c());
	gs_hash_table_insert(g_font.glyphs, 'd', __glyph_create_d());
	gs_hash_table_insert(g_font.glyphs, 'e', __glyph_create_e());
	gs_hash_table_insert(g_font.glyphs, 'f', __glyph_create_f());
	gs_hash_table_insert(g_font.glyphs, 'g', __glyph_create_g());
	gs_hash_table_insert(g_font.glyphs, 'h', __glyph_create_h());
	gs_hash_table_insert(g_font.glyphs, 'i', __glyph_create_i());
	gs_hash_table_insert(g_font.glyphs, 'j', __glyph_create_j());
	gs_hash_table_insert(g_font.glyphs, 'k', __glyph_create_k());
	gs_hash_table_insert(g_font.glyphs, 'l', __glyph_create_l());
	gs_hash_table_insert(g_font.glyphs, 'm', __glyph_create_m());
	gs_hash_table_insert(g_font.glyphs, 'n', __glyph_create_n());
	gs_hash_table_insert(g_font.glyphs, 'o', __glyph_create_o());
	gs_hash_table_insert(g_font.glyphs, 'p', __glyph_create_p());
	gs_hash_table_insert(g_font.glyphs, 'q', __glyph_create_q());
	gs_hash_table_insert(g_font.glyphs, 'r', __glyph_create_r());
	gs_hash_table_insert(g_font.glyphs, 's', __glyph_create_s());
	gs_hash_table_insert(g_font.glyphs, 't', __glyph_create_t());
	gs_hash_table_insert(g_font.glyphs, 'u', __glyph_create_u());
	gs_hash_table_insert(g_font.glyphs, 'v', __glyph_create_v());
	gs_hash_table_insert(g_font.glyphs, 'w', __glyph_create_w());
	gs_hash_table_insert(g_font.glyphs, 'x', __glyph_create_x());
	gs_hash_table_insert(g_font.glyphs, 'y', __glyph_create_y());
	gs_hash_table_insert(g_font.glyphs, '0', __glyph_create_zero());
	gs_hash_table_insert(g_font.glyphs, '1', __glyph_create_one());
	gs_hash_table_insert(g_font.glyphs, '2', __glyph_create_two());
	gs_hash_table_insert(g_font.glyphs, '3', __glyph_create_three());
	gs_hash_table_insert(g_font.glyphs, '4', __glyph_create_four());
	gs_hash_table_insert(g_font.glyphs, '5', __glyph_create_five());
	gs_hash_table_insert(g_font.glyphs, '6', __glyph_create_six());
	gs_hash_table_insert(g_font.glyphs, '7', __glyph_create_seven());
	gs_hash_table_insert(g_font.glyphs, '8', __glyph_create_eight());
	gs_hash_table_insert(g_font.glyphs, '9', __glyph_create_nine());
	gs_hash_table_insert(g_font.glyphs, ' ', __glyph_create_space());
	gs_hash_table_insert(g_font.glyphs, '_', __glyph_create_underscore());
	gs_hash_table_insert(g_font.glyphs, '*', __glyph_create_asterisk());
	gs_hash_table_insert(g_font.glyphs, ';', __glyph_create_semi_colon());
	gs_hash_table_insert(g_font.glyphs, '}', __glyph_create_right_brace());
	gs_hash_table_insert(g_font.glyphs, '{', __glyph_create_left_brace());
	gs_hash_table_insert(g_font.glyphs, '=', __glyph_create_equal());
	gs_hash_table_insert(g_font.glyphs, '(', __glyph_create_left_paren());
	gs_hash_table_insert(g_font.glyphs, ')', __glyph_create_right_paren());
	gs_hash_table_insert(g_font.glyphs, ',', __glyph_create_comma());
	gs_hash_table_insert(g_font.glyphs, '.', __glyph_create_dot());
	gs_hash_table_insert(g_font.glyphs, '/', __glyph_create_forward_slash());
	gs_hash_table_insert(g_font.glyphs, '\\', __glyph_create_backward_slash());
	gs_hash_table_insert(g_font.glyphs, '-', __glyph_create_minus());
	gs_hash_table_insert(g_font.glyphs, '+', __glyph_create_plus());
	gs_hash_table_insert(g_font.glyphs, '<', __glyph_create_less_than());
	gs_hash_table_insert(g_font.glyphs, '>', __glyph_create_greater_than());
	gs_hash_table_insert(g_font.glyphs, ':', __glyph_create_colon());
	gs_hash_table_insert(g_font.glyphs, '[', __glyph_create_left_bracket());
	gs_hash_table_insert(g_font.glyphs, ']', __glyph_create_right_bracket());
	gs_hash_table_insert(g_font.glyphs, '\'', __glyph_create_apostrophe());
}

#define v4(_x, _y, _z, _w)\
	((gs_vec4){(_x), (_y), (_z), (_w)})

#define v3(_x, _y, _z)\
	((gs_vec3){(_x), (_y), (_z)})

#define v2(_x, _y)\
	((gs_vec2){(_x), (_y)})

#define xform_translate(_t, _v)\
	((gs_vqs){gs_vec3_add((_t).position, _v), (_t).rotation, (_t).scale})

#define func_col 		v4(0.2f, 0.6f, 0.8f, 1.f)
#define member_var_col 	v4(0.8f, 0.3f, 0.3f, 1.f)
#define default_col  	v4(1.f, 1.f, 1.f, 1.f)
#define primitive_col  	v4(0.9f, 0.7f, 0.2f, 1.f)
#define keyword_col  	v4(0.7f, 0.6f, 0.9f, 1.f)
#define comment_col 	v4(0.4f, 0.4f, 0.4f, 1.f)
#define code_bg_col 	v4(0.05f, 0.08f, 0.2f, 1.f)

#define color_alpha(_c, _a)\
	(gs_vec4){(_c).x, (_c).y, (_c).z, (_a)}

animation_t* new_anim(shape_t* shape, f32 anim_speed)
{
	gs_dyn_array_push(g_animations, animation_create_new());
	animation_t* anim = gs_dyn_array_back(g_animations);
	animation_set_shape(anim, shape);
	anim->animation_speed = anim_speed;
	return anim;
}

#define anim_fade_to(_ft, _amt)\
do {\
	f32 _alpha = (_amt);\
	animation_add_action(anim, animation_action_set_alpha, animation_ease_type_smooth_step, (_ft), &_alpha);\
} while(0)

#define anim_fade_out_forever(_fat)\
do {\
	f32 _fade = 0.0f;\
	animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, (_fat), &_fade);\
} while ( 0 )

#define anim_blink(_start_fade, _end_fade, _iterations, _ft)\
do {\
	f32 _sf = (_start_fade);\
	f32 _ef = (_end_fade);\
	for ( u32 i = 0; i < (_iterations); ++i ) {\
		animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, (_ft), &_sf);\
		animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, (_ft), &_ef);\
	}\
} while ( 0 )

#define anim_translate(_tt, translation)\
do {\
	gs_vqs __trans = gs_vqs_default();\
	__trans.position = gs_vec3_add(anim->shape.xform.position, translation);\
	animation_add_action(anim, animation_action_type_transform, animation_ease_type_smooth_step, (_tt), &__trans);\
} while ( 0 )

#define anim_wait(_wt)\
do {\
	animation_add_action(anim, animation_action_type_wait, animation_ease_type_smooth_step, (_wt), NULL);\
} while ( 0 )

#define anim_walk(_t)\
do {\
	animation_add_action(anim, animation_action_type_walk_path, animation_ease_type_smooth_step, (_t), NULL);\
} while ( 0 )

#define anim_disable(_t)\
do {\
	animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_smooth_step, (_t), NULL);\
} while ( 0 )

#define anim_transform(_t, _xform)\
do {\
	gs_vqs trans = (_xform);\
	animation_add_action(anim, animation_action_type_transform, animation_ease_type_smooth_step, (_t), &trans);\
} while(0)

b32 is_keyword(gs_token t)
{
	if (gs_token_compare_text(t, "typedef") ||
		gs_token_compare_text(t, "const") ||
		gs_token_compare_text(t, "for") ||
		gs_token_compare_text(t, "while") || 
		gs_token_compare_text(t, "do") ||
		gs_token_compare_text(t, "return")
	)
	{
		return true;
	}

	return false;
}

b32 is_primitive(gs_token t)
{
	if (gs_token_compare_text(t, "u8") || gs_token_compare_text(t, "uint8_t") ||
		gs_token_compare_text(t, "s8") || gs_token_compare_text(t, "int8_t") ||
		gs_token_compare_text(t, "u16") || gs_token_compare_text(t, "uint16_t") ||
		gs_token_compare_text(t, "s16") || gs_token_compare_text(t, "int16_t") ||
		gs_token_compare_text(t, "u32") || gs_token_compare_text(t, "uint32_t") || 
		gs_token_compare_text(t, "s32") || gs_token_compare_text(t, "int32_t") ||
		gs_token_compare_text(t, "u64") || gs_token_compare_text(t, "uint64_t") ||
		gs_token_compare_text(t, "float") || gs_token_compare_text(t, "f32") ||
		gs_token_compare_text(t, "double") || gs_token_compare_text(t, "f64") ||
		gs_token_compare_text(t, "char") || 
		gs_token_compare_text(t, "bool") ||
		gs_token_compare_text(t, "b32") ||
		gs_token_compare_text(t, "usize") || gs_token_compare_text(t, "size_t") ||
		gs_token_compare_text(t, "struct") || 
		gs_token_compare_text(t, "void") 
	)
	{
		return true;
	}
	return false;	
}

b32 is_comment(gs_token t)
{
	if (gs_token_compare_type(t, "single_line_comment") ||
		gs_token_compare_type(t, "multi_line_comment")
	)
	{
		return true;
	}
	return false;
}

gs_vec4 get_color_from_token(gs_lexer* lex, gs_token t)
{
	// Function color
	if (gs_token_compare_type(gs_lexer_peek_next_token(lex), "lparen")) 
	{
		return func_col;
	}
	else if (is_keyword(t)) 
	{
		return keyword_col;
	} 
	else if ( is_primitive(t)) 
	{
		return primitive_col;
	}
	else if ( is_comment(t) )
	{
		return comment_col;
	}

	return default_col;
}

// For now, just pass in a specific animation to add this to
void animate_code(const char* txt, gs_vec2 position, f32 txt_size, f32 typing_speed, f32 wait_time, f32 alive_time)
{
	// How do, negro? Build a lexer for the text based on C, which I provide.	
	gs_lexer_c _lex = gs_lexer_c_ctor( txt );
	gs_lexer* lex = gs_cast(gs_lexer, &_lex);
	shape_t* shape = &g_shape;
	animation_t* anim = NULL;

	gs_vec2 origin = position;
	gs_vec2 ws = frame_buffer_size;

	char tmp[256];

	// Want cursor as well...

	u32 i = 0;
	while (gs_lexer_can_lex(lex)) 
	{
		gs_token t = gs_lexer_next_token(lex);
		strncpy(tmp, t.text, t.len);
		tmp[t.len] = '\0';

		// Get color from token
		gs_vec4 col = get_color_from_token(lex, t);

		for (u32 j = 0; j < t.len; ++j) 
		{
			gs_vec2 p = position;

			char buff[2] = {tmp[j], '\0'};
			// For each character in token's text, we'll just display to screen for now and then advance position
			shape_begin(shape);
			shape->xform.position = v3(ws.x / 2.f, ws.y / 2.f, 0.f);
			shape->xform.scale = v3(1.f, 1.f, 1.f);
			position.x += shape_draw_text(shape, position, &g_font, buff, txt_size, color_alpha(col, 0.f));
			anim = new_anim(shape, 1.f);
			animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_lerp, i * typing_speed + wait_time, NULL);
			anim_fade_to(0.f, 1.f);
			anim_wait(alive_time - (i * typing_speed));
			anim_fade_out_forever(0.f);

			// Draw caret
			gs_vec2 hext = v2(5.f / 2.f, 30.f);
			shape_begin(shape);
			shape->xform.position = v3(ws.x / 2.f, ws.y / 2.f, 0.f);
			shape->xform.scale = v3(txt_size / 96.f, txt_size / 96.f, 1.f);
			shape_draw_square(shape, gs_vec2_add(position, v2(0.f, -hext.y / 2.f - 10.f)), hext, 0.2f, color_alpha(white, 0.1f), true);
			anim = new_anim(shape, 1.f);
			animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_lerp, i * typing_speed + wait_time, NULL);
			anim_fade_to(typing_speed, 0.4f);
			anim_fade_out_forever(0.0f);
			i++;
		}

		if (gs_token_compare_type(t, "newline")) 
		{
			position.x = origin.x;
			position.y += 120.f * txt_size / 96.f;
		}
	}
}

#define animate_txt(_txt, _x, _y, _size, _col, _wait_time, _fade_time, ...)\
do\
{\
	shape_begin(shape);\
	shape->xform.position = (gs_vec3){ws.x / 2.f, ws.y / 2.f, 0.f};\
	shape->xform.scale = (gs_vec3){1.f, 1.f, 1.f};\
	gs_dyn_array_push(g_animations, animation_create_new());\
	anim = gs_dyn_array_back(g_animations);\
	anim->animation_speed = 1.0f;\
	(_x) += shape_draw_text(shape, (gs_vec2){(_x), (_y)}, &g_font, (_txt), (_size), (_col));\
	for (u32 i = 0; i < gs_string_length((_txt)); ++i) {\
		if ((_txt)[i] == '\n') {\
			(_x) = 0.f;\
			(_y) += 75.f;\
		}\
	}\
	animation_set_shape(anim, shape);\
	animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_lerp, (_wait_time), NULL);\
	animation_add_action(anim, animation_action_type_walk_path, animation_ease_type_smooth_step, (_fade_time), NULL);\
	__VA_ARGS__;\
	\
} while(0)

void play_scene_one()
{
	anim_mult = 1.f;

	gs_for_range_i(gs_dyn_array_size(g_animations)) {
		animation_clear(&g_animations[i]);
	}

	gs_dyn_array_clear(g_animations);
	// Cache instance of graphics api from engine
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Viewport
	const gs_vec2 ws = frame_buffer_size;

	animation_t* anim = NULL;
	shape_t* shape = &g_shape;
	shape->xform = gs_vqs_default();

	const u32 num_cols = 10;
	const u32 num_rows = 10;
	const f32 grid_size = 600.f;
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

	f32 fade_amt = 0.1f;
	f32 txt_size = 60.f;

	const char* code = NULL;

	/*
		Center Cell Animation
	*/
	// Set up shape
	shape_begin(shape);
	shape->xform.position = (gs_vec3){ws.x / 2.f, ws.y / 2.f, 0.f};
	shape_draw_square(shape, ocp, chext, ct, (gs_vec4){grid_col.x, grid_col.y, grid_col.z, 0.1f}, true);

	fade_amt = 10.f;
	gs_dyn_array_push(g_animations, animation_create_new());
	anim = gs_dyn_array_back(g_animations);
	animation_set_shape(anim, shape);
	animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_lerp, 5.f, NULL);
	animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, 3.f, &fade_amt);
	animation_add_action(anim, animation_action_type_wait, animation_ease_type_smooth_step, 10.f, NULL);

	trans = shape->xform;
	trans.scale = (gs_vec3){ 2.5, 2.5, 1.f };
	trans.position = (gs_vec3){ ws.x * 0.12f, ws.y * 0.5f - 1.3f * ch, 0.f };
	animation_add_action(anim, animation_action_type_transform, animation_ease_type_smooth_step, 3.f, &trans);

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
	fade_amt = 0.1f;
	gs_dyn_array_push(g_animations, animation_create_new());
	anim = gs_dyn_array_back(g_animations);
	animation_set_shape(anim, shape);
	animation_add_action(anim, animation_action_type_walk_path, animation_ease_type_smooth_step, 5.f, NULL);
	animation_add_action(anim, animation_action_type_wait, animation_ease_type_lerp, 7.f, NULL);
	animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, 2.f, &fade_amt);
	animation_add_action(anim, animation_action_type_wait, animation_ease_type_lerp, 4.f, NULL);
	fade_amt = 0.0f;
	animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, 1.f, &fade_amt);
	animation_add_action(anim, animation_action_type_wait, animation_ease_type_lerp, 10.f, NULL);

	shape_begin(shape);
	shape->xform.position = (gs_vec3){ws.x / 2.f, ws.y / 2.f, 0.f};
	shape_draw_text(shape, (gs_vec2){(-45.f * 4.f) / 2.f + 45.f, grid_size * 1.f }, &g_font, "width", txt_size, white);
	shape_draw_text(shape, (gs_vec2){-grid_size * 1.3f - 1.f * 45.f, 0.f}, &g_font, "height", txt_size, white);

	gs_dyn_array_push(g_animations, animation_create_new());
	anim = gs_dyn_array_back(g_animations);
	animation_set_shape(anim, shape);
	animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_lerp, 3.f, NULL);
	animation_add_action(anim, animation_action_type_walk_path, animation_ease_type_smooth_step, 5.f, NULL);
	animation_add_action(anim, animation_action_type_wait, animation_ease_type_lerp, 11.f, NULL);

	trans = shape->xform;
	trans.position = (gs_vec3){ ws.x / 2.f, ws.y * 2.f, 0.f };
	animation_add_action(anim, animation_action_type_transform, animation_ease_type_smooth_step, 2.f, &trans);
	animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, 1.5f, &fade_amt);
	animation_add_action(anim, animation_action_type_wait, animation_ease_type_lerp, 4.f, NULL);

	/*
		// Highlight center cell animation
	*/
	shape_begin(shape);
	shape->xform.position = (gs_vec3){ws.x / 2.f, ws.y / 2.f, 0.f};
	shape_draw_square(shape, ocp, chext, 2.f, highlight_col, false);

	gs_dyn_array_push(g_animations, animation_create_new());
	anim = gs_dyn_array_back(g_animations);
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

	/*
		// Code
	*/
	const f32 glyph_height = 45.f;
	const f32 glyph_width = 45.f;
	shape_begin(shape);
	shape->xform.position = (gs_vec3){ws.x / 2.f, ws.y / 2.f, 0.f};
	shape->xform.scale = (gs_vec3){1.f, 1.f, 1.f};

	shape_draw_line(shape, v2(0.f, -ws.y * 0.25f * 0.9f), v2(0.f, ws.y * 0.25f * 0.9f), ws.x * 0.46f * 0.55f, color_alpha(code_bg_col, 0.3f));
	shape_draw_square(shape, (gs_vec2){0.f, 0.f}, (gs_vec2){ws.x * 0.46f, ws.y * 0.25f}, 1.f, white, false);

	gs_dyn_array_push(g_animations, animation_create_new());
	anim = gs_dyn_array_back(g_animations);
	animation_set_shape(anim, shape);
	animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_lerp, 20.f, NULL);
	animation_add_action(anim, animation_action_type_walk_path, animation_ease_type_smooth_step, 10.f, NULL);

	f32 base_xp = -ws.x * 0.35f;
	f32 xpos = base_xp;
	f32 ypos = glyph_height - 300.f;

	code = "typedef struct particle_t\n"\
	"{\n"
	"  u32 id;               // 4 bytes\n"
	"  f32 life_time;        // 4 bytes\n"
	"  gs_vec2 velocity;     // 64 bytes\n"
	"  color_t color;        // 32 bytes\n"
	"  b32 has_been_updated; // 32 bytes\n"
	"} particle_t;           // total: 136 bytes";
	animate_code(code, v2(xpos, ypos), txt_size, 0.15f, 20.f, 100.f);

	shape_begin(shape);
	shape->xform.position = (gs_vec3){ws.x * 0.5f, ws.y * 0.5f, 0.f};
	shape->xform.scale = (gs_vec3){1.f, 1.f, 1.f};

	gs_dyn_array_push(g_animations, animation_create_new());
	anim = gs_dyn_array_back(g_animations);
	animation_set_shape(anim, shape);
	animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_lerp, 30.f, NULL);
	fade_amt = 10.0f;
	animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, 4.0f, &fade_amt);

	// Arrow
	shape_begin(shape);
	shape->xform.position = (gs_vec3){ws.x / 2.f, ws.y / 2.f, 0.f};
	shape->xform.scale = (gs_vec3){1.f, 1.f, 1.f};
	gs_vec2 slp = (gs_vec2){-ws.x * 0.13f, 0.f};
	gs_vec2 elp = gs_vec2_add(slp, (gs_vec2){70.f, 0.f});
	shape_draw_line(shape, slp, elp, 0.1f, white);
	slp = gs_vec2_add(slp, (gs_vec2){0.f, 10.f});
	elp = gs_vec2_add(slp, (gs_vec2){70.f, 0.f});
	shape_draw_line(shape, slp, elp, 0.1f, white);
}

#define anim_wait_fade_out(_wt, _ft)\
do {\
	anim_wait(_wt);\
	anim_fade_out_forever(_ft);\
} while (0)

void play_scene_two()
{
	anim_mult = 1.f;

	gs_for_range_i(gs_dyn_array_size(g_animations)) {
		animation_clear(&g_animations[i]);
	}
	gs_dyn_array_clear(g_animations);

	// Cache instance of graphics api from engine
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Viewport
	const gs_vec2 ws = frame_buffer_size;

	animation_t* anim = NULL;
	shape_t* shape = &g_shape;
	shape->xform = gs_vqs_default();
	u32 anim_ct = 0;

	const u32 num_cols = 10;
	const u32 num_rows = 10;
	const f32 grid_size = 600.f;
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

	const char* code = NULL;

	// Initialize shape and animation for grid
	// Want each of these squares to animate in as well...
	shape_begin(shape);
	shape->xform.position = (gs_vec3){ws.x / 2.f, ws.y / 2.f, 0.f};
	shape_draw_square(shape, (gs_vec2){ocp.x - cw, ocp.y + ch}, chext, ct, grid_col, true);
	shape_draw_square(shape, (gs_vec2){ocp.x, ocp.y + ch}, chext, ct, grid_col, true);
	shape_draw_square(shape, (gs_vec2){ocp.x, ocp.y + ch}, chext, ct, grid_col, true);
	shape_draw_square(shape, (gs_vec2){ocp.x - cw * 2, ocp.y + ch}, chext, ct, grid_col, true);
	shape_draw_square(shape, (gs_vec2){ocp.x + cw * 2, ocp.y + ch * 2}, chext, ct, grid_col, true);
	shape_draw_grid(shape, (gs_vec2){0.f, 0.f}, grid_size, grid_size, num_rows, num_cols, 0.5f, (gs_vec4){1.f, 1.f, 1.f, 0.1f});

	// Total play time should be determined by total number of tracks...
	// Animate one of the paths
	trans.scale = (gs_vec3){0.9f, 0.9f, 0.f};
	trans.position = (gs_vec3){grid_size * trans.scale.x - 80.f, grid_size * trans.scale.y + 20.f, 0.f};
	f32 fade_amt = 10.0f;
	gs_dyn_array_push(g_animations, animation_create_new());
	anim = gs_dyn_array_back(g_animations);
	animation_set_shape(anim, shape);
	animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, 3.f, &fade_amt);
	animation_add_action(anim, animation_action_type_transform, animation_ease_type_smooth_step, 2.5f, &trans);	

	// Want an "animated" cell, to look as though it's iterating through the grid
	/*
		// Highlight center cell animation
	*/
	// What's top grid position?
	gs_vec2 bl = (gs_vec2){ocp.x - grid_size / 2.f - 110.f, ocp.y + grid_size / 2.f};

	const f32 alpha = 1.0f;

	b32 done = false;
	u32 count = 0;
	u32 max_count = 2 * num_cols + 8;
	f32 yoff = -grid_size;
	const f32 txt_size = 55.f;
	for ( u32 r = 1; r < num_cols; ++r )
	{
		if ( done ) {
			break;
		}

		for ( u32 c = 0; c < num_cols; ++c ) 
		{
			u32 i = r * num_cols + c;

			if (count > max_count - 1) {
				done = true;
				break;
			}

			count++;

			shape_begin(shape);
			shape->xform.scale = (gs_vec3){0.9f, 0.9f, 1.f};
			shape->xform.position = (gs_vec3){grid_size * shape->xform.scale.x + 20.f, grid_size * shape->xform.scale.y + 20.f, 0.f};
			shape_draw_square(shape, gs_vec2_add(bl, (gs_vec2){cw * c, -ch * r}), chext, 2.f, highlight_col, false);

			f32 xval = 0.35f;
			if ( count <= max_count - 1 ) {
				const char* txt = "cell_id";
				f32 xoff = shape_draw_text(shape, (gs_vec2){-ws.x * xval, yoff }, 
					&g_font, txt, txt_size, color_alpha(primitive_col, 0.1f));
				txt = " = mat_id_empty;";
				shape_draw_text(shape, (gs_vec2){-ws.x * xval + xoff, yoff }, 
					&g_font, txt, txt_size, color_alpha(default_col, 0.1f));
			} else {
				const char* txt = "cell_id";
				f32 xoff = shape_draw_text(shape, (gs_vec2){-ws.x * xval, yoff }, 
					&g_font, txt, txt_size, color_alpha(primitive_col, 0.1f));
				txt = " = mat_id_sand;";
				shape_draw_text(shape, (gs_vec2){-ws.x * xval + xoff, yoff }, 
					&g_font, txt, txt_size, color_alpha(default_col, 0.1f));
			}

			char tmp[256];
			gs_snprintf(tmp, sizeof(tmp), "col = %zu", c);
			shape_draw_text(shape, (gs_vec2){-ws.x * xval, yoff - 60.f }, 
				&g_font, tmp, txt_size, (gs_vec4){1.f, 1.f, 1.f, 0.1f});
			gs_snprintf(tmp, sizeof(tmp), "row = %zu", num_rows - r);
			shape_draw_text(shape, (gs_vec2){-ws.x * xval, yoff - 2.f * 60.f }, 
				&g_font, tmp, txt_size, (gs_vec4){1.f, 1.f, 1.f, 0.1f});

			gs_dyn_array_push(g_animations, animation_create_new());
			anim = gs_dyn_array_back(g_animations);
			anim->animation_speed = 0.8f;
			animation_set_shape(anim, shape);

			fade_amt = 10.0f;
			animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_lerp, i * 0.8f + 5.f, NULL);
			animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, 1.0f, &fade_amt);

			if ( count <= max_count - 1 ) {
				fade_amt = 0.0f;
				animation_add_action(anim, animation_action_percentage_alpha, animation_ease_type_smooth_step, 0.4f, &fade_amt);
				animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_smooth_step, 100.0f, &fade_amt);
			}
		}
	}

	// trans.scale = (gs_vec3){0.9f, 0.9f, 0.f};
	// trans.position = (gs_vec3){grid_size * trans.scale.x + 20.f, grid_size * trans.scale.y + 20.f, 0.f};
	// animation_add_action(anim, animation_action_type_transform, animation_ease_type_smooth_step, 1.f, &trans);	

	// Draw a box around all of the code
	shape_begin(shape);
	shape->xform.position = (gs_vec3){ws.x / 2.f + grid_size * 0.55f, ws.y / 2.f, 0.f};
	shape->xform.scale = (gs_vec3){1.f, 1.f, 1.f};
	shape_draw_line(shape, v2(60.f, -grid_size * 0.66f), 
		v2(60.f, grid_size * 0.66f), grid_size * 0.49f, color_alpha(code_bg_col, 0.3f));
	shape_draw_square(shape, v2(60.f, 0.f), 
		v2(grid_size * 0.88f, grid_size * 0.68f), 2.5f, color_alpha(white, 0.05f), false);

	gs_dyn_array_push(g_animations, animation_create_new());
	anim = gs_dyn_array_back(g_animations);
	anim->animation_speed = 1.0f;
	animation_set_shape(anim, shape);
	animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_lerp, 5.0f, NULL);
	animation_add_action(anim, animation_action_type_walk_path, animation_ease_type_smooth_step, 10.f, NULL);

	f32 xpos = -100.f;
	f32 _bt = 2.5f;
	f32 wt = 43.f;
	f32 wt2 = wt - 5.f;
	f32 wt3 = wt2 - 5.f;
	f32 fat = 1.0f;
	f32 cached_xpos = 0.f;
	f32 cached_ypos = 0.f;
	yoff = yoff + 120.f;

	code = "for (u32 y = height - 1; y > 0; --y)\n"\
	"{\n"
	"  for (u32 x = 0; x < width; ++x)\n"
	"  {\n"
	"    u8 mat_id = get_p(x,y)->id;\n"
	"    switch (mat_id)\n"
	"    {\n"
	"      // do nothing\n"
	"      case mat_id_empty: \n"
	"        break;\n"
	"      // do something\n"
	"      case mat_id_sand:\n"
	"        update_sand(x, y);\n"
	"        break;\n"
	"    }\n"
	"  }\n"
	"}";
	animate_code(code, v2(xpos, yoff), txt_size, 0.12f, 5.f, 50.f);

	// Line to empty 
	{
		const f32 thickness = 0.2f;
		shape_begin(shape);
		shape->xform.position = (gs_vec3){ws.x / 2.f, ws.y / 2.f, 0.f};
		shape->xform.scale = (gs_vec3){1.f, 1.f, 1.f};
		gs_vec2 slp = (gs_vec2){-225.f, -295.f};
		gs_vec2 elp = gs_vec2_add(slp, (gs_vec2){60.f, 0.f});
		path_t* p = shape_begin_path(shape);
		p->joint_style = joint_style_miter;
		path_draw_line(p, slp, elp, thickness, primitive_col);
		slp = elp;
		elp = gs_vec2_add(slp, (gs_vec2){0.f, 325.f});
		path_draw_line(p, slp, elp, thickness, primitive_col);
		slp = elp;
		elp = gs_vec2_add(slp, (gs_vec2){250.f, 0.f});
		path_draw_line(p, slp, elp, thickness, primitive_col);
		// path_draw_line(p, elp, (gs_vec2){elp.x, elp.y - 15.f}, thickness, white);

		gs_dyn_array_push(g_animations, animation_create_new());
		anim = gs_dyn_array_back(g_animations);
		anim->animation_speed = 1.0f;
		animation_set_shape(anim, shape);
		animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_smooth_step, 10.5f, NULL);
		animation_add_action(anim, animation_action_type_walk_path, animation_ease_type_smooth_step, 10.f, NULL);
		animation_add_action(anim, animation_action_type_wait, animation_ease_type_smooth_step, 17.5f, NULL);
		anim_wait_fade_out(3.f, 8.f);

		// box
		shape_begin(shape);
		shape->xform.position = (gs_vec3){ws.x / 2.f, ws.y / 2.f, 0.f};
		shape->xform.scale = (gs_vec3){1.f, 1.f, 1.f};
		const f32 sq_half_width = 220.f;
		elp = gs_vec2_add(elp, (gs_vec2){sq_half_width, 0.f});
		shape_draw_square(shape, elp, (gs_vec2){sq_half_width, 20.f}, thickness, primitive_col, false);
		gs_dyn_array_push(g_animations, animation_create_new());
		anim = gs_dyn_array_back(g_animations);
		anim->animation_speed = 1.0f;
		animation_set_shape(anim, shape);
		animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_smooth_step, 18.5f, NULL);
		animation_add_action(anim, animation_action_type_walk_path, animation_ease_type_smooth_step, 5.f, NULL);
		anim_blink(0.1f, 10.0f, 3, 3.f);
		anim_wait_fade_out(3.f, 2.5f);
	}

	// Line to sand update
	{
		const f32 thickness = 0.2f;
		shape_begin(shape);
		shape->xform.position = (gs_vec3){ws.x / 2.f, ws.y / 2.f, 0.f};
		shape->xform.scale = (gs_vec3){1.f, 1.f, 1.f};
		gs_vec2 slp = (gs_vec2){-225.f, -295.f};
		gs_vec2 elp = gs_vec2_add(slp, (gs_vec2){60.f, 0.f});
		path_t* p = shape_begin_path(shape);
		p->joint_style = joint_style_miter;
		path_draw_line(p, slp, elp, thickness, color_alpha(primitive_col, 1.f));
		slp = elp;
		elp = gs_vec2_add(slp, (gs_vec2){0.f, 480.f});
		path_draw_line(p, slp, elp, thickness, color_alpha(primitive_col, 1.f));
		slp = elp;
		elp = gs_vec2_add(slp, (gs_vec2){250.f, 0.f});
		path_draw_line(p, slp, elp, thickness, color_alpha(primitive_col, 1.f));

		gs_dyn_array_push(g_animations, animation_create_new());
		anim = gs_dyn_array_back(g_animations);
		anim->animation_speed = 1.0f;
		animation_set_shape(anim, shape);
		animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_smooth_step, 45.0f, NULL);
		animation_add_action(anim, animation_action_type_walk_path, animation_ease_type_smooth_step, 1.f, NULL);
		anim_wait_fade_out(3.f, 8.f);

		// Blinking box around sand selection
		shape_begin(shape);
		shape->xform.position = (gs_vec3){ws.x / 2.f, ws.y / 2.f, 0.f};
		shape->xform.scale = (gs_vec3){1.f, 1.f, 1.f};
		const f32 sq_half_width = 250.f;
		elp = gs_vec2_add(elp, (gs_vec2){sq_half_width, 0.f});
		shape_draw_square(shape, elp, 
			(gs_vec2){sq_half_width, grid_size * 0.032f}, 
			0.1f, color_alpha(primitive_col, 0.1f), false);

		gs_dyn_array_push(g_animations, animation_create_new());
		anim = gs_dyn_array_back(g_animations);
		anim->animation_speed = 1.0f;
		animation_set_shape(anim, shape);

		fade_amt = 10.0f;
		animation_add_action(anim, animation_action_type_disable_shape, animation_ease_type_lerp, 45.5f, NULL);
		anim_blink(10.f, 0.1f, 2, 1.f);
		anim_wait_fade_out(0.f, 0.5f);
	}

	// Eventually, can just have a function to lex over text and set up the animations automatically
	{
		const f32 _yoff = 250.f;
		const f32 _xoff = 0.f;
		const f32 bt2 = 22.f;
		const f32 wt = _bt * bt2;
		const f32 tt = 3.f;
		gs_vec3 trans = (gs_vec3){-37.f + _xoff, -grid_size + _yoff + 120.f, 0.f}; 
		xpos = cached_xpos + 270.f;
		yoff = cached_ypos + 300.f;	
		animate_txt("update_sand", xpos, yoff, txt_size, func_col, wt, 1.f, anim_translate(tt, trans));
		anim_wait(3.5f);
		anim_fade_out_forever(1.f);

		// Need to animate these separately, since they're going to show up AFTER the 'update sand' goes up top
		xpos = -20.f + _xoff;
		yoff = -grid_size + 250.f + _yoff;
		const f32 wt2 = wt + 5.f;
		const f32 ft2 = 3.f;

		code = "void update_sand(u32 x, u32 y)\n"\
		"{\n"\
		"  particle_t* p = get_p(x,y);\n"\
		"  // update particle as sand...\n"\
		"}";
		animate_code(code, v2(xpos, yoff), txt_size, 0.05f, wt2, 100.f);
	}
}

void play_scene_three()
{
	anim_mult = 1.5f;

	gs_for_range_i(gs_dyn_array_size(g_animations)) {
		animation_clear(&g_animations[i]);
	}
	gs_dyn_array_clear(g_animations);

	// Cache instance of graphics api from engine
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Viewport
	const gs_vec2 ws = frame_buffer_size;

	animation_t* anim = NULL;
	shape_t* shape = &g_shape;
	shape->xform = gs_vqs_default();
	u32 anim_ct = 0;

	gs_vec2 slp = {0};
	gs_vec2 elp = {0};
	const u32 num_cols = 10;
	const u32 num_rows = 10;
	const f32 grid_size = 750.f;
	const f32 ct = 2.4f;
	f32 cw = grid_size / (f32)num_cols;
	f32 ch = grid_size / (f32)num_rows;
	gs_vec2 chext = (gs_vec2){cw / 2.f - ct * 2.f, ch / 2.f - ct * 2.f};
	gs_vec2 ocp = (gs_vec2){cw / 2.f, ch / 2.f};

	gs_vqs grid_trans = gs_vqs_default();
	grid_trans.scale = v3(0.9f, 0.9f, 0.f);
	grid_trans.position = v3(grid_size * grid_trans.scale.x - 250.f, grid_size * grid_trans.scale.y - 130.f, 0.f);

	gs_vqs code_trans = gs_vqs_default();
	code_trans.position = v3(ws.x / 2.f + grid_size * 0.55f, ws.y / 2.f, 0.f);
	code_trans.scale = v3(0.9f, 0.9f, 1.f);

	gs_vec4 grid_col = v4(0.7f, 0.5f, 0.2f, 1.f);
	gs_vec4 highlight_col = v4(0.9f, 0.8f, 0.1f, 0.1f);

	const f32 glyph_height = 80.f;
	const f32 txt_size = 60.f;

	f32 fade_amt = 10.0f;

	f32 w = (cw - ct);
	f32 h = (ch - ct);

	const f32 cto = 3.f * ct;

	// Blocking square shape down
	shape_begin(shape);
	shape->xform = grid_trans;
	shape_draw_square(shape, v2(ocp.x - cw * 2.f, ocp.y + ch * 2.f), chext, ct, keyword_col, true);
	anim = new_anim(shape, 1.f);
	anim_disable(10.f);
	anim_wait(50.f);
	anim_transform(2.f, xform_translate(grid_trans, v3(cw - cto, 0.f, 0.f)));
	anim_wait(1000.f);

	// Second blocking square shape down
	shape_begin(shape);
	shape->xform = grid_trans;
	shape_draw_square(shape, v2(ocp.x - cw, ocp.y + ch * 2.f), chext, ct, keyword_col, true);
	anim = new_anim(shape, 1.f);
	anim_disable(62.f);
	anim_wait(45.f);
	anim_transform(2.f, xform_translate(grid_trans, v3(-cw + cto, 0.f, 0.f)));

	// Third blocking square shape down
	shape_begin(shape);
	shape->xform = grid_trans;
	shape_draw_square(shape, v2(ocp.x - cw, ocp.y + ch * 2.f), chext, ct, keyword_col, true);
	anim = new_anim(shape, 1.f);
	anim_disable(62.f);
	anim_wait(90.f);
	anim_transform(2.f, xform_translate(grid_trans, v3(cw - cto, 0.f, 0.f)));

	// Grid
	shape_begin(shape);
	shape->xform.position = v3(ws.x / 2.f, ws.y / 2.f, 0.f);
	shape_draw_grid(shape, v2(0.f, 0.f), grid_size, grid_size, num_rows, num_cols, 0.5f, white);
	anim = new_anim(shape, 1.f);
	anim_walk(6.f);
	anim_transform(2.5f, grid_trans);

	// Main square
	shape_begin(shape);
	shape->xform.position = v3(ws.x / 2.f, ws.y / 2.f, 0.f);
	shape_draw_square(shape, v2(ocp.x - cw, ocp.y + ch), chext, ct, grid_col, true);
	anim = new_anim(shape, 1.f);
	anim_walk(6.f);
	anim_transform(2.5f, grid_trans);
	anim_wait(30.f);
	anim_transform(2.f, xform_translate(grid_trans, v3(0.f, ch - cto, 0.f)));
	anim_wait(10.f);
	anim_fade_to(2.f, 0.f);
	anim_transform(1.f, grid_trans);	// Transform back to original position
	anim_fade_to(2.f, 1.f);				// Fade back in
	anim_wait(30.f);					// Wait some
	anim_transform(2.f, xform_translate(grid_trans, v3(-cw + cto, ch - cto, 0.f)));		// Transform down and to left
	anim_wait(10.f);
	anim_fade_to(2.f, 0.f);
	anim_transform(1.f, grid_trans);	// Transform back to original position
	anim_fade_to(2.f, 1.f);				// Fade back in
	anim_wait(28.f);					// Wait some
	anim_transform(2.f, xform_translate(grid_trans, v3(cw - cto, ch - cto, 0.f)));		// Transform down and to right
	anim_wait(10.f);
	anim_fade_to(2.f, 0.f);
	anim_transform(1.f, grid_trans);	// Transform back to original position
	anim_fade_to(2.f, 1.f);				// Fade back ino

	// Highlight square
	shape_begin(shape);
	shape->xform.position = v3(ws.x / 2.f, ws.y / 2.f, 0.f);
	shape_draw_square(shape, v2(ocp.x - cw, ocp.y + ch * 2.f), chext, ct, member_var_col, false);
	anim = new_anim(shape, 1.f);
	anim_fade_to(0.f, 0.f);				// Fade out
	anim_transform(2.5f, grid_trans);
	anim_wait(19.f);
	anim_fade_to(0.f, 1.f);				// Fade in
	anim_blink(0.1f, 10.f, 3, 3.f);
	anim_fade_to(3.f, 0.f);

	// Down to left
	anim_transform(1.f, xform_translate(grid_trans, v3(-cw + cto, 0.f, 0.f)));
	anim_wait(19.f);
	anim_fade_to(0.f, 1.f);				// Fade in
	anim_blink(0.1f, 10.f, 3, 3.f);
	anim_fade_to(3.f, 0.f);

	// Down to right
	anim_transform(1.f, xform_translate(grid_trans, v3(cw - cto, 0.f, 0.f)));
	anim_wait(21.f);
	anim_fade_to(0.f, 1.f);				// Fade in
	anim_blink(0.1f, 10.f, 3, 3.f);
	anim_fade_to(3.f, 0.f);

	// Down
	anim_transform(1.f, xform_translate(grid_trans, v3(0.f, 0.f, 0.f)));
	anim_wait(30.f);
	anim_fade_to(0.f, 1.f);				// Fade in
	anim_blink(0.1f, 10.f, 1, 1.f);
	anim_fade_to(1.f, 0.f);

	// Down to left
	anim_transform(1.f, xform_translate(grid_trans, v3(-cw + cto, 0.f, 0.f)));
	anim_fade_to(0.f, 1.f);				// Fade in
	anim_blink(0.1f, 10.f, 1, 1.f);
	anim_fade_to(1.f, 0.f);

	// Down to right
	anim_transform(1.f, xform_translate(grid_trans, v3(cw - cto, 0.f, 0.f)));
	anim_fade_to(0.f, 1.f);				// Fade in
	anim_blink(0.1f, 10.f, 1, 1.f);
	anim_fade_to(1.f, 0.f);

	// Arrow shape down
	shape_begin(shape);
	shape->xform = grid_trans;
	slp = v2(ocp.x - cw, ocp.y + ch);
	elp = gs_vec2_add(slp, v2(0.f, ch));
	shape_draw_arrow(shape, slp, elp, 8.f, 8.f, 1.f, member_var_col);
	anim = new_anim(shape, 1.f);
	anim_disable(20.f);
	anim_walk(2.f);
	anim_wait(17.f);
	anim_fade_to(3.f, 0.f);
	anim_wait(120.f);
	anim_fade_to(1.f, 1.f);
	anim_wait(15.f);
	anim_fade_to(15.f, 0.f);

	// Arrow shape left
	shape_begin(shape);
	shape->xform = grid_trans;
	slp = v2(ocp.x - cw, ocp.y + ch);
	elp = gs_vec2_add(slp, v2(-cw, ch));
	shape_draw_arrow(shape, slp, elp, 8.f, 8.f, 1.f, member_var_col);
	anim = new_anim(shape, 1.f);
	anim_disable(65.f);
	anim_walk(2.f);
	anim_wait(17.f);
	anim_fade_to(3.f, 0.f);
	anim_wait(80.f);
	anim_fade_to(1.f, 1.f);
	anim_wait(10.f);
	anim_fade_to(15.f, 0.f);

	// Arrow shape right
	shape_begin(shape);
	shape->xform = grid_trans;
	slp = v2(ocp.x - cw, ocp.y + ch);
	elp = gs_vec2_add(slp, v2(cw, ch));
	shape_draw_arrow(shape, slp, elp, 8.f, 8.f, 1.f, member_var_col);
	anim = new_anim(shape, 1.f);
	anim_disable(110.f);
	anim_walk(2.f);
	anim_wait(17.f);
	anim_fade_to(3.f, 0.f);
	anim_wait(40.f);
	anim_fade_to(1.f, 1.f);
	anim_wait(2.f);
	anim_fade_to(15.f, 0.f);

	// Code arrow
	shape_begin(shape);
	shape->xform = code_trans;
	slp = v2(-550.f, -155.f);
	elp = gs_vec2_add(slp, v2(50.f, 0.f));
	shape_draw_arrow(shape, slp, elp, 8.f, 8.f, 1.f, func_col);
	anim = new_anim(shape, 1.f);
	const f32 offset = txt_size - 14.5f;
	anim_disable(15.f);
	anim_walk(4.f);
	anim_wait(20.f);
	anim_transform(2.f, xform_translate(shape->xform, v3(0.f, offset, 0.f)));
	anim_wait(18.f);
	anim_transform(2.f, xform_translate(shape->xform, v3(0.f, 2.f * offset, 0.f)));
	anim_wait(25.f);
	anim_transform(2.f, xform_translate(shape->xform, v3(0.f, 3.f * offset, 0.f)));
	anim_wait(19.f);
	anim_transform(2.f, xform_translate(shape->xform, v3(0.f, 4.f * offset, 0.f)));
	anim_wait(23.f);
	anim_transform(2.f, xform_translate(shape->xform, v3(0.f, 5.f * offset, 0.f)));
	anim_wait(13.f);
	anim_transform(2.f, xform_translate(shape->xform, v3(0.f, 6.f * offset, 0.f)));
	anim_wait(26.f);
	anim_transform(2.f, xform_translate(shape->xform, v3(0.f, 7.f * offset, 0.f)));
	anim_fade_to(5.f, 0.f);

	// Code background
	shape_begin(shape);
	shape->xform = code_trans;
	shape->xform.position = gs_vec3_add(shape->xform.position, v3(-50.f, 0.f, 0.f));
	shape_draw_line(shape, v2(0.f, -grid_size * 0.57f), 
		v2(0.f, grid_size * 0.57f), grid_size * 0.39f, color_alpha(code_bg_col, 0.3f));
	shape_draw_square(shape, (gs_vec2){0.f, 0.f}, 
		v2(grid_size * 0.739f, grid_size * 0.6f), 2.0f, v4(0.2f, 0.2f, 0.2f, 1.f), false);
	anim = new_anim(shape, 1.f);
	anim_disable(5.f);
	anim_walk(10.f);

	// Code
	f32 xpos = 0.f;
	f32 ypos = -400.f;

	f32 _xpos = 20.f;
	animate_txt("falling sand algorithm\n", _xpos, ypos, txt_size + 10.f, primitive_col, 10.f, 1.f);
	shape_begin(shape);
	shape->xform = code_trans;
	shape->xform.position = gs_vec3_add(shape->xform.position, v3(-50.f, 0.f, 0.f));
	shape_draw_line(shape, v2(0.f, -grid_size * 0.55f), 
		v2(0.f, -grid_size * 0.38f), grid_size * 0.35f, color_alpha(keyword_col, 0.01f));
	anim = new_anim(shape, 1.f);
	anim_disable(5.f);
	anim_walk(10.f);

	ypos = -200.f;
	animate_txt("if down empty:\n", xpos, ypos, txt_size, default_col, 20.f, 1.f);
	animate_txt("  move down\n", xpos, ypos, txt_size, keyword_col, 40.f, 1.f);

	animate_txt("elif down and left empty:\n", xpos, ypos, txt_size, default_col, 60.f, 1.f);
	animate_txt("  move down and left\n", xpos, ypos, txt_size, keyword_col, 90.f, 1.f);

	animate_txt("elif down and right empty:\n", xpos, ypos, txt_size, default_col, 110.f, 1.f);
	animate_txt("  move down and right\n", xpos, ypos, txt_size, keyword_col, 135.f, 1.f);

	animate_txt("else:\n", xpos, ypos, txt_size, default_col, 150.f, 1.f);
	animate_txt("  stay\n", xpos, ypos, txt_size, keyword_col, 180.f, 1.f);

}

void play_scene_four()
{
	anim_mult = 2.f;

	gs_for_range_i(gs_dyn_array_size(g_animations)) {
		animation_clear(&g_animations[i]);
	}
	gs_dyn_array_clear(g_animations);

	// Cache instance of graphics api from engine
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Viewport
	const gs_vec2 ws = frame_buffer_size;

	animation_t* anim = NULL;
	shape_t* shape = &g_shape;
	shape->xform = gs_vqs_default();
	u32 anim_ct = 0;

	gs_vec2 slp = {0};
	gs_vec2 elp = {0};
	const u32 num_cols = 10;
	const u32 num_rows = 10;
	const f32 grid_size = 800.f;
	const f32 ct = 2.4f;
	f32 cw = grid_size / (f32)num_cols;
	f32 ch = grid_size / (f32)num_rows;
	gs_vec2 chext = (gs_vec2){cw / 2.f - ct * 2.f, ch / 2.f - ct * 2.f};
	gs_vec2 ocp = (gs_vec2){cw / 2.f, ch / 2.f};

	gs_vqs grid_trans = gs_vqs_default();
	grid_trans.scale = v3(1.f, 1.f, 0.f);
	grid_trans.position = v3(ws.x / 2.f, ws.y / 2.f, 0.f);

	gs_vqs code_trans = gs_vqs_default();
	code_trans.position = v3(ws.x / 2.f + grid_size * 0.55f, ws.y / 2.f, 0.f);
	code_trans.scale = v3(0.9f, 0.9f, 1.f);

	gs_vec4 grid_col = v4(0.7f, 0.5f, 0.2f, 1.f);
	gs_vec4 highlight_col = v4(0.9f, 0.8f, 0.1f, 0.1f);

	const f32 glyph_height = 80.f;
	const f32 txt_size = 60.f;

	f32 fade_amt = 10.0f;

	f32 w = (cw - ct);
	f32 h = (ch - ct);

	const f32 cto = 3.f * ct;
	const f32 fall_speed = 5.f;
	const f32 bw = fall_speed * 4.f;
	const f32 wo = 10.f;

	// Fall to bottom
	shape_begin(shape);
	shape->xform = grid_trans;
	shape_draw_square(shape, v2(ocp.x, ocp.y - ch * 5.f), chext, ct, grid_col, true);
	anim = new_anim(shape, 1.f);
	anim_disable((bw * 0.f) / fall_speed + wo);
	for (u32 i = 0; i < num_rows; ++i) {
		anim_transform(1.f / fall_speed, xform_translate(shape->xform, v3(0.f, i * (ch), 0.f)));
		anim_wait(2.f / fall_speed);
	}

	shape_begin(shape);
	shape->xform = grid_trans;
	shape_draw_square(shape, v2(ocp.x, ocp.y - ch * 5.f), chext, ct, grid_col, true);
	anim = new_anim(shape, 1.f);
	anim_disable((bw * 1.f) / fall_speed + wo);
	for (u32 i = 0; i < num_rows - 1; ++i) {
		anim_transform(1.f / fall_speed, xform_translate(shape->xform, v3(0.f, i * (ch), 0.f)));
		anim_wait(2.f / fall_speed);
	}
	for (u32 i = num_rows - 1; i < num_rows; ++i) {
		anim_transform(1.f / fall_speed, xform_translate(shape->xform, v3(-cw, i * (ch), 0.f)));
		anim_wait(2.f / fall_speed);
	}

	shape_begin(shape);
	shape->xform = grid_trans;
	shape_draw_square(shape, v2(ocp.x, ocp.y - ch * 5.f), chext, ct, grid_col, true);
	anim = new_anim(shape, 1.f);
	anim_disable((bw * 2.f) / fall_speed + wo);
	for (u32 i = 0; i < num_rows - 1; ++i) {
		anim_transform(1.f / fall_speed, xform_translate(shape->xform, v3(0.f, i * (ch), 0.f)));
		anim_wait(2.f / fall_speed);
	}
	for (u32 i = num_rows - 1; i < num_rows; ++i) {
		anim_transform(1.f / fall_speed, xform_translate(shape->xform, v3(cw, i * (ch), 0.f)));
		anim_wait(2.f / fall_speed);
	}

	shape_begin(shape);
	shape->xform = grid_trans;
	shape_draw_square(shape, v2(ocp.x, ocp.y - ch * 5.f), chext, ct, grid_col, true);
	anim = new_anim(shape, 1.f);
	anim_disable((bw * 3.f) / fall_speed + wo);
	for (u32 i = 0; i < num_rows - 1; ++i) {
		anim_transform(1.f / fall_speed, xform_translate(shape->xform, v3(0.f, i * (ch), 0.f)));
		anim_wait(2.f / fall_speed);
	}

	shape_begin(shape);
	shape->xform = grid_trans;
	shape_draw_square(shape, v2(ocp.x, ocp.y - ch * 5.f), chext, ct, grid_col, true);
	anim = new_anim(shape, 1.f);
	anim_disable((bw * 4.f) / fall_speed + wo);
	for (u32 i = 0; i < num_rows - 2; ++i) {
		anim_transform(1.f / fall_speed, xform_translate(shape->xform, v3(0.f, i * (ch), 0.f)));
		anim_wait(2.f / fall_speed);
	}
	for (u32 i = num_rows - 2, j = 1; i < num_rows; ++i, ++j) {
		anim_transform(1.f / fall_speed, xform_translate(shape->xform, v3(-cw * j, i * ch, 0.f)));
		anim_wait(2.f / fall_speed);
	}

	shape_begin(shape);
	shape->xform = grid_trans;
	shape_draw_square(shape, v2(ocp.x, ocp.y - ch * 5.f), chext, ct, grid_col, true);
	anim = new_anim(shape, 1.f);
	anim_disable((bw * 5.f) / fall_speed + wo);
	for (u32 i = 0; i < num_rows - 2; ++i) {
		anim_transform(1.f / fall_speed, xform_translate(shape->xform, v3(0.f, i * (ch), 0.f)));
		anim_wait(2.f / fall_speed);
	}
	for (u32 i = num_rows - 2, j = 1; i < num_rows - 1; ++i, ++j) {
		anim_transform(1.f / fall_speed, xform_translate(shape->xform, v3(-cw, i * ch, 0.f)));
		anim_wait(2.f / fall_speed);
	}

	shape_begin(shape);
	shape->xform = grid_trans;
	shape_draw_square(shape, v2(ocp.x, ocp.y - ch * 5.f), chext, ct, grid_col, true);
	anim = new_anim(shape, 1.f);
	anim_disable((bw * 6.f) / fall_speed + wo);
	for (u32 i = 0; i < num_rows - 2; ++i) {
		anim_transform(1.f / fall_speed, xform_translate(shape->xform, v3(0.f, i * (ch), 0.f)));
		anim_wait(2.f / fall_speed);
	}
	for (u32 i = num_rows - 2, j = 1; i < num_rows; ++i, ++j) {
		anim_transform(1.f / fall_speed, xform_translate(shape->xform, v3(cw * j , i * ch, 0.f)));
		anim_wait(2.f / fall_speed);
	}

	shape_begin(shape);
	shape->xform = grid_trans;
	shape_draw_square(shape, v2(ocp.x, ocp.y - ch * 5.f), chext, ct, grid_col, true);
	anim = new_anim(shape, 1.f);
	anim_disable((bw * 7.f) / fall_speed + wo);
	for (u32 i = 0; i < num_rows - 2; ++i) {
		anim_transform(1.f / fall_speed, xform_translate(shape->xform, v3(0.f, i * (ch), 0.f)));
		anim_wait(2.f / fall_speed);
	}
	for (u32 i = num_rows - 2, j = 1; i < num_rows - 1; ++i, ++j) {
		anim_transform(1.f / fall_speed, xform_translate(shape->xform, v3(cw * j , i * ch, 0.f)));
		anim_wait(2.f / fall_speed);
	}

	shape_begin(shape);
	shape->xform = grid_trans;
	shape_draw_square(shape, v2(ocp.x, ocp.y - ch * 5.f), chext, ct, grid_col, true);
	anim = new_anim(shape, 1.f);
	anim_disable((bw * 8.f) / fall_speed + wo);
	for (u32 i = 0; i < num_rows - 2; ++i) {
		anim_transform(1.f / fall_speed, xform_translate(shape->xform, v3(0.f, i * (ch), 0.f)));
		anim_wait(2.f / fall_speed);
	}

	// Grid
	shape_begin(shape);
	shape->xform.position = v3(ws.x / 2.f, ws.y / 2.f, 0.f);
	shape_draw_grid(shape, v2(0.f, 0.f), grid_size, grid_size, num_rows, num_cols, 0.5f, white);
	anim = new_anim(shape, 1.f);
	anim_fade_to(0.f, 0.f);
	anim_fade_to(20.f, 10.f);
}











