#include <gs.h>

// Forward Decls.
gs_result app_init();		// Use to init your application
gs_result app_update();		// Use to update your application
gs_result app_shutdown();	// Use to shutdown your appliaction

// Ported to a c99 impl from: https://github.com/CrushedPixel/Polyline2D/

// Colors
#define white (gs_vec4){1.f, 1.f, 1.f, 1.f}
#define red (gs_vec4){1.f, 0.f, 0.f, 1.f}
#define green (gs_vec4){0.f, 1.f, 0.f, 1.f}
#define blue (gs_vec4){0.f, 0.f, 1.f, 1.f}

_global b32 anti_alias = true;

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

			f32 s_thick = gs_max(start1.thickness, start2.thickness) / 0.8f; 
			f32 e_thick = gs_max(end1.thickness, end2.thickness) / 0.8f; 

			// Push back verts for anti-aliasing as well...somehow
			gs_vec2 sn = gs_vec2_norm(gs_vec2_sub(start2.position, start1.position));
			gs_vec2 en = gs_vec2_norm(gs_vec2_sub(end2.position, end1.position));
			const f32 sl = gs_vec2_len(gs_vec2_sub(start2.position, start1.position)) / (s_thick);
			const f32 el = gs_vec2_len(gs_vec2_sub(end2.position, end1.position)) / (e_thick);

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
			gs_dyn_array_push(*vertices, vert_t_create(outer1->b.position, outer1->a.color));
			gs_dyn_array_push(*vertices, vert_t_create(outer2->a.position, outer2->b.color));
			gs_dyn_array_push(*vertices, vert_t_create(inner_sec.position, inner_sec.color));

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

		start_point = end_point;
	}
}

// Command buffer global var
_global gs_resource( gs_command_buffer ) g_cb = {0};
_global gs_resource( gs_shader ) g_shader = {0};
_global gs_resource( gs_vertex_buffer ) g_vb = {0};
_global gs_resource( gs_index_buffer ) g_ib = {0};
_global gs_resource( gs_uniform ) u_proj = {0};
_global gs_resource( gs_uniform ) u_view = {0};

_global gs_dyn_array( vert_t ) verts;

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

	verts = gs_dyn_array_new( vert_t );

	return gs_result_success;
}

void draw_line( gs_dyn_array(vert_t)* verts, gs_vec2 start, gs_vec2 end, f32 thickness, gs_vec4 color )
{
	poly_point_t points[] = 
	{
		poly_point_create(start, white, thickness / 100.f), 
		poly_point_create(end, red, thickness * 10.f)
	};
	u32 pt_count = sizeof(points) / sizeof(poly_point_t);

	// Pass in verts for creating poly line
	poly_line_create(verts, points, pt_count, joint_style_miter, end_cap_style_butt, true);
}

void draw_circle( gs_dyn_array(vert_t)* verts, gs_vec2 origin, f32 r, s32 num_segments, f32 thickness, gs_vec4 color )
{
	gs_dyn_array(poly_point_t) points = gs_dyn_array_new(poly_point_t);

	f32 step = num_segments < 5 ? 360.f / 5.f : 360.f / (f32)num_segments;
	for ( f32 i = 0; i < 360.f; i += step ) {
		f32 a = gs_deg_to_rad(i);
		poly_point_t p = poly_point_create(gs_vec2_add(origin, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								color, thickness);
		gs_dyn_array_push(points, p);
	}
	// Push last angle on as well
	{
		f32 a = gs_deg_to_rad(360.f);
		poly_point_t p = poly_point_create(gs_vec2_add(origin, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								color, thickness);
		gs_dyn_array_push(points, p);
	}
	u32 pt_count = gs_dyn_array_size(points);

	// Pass in verts for creating poly line
	poly_line_create(verts, points, pt_count, joint_style_round, end_cap_style_joint, true);

	gs_dyn_array_free(points);
}

void draw_arc( gs_dyn_array(vert_t)* verts, gs_vec2 origin, f32 r, f32 start_angle, f32 end_angle, s32 num_segments, f32 thickness, gs_vec4 color )
{
	gs_dyn_array(poly_point_t) points = gs_dyn_array_new(poly_point_t);
	if ( start_angle > end_angle ) {
		f32 tmp = start_angle;
		start_angle = end_angle;
		end_angle = tmp;
	}
	f32 diff = end_angle - start_angle;
	if ( fabsf(diff) <= 0.01f ) {
		return;
	}

	f32 step = num_segments < 5 ? diff / 5.f : diff / (f32)num_segments;
	for ( f32 i = start_angle; i <= end_angle; i += step ) {
		f32 a = gs_deg_to_rad(i);
		poly_point_t p = poly_point_create(gs_vec2_add(origin, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								color, thickness);
		gs_dyn_array_push(points, p);
	}
	// Push last angle on as well
	{
		f32 a = gs_deg_to_rad(end_angle);
		poly_point_t p = poly_point_create(gs_vec2_add(origin, gs_vec2_scale((gs_vec2){cos(a), sin(a)}, r)), 
								color, thickness);
		gs_dyn_array_push(points, p);
	}
	u32 pt_count = gs_dyn_array_size(points);

	// Pass in verts for creating poly line
	poly_line_create(verts, points, pt_count, joint_style_round, end_cap_style_butt, true);

	gs_dyn_array_free(points);
}

void draw_square( gs_dyn_array(vert_t)* verts, gs_vec2 origin, gs_vec2 half_extents, f32 thickness, gs_vec4 color )
{
	poly_point_t points[] = 
	{
		poly_point_create((gs_vec2){origin.x - half_extents.x, origin.y - half_extents.y}, color, thickness),
		poly_point_create((gs_vec2){origin.x + half_extents.x, origin.y - half_extents.y}, color, thickness),
		poly_point_create((gs_vec2){origin.x + half_extents.x, origin.y + half_extents.y}, color, thickness),
		poly_point_create((gs_vec2){origin.x - half_extents.x, origin.y + half_extents.y}, color, thickness)
	};
	u32 pt_count = sizeof(points) / sizeof(poly_point_t);

	// Pass in verts for creating poly line
	poly_line_create(verts, points, pt_count, joint_style_miter, end_cap_style_joint, true);
}

void draw_bezier_curve( gs_dyn_array(vert_t)* verts, gs_vec2 cp0, gs_vec2 cp1, gs_vec2 cp2, gs_vec2 cp3, u32 num_segments, f32 thickness, gs_vec4 color )
{
	gs_dyn_array(poly_point_t) points = gs_dyn_array_new(poly_point_t);
	f64 xu = 0.0, yu = 0.0; 	
	f64 step = num_segments < 3 ? 3 : 1.0 / num_segments;
	for ( f64 u = 0.0; u <= 1.0; u += step )
	{
		xu = pow(1.0 - u, 3) * cp0.x + 3 * u * pow(1.0 - u, 2) * cp1.x + pow(u, 2) * (1.0 - u) * cp2.x + 
			pow(u, 3) * cp3.x;
		yu = pow(1.0 - u, 3) * cp0.y + 3 * u * pow(1.0 - u, 2) * cp1.y + 3 * pow(u, 2) * (1.0 - u) * cp2.y + 
			pow(u, 3) * cp3.y;
		poly_point_t p = poly_point_create((gs_vec2){xu, yu}, color, thickness);
		gs_dyn_array_push(points, p);
	}
	u32 pt_count = gs_dyn_array_size(points);

	// Pass in verts for creating poly line
	poly_line_create(verts, points, pt_count, joint_style_miter, end_cap_style_butt, true);

	gs_dyn_array_free(points);
}

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

	/*===============
	// Render scene
	================*/

	/*
	Note: 
		You'll notice that 'g_cb', our command buffer handle, is the first argument for every one of these api calls.
		All graphics api functions for immediate rendering will require a command buffer. 
		Every call you make adds these commands into this buffer and then will be processed later on in the frame
		for drawing. This allows you to make these calls at ANY point in your code before the final rendering submission occurs.
	*/

	// poly line - hand the drawing api a context and then a list of path points.
	// shape desc - describes properties about the shape being created, like stroke, fill, color, etc.

	// Need to determine normal of line at given point.
	static f32 x = 0.f;
	static f32 y = 0.f;

	gs_vec2 center = (gs_vec2){ ws.x / 2.f, ws.y / 2.f };
	f32 r = 200.f;
	f32 tv = sin(t * 0.3f) * 0.5f + 0.5f;

	x = cos(gs_deg_to_rad(gs_interp_smooth_step( 180.f, 360.f, tv))) * r;
	y = sin(gs_deg_to_rad(gs_interp_smooth_step( 180.f, 360.f, tv))) * r;

	gs_vec2 sp = (gs_vec2){ center.x + x, center.y + y };
	gs_vec2 ep = (gs_vec2){ center.x, center.y }; 

	gs_vec2 a = gs_vec2_norm(gs_vec2_sub(ep, sp));
	gs_vec2 n = (gs_vec2){-a.y, a.x};

	static f32 width = 10.f;
	if ( platform->key_down( gs_keycode_q ) ) {
		width -= 1.f;
	}
	if ( platform->key_down( gs_keycode_e ) ) {
		width += 1.f;
	}
	width = gs_clamp(width, 1.0f, 1000.f);

	// Add a square to be drawn
	gs_vqs xform = gs_vqs_default();
	f32 scl = gs_interp_smooth_step( 1.f, 2.f, sin(t * 0.7f) * 0.5f + 0.5f);
	xform.rotation = gs_quat_angle_axis(t * 0.1f, (gs_vec3){0.f, 0.f, 1.f});
	gs_mat4 model = gs_vqs_to_mat4(&xform);

	// Pass in a descriptor to close the path for certain paths, like a square

	gs_vec2 origin = (gs_vec2){ws.x / 2.f, ws.y / 2.f};
	gs_vec2 h_ext = (gs_vec2){10.f, 10.f};
	gs_vec3 tl = gs_mat4_mul_vec3(model, (gs_vec3){origin.x - h_ext.x, origin.y - h_ext.y, 0.f});
	gs_vec3 tr = gs_mat4_mul_vec3(model, (gs_vec3){origin.x + h_ext.x, origin.y - h_ext.y, 0.f});
	gs_vec3 br = gs_mat4_mul_vec3(model, (gs_vec3){origin.x + h_ext.x, origin.y + h_ext.y, 0.f});
	gs_vec3 bl = gs_mat4_mul_vec3(model, (gs_vec3){origin.x - h_ext.x, origin.y + h_ext.y, 0.f});

	static f32 thickness = 1.f;
	if (platform->key_down(gs_keycode_e)) {
		thickness += 1.f;
	}
	if (platform->key_down(gs_keycode_q)) {
		thickness -= 1.f;
	}
	thickness = gs_clamp(thickness, 1.f, 1000.f);

	draw_line( &verts, sp, ep, thickness, red );
	// draw_circle( &verts, sp, 150.f, 100, thickness, green );
	// draw_square( &verts, sp, (gs_vec2){100.f, 100.f}, thickness, blue );
	draw_bezier_curve( &verts, (gs_vec2){100.f, 100.f}, (gs_vec2){200.f, 200.f}, 
		(gs_vec2){300.f, 10.f}, (gs_vec2){600.f, 50.f}, 20, thickness, white );

	f32 start_angle = gs_interp_cosine( 0.f, 180.f, tv);
	f32 end_angle = gs_interp_cosine( 180.f, 360.f, tv);
	gs_vec4 col = (gs_vec4){gs_interp_cosine(0.f, 1.f, tv), gs_interp_cosine(1.f, 0.f, tv), 0.1f, tv};
	draw_arc( &verts, (gs_vec2){ws.x / 2.f, ws.y / 2.f}, r + 30.f, 180.f, end_angle, 100, 2.f, col );

	// Tweening square
	{
		// Want to be able to pass in a transform as well for rotation/scale/etc.
		// draw_square( &verts, sp, (gs_vec2){50.f, 50.f}, thickness, white );
		// Need to get complex poly lines working, so I can have connected multiple lines.

		gs_vec2 bcsp = (gs_vec2){100.f, 100.f};
		// draw_line( &verts, (gs_vec2){100.f, 10.f}, bcsp, thickness, red );
		// draw_bezier_curve( &verts, bcsp, (gs_vec2){200.f, 200.f}, 
		// 	(gs_vec2){300.f, 10.f}, (gs_vec2){600.f, 50.f}, 20, thickness, white );
	}

	// Animating arcs
	/*
	{
		// Try to animate an arc (empty -> full circle -> empty) over time
		const f32 rad = 40.f;
		const f32 thick = 9.f;
		const u32 count = 1;
		const f32 offset = 0.f;

		gs_for_range_i( count )
		{
			f32 v = sin(t * 0.3f + i) * 0.5f + 0.5f;
			f32 y = gs_interp_cosine( 0.f, ws.y / 2.f, v);
			f32 start_angle = gs_interp_cosine( 0.f, 180.f, v);
			f32 end_angle = gs_interp_cosine( 180.f, 360.f, v);
			gs_vec4 col = (gs_vec4){gs_interp_cosine(0.f, 1.f, v), 0.f, gs_interp_cosine(1.f, 0.f, v), v};
			draw_arc( &verts, (gs_vec2){i * 100.f + ws.x / 2.f - offset, y}, rad, start_angle, end_angle, 100, thick, col );
		}

		gs_for_range_i( count )
		{
			f32 v = sin(t * 0.3f + i) * 0.5f + 0.5f;
			f32 y = gs_interp_smooth_step( ws.y, ws.y / 2.f, v);
			f32 start_angle = gs_clamp(gs_interp_smooth_step( 180.f, 0.f, v), 0.f, 180.f);
			// f32 end_angle = gs_interp_smooth_step( 360.f, 180.f, v);
			f32 end_angle = gs_interp_smooth_step( 0.f, 180.f, v);
			gs_vec4 col = (gs_vec4){gs_interp_smooth_step(0.f, 1.f, v), 0.f, 1.f, v};
			draw_arc( &verts, (gs_vec2){i * 100.f + ws.x / 2.f - offset, ws.y / 2.f}, rad, start_angle, 180.f, 100, thick, col );
		}
	}
	*/

	// Update vertex data for frame
	gfx->update_vertex_buffer_data( g_vb, verts, gs_dyn_array_size(verts) * sizeof(vert_t) );

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
	gfx->draw( g_cb, 0, gs_dyn_array_size( verts ) );

	// Submit command buffer for rendering
	gfx->submit_command_buffer( g_cb ); 	// I suppose this COULD flush the command buffer altogether? ...

	 // Clear line data from array
	gs_dyn_array_clear( verts );

	return gs_result_in_progress;
}

gs_result app_shutdown()
{
	// Free all da things
	gs_dyn_array_free( verts );

	gs_println( "Goodbye, Gunslinger." );
	return gs_result_success;
}

/*
	path_start(ctx);
		path_set_end_cap_style(ctx, end_cap_style);
		path_set_connecting_joint_style(ctx, joint_style)
		path_add_points(ctx, line_pts, joint_style);
		path_add_points(ctx, bezier_curve_pts, joint_style);
	path_end(ctx);

	// Just need a simple way to connect two polylines with different joint styles with single end_cap style
	// Test is to do letters, like Google 'G'

*/

