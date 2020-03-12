#ifndef GS_TYPES_H
#define GS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>		// malloc, realloc, free ( for now )
#include <stdint.h>		// standard types
#include <limits.h>		// INT32_MAX, UINT32_MAX
#include <string.h> 	// memset
#include <float.h>		// FLT_MAX

#define _inline 			static inline
#define _local_persist 		static

 #if ( defined WIN32 || defined WIN64 )
	#define _force_inline 		static __forceinline
#else
	#define _force_inline 		static __attribute__((always_inline))
#endif

typedef enum
{
	gs_result_success,
	gs_result_in_progress,
	gs_result_incomplete,
	gs_result_failure
} gs_result;

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

#define u32_max			UINT32_MAX
#define s32_max			INT32_MAX
#define f32_max 		FLT_MAX

#ifdef __cplusplus
}
#endif 	// c++

#endif // GS_TYPES_H
