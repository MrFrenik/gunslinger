#ifndef __GS_META_CLASS_H__
#define __GS_META_CLASS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gs_types.h"
#include "common/gs_containers.h"

// Forward Decl.
struct gs_object;
struct gs_byte_buffer;

typedef enum 
{
	gs_meta_property_type_b8,
	gs_meta_property_type_u8,
	gs_meta_property_type_s8,
	gs_meta_property_type_u16,
	gs_meta_property_type_s16,
	gs_meta_property_type_u32,
	gs_meta_property_type_s32,
	gs_meta_property_type_s64,
	gs_meta_property_type_u64,
	gs_meta_property_type_f32,
	gs_meta_property_type_f64,
	gs_meta_property_type_vec2,
	gs_meta_property_type_vec3,
	gs_meta_property_type_vec4,
	gs_meta_property_type_quat,
	gs_meta_property_type_vqs,
	gs_meta_property_type_mat4,
	gs_meta_property_type_entity,
	gs_meta_property_type_const_str,
	gs_meta_property_type_uuid,
	gs_meta_property_type_enum,
	gs_meta_property_type_object,
	gs_meta_property_type_count,
} gs_meta_property_type;

// Type alias of unsigned 16 bit integer as a meta class id  
typedef u16 	gs_meta_class_id;

typedef struct
{
	gs_meta_property_type 	type;
	u16 					offset;
	const char* 			label;
} gs_meta_property;

#define gs_meta_property_set_value( prop, object, type, val )\
	*( type * )( ( u8* )object + prop->offset ) = val

#define gs_meta_property_get_value( prop, object, type )\
	*( ( type* )( ( u8* )object + prop->offset ) )

#define gs_type_to_meta_property_type( cls )\
	__gs_type_to_meta_property_type( (#cls) )

#define gs_type_to_meta_property_str( cls )\
	__gs_type_to_meta_property_str( (#cls) )

_inline gs_meta_property gs_meta_property_ctor( const char* _label, gs_meta_property_type _type, u16 _offset )
{
	gs_meta_property prop;
	prop.label 		= _label;
	prop.type 		= _type;
	prop.offset 	= _offset;
	return prop;
}

_inline 
const char* gs_meta_property_to_str( gs_meta_property_type type )
{
	switch ( type )
	{
		case gs_meta_property_type_u8: 			return "u8"; 			break;
		case gs_meta_property_type_s8: 			return "s8"; 			break;
		case gs_meta_property_type_u16: 		return "u16"; 			break;
		case gs_meta_property_type_s16: 		return "s16"; 			break;
		case gs_meta_property_type_u32: 		return "u32"; 			break;
		case gs_meta_property_type_s32: 		return "s32"; 			break;
		case gs_meta_property_type_u64: 		return "u64"; 			break;
		case gs_meta_property_type_s64: 		return "s64"; 			break;
		case gs_meta_property_type_f32: 		return "f32"; 			break;
		case gs_meta_property_type_f64: 		return "f64"; 			break;
		case gs_meta_property_type_vec2: 		return "gs_vec2"; 		break;
		case gs_meta_property_type_vec3: 		return "gs_vec3"; 		break;
		case gs_meta_property_type_vec4: 		return "gs_vec4"; 		break;
		case gs_meta_property_type_mat4: 		return "gs_mat4"; 		break;
		case gs_meta_property_type_quat: 		return "gs_quat"; 		break;
		case gs_meta_property_type_vqs: 		return "gs_vqs"; 		break;
		case gs_meta_property_type_const_str: 	return "const_str"; 	break;
		case gs_meta_property_type_entity: 		return "gs_entity"; 	break;
		case gs_meta_property_type_object: 		return "gs_object"; 	break;
		case gs_meta_property_type_uuid: 		return "gs_uuid"; 		break;
		case gs_meta_property_type_enum: 		return "enum"; 			break;
		default: 								return "unknown"; 		break;
	};
}

// NOTE(john): Might not belong in here, but will keep it here for now...
gs_result gs_meta_property_default_serialize( struct gs_byte_buffer* buffer, gs_meta_property* prop, struct gs_object* obj );
gs_result gs_meta_property_default_deserialize( struct gs_byte_buffer* buffer, struct gs_object* obj );

typedef gs_meta_property* 	gs_meta_property_ptr;
// Hash table := key: u32, val: gs_meta_property_ptr
gs_hash_table_decl( u32, gs_meta_property_ptr, gs_hash_u32, gs_hash_key_comp_std_type );

typedef struct gs_meta_class
{
	gs_meta_class_id 							id;
	gs_meta_property* 							properties;
	u16 										property_count;
	const char* 								label;
	gs_hash_table( u32, gs_meta_property_ptr )	property_name_to_index_table; 
	gs_result ( * serialize_func )( struct gs_object*, struct gs_byte_buffer* );
	gs_result ( * deserialize_func )( struct gs_object*, struct gs_byte_buffer* );
} gs_meta_class;

/*============================================================
// Meta Class Registry
============================================================*/

typedef struct
{
	// Has a list of registry information that can be index by ID
	gs_meta_class* classes;
	u32 count;
	gs_hash_table( u32, u32 ) class_idx_ht;
} gs_meta_class_registry;

const gs_meta_class* gs_meta_class_registry_get_class_by_label( gs_meta_class_registry* restrict registry, const char* label );

/*============================================================
// Function Decls
============================================================*/

const gs_meta_property* gs_meta_class_get_property_by_name( struct gs_object* obj, const char* name );

const char* gs_meta_class_get_label( gs_meta_class* restrict cls );

void gs_meta_class_registry_init_meta_properties( gs_meta_class_registry* restrict registry );

gs_meta_class* gs_meta_class_get( void* _obj );

gs_result gs_meta_class_registry_init( gs_meta_class_registry* restrict registry );

void gs_meta_property_value_to_str( gs_meta_property* restrict prop, struct gs_object* restrict obj, u8* restrict buffer, usize buffer_size );

const gs_meta_class* __gs_meta_class_impl( struct gs_object* restrict obj );

const char* __gs_type_name_cls( u32 id );

gs_meta_property_type __gs_type_to_meta_property_type( const char* type );

const char* __gs_type_to_meta_property_str( const char* type );	

gs_result __gs_object_serialization_base( struct gs_object* obj, struct gs_byte_buffer* buffer );
gs_result __gs_object_deserialization_base( struct gs_object* obj, struct gs_byte_buffer* buffer );

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_META_CLASS_H__


/*
	Meta class registry should have mechanism for registering and unregistering meta class information based on hashed key of type name
	Will need slot_map to map from hashed name to slot index in array of meta class information

	typedef struct gs_meta_class_registry {
		gs_slot_map( gs_meta_class_ptr ) classes;	
	} gs_meta_class_registry;

	const gs_meta_class* gs_register_meta_class( u32 hash, gs_meta_class_desc cls_desc )
	{
		
	}


*/






