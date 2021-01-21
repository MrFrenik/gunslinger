/*================================================================
    * Copyright: 2020 John Jackson
    * GSAsset: Meta Data Util for Gunslinger
    * File: gs_meta.h
    All Rights Reserved
=================================================================*/

#ifndef __GS_META_H__
#define __GS_META_H__

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

typedef enum gs_meta_property_type
{
    GS_META_PROPERTY_TYPE_U8,
    GS_META_PROPERTY_TYPE_U16,
    GS_META_PROPERTY_TYPE_U32,
    GS_META_PROPERTY_TYPE_U64,
    GS_META_PROPERTY_TYPE_S8,
    GS_META_PROPERTY_TYPE_S16,
    GS_META_PROPERTY_TYPE_S32,
    GS_META_PROPERTY_TYPE_S64,
    GS_META_PROPERTY_TYPE_F32,
    GS_META_PROPERTY_TYPE_F64
} gs_meta_property_type;

typedef struct gs_meta_property_t
{
    const char* name;           
    uint32_t offset;            
    gs_meta_property_type type;
} gs_meta_property_t;

typedef struct gs_meta_class_t
{
    gs_meta_property_t* properties;
    uint32_t property_count;
} gs_meta_class_t;

typedef struct gs_meta_registry_t
{
    gs_hash_table(u64, gs_meta_class_t) meta_classes;
} gs_meta_registry_t;

typedef struct gs_meta_class_decl_t
{
    gs_meta_property_t* properties;
    size_t properties_size;
} gs_meta_class_decl_t;

GS_API_DECL gs_meta_registry_t gs_meta_registry_new();
GS_API_DECL const char* gs_meta_typestr(gs_meta_property_type type);

#define gs_meta_register_class(META, T, DECL)\
    do {\
        uint32_t ct = (DECL)->properties_size / sizeof(gs_meta_property_t);\
        size_t sz = sizeof(gs_meta_property_t) * ct;\
        gs_meta_class_t cls = gs_default_val();\
        cls.property_count = ct;\
        cls.properties = gs_malloc(sz);\
        memcpy(cls.properties, (DECL)->properties, sz);\
        gs_hash_table_insert((META)->meta_classes, gs_hash_str64(gs_to_str(T)), cls);\
    } while (0)

#define gs_meta_get_class(META, T)\
    gs_hash_table_getp((META)->meta_classes, gs_hash_str64(gs_to_str(T)))

#define gs_meta_getv(OBJ, T, PROP)\
    (*((T*)((uint8_t*)(OBJ) + (PROP)->offset)))

#define gs_meta_getvp(OBJ, T, PROP)\
    (((T*)((uint8_t*)(OBJ) + (PROP)->offset)))

gs_meta_property_t __gs_meta_property_impl(const char* name, uint32_t offset, gs_meta_property_type type)
{
    gs_meta_property_t mp = gs_default_val();
    mp.name = name;
    mp.offset = offset;
    mp.type = type;
    return mp;
}

#define gs_meta_property_decl(NAME, T, TYPE)\
    __gs_meta_property_impl(gs_to_str(NAME), gs_offset(T, NAME), TYPE)

/*
    #define GS_META_IMPL
    #include <gs/util/gs_meta.h>

    gs_meta_data_t* data = gs_meta_getp(&meta, gs_asset_texture_t);
    gs_assert(data);

    gs_asset_texture_t tex = {0};

    typedef struct gs_meta_property_t
    {
        gs_meta_property_type type;
        uint32_t offset;
    } gs_meta_property_t;

    typedef struct gs_meta_data_t
    {
        gs_dyn_array(gs_meta_property_t) properties;
        uint32_t property_count;
    } gs_meta_data_t;

    // Have to feed in meta registry data...somehow
    typedef struct gs_meta_registry_t
    {
    } gs_meta_registry_t;

    gs_meta_registry_t meta = gs_meta_registry_new();

    gs_meta_property_t properties[] = {
        (gs_meta_property_t){.offset = gs_offset(T, prop)},
        ...
    };

    gs_meta_registry_cls_decl_t cdecl = {
        .properties = properties,
        .size = sizeof(properties)
    };

    gs_meta_register_cls(&meta, T, &cdecl);

    for (uint32_t i = 0; i < data->property_count; ++i)
    {
        gs_meta_property_t* prop = &data->properties[i];
        switch (prop->type)
        {
            case GS_META_PROPERTY_TYPE_F32: 
            {
                f32 v = gs_meta_class_getv(&tex, f32, prop);
            } break;

        }
    }

    // Utility for generating reflection data / reading reflection data
    // Has to be run as a pre-processing step
    // Generate reflection information
    // Write parser to generate reflection code

    Types:

    u8
    i8
    u16
    i16
    u32
    i32
    u64
    i64
    f32
    f64
    gs_dyn_array(u64)

    Example: 
    struct gs_asset_texture_t
    {
        f32 : mValue,
        f64 : 
        u32 : 
        i32 : 
    };
    
*/

/*==== Implementation ====*/

#ifdef GS_META_IMPL

gs_meta_registry_t gs_meta_registry_new()
{
    gs_meta_registry_t meta = gs_default_val();
    return meta;
}

const char* gs_meta_typestr(gs_meta_property_type type)
{
    switch (type)
    {
        case GS_META_PROPERTY_TYPE_U8:  return gs_to_str(GS_META_PROPERTY_TYPE_U8); break;
        case GS_META_PROPERTY_TYPE_S8:  return gs_to_str(GS_META_PROPERTY_TYPE_S8); break;
        case GS_META_PROPERTY_TYPE_U16: return gs_to_str(GS_META_PROPERTY_TYPE_U16); break;
        case GS_META_PROPERTY_TYPE_S16: return gs_to_str(GS_META_PROPERTY_TYPE_S16); break;
        case GS_META_PROPERTY_TYPE_U32: return gs_to_str(GS_META_PROPERTY_TYPE_U32); break;
        case GS_META_PROPERTY_TYPE_S32: return gs_to_str(GS_META_PROPERTY_TYPE_S32); break;
        case GS_META_PROPERTY_TYPE_U64: return gs_to_str(GS_META_PROPERTY_TYPE_U64); break;
        case GS_META_PROPERTY_TYPE_S64: return gs_to_str(GS_META_PROPERTY_TYPE_S64); break;
        case GS_META_PROPERTY_TYPE_F32: return gs_to_str(GS_META_PROPERTY_TYPE_F32); break;
        case GS_META_PROPERTY_TYPE_F64: return gs_to_str(GS_META_PROPERTY_TYPE_F64); break;
    }
    return "invalid";
}

#undef GS_META_IMPL

#endif // GS_META_IMPL
#endif // __GS_META_H__

