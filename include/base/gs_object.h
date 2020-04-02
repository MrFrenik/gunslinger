#ifndef __GS_OBJECT_H__
#define __GS_OBJECT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gs_types.h"
#include "common/gs_util.h"

// Intropection / Reflection keywords that should "compile away" but can be used for code generation
#define _introspect				gs_empty_instruction( _introspect )
#define _non_serializable		gs_empty_instruction( _non_serializable )
#define _immutable				gs_empty_instruction( _immutable )
#define _read_only				gs_empty_instruction( _immutable )
#define _ignore 				gs_empty_instruction( _ignore )
#define _default( ... )			gs_empty_instruction( _default )
#define _attributes( ... )		gs_empty_instruction( _attributes )
#define _ctor( ... )			gs_empty_instruction( _ctor )
#define _defaults( ... ) 		gs_empty_instruction( _defaults )
#define _serialize( ... )		gs_empty_instruction( _serialize )
#define _deserialize( ... )	 	gs_empty_instruction( _deserialize )
#define _components( ... )		gs_empty_instruction( _components )
#define _struct_default( type ) type##_default

// Helper macro for typedefing a struture definition
#define gs_struct_def( name, ... ) typedef struct { __VA_ARGS__ } name

// Definition for derived struct ( based another parent struct )
#define gs_derive_def( name, parent, ... ) gs_struct_def( name, parent _base; __VA_ARGS__ )

#define gs_engine_check( statement ) ( statement ) ? true : false

#define _base( base_type ) base_type _base

/*============================================================
// Object Definition: gs_object
============================================================*/

// This could, instead, be a way to grab the meta class from the given reflected object
typedef struct gs_object
{
	// Function pointer for finding the id for a particular object instance
	u32 ( * type_id )();
} gs_object;

// Helper macro for specifically deriving some structure from an object struct
#define gs_object_def( name, ... ) gs_derive_def( name, object, __VA_ARGS__ )

#define gs_construct( type )\
	*( type* )__gs_default_object_##type()

#define gs_construct_heap( type )\
	( type* )__gs_default_object_##type##_heap()

_inline u32 __gs_type_id_impl( gs_object* obj ) 
{
	gs_assert( obj->type_id != NULL );
	return ( obj->type_id() );
}

const char* gs_type_name_obj( gs_object* obj );

#define gs_type_name( obj )\
	gs_type_name_obj( gs_cast( gs_object, obj ) )

#define gs_type_id( obj )\
	__gs_type_id_impl( gs_cast( gs_object, obj ) )

#define gs_meta_class( obj )\
	__gs_meta_class_impl( gs_cast( gs_object, obj ) )

#define gs_type_id_cls( cls )\
	( u32 )gs_meta_class_id_##cls

// #define gs_type_name_cls( cls )\
// 	__gs_type_name_cls( gs_type_id_cls( cls ) )

#define gs_type_name_cls( cls )\
	__gs_type_name_cls( cls )

void 
__gs_object_print_impl( gs_object* obj );

#define gs_object_print( obj )\
	( __gs_object_print_impl( gs_cast( gs_object, ( obj ) ) ) )

#define gs_object_serialize( obj, buffer )\
	gs_meta_class( gs_cast( gs_object, obj ) )->serialize_func( gs_cast( gs_object, obj ), buffer )

#define gs_object_deserialize( obj, buffer )\
	gs_meta_class( gs_cast( gs_object, obj ) )->deserialize_func( gs_cast( gs_object, obj ), buffer )

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_OBJECT_H__
