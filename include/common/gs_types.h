#ifndef __GS_TYPES_H__
#define __GS_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>		// malloc, realloc, free ( for now )
#include <stdint.h>		// standard types
#include <limits.h>		// INT32_MAX, UINT32_MAX
#include <string.h> 	// memset
#include <float.h>		// FLT_MAX
#include <time.h>
#include <ctype.h>

#define _inline 			static inline
#define _local_persist 		static
#define _global 			static

 #if ( defined _WIN32 || defined _WIN64 )
	#define _force_inline 		static __forceinline
#elif ( defined __APPLE__ || defined _APPLE )
	#define _force_inline 		static __attribute__((always_inline))
#else
	#define _force_inline 		_inline
#endif

/*============================================================
// Resource Declarations
============================================================*/

#define gs_resource( type )\
	gs_resource_##type

// Strongly typed declarations for resource handles (slot array handles)
#define gs_declare_resource_type( type )\
	typedef struct gs_resource( type ) {\
		u32 id;\
	} gs_resource( type );\

#define gs_resource_invalid( type )\
	(gs_resource( type )){ u32_max }

/*============================================================
// Result
============================================================*/

typedef enum
{
	gs_result_success,
	gs_result_in_progress,
	gs_result_incomplete,
	gs_result_failure
} gs_result;

/*============================================================
// Primitives
============================================================*/

#ifndef __cplusplus
	#define false 		0
	#define true 		1
#endif

typedef size_t		usize;

#ifdef __cplusplus
	typedef bool 		b8;
#else
	typedef _Bool 		b8;
#endif

typedef uint8_t 		u8;
typedef int8_t 			s8;
typedef uint16_t 		u16;
typedef int16_t 		s16;
typedef uint32_t 		u32;
typedef int32_t			s32;
typedef s32 			b32;
typedef uint64_t		u64;
typedef int64_t			s64;
typedef float 	 		f32;
typedef double			f64;
typedef const char* 	const_str;

#define u16_max 		UINT16_MAX
#define u32_max			UINT32_MAX
#define s32_max			INT32_MAX
#define f32_max 		FLT_MAX
#define f32_min 		FLT_MIN

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_TYPES_H__
