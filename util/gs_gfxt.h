/*================================================================
    * Copyright: 2020 John Jackson
    * gs_gfxt: Graphics Extension Util for Gunslinger
    * File: gs_gfxt.h
    All Rights Reserved
=================================================================*/

#ifndef GS_GFXT_H
#define GS_GFXT_H

/*
    USAGE: (IMPORTANT)

    =================================================================================================================

    Before including, define the gunslinger graphics extension implementation like this:

        #define GS_GFXT_IMPL

    in EXACTLY ONE C or C++ file that includes this header, BEFORE the
    include, like this:

        #define GS_GFXT_IMPL
        #include "gs_gfxt.h"

    All other files should just #include "gs_gfxt.h" without the #define.

    MUST include "gs.h" and declare GS_IMPL BEFORE this file, since this file relies on gunslinger core:

        #define GS_IMPL
        #include "gs.h"

        #define GS_GFXT_IMPL
        #include "gs_gfxt.h"

    GS_GFXT_HNDL: 

        Internally, gfxt uses handles to retrieve raw data for shared types, like materials, meshes, pipelines, etc.
        This keeps memory consumption low allowing for trivial copying of references and handles without having to copy raw data.
        This handle, by default, is defined to a void* for pointers for raw data storage. This can be defined by the user to whatever
        handle type he'd like to use, such as a uint32_t handle to a slot array id. Additionally, all description objects that 
        operate on shared data pass around a function pointer descriptor for how to retrieve that data - all of which can be user defined.

    ================================================================================================================
*/

/*==== Interface ====*/

/** @defgroup gs_graphics_extension_util Graphics Extension Util
 *  Gunslinger Graphics Extension Util
 *  @{
 */

//=== Raw data retrieval/Handles ===//
#ifndef GS_GFXT_HNDL
    #define GS_GFXT_HNDL void*                          // Default handle will just be a pointer to the data itself 
#endif

// Config
#ifndef GS_GFXT_TEX_COORD_MAX
    #define GS_GFXT_TEX_COORD_MAX 4
#endif

#ifndef GS_GFXT_COLOR_MAX
    #define GS_GFXT_COLOR_MAX 4
#endif

#ifndef GS_GFXT_JOINT_MAX
    #define GS_GFXT_JOINT_MAX 4
#endif

#ifndef GS_GFXT_WEIGHT_MAX
    #define GS_GFXT_WEIGHT_MAX 4
#endif

// Custom UINT field
#ifndef GS_GFXT_CUSTOM_UINT_MAX
    #define GS_GFXT_CUSTOM_UINT_MAX 4
#endif

#ifndef GS_GFXT_INCLUDE_DIR_MAX
    #define GS_GFXT_INCLUDE_DIR_MAX 8 
#endif

#ifndef GS_GFXT_UNIFORM_VIEW_MATRIX
    #define GS_GFXT_UNIFORM_VIEW_MATRIX "U_VIEW_MTX"
#endif

#ifndef GS_GFXT_UNIFORM_PROJECTION_MATRIX
    #define GS_GFXT_UNIFORM_PROJECTION_MATRIX "U_PROJECTION_MTX"
#endif

#ifndef GS_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX
    #define GS_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX "U_VIEW_PROJECTION_MTX"
#endif

#ifndef GS_GFXT_UNIFORM_MODEL_MATRIX
    #define GS_GFXT_UNIFORM_MODEL_MATRIX "U_MODEL_MTX"
#endif

#ifndef GS_GFXT_UNIFORM_INVERSE_MODEL_MATRIX
    #define GS_GFXT_UNIFORM_INVERSE_MODEL_MATRIX "U_INVERSE_MODEL_MTX"
#endif

#ifndef GS_GFXT_UNIFORM_VIEW_WORLD_POSITION
    #define GS_GFXT_UNIFORM_VIEW_WORLD_POSITION "U_VIEW_WORLD_POSITION"
#endif

#ifndef GS_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX
    #define GS_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX "U_MVP_MTX"
#endif

#ifndef GS_GFXT_UNIFORM_TIME
    #define GS_GFXT_UNIFORM_TIME "U_TIME"
#endif

typedef void* (*gs_gfxt_raw_data_func)(GS_GFXT_HNDL hndl, void* user_data);

#define GS_GFXT_RAW_DATA(FUNC_DESC, T)\
    ((T*)(FUNC_DESC)->func((FUNC_DESC)->hndl, (FUNC_DESC)->user_data))

typedef struct gs_gfxt_raw_data_func_desc_t {
    GS_GFXT_HNDL hndl;                              // Handle used for retrieving data.
    gs_gfxt_raw_data_func func;                     // User defined function for pipeline data retrieval
    void* user_data;                                // Optional user data for function
} gs_gfxt_raw_data_func_desc_t;

//=== Uniforms/Uniform blocks ===//
typedef struct gs_gfxt_uniform_desc_t {
    char name[64];                                  // Name of uniform (for binding to shader)
    gs_graphics_uniform_type type;                  // Type of uniform: GS_GRAPHICS_UNIFORM_VEC2, GS_GRAPHICS_UNIFORM_VEC3, etc.
    uint32_t binding;                               // Binding for this uniform in shader
    gs_graphics_shader_stage_type stage;            // Shader stage for this uniform
    gs_graphics_access_type access_type;            // Access type for this uniform (compute only)
} gs_gfxt_uniform_desc_t;

typedef struct gs_gfxt_uniform_t {
    gs_handle(gs_graphics_uniform_t) hndl;          // Graphics handle resource for actual uniform
    uint32_t offset;                                // Individual offset for this uniform in material byte buffer data
    uint32_t binding;                               // Binding for this uniform
    size_t size;                                    // Size of this uniform data in bytes
    gs_graphics_uniform_type type;                  // Type of this uniform
    gs_graphics_access_type access_type;            // Access type of uniform (compute only)
} gs_gfxt_uniform_t;

typedef struct gs_gfxt_uniform_block_desc_t {
    gs_gfxt_uniform_desc_t* layout;                 // Layout for all uniform data for this block to hold
    size_t size;                                    // Size of layout in bytes
} gs_gfxt_uniform_block_desc_t;

typedef struct gs_gfxt_uniform_block_lookup_key_t {
    char name[64];
} gs_gfxt_uniform_block_lookup_key_t;

typedef struct gs_gfxt_uniform_block_t {
    gs_dyn_array(gs_gfxt_uniform_t) uniforms;    // Raw uniform handle array
    gs_hash_table(uint64_t, uint32_t) lookup;    // Index lookup table (used for byte buffer offsets in material uni. data)
    size_t size;                                 // Total size of material data for entire block
} gs_gfxt_uniform_block_t; 

//=== Texture ===//
typedef gs_handle(gs_graphics_texture_t) gs_gfxt_texture_t;

//=== Mesh ===// 
typedef gs_asset_mesh_attribute_type gs_gfxt_mesh_attribute_type;
typedef gs_asset_mesh_layout_t gs_gfxt_mesh_layout_t;

/*
    typedef struct
    {
        union
        {
            void* interleave; 
            struct
            {
                void* positions;
                void* normals;
                void* tangents;
                void* tex_coords[TEX_COORD_MAX];
                void* joints[JOINT_MAX];
                void* weights[WEIGHT_MAX];
            } non_interleave;
        } vertex;
        size_t vertex_size;
        void* indices;
    } gs_gfxt_mesh_primitive_data_t;
*/

typedef struct
{
    void* data;
    size_t size;
} gs_gfxt_mesh_vertex_attribute_t;

typedef struct 
{
    gs_gfxt_mesh_vertex_attribute_t positions;         // All position data
    gs_gfxt_mesh_vertex_attribute_t normals;
    gs_gfxt_mesh_vertex_attribute_t tangents;
    gs_gfxt_mesh_vertex_attribute_t tex_coords[GS_GFXT_TEX_COORD_MAX];
    gs_gfxt_mesh_vertex_attribute_t colors[GS_GFXT_COLOR_MAX];
    gs_gfxt_mesh_vertex_attribute_t joints[GS_GFXT_JOINT_MAX];
    gs_gfxt_mesh_vertex_attribute_t weights[GS_GFXT_WEIGHT_MAX];
    gs_gfxt_mesh_vertex_attribute_t custom_uint[GS_GFXT_CUSTOM_UINT_MAX];
    gs_gfxt_mesh_vertex_attribute_t indices;                           
    uint32_t count;                                                     // Total count of indices
} gs_gfxt_mesh_vertex_data_t;

// Structured/packed raw mesh data
// TODO(john): Make the primitives array static to avoid heap alloc
typedef struct gs_gfxt_mesh_raw_data_t {
    gs_dyn_array(gs_gfxt_mesh_vertex_data_t) primitives;   // All primitive data
} gs_gfxt_mesh_raw_data_t;

typedef struct gs_gfxt_mesh_import_options_t {
    gs_gfxt_mesh_layout_t* layout;        // Mesh attribute layout array
    size_t size;                          // Size of mesh attribute layout array in bytes
    size_t index_buffer_element_size;     // Size of index data size in bytes
} gs_gfxt_mesh_import_options_t;

GS_API_DECL void gs_gfxt_mesh_import_options_free(gs_gfxt_mesh_import_options_t* opt);

typedef struct gs_gfxt_mesh_desc_s {
    gs_gfxt_mesh_raw_data_t* meshes;   // Mesh data array
    size_t size;                       // Size of mesh data array in bytes
    bool32 keep_data;                  // Whether or not to free data after use
} gs_gfxt_mesh_desc_t;

typedef struct gs_gfxt_vertex_stream_s
{
    gs_handle(gs_graphics_vertex_buffer_t) positions;
    gs_handle(gs_graphics_vertex_buffer_t) normals;
    gs_handle(gs_graphics_vertex_buffer_t) tangents;
    gs_handle(gs_graphics_vertex_buffer_t) colors[GS_GFXT_COLOR_MAX];
    gs_handle(gs_graphics_vertex_buffer_t) tex_coords[GS_GFXT_TEX_COORD_MAX];
    gs_handle(gs_graphics_vertex_buffer_t) joints[GS_GFXT_JOINT_MAX];
    gs_handle(gs_graphics_vertex_buffer_t) weights[GS_GFXT_WEIGHT_MAX];
    gs_handle(gs_graphics_vertex_buffer_t) custom_uint[GS_GFXT_CUSTOM_UINT_MAX];
} gs_gfxt_vertex_stream_t;

typedef struct gs_gfxt_mesh_primitive_s {
    gs_gfxt_vertex_stream_t stream;                 // All vertex data streams
    gs_handle(gs_graphics_index_buffer_t) indices;  // Index buffer
    uint32_t count;                                 // Total number of vertices
} gs_gfxt_mesh_primitive_t;

typedef struct gs_gfxt_mesh_s {
    gs_dyn_array(gs_gfxt_mesh_primitive_t) primitives;
    gs_gfxt_mesh_desc_t desc;
} gs_gfxt_mesh_t;

//=== Pipeline ===//
typedef struct gs_gfxt_pipeline_desc_s {
    gs_graphics_pipeline_desc_t  pip_desc;           // Description for constructing pipeline object
    gs_gfxt_uniform_block_desc_t ublock_desc;        // Description for constructing uniform block object
} gs_gfxt_pipeline_desc_t;

typedef struct gs_gfxt_pipeline_s {
    gs_handle(gs_graphics_pipeline_t) hndl;                         // Graphics handle resource for actual pipeline
    gs_gfxt_uniform_block_t ublock;                                 // Uniform block for holding all uniform data
    gs_dyn_array(gs_gfxt_mesh_layout_t) mesh_layout;  
    gs_graphics_pipeline_desc_t desc;
} gs_gfxt_pipeline_t;

//=== Material ===//
typedef struct gs_gfxt_material_desc_s {
    gs_gfxt_raw_data_func_desc_t pip_func;      // Description for retrieving raw pipeline pointer data from handle.
} gs_gfxt_material_desc_t;

typedef struct gs_gfxt_material_s {
    gs_gfxt_material_desc_t desc;               // Material description object
    gs_byte_buffer_t uniform_data;              // Byte buffer of actual uniform data to send to GPU
    gs_byte_buffer_t image_buffer_data;         // Image buffer data
} gs_gfxt_material_t; 

//=== Renderable ===// 
typedef struct gs_gfxt_renderable_desc_s {
    gs_gfxt_raw_data_func_desc_t mesh;      // Description for retrieving raw mesh pointer data from handle.
    gs_gfxt_raw_data_func_desc_t material;  // Description for retrieving raw material pointer data from handle.
} gs_gfxt_renderable_desc_t;

typedef struct gs_gfxt_renderable_s {
    gs_gfxt_renderable_desc_t desc;     // Renderable description object
    gs_mat4 model_matrix;               // Model matrix for renderable
} gs_gfxt_renderable_t;

//=== Graphics scene ===//
typedef struct gs_gfxt_scene_s {
    gs_slot_array(gs_gfxt_renderable_t) renderables;
} gs_gfxt_scene_t;

//==== API =====//

//=== Creation ===// 
GS_API_DECL gs_gfxt_pipeline_t      gs_gfxt_pipeline_create(const gs_gfxt_pipeline_desc_t* desc);
GS_API_DECL gs_gfxt_material_t      gs_gfxt_material_create(gs_gfxt_material_desc_t* desc);
GS_API_DECL gs_gfxt_mesh_t          gs_gfxt_mesh_create(const gs_gfxt_mesh_desc_t* desc); 
GS_API_DECL void                    gs_gfxt_mesh_update_or_create(gs_gfxt_mesh_t* mesh, const gs_gfxt_mesh_desc_t* desc);
GS_API_DECL gs_gfxt_renderable_t    gs_gfxt_renderable_create(const gs_gfxt_renderable_desc_t* desc);
GS_API_DECL gs_gfxt_uniform_block_t gs_gfxt_uniform_block_create(const gs_gfxt_uniform_block_desc_t* desc);
GS_API_DECL gs_gfxt_texture_t       gs_gfxt_texture_create(gs_graphics_texture_desc_t* desc); 

//=== Destruction ===// 
GS_API_DECL void gs_gfxt_texture_destroy(gs_gfxt_texture_t* texture);
GS_API_DECL void gs_gfxt_material_destroy(gs_gfxt_material_t* material);
GS_API_DECL void gs_gfxt_mesh_destroy(gs_gfxt_mesh_t* mesh);
GS_API_DECL void gs_gfxt_uniform_block_destroy(gs_gfxt_uniform_block_t* ub);
GS_API_DECL void gs_gfxt_pipeline_destroy(gs_gfxt_pipeline_t* pipeline);

//=== Resource Loading ===//
GS_API_DECL gs_gfxt_pipeline_t gs_gfxt_pipeline_load_from_file(const char* path);
GS_API_DECL gs_gfxt_pipeline_t gs_gfxt_pipeline_load_from_memory(const char* data, size_t sz);
GS_API_DECL gs_gfxt_pipeline_t gs_gfxt_pipeline_load_from_memory_ext(const char* data, size_t sz, const char* file_dir);
GS_API_DECL gs_gfxt_texture_t  gs_gfxt_texture_load_from_file(const char* path, gs_graphics_texture_desc_t* desc, bool flip, bool keep_data);
GS_API_DECL gs_gfxt_texture_t  gs_gfxt_texture_load_from_memory(const char* data, size_t sz, gs_graphics_texture_desc_t* desc, bool flip, bool keep_data);

//=== Copy ===//
GS_API_DECL gs_gfxt_material_t gs_gfxt_material_deep_copy(gs_gfxt_material_t* src);

//=== Pipeline API ===//
GS_API_DECL gs_gfxt_uniform_t* gs_gfxt_pipeline_get_uniform(gs_gfxt_pipeline_t* pip, const char* name);

//=== Material API ===//
GS_API_DECL void gs_gfxt_material_set_uniform(gs_gfxt_material_t* mat, const char* name, const void* data);
GS_API_DECL void gs_gfxt_material_bind(gs_command_buffer_t* cb, gs_gfxt_material_t* mat);
GS_API_DECL void gs_gfxt_material_bind_pipeline(gs_command_buffer_t* cb, gs_gfxt_material_t* mat);
GS_API_DECL void gs_gfxt_material_bind_uniforms(gs_command_buffer_t* cb, gs_gfxt_material_t* mat);
GS_API_DECL gs_gfxt_pipeline_t* gs_gfxt_material_get_pipeline(gs_gfxt_material_t* mat);

//=== Mesh API ===//
GS_API_DECL void gs_gfxt_mesh_draw_pipeline(gs_command_buffer_t* cb, gs_gfxt_mesh_t* mesh, gs_gfxt_pipeline_t* pip);
GS_API_DECL void gs_gfxt_mesh_draw_material(gs_command_buffer_t* cb, gs_gfxt_mesh_t* mesh, gs_gfxt_material_t* mat);
GS_API_DECL void gs_gfxt_mesh_draw_materials(gs_command_buffer_t* cb, gs_gfxt_mesh_t* mesh, gs_gfxt_material_t** mats, size_t mats_size);
GS_API_DECL void gs_gfxt_mesh_draw_layout(gs_command_buffer_t* cb, gs_gfxt_mesh_t* mesh, gs_gfxt_mesh_layout_t* layout, size_t layout_size);
GS_API_DECL gs_gfxt_mesh_t gs_gfxt_mesh_load_from_file(const char* file, gs_gfxt_mesh_import_options_t* options);
GS_API_DECL bool gs_gfxt_load_gltf_data_from_file(const char* path, gs_gfxt_mesh_import_options_t* options, gs_gfxt_mesh_raw_data_t** out, uint32_t* mesh_count);

// Util API
GS_API_DECL void* gs_gfxt_raw_data_default_impl(GS_GFXT_HNDL hndl, void* user_data);

// Mesh Generation API
GS_API_DECL gs_gfxt_mesh_t gs_gfxt_mesh_unit_quad_generate(gs_gfxt_mesh_import_options_t* options);
gs_handle(gs_graphics_texture_t) gs_gfxt_texture_generate_default();

/** @} */ // end of gs_graphics_extension_util

#ifdef GS_GFXT_IMPL
/*==== Implementation ====*/

// Creation/Destruction
GS_API_DECL gs_gfxt_pipeline_t 
gs_gfxt_pipeline_create(const gs_gfxt_pipeline_desc_t* desc)
{
    gs_gfxt_pipeline_t pip = gs_default_val();

    if (!desc) {
        gs_assert(false);
        return pip;
    }

    pip.hndl = gs_graphics_pipeline_create(&desc->pip_desc);
    pip.ublock = gs_gfxt_uniform_block_create(&desc->ublock_desc);
    pip.desc = desc->pip_desc;
	pip.desc.layout.attrs = gs_malloc(desc->pip_desc.layout.size);
	memcpy(pip.desc.layout.attrs, desc->pip_desc.layout.attrs, desc->pip_desc.layout.size);
    return pip;
}

GS_API_DECL gs_gfxt_uniform_block_t 
gs_gfxt_uniform_block_create(const gs_gfxt_uniform_block_desc_t* desc)
{
    gs_gfxt_uniform_block_t block = gs_default_val();

    if (!desc) return block;

    // Iterate through layout, construct uniforms, place them into hash table
    uint32_t offset = 0;
    uint32_t image2D_offset = 0;
    uint32_t ct = desc->size / sizeof(gs_gfxt_uniform_desc_t);
    for (uint32_t i = 0; i < ct; ++i)
    {
        gs_gfxt_uniform_desc_t* ud = &desc->layout[i];

        gs_gfxt_uniform_t u = gs_default_val();
        gs_graphics_uniform_desc_t u_desc = gs_default_val();
        gs_graphics_uniform_layout_desc_t u_layout = gs_default_val();
        u_layout.type = ud->type;
        memcpy(u_desc.name, ud->name, 64);
        u_desc.layout = &u_layout;          
        u.binding = ud->binding;
        u.type = ud->type; 

        // Determine offset/hndl
        switch (ud->type)
        {
            case GS_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F:
            {
                u.offset = image2D_offset;
            } break;

            default:
            {
                u.hndl = gs_graphics_uniform_create(&u_desc);
                u.offset = offset;
            } break; 
        }

        // Add to data offset based on type
        switch (ud->type) {
            default:
            case GS_GRAPHICS_UNIFORM_FLOAT:     offset += sizeof(float); break;
            case GS_GRAPHICS_UNIFORM_INT:       offset += sizeof(int32_t); break;
            case GS_GRAPHICS_UNIFORM_VEC2:      offset += sizeof(gs_vec2); break;
            case GS_GRAPHICS_UNIFORM_VEC3:      offset += sizeof(gs_vec3); break;
            case GS_GRAPHICS_UNIFORM_VEC4:      offset += sizeof(gs_vec4); break;
            case GS_GRAPHICS_UNIFORM_MAT4:      offset += sizeof(gs_mat4); break;
            case GS_GRAPHICS_UNIFORM_SAMPLER2D: offset += sizeof(gs_handle(gs_graphics_texture_t)); break;
            case GS_GRAPHICS_UNIFORM_USAMPLER2D: offset += sizeof(gs_handle(gs_graphics_texture_t)); break;
            case GS_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F: 
            {
                image2D_offset += sizeof(gs_handle(gs_graphics_texture_t));
            } break;
        }

        // Add uniform to block with name as key
        uint64_t key = gs_hash_str64(ud->name);
        gs_dyn_array_push(block.uniforms, u);
        gs_hash_table_insert(block.lookup, key, gs_dyn_array_size(block.uniforms) - 1);
    }
    block.size = offset;

    return block;
}

GS_API_DECL gs_gfxt_texture_t 
gs_gfxt_texture_create(gs_graphics_texture_desc_t* desc)
{
    return gs_graphics_texture_create(desc);
}

GS_API_DECL gs_gfxt_material_t 
gs_gfxt_material_create(gs_gfxt_material_desc_t* desc)
{
    gs_gfxt_material_t mat = gs_default_val();

    if (!desc) {
        gs_assert(false);
        return mat;
    }

    // Set desc information to defaults if not provided.
    if (!desc->pip_func.func) desc->pip_func.func = gs_gfxt_raw_data_default_impl;
    gs_gfxt_pipeline_t* pip = GS_GFXT_RAW_DATA(&desc->pip_func, gs_gfxt_pipeline_t);
    gs_assert(pip);

    mat.desc = *desc;
    mat.uniform_data = gs_byte_buffer_new();
    mat.image_buffer_data = gs_byte_buffer_new();

    gs_byte_buffer_resize(&mat.uniform_data, pip->ublock.size);
    gs_byte_buffer_memset(&mat.uniform_data, 0);
    return mat;
}

GS_API_DECL gs_gfxt_mesh_t 
gs_gfxt_mesh_create(const gs_gfxt_mesh_desc_t* desc)
{
    gs_gfxt_mesh_t mesh = gs_default_val();

    if (!desc) {
        return mesh;
    }

    const uint32_t mesh_count = desc->size / sizeof(gs_gfxt_mesh_raw_data_t);

    // Process all mesh data, add meshes
    for (uint32_t i = 0; i < mesh_count; ++i)
    {
        gs_gfxt_mesh_raw_data_t* m = &desc->meshes[i];

        for (uint32_t p = 0; p < gs_dyn_array_size(m->primitives); ++p)
        {
            // Get raw vertex data
            gs_gfxt_mesh_vertex_data_t* vdata = &m->primitives[p];

            // Construct primitive
            gs_gfxt_mesh_primitive_t prim = gs_default_val();
            prim.count = vdata->count;

            // Positions
            if (vdata->positions.data) 
            {
                gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
                vdesc.data = vdata->positions.data;
                vdesc.size = vdata->positions.size;
                prim.stream.positions = gs_graphics_vertex_buffer_create(&vdesc);
                if (!desc->keep_data)
                { 
                    gs_free(vdata->positions.data);
                }
            }

            // Normals
            if (vdata->normals.data) 
            {
                gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
                vdesc.data = vdata->normals.data;
                vdesc.size = vdata->normals.size;
                prim.stream.normals = gs_graphics_vertex_buffer_create(&vdesc);
                if (!desc->keep_data)
                { 
                    gs_free(vdata->normals.data);
                }
            }

            // Tangents
            if (vdata->tangents.data) 
            {
                gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
                vdesc.data = vdata->tangents.data;
                vdesc.size = vdata->tangents.size;
                prim.stream.tangents = gs_graphics_vertex_buffer_create(&vdesc);
                if (!desc->keep_data)
                { 
                    gs_free(vdata->tangents.data);
                }
            }

            // Texcoords
            for (uint32_t j = 0; j < GS_GFXT_TEX_COORD_MAX; ++j)
            {
                if (vdata->tex_coords[j].data) 
                {
                    gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
                    vdesc.data = vdata->tex_coords[j].data;
                    vdesc.size = vdata->tex_coords[j].size;
                    prim.stream.tex_coords[j] = gs_graphics_vertex_buffer_create(&vdesc);
                    if (!desc->keep_data)
                    { 
                        gs_free(vdata->tex_coords[j].data);
                    }
                }
            }

            // Colors
            for (uint32_t j = 0; j < GS_GFXT_COLOR_MAX; ++j)
            {
                if (vdata->colors[j].data) 
                {
                    gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
                    vdesc.data = vdata->colors[j].data;
                    vdesc.size = vdata->colors[j].size;
                    prim.stream.colors[j] = gs_graphics_vertex_buffer_create(&vdesc);
                    if (!desc->keep_data)
                    { 
                        gs_free(vdata->colors[j].data);
                    }
                }
            }

            // Joints
            for (uint32_t j = 0; j < GS_GFXT_JOINT_MAX; ++j)
            {
                if (vdata->joints[j].data) 
                {
                    gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
                    vdesc.data = vdata->joints[j].data;
                    vdesc.size = vdata->joints[j].size;
                    prim.stream.joints[j] = gs_graphics_vertex_buffer_create(&vdesc);
                    if (!desc->keep_data)
                    { 
                        gs_free(vdata->joints[j].data);
                    }
                }
            }

            // Weights
            for (uint32_t j = 0; j < GS_GFXT_WEIGHT_MAX; ++j)
            {
                if (vdata->weights[j].data) 
                {
                    gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
                    vdesc.data = vdata->weights[j].data;
                    vdesc.size = vdata->weights[j].size;
                    prim.stream.weights[j] = gs_graphics_vertex_buffer_create(&vdesc); 
                    if (!desc->keep_data)
                    { 
                        gs_free(vdata->weights[j].data);
                    }
                }
            }

            // Index buffer decl
            gs_graphics_index_buffer_desc_t idesc = gs_default_val();
            idesc.data = vdata->indices.data;
            idesc.size = vdata->indices.size;

            // Construct index buffer for primitive
            prim.indices = gs_graphics_index_buffer_create(&idesc);

			if (!desc->keep_data)
			{
				gs_free(vdata->indices.data);
			}

            // Add primitive to mesh
            gs_dyn_array_push(mesh.primitives, prim); 
        } 

        if (!desc->keep_data)
        { 
            gs_dyn_array_free(m->primitives);
        }
    }

    if (!desc->keep_data)
    {
        gs_free(desc->meshes);
    }

    return mesh;
}

GS_API_DECL void 
gs_gfxt_mesh_update_or_create(gs_gfxt_mesh_t* mesh, const gs_gfxt_mesh_desc_t* desc)
{
    if (!desc || !mesh) {
        return;
    }

    /*
    // Need to create mesh if not already done 
    if (gs_dyn_array_empty(mesh->primitives)) { 
        *mesh = gs_gfxt_mesh_create(desc);
        return;
    } 
    */

    const uint32_t mesh_count = desc->size / sizeof(gs_gfxt_mesh_raw_data_t);

    // Process all mesh data, add meshes
    for (uint32_t i = 0; i < mesh_count; ++i)
    {
        gs_gfxt_mesh_raw_data_t* m = &desc->meshes[i];

        for (uint32_t p = 0; p < gs_dyn_array_size(m->primitives); ++p)
        {
            // Get raw vertex data
            gs_gfxt_mesh_vertex_data_t* vdata = &m->primitives[p];

            // Construct or retrieve mesh primitive
            gs_gfxt_mesh_primitive_t* prim = NULL;
            if (gs_dyn_array_empty(mesh->primitives) || gs_dyn_array_size(mesh->primitives) < p)
            { 
                gs_gfxt_mesh_primitive_t dprim = gs_default_val(); 
                gs_dyn_array_push(mesh->primitives, dprim);
            }
            prim = &mesh->primitives[p];

            // Set prim count
            prim->count = vdata->count;

            // Positions
            if (vdata->positions.data) 
            {
                gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
                vdesc.data = vdata->positions.data;
                vdesc.size = vdata->positions.size;

                // Update
                if (prim->stream.positions.id) 
                {
                    gs_graphics_vertex_buffer_update(prim->stream.positions, &vdesc); 
                }
                // Create
                else 
                {
                    prim->stream.positions = gs_graphics_vertex_buffer_create(&vdesc);
                }
                if (!desc->keep_data)
                { 
                    gs_free(vdata->positions.data);
                }
            }

            // Normals
            if (vdata->normals.data) 
            {
                gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
                vdesc.data = vdata->normals.data;
                vdesc.size = vdata->normals.size;

                // Update
                if (prim->stream.normals.id) 
                { 
                    gs_graphics_vertex_buffer_update(prim->stream.normals, &vdesc);
                }
                else
                { 
                    prim->stream.normals = gs_graphics_vertex_buffer_create(&vdesc);
                }
                if (!desc->keep_data)
                { 
                    gs_free(vdata->normals.data);
                }
            }

            // Tangents
            if (vdata->tangents.data) 
            {
                gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
                vdesc.data = vdata->tangents.data;
                vdesc.size = vdata->tangents.size;

                if (prim->stream.tangents.id) 
                { 
                    gs_graphics_vertex_buffer_update(prim->stream.tangents, &vdesc);
                } 
                else
                { 
                    prim->stream.tangents = gs_graphics_vertex_buffer_create(&vdesc);
                }
                if (!desc->keep_data)
                { 
                    gs_free(vdata->tangents.data);
                }
            }

            // Texcoords
            for (uint32_t j = 0; j < GS_GFXT_TEX_COORD_MAX; ++j)
            {
                if (vdata->tex_coords[j].data) 
                {
                    gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
                    vdesc.data = vdata->tex_coords[j].data;
                    vdesc.size = vdata->tex_coords[j].size;

                    if (prim->stream.tex_coords[j].id)
                    { 
                        gs_graphics_vertex_buffer_update(prim->stream.tex_coords[j], &vdesc);
                    }
                    else
                    { 
                        prim->stream.tex_coords[j] = gs_graphics_vertex_buffer_create(&vdesc);
                    }
                    if (!desc->keep_data)
                    { 
                        gs_free(vdata->tex_coords[j].data);
                    }
                }
            }

            // Colors
            for (uint32_t j = 0; j < GS_GFXT_COLOR_MAX; ++j)
            {
                if (vdata->colors[j].data) 
                {
                    gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
                    vdesc.data = vdata->colors[j].data;
                    vdesc.size = vdata->colors[j].size; 
                    
                    if (prim->stream.colors[j].id)
                    { 
                        gs_graphics_vertex_buffer_update(prim->stream.colors[j], &vdesc); 
                    }
                    else
                    {
                        prim->stream.colors[j] = gs_graphics_vertex_buffer_create(&vdesc); 
                    }
                    if (!desc->keep_data)
                    { 
                        gs_free(vdata->colors[j].data);
                    }
                }
            }

            // Joints
            for (uint32_t j = 0; j < GS_GFXT_JOINT_MAX; ++j)
            {
                if (vdata->joints[j].data) 
                {
                    gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
                    vdesc.data = vdata->joints[j].data;
                    vdesc.size = vdata->joints[j].size;

                    if (prim->stream.joints[j].id)
                    {
                        gs_graphics_vertex_buffer_update(prim->stream.joints[j], &vdesc); 
                    }
                    else
                    { 
                        prim->stream.joints[j] = gs_graphics_vertex_buffer_create(&vdesc); 
                    }
                    if (!desc->keep_data)
                    { 
                        gs_free(vdata->joints[j].data);
                    }
                }
            }

            // Weights
            for (uint32_t j = 0; j < GS_GFXT_WEIGHT_MAX; ++j)
            {
                if (vdata->weights[j].data) 
                {
                    gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
                    vdesc.data = vdata->weights[j].data;
                    vdesc.size = vdata->weights[j].size;

                    if (prim->stream.weights[j].id)
                    { 
                        gs_graphics_vertex_buffer_update(prim->stream.weights[j], &vdesc); 
                    }
                    else
                    { 
                        prim->stream.weights[j] = gs_graphics_vertex_buffer_create(&vdesc); 
                    }
                    if (!desc->keep_data)
                    { 
                        gs_free(vdata->weights[j].data);
                    }
                }
            }

            // Custom uint
            for (uint32_t j = 0; j < GS_GFXT_CUSTOM_UINT_MAX; ++j)
            {
                if (vdata->custom_uint[j].data) 
                {
                    gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
                    vdesc.data = vdata->custom_uint[j].data;
                    vdesc.size = vdata->custom_uint[j].size;

                    if (prim->stream.custom_uint[j].id)
                    { 
                        gs_graphics_vertex_buffer_update(prim->stream.custom_uint[j], &vdesc); 
                    }
                    else
                    { 
                        prim->stream.custom_uint[j] = gs_graphics_vertex_buffer_create(&vdesc); 
                    }
                    if (!desc->keep_data)
                    { 
                        gs_free(vdata->custom_uint[j].data);
                    }
                }
            }

            // Index buffer decl
            gs_graphics_index_buffer_desc_t idesc = gs_default_val();
            idesc.data = vdata->indices.data;
            idesc.size = vdata->indices.size;

            // Construct index buffer for primitive
            if (prim->indices.id)
            {
                gs_graphics_index_buffer_update(prim->indices, &idesc); 
            }
            else
            { 
                prim->indices = gs_graphics_index_buffer_create(&idesc); 
            }

			if (!desc->keep_data)
			{
				gs_free(vdata->indices.data);
			}
        } 

        if (!desc->keep_data)
        { 
            gs_dyn_array_free(m->primitives);
        }
    }

    if (!desc->keep_data)
    {
        gs_free(desc->meshes);
    }
}

GS_API_DECL gs_gfxt_renderable_t 
gs_gfxt_renderable_create(const gs_gfxt_renderable_desc_t* desc)
{
    gs_gfxt_renderable_t rend = gs_default_val();

    if (!desc) {
        return rend;
    }

    rend.model_matrix = gs_mat4_identity();
    rend.desc = *desc;

    return rend;
}

//=== Destruction ===// 
GS_API_DECL void 
gs_gfxt_texture_destroy(gs_gfxt_texture_t* texture)
{
    gs_graphics_texture_destroy(*texture);
}

GS_API_DECL void 
gs_gfxt_material_destroy(gs_gfxt_material_t* material)
{
    // Destroy all material data
    gs_byte_buffer_free(&material->uniform_data);
    gs_byte_buffer_free(&material->image_buffer_data);
}

GS_API_DECL void 
gs_gfxt_mesh_destroy(gs_gfxt_mesh_t* mesh)
{
    // Iterate through all primitives, destroy all vertex and index buffers
    for (uint32_t p = 0; p < gs_dyn_array_size(mesh->primitives); ++p)
    {
        gs_gfxt_mesh_primitive_t* prim = &mesh->primitives[p]; 

        // Free index buffer
		if (prim->indices.id) gs_graphics_index_buffer_destroy(prim->indices);

        // Free vertex stream
        if (prim->stream.positions.id) gs_graphics_vertex_buffer_destroy(prim->stream.positions);
        if (prim->stream.normals.id)   gs_graphics_vertex_buffer_destroy(prim->stream.normals);
        if (prim->stream.tangents.id)  gs_graphics_vertex_buffer_destroy(prim->stream.tangents);

		for (uint32_t i = 0; i < GS_GFXT_COLOR_MAX; ++i)
		{
			if (prim->stream.colors[i].id) gs_graphics_vertex_buffer_destroy(prim->stream.colors[i]);
		}

		for (uint32_t i = 0; i < GS_GFXT_TEX_COORD_MAX; ++i)
		{ 
            if (prim->stream.tex_coords[i].id) gs_graphics_vertex_buffer_destroy(prim->stream.tex_coords[i]);
		}

		for (uint32_t i = 0; i < GS_GFXT_JOINT_MAX; ++i)
		{ 
            if (prim->stream.joints[i].id) gs_graphics_vertex_buffer_destroy(prim->stream.joints[i]);
		}

		for ( uint32_t i = 0; i < GS_GFXT_WEIGHT_MAX; ++i )
		{ 
            if (prim->stream.weights[i].id) gs_graphics_vertex_buffer_destroy(prim->stream.weights[i]);
		}
    }
}

GS_API_DECL void 
gs_gfxt_uniform_block_destroy(gs_gfxt_uniform_block_t* ub)
{ 
    for (uint32_t i = 0; i < gs_dyn_array_size(ub->uniforms); ++i)
    {
        gs_gfxt_uniform_t* u = &ub->uniforms[i];
        gs_graphics_uniform_destroy(u->hndl);
    }

    gs_dyn_array_free(ub->uniforms);
    gs_hash_table_free(ub->lookup);
}

GS_API_DECL void 
gs_gfxt_pipeline_destroy(gs_gfxt_pipeline_t* pipeline)
{ 
    // Destroy uniform block for pipeline
    gs_gfxt_uniform_block_destroy(&pipeline->ublock);

	// Free shaders (if responsible for them)
    gs_graphics_shader_destroy(pipeline->desc.raster.shader);
    
    // Destroy pipeline
	if (pipeline->desc.layout.attrs) gs_free(pipeline->desc.layout.attrs);
    if (pipeline->mesh_layout) gs_dyn_array_free(pipeline->mesh_layout);
    gs_graphics_pipeline_destroy(pipeline->hndl); 
}

//=== Copy API ===//

GS_API_DECL gs_gfxt_material_t gs_gfxt_material_deep_copy(gs_gfxt_material_t* src)
{
    gs_gfxt_material_t mat = gs_gfxt_material_create(&src->desc);
    gs_byte_buffer_copy_contents(&mat.uniform_data, &src->uniform_data);
    return mat;
}

//=== Pipeline API ===//
GS_API_DECL gs_gfxt_uniform_t* 
gs_gfxt_pipeline_get_uniform(gs_gfxt_pipeline_t* pip, const char* name)
{
    uint64_t key = gs_hash_str64(name);
    if (!gs_hash_table_exists(pip->ublock.lookup, key)) {
        return NULL;
    }
    // Based on name, need to get uniform
    uint32_t uidx = gs_hash_table_get(pip->ublock.lookup, key);
    return &pip->ublock.uniforms[uidx];
}

//=== Material API ===//

GS_API_DECL
void gs_gfxt_material_set_uniform(gs_gfxt_material_t* mat, const char* name, const void* data)
{
    if (!mat || !name || !data) return;

    gs_gfxt_pipeline_t* pip = GS_GFXT_RAW_DATA(&mat->desc.pip_func, gs_gfxt_pipeline_t);
    gs_assert(pip);

    // Get key for name lookup
    uint64_t key = gs_hash_str64(name);
    if (!gs_hash_table_exists(pip->ublock.lookup, key)) {
        gs_timed_action(60, {
            gs_log_warning("Unable to find uniform: %s", name);
        });
        return;
    }

    // Based on name, need to get uniform
    uint32_t uidx = gs_hash_table_get(pip->ublock.lookup, key);
    gs_gfxt_uniform_t* u = &pip->ublock.uniforms[uidx];

    // Seek to beginning of data
    gs_byte_buffer_seek_to_beg(&mat->uniform_data); 
    gs_byte_buffer_seek_to_beg(&mat->image_buffer_data);    

    // Advance by offset
    switch (u->type)
    {
        case GS_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F:   gs_byte_buffer_advance_position(&mat->image_buffer_data, u->offset); break;
        default:                                    gs_byte_buffer_advance_position(&mat->uniform_data, u->offset); break; 
    }

    switch (u->type)
    {
        case GS_GRAPHICS_UNIFORM_FLOAT: gs_byte_buffer_write(&mat->uniform_data, float, *(float*)data);     break;
        case GS_GRAPHICS_UNIFORM_INT:   gs_byte_buffer_write(&mat->uniform_data, int32_t, *(int32_t*)data); break;
        case GS_GRAPHICS_UNIFORM_VEC2:  gs_byte_buffer_write(&mat->uniform_data, gs_vec2, *(gs_vec2*)data); break;
        case GS_GRAPHICS_UNIFORM_VEC3:  gs_byte_buffer_write(&mat->uniform_data, gs_vec3, *(gs_vec3*)data); break;
        case GS_GRAPHICS_UNIFORM_VEC4:  gs_byte_buffer_write(&mat->uniform_data, gs_vec4, *(gs_vec4*)data); break;
        case GS_GRAPHICS_UNIFORM_MAT4:  gs_byte_buffer_write(&mat->uniform_data, gs_mat4, *(gs_mat4*)data); break;

		case GS_GRAPHICS_UNIFORM_SAMPLERCUBE:
        case GS_GRAPHICS_UNIFORM_SAMPLER2D:
        case GS_GRAPHICS_UNIFORM_USAMPLER2D:
		{
            gs_byte_buffer_write(&mat->uniform_data, gs_handle(gs_graphics_texture_t), *(gs_handle(gs_graphics_texture_t)*)data);
        } break;

        case GS_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F: {
            gs_byte_buffer_write(&mat->image_buffer_data, gs_handle(gs_graphics_texture_t), *(gs_handle(gs_graphics_texture_t)*)data);
        } break;
    }
}

GS_API_DECL gs_gfxt_pipeline_t* gs_gfxt_material_get_pipeline(gs_gfxt_material_t* mat)
{
    gs_gfxt_pipeline_t* pip = GS_GFXT_RAW_DATA(&mat->desc.pip_func, gs_gfxt_pipeline_t);
    return pip;
}

GS_API_DECL
void gs_gfxt_material_bind(gs_command_buffer_t* cb, gs_gfxt_material_t* mat)
{
    gs_gfxt_material_bind_pipeline(cb, mat);
    gs_gfxt_material_bind_uniforms(cb, mat);
}

GS_API_DECL
void gs_gfxt_material_bind_pipeline(gs_command_buffer_t* cb, gs_gfxt_material_t* mat)
{
    // Binds the pipeline
    gs_gfxt_pipeline_t* pip = GS_GFXT_RAW_DATA(&mat->desc.pip_func, gs_gfxt_pipeline_t);
    gs_assert(pip);
    gs_graphics_pipeline_bind(cb, pip->hndl);
}

GS_API_DECL 
void gs_gfxt_material_bind_uniforms(gs_command_buffer_t* cb, gs_gfxt_material_t* mat)
{
    if (!mat) return;

    gs_gfxt_pipeline_t* pip = GS_GFXT_RAW_DATA(&mat->desc.pip_func, gs_gfxt_pipeline_t);
    gs_assert(pip);

    // Grab uniform layout from pipeline
    for (uint32_t i = 0; i < gs_dyn_array_size(pip->ublock.uniforms); ++i) 
    { 
        gs_gfxt_uniform_t* u = &pip->ublock.uniforms[i];
        gs_graphics_bind_desc_t bind = gs_default_val(); 

        // Need to buffer these up so it's a single call...
        switch (u->type)
        {
            case GS_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F:
            {
                gs_graphics_bind_image_buffer_desc_t ibuffer[1];
                ibuffer[0].tex = *(gs_handle(gs_graphics_texture_t)*)(mat->image_buffer_data.data + u->offset);
                ibuffer[0].binding = u->binding;
                ibuffer[0].access = GS_GRAPHICS_ACCESS_WRITE_ONLY;
                bind.image_buffers.desc = ibuffer;
                bind.image_buffers.size = sizeof(ibuffer);
                gs_graphics_apply_bindings(cb, &bind);
            } break;

            default:
            {
                gs_graphics_bind_uniform_desc_t uniforms[1];
                uniforms[0].uniform = u->hndl;
                uniforms[0].data = (mat->uniform_data.data + u->offset);
                uniforms[0].binding = u->binding;
                bind.uniforms.desc = uniforms;
                bind.uniforms.size = sizeof(uniforms); 
                gs_graphics_apply_bindings(cb, &bind);
            } break;
        }

    }
}

// Mesh API
GS_API_DECL void 
gs_gfxt_mesh_draw(gs_command_buffer_t* cb, gs_gfxt_mesh_t* mp)
{
    /*
    // For each primitive in mesh
    for (uint32_t i = 0; i < gs_dyn_array_size(mp->primitives); ++i)
    {
        gs_gfxt_mesh_primitive_t* prim = &mp->primitives[i];

        // Bindings for all buffers: vertex, index, uniform, sampler
        gs_graphics_bind_desc_t binds = gs_default_val();
        gs_graphics_bind_vertex_buffer_desc_t vdesc = gs_default_val();
        gs_graphics_bind_index_buffer_desc_t idesc = gs_default_val();
        vdesc.buffer = prim->vbo;
        idesc.buffer = prim->indices;
        binds.vertex_buffers.desc = &vdesc;
        binds.index_buffers.desc = &idesc;
        gs_graphics_draw_desc_t ddesc = gs_default_val();
        ddesc.start = 0;
        ddesc.count = prim->count;
        gs_graphics_apply_bindings(cb, &binds);
        gs_graphics_draw(cb, &ddesc);
    }
    */
}

GS_API_DECL void
gs_gfxt_mesh_primitive_draw_layout(gs_command_buffer_t* cb, gs_gfxt_mesh_primitive_t* prim, gs_gfxt_mesh_layout_t* layout, size_t layout_size, uint32_t instance_count)
{ 
    if (!layout || !layout_size || !prim || !cb)
    {
        return;
    }

    gs_graphics_bind_vertex_buffer_desc_t vbos[8] = {0};     // Make this a define
    uint32_t l = 0;
    const uint32_t ct = layout_size / sizeof(gs_gfxt_mesh_layout_t);
    for (uint32_t a = 0; a < ct; ++a)
    {
        vbos[l].data_type = GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED;
        switch (layout[a].type)
        {
            case GS_ASSET_MESH_ATTRIBUTE_TYPE_POSITION: {if (!prim->stream.positions.id) continue; vbos[l].buffer = prim->stream.positions;} break;
            case GS_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL:   {if (!prim->stream.normals.id) continue; vbos[l].buffer = prim->stream.normals;} break;
            case GS_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT:  {if (!prim->stream.tangents.id) continue; vbos[l].buffer = prim->stream.tangents;} break;
            case GS_ASSET_MESH_ATTRIBUTE_TYPE_JOINT:    {if (!prim->stream.joints[0].id) continue; vbos[l].buffer = prim->stream.joints[0];} break;
            case GS_ASSET_MESH_ATTRIBUTE_TYPE_WEIGHT:   {if (!prim->stream.weights[0].id) continue; vbos[l].buffer = prim->stream.weights[0];} break;
            case GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD: {if (!prim->stream.tex_coords[0].id) continue; vbos[l].buffer = prim->stream.tex_coords[0];} break;
            case GS_ASSET_MESH_ATTRIBUTE_TYPE_COLOR:    {if (!prim->stream.colors[0].id) continue; vbos[l].buffer = prim->stream.colors[0];} break;
            case GS_ASSET_MESH_ATTRIBUTE_TYPE_UINT:     {if (!prim->stream.custom_uint[0].id) continue; vbos[l].buffer = prim->stream.custom_uint[0];} break;
        }
        ++l;
    }

    gs_graphics_bind_index_buffer_desc_t ibos = gs_default_val();
    ibos.buffer = prim->indices;

    // Bindings for all buffers: vertex, index, uniform, sampler
    gs_graphics_bind_desc_t binds = gs_default_val();

        // .vertex_buffers = {.desc = vbos, .size = sizeof(vbos)},
    binds.vertex_buffers.desc = vbos;
    binds.vertex_buffers.size = l * sizeof(gs_graphics_bind_vertex_buffer_desc_t); 
    binds.index_buffers.desc = &ibos;

    gs_graphics_draw_desc_t ddesc = gs_default_val();
    ddesc.start = 0;
    ddesc.count = prim->count;
    ddesc.instances = instance_count;

    gs_graphics_apply_bindings(cb, &binds);
    gs_graphics_draw(cb, &ddesc);
}

GS_API_DECL void 
gs_gfxt_mesh_draw_layout(gs_command_buffer_t* cb, gs_gfxt_mesh_t* mesh, gs_gfxt_mesh_layout_t* layout, size_t layout_size)
{
    if (!layout || !mesh || !cb)
    {
        return;
    }

    uint32_t ct = layout_size / sizeof(gs_gfxt_mesh_layout_t);

    // For each primitive in mesh
    for (uint32_t i = 0; i < gs_dyn_array_size(mesh->primitives); ++i)
    {
        gs_gfxt_mesh_primitive_t* prim = &mesh->primitives[i]; 
        gs_gfxt_mesh_primitive_draw_layout(cb, prim, layout, layout_size, 1);
    }
}

GS_API_DECL void 
gs_gfxt_mesh_draw_materials(gs_command_buffer_t* cb, gs_gfxt_mesh_t* mesh, gs_gfxt_material_t** mats, size_t mats_size)
{
    // Iterate through primitives, draw each primitive with assigned mat
    if (!mats || !mats_size || !cb || !mesh)
    {
        return;
    }

    const uint32_t ct = mats_size / sizeof(gs_gfxt_material_t*);
    gs_gfxt_material_t* mat = NULL;

    // For each primitive in mesh
    for (uint32_t i = 0; i < gs_dyn_array_size(mesh->primitives); ++i)
    {
        gs_gfxt_mesh_primitive_t* prim = &mesh->primitives[i]; 

        // Get corresponding material, if available
        uint32_t mat_idx = i < ct ? i : ct - 1;
        mat = mats[mat_idx] ? mats[mat_idx] : mat;

        // Can't draw without a valid material present
        if (!mat) continue;

        // Bind material pipeline and uniforms
        gs_gfxt_material_bind(cb, mat);

        // Get pipeline
        gs_gfxt_pipeline_t* pip = gs_gfxt_material_get_pipeline(mat);

        gs_gfxt_mesh_primitive_draw_layout(cb, prim, pip->mesh_layout, gs_dyn_array_size(pip->mesh_layout) * sizeof(gs_gfxt_mesh_layout_t), 1);
    } 
} 

GS_API_DECL void 
gs_gfxt_mesh_draw_material(gs_command_buffer_t* cb, gs_gfxt_mesh_t* mesh, gs_gfxt_material_t* mat)
{ 
    if (!mat || !mesh || !cb)
    {
        return;
    }

    gs_gfxt_pipeline_t* pip = gs_gfxt_material_get_pipeline(mat);
    gs_gfxt_mesh_draw_layout(cb, mesh, pip->mesh_layout, gs_dyn_array_size(pip->mesh_layout) * sizeof(gs_gfxt_mesh_layout_t));
}

GS_API_DECL void 
gs_gfxt_mesh_draw_pipeline(gs_command_buffer_t* cb, gs_gfxt_mesh_t* mesh, gs_gfxt_pipeline_t* pip)
{
    if (!pip || !mesh || !cb)
    {
        return;
    }

    gs_gfxt_mesh_draw_layout(cb, mesh, pip->mesh_layout, gs_dyn_array_size(pip->mesh_layout) * sizeof(gs_gfxt_mesh_layout_t));
}

// Util API
GS_API_DECL
void* gs_gfxt_raw_data_default_impl(GS_GFXT_HNDL hndl, void* user_data)
{
    return hndl;
}

GS_API_DECL void gs_gfxt_mesh_import_options_free(gs_gfxt_mesh_import_options_t* opt)
{
    if (opt->layout)
    {
        gs_dyn_array_free(opt->layout);
    }
}

GS_API_DECL 
gs_gfxt_mesh_t gs_gfxt_mesh_load_from_file(const char* path, gs_gfxt_mesh_import_options_t* options)
{
    gs_gfxt_mesh_t mesh = gs_default_val();

    if (!gs_platform_file_exists(path)) {
        gs_println("Warning:GFXT:MeshLoadFromFile:File does not exist: %s", path);
        return mesh;
    }

    // Mesh data to fill out
    uint32_t mesh_count = 0;
    gs_gfxt_mesh_raw_data_t* meshes = NULL;

    // Get file extension from path
    gs_transient_buffer(file_ext, 32);
    gs_platform_file_extension(file_ext, 32, path);

    // GLTF
    if (gs_string_compare_equal(file_ext, "gltf")) {
        gs_gfxt_load_gltf_data_from_file(path, options, &meshes, &mesh_count);
    }
    // GLB
    else if (gs_string_compare_equal(file_ext, "glb")) {
        gs_gfxt_load_gltf_data_from_file(path, options, &meshes, &mesh_count);
    }
    else {
        gs_println("Warning:GFXT:MeshLoadFromFile:File extension not supported: %s, file: %s", file_ext, path);
        return mesh;
    }

    gs_gfxt_mesh_desc_t mdesc = gs_default_val();
    mdesc.meshes = meshes;
    mdesc.size = mesh_count * sizeof(gs_gfxt_mesh_raw_data_t);

    mesh = gs_gfxt_mesh_create(&mdesc);
    mesh.desc = mdesc;

    return mesh;
}

GS_API_DECL bool 
gs_gfxt_load_gltf_data_from_file(const char* path, gs_gfxt_mesh_import_options_t* options, 
    gs_gfxt_mesh_raw_data_t** out, uint32_t* mesh_count)
{
    // Use cgltf like a boss
    cgltf_options cgltf_options = gs_default_val();
    size_t len = 0;
    char* file_data = NULL;

    // Get file extension from path
    gs_transient_buffer(file_ext, 32);
    gs_platform_file_extension(file_ext, 32, path);

    // GLTF
    if (gs_string_compare_equal(file_ext, "gltf")) {
        file_data = gs_platform_read_file_contents(path, "rb", &len);
        gs_println("GFXT:Loading GLTF: %s", path);
    }
    // GLB
    else if (gs_string_compare_equal(file_ext, "glb")) {
        file_data = gs_platform_read_file_contents(path, "rb", &len);
        gs_println("GFXT:Loading GLTF: %s", path);
    }
    else {
        gs_println("Warning:GFXT:LoadGLTFDataFromFile:File extension not supported: %s, file: %s", file_ext, path);
        return false;
    }

    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse(&cgltf_options, file_data, (cgltf_size)len, &data);
    gs_free(file_data);

    if (result != cgltf_result_success) {
        gs_println("GFXT:Mesh:LoadFromFile:Failed load gltf");
        cgltf_free(data);
        return false;
    }

    // Load buffers as well
    result = cgltf_load_buffers(&cgltf_options, data, path);
    if (result != cgltf_result_success) {
        cgltf_free(data);
        gs_println("GFXT:Mesh:LoadFromFile:Failed to load buffers");
        return false;
    }

    // Type of index data
    size_t index_element_size = options ? options->index_buffer_element_size : 0;

    // Temporary structures
    gs_dyn_array(gs_vec3) positions = NULL;
    gs_dyn_array(gs_vec3) normals = NULL;
    gs_dyn_array(gs_vec3) tangents = NULL;
    gs_dyn_array(gs_color_t) colors[GS_GFXT_COLOR_MAX] = gs_default_val();
    gs_dyn_array(gs_vec2) uvs[GS_GFXT_TEX_COORD_MAX] = gs_default_val();
    gs_dyn_array(float) weights[GS_GFXT_WEIGHT_MAX] = gs_default_val();
    gs_dyn_array(float) joints[GS_GFXT_JOINT_MAX] = gs_default_val();
    gs_dyn_array(gs_gfxt_mesh_layout_t) layouts = gs_default_val();
    gs_byte_buffer_t v_data = gs_byte_buffer_new();
    gs_byte_buffer_t i_data = gs_byte_buffer_new();
    gs_mat4 world_mat = gs_mat4_identity();

    // Allocate memory for buffers
    *mesh_count = data->meshes_count;
    *out = (gs_gfxt_mesh_raw_data_t*)gs_malloc(data->meshes_count * sizeof(gs_gfxt_mesh_raw_data_t));
    memset(*out, 0, sizeof(gs_gfxt_mesh_raw_data_t) * data->meshes_count);

    // For each node, for each mesh
    uint32_t i = 0;
    for (uint32_t _n = 0; _n < data->nodes_count; ++_n)
    {
        cgltf_node* node = &data->nodes[_n];
        if (node->mesh == NULL) continue;

        gs_println("Load mesh from node: %s", node->name);

        // Reset matrix
        world_mat = gs_mat4_identity();

        // gs_println("i: %zu, r: %zu, t: %zu, s: %zu, m: %zu", i, node->has_rotation, node->has_translation, node->has_scale, node->has_matrix);

        // Not sure what "local transform" does, since world gives me the actual world result...probably for animation
        if (node->has_rotation || node->has_translation || node->has_scale) 
        {
            cgltf_node_transform_world(node, (float*)&world_mat);
        }

        // if (node->has_matrix)
        // {
        //  // Multiply local by world
        //  gs_mat4 tmp = gs_default_val();
        //  cgltf_node_transform_world(node, (float*)&tmp);
        //  world_mat = gs_mat4_mul(world_mat, tmp);
        // }

        
        // Do node mesh data
        cgltf_mesh* cmesh = node->mesh;
        {
            // Initialize mesh data
            gs_gfxt_mesh_raw_data_t* mesh = &((*out)[i]);
            bool warnings[gs_enum_count(gs_asset_mesh_attribute_type)] = gs_default_val();
            bool printed = false;

            // For each primitive in mesh 
            for (uint32_t p = 0; p < cmesh->primitives_count; ++p)
            {
                cgltf_primitive* prim = &cmesh->primitives[p];

                // Mesh primitive to fill out
                gs_gfxt_mesh_vertex_data_t primitive = gs_default_val();

                // Clear temp data from previous use
                gs_dyn_array_clear(positions);
                gs_dyn_array_clear(normals);
                gs_dyn_array_clear(tangents);
                for (uint32_t ci = 0; ci < GS_GFXT_COLOR_MAX; ++ci) gs_dyn_array_clear(colors[ci]);
                for (uint32_t tci = 0; tci < GS_GFXT_TEX_COORD_MAX; ++tci) gs_dyn_array_clear(uvs[tci]);
                for (uint32_t wi = 0; wi < GS_GFXT_WEIGHT_MAX; ++wi) gs_dyn_array_clear(weights[wi]);
                for (uint32_t ji = 0; ji < GS_GFXT_JOINT_MAX; ++ji) gs_dyn_array_clear(joints[ji]);
                gs_dyn_array_clear(layouts);
                gs_byte_buffer_clear(&v_data);
                gs_byte_buffer_clear(&i_data);

                // Collect all provided attribute data for each vertex that's available in gltf data
                #define __GFXT_GLTF_PUSH_ATTR(ATTR, TYPE, COUNT, ARR, ARR_TYPE, LAYOUTS, LAYOUT_TYPE)\
                    do {\
                        int32_t N = 0;\
                        TYPE* BUF = (TYPE*)ATTR->buffer_view->buffer->data + ATTR->buffer_view->offset/sizeof(TYPE) + ATTR->offset/sizeof(TYPE);\
                        gs_assert(BUF);\
                        TYPE V[COUNT] = gs_default_val();\
                        /* For each vertex */\
                        for (uint32_t k = 0; k < ATTR->count; k++)\
                        {\
                            /* For each element */\
                            for (int l = 0; l < COUNT; l++) {\
                                V[l] = BUF[N + l];\
                            }\
                            N += (int32_t)(ATTR->stride/sizeof(TYPE));\
                            /* Add to temp data array */\
                            ARR_TYPE ELEM = gs_default_val();\
                            memcpy((void*)&ELEM, (void*)V, sizeof(ARR_TYPE));\
                            gs_dyn_array_push(ARR, ELEM);\
                        }\
                        /* Push into layout */\
                        gs_gfxt_mesh_layout_t LAYOUT = gs_default_val();\
                        LAYOUT.type = LAYOUT_TYPE;\
                        gs_dyn_array_push(LAYOUTS, LAYOUT);\
                    } while (0)

                // For each attribute in primitive
                for (uint32_t a = 0; a < prim->attributes_count; ++a)
                {
                    // Accessor for attribute data
                    cgltf_accessor* attr = prim->attributes[a].data;

                    // Index for data
                    int32_t aidx = prim->attributes[a].index;

                    // Switch on type for reading data
                    switch (prim->attributes[a].type)
                    {
                        case cgltf_attribute_type_position: {
                            int32_t N = 0;
                            float* BUF = (float*)attr->buffer_view->buffer->data + attr->buffer_view->offset/sizeof(float) + attr->offset/sizeof(float);
                            gs_assert(BUF);
                            float V[3] = gs_default_val();
                            /* For each vertex */
                            for (uint32_t k = 0; k < attr->count; k++)
                            {
                                /* For each element */
                                for (int l = 0; l < 3; l++) {
                                    V[l] = BUF[N + l];
                                } 
                                N += (int32_t)(attr->stride/sizeof(float));
                                /* Add to temp data array */
                                gs_vec3 ELEM = gs_default_val();
                                memcpy((void*)&ELEM, (void*)V, sizeof(gs_vec3));
                                // Transform into world space
                                ELEM = gs_mat4_mul_vec3(world_mat, ELEM);
                                gs_dyn_array_push(positions, ELEM);
                            }
                            /* Push into layout */
                            gs_gfxt_mesh_layout_t LAYOUT = gs_default_val();
                            LAYOUT.type = GS_ASSET_MESH_ATTRIBUTE_TYPE_POSITION;
                            gs_dyn_array_push(layouts, LAYOUT);
                        } break;

                        case cgltf_attribute_type_normal: {
                            __GFXT_GLTF_PUSH_ATTR(attr, float, 3, normals, gs_vec3, layouts, GS_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL);
                        } break;

                        case cgltf_attribute_type_tangent: {
                            __GFXT_GLTF_PUSH_ATTR(attr, float, 3, tangents, gs_vec3, layouts, GS_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT);
                        } break;

                        case cgltf_attribute_type_texcoord: {
                            __GFXT_GLTF_PUSH_ATTR(attr, float, 2, uvs[aidx], gs_vec2, layouts, GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD);
                        } break;

                        case cgltf_attribute_type_color: {
                            // Need to parse color as sRGB then convert to gs_color_t
                            int32_t N = 0;
                            float* BUF = (float*)attr->buffer_view->buffer->data + attr->buffer_view->offset/sizeof(float) + attr->offset/sizeof(float);
                            gs_assert(BUF);
                            float V[3] = gs_default_val();
                            /* For each vertex */\
                            for (uint32_t k = 0; k < attr->count; k++)
                            {
                                /* For each element */
                                for (int l = 0; l < 3; l++) {
                                    V[l] = BUF[N + l];
                                }
                                N += (int32_t)(attr->stride/sizeof(float));
                                /* Add to temp data array */
                                gs_color_t ELEM = gs_default_val();
                                // Need to convert over now
                                ELEM.r = (uint8_t)(V[0] * 255.f);
                                ELEM.g = (uint8_t)(V[1] * 255.f);
                                ELEM.b = (uint8_t)(V[2] * 255.f);
                                ELEM.a = 255; 
                                gs_dyn_array_push(colors[aidx], ELEM);
                            }
                            /* Push into layout */
                            gs_gfxt_mesh_layout_t LAYOUT = gs_default_val();
                            LAYOUT.type = GS_ASSET_MESH_ATTRIBUTE_TYPE_COLOR;
                            gs_dyn_array_push(layouts, LAYOUT);
                        } break;

                        // Not sure what to do with these for now
                        case cgltf_attribute_type_joints: 
                        {
                            // Push into layout
                            gs_gfxt_mesh_layout_t layout = gs_default_val();
                            layout.type = GS_ASSET_MESH_ATTRIBUTE_TYPE_JOINT;
                            gs_dyn_array_push(layouts, layout);
                        } break;

                        case cgltf_attribute_type_weights:
                        {
                            // Push into layout
                            gs_gfxt_mesh_layout_t layout = gs_default_val();
                            layout.type = GS_ASSET_MESH_ATTRIBUTE_TYPE_WEIGHT;
                            gs_dyn_array_push(layouts, layout);
                        } break;

                        // Shouldn't hit here...   
                        default: {
                        } break;
                    }
                }

                // Indices for primitive
                cgltf_accessor* acc = prim->indices;

                #define __GFXT_GLTF_PUSH_IDX(BB, ACC, TYPE)\
                    do {\
                        int32_t n = 0;\
                        TYPE* buf = (TYPE*)acc->buffer_view->buffer->data + acc->buffer_view->offset/sizeof(TYPE) + acc->offset/sizeof(TYPE);\
                        gs_assert(buf);\
                        TYPE v = 0;\
                        /* For each index */\
                        for (uint32_t k = 0; k < acc->count; k++) {\
                            /* For each element */\
                            for (int l = 0; l < 1; l++) {\
                                v = buf[n + l];\
                            }\
                            n += (int32_t)(acc->stride/sizeof(TYPE));\
                            /* Add to temp positions array */\
                            switch (index_element_size) {\
                                case 0: gs_byte_buffer_write(BB, uint16_t, (uint16_t)v); break;\
                                case 2: gs_byte_buffer_write(BB, uint16_t, (uint16_t)v); break;\
                                case 4: gs_byte_buffer_write(BB, uint32_t, (uint32_t)v); break;\
                            }\
                        }\
                    } while (0)

                // If indices are available
                if (acc) 
                {
                    switch (acc->component_type) 
                    {
                        case cgltf_component_type_r_8:   __GFXT_GLTF_PUSH_IDX(&i_data, acc, int8_t);   break;
                        case cgltf_component_type_r_8u:  __GFXT_GLTF_PUSH_IDX(&i_data, acc, uint8_t);  break;
                        case cgltf_component_type_r_16:  __GFXT_GLTF_PUSH_IDX(&i_data, acc, int16_t);  break;
                        case cgltf_component_type_r_16u: __GFXT_GLTF_PUSH_IDX(&i_data, acc, uint16_t); break;
                        case cgltf_component_type_r_32u: __GFXT_GLTF_PUSH_IDX(&i_data, acc, uint32_t); break;
                        case cgltf_component_type_r_32f: __GFXT_GLTF_PUSH_IDX(&i_data, acc, float);    break;

                        // Shouldn't hit here
                        default: {
                        } break;
                    }
                }
                else 
                {
                    // Iterate over positions size, then just push back indices
                    for (uint32_t i = 0; i < gs_dyn_array_size(positions); ++i) 
                    {
                        switch (index_element_size)
                        {
                            default:
                            case 0: gs_byte_buffer_write(&i_data, uint16_t, (uint16_t)i); break;
                            case 2: gs_byte_buffer_write(&i_data, uint16_t, (uint16_t)i); break;
                            case 4: gs_byte_buffer_write(&i_data, uint32_t, (uint32_t)i); break;
                        }
                    }
                }

                // Grab mesh layout pointer to use
                /*
                gs_gfxt_mesh_layout_t* layoutp = options ? options->layout : layouts;
                uint32_t layout_ct = options ? options->size / sizeof(gs_gfxt_mesh_layout_t) : gs_dyn_array_size(layouts);

                // Iterate layout to fill data buffers according to provided layout
                {
                    uint32_t vct = 0; 
                    vct = gs_max(vct, gs_dyn_array_size(positions)); 
                    vct = gs_max(vct, gs_dyn_array_size(colors)); 
                    vct = gs_max(vct, gs_dyn_array_size(uvs));
                    vct = gs_max(vct, gs_dyn_array_size(normals));
                    vct = gs_max(vct, gs_dyn_array_size(tangents));

                    #define __GLTF_WRITE_DATA(IT, VDATA, ARR, ARR_TYPE, ARR_DEF_VAL, LAYOUT_TYPE)\
                        do {\
                            if (IT < gs_dyn_array_size(ARR)) {\
                                gs_byte_buffer_write(&(VDATA), ARR_TYPE, ARR[IT]);\
                            }\
                            else {\
                                gs_byte_buffer_write(&(VDATA), ARR_TYPE, ARR_DEF_VAL);\
                                if (!warnings[LAYOUT_TYPE]) {\
                                    warnings[LAYOUT_TYPE] = true;\
                                }\
                            }\
                        } while (0)

                    for (uint32_t it = 0; it < vct; ++it)
                    {
                        // For each attribute in layout
                        for (uint32_t l = 0; l < layout_ct; ++l)
                        {
                            switch (layoutp[l].type)
                            {
                                case GS_ASSET_MESH_ATTRIBUTE_TYPE_POSITION: {
                                    __GLTF_WRITE_DATA(it, v_data, positions, gs_vec3, gs_v3(0.f, 0.f, 0.f), GS_ASSET_MESH_ATTRIBUTE_TYPE_POSITION); 
                                } break;

                                case GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD: {
                                    __GLTF_WRITE_DATA(it, v_data, uvs, gs_vec2, gs_v2(0.f, 0.f), GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD); 
                                } break;

                                case GS_ASSET_MESH_ATTRIBUTE_TYPE_COLOR: {
                                    __GLTF_WRITE_DATA(it, v_data, colors, gs_color_t, GS_COLOR_WHITE, GS_ASSET_MESH_ATTRIBUTE_TYPE_COLOR); 
                                } break;

                                case GS_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL: {
                                    __GLTF_WRITE_DATA(it, v_data, normals, gs_vec3, gs_v3(0.f, 0.f, 1.f), GS_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL); 
                                } break;

                                case GS_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT: {
                                    __GLTF_WRITE_DATA(it, v_data, tangents, gs_vec3, gs_v3(0.f, 1.f, 0.f), GS_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT); 
                                } break;

                                default:
                                {
                                } break;
                            }
                        }
                    }
                }

                // Add to out data
                mesh->vertices[p] = gs_malloc(v_data.size);
                mesh->indices[p] = gs_malloc(i_data.size);
                mesh->vertex_sizes[p] = v_data.size;
                mesh->index_sizes[p] = i_data.size;

                // Copy data
                memcpy(mesh->vertices[p], v_data.data, v_data.size);
                memcpy(mesh->indices[p], i_data.data, i_data.size);
                */

                /*
                    typedef struct
                    {
                        void* data;
                        size_t size;
                    } gs_gfxt_mesh_vertex_attribute_t;

                    typedef struct 
                    {
                        gs_gfxt_mesh_vertex_attribute_t positions;         // All position data
                        gs_gfxt_mesh_vertex_attribute_t normals;
                        gs_gfxt_mesh_vertex_attribute_t tangents;
                        gs_gfxt_mesh_vertex_attribute_t tex_coords[GS_GFXT_TEX_COORD_MAX];
                        gs_gfxt_mesh_vertex_attribute_t joints[GS_GFXT_JOINT_MAX];
                        gs_gfxt_mesh_vertex_attribute_t weights[GS_GFXT_WEIGHT_MAX];
                        gs_gfxt_mesh_vertex_attribute_t indices;
                    } gs_gfxt_mesh_vertex_data_t;

                    // Structured/packed raw mesh data
                    typedef struct gs_gfxt_mesh_raw_data_t {
                        uint16_t prim_count;
                        size_t* vertex_sizes;
                        size_t* index_sizes;
                        void** vertices;
                        void** indices;

                        gs_dyn_array(gs_gfxt_mesh_vertex_data_t) primitives;   // All primitive data
                    } gs_gfxt_mesh_raw_data_t;
                */

                // Count
                primitive.count = prim->indices->count;

                // Indices
                primitive.indices.size = i_data.size;
                primitive.indices.data = gs_malloc(i_data.size);
                memcpy(primitive.indices.data, i_data.data, i_data.size);

                // Positions
                if (!gs_dyn_array_empty(positions))
                {
                    primitive.positions.size = gs_dyn_array_size(positions) * sizeof(gs_vec3);
                    primitive.positions.data = gs_malloc(primitive.positions.size);
                    memcpy(primitive.positions.data, positions, primitive.positions.size);
                }

                // Normals
                if (!gs_dyn_array_empty(normals))
                {
                    primitive.normals.size = gs_dyn_array_size(normals) * sizeof(gs_vec3);
                    primitive.normals.data = gs_malloc(primitive.normals.size);
                    memcpy(primitive.normals.data, normals, primitive.normals.size);
                }

                // Tangents
                if (!gs_dyn_array_empty(tangents))
                {
                    primitive.tangents.size = gs_dyn_array_size(tangents) * sizeof(gs_vec3);
                    primitive.tangents.data = gs_malloc(primitive.tangents.size);
                    memcpy(primitive.tangents.data, tangents, primitive.tangents.size);
                }

                // Texcoords
                for (uint32_t tci = 0; tci < GS_GFXT_TEX_COORD_MAX; ++tci)
                {
                    if (!gs_dyn_array_empty(uvs[tci]))
                    {
                        primitive.tex_coords[tci].size = gs_dyn_array_size(uvs[tci]) * sizeof(gs_vec2);
                        primitive.tex_coords[tci].data = gs_malloc(primitive.tex_coords[tci].size);
                        memcpy(primitive.tex_coords[tci].data, uvs[tci], primitive.tex_coords[tci].size);
                    }
                    else
                    {
                        break;
                    }
                }

                // Colors
                for (uint32_t ci = 0; ci < GS_GFXT_COLOR_MAX; ++ci)
                {
                    if (!gs_dyn_array_empty(colors[ci]))
                    {
                        primitive.colors[ci].size = gs_dyn_array_size(colors[ci]) * sizeof(gs_color_t);
                        primitive.colors[ci].data = gs_malloc(primitive.colors[ci].size);
                        memcpy(primitive.colors[ci].data, colors[ci], primitive.colors[ci].size);
                    }
                    else
                    {
                        break;
                    }
                }

                // Joints
                for (uint32_t ji = 0; ji < GS_GFXT_JOINT_MAX; ++ji)
                {
                    if (!gs_dyn_array_empty(joints[ji]))
                    {
                        primitive.joints[ji].size = gs_dyn_array_size(joints[ji]) * sizeof(float);
                        primitive.joints[ji].data = gs_malloc(primitive.joints[ji].size);
                        memcpy(primitive.joints[ji].data, joints[ji], primitive.joints[ji].size);
                    }
                    else
                    {
                        break;
                    }
                }

                // Weights
                for (uint32_t wi = 0; wi < GS_GFXT_WEIGHT_MAX; ++wi)
                {
                    if (!gs_dyn_array_empty(weights[wi]))
                    {
                        primitive.weights[wi].size = gs_dyn_array_size(weights[wi]) * sizeof(float);
                        primitive.weights[wi].data = gs_malloc(primitive.weights[wi].size);
                        memcpy(primitive.weights[wi].data, weights[wi], primitive.weights[wi].size);
                    }
                    else
                    {
                        break;
                    }
                }

                // Add primitive to mesh
                gs_dyn_array_push(mesh->primitives, primitive);
            } 

            if (!printed)
            {
                printed = true;
                if (warnings[GS_ASSET_MESH_ATTRIBUTE_TYPE_POSITION]){
                    gs_log_warning("Mesh attribute: POSITION not found. Resorting to default."); 
                }

                if (warnings[GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD]) {
                    gs_log_warning("Mesh attribute: TEXCOORD not found. Resorting to default."); 
                }

                if (warnings[GS_ASSET_MESH_ATTRIBUTE_TYPE_COLOR]) {
                    gs_log_warning("Mesh attribute: COLOR not found. Resorting to default."); 
                }

                if (warnings[GS_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL]) {
                    gs_log_warning("Mesh attribute: NORMAL not found. Resorting to default."); 
                }

                if (warnings[GS_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT]) {
                    gs_log_warning("Mesh attribute: WEIGHTS not found. Resorting to default."); 
                } 
            }
        }

        // Increment i if successful
        i++;
    }

    gs_println("Finished loading mesh.");

    // Free all data at the end
    cgltf_free(data);
    gs_dyn_array_free(positions);
    gs_dyn_array_free(normals);
    gs_dyn_array_free(tangents);
    for (uint32_t ci = 0; ci < GS_GFXT_COLOR_MAX; ++ci) gs_dyn_array_free(colors[ci]);
    for (uint32_t tci = 0; tci < GS_GFXT_TEX_COORD_MAX; ++tci) gs_dyn_array_free(uvs[tci]);
    for (uint32_t wi = 0; wi < GS_GFXT_WEIGHT_MAX; ++wi) gs_dyn_array_free(weights[wi]);
    for (uint32_t ji = 0; ji < GS_GFXT_JOINT_MAX; ++ji) gs_dyn_array_free(joints[ji]);
    gs_dyn_array_free(layouts);
    gs_byte_buffer_free(&v_data);
    gs_byte_buffer_free(&i_data);
    return true;
}

GS_API_DECL 
gs_gfxt_mesh_t gs_gfxt_mesh_unit_quad_generate(gs_gfxt_mesh_import_options_t* options)
{
    gs_gfxt_mesh_t mesh = gs_default_val();

    gs_vec3 v_pos[] = {
        gs_v3(-1.0f, -1.0f, 0.f), // Top Left
        gs_v3(+1.0f, -1.0f, 0.f), // Top Right 
        gs_v3(-1.0f, +1.0f, 0.f), // Bottom Left
        gs_v3(+1.0f, +1.0f, 0.f)  // Bottom Right
    };

    // Vertex data for quad
    gs_vec2 v_uvs[] = {
        gs_v2(0.0f, 0.0f),  // Top Left
        gs_v2(1.0f, 0.0f),  // Top Right 
        gs_v2(0.0f, 1.0f),  // Bottom Left
        gs_v2(1.0f, 1.0f)   // Bottom Right
    };

    gs_vec3 v_norm[] = {
        gs_v3(0.f, 0.f, 1.f),
        gs_v3(0.f, 0.f, 1.f),
        gs_v3(0.f, 0.f, 1.f),
        gs_v3(0.f, 0.f, 1.f)
    };

    gs_vec3 v_tan[] = {
        gs_v3(1.f, 0.f, 0.f),
        gs_v3(1.f, 0.f, 0.f),
        gs_v3(1.f, 0.f, 0.f),
        gs_v3(1.f, 0.f, 0.f)
    };

    gs_color_t v_color[] = {
        GS_COLOR_WHITE,
        GS_COLOR_WHITE,
        GS_COLOR_WHITE,
        GS_COLOR_WHITE
    };

    // Index data for quad
    uint16_t i_data[] = {
        0, 3, 2,    // First Triangle
        0, 1, 3     // Second Triangle
    }; 

    // Mesh data
    gs_gfxt_mesh_raw_data_t mesh_data = gs_default_val();

    // Primitive to upload
    gs_gfxt_mesh_vertex_data_t vert_data = gs_default_val();
    vert_data.positions.data = v_pos; vert_data.positions.size = sizeof(v_pos);
    vert_data.normals.data = v_norm; vert_data.normals.size = sizeof(v_norm);
    vert_data.tangents.data = v_tan; vert_data.tangents.size = sizeof(v_tan);
    vert_data.colors[0].data = v_color; vert_data.colors[0].size = sizeof(v_color);
    vert_data.tex_coords[0].data = v_uvs; vert_data.tex_coords[0].size = sizeof(v_uvs);
    vert_data.indices.data = i_data; vert_data.indices.size = sizeof(i_data);
    vert_data.count = 6;

    // Push into primitives
    gs_dyn_array_push(mesh_data.primitives, vert_data); 

    // If no decl, then just use default layout
    /*
    gs_gfxt_mesh_import_options_t* moptions = options ? options : &def_options; 
    uint32_t ct = moptions->size / sizeof(gs_asset_mesh_layout_t);
    */

    gs_gfxt_mesh_desc_t mdesc = gs_default_val();
    mdesc.meshes = &mesh_data;
    mdesc.size = 1 * sizeof(gs_gfxt_mesh_raw_data_t);
    mdesc.keep_data = true;

    mesh = gs_gfxt_mesh_create(&mdesc);
    mesh.desc = mdesc;

    // Free data
    gs_dyn_array_free(mesh_data.primitives);

    return mesh;
}

GS_API_DECL gs_handle(gs_graphics_texture_t) 
gs_gfxt_texture_generate_default()
{
    // Generate procedural texture data (checkered texture)
    #define GS_GFXT_ROW_COL_CT  5
    gs_color_t c0 = GS_COLOR_WHITE;
    gs_color_t c1 = gs_color(20, 50, 150, 255);
    gs_color_t pixels[GS_GFXT_ROW_COL_CT * GS_GFXT_ROW_COL_CT] = gs_default_val();
    for (uint32_t r = 0; r < GS_GFXT_ROW_COL_CT; ++r) {
        for (uint32_t c = 0; c < GS_GFXT_ROW_COL_CT; ++c) {
            const bool re = (r % 2) == 0;
            const bool ce = (c % 2) == 0;
            uint32_t idx = r * GS_GFXT_ROW_COL_CT + c;
            pixels[idx] = (re && ce) ? c0 : (re) ? c1 : (ce) ? c1 : c0;
        } 
    }

    gs_graphics_texture_desc_t desc = gs_default_val();
    desc.width = GS_GFXT_ROW_COL_CT;
    desc.height = GS_GFXT_ROW_COL_CT;
    desc.format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    desc.min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST; 
    desc.mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST;
    desc.wrap_s = GS_GRAPHICS_TEXTURE_WRAP_REPEAT;
    desc.wrap_t = GS_GRAPHICS_TEXTURE_WRAP_REPEAT;
    *desc.data = pixels;

    // Create dynamic texture
    return gs_graphics_texture_create(&desc);
}

//=== Resource Loading ===//

typedef struct tmp_buffer_t
{
    char txt[1024]; 
} tmp_buffer_t; 

typedef struct gs_shader_io_data_t
{
    char type[64];
    char name[64];
} gs_shader_io_data_t;

typedef struct gs_pipeline_parse_data_t 
{ 
    gs_dyn_array(gs_shader_io_data_t) io_list[3];
    gs_dyn_array(gs_gfxt_mesh_layout_t) mesh_layout;
    gs_dyn_array(gs_graphics_vertex_attribute_type) vertex_layout;
    char* code[3];
    char dir[256];
} gs_ppd_t;

#define gs_parse_warning(TXT, ...)\
    do {\
        gs_printf("WARNING::");\
        gs_printf(TXT, ##__VA_ARGS__);\
        gs_println("");\
    } while (0)

#define gs_parse_error(TXT, ASSERT, ...)\
    do {\
        gs_printf("ERROR::");\
        gs_printf(TXT, ##__VA_ARGS__);\
        gs_println("");\
        if (ASSERT) gs_assert(false);\
    } while (0)

#define gs_parse_block(NAME, ...)\
    do {\
        gs_println("gs_pipeline_load_from_file::parsing::%s", #NAME);\
        if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_LBRACE))\
        {\
            gs_println("error::gs_pipeline_load_from_file::error parsing raster from .sf resource");\
            gs_assert(false);\
        }\
\
        uint32_t bc = 1;\
        while (gs_lexer_can_lex(lex) && bc)\
        {\
            gs_token_t token = gs_lexer_next_token(lex);\
            switch (token.type)\
            {\
                case GS_TOKEN_LBRACE: {bc++;} break;\
                case GS_TOKEN_RBRACE: {bc--;} break;\
\
                case GS_TOKEN_IDENTIFIER:\
                {\
                    __VA_ARGS__\
                }\
            }\
        }\
    } while (0)

const char* gs_get_vertex_attribute_string(gs_graphics_vertex_attribute_type type)
{ 
    switch (type)
    {
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT:   return "float"; break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2:  return "vec2";  break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3:  return "vec3";  break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4:  return "vec4";  break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT:    return "int";   break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT2:   return "vec2";  break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT3:   return "vec3";  break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT4:   return "vec4";  break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE:    return "float"; break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE2:   return "vec2"; break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE3:   return "vec3"; break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4:   return "vec4"; break;
        default: return "UNKNOWN"; break;
    }
}

gs_graphics_vertex_attribute_type gs_get_vertex_attribute_from_token(const gs_token_t* t)
{ 
    if (gs_token_compare_text(t, "float"))       return GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT;
    else if (gs_token_compare_text(t, "float2")) return GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2;
    else if (gs_token_compare_text(t, "float3")) return GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3;
    else if (gs_token_compare_text(t, "float4")) return GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4;
    else if (gs_token_compare_text(t, "uint4"))  return GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT4;
    else if (gs_token_compare_text(t, "uint3"))  return GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT3;
    else if (gs_token_compare_text(t, "uint2"))  return GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT2;
    else if (gs_token_compare_text(t, "uint"))   return GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT;
    else if (gs_token_compare_text(t, "byte4"))  return GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4;
    else if (gs_token_compare_text(t, "byte3"))  return GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE3;
    else if (gs_token_compare_text(t, "byte2"))  return GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE2;
    else if (gs_token_compare_text(t, "byte"))   return GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE;
    return (gs_graphics_vertex_attribute_type)0x00;
}

gs_graphics_uniform_type gs_uniform_type_from_token(const gs_token_t* t)
{
    if (gs_token_compare_text(t, "float"))               return GS_GRAPHICS_UNIFORM_FLOAT; 
    else if (gs_token_compare_text(t, "int"))            return GS_GRAPHICS_UNIFORM_INT;
    else if (gs_token_compare_text(t, "vec2"))           return GS_GRAPHICS_UNIFORM_VEC2;
    else if (gs_token_compare_text(t, "vec3"))           return GS_GRAPHICS_UNIFORM_VEC3; 
    else if (gs_token_compare_text(t, "vec4"))           return GS_GRAPHICS_UNIFORM_VEC4; 
    else if (gs_token_compare_text(t, "mat4"))           return GS_GRAPHICS_UNIFORM_MAT4; 
    else if (gs_token_compare_text(t, "sampler2D"))      return GS_GRAPHICS_UNIFORM_SAMPLER2D; 
    else if (gs_token_compare_text(t, "usampler2D"))     return GS_GRAPHICS_UNIFORM_USAMPLER2D; 
    else if (gs_token_compare_text(t, "samplerCube"))    return GS_GRAPHICS_UNIFORM_SAMPLERCUBE; 
    else if (gs_token_compare_text(t, "img2D_rgba32f"))  return GS_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F; 
    return (gs_graphics_uniform_type)0x00;
}

const char* gs_uniform_string_from_type(gs_graphics_uniform_type type)
{
    switch (type)
    {
        case GS_GRAPHICS_UNIFORM_FLOAT:           return "float"; break;
        case GS_GRAPHICS_UNIFORM_INT:             return "int"; break;
        case GS_GRAPHICS_UNIFORM_VEC2:            return "vec2"; break;
        case GS_GRAPHICS_UNIFORM_VEC3:            return "vec3"; break; 
        case GS_GRAPHICS_UNIFORM_VEC4:            return "vec4"; break; 
        case GS_GRAPHICS_UNIFORM_MAT4:            return "mat4"; break;
        case GS_GRAPHICS_UNIFORM_SAMPLER2D:       return "sampler2D"; break; 
        case GS_GRAPHICS_UNIFORM_USAMPLER2D:      return "usampler2D"; break; 
        case GS_GRAPHICS_UNIFORM_SAMPLERCUBE:     return "samplerCube"; break; 
        case GS_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F: return "image2D"; break; 
        default: return "UNKNOWN"; break;
    }
    return (char*)0x00;
}

// Make this an extern function that can be bubbled up to the app
bool gs_parse_uniform_special_keyword(gs_lexer_t* lex, gs_gfxt_pipeline_desc_t* desc, gs_ppd_t* ppd, gs_graphics_shader_stage_type stage, gs_gfxt_uniform_desc_t* uniform)
{
    gs_token_t token = lex->current_token;

    // Determine if uniform is one of special key defines
    if (gs_token_compare_text(&token, "GS_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX"))
    {
        uniform->type = GS_GRAPHICS_UNIFORM_MAT4; 
        memcpy(uniform->name, GS_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX, sizeof(GS_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX));
        return true;
    }
    else if (gs_token_compare_text(&token, "GS_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX"))
    {
        uniform->type = GS_GRAPHICS_UNIFORM_MAT4; 
        memcpy(uniform->name, GS_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX, sizeof(GS_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX));
        return true;
    }
    else if (gs_token_compare_text(&token, "GS_GFXT_UNIFORM_MODEL_MATRIX"))
    {
        uniform->type = GS_GRAPHICS_UNIFORM_MAT4; 
        memcpy(uniform->name, GS_GFXT_UNIFORM_MODEL_MATRIX, sizeof(GS_GFXT_UNIFORM_MODEL_MATRIX));
        return true;
    }
    else if (gs_token_compare_text(&token, "GS_GFXT_UNIFORM_INVERSE_MODEL_MATRIX"))
    {
        uniform->type = GS_GRAPHICS_UNIFORM_MAT4; 
        memcpy(uniform->name, GS_GFXT_UNIFORM_INVERSE_MODEL_MATRIX, sizeof(GS_GFXT_UNIFORM_INVERSE_MODEL_MATRIX));
        return true;
    }
    else if (gs_token_compare_text(&token, "GS_GFXT_UNIFORM_PROJECTION_MATRIX"))
    {
        uniform->type = GS_GRAPHICS_UNIFORM_MAT4; 
        memcpy(uniform->name, GS_GFXT_UNIFORM_PROJECTION_MATRIX, sizeof(GS_GFXT_UNIFORM_PROJECTION_MATRIX));
        return true;
    }
    else if (gs_token_compare_text(&token, "GS_GFXT_UNIFORM_VIEW_MATRIX"))
    {
        uniform->type = GS_GRAPHICS_UNIFORM_MAT4; 
        memcpy(uniform->name, GS_GFXT_UNIFORM_VIEW_MATRIX, sizeof(GS_GFXT_UNIFORM_VIEW_MATRIX));
        return true;
    }
    else if (gs_token_compare_text(&token, "GS_GFXT_UNIFORM_TIME"))
    {
        uniform->type = GS_GRAPHICS_UNIFORM_FLOAT; 
        memcpy(uniform->name, GS_GFXT_UNIFORM_TIME, sizeof(GS_GFXT_UNIFORM_TIME));
        return true;
    }

    return false;
}

bool gs_parse_uniforms(gs_lexer_t* lex, gs_gfxt_pipeline_desc_t* desc, gs_ppd_t* ppd, gs_graphics_shader_stage_type stage)
{
    uint32_t image_binding = 0;

    if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_LBRACE))
    {
        gs_log_warning("Unable to parsing uniforms from .sf resource");
        return false;
    }

    uint32_t bc = 1;\
    while (gs_lexer_can_lex(lex) && bc)
    {
        gs_token_t token = gs_lexer_next_token(lex);
        switch (token.type)
        {
            case GS_TOKEN_LBRACE: {bc++;} break;
            case GS_TOKEN_RBRACE: {bc--;} break;

            case GS_TOKEN_IDENTIFIER:
            {
                gs_gfxt_uniform_desc_t uniform = {0};
                uniform.stage = stage;

                bool special = gs_parse_uniform_special_keyword(lex, desc, ppd, stage, &uniform);

                // Determine if uniform is one of special key defines
                if (!special)
                {
                    uniform.type = gs_uniform_type_from_token(&token);
                    switch (uniform.type)
                    {
                        default: break;

                        case GS_GRAPHICS_UNIFORM_SAMPLER2D:
                        case GS_GRAPHICS_UNIFORM_USAMPLER2D:
                        case GS_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F:
                        {
                            uniform.binding = image_binding++;
                        } break;
                    } 

                    if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER)) 
                    { 
                        gs_log_warning("Unidentified token (Expected identifier)");
                        gs_token_debug_print(&lex->current_token);
                        return false;
                    } 
                    token = lex->current_token;

                    memcpy(uniform.name, token.text, token.len);
                }

                // Add uniform to ublock descriptor
                gs_dyn_array_push(desc->ublock_desc.layout, uniform); 

            } break;
        }
    } 
    return true;
} 

bool gs_parse_io(gs_lexer_t* lex, gs_gfxt_pipeline_desc_t* desc, gs_ppd_t* ppd, gs_graphics_shader_stage_type type)
{
    if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_LBRACE))
    {
        gs_log_warning("Expected opening left brace. Unable to parse io from .sf resource");\
        return false;
    }

    uint32_t bc = 1;
    while (gs_lexer_can_lex(lex) && bc)
    {
        gs_token_t token = gs_lexer_next_token(lex);
        switch (token.type)
        {
            case GS_TOKEN_LBRACE: {bc++;} break;
            case GS_TOKEN_RBRACE: {bc--;} break;
            case GS_TOKEN_IDENTIFIER:
            {
                gs_shader_io_data_t io = {0}; 
                memcpy(io.type, token.text, token.len);

                switch (type)
                {
                    case GS_GRAPHICS_SHADER_STAGE_VERTEX:
                    {
                        if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER)) 
                        { 
                            gs_log_warning("IO expected identifier name after type, shader stage vertex.");
                            gs_token_debug_print(&lex->current_token);
                            return false;
                        } 
                        token = lex->current_token;
                        memcpy(io.name, token.text, token.len);
                        gs_dyn_array_push(ppd->io_list[0], io);
                    } break;

                    case GS_GRAPHICS_SHADER_STAGE_FRAGMENT:
                    {
                        if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER)) 
                        {
                            gs_log_warning("IO expected identifier name after type, shader stage fragment.");
                            gs_token_debug_print(&lex->current_token);
                            return false;
                        }
                        token = lex->current_token;
                        memcpy(io.name, token.text, token.len);
                        gs_dyn_array_push(ppd->io_list[1], io);
                    } break;

                    case GS_GRAPHICS_SHADER_STAGE_COMPUTE:
                    {
                        if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_NUMBER)) 
                        {
                            gs_log_warning("IO expected number after type, shader stage compute.");
                            gs_token_debug_print(&lex->current_token);
                            return false;
                        }
                        token = lex->current_token;
                        memcpy(io.name, token.text, token.len);
                        gs_dyn_array_push(ppd->io_list[2], io);
                    } break;
                } 
            } break; 
        }
    }
    return true;
}

bool gs_parse_code(gs_lexer_t* lex, gs_gfxt_pipeline_desc_t* desc, gs_ppd_t* ppd, gs_graphics_shader_stage_type stage)
{
    if (!gs_lexer_require_token_type(lex, GS_TOKEN_LBRACE)) 
    { 
        gs_log_warning("Expected opening left brace");
        return false; 
    } 

	// Something is broken up here...
    uint32_t bc = 1; 
    gs_token_t cur = gs_lexer_peek(lex);
    gs_token_t token = lex->current_token;
    while (gs_lexer_can_lex(lex) && bc)
    {
        token = lex->next_token(lex);
        switch (token.type)
        { 
            case GS_TOKEN_LBRACE: {bc++;} break; 
            case GS_TOKEN_RBRACE: {bc--;} break; 
        }
    }

    // Allocate size for code
    const size_t sz = (size_t)(token.text - cur.text);
    char* code = (char*)gs_malloc(sz);
    memset(code, 0, sz); 
    memcpy(code, cur.text, sz - 1);

    // List of include directories to gather
    uint32_t iidx = 0;
    char includes[GS_GFXT_INCLUDE_DIR_MAX][256] = {0};

    // Need to parse through code and replace keywords with appropriate mappings
    gs_lexer_t clex = gs_lexer_c_ctor(code);
    while (clex.can_lex(&clex))
    {
        gs_token_t tkn = clex.next_token(&clex);
        switch (tkn.type)
        {
            case GS_TOKEN_IDENTIFIER:
            {
                if (gs_token_compare_text(&tkn, "GS_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX"))
                {
                    gs_util_string_replace(tkn.text, tkn.len, GS_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX, (char)32);
                }
                else if (gs_token_compare_text(&tkn, "GS_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX"))
                {
                    gs_util_string_replace(tkn.text, tkn.len, GS_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX, (char)32);
                }
                else if (gs_token_compare_text(&tkn, "GS_GFXT_UNIFORM_MODEL_MATRIX"))
                {
                    gs_util_string_replace(tkn.text, tkn.len, GS_GFXT_UNIFORM_MODEL_MATRIX, (char)32);
                }
                else if (gs_token_compare_text(&tkn, "GS_GFXT_UNIFORM_INVERSE_MODEL_MATRIX"))
                {
                    gs_util_string_replace(tkn.text, tkn.len, GS_GFXT_UNIFORM_INVERSE_MODEL_MATRIX, (char)32);
                }
                else if (gs_token_compare_text(&tkn, "GS_GFXT_UNIFORM_VIEW_MATRIX"))
                {
                    gs_util_string_replace(tkn.text, tkn.len, GS_GFXT_UNIFORM_VIEW_MATRIX, (char)32);
                }
                else if (gs_token_compare_text(&tkn, "GS_GFXT_UNIFORM_PROJECTION_MATRIX"))
                {
                    gs_util_string_replace(tkn.text, tkn.len, GS_GFXT_UNIFORM_PROJECTION_MATRIX, (char)32);
                }
                else if (gs_token_compare_text(&tkn, "GS_GFXT_UNIFORM_TIME"))
                {
                    gs_util_string_replace(tkn.text, tkn.len, GS_GFXT_UNIFORM_TIME, (char)32);
                }
            } break;

            case GS_TOKEN_HASH:
            { 
                // Parse include
                tkn = clex.next_token(&clex);
                switch (tkn.type)
                {
                    case GS_TOKEN_IDENTIFIER:
                    { 
                        if (gs_token_compare_text(&tkn, "include") && iidx < GS_GFXT_INCLUDE_DIR_MAX)
                        { 
                            // Length of include string
                            size_t ilen = 8;

                            // Grab next token, expect string
                            tkn = clex.next_token(&clex);
                            if (tkn.type == GS_TOKEN_STRING)
                            {
                                memcpy(includes[iidx], tkn.text + 1, tkn.len - 2);
                                gs_util_string_replace(tkn.text - ilen, tkn.len + ilen,
                                    " ", (char)32);
                                iidx++;
                            }
                        }
                    }
                }
            } break;
        }
    }

    for (uint32_t i = 0; i < GS_GFXT_INCLUDE_DIR_MAX; ++i)
    { 
        if (!includes[i][0]) continue;

        // Need to collect other uniforms from these includes (parse code)
        gs_snprintfc(FINAL_PATH, 256, "%s/%s", ppd->dir, includes[i]);
        // gs_println("INC_DIR: %s", FINAL_PATH);

        // Load include using final path and relative path from include
        size_t len = 0;
        char* inc_src = gs_platform_read_file_contents(FINAL_PATH, "rb", &len);
        gs_assert(inc_src);

        // Realloc previous code to greater size, shift contents around
        char* cat = gs_util_string_concat(inc_src, code);
        gs_free(code);
        code = cat;
    }

    switch (stage)
    {
        case GS_GRAPHICS_SHADER_STAGE_VERTEX:   ppd->code[0] = code; break; 
        case GS_GRAPHICS_SHADER_STAGE_FRAGMENT: ppd->code[1] = code; break;
        case GS_GRAPHICS_SHADER_STAGE_COMPUTE:  ppd->code[2] = code; break;
    }

    return true;
}

gs_gfxt_mesh_attribute_type gs_mesh_attribute_type_from_token(const gs_token_t* token)
{
    if (gs_token_compare_text(token, "POSITION"))        return GS_ASSET_MESH_ATTRIBUTE_TYPE_POSITION;
    else if (gs_token_compare_text(token, "NORMAL"))     return GS_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL;
    else if (gs_token_compare_text(token, "COLOR"))      return GS_ASSET_MESH_ATTRIBUTE_TYPE_COLOR;
    else if (gs_token_compare_text(token, "TANGENT"))    return GS_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT;
    else if (gs_token_compare_text(token, "TEXCOORD0"))  return GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (gs_token_compare_text(token, "TEXCOORD1"))  return GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (gs_token_compare_text(token, "TEXCOORD2"))  return GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (gs_token_compare_text(token, "TEXCOORD3"))  return GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (gs_token_compare_text(token, "TEXCOORD4"))  return GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (gs_token_compare_text(token, "TEXCOORD5"))  return GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (gs_token_compare_text(token, "TEXCOORD6"))  return GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (gs_token_compare_text(token, "TEXCOORD7"))  return GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (gs_token_compare_text(token, "TEXCOORD8"))  return GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (gs_token_compare_text(token, "TEXCOORD9"))  return GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (gs_token_compare_text(token, "TEXCOORD10")) return GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (gs_token_compare_text(token, "TEXCOORD11")) return GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (gs_token_compare_text(token, "TEXCOORD12")) return GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD;
    else if (gs_token_compare_text(token, "UINT"))       return GS_ASSET_MESH_ATTRIBUTE_TYPE_UINT;

    // Default
    return (gs_gfxt_mesh_attribute_type)0x00;
} 

bool gs_parse_vertex_mesh_attributes(gs_lexer_t* lex, gs_gfxt_pipeline_desc_t* desc, gs_ppd_t* ppd)
{
    if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_LBRACE)) 
    { 
        gs_assert(false);
    } 

    uint32_t bc = 1; 
    while (gs_lexer_can_lex(lex) && bc)
    {
        gs_token_t token = gs_lexer_next_token(lex);
    // gs_token_debug_print(&token);
        switch (token.type)
        { 
            case GS_TOKEN_LBRACE: {bc++;} break; 
            case GS_TOKEN_RBRACE: {bc--;} break;
            
            case GS_TOKEN_IDENTIFIER: 
            { 
                // Get attribute name
                if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER))
                {
                    gs_assert(false);
                } 

                gs_token_t token_name = lex->current_token;
        // gs_token_debug_print(&token_name);

                #define PUSH_ATTR(MESH_ATTR, VERT_ATTR)\
                    do {\
                        gs_gfxt_mesh_layout_t layout = gs_default_val();\
                        layout.type = GS_ASSET_MESH_ATTRIBUTE_TYPE_##MESH_ATTR;\
                        gs_dyn_array_push(ppd->mesh_layout, layout);\
                        gs_graphics_vertex_attribute_desc_t attr = gs_default_val();\
                        memcpy(attr.name, token_name.text, token_name.len);\
                        attr.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_##VERT_ATTR;\
                        gs_dyn_array_push(desc->pip_desc.layout.attrs, attr);\
                        /*gs_println("%s: %s", #MESH_ATTR, #VERT_ATTR);*/\
                    } while (0) 

                if (gs_token_compare_text(&token, "POSITION"))        PUSH_ATTR(POSITION, FLOAT3);
                else if (gs_token_compare_text(&token, "NORMAL"))     PUSH_ATTR(NORMAL, FLOAT3);
                else if (gs_token_compare_text(&token, "COLOR"))      PUSH_ATTR(COLOR, BYTE4);
                else if (gs_token_compare_text(&token, "TANGENT"))    PUSH_ATTR(TANGENT, FLOAT3);
                else if (gs_token_compare_text(&token, "TEXCOORD"))   PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (gs_token_compare_text(&token, "TEXCOORD0"))  PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (gs_token_compare_text(&token, "TEXCOORD1"))  PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (gs_token_compare_text(&token, "TEXCOORD2"))  PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (gs_token_compare_text(&token, "TEXCOORD3"))  PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (gs_token_compare_text(&token, "TEXCOORD4"))  PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (gs_token_compare_text(&token, "TEXCOORD5"))  PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (gs_token_compare_text(&token, "TEXCOORD6"))  PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (gs_token_compare_text(&token, "TEXCOORD8"))  PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (gs_token_compare_text(&token, "TEXCOORD9"))  PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (gs_token_compare_text(&token, "TEXCOORD10")) PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (gs_token_compare_text(&token, "TEXCOORD11")) PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (gs_token_compare_text(&token, "TEXCOORD12")) PUSH_ATTR(TEXCOORD, FLOAT2);
                else if (gs_token_compare_text(&token, "FLOAT"))      PUSH_ATTR(WEIGHT, FLOAT4);   
                else if (gs_token_compare_text(&token, "FLOAT2"))     PUSH_ATTR(TEXCOORD, FLOAT2); 
                else if (gs_token_compare_text(&token, "FLOAT3"))     PUSH_ATTR(POSITION, FLOAT3);  
                else if (gs_token_compare_text(&token, "UINT"))       PUSH_ATTR(UINT, UINT);  
                // else if (gs_token_compare_text(&token, "FLOAT4"))     PUSH_ATTR(TANGENT, FLOAT4);  
                else 
                {
                    gs_log_warning("Unidentified vertex attribute: %.*s: %.*s", 
                    token.len, token.text, token_name.len, token_name.text);
                    return false;
                }
            }
        }
    }
    return true;
}

bool gs_parse_vertex_attributes(gs_lexer_t* lex, gs_gfxt_pipeline_desc_t* desc, gs_ppd_t* ppd)
{
    return gs_parse_vertex_mesh_attributes(lex, desc, ppd);
}

bool gs_parse_shader_stage(gs_lexer_t* lex, gs_gfxt_pipeline_desc_t* desc, gs_ppd_t* ppd, gs_graphics_shader_stage_type stage)
{
    if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_LBRACE))
    {
        gs_println("error::gs_pipeline_load_from_file::error parsing raster from .sf resource");
        gs_assert(false);
    }

    uint32_t bc = 1;
    while (gs_lexer_can_lex(lex) && bc)
    {
        gs_token_t token = gs_lexer_next_token(lex);
        switch (token.type)
        {
            case GS_TOKEN_LBRACE: {bc++;} break;
            case GS_TOKEN_RBRACE: {bc--;} break;

            case GS_TOKEN_IDENTIFIER:
            {
                if (stage == GS_GRAPHICS_SHADER_STAGE_VERTEX && 
                     gs_token_compare_text(&token, "attributes"))
                {
                    gs_println("parsing attributes...");
                    if (!gs_parse_vertex_attributes(lex, desc, ppd))
                    {
                        gs_log_warning("Unable to parse vertex attributes.");
                        return false;
                    }
                }

                else if (gs_token_compare_text(&token, "uniforms"))
                { 
                    gs_println("parsing uniforms...");
                    if (!gs_parse_uniforms(lex, desc, ppd, stage))
                    {
                        gs_log_warning("Unable to parse 'uniforms' for stage: %zu.", (u32)stage);
                        return false;
                    }
                }

                else if (gs_token_compare_text(&token, "out"))
                {
                    gs_println("parsing out...");
                    if (!gs_parse_io(lex, desc, ppd, stage))
                    {
                        gs_log_warning("Unable to parse 'out' for stage: %zu.", (u32)stage);
                        return false;
                    }
                }

                else if (gs_token_compare_text(&token, "in"))
                {
                    gs_println("parsing in...");
                    if (!gs_parse_io(lex, desc, ppd, stage))
                    {
                        gs_log_warning("Unable to parse 'in' for stage: %zu.", (u32)stage);
                        return false;
                    }
                }

                else if (gs_token_compare_text(&token, "code"))
                {
                    gs_println("parsing code...");
                    if (!gs_parse_code(lex, desc, ppd, stage))
                    {
                        gs_log_warning("Unable to parse 'code' for stage: %zu.", (u32)stage);
                        return false;
                    }
                }
            } break; 
        }
    }
    return true;
}

bool gs_parse_compute_shader_stage(gs_lexer_t* lex, gs_gfxt_pipeline_desc_t* desc, gs_ppd_t* ppd)
{
    gs_parse_block(
    PIPELINE::COMPUTE_SHADER_STAGE,
    { 
        if (gs_token_compare_text(&token, "uniforms"))
        {
            if (!gs_parse_uniforms(lex, desc, ppd, GS_GRAPHICS_SHADER_STAGE_COMPUTE))
            {
                gs_log_warning("Unable to parse 'uniforms' for compute shader");
                return false;
            }
        }

        else if (gs_token_compare_text(&token, "in"))
        {
            if (!gs_parse_io(lex, desc, ppd, GS_GRAPHICS_SHADER_STAGE_COMPUTE))
            {
                gs_log_warning("Unable to parse 'in' for compute shader");
                return false;
            }
        } 

        else if (gs_token_compare_text(&token, "code"))
        {
            if (!gs_parse_code(lex, desc, ppd, GS_GRAPHICS_SHADER_STAGE_COMPUTE))
            {
                gs_log_warning("Unable to parse 'code' for compute shader");
                return false;
            }
        }
    }); 
    return true;
}

bool gs_parse_shader(gs_lexer_t* lex, gs_gfxt_pipeline_desc_t* desc, gs_ppd_t* ppd)
{ 
    if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_LBRACE)) 
    { 
        gs_log_warning("Unable to parse shader from .sf resource. Expected opening left brace.");
        return false;
    } 

    // Braces
    uint32_t bc = 1; 
    while (gs_lexer_can_lex(lex) && bc)
    {
        gs_token_t token = lex->next_token(lex);
        switch (token.type)
        {
            case GS_TOKEN_LBRACE: {bc++;} break; 
            case GS_TOKEN_RBRACE: {bc--;} break;

            case GS_TOKEN_IDENTIFIER:
            {
                // Vertex shader
                if (gs_token_compare_text(&token, "vertex"))
                {
                    gs_println("parsing vertex shader");
                    if (!gs_parse_shader_stage(lex, desc, ppd, GS_GRAPHICS_SHADER_STAGE_VERTEX))
                    {
                        gs_log_warning("Unable to parse shader stage: Vertex");
                        return false;
                    }
                }

                // Fragment shader
                else if (gs_token_compare_text(&token, "fragment"))
                {
                    gs_println("parsing fragment shader");
                    if (!gs_parse_shader_stage(lex, desc, ppd, GS_GRAPHICS_SHADER_STAGE_FRAGMENT))
                    {
                        gs_log_warning("Unable to parse shader stage: Fragment");
                        return false;
                    }
                } 

                // Compute shader
                else if (gs_token_compare_text(&token, "compute"))
                {
                    gs_println("parsing compute shader");
                    if (!gs_parse_shader_stage(lex, desc, ppd, GS_GRAPHICS_SHADER_STAGE_COMPUTE))
                    {
                        gs_log_warning("Unable to parse shader stage: Compute");
                        return false;
                    }
                } 

            } break;
        }
    }
    return true;
}

bool gs_parse_depth(gs_lexer_t* lex, gs_gfxt_pipeline_desc_t* pdesc, gs_ppd_t* ppd)
{
    gs_parse_block(
    PIPELINE::DEPTH,
    { 
        // Depth function
        if (gs_token_compare_text(&token, "func"))
        {
            if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER))
            {
                token = lex->current_token;
                gs_log_warning("Depth func type not found after function decl: %.*s", token.len, token.text);
                return false;
            }

            token = lex->current_token;
            
            if      (gs_token_compare_text(&token, "LESS"))     pdesc->pip_desc.depth.func = GS_GRAPHICS_DEPTH_FUNC_LESS;
            else if (gs_token_compare_text(&token, "EQUAL"))    pdesc->pip_desc.depth.func = GS_GRAPHICS_DEPTH_FUNC_EQUAL;
            else if (gs_token_compare_text(&token, "LEQUAL"))   pdesc->pip_desc.depth.func = GS_GRAPHICS_DEPTH_FUNC_LEQUAL;
            else if (gs_token_compare_text(&token, "GREATER"))  pdesc->pip_desc.depth.func = GS_GRAPHICS_DEPTH_FUNC_GREATER;
            else if (gs_token_compare_text(&token, "NOTEQUAL")) pdesc->pip_desc.depth.func = GS_GRAPHICS_DEPTH_FUNC_NOTEQUAL;
            else if (gs_token_compare_text(&token, "GEQUAL"))   pdesc->pip_desc.depth.func = GS_GRAPHICS_DEPTH_FUNC_GEQUAL;
            else if (gs_token_compare_text(&token, "ALWAYS"))   pdesc->pip_desc.depth.func = GS_GRAPHICS_DEPTH_FUNC_ALWAYS; 
            else if (gs_token_compare_text(&token, "NEVER"))    pdesc->pip_desc.depth.func = GS_GRAPHICS_DEPTH_FUNC_NEVER; 
            else
            {
                token = lex->current_token;
                gs_log_warning("Func type %.*s not valid.", token.len, token.text);
                return false;
            }
        }
        if (gs_token_compare_text(&token, "mask"))
        {
            if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER))
            {
                token = lex->current_token;
                gs_log_warning("Depth mask type not found after function decl: %.*s", token.len, token.text);
                return false;
            }

            token = lex->current_token;
            
            if      (gs_token_compare_text(&token, "ENABLED"))  pdesc->pip_desc.depth.mask = GS_GRAPHICS_DEPTH_MASK_ENABLED;
            else if (gs_token_compare_text(&token, "TRUE"))     pdesc->pip_desc.depth.mask = GS_GRAPHICS_DEPTH_MASK_ENABLED;
            else if (gs_token_compare_text(&token, "true"))     pdesc->pip_desc.depth.mask = GS_GRAPHICS_DEPTH_MASK_ENABLED;
            else if (gs_token_compare_text(&token, "DISABLED")) pdesc->pip_desc.depth.mask = GS_GRAPHICS_DEPTH_MASK_DISABLED;
            else if (gs_token_compare_text(&token, "FALSE"))    pdesc->pip_desc.depth.mask = GS_GRAPHICS_DEPTH_MASK_DISABLED;
            else if (gs_token_compare_text(&token, "false"))    pdesc->pip_desc.depth.mask = GS_GRAPHICS_DEPTH_MASK_DISABLED;
            else
            {
                token = lex->current_token;
                gs_log_warning("Mask type %.*s not valid.", token.len, token.text);
                return false;
            } 
            gs_println("MASK: %zu", (uint32_t)pdesc->pip_desc.depth.mask);
        }
    });
    return true;
}

bool gs_parse_blend(gs_lexer_t* lex, gs_gfxt_pipeline_desc_t* pdesc, gs_ppd_t* ppd)
{
    gs_parse_block(
    PIPELINE::BLEND,
    { 
        // Blend function
        if (gs_token_compare_text(&token, "func"))
        { 
            if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER))
            {
                gs_log_warning("Blend func type not found after function decl.");
                return false;
            }

            token = lex->current_token;
            
            if      (gs_token_compare_text(&token, "ADD"))              pdesc->pip_desc.blend.func = GS_GRAPHICS_BLEND_EQUATION_ADD;
            else if (gs_token_compare_text(&token, "SUBTRACT"))         pdesc->pip_desc.blend.func = GS_GRAPHICS_BLEND_EQUATION_SUBTRACT;
            else if (gs_token_compare_text(&token, "REVERSE_SUBTRACT")) pdesc->pip_desc.blend.func = GS_GRAPHICS_BLEND_EQUATION_REVERSE_SUBTRACT;
            else if (gs_token_compare_text(&token, "MIN"))              pdesc->pip_desc.blend.func = GS_GRAPHICS_BLEND_EQUATION_MIN;
            else if (gs_token_compare_text(&token, "MAX"))              pdesc->pip_desc.blend.func = GS_GRAPHICS_BLEND_EQUATION_MAX;
            else
            {
                gs_log_warning("Blend func type %.*s not valid.", token.len, token.text);
                return false;
            } 
        }

        // Source blend
        else if (gs_token_compare_text(&token, "src"))
        {
            if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER))
            {
                gs_log_warning("Blend src type not found after decl.");
                return false;
            }

            token = lex->current_token;
            
            if      (gs_token_compare_text(&token, "ZERO"))                     pdesc->pip_desc.blend.src = GS_GRAPHICS_BLEND_MODE_ZERO;
            else if (gs_token_compare_text(&token, "ONE"))                      pdesc->pip_desc.blend.src = GS_GRAPHICS_BLEND_MODE_ONE; 
            else if (gs_token_compare_text(&token, "SRC_COLOR"))                pdesc->pip_desc.blend.src = GS_GRAPHICS_BLEND_MODE_SRC_COLOR; 
            else if (gs_token_compare_text(&token, "ONE_MINUS_SRC_COLOR"))      pdesc->pip_desc.blend.src = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_COLOR; 
            else if (gs_token_compare_text(&token, "DST_COLOR"))                pdesc->pip_desc.blend.src = GS_GRAPHICS_BLEND_MODE_DST_COLOR; 
            else if (gs_token_compare_text(&token, "ONE_MINUS_DST_COLOR"))      pdesc->pip_desc.blend.src = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_COLOR; 
            else if (gs_token_compare_text(&token, "SRC_ALPHA"))                pdesc->pip_desc.blend.src = GS_GRAPHICS_BLEND_MODE_SRC_ALPHA; 
            else if (gs_token_compare_text(&token, "ONE_MINUS_SRC_ALPHA"))      pdesc->pip_desc.blend.src = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA; 
            else if (gs_token_compare_text(&token, "DST_ALPHA"))                pdesc->pip_desc.blend.src = GS_GRAPHICS_BLEND_MODE_DST_ALPHA; 
            else if (gs_token_compare_text(&token, "ONE_MINUS_DST_ALPHA"))      pdesc->pip_desc.blend.src = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_ALPHA; 
            else if (gs_token_compare_text(&token, "CONSTANT_COLOR"))           pdesc->pip_desc.blend.src = GS_GRAPHICS_BLEND_MODE_CONSTANT_COLOR; 
            else if (gs_token_compare_text(&token, "ONE_MINUS_CONSTANT_COLOR")) pdesc->pip_desc.blend.src = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA; 
            else if (gs_token_compare_text(&token, "CONSTANT_ALPHA"))           pdesc->pip_desc.blend.src = GS_GRAPHICS_BLEND_MODE_CONSTANT_ALPHA; 
            else if (gs_token_compare_text(&token, "ONE_MINUS_CONSTANT_ALPHA")) pdesc->pip_desc.blend.src = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA; 
            else
            {
                gs_log_warning("Blend src type %.*s not valid.", token.len, token.text);
                return false;
            } 
        }

        // Dest blend
        else if (gs_token_compare_text(&token, "dst"))
        {
            if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER))
            {
                gs_log_warning("Blend dst type not found after decl.");
                return false;
            }

            token = lex->current_token;
            
            if      (gs_token_compare_text(&token, "ZERO"))                     pdesc->pip_desc.blend.dst = GS_GRAPHICS_BLEND_MODE_ZERO;
            else if (gs_token_compare_text(&token, "ONE"))                      pdesc->pip_desc.blend.dst = GS_GRAPHICS_BLEND_MODE_ONE; 
            else if (gs_token_compare_text(&token, "SRC_COLOR"))                pdesc->pip_desc.blend.dst = GS_GRAPHICS_BLEND_MODE_SRC_COLOR; 
            else if (gs_token_compare_text(&token, "ONE_MINUS_SRC_COLOR"))      pdesc->pip_desc.blend.dst = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_COLOR; 
            else if (gs_token_compare_text(&token, "DST_COLOR"))                pdesc->pip_desc.blend.dst = GS_GRAPHICS_BLEND_MODE_DST_COLOR; 
            else if (gs_token_compare_text(&token, "ONE_MINUS_DST_COLOR"))      pdesc->pip_desc.blend.dst = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_COLOR; 
            else if (gs_token_compare_text(&token, "SRC_ALPHA"))                pdesc->pip_desc.blend.dst = GS_GRAPHICS_BLEND_MODE_SRC_ALPHA; 
            else if (gs_token_compare_text(&token, "ONE_MINUS_SRC_ALPHA"))      pdesc->pip_desc.blend.dst = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA; 
            else if (gs_token_compare_text(&token, "DST_ALPHA"))                pdesc->pip_desc.blend.dst = GS_GRAPHICS_BLEND_MODE_DST_ALPHA; 
            else if (gs_token_compare_text(&token, "ONE_MINUS_DST_ALPHA"))      pdesc->pip_desc.blend.dst = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_ALPHA; 
            else if (gs_token_compare_text(&token, "CONSTANT_COLOR"))           pdesc->pip_desc.blend.dst = GS_GRAPHICS_BLEND_MODE_CONSTANT_COLOR; 
            else if (gs_token_compare_text(&token, "ONE_MINUS_CONSTANT_COLOR")) pdesc->pip_desc.blend.dst = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA; 
            else if (gs_token_compare_text(&token, "CONSTANT_ALPHA"))           pdesc->pip_desc.blend.dst = GS_GRAPHICS_BLEND_MODE_CONSTANT_ALPHA; 
            else if (gs_token_compare_text(&token, "ONE_MINUS_CONSTANT_ALPHA")) pdesc->pip_desc.blend.dst = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA; 
            else
            {
                gs_log_warning("Blend dst type %.*s not valid.", token.len, token.text);
                return false;
            } 
        } 
    });

    return true;
}

bool gs_parse_stencil(gs_lexer_t* lex, gs_gfxt_pipeline_desc_t* pdesc, gs_ppd_t* ppd)
{ 
    gs_parse_block(
    PIPELINE::STENCIL,
    { 
        // Function
        if (gs_token_compare_text(&token, "func"))
        {
            if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER))
            {
                gs_log_warning("Stencil func type not found after decl.");
                return false;
            }

            else
            {
                token = lex->current_token; 
                
                if      (gs_token_compare_text(&token, "LESS"))     pdesc->pip_desc.stencil.func = GS_GRAPHICS_STENCIL_FUNC_LESS;
                else if (gs_token_compare_text(&token, "EQUAL"))    pdesc->pip_desc.stencil.func = GS_GRAPHICS_STENCIL_FUNC_EQUAL;
                else if (gs_token_compare_text(&token, "LEQUAL"))   pdesc->pip_desc.stencil.func = GS_GRAPHICS_STENCIL_FUNC_LEQUAL;
                else if (gs_token_compare_text(&token, "GREATER"))  pdesc->pip_desc.stencil.func = GS_GRAPHICS_STENCIL_FUNC_GREATER;
                else if (gs_token_compare_text(&token, "NOTEQUAL")) pdesc->pip_desc.stencil.func = GS_GRAPHICS_STENCIL_FUNC_NOTEQUAL;
                else if (gs_token_compare_text(&token, "GEQUAL"))   pdesc->pip_desc.stencil.func = GS_GRAPHICS_STENCIL_FUNC_GEQUAL;
                else if (gs_token_compare_text(&token, "ALWAYS"))   pdesc->pip_desc.stencil.func = GS_GRAPHICS_STENCIL_FUNC_ALWAYS; 
                else if (gs_token_compare_text(&token, "NEVER"))    pdesc->pip_desc.stencil.func = GS_GRAPHICS_STENCIL_FUNC_NEVER; 
                else
                {
                    gs_log_warning("Stencil func type %.*s not valid.", token.len, token.text);
                    return false;
                } 
            }

        }

        // Reference value
        else if (gs_token_compare_text(&token, "ref"))
        { 
            if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_NUMBER))
            {
                gs_log_warning("Stencil reference value not found after decl."); 
                return false;
            }

            else
            {
                token = lex->current_token; 
                gs_snprintfc(TMP, 16, "%.*s", token.len, token.text);
                pdesc->pip_desc.stencil.ref = atoi(TMP); 
            }
        }

        // Component mask
        else if (gs_token_compare_text(&token, "comp_mask"))
        { 
            if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_NUMBER))
            {
                gs_log_warning("Stencil component mask value not found after decl."); 
                return false;
            }

            else
            {
                token = lex->current_token; 
                gs_snprintfc(TMP, 16, "%.*s", token.len, token.text);
                pdesc->pip_desc.stencil.comp_mask = atoi(TMP); 
            }
        }

        // Write mask
        else if (gs_token_compare_text(&token, "write_mask"))
        { 
            if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_NUMBER))
            {
                gs_log_warning("Stencil write mask value not found after decl."); 
                return false;
            }

            else
            {
                token = lex->current_token; 
                gs_snprintfc(TMP, 16, "%.*s", token.len, token.text);
                pdesc->pip_desc.stencil.write_mask = atoi(TMP); 
            }
        }

        // Stencil test failure
        else if (gs_token_compare_text(&token, "sfail"))
        { 
            if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER))
            {
                gs_log_warning("Stencil sfail value not found after decl."); 
                return false;
            }

            else
            {
                token = lex->current_token; 

                if (gs_token_compare_text(&token, "KEEP"))              pdesc->pip_desc.stencil.sfail = GS_GRAPHICS_STENCIL_OP_KEEP;
                else if (gs_token_compare_text(&token, "ZERO"))         pdesc->pip_desc.stencil.sfail = GS_GRAPHICS_STENCIL_OP_ZERO;
                else if (gs_token_compare_text(&token, "REPLACE"))      pdesc->pip_desc.stencil.sfail = GS_GRAPHICS_STENCIL_OP_REPLACE;
                else if (gs_token_compare_text(&token, "INCR"))         pdesc->pip_desc.stencil.sfail = GS_GRAPHICS_STENCIL_OP_INCR;
                else if (gs_token_compare_text(&token, "INCR_WRAP"))    pdesc->pip_desc.stencil.sfail = GS_GRAPHICS_STENCIL_OP_INCR_WRAP;
                else if (gs_token_compare_text(&token, "DECR"))         pdesc->pip_desc.stencil.sfail = GS_GRAPHICS_STENCIL_OP_DECR;
                else if (gs_token_compare_text(&token, "DECR_WRAP"))    pdesc->pip_desc.stencil.sfail = GS_GRAPHICS_STENCIL_OP_DECR_WRAP;
                else if (gs_token_compare_text(&token, "INVERT"))       pdesc->pip_desc.stencil.sfail = GS_GRAPHICS_STENCIL_OP_INVERT;
                else
                {
                    gs_log_warning("Stencil sfail type %.*s not valid.", token.len, token.text);
                    return false;
                } 
            }
        }

        // Stencil test pass, Depth fail
        else if (gs_token_compare_text(&token, "dpfail"))
        { 
            if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER))
            {
                gs_log_warning("Stencil dpfail value not found after decl."); 
                return false;
            }

            else
            {
                token = lex->current_token; 

                if (gs_token_compare_text(&token, "KEEP"))              pdesc->pip_desc.stencil.dpfail = GS_GRAPHICS_STENCIL_OP_KEEP;
                else if (gs_token_compare_text(&token, "ZERO"))         pdesc->pip_desc.stencil.dpfail = GS_GRAPHICS_STENCIL_OP_ZERO;
                else if (gs_token_compare_text(&token, "REPLACE"))      pdesc->pip_desc.stencil.dpfail = GS_GRAPHICS_STENCIL_OP_REPLACE;
                else if (gs_token_compare_text(&token, "INCR"))         pdesc->pip_desc.stencil.dpfail = GS_GRAPHICS_STENCIL_OP_INCR;
                else if (gs_token_compare_text(&token, "INCR_WRAP"))    pdesc->pip_desc.stencil.dpfail = GS_GRAPHICS_STENCIL_OP_INCR_WRAP;
                else if (gs_token_compare_text(&token, "DECR"))         pdesc->pip_desc.stencil.dpfail = GS_GRAPHICS_STENCIL_OP_DECR;
                else if (gs_token_compare_text(&token, "DECR_WRAP"))    pdesc->pip_desc.stencil.dpfail = GS_GRAPHICS_STENCIL_OP_DECR_WRAP;
                else if (gs_token_compare_text(&token, "INVERT"))       pdesc->pip_desc.stencil.dpfail = GS_GRAPHICS_STENCIL_OP_INVERT;
                else
                {
                    gs_log_warning("Stencil dpfail type %.*s not valid.", token.len, token.text);
                    return false;
                } 
            }
        }

        // Stencil test pass, Depth pass
        else if (gs_token_compare_text(&token, "dppass"))
        { 
            if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER))
            {
                gs_log_warning("Stencil dppass value not found after decl."); 
                return false;
            }

            else
            {
                token = lex->current_token; 

                if (gs_token_compare_text(&token, "KEEP"))              pdesc->pip_desc.stencil.dppass = GS_GRAPHICS_STENCIL_OP_KEEP;
                else if (gs_token_compare_text(&token, "ZERO"))         pdesc->pip_desc.stencil.dppass = GS_GRAPHICS_STENCIL_OP_ZERO;
                else if (gs_token_compare_text(&token, "REPLACE"))      pdesc->pip_desc.stencil.dppass = GS_GRAPHICS_STENCIL_OP_REPLACE;
                else if (gs_token_compare_text(&token, "INCR"))         pdesc->pip_desc.stencil.dppass = GS_GRAPHICS_STENCIL_OP_INCR;
                else if (gs_token_compare_text(&token, "INCR_WRAP"))    pdesc->pip_desc.stencil.dppass = GS_GRAPHICS_STENCIL_OP_INCR_WRAP;
                else if (gs_token_compare_text(&token, "DECR"))         pdesc->pip_desc.stencil.dppass = GS_GRAPHICS_STENCIL_OP_DECR;
                else if (gs_token_compare_text(&token, "DECR_WRAP"))    pdesc->pip_desc.stencil.dppass = GS_GRAPHICS_STENCIL_OP_DECR_WRAP;
                else if (gs_token_compare_text(&token, "INVERT"))       pdesc->pip_desc.stencil.dppass = GS_GRAPHICS_STENCIL_OP_INVERT;
                else
                {
                    gs_log_warning("Stencil dppass type %.*s not valid.", token.len, token.text);
                    return false;
                } 
            }
        }
    });
    return true;
}

bool gs_parse_raster(gs_lexer_t* lex, gs_gfxt_pipeline_desc_t* pdesc, gs_ppd_t* ppd)
{
    gs_parse_block(
    PIPELINE::RASTER,
    { 
        // Index Buffer Element Size 
        if (gs_token_compare_text(&token, "index_buffer_element_size"))
        { 
            if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER))
            {
                gs_log_warning("Raster index buffer element size not found.", token.len, token.text);
            }
            
            token = lex->current_token;

            if (gs_token_compare_text(&token, "UINT32") || gs_token_compare_text(&token, "uint32_t") || gs_token_compare_text(&token, "u32"))
            { 
                pdesc->pip_desc.raster.index_buffer_element_size = sizeof(uint32_t); 
            }

            else if (gs_token_compare_text(&token, "UINT16") || gs_token_compare_text(&token, "uint16_t") || gs_token_compare_text(&token, "u16"))
            {
                pdesc->pip_desc.raster.index_buffer_element_size = sizeof(uint16_t); 
            }
            
            // Default
            else
            {
                pdesc->pip_desc.raster.index_buffer_element_size = sizeof(uint32_t); 
            }
        } 

        // Face culling 
        if (gs_token_compare_text(&token, "face_culling"))
        { 
            if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER))
            {
                gs_log_warning("Raster face culling type not found.");
                return false;
            }
            
            token = lex->current_token;

            if (gs_token_compare_text(&token, "FRONT"))               pdesc->pip_desc.raster.face_culling = GS_GRAPHICS_FACE_CULLING_FRONT;
            else if (gs_token_compare_text(&token, "BACK"))           pdesc->pip_desc.raster.face_culling = GS_GRAPHICS_FACE_CULLING_BACK;
            else if (gs_token_compare_text(&token, "FRONT_AND_BACK")) pdesc->pip_desc.raster.face_culling = GS_GRAPHICS_FACE_CULLING_FRONT_AND_BACK;
            else
            {
                gs_log_warning("Raster face culling type %.*s not valid.", token.len, token.text);
                return false;
            }
        } 

        // Winding order 
        if (gs_token_compare_text(&token, "winding_order"))
        { 
            if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER))
            {
                gs_log_warning("Raster winding order type not found.");
                return false;
            }
            
            token = lex->current_token;

            if (gs_token_compare_text(&token, "CW"))         pdesc->pip_desc.raster.winding_order = GS_GRAPHICS_WINDING_ORDER_CW;
            else if (gs_token_compare_text(&token, "CCW"))   pdesc->pip_desc.raster.winding_order = GS_GRAPHICS_WINDING_ORDER_CCW;
            else
            {
                gs_log_warning("Raster winding order type %.*s not valid.", token.len, token.text);
                return false;
            }
        } 

        // Primtive 
        if (gs_token_compare_text(&token, "primitive"))
        { 
            if (!gs_lexer_find_next_token_type(lex, GS_TOKEN_IDENTIFIER))
            {
                gs_log_warning("Raster primitive type not found.");
                return false;
            }
            
            token = lex->current_token;

            if (gs_token_compare_text(&token, "LINES"))            pdesc->pip_desc.raster.primitive = GS_GRAPHICS_PRIMITIVE_LINES;
            else if (gs_token_compare_text(&token, "TRIANGLES"))   pdesc->pip_desc.raster.primitive = GS_GRAPHICS_PRIMITIVE_TRIANGLES;
            else if (gs_token_compare_text(&token, "QUADS"))       pdesc->pip_desc.raster.primitive = GS_GRAPHICS_PRIMITIVE_QUADS;
            else
            {
                gs_log_warning("Raster primitive type %.*s not valid.", token.len, token.text);
                return false;
            }
        } 
    });
    return true;
}

bool gs_parse_pipeline(gs_lexer_t* lex, gs_gfxt_pipeline_desc_t* desc, gs_ppd_t* ppd)
{ 
    // Get next identifier
    while (lex->can_lex(lex))
    {
        gs_token_t token = lex->next_token(lex);
        switch (token.type)
        {
            case GS_TOKEN_IDENTIFIER:
            {
                if (gs_token_compare_text(&token, "shader"))
                {
                    gs_println("parsing shader");
                    if (!gs_parse_shader(lex, desc, ppd))
                    {
                        gs_log_warning("Unable to parse shader descriptor"); 
                        return false;
                    }
                }

                else if (gs_token_compare_text(&token, "raster"))
                {
                    if (!gs_parse_raster(lex, desc, ppd))
                    {
                        gs_log_warning("Unable to parse raster descriptor");
                        return false;
                    }
                }

                else if (gs_token_compare_text(&token, "depth"))
                {
                    if (!gs_parse_depth(lex, desc, ppd))
                    {
                        gs_log_warning("Unable to parse depth descriptor");
                        return false;
                    }
                }

                else if (gs_token_compare_text(&token, "stencil"))
                {
                    if (!gs_parse_stencil(lex, desc, ppd))
                    {
                        gs_log_warning("Unable to parse stencil descriptor");
                        return false;
                    } 
                }

                else if (gs_token_compare_text(&token, "blend"))
                {
                    if (!gs_parse_blend(lex, desc, ppd))
                    {
                        gs_log_warning("Unable to parse blend descriptor");
                        return false;
                    }
                }

            } break;
        }
    }
    return true;
}

char* gs_pipeline_generate_shader_code(gs_gfxt_pipeline_desc_t* pdesc, gs_ppd_t* ppd, gs_graphics_shader_stage_type stage)
{
    gs_println("GENERATING CODE...");

    // Get major/minor version of shader 
    gs_graphics_info_t* ginfo = gs_graphics_info();
    gs_snprintfc(MAJMINSTR, 128, "#version %zu%zu0\n", ginfo->major_version, ginfo->minor_version);

    // Shaders
    #ifdef GS_PLATFORM_WEB
        #define _GS_VERSION_STR "#version 300 es\n"
    #else
        #define _GS_VERSION_STR "#version 430\n"
    #endif

    // Source code 
    char* src = NULL; 
    uint32_t sidx = 0;

    // Set sidx
    switch (stage)
    {
        case GS_GRAPHICS_SHADER_STAGE_VERTEX:   sidx = 0; break;
        case GS_GRAPHICS_SHADER_STAGE_FRAGMENT: sidx = 1; break;
        case GS_GRAPHICS_SHADER_STAGE_COMPUTE:  sidx = 2; break;
    }

    // Early out for now...
    if (!ppd->code[sidx])
    {
        return src;
    }

    // Shader header
    gs_snprintfc(shader_header, 512, "%s precision mediump float;\n", stage == GS_GRAPHICS_SHADER_STAGE_COMPUTE ? "#version 430\n" : _GS_VERSION_STR);

    // Generate shader code
    if (ppd->code[sidx])
    {
        const size_t header_sz = (size_t)gs_string_length(shader_header);
        size_t total_sz = gs_string_length(ppd->code[sidx]) + header_sz + 2048;
        src = (char*)gs_malloc(total_sz); 
        memset(src, 0, total_sz);
        strncat(src, shader_header, header_sz);
        
        // Attributes
        if (stage == GS_GRAPHICS_SHADER_STAGE_VERTEX)
        {
            for (uint32_t i = 0; i < gs_dyn_array_size(pdesc->pip_desc.layout.attrs); ++i)
            { 
                const char* aname = pdesc->pip_desc.layout.attrs[i].name;
                const char* atype = gs_get_vertex_attribute_string(pdesc->pip_desc.layout.attrs[i].format); 

                gs_snprintfc(ATTR, 64, "layout(location = %zu) in %s %s;\n", i, atype, aname);
                const size_t sz = gs_string_length(ATTR);
                strncat(src, ATTR, sz);
            } 
        }

        // Compute shader image buffer binding
        uint32_t img_binding = 0;

        // Uniforms
        for (uint32_t i = 0; i < gs_dyn_array_size(pdesc->ublock_desc.layout); ++i)
        { 
            gs_gfxt_uniform_desc_t* udesc = &pdesc->ublock_desc.layout[i]; 

            if (udesc->stage != stage) continue;

            switch (stage)
            {
                case GS_GRAPHICS_SHADER_STAGE_COMPUTE:
                {
                    // Need to go from uniform type to string
                    const char* utype = gs_uniform_string_from_type(udesc->type);
                    const char* uname = udesc->name;

                    switch (udesc->type)
                    {
                        default:
                        {
                            gs_snprintfc(TMP, 64, "uniform %s %s;\n", utype, uname);
                            const size_t sz = gs_string_length(TMP);
                            strncat(src, TMP, sz);
                        } break;

                        case GS_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F:
                        {
                            gs_snprintfc(TMP, 64, "layout (rgba32f, binding = %zu) uniform image2D %s;\n", img_binding++, uname);
                            const size_t sz = gs_string_length(TMP);
                            strncat(src, TMP, sz);
                        } break;
                    }
                } break;

                default:
                {
                    // Need to go from uniform type to string
                    const char* utype = gs_uniform_string_from_type(udesc->type);
                    const char* uname = udesc->name;
                    gs_snprintfc(TMP, 64, "uniform %s %s;\n", utype, uname);
                    const size_t sz = gs_string_length(TMP);
                    strncat(src, TMP, sz);
                } break;
            }

        }

        // Out
        switch (stage)
        {
            case GS_GRAPHICS_SHADER_STAGE_FRAGMENT:
            case GS_GRAPHICS_SHADER_STAGE_VERTEX:
            {
                for (uint32_t i = 0; i < gs_dyn_array_size(ppd->io_list[sidx]); ++i)
                {
                    gs_shader_io_data_t* out = &ppd->io_list[sidx][i];
                    const char* otype = out->type;
                    const char* oname = out->name;
                    gs_transient_buffer(TMP, 64);
                    if (stage == GS_GRAPHICS_SHADER_STAGE_FRAGMENT)
                    {
                        gs_snprintf(TMP, 64, "layout(location = %zu) out %s %s;\n", i, otype, oname);
                    }
                    else
                    {
                        gs_snprintf(TMP, 64, "out %s %s;\n", otype, oname);
                    }
                    const size_t sz = gs_string_length(TMP);
                    strncat(src, TMP, sz); 
                }
            } break; 

            default: break;
        }

        // In
        switch (stage)
        {
            case GS_GRAPHICS_SHADER_STAGE_FRAGMENT:
            {
                for (uint32_t i = 0; i < gs_dyn_array_size(ppd->io_list[0]); ++i)
                {
                    gs_shader_io_data_t* out = &ppd->io_list[0][i];
                    const char* otype = out->type;
                    const char* oname = out->name;
                    gs_snprintfc(TMP, 64, "in %s %s;\n", otype, oname);
                    const size_t sz = gs_string_length(TMP);
                    strncat(src, TMP, sz); 
                }
            } break;

            case GS_GRAPHICS_SHADER_STAGE_COMPUTE:
            {
                /*
                gs_snprintfc(TMP, 64, "layout(");
                strncat(src, "layout(", 7);

                for (uint32_t i = 0; i < gs_dyn_array_size(ppd->io_list[2]); ++i)
                {
                    gs_shader_io_data_t* out = &ppd->io_list[2][i];
                    const char* otype = out->type;
                    const char* oname = out->name;
                    gs_snprintfc(TMP, 64, "%s = %s%s", otype, oname, i == gs_dyn_array_size(ppd->io_list[2]) - 1 ? "" : ", ");
                    const size_t sz = gs_string_length(TMP);
                    strncat(src, TMP, sz); 
                }

                strncat(src, ") in;\n", 7);
                */
            } break;

            default: break;
        }

        // Code
        { 
            const size_t sz = gs_string_length(ppd->code[sidx]);
            strncat(src, ppd->code[sidx], sz); 
        } 
    } 

    return src;
}

GS_API_DECL gs_gfxt_pipeline_t 
gs_gfxt_pipeline_load_from_file(const char* path)
{
    // Load file, generate lexer off of file data, parse contents for pipeline information 
    size_t len = 0;
    char* file_data = gs_platform_read_file_contents(path, "rb", &len);
    gs_assert(file_data); 
    gs_log_success("Parsing pipeline: %s", path);
    gs_gfxt_pipeline_t pip = gs_gfxt_pipeline_load_from_memory_ext(file_data, len, path);
    gs_free(file_data);
    return pip;
}

GS_API_DECL gs_gfxt_pipeline_t 
gs_gfxt_pipeline_load_from_memory(const char* file_data, size_t sz)
{ 
    return gs_gfxt_pipeline_load_from_memory_ext(file_data, sz, ".");
}

GS_API_DECL gs_gfxt_pipeline_t 
gs_gfxt_pipeline_load_from_memory_ext(const char* file_data, size_t sz, const char* file_path)
{ 
    // Cast to pip
    gs_gfxt_pipeline_t pip = gs_default_val();

    gs_ppd_t ppd = gs_default_val();
    gs_gfxt_pipeline_desc_t pdesc = gs_default_val();
    pdesc.pip_desc.raster.index_buffer_element_size = sizeof(uint32_t); 

    // Determine original file directory from path
    if (file_path)
    {
        gs_lexer_t lex = gs_lexer_c_ctor(file_path);
        gs_token_t tparen = {0};
        while (lex.can_lex(&lex))
        { 
            gs_token_t token = lex.next_token(&lex);

            // Look for last paren + identifier combo
            switch (token.type)
            { 
                case GS_TOKEN_FSLASH:
                case GS_TOKEN_BSLASH:
                { 
                    tparen = token;
                } break;
            }
        }
        // Now save dir
        gs_println("HERE: %zu", tparen.text - file_path);
        memcpy(ppd.dir, file_path, tparen.text - file_path);
        gs_println("PPD_DIR: %s", ppd.dir);
    }

    gs_lexer_t lex = gs_lexer_c_ctor(file_data);
    while (lex.can_lex(&lex))
    {
        gs_token_t token = lex.next_token(&lex);
        switch (token.type)
        {
            case GS_TOKEN_IDENTIFIER:
            {
                if (gs_token_compare_text(&token, "pipeline"))
                {
                    if (!gs_parse_pipeline(&lex, &pdesc, &ppd))
                    {
                        gs_log_warning("Unable to parse pipeline");
                        return pip;
                    }
                }
            } break;
        }
    }

    // Generate vertex shader code
    char* v_src = gs_pipeline_generate_shader_code(&pdesc, &ppd, GS_GRAPHICS_SHADER_STAGE_VERTEX); 
    // gs_println("%s", v_src);

    // Generate fragment shader code
    char* f_src = gs_pipeline_generate_shader_code(&pdesc, &ppd, GS_GRAPHICS_SHADER_STAGE_FRAGMENT); 
    // gs_println("%s", f_src);
    
    // Generate compute shader code (need to check for this first)
    char* c_src = gs_pipeline_generate_shader_code(&pdesc, &ppd, GS_GRAPHICS_SHADER_STAGE_COMPUTE);
    // gs_println("%s", c_src);

    // Construct compute shader
    if (c_src)
    {
        gs_graphics_shader_desc_t sdesc = gs_default_val();
        gs_graphics_shader_source_desc_t source_desc[1] = gs_default_val();
        source_desc[0].type = GS_GRAPHICS_SHADER_STAGE_COMPUTE;
        source_desc[0].source = c_src;
        sdesc.sources = source_desc;
        sdesc.size = 1 * sizeof(gs_graphics_shader_source_desc_t);

        pdesc.pip_desc.compute.shader = gs_graphics_shader_create(&sdesc);    
    } 
    // Construct raster shader
    else
    {
        gs_graphics_shader_desc_t sdesc = gs_default_val();
        gs_graphics_shader_source_desc_t source_desc[2] = gs_default_val();
        source_desc[0].type = GS_GRAPHICS_SHADER_STAGE_VERTEX;
        source_desc[0].source = v_src;
        source_desc[1].type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT;
        source_desc[1].source = f_src;
        sdesc.sources = source_desc;
        sdesc.size = 2 * sizeof(gs_graphics_shader_source_desc_t);

        pdesc.pip_desc.raster.shader = gs_graphics_shader_create(&sdesc);
    } 

    // Set up layout
    pdesc.pip_desc.layout.size = gs_dyn_array_size(pdesc.pip_desc.layout.attrs) * sizeof(gs_graphics_vertex_attribute_desc_t);

    // Set up ublock
    pdesc.ublock_desc.size = gs_dyn_array_size(pdesc.ublock_desc.layout) * sizeof(gs_gfxt_uniform_desc_t); 

    // Create pipeline
    pip = gs_gfxt_pipeline_create(&pdesc);

    // Create mesh layout
    if (ppd.mesh_layout)
    {
        for (uint32_t i = 0; i < gs_dyn_array_size(ppd.mesh_layout); ++i)
        {
            gs_dyn_array_push(pip.mesh_layout, ppd.mesh_layout[i]);
        }
    } 

    // Free all malloc'd data 
    if (v_src) gs_free(v_src);
    if (f_src) gs_free(f_src); 
    if (c_src) gs_free(c_src);
    gs_dyn_array_free(pdesc.ublock_desc.layout);
	gs_dyn_array_free(pdesc.pip_desc.layout.attrs);
    gs_dyn_array_free(ppd.mesh_layout);
	gs_dyn_array_free(ppd.vertex_layout);
    
    for (uint32_t i = 0; i < 3; ++i)
    {
        if (ppd.code[i]) gs_free(ppd.code[i]);
        gs_dyn_array_free(ppd.io_list[i]);
    }

    return pip;
}

GS_API_DECL gs_gfxt_texture_t 
gs_gfxt_texture_load_from_file(const char* path, gs_graphics_texture_desc_t* desc, bool flip, bool keep_data)
{
    gs_asset_texture_t tex = gs_default_val();
    gs_asset_texture_load_from_file(path, &tex, desc, flip, keep_data);
    if (desc) {
        *desc = tex.desc;
    }
    return tex.hndl;
}

GS_API_DECL gs_gfxt_texture_t gs_gfxt_texture_load_from_memory(const char* data, size_t sz, gs_graphics_texture_desc_t* desc, bool flip, bool keep_data)
{
    gs_asset_texture_t tex = gs_default_val(); 
    gs_asset_texture_load_from_memory(data, sz, &tex, desc, flip, keep_data);
    if (desc) {
        *desc = tex.desc;
    }
    return tex.hndl;
}


#endif // GS_GFXT_IMPL 
#endif // GS_GFXT_H



/*
*/






