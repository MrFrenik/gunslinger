/*================================================================
    * Copyright: 2020 John Jackson
    * GSAsset: Meta Data Util for Gunslinger
    * File: gs_meta.h
    All Rights Reserved
=================================================================*/

#ifndef GS_META_H
#define GS_META_H

/*
    USAGE: (IMPORTANT)

    =================================================================================================================

    Before including, define the gunslinger asset manager implementation like this:

        #define GS_META_IMPL

    in EXACTLY ONE C or C++ file that includes this header, BEFORE the
    include, like this:

        #define GS_META_IMPL
        #include "gs_meta.h"

    All other files should just #include "gs_meta.h" without the #define.

    MUST include "gs.h" and declare GS_IMPL BEFORE this file, since this file relies on that:

        #define GS_IMPL
        #include "gs.h"

        #define GS_META_IMPL
        #include "gs_meta.h"

    ================================================================================================================
*/

/*==== Interface ====*/

/** @defgroup gs_meta_util Meta Data Util
 *  Gunslinger Meta Data Util
 *  @{
 */

#define GS_META_PROPERTY_FLAG_POINTER 		 0x01
#define GS_META_PROPERTY_FLAG_DOUBLE_POINTER 0x02

typedef struct gs_meta_property_type_info_t 
{
    const char* name;   // Used for display name
    uint32_t id;        // Id used for lookups and associations (most likely by enum)
	uint32_t flags;		// Number of times this field needs to be dereferenced
	union
	{
		struct
		{
			uint64_t enum_id;
		} enum_info;

		struct 
		{
			uint32_t key_id;
			uint32_t val_id;
		} container_info;
	} info;
} gs_meta_property_type_info_t;

// Default meta property type ids
typedef enum gs_meta_property_type
{
    GS_META_PROPERTY_TYPE_U8 = 0x00,
    GS_META_PROPERTY_TYPE_U16,
    GS_META_PROPERTY_TYPE_U32,
    GS_META_PROPERTY_TYPE_U64,
    GS_META_PROPERTY_TYPE_S8,
    GS_META_PROPERTY_TYPE_S16,
    GS_META_PROPERTY_TYPE_S32,
    GS_META_PROPERTY_TYPE_S64,
    GS_META_PROPERTY_TYPE_F32,
    GS_META_PROPERTY_TYPE_F64,
	GS_META_PROPERTY_TYPE_ENUM,
    GS_META_PROPERTY_TYPE_VEC2,
    GS_META_PROPERTY_TYPE_VEC3,
    GS_META_PROPERTY_TYPE_VEC4,
    GS_META_PROPERTY_TYPE_QUAT,
    GS_META_PROPERTY_TYPE_MAT3,
    GS_META_PROPERTY_TYPE_MAT4,
    GS_META_PROPERTY_TYPE_VQS,
    GS_META_PROPERTY_TYPE_UUID,
    GS_META_PROPERTY_TYPE_SIZE_T,       // Used for pointers or size_t variables
    GS_META_PROPERTY_TYPE_STR,          // Used for const char*, char*
	GS_META_PROPERTY_TYPE_OBJ,
    GS_META_PROPERTY_TYPE_COUNT
} gs_meta_property_type; 

GS_API_PRIVATE gs_meta_property_type_info_t _gs_meta_property_type_decl_impl(const char* name, uint32_t id);

#define _gs_meta_property_type_decl(T, PROP_TYPE)\
    _gs_meta_property_type_decl_impl(gs_to_str(T), PROP_TYPE)

// Default meta property type info defines
#define GS_META_PROPERTY_TYPE_INFO_U8       _gs_meta_property_type_decl(uint8_t, GS_META_PROPERTY_TYPE_U8)
#define GS_META_PROPERTY_TYPE_INFO_S8       _gs_meta_property_type_decl(int8_t, GS_META_PROPERTY_TYPE_S8)
#define GS_META_PROPERTY_TYPE_INFO_U16      _gs_meta_property_type_decl(uint16_t, GS_META_PROPERTY_TYPE_U16)
#define GS_META_PROPERTY_TYPE_INFO_S16      _gs_meta_property_type_decl(int16_t, GS_META_PROPERTY_TYPE_S16)
#define GS_META_PROPERTY_TYPE_INFO_U32      _gs_meta_property_type_decl(uint32_t, GS_META_PROPERTY_TYPE_U32)
#define GS_META_PROPERTY_TYPE_INFO_S32      _gs_meta_property_type_decl(int32_t, GS_META_PROPERTY_TYPE_S32)
#define GS_META_PROPERTY_TYPE_INFO_U64      _gs_meta_property_type_decl(uint64_t, GS_META_PROPERTY_TYPE_U64)
#define GS_META_PROPERTY_TYPE_INFO_S64      _gs_meta_property_type_decl(int64_t, GS_META_PROPERTY_TYPE_S64)
#define GS_META_PROPERTY_TYPE_INFO_F32      _gs_meta_property_type_decl(float, GS_META_PROPERTY_TYPE_F32)
#define GS_META_PROPERTY_TYPE_INFO_F64      _gs_meta_property_type_decl(double, GS_META_PROPERTY_TYPE_F64)
#define GS_META_PROPERTY_TYPE_INFO_ENUM     _gs_meta_property_type_decl(enum, GS_META_PROPERTY_TYPE_ENUM)
#define GS_META_PROPERTY_TYPE_INFO_VEC2     _gs_meta_property_type_decl(gs_vec2, GS_META_PROPERTY_TYPE_VEC2)
#define GS_META_PROPERTY_TYPE_INFO_VEC3     _gs_meta_property_type_decl(gs_vec3, GS_META_PROPERTY_TYPE_VEC3)
#define GS_META_PROPERTY_TYPE_INFO_VEC4     _gs_meta_property_type_decl(gs_vec4, GS_META_PROPERTY_TYPE_VEC4)
#define GS_META_PROPERTY_TYPE_INFO_QUAT     _gs_meta_property_type_decl(gs_quat, GS_META_PROPERTY_TYPE_QUAT)
#define GS_META_PROPERTY_TYPE_INFO_MAT3     _gs_meta_property_type_decl(gs_mat3, GS_META_PROPERTY_TYPE_MAT3)
#define GS_META_PROPERTY_TYPE_INFO_MAT4     _gs_meta_property_type_decl(gs_mat4, GS_META_PROPERTY_TYPE_MAT4)
#define GS_META_PROPERTY_TYPE_INFO_VQS      _gs_meta_property_type_decl(gs_vqs, GS_META_PROPERTY_TYPE_VQS)
#define GS_META_PROPERTY_TYPE_INFO_UUID     _gs_meta_property_type_decl(gs_uuid_t, GS_META_PROPERTY_TYPE_UUID)
#define GS_META_PROPERTY_TYPE_INFO_SIZE_T   _gs_meta_property_type_decl(size_t, GS_META_PROPERTY_TYPE_SIZE_T)
#define GS_META_PROPERTY_TYPE_INFO_STR      _gs_meta_property_type_decl(char*, GS_META_PROPERTY_TYPE_STR)
#define GS_META_PROPERTY_TYPE_INFO_OBJ      _gs_meta_property_type_decl(object, GS_META_PROPERTY_TYPE_OBJ)

typedef struct gs_meta_property_t
{
    const char* name;           
    uint32_t offset;            
	const char* type_name;
    gs_meta_property_type_info_t type;
} gs_meta_property_t;

typedef struct gs_meta_enum_value_t 
{
	const char* name;
} gs_meta_enum_value_t;

GS_API_PRIVATE gs_meta_property_t _gs_meta_property_impl(const char* field_type_name, const char* field, uint32_t offset, gs_meta_property_type_info_t type);

#define gs_meta_property(CLS, FIELD_TYPE_NAME, FIELD, TYPE)\
    _gs_meta_property_impl(gs_to_str(FIELD_TYPE_NAME), gs_to_str(FIELD), gs_offset(CLS, FIELD), TYPE)

typedef struct gs_meta_vtable_t
{
    gs_hash_table(uint64_t, void*) funcs;   // Hash function name to function pointer
} gs_meta_vtable_t;

typedef struct gs_meta_class_t
{
    gs_meta_property_t* properties;   // Property list
    uint32_t property_count;          // Number of properties in list
    const char* name;                 // Display name of class
    uint64_t id;                      // Class ID
    uint64_t base;                    // Parent class ID
    gs_meta_vtable_t vtable;          // VTable for class
    size_t size;                      // Size of class in bytes (for heap allocations)
} gs_meta_class_t;

typedef struct gs_meta_enum_t
{
	gs_meta_enum_value_t* values;	// Value list
	uint32_t value_count;			// Count of enum values
	const char* name;				// Name of enum
	uint64_t id;					// Enum id
} gs_meta_enum_t;

typedef struct gs_meta_registry_t
{
    gs_hash_table(uint64_t, gs_meta_class_t) classes;
	gs_hash_table(uint64_t, gs_meta_enum_t) enums; 
    void* user_data;
} gs_meta_registry_t;

typedef struct gs_meta_class_decl_t
{
    gs_meta_property_t* properties;
    size_t size;
    const char* name;                   // Display name of class
    const char* base;                   // Base parent class name (will be used for hash id, NULL for invalid id) 
    gs_meta_vtable_t* vtable;           // Vtable
    size_t cls_size;                    // Size of class in bytes
} gs_meta_class_decl_t;

typedef struct gs_meta_enum_decl_t
{
    gs_meta_enum_value_t* values;
    size_t size;
    const char* name;                   // Display name of class
} gs_meta_enum_decl_t;

GS_API_DECL gs_meta_registry_t gs_meta_registry_new();
GS_API_DECL void gs_meta_registry_free(gs_meta_registry_t* meta);
GS_API_DECL const char* gs_meta_typestr(gs_meta_property_type type);
GS_API_DECL bool32 gs_meta_has_base_class(const gs_meta_registry_t* meta, const gs_meta_class_t* cls);
GS_API_DECL uint64_t gs_meta_class_register(gs_meta_registry_t* meta, const gs_meta_class_decl_t* decl);
GS_API_DECL uint64_t gs_meta_enum_register(gs_meta_registry_t* meta, const gs_meta_enum_decl_t* decl); 

#define gs_meta_class_get(META, T)\
    (gs_hash_table_getp((META)->classes, gs_hash_str64(gs_to_str(T)))) 

#define gs_meta_class_get_w_name(META, NAME)\
	(gs_hash_table_getp((META)->classes, gs_hash_str64(NAME)))

#define gs_meta_class_get_w_id(META, ID)\
    (gs_hash_table_getp((META)->classes, (ID)))

#define gs_meta_class_exists(META, ID)\
    (gs_hash_table_exists((META)->classes, ID))

#define gs_meta_getv(OBJ, T, PROP)\
    (*((T*)((uint8_t*)(OBJ) + (PROP)->offset)))

#define gs_meta_getvp(OBJ, T, PROP)\
    (((T*)((uint8_t*)(OBJ) + (PROP)->offset))) 

#define gs_meta_func_get(CLS, NAME)\
    (_gs_meta_func_get_internal(CLS, gs_to_str(NAME))); 

#define gs_meta_func_get_w_id(META, ID, NAME)\
    (_gs_meta_func_get_internal_w_id(META, ID, gs_to_str(NAME))); 

GS_API_DECL void* _gs_meta_func_get_internal(const gs_meta_class_t* cls, const char* func_name);
GS_API_DECL void* _gs_meta_func_get_internal_w_id(const gs_meta_registry_t* meta, uint64_t id, const char* func_name);

// Reflection Utils

/** @} */ // end of gs_meta_data_util

/*==== Implementation ====*/

#ifdef GS_META_IMPL

GS_API_DECL gs_meta_registry_t gs_meta_registry_new()
{
    gs_meta_registry_t meta = gs_default_val();
    return meta;
}

GS_API_DECL void gs_meta_registry_free(gs_meta_registry_t* meta)
{
    // Free all entries in classes
    for (
        gs_hash_table_iter it = gs_hash_table_iter_new(meta->classes);
        gs_hash_table_iter_valid(meta->classes, it);
        gs_hash_table_iter_advance(meta->classes, it)
    ) 
    {
        gs_meta_class_t* cls = gs_hash_table_iter_getp(meta->classes, it);
        gs_free(cls->properties);
    }
    gs_hash_table_free(meta->classes);
}

GS_API_PRIVATE gs_meta_property_t _gs_meta_property_impl(const char* field_type_name, const char* field, uint32_t offset, gs_meta_property_type_info_t type)
{
    gs_meta_property_t mp = gs_default_val();
    mp.name = field;
	mp.type_name = field_type_name;
    mp.offset = offset;
    mp.type = type;
    return mp;
}

GS_API_PRIVATE uint64_t gs_meta_class_register(gs_meta_registry_t* meta, const gs_meta_class_decl_t* decl)
{
    uint32_t ct = decl->size / sizeof(gs_meta_property_t);
    gs_meta_class_t cls = gs_default_val();
    cls.properties = (gs_meta_property_t*)gs_malloc(decl->size);
	cls.property_count = ct;
    cls.name = decl->name;
    cls.base = decl->base ? gs_hash_str64(decl->base) : gs_hash_str64("NULL");
    memcpy(cls.properties, decl->properties, decl->size);
    uint64_t id = gs_hash_str64(decl->name);
    cls.id = id;
    cls.vtable = decl->vtable ? *decl->vtable : cls.vtable;
    cls.size = decl->cls_size;
    gs_hash_table_insert(meta->classes, id, cls);
    return id;
}

GS_API_DECL uint64_t gs_meta_enum_register(gs_meta_registry_t* meta, const gs_meta_enum_decl_t* decl)
{
    uint32_t ct = decl->size / sizeof(gs_meta_enum_value_t);
    gs_meta_enum_t enm = gs_default_val();
    enm.values = (gs_meta_enum_value_t*)gs_malloc(decl->size);
	enm.value_count = ct;
    enm.name = decl->name;
    memcpy(enm.values, decl->values, decl->size);
    uint64_t id = gs_hash_str64(decl->name);
    enm.id = id;
    gs_hash_table_insert(meta->enums, id, enm);
    return id;
}

GS_API_PRIVATE gs_meta_property_type_info_t _gs_meta_property_type_decl_impl(const char* name, uint32_t id)
{
    gs_meta_property_type_info_t info = gs_default_val();
    info.name = name;
    info.id = id;
    return info;
}

GS_API_DECL bool32 gs_meta_has_base_class(const gs_meta_registry_t* meta, const gs_meta_class_t* cls)
{
    return (gs_hash_table_key_exists(meta->classes, cls->base));
}

GS_API_DECL void* _gs_meta_func_get_internal(const gs_meta_class_t* cls, const char* func_name)
{
    uint64_t hash = gs_hash_str64(func_name);
    if (gs_hash_table_exists(cls->vtable.funcs, hash))
    {
        return gs_hash_table_get(cls->vtable.funcs, hash);
    }
    return NULL;
}

GS_API_DECL void* _gs_meta_func_get_internal_w_id(const gs_meta_registry_t* meta, uint64_t id, const char* func_name)
{
    const gs_meta_class_t* cls = gs_hash_table_getp(meta->classes, id);
    uint64_t hash = gs_hash_str64(func_name);
    if (gs_hash_table_exists(cls->vtable.funcs, hash))
    {
        return gs_hash_table_get(cls->vtable.funcs, hash);
    }
    return NULL;
}

#undef GS_META_IMP

#endif // GS_META_IMPL
#endif // GS_META_H

