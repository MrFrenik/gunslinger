#ifndef __GS_MATH_H__
#define __GS_MATH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include "common/gs_types.h"

// Defines
#define gs_pi 		3.1415926535897932
#define gs_tau		2.0 * gs_pi

/*================================================================================
// Useful Common Functions
================================================================================*/

#define gs_rad_to_deg( rad )\
	( f32 )( ( rad * 180.0 ) / gs_pi ) 

#define gs_deg_to_rad( deg )\
	( f32 )( ( deg * gs_pi ) / 180.0 )

// Interpolation
// Source: https://codeplea.com/simple-interpolation

_inline f32
gs_interp_linear( f32 a, f32 b, f32 t )
{
	return ( a + t * ( b - a ) );
}

_inline f32
gs_interp_smooth_step( f32 a, f32 b, f32 t )
{
	return gs_interp_linear( a, b, t * t * ( 3.0 - 2.0 * t ) );
}

_inline f32 
gs_interp_cosine( f32 a, f32 b, f32 t )
{
	return gs_interp_linear( a, b, -cos( gs_pi * t ) * 0.5 + 0.5 );
}

_inline f32 
gs_interp_acceleration( f32 a, f32 b, f32 t ) 
{
	return gs_interp_linear( a, b, t * t );
}

_inline f32 
gs_interp_deceleration( f32 a, f32 b, f32 t ) 
{
	return gs_interp_linear( a, b, 1.0 - ( 1.0 - t ) * ( 1.0 - t ) );
}

_inline f32 
gs_round( f32 val ) 
{
	return floor( val + 0.5f );
}

_inline f32
gs_map_range( f32 input_start, f32 input_end, f32 output_start, f32 output_end, f32 val )
{
	f32 slope = ( output_end - output_start ) / ( input_end - input_start );
	return ( output_start + ( slope * ( val - input_start ) ) );
}

/*================================================================================
// Vec2
================================================================================*/

typedef struct 
{
	f32 x;
	f32 y;
} gs_vec2;

_inline gs_vec2 
gs_vec2_ctor( f32 _x, f32 _y ) 
{
	gs_vec2 v = { _x, _y };
	return v;
}

_inline gs_vec2 
gs_vec2_add( gs_vec2 v0, gs_vec2 v1 ) 
{
	return gs_vec2_ctor( v0.x + v1.x, v0.y + v1.y );
}

_inline gs_vec2 
gs_vec2_sub( gs_vec2 v0, gs_vec2 v1 )
{
	return gs_vec2_ctor( v0.x - v1.x, v0.y - v1.y );
}

_inline gs_vec2 
gs_vec2_mul( gs_vec2 v0, gs_vec2 v1 ) 
{
	return gs_vec2_ctor( v0.x * v1.x, v0.y * v1.y );
}

_inline gs_vec2 
gs_vec2_div( gs_vec2 v0, gs_vec2 v1 ) 
{
	return gs_vec2_ctor( v0.x / v1.x, v0.y / v1.y );
}

_inline gs_vec2 
gs_vec2_scale( gs_vec2 v, f32 s )
{
	return gs_vec2_ctor( v.x * s, v.y * s );
}

_inline f32 
gs_vec2_dot( gs_vec2 v0, gs_vec2 v1 ) 
{
	return ( f32 )( v0.x * v1.x + v0.y * v1.y );
}

_inline f32 
gs_vec2_len( gs_vec2 v )
{
	return ( f32 )sqrt( gs_vec2_dot( v, v ) );
}

_inline gs_vec2 gs_vec2_norm( gs_vec2 v ) 
{
	return gs_vec2_scale( v, 1.0f / gs_vec2_len( v ) );
}

_inline 
f32 gs_vec2_dist( gs_vec2 a, gs_vec2 b )
{
	f32 dx = (a.x - b.x);
	f32 dy = (a.y - b.y);
	return ( sqrt(dx * dx + dy * dy) );
}

_inline
f32 gs_vec2_cross( gs_vec2 a, gs_vec2 b ) 
{
	return a.x * b.y - a.y * b.x;
}

_inline
f32 gs_vec2_angle( gs_vec2 a, gs_vec2 b ) 
{
	return acos( gs_vec2_dot(a, b) / ( gs_vec2_len(a) * gs_vec2_len(b) ) );
}

_inline
b32 gs_vec2_equal( gs_vec2 a, gs_vec2 b )
{
	return (a.x == b.x && a.y == b.y);
}

/*================================================================================
// Vec3
================================================================================*/

typedef struct
{
	f32 x;
	f32 y;
	f32 z;
} gs_vec3;

_inline gs_vec3 
gs_vec3_ctor( f32 _x, f32 _y, f32 _z )
{
	gs_vec3 v = { _x, _y, _z };
	return v;
}

_inline gs_vec3 
gs_vec3_add( gs_vec3 v0, gs_vec3 v1 )
{
	return gs_vec3_ctor( v0.x + v1.x, v0.y + v1.y, v0.z + v1.z );
}

_inline gs_vec3 
gs_vec3_sub( gs_vec3 v0, gs_vec3 v1 ) 
{
	return gs_vec3_ctor( v0.x - v1.x, v0.y - v1.y, v0.z - v1.z );
}

_inline gs_vec3 
gs_vec3_mul( gs_vec3 v0, gs_vec3 v1 ) 
{
	return gs_vec3_ctor( v0.x * v1.x, v0.y * v1.y, v0.z * v1.z );
}

_inline gs_vec3 
gs_vec3_div( gs_vec3 v0, gs_vec3 v1 ) 
{
	return gs_vec3_ctor( v0.x / v1.x, v0.y / v1.y, v0.z / v1.z );
}

_inline gs_vec3 
gs_vec3_scale( gs_vec3 v, f32 s ) 
{
	return gs_vec3_ctor( v.x * s, v.y * s, v.z * s );
}

_inline f32 
gs_vec3_dot( gs_vec3 v0, gs_vec3 v1 ) 
{
	f32 dot = ( f32 )( (v0.x * v1.x) + (v0.y * v1.y) + v0.z * v1.z );
	return dot;
}

_inline 
f32 gs_vec3_dist( gs_vec3 a, gs_vec3 b )
{
	f32 dx = (a.x - b.x);
	f32 dy = (a.y - b.y);
	f32 dz = (a.z - b.z);
	return ( sqrt(dx * dx + dy * dy + dz * dz) );
}

_inline f32 
gs_vec3_len( gs_vec3 v )
{
	return ( f32 )sqrt( gs_vec3_dot( v, v ) );
}

_inline gs_vec3 
gs_vec3_norm( gs_vec3 v )
{
	f32 len = gs_vec3_len( v );
	return len == 0.f ? v : gs_vec3_scale( v, 1.f / len );
}

_inline gs_vec3 
gs_vec3_cross( gs_vec3 v0, gs_vec3 v1 ) 
{
	return gs_vec3_ctor
	(
		v0.y * v1.z - v0.z * v1.y,
		v0.z * v1.x - v0.x * v1.z,
		v0.x * v1.y - v0.y * v1.x
	);
}

_inline void gs_vec3_scale_ip( gs_vec3* vp, f32 s )
{
	vp->x *= s;
	vp->y *= s;
	vp->z *= s;
}

/*================================================================================
// Vec4
================================================================================*/

typedef struct
{
	f32 x;
	f32 y;
	f32 z;
	f32 w;
} gs_vec4;

_inline gs_vec4 
gs_vec4_ctor( f32 _x, f32 _y, f32 _z, f32 _w )
{
	gs_vec4 v = { _x, _y, _z, _w };
	return v;
} 

_inline gs_vec4
gs_vec4_add( gs_vec4 v0, gs_vec4 v1 ) 
{
	return gs_vec4_ctor( v0.x + v1.y, v0.y + v1.y, v0.z + v1.z, v0.w + v1.w );
}

_inline gs_vec4
gs_vec4_sub( gs_vec4 v0, gs_vec4 v1 ) 
{
	return gs_vec4_ctor( v0.x - v1.y, v0.y - v1.y, v0.z - v1.z, v0.w - v1.w );
}

_inline gs_vec4
gs_vec4_div( gs_vec4 v0, gs_vec4 v1 ) 
{
	return gs_vec4_ctor( v0.x / v1.x, v0.y / v1.y, v0.z / v1.z, v0.w / v1.w );
}

_inline gs_vec4
gs_vec4_scale( gs_vec4 v, f32 s ) 
{
	return gs_vec4_ctor( v.x / s, v.y / s, v.z / s, v.w / s );
}

_inline f32
gs_vec4_dot( gs_vec4 v0, gs_vec4 v1 ) 
{
	return ( f32 )( v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w );
}

_inline f32
gs_vec4_len( gs_vec4 v ) 
{
	return ( f32 )sqrt( gs_vec4_dot( v, v ) );
}

_inline gs_vec4
gs_vec4_norm( gs_vec4 v ) 
{
	return gs_vec4_scale( v, 1.0f / gs_vec4_len( v ) );
}

_inline f32
gs_vec4_dist( gs_vec4 v0, gs_vec4 v1 )
{
	f32 dx = (v0.x - v1.x);
	f32 dy = (v0.y - v1.y);
	f32 dz = (v0.z - v1.z);
	f32 dw = (v0.w - v1.w);
	return ( sqrt(dx * dx + dy * dy + dz * dz + dw * dw) );
}

/*================================================================================
// Mat4x4
================================================================================*/

/*
	Matrices are stored in linear, contiguous memory and assume a column-major ordering.
*/

typedef struct gs_mat4
{
	f32 elements[16];
} gs_mat4;

_inline gs_mat4 
gs_mat4_diag( f32 val )
{
	gs_mat4 m = { 0 };
	m.elements[ 0 + 0 * 4 ] = val;
	m.elements[ 1 + 1 * 4 ] = val;
	m.elements[ 2 + 2 * 4 ] = val;
	m.elements[ 3 + 3 * 4 ] = val;
	return m;
}

#define gs_mat4_identity()\
	gs_mat4_diag( 1.0f )

#define gs_mat4_ctor()\
	( gs_mat4 ){ 0 }

_inline gs_mat4 
gs_mat4_mul( gs_mat4 m0, gs_mat4 m1 )
{
	gs_mat4 m_res = gs_mat4_ctor(); 
	for ( u32 y = 0; y < 4; ++y )
	{
		for ( u32 x = 0; x < 4; ++x )
		{
			f32 sum = 0.0f;
			for ( u32 e = 0; e < 4; ++e )
			{
				sum += m0.elements[ x + e * 4 ] * m1.elements[ e + y * 4 ];
			}
			m_res.elements[ x + y * 4 ] = sum;
		}
	}

	return m_res;
}

_inline
gs_mat4 gs_mat4_transpose( gs_mat4 m )
{
	gs_mat4 t = gs_mat4_identity();

	// First row
	t.elements[ 0 * 4 + 0 ] = m.elements[ 0 * 4 + 0 ];
	t.elements[ 1 * 4 + 0 ] = m.elements[ 0 * 4 + 1 ];
	t.elements[ 2 * 4 + 0 ] = m.elements[ 0 * 4 + 2 ];
	t.elements[ 3 * 4 + 0 ] = m.elements[ 0 * 4 + 3 ];

	// Second row
	t.elements[ 0 * 4 + 1 ] = m.elements[ 1 * 4 + 0 ];
	t.elements[ 1 * 4 + 1 ] = m.elements[ 1 * 4 + 1 ];
	t.elements[ 2 * 4 + 1 ] = m.elements[ 1 * 4 + 2 ];
	t.elements[ 3 * 4 + 1 ] = m.elements[ 1 * 4 + 3 ];

	// Third row
	t.elements[ 0 * 4 + 2 ] = m.elements[ 2 * 4 + 0 ];
	t.elements[ 1 * 4 + 2 ] = m.elements[ 2 * 4 + 1 ];
	t.elements[ 2 * 4 + 2 ] = m.elements[ 2 * 4 + 2 ];
	t.elements[ 3 * 4 + 2 ] = m.elements[ 2 * 4 + 3 ];

	// Fourth row
	t.elements[ 0 * 4 + 3 ] = m.elements[ 3 * 4 + 0 ];
	t.elements[ 1 * 4 + 3 ] = m.elements[ 3 * 4 + 1 ];
	t.elements[ 2 * 4 + 3 ] = m.elements[ 3 * 4 + 2 ];
	t.elements[ 3 * 4 + 3 ] = m.elements[ 3 * 4 + 3 ];

	return t;
}

/*
	f32 l : left
	f32 r : right
	f32 b : bottom
	f32 t : top
	f32 n : near
	f32 f : far
*/
_inline gs_mat4 
gs_mat4_ortho( f32 l, f32 r, f32 b, f32 t, f32 n, f32 f )
{
	gs_mat4 m_res = gs_mat4_identity();		

	// Main diagonal
	m_res.elements[ 0 + 0 * 4 ] = 2.0f / ( r - l );
	m_res.elements[ 1 + 1 * 4 ] = 2.0f / ( t - b );
	m_res.elements[ 2 + 2 * 4 ] = -2.0f / ( f - n );

	// Last column
	m_res.elements[ 0 + 3 * 4 ] = -( r + l ) / ( r - l );
	m_res.elements[ 1 + 3 * 4 ] = -( t + b ) / ( t - b );
	m_res.elements[ 2 + 3 * 4 ] = -( f + n ) / ( f - n );

	return m_res;
}

_inline gs_mat4 
gs_mat4_perspective( f32 fov, f32 asp_ratio, f32 n, f32 f )
{
	// Zero matrix
	gs_mat4 m_res = gs_mat4_ctor();

	f32 q = 1.0f / tan( gs_deg_to_rad( 0.5f * fov ) );
	f32 a = q / asp_ratio;
	f32 b = ( n + f ) / ( n - f );
	f32 c = ( 2.0f * n * f ) / ( n - f );

	m_res.elements[ 0 + 0 * 4 ] = a;
	m_res.elements[ 1 + 1 * 4 ] = q;
	m_res.elements[ 2 + 2 * 4 ] = b;
	m_res.elements[ 2 + 3 * 4 ] = c;
	m_res.elements[ 3 + 2 * 4 ] = -1.0f;

	return m_res;
}

_inline gs_mat4 
gs_mat4_translate( const gs_vec3 v )
{
	gs_mat4 m_res = gs_mat4_identity();

	m_res.elements[ 0 + 4 * 3 ] = v.x;
	m_res.elements[ 1 + 4 * 3 ] = v.y;
	m_res.elements[ 2 + 4 * 3 ] = v.z;

	return m_res;
}

_inline gs_mat4 
gs_mat4_scale( const gs_vec3 v )
{
	gs_mat4 m_res = gs_mat4_identity();
	m_res.elements[ 0 + 0 * 4 ] = v.x;
	m_res.elements[ 1 + 1 * 4 ] = v.y;
	m_res.elements[ 2 + 2 * 4 ] = v.z;
	return m_res;
}

_inline gs_mat4 gs_mat4_rotate( f32 angle, gs_vec3 axis )
{
	gs_mat4 m_res = gs_mat4_identity();

	f32 a = gs_deg_to_rad( angle );
	f32 c = cos( a );
	f32 s = sin( a );

	f32 x = axis.x;
	f32 y = axis.y;
	f32 z = axis.z;

	//First column
	m_res.elements[ 0 + 0 * 4 ] = x * x * ( 1 - c ) + c;	
	m_res.elements[ 1 + 0 * 4 ] = x * y * ( 1 - c ) + z * s;	
	m_res.elements[ 2 + 0 * 4 ] = x * z * ( 1 - c ) - y * s;	
	
	//Second column
	m_res.elements[ 0 + 1 * 4 ] = x * y * ( 1 - c ) - z * s;	
	m_res.elements[ 1 + 1 * 4 ] = y * y * ( 1 - c ) + c;	
	m_res.elements[ 2 + 1 * 4 ] = y * z * ( 1 - c ) + x * s;	
	
	//Third column
	m_res.elements[ 0 + 2 * 4 ] = x * z * ( 1 - c ) + y * s;	
	m_res.elements[ 1 + 2 * 4 ] = y * z * ( 1 - c ) - x * s;	
	m_res.elements[ 2 + 2 * 4 ] = z * z * ( 1 - c ) + c;	

	return m_res;
}

_inline gs_mat4 
gs_mat4_look_at( gs_vec3 position, gs_vec3 target, gs_vec3 up )
{
	gs_vec3 f = gs_vec3_norm( gs_vec3_sub( target, position ) );
	gs_vec3 s = gs_vec3_norm( gs_vec3_cross( f, up ) );
	gs_vec3 u = gs_vec3_cross( s, f );

	gs_mat4 m_res = gs_mat4_identity();
	m_res.elements[ 0 * 4 + 0 ] = s.x;
	m_res.elements[ 1 * 4 + 0 ] = s.y;
	m_res.elements[ 2 * 4 + 0 ] = s.z;

	m_res.elements[ 0 * 4 + 1 ] = u.x;
	m_res.elements[ 1 * 4 + 1 ] = u.y;
	m_res.elements[ 2 * 4 + 1 ] = u.z;

	m_res.elements[ 0 * 4 + 2 ] = -f.x;
	m_res.elements[ 1 * 4 + 2 ] = -f.y;
	m_res.elements[ 2 * 4 + 2 ] = -f.z;

	m_res.elements[ 3 * 4 + 0 ] = -gs_vec3_dot( s, position );;
	m_res.elements[ 3 * 4 + 1 ] = -gs_vec3_dot( u, position );
	m_res.elements[ 3 * 4 + 2 ] = gs_vec3_dot( f, position ); 

	return m_res;
}

_inline
gs_vec3 gs_mat4_mul_vec3( gs_mat4 m, gs_vec3 v )
{
	m = gs_mat4_transpose(m);
	return (gs_vec3)
	{
		m.elements[0 * 4 + 0] * v.x + m.elements[0 * 4 + 1] * v.y + m.elements[0 * 4 + 2] * v.z,  
		m.elements[1 * 4 + 0] * v.x + m.elements[1 * 4 + 1] * v.y + m.elements[1 * 4 + 2] * v.z,  
		m.elements[2 * 4 + 0] * v.x + m.elements[2 * 4 + 1] * v.y + m.elements[2 * 4 + 2] * v.z,  
	};
}
	
_inline
gs_vec4 gs_mat4_mul_vec4( gs_mat4 m, gs_vec4 v )
{
	m = gs_mat4_transpose(m);
	return (gs_vec4)
	{
		m.elements[0 * 4 + 0] * v.x + m.elements[0 * 4 + 1] * v.y + m.elements[0 * 4 + 2] * v.z + m.elements[0 * 4 + 3] * v.w,  
		m.elements[1 * 4 + 0] * v.x + m.elements[1 * 4 + 1] * v.y + m.elements[1 * 4 + 2] * v.z + m.elements[1 * 4 + 3] * v.w,  
		m.elements[2 * 4 + 0] * v.x + m.elements[2 * 4 + 1] * v.y + m.elements[2 * 4 + 2] * v.z + m.elements[2 * 4 + 3] * v.w,  
		m.elements[3 * 4 + 0] * v.x + m.elements[3 * 4 + 1] * v.y + m.elements[3 * 4 + 2] * v.z + m.elements[3 * 4 + 3] * v.w
	};
}

/*================================================================================
// Quaternion
================================================================================*/

typedef struct
{
	f32 x;
	f32 y;
	f32 z;
	f32 w;
} gs_quat;

#define gs_quat_default()\
	( ( gs_quat ){ .x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 1.0f } )

#define gs_quat_ctor( _x, _y, _z, _w )\
	( ( gs_quat ){ .x = ( _x ), .y = ( _y ), .z = ( _z ), .w = ( _w ) } )

_inline gs_quat 
__gs_quat_add_impl( const gs_quat* q0, const gs_quat* q1 ) 
{
	return gs_quat_ctor( q0->x + q1->x, q0->y + q1->y, q0->z + q1->z, q0->w + q1->w );
}

#define gs_quat_add( q0, q1 )\
	( __gs_quat_add_impl( &( q0 ), &( q1 ) ) )

_inline gs_quat 
__gs_quat_sub_impl( const gs_quat* q0, const gs_quat* q1 ) 
{
	return gs_quat_ctor( q0->x - q1->x, q0->y - q1->y, q0->z - q1->z, q0->w - q1->w );
}

#define gs_quat_sub( q0, q1 )\
	( __gs_quat_sub_impl( &( q0 ), &( q1 ) ) )

_inline gs_quat
__gs_quat_mul_impl( gs_quat q0, gs_quat q1 )
{
	return gs_quat_ctor(
		q0.w * q1.x + q1.w * q0.x + q0.y * q1.z - q1.y * q0.z,
		q0.w * q1.y + q1.w * q0.y + q0.z * q1.x - q1.z * q0.x,
		q0.w * q1.z + q1.w * q0.z + q0.x * q1.y - q1.x * q0.y,
		q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z
	);
}

_inline
gs_quat gs_quat_mul_quat( gs_quat q0, gs_quat q1 )
{
	return gs_quat_ctor(
		q0.w * q1.x + q1.w * q0.x + q0.y * q1.z - q1.y * q0.z,
		q0.w * q1.y + q1.w * q0.y + q0.z * q1.x - q1.z * q0.x,
		q0.w * q1.z + q1.w * q0.z + q0.x * q1.y - q1.x * q0.y,
		q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z
	);
}

#define gs_quat_mul( q0, q1 )\
	__gs_quat_mul_impl( ( q0 ), ( q1 ) )

_inline gs_quat
__gs_quat_scale_impl( const gs_quat* q, f32 s )
{
	return gs_quat_ctor( q->x * s, q->y * s, q->z * s, q->w * s );
}

#define gs_quat_scale( q, s )\
	( __gs_quat_scale_impl( &( q ), s ) )

_inline f32 
__gs_quat_dot_impl( const gs_quat* q0, const gs_quat* q1 )
{
	return ( f32 )( q0->x * q1->x + q0->y * q1->y + q0->z * q1->z + q0->w * q1->w );
}

#define gs_quat_dot( q0, q1 )\
	( __gs_quat_dot_impl( &( q0 ), &( q1 ) ) )

#define gs_quat_conjugate( q )\
	( gs_quat_ctor( -q.x, -q.y, -q.z, q.w ) )

_inline f32
_gs_quat_len_impl( const gs_quat* q )
{
	return ( f32 )sqrt( gs_quat_dot( *q, *q ) );
}

#define gs_quat_len( q )\
	( _gs_quat_len_impl( &( q ) ) )

_inline gs_quat
gs_quat_norm( gs_quat q ) 
{
	return gs_quat_scale( q, 1.0f / gs_quat_len( q ) );
}

_inline gs_quat
__gs_quat_cross_impl( gs_quat q0, gs_quat q1 )
{
	return gs_quat_ctor (											
		q0.x * q1.x + q0.x * q1.w + q0.y * q1.z - q0.z * q1.y,	
		q0.w * q1.y + q0.y * q1.w + q0.z * q1.x - q0.x * q1.z,	
		q0.w * q1.z + q0.z * q1.w + q0.x * q1.y - q0.y * q1.x,	
		q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z 	
	);
}

#define gs_quat_cross( q0, q1 )\
	( __gs_quat_cross_impl( q0, q1 ) )

// Inverse := Conjugate / Dot;
#define gs_quat_inverse( q )\
	( gs_quat_scale( gs_quat_conjugate( q ), 1.0f / gs_quat_dot( q, q ) ) )

_inline gs_vec3 gs_quat_rotate( gs_quat q, gs_vec3 v )
{
	// nVidia SDK implementation
	gs_vec3 qvec = { q.x, q.y, q.z };
	gs_vec3 uv = gs_vec3_cross( qvec, v );
	gs_vec3 uuv = gs_vec3_cross( qvec, uv );
	gs_vec3_scale_ip( &uv, 2.0f * q.w );
	gs_vec3_scale_ip( &uuv, 2.0f );
	return ( gs_vec3_add( v, gs_vec3_add( uv, uuv ) ) );
}

_inline gs_quat gs_quat_angle_axis( f32 rad, gs_vec3 axis )
{
	// Normalize axis
	gs_vec3 a = gs_vec3_norm( axis );

	// Get scalar
	f32 half_angle = 0.5f * rad;
	f32 s = sin( half_angle );

	return gs_quat_ctor( a.x * s, a.y * s, a.z * s, cos( half_angle ) );
}

_inline
gs_quat gs_quat_slerp( gs_quat a, gs_quat b, f32 t )
{
	f32 c = gs_quat_dot(a, b);
	gs_quat end = b;

	if ( c < 0.0f )
	{
		// Reverse all signs
		c *= -1.0f;
		end.x *= -1.0f;
		end.y *= -1.0f;
		end.z *= -1.0f;
		end.w *= -1.0f;
	}

	// Calculate coefficients
	f32 sclp, sclq;
	if ( ( 1.0f - c ) > 0.0001f )
	{
		f32 omega = acosf( c );
		f32 s = sinf( omega );
		sclp = sinf( ( 1.0f - t ) * omega ) / s;
		sclq = sinf( t * omega ) / s; 
	}
	else
	{
		sclp = 1.0f - t;
		sclq = t;
	}

	gs_quat q;
	q.x = sclp * a.x + sclq * end.x;
	q.y = sclp * a.y + sclq * end.y;
	q.z = sclp * a.z + sclq * end.z;
	q.w = sclp * a.w + sclq * end.w;

	return q;
}

#define quat_axis_angle( axis, angle )\
	gs_quat_angle_axis( angle, axis )

/*
* @brief Convert given quaternion param into equivalent 4x4 rotation matrix
* @note: From http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm 
*/
_inline gs_mat4 gs_quat_to_mat4( gs_quat _q )
{
	gs_mat4 mat = gs_mat4_identity();
	gs_quat q = gs_quat_norm( _q );

	f32 xx = q.x * q.x;	
	f32 yy = q.y * q.y;	
	f32 zz = q.z * q.z;	
	f32 xy = q.x * q.y;
	f32 xz = q.x * q.z;
	f32 yz = q.y * q.z;
	f32 wx = q.w * q.x;
	f32 wy = q.w * q.y;
	f32 wz = q.w * q.z;

	mat.elements[ 0 * 4 + 0 ] = 1.0f - 2.0f * ( yy + zz );
	mat.elements[ 1 * 4 + 0 ] = 2.0f * ( xy - wz );
	mat.elements[ 2 * 4 + 0 ] = 2.0f * ( xz + wy );

	mat.elements[ 0 * 4 + 1 ] = 2.0f * ( xy + wz );
	mat.elements[ 1 * 4 + 1 ] = 1.0f - 2.0f * ( xx + zz );
	mat.elements[ 2 * 4 + 1 ] = 2.0f * ( yz - wx );

	mat.elements[ 0 * 4 + 2 ] = 2.0f * ( xz - wy );
	mat.elements[ 1 * 4 + 2 ] = 2.0f * ( yz + wx );
	mat.elements[ 2 * 4 + 2 ] = 1.0f - 2.0f * ( xx + yy );

	return mat;
}

/*================================================================================
// Transform ( Non-Uniform Scalar VQS )
================================================================================*/

/*
	- This follows a traditional 'VQS' structure for complex object transformations, 
		however it differs from the standard in that it allows for non-uniform 
		scaling in the form of a vec3.
*/
// Source: https://www.eurosis.org/cms/files/conf/gameon-asia/gameon-asia2007/R-SESSION/G1.pdf

typedef struct 
{
	gs_vec3 	position;
	gs_quat 	rotation;
	gs_vec3 	scale;		
} gs_vqs;

_inline gs_vqs gs_vqs_default()
{
	gs_vqs t = ( gs_vqs )
	{
		(gs_vec3){ 0.0f, 0.0f, 0.0f },
		 (gs_quat){ 0.0f, 0.0f, 0.0f, 1.0f },
		(gs_vec3){ 1.0f, 1.0f, 1.0f }
	};
	return t;
}

_inline gs_vqs gs_vqs_ctor( gs_vec3 tns, gs_quat rot, gs_vec3 scl )
{
	gs_vqs t;	
	t.position = tns;
	t.rotation = rot;
	t.scale = scl;
	return t;
}

// AbsScale	= ParentScale * LocalScale
// AbsRot	= LocalRot * ParentRot
// AbsTrans	= ParentPos + [ ParentRot * ( ParentScale * LocalPos ) ]
_inline gs_vqs gs_vqs_absolute_transform( const gs_vqs* local, const gs_vqs* parent )
{
	// Normalized rotations
	gs_quat p_rot_norm = gs_quat_norm( parent->rotation );
	gs_quat l_rot_norm = gs_quat_norm( local->rotation );

	// Scale
	gs_vec3 scl = gs_vec3_mul( local->scale, parent->scale );
	// Rotation
	gs_quat rot = gs_quat_norm( gs_quat_mul( p_rot_norm, l_rot_norm ) );
	// position
	gs_vec3 tns = gs_vec3_add( parent->position, gs_quat_rotate( p_rot_norm, gs_vec3_mul( parent->scale, local->position ) ) );

	return gs_vqs_ctor( tns, rot, scl );
}

// RelScale = AbsScale / ParentScale 
// RelRot	= Inverse(ParentRot) * AbsRot
// RelTrans	= [Inverse(ParentRot) * (AbsPos - ParentPosition)] / ParentScale;
_inline gs_vqs gs_vqs_relative_transform( const gs_vqs* absolute, const gs_vqs* parent )
{
	// Get inverse rotation normalized
	gs_quat p_rot_inv = gs_quat_norm( gs_quat_inverse( parent->rotation ) );
	// Normalized abs rotation
	gs_quat a_rot_norm = gs_quat_norm( absolute->rotation );

	// Scale
	gs_vec3 scl = gs_vec3_div( absolute->scale, parent->scale );
	// Rotation
	gs_quat rot = gs_quat_norm( gs_quat_mul( p_rot_inv, a_rot_norm ) );
	// position
	gs_vec3 tns = gs_vec3_div( gs_quat_rotate( p_rot_inv, gs_vec3_sub( absolute->position, parent->position ) ), parent->scale );

	return gs_vqs_ctor( tns, rot, scl );
}

_inline gs_mat4 gs_vqs_to_mat4( const gs_vqs* transform )
{
	gs_mat4 mat = gs_mat4_identity();
	gs_mat4 trans = gs_mat4_translate( transform->position );
	gs_mat4 rot = gs_quat_to_mat4( transform->rotation );
	gs_mat4 scl = gs_mat4_scale( transform->scale );
	mat = gs_mat4_mul( mat, trans );
	mat = gs_mat4_mul( mat, rot );
	mat = gs_mat4_mul( mat, scl );
	return mat;
}

/*================================================================================
// Ray
================================================================================*/

typedef struct 
{
	gs_vec3 point;
	gs_vec3 direction;	
} gs_ray;

_inline gs_ray gs_ray_ctor( gs_vec3 pt, gs_vec3 dir )
{
	gs_ray r = { pt, dir };
	return r;
}

/*================================================================================
// Plane
================================================================================*/

typedef struct 
{
	f32 a;
	f32 b;
	f32 c;
	f32 d;
} gs_plane;


#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_MATH_H__



















