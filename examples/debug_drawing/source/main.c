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
#define col_empty (gs_vec4){0.f, 0.f, 0.f, 0.f}

_global b32 anti_alias = true;

 // ~20 degrees
#define miter_min_angle gs_deg_to_rad(20)
 // ~10 degrees
#define round_min_angle gs_deg_to_rad(20)

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
	gs_vec2 a;
	gs_vec2 b;
} line_segment_t;

typedef struct line_segment_intersection_t
{
	b32 intersected;
	gs_vec2 point;
} line_segment_intersection_t;

/**
 * @return A copy of the line segment, offset by the given vector.
 */
line_segment_t line_seg_add( line_segment_t l, gs_vec2 os ) 
{
	return (line_segment_t){ gs_vec2_add(l.a, os), gs_vec2_add(l.b, os) };
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
	gs_vec2 vec = gs_vec2_sub(l.b, l.a);
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
	(line_segment_intersection_t){ false, (gs_vec2){0.f, 0.f}}

line_segment_intersection_t line_seg_intersection( line_segment_t a, line_segment_t b, b32 infiniteLines ) 
{
	// calculate un-normalized direction vectors
	gs_vec2 r = line_seg_dir( a, false );
	gs_vec2 s = line_seg_dir( b, false );

	gs_vec2 origin_dist = gs_vec2_sub(b.a, a.a);

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
	return (line_segment_intersection_t){ true, gs_vec2_add(a.a, gs_vec2_scale(r, t)) };
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
	                                  joint_style_t joint_style, gs_vec2* end1, gs_vec2* end2,
	                                  gs_vec2* next_start1, gs_vec2* next_start2,
	                                  b32 allow_overlap);

void poly_line_create_triangle_fan( gs_dyn_array(vert_t)* vertices, gs_vec2 connect_to, gs_vec2 origin,
                                        gs_vec2 start, gs_vec2 end, b32 clockwise );

poly_segment_t poly_seg_create( line_segment_t center, f32 thickness) 
{
	poly_segment_t p = {0};
	p.center = center;
	// calculate the segment's outer edges by offsetting
	// the central line by the normal vector
	// multiplied with the thickness

	// center + center.normal() * thickness
	// This is for growing along center
	p.edge1 = line_seg_add(center, gs_vec2_scale(line_seg_normal(center), thickness));
	p.edge2 = line_seg_sub(center, gs_vec2_scale(line_seg_normal(center), thickness));

	// This is for growing 'outwards' away from center
	// p.edge1 = center;
	// p.edge2 = line_seg_sub(center, gs_vec2_scale(line_seg_normal(center), thickness * 2.f));

	// This is for growing 'inwards' away from center
	// p.edge1 = center;
	// p.edge2 = line_seg_add(center, gs_vec2_scale(line_seg_normal(center), thickness * 2.f));

	return p;
}

void poly_line_create( gs_dyn_array(vert_t)* vertices, gs_vec2* points, u32 count, f32 thickness,
                             joint_style_t joint_style,
                             end_cap_style_t end_cap_style,
                             b32 allow_overlap) 
{
	// operate on half the thickness to make our lives easier
	thickness /= 2.0f;

	gs_dyn_array(poly_segment_t) segments = gs_dyn_array_new(poly_segment_t);
	for (u32 i = 0; i + 1 < count; ++i) 
	{
		gs_vec2 point1 = points[i];
		gs_vec2 point2 = points[i + 1];

		// to avoid division-by-zero errors,
		// only create a line segment for non-identical points
		if (!gs_vec2_equal(point1, point2)) 
		{
			poly_segment_t ps = poly_seg_create( (line_segment_t){point1, point2}, thickness ); 
			gs_dyn_array_push(segments, ps);
		}
	}

	if (end_cap_style == end_cap_style_joint) {
		// create a connecting segment from the last to the first point

		gs_vec2 point1 = points[count - 1];
		gs_vec2 point2 = points[0];

		// to avoid division-by-zero errors,
		// only create a line segment for non-identical points
		if (!gs_vec2_equal(point1, point2)) 
		{
			poly_segment_t ps = poly_seg_create( (line_segment_t){point1, point2}, thickness ); 
			gs_dyn_array_push(segments, ps);
		}
	}

	if (gs_dyn_array_empty(segments)) {
		// handle the case of insufficient input points
		return;
	}

	gs_vec2 next_start1 = (gs_vec2){0.f, 0.f};
	gs_vec2 next_start2 = (gs_vec2){0.f, 0.f};
	gs_vec2 start1 = (gs_vec2){0.f, 0.f};
	gs_vec2 start2 = (gs_vec2){0.f, 0.f};
	gs_vec2 end1 = (gs_vec2){0.f, 0.f};
	gs_vec2 end2 = (gs_vec2){0.f, 0.f};

	// calculate the path's global start and end points
	poly_segment_t first_segment = segments[0];
	poly_segment_t last_segment = segments[gs_dyn_array_size(segments) - 1];

	gs_vec2 path_start1 = first_segment.edge1.a;
	gs_vec2 path_start2 = first_segment.edge2.a;
	gs_vec2 path_end1 = last_segment.edge1.b;
	gs_vec2 path_end2 = last_segment.edge2.b;

	// handle different end cap styles
	if (end_cap_style == end_cap_style_square) {
		// extend the start/end points by half the thickness
		path_start1 = gs_vec2_sub(path_start1, gs_vec2_scale(line_seg_dir(first_segment.edge1, true), thickness));
		path_start2 = gs_vec2_sub(path_start2, gs_vec2_scale( line_seg_dir(first_segment.edge2, true), thickness));
		path_end1 = gs_vec2_add(path_end1, gs_vec2_scale( line_seg_dir(last_segment.edge1, true), thickness));
		path_end2 = gs_vec2_add(path_end2, gs_vec2_scale( line_seg_dir(last_segment.edge2, true), thickness));

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

			// Push back verts for anti-aliasing as well...somehow
			gs_vec2 sn = gs_vec2_norm(gs_vec2_sub(start2, start1));
			gs_vec2 en = gs_vec2_norm(gs_vec2_sub(end2, end1));
			const f32 sl = gs_vec2_len(gs_vec2_sub(start2, start1)) / (thickness / 1.1f);
			const f32 el = gs_vec2_len(gs_vec2_sub(end2, end1)) / (thickness / 1.1f);

			gs_vec4 s1_col = (i == 0) && (end_cap_style == end_cap_style_square || end_cap_style == end_cap_style_butt) ? 
				white : col_empty;
			gs_vec4 s2_col = s1_col;

			gs_vec2 s1 = gs_vec2_sub(start1, gs_vec2_scale(sn, sl));
			gs_vec2 s2 = gs_vec2_add(start2, gs_vec2_scale(sn, sl));
			gs_vec2 e1 = gs_vec2_sub(end1, gs_vec2_scale(en, el));
			gs_vec2 e2 = gs_vec2_add(end2, gs_vec2_scale(en, el));

			gs_dyn_array_push(*vertices, vert_t_create(s1, s1_col));
			gs_dyn_array_push(*vertices, vert_t_create(start1, white));
			gs_dyn_array_push(*vertices, vert_t_create(e1, col_empty));

			gs_dyn_array_push(*vertices, vert_t_create(e1, col_empty));
			gs_dyn_array_push(*vertices, vert_t_create(end1, white));
			gs_dyn_array_push(*vertices, vert_t_create(start1, white));

			gs_dyn_array_push(*vertices, vert_t_create(s2, s2_col));
			gs_dyn_array_push(*vertices, vert_t_create(start2, white));
			gs_dyn_array_push(*vertices, vert_t_create(e2, col_empty));

			gs_dyn_array_push(*vertices, vert_t_create(e2, col_empty));
			gs_dyn_array_push(*vertices, vert_t_create(end2, white));
			gs_dyn_array_push(*vertices, vert_t_create(start2, white));

			// If we're at beginning and not end_cap_joint, then we need to anti-alias edge
			if ( i == 0 && end_cap_style == end_cap_style_square || end_cap_style == end_cap_style_butt )
			{
				snc = gs_vec2(-sn.y, sn.x); 
				gs_vec2 s1c = gs_vec2_sub(s1, gs_vec2_scale(snc, sl));
				gs_vec2 s2c = gs_vec2_sub(s2, gs_vec2_scale(snc, sl));

				gs_dyn_array_push(*vertices, vert_t_create(s1c, col_empty));
				gs_dyn_array_push(*vertices, vert_t_create(s1, s1_col));
				gs_dyn_array_push(*vertices, vert_t_create(s2, s2_col));

				gs_dyn_array_push(*vertices, vert_t_create(e2, col_empty));
				gs_dyn_array_push(*vertices, vert_t_create(end2, white));
				gs_dyn_array_push(*vertices, vert_t_create(start2, white));
			}
		}

		// Push back verts
		gs_dyn_array_push(*vertices, vert_t_create(start1, white));
		gs_dyn_array_push(*vertices, vert_t_create(start2, white));
		gs_dyn_array_push(*vertices, vert_t_create(end1, white));

		gs_dyn_array_push(*vertices, vert_t_create(end1, white));
		gs_dyn_array_push(*vertices, vert_t_create(start2, white));
		gs_dyn_array_push(*vertices, vert_t_create(end2, white));

		start1 = next_start1;
		start2 = next_start2;
	}

	gs_dyn_array_free(segments);
}

void poly_line_create_joint( gs_dyn_array(vert_t)* vertices,
	                                  poly_segment_t seg1, poly_segment_t seg2,
	                                  joint_style_t joint_style, gs_vec2* end1, gs_vec2* end2,
	                                  gs_vec2* next_start1, gs_vec2* next_start2,
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

		gs_vec2 inner_sec = inner_sec_opt.intersected 
		                ? inner_sec_opt.point
		                // for parallel lines, simply connect them directly
		                : inner1->b;

		// if there's no inner intersection, flip
		// the next start position for near-180° turns
		gs_vec2 inner_start;
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
			gs_dyn_array_push(*vertices, vert_t_create(outer1->b, white));
			gs_dyn_array_push(*vertices, vert_t_create(outer2->a, white));
			gs_dyn_array_push(*vertices, vert_t_create(inner_sec, white));

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

void poly_line_create_triangle_fan( gs_dyn_array(vert_t)* vertices, gs_vec2 connect_to, gs_vec2 origin,
                                        gs_vec2 start, gs_vec2 end, b32 clockwise ) 
{
	gs_vec2 point1 = gs_vec2_sub(start, origin);
	gs_vec2 point2 = gs_vec2_sub(end, origin);

	// calculate the angle between the two points
	f32 angle1 = atan2(point1.y, point1.x);
	f32 angle2 = atan2(point2.y, point2.x);

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

	gs_vec2 start_point = start;
	gs_vec2 end_point;
	for (s32 t = 0; t < num_triangles; t++) 
	{
		if (t + 1 == num_triangles) {
			// it's the last triangle - ensure it perfectly
			// connects to the next line
			end_point = end;
		} else {
			f32 rot = (t + 1) * tri_angle;

			// rotate the original point around the origin
			end_point.x = cos(rot) * point1.x - sin(rot) * point1.y;
			end_point.y = sin(rot) * point1.x + cos(rot) * point1.y;

			// re-add the rotation origin to the target point
			end_point = gs_vec2_add(end_point, origin);
		}

		// emit the triangle
		gs_dyn_array_push( *vertices, vert_t_create(start_point, white));
		gs_dyn_array_push( *vertices, vert_t_create(end_point, white));
		gs_dyn_array_push( *vertices, vert_t_create(connect_to, white));

		start_point = end_point;
	}

	return vertices;
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
// " float miter = sign(a_normal.x) * sign(a_normal.y);\n"
// " vec2 pp = a_position.xy + vec2(normalize(a_normal.xy) * a_normal.z);\n"
// " gl_Position = u_proj * u_view * vec4(pp, 0.0, 1.0);\n"
// " gl_PointSize = 1.0;\n"
// " fs_out.color = a_color;\n"
// " fs_out.normal = a_normal.xy;\n"
// " fs_out.width = a_normal.z;\n"
// " fs_out.edge = sign(miter);\n"
// " fs_out.uv = a_uv;\n"
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
// "	vec2 uv = fs_in.uv * 2.0 - 1.0;\n"
// "	float v1 = abs(uv.y);\n"
// "	float v0 = abs(uv.x);\n"
// "	float inv_width = 1.0 / (fs_in.width + 0.0001);\n"
// "	float inner = 0.5;\n"
// "	float omi = 1.0 - (1.0 / log(fs_in.width * 10.0 + 0.0001));\n"
// "	float omiss = smoothstep(0.5, 1.0, omi);\n"
// "	float minx = mix(0.2, 0.99, omiss);\n"
// "	float maxx = mix(0.4, 0.99, omiss);\n"
// "	float in_x = mix(0.5, 0.99, omiss);\n"
// "	float in_y = mix(0.4, 0.99, omiss);\n"
// "	float v_x = smoothstep(minx, maxx, v0 * in_x);\n"
// "	float v_y = smoothstep(0.99, 0.99, v1 * 0.99);\n"
// "	vec4 col_x = mix(vec4(fs_in.color, 1.0), vec4(0.0), v_x);\n"
// "	vec4 col_y = mix(vec4(fs_in.color, 1.0), vec4(0.0), v_y);\n"
// "	frag_color = mix(col_x, col_y, v_y);\n"
	"	frag_color = fs_in.color;\n"
// "	frag_color = vec4(uv, 1.0, 1.0);\n"
"}";

typedef struct point_t
{
	gs_vec2 position;
	gs_vec3 color;
} point_t;

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

gs_result app_update()
{
	// Grab global instance of engine
	gs_engine* engine = gs_engine_instance();

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
	f32 r = 300.f;

	x = cos(gs_deg_to_rad(gs_interp_smooth_step( 0.f, 360.f, sin(t * 0.3) * 0.5f + 0.5f))) * r;
	y = sin(gs_deg_to_rad(gs_interp_smooth_step( 0.f, 360.f, sin(t * 0.3) * 0.5f + 0.5f))) * r;

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
	xform.scale = (gs_vec3){scl, scl, scl};
	// xform.scale = gs_quat_angle_axis(t * 0.1f, (gs_vec3){0.f, 0.f, 1.f});
	// draw_square_xform( (gs_vec2){100.f, 100.f}, (gs_vec2){50.f, 50.f}, xform, white, width );

	// Pass in a descriptor to close the path for certain paths, like a square

	gs_vec2 points[] = {
		sp, ep
	};
	u32 pt_count = sizeof(points) / sizeof(gs_vec2);

	static f32 thickness = 1.f;
	if (platform->key_down(gs_keycode_e)) {
		thickness += 1.f;
	}
	if (platform->key_down(gs_keycode_q)) {
		thickness -= 1.f;
	}
	thickness = gs_clamp(thickness, 1.f, 1000.f);

	// Pass in verts for creating poly line
	poly_line_create( &verts, points, pt_count, thickness,
                             joint_style_miter,
                             end_cap_style_square,
                             true);

	// Want to create "feather" data for anti-aliasing

	// Iterate through all v_data, then format it into vertex data for uvs, or normals instead?

	// Each line will be a quad, so need to update the vertex buffer with data somehow.
	// Need that exposed to the user...
	gfx->update_vertex_buffer_data( g_vb, verts, gs_dyn_array_size(verts) * sizeof(vert_t) );

	// Set clear color and clear screen
	f32 clear_color[4] = { 0.1f, 0.1f, 0.1f, 1.f };
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

/*
	// Need a method for adding lines to a vertex buffer

*/

gs_result app_shutdown()
{
	// Free all da things
	gs_dyn_array_free( verts );

	gs_println( "Goodbye, Gunslinger." );
	return gs_result_success;
}

void debug_draw()
{
	// Grab global instance of engine
	gs_engine* engine = gs_engine_instance();

	// Api instances
	gs_graphics_i* gfx = engine->ctx.graphics;
	gs_platform_i* platform = engine->ctx.platform;

	// const f32 t = platform->elapsed_time() / 1000.f;
	const f32 dt = platform->time.delta;
	static f32 t = 0.f;
	t += 0.1f;

	// Construct orthographic camera for rendering debug drawing
	gs_camera ortho_cam = {0};
	ortho_cam.transform = gs_vqs_default();
	ortho_cam.fov = 60.f;
	ortho_cam.aspect_ratio = 800.f / 600.f;
	ortho_cam.near_plane = 0.01f;
	ortho_cam.far_plane = 1000.0f;
	ortho_cam.ortho_scale = 1.f;
	ortho_cam.proj_type = gs_projection_type_orthographic;

	// Set debug drawing properties
	gs_debug_draw_properties debug_props = (gs_debug_draw_properties) 
	{
		gs_camera_get_view( &ortho_cam ),
		gs_camera_get_projection( &ortho_cam, 800, 600 )
	};
	gfx->set_debug_draw_properties( g_cb, debug_props );

	// gfx->draw_line( g_cb, (gs_vec3){30.f, 30.f, 0.f}, (gs_vec3){30.f, 200.f, 0.f }, white );
	// gfx->draw_line( g_cb, (gs_vec3){40.f, 30.f, 0.f}, (gs_vec3){40.f, 200.f, 0.f }, red );
	// gfx->draw_line( g_cb, (gs_vec3){50.f, 30.f, 0.f}, (gs_vec3){50.f, 200.f, 0.f }, blue );
	// gfx->draw_line( g_cb, (gs_vec3){60.f, 30.f, 0.f}, (gs_vec3){60.f, 200.f, 0.f }, green );

	// Try and rotate square?
	gs_vqs xform = gs_vqs_default();
	xform.rotation = gs_quat_angle_axis( gs_deg_to_rad(t * 10.f), (gs_vec3){0.0f, 0.0f, 1.f} );

	// Draw debug square
	// gfx->draw_square( g_cb, (gs_vec3){100.f, 100.f, 0.f}, 100.f, 100.f, green, gs_vqs_to_mat4(&xform) );

	// Flush debug rendering buffer
	gfx->submit_debug_drawing( g_cb );

}







