#ifndef __GS_UTIL_H__
#define __GS_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gs_types.h"

#include <stdarg.h>
#include <stdio.h>

// Helper macro for compiling to nothing
#define gs_empty_instruction( ... )

#define gs_array_size( arr ) sizeof( arr ) / sizeof( arr[ 0 ] )

#define gs_assert( x ) if ( !( x ) ) {\
	gs_printf( "assertion failed: ( %s ), function %s, file %s, line %d.\n", #x, __func__, __FILE__, __LINE__ );\
	abort();\
}

// Helper macro for an in place for-range loop
#define gs_for_range_i( count )\
	for ( u32 i = 0; i < count; ++i )

// Helper macro for an in place for-range loop
#define gs_for_range_j( count )\
	for ( u32 j = 0; j < count; ++j )

#define gs_for_range( iter_type, iter_name, iter_end )\
	for( iter_type iter_name = 0; iter_name < iter_end; ++iter_name )

#define gs_max( a, b ) ( (a) > (b) ? (a) : (b) )

#define gs_min( a, b ) ( (a) < (b) ? (a) : (b) )

#define gs_clamp( v, min, max ) ( (v) > (max) ? (max) : (v) < (min) ? (min) : (v) )

// Helpful macro for casting one type to another
#define gs_cast( a, b ) ( ( a* )(b) )

// Helpful marco for calculating offset ( in bytes ) of an element from a given structure type
#define gs_offset( type, element ) ( ( usize )( &( ( ( type* )( 0 ) )->element ) ) )

// macro for turning any given type into a const char* of itself
#define gs_to_str( type ) ((const char*)#type)

#define gs_print_enabled	0

#if gs_print_enabled
	#define gs_measure_time( label, ... )\
		do {\
			u32 __st = gs_platform_ticks();\
			__VA_ARGS__\
			gs_println( "%s: %d", label, gs_platform_ticks() - __st );\
		} while ( 0 )
#else
	#define gs_measure_time( label, ... )\
		__VA_ARGS__
#endif

#define gs_timed_action( interval, ... )\
	do {\
		static u32 __t = 0;\
		if ( __t++ > interval ) {\
			__t = 0;\
			__VA_ARGS__\
		}\
	} while ( 0 )		

_force_inline void* 
_gs_malloc_init_impl( usize sz )
{
	void* data = malloc( sz );
	memset( data, 0, sz );
	return data;
}

// Helper functions for heap allocations
#define gs_malloc( sz ) 			malloc( sz )
#define gs_free( mem )				free( mem )
#define gs_realloc( mem, sz )		realloc( mem, sz )
#define gs_malloc_init( type )	( type* )_gs_malloc_init_impl( sizeof( type ) )

/*===================================
// String Utils
===================================*/

_force_inline u32 
gs_string_length( const char* txt )
{
	u32 sz = 0;
	while ( txt != NULL && txt[ sz ] != '\0' ) 
	{
		sz++;
	}
	return sz;
}

// Expects null terminated strings
_force_inline b8 
gs_string_compare_equal
( 
	const char* 	txt, 
	const char* 	cmp 
)
{
	// Grab sizes of both strings
	u32 a_sz = gs_string_length( txt );
	u32 b_sz = gs_string_length( cmp );

	// Return false if sizes do not match
	if ( a_sz != b_sz ) 
	{
		return false;
	}

	for( u32 i = 0; i < a_sz; ++i ) 
	{
		if ( *txt++ != *cmp++ )
		{
			return false;
		}
	};

	return true;
}

_force_inline b8 
gs_string_compare_equal_n
( 
	const char* txt, 
	const char* cmp, 
	u32 n 
)
{
	u32 a_sz = gs_string_length( txt );
	u32 b_sz = gs_string_length( cmp );

	// Not enough characters to do operation
	if ( a_sz < n || b_sz < n )
	{
		return false;
	}

	for( u32 i = 0; i < n; ++i ) 
	{
		if ( *txt++ != *cmp++ )
		{
			return false;
		}
	};

	return true;
}

_force_inline b32
gs_util_str_is_numeric( const char* str )
{
	const char* at = str;
	while ( at && *at )
	{
		while ( *at == '\n' || *at == '\t' || *at == ' ' || *at == '\r' ) at++;;
		char c = *at++;
		printf( "\n%c", c );
		if ( c >= '0' && c <= '9' )
		{
			printf( "not numeric\n" );
			return false;
		} 
	}	

	return true;
}

// Will return a null buffer if file does not exist or allocation fails
_force_inline char* 
gs_read_file_contents_into_string_null_term
( 
	const char* file_path, 
	const char* mode,
	usize* sz
)
{
	char* buffer = 0;
	FILE* fp = fopen( file_path, mode );
	if ( fp )
	{
		fseek( fp, 0, SEEK_END );
		*sz = ftell( fp );
		fseek( fp, 0, SEEK_SET );
		buffer = ( char* )gs_malloc( *sz + 1 );
		if ( buffer )
		{
			fread( buffer, 1, *sz, fp );
		}
		fclose( fp );
		buffer[ *sz ] = '0';
	}
	return buffer;
}

_force_inline b32 
gs_util_file_exists( const char* file_path )
{
	FILE* fp = fopen( file_path, "r" );	
	if ( fp )
	{
		fclose( fp );
		return true;
	}
	return false;
}

_force_inline void 
gs_util_get_file_extension
( 
	char* buffer,
	u32 buffer_size,
	const char* file_path 
)
{
	u32 str_len = gs_string_length( file_path );
	const char* at = ( file_path + str_len - 1 );
	while ( *at != '.' && at != file_path )
	{
		at--;
	}

	if ( *at == '.' )
	{
		at++;
		u32 i = 0; 
		while ( *at )
		{
			char c = *at;
			buffer[ i++ ] = *at++;
		}
		buffer[ i ] = '\0';
	}
}

_force_inline void
gs_util_get_dir_from_file
(
	char* buffer, 
	u32 buffer_size,
	const char* file_path 
)
{
	u32 str_len = gs_string_length( file_path );
	const char* end = ( file_path + str_len );
	while ( *end != '/' && end != file_path )
	{
		end--;
	}
	memcpy( buffer, file_path, gs_min( buffer_size, ( end - file_path ) + 1 ) );
}

_force_inline void
gs_util_get_file_name
( 
	char* buffer, 
	u32 buffer_size,
	const char* file_path 
)
{
	u32 str_len = gs_string_length( file_path );
	const char* end = ( file_path + str_len );
	const char* dot_at = end;
	while ( *end != '.' && end != file_path )
	{
		end--;
	}
	const char* start = end; 
	while ( *start != '/' && start != file_path )
	{
		start--;
	}
	memcpy( buffer, start, ( end - start ) );
}

_force_inline void
gs_util_string_replace
( 
	const char* source_str, 
	char* buffer, 
	u32 buffer_size, 
	char delimiter,
	char replace 
)
{
	u32 str_len = gs_string_length( source_str );
	const char* at = source_str;
	while ( at && *at != '\0' )
	{
		char c = *at; 
		if ( c == delimiter ) {
			c = replace;
		}
		buffer[ ( at - source_str ) ] = c;
		at++;
	}
}

_force_inline void 
gs_util_normalize_path
( 
	const char* path, 
	char* buffer, 
	u32 buffer_size 
)
{
	// Normalize the path somehow...
}

_force_inline void 
gs_printf
( 
	const char* fmt,
	... 
)
{
	va_list args;
	va_start ( args, fmt );
	vprintf( fmt, args );
	va_end( args );
}

_force_inline void 
gs_println
( 
	const char* fmt, 
	... 
)
{ 
	va_list args;
	va_start( args, fmt );
	vprintf( fmt, args );
	va_end( args );
	gs_printf( "\n" );
}

_force_inline void 
gs_fprintf
( 
	FILE* fp, 
	const char* fmt, 
	... 
)
{
	va_list args;
	va_start ( args, fmt );
	vfprintf( fp, fmt, args );
	va_end( args );
}

_force_inline void 
gs_fprintln
( 
	FILE* fp, 
	const char* fmt, 
	... 
)
{
	va_list args;
	va_start( args, fmt );
	vfprintf( fp, fmt, args );
	va_end( args );
	gs_fprintf( fp, "\n" );
}

// _force_inline char* 
// gs_sprintf
// ( 
// 	const char* fmt, 
// 	... 
// )
// {
// 	va_list args;
// 	va_start( args, fmt );
// 	usize sz = vsnprintf( NULL, 0, fmt, args );	
// 	char* ret = malloc( sz + 1 );
// 	vsnprintf( ret, sz, fmt, args );
// 	va_end( args );
// 	return ret;
// }

_force_inline void 
gs_snprintf
( 
	char* buffer, 
	usize buffer_size, 
	const char* fmt, 
	... 
)
{
	va_list args;
    va_start( args, fmt );
    vsnprintf( buffer, buffer_size, fmt, args );
    va_end( args );
}

_force_inline u32 
gs_hash_u32( u32 x ) 
{
    x = ( ( x >> 16 ) ^ x ) * 0x45d9f3b;
    x = ( ( x >> 16) ^ x ) * 0x45d9f3b;
    x = ( x >> 16 ) ^ x;
    return x;
}

#define gs_hash_u32_ip( x, out )\
	do {\
	    out = ( ( x >> 16 ) ^ x ) * 0x45d9f3b;\
	    out = ( ( out >> 16) ^ out ) * 0x45d9f3b;\
	    out = ( out >> 16 ) ^ out;\
	} while ( 0 )	

_force_inline u32 
gs_hash_u64( u64 x )
{
	x = ( x ^ ( x >> 31 ) ^ ( x >> 62 ) ) * UINT64_C( 0x319642b2d24d8ec3 );
    x = ( x ^ ( x >> 27 ) ^ ( x >> 54 ) ) * UINT64_C( 0x96de1b173f119089 );
    x = x ^ ( x >> 30 ) ^ ( x >> 60 );
    return ( u32 )x;	
}

// Note(john): source: http://www.cse.yorku.ca/~oz/hash.html
// djb2 hash by dan bernstein
_force_inline u32 
gs_hash_str( const char* str )
{
    u32 hash = 5381;
    s32 c;
    while ( ( c = *str++ ) )
    {
        hash = ( ( hash << 5 ) + hash ) + c; /* hash * 33 + c */
    }
    return hash;
}

_force_inline u64
gs_hash_str_64( const char* str )
{
	u32 hash1 = 5381;
	u32 hash2 = 52711;
	u32 i = gs_string_length( str );
	while( i-- ) 
	{
		char c = str[ i ];
		hash1 = ( hash1 * 33 ) ^ c;
		hash2 = ( hash2 * 33 ) ^ c;
	}

	return ( hash1 >> 0 ) * 4096 + ( hash2 >> 0 );
}

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_UTIL_H__






