/*================================================================
	* Copyright: 2020 John Jackson
	* gs_gfxt: Graphics Extension Util for Gunslinger
	* File: gs_gfxt.h
	All Rights Reserved
=================================================================*/

#ifndef __GS_GFXT_H__
#define __GS_GFXT_H__

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

// Raw data retrieval/Handles
#ifndef GS_GFXT_HNDL
	#define GS_GFXT_HNDL void*							// Default handle will just be a pointer to the data itself 
#endif

typedef void* (*gs_gfxt_raw_data_func)(GS_GFXT_HNDL hndl, void* user_data);

#define GS_GFXT_RAW_DATA(FUNC_DESC, T)\
	((T*)(FUNC_DESC)->func((FUNC_DESC)->hndl, (FUNC_DESC)->user_data))

typedef struct gs_gfxt_raw_data_func_desc_t {
	GS_GFXT_HNDL hndl;								// Handle used for retrieving data.
	gs_gfxt_raw_data_func func;						// User defined function for pipeline data retrieval
	void* user_data;								// Optional user data for function
} gs_gfxt_raw_data_func_desc_t;

// Uniforms/Uniform blocks
typedef struct gs_gfxt_uniform_desc_t {
	const char* name;									// Name of uniform (for binding to shader)
	gs_graphics_uniform_type type;						// Type of uniform: GS_GRAPHICS_UNIFORM_VEC2, GS_GRAPHICS_UNIFORM_VEC3, etc.
	uint32_t binding;									// Binding for this uniform in shader
} gs_gfxt_uniform_desc_t;

typedef struct gs_gfxt_uniform_t {
	gs_handle(gs_graphics_uniform_t) hndl;				// Graphics handle resource for actual uniform
	uint32_t offset;									// Individual offset for this uniform in material byte buffer data
	uint32_t binding;									// Binding for this uniform
	size_t size;										// Size of this uniform data in bytes
	gs_graphics_uniform_type type;						// Type of this uniform
} gs_gfxt_uniform_t;

typedef struct gs_gfxt_uniform_block_desc_t {
	gs_gfxt_uniform_desc_t* layout;						// Layout for all uniform data for this block to hold
	size_t layout_size;									// Size of layout in bytes
} gs_gfxt_uniform_block_desc_t;

typedef struct gs_gfxt_uniform_block_t {
	gs_dyn_array(gs_gfxt_uniform_t) uniforms;			// Raw uniform handle array
	gs_hash_table(uint64_t, uint32_t) lookup;			// Index lookup table (used for byte buffer offsets in material uni. data)
	size_t size;										// Total size of material data for entire block
} gs_gfxt_uniform_block_t;

// Pipeline
typedef struct gs_gfxt_pipeline_desc_t {
	gs_graphics_pipeline_desc_t  pip_desc;		// Description for constructing pipeline object
	gs_gfxt_uniform_block_desc_t ublock_desc;	// Description for constructing uniform block object
} gs_gfxt_pipeline_desc_t;

typedef struct gs_gfxt_pipeline_t {
	gs_handle(gs_graphics_pipeline_t) hndl;				// Graphics handle resource for actual pipeline
	gs_gfxt_uniform_block_t ublock;						// Uniform block for holding all uniform data
} gs_gfxt_pipeline_t;

// Material
typedef struct gs_gfxt_material_desc_t {
	gs_gfxt_raw_data_func_desc_t pip_func;	 	// Description for retrieving raw pipeline pointer data from handle.
} gs_gfxt_material_desc_t;

typedef struct gs_gfxt_material_t {
	gs_gfxt_material_desc_t desc;				// Material description object
	gs_byte_buffer_t uniform_data;				// Byte buffer of actual uniform data to send to GPU
} gs_gfxt_material_t;

// Mesh
gs_enum_decl(gs_gfxt_mesh_attribute_type,
    GS_GFXT_MESH_ATTRIBUTE_TYPE_POSITION,
    GS_GFXT_MESH_ATTRIBUTE_TYPE_NORMAL,
    GS_GFXT_MESH_ATTRIBUTE_TYPE_TANGENT,
    GS_GFXT_MESH_ATTRIBUTE_TYPE_JOINT,
    GS_GFXT_MESH_ATTRIBUTE_TYPE_WEIGHT,
    GS_GFXT_MESH_ATTRIBUTE_TYPE_TEXCOORD,
    GS_GFXT_MESH_ATTRIBUTE_TYPE_COLOR
);

typedef struct gs_gfxt_mesh_layout_t {
    gs_gfxt_mesh_attribute_type type;     // Type of attribute
    uint32_t idx;                         // Optional index (for joint/weight/texcoord/color)
} gs_gfxt_mesh_layout_t;

// Structured/packed raw mesh data
typedef struct gs_gfxt_mesh_raw_data_t {
    uint16_t prim_count;
    size_t* vertex_sizes;
    size_t* index_sizes;
    void** vertices;
    void** indices;
} gs_gfxt_mesh_raw_data_t;

typedef struct gs_gfxt_mesh_import_options_t {
    gs_gfxt_mesh_layout_t* layout;        // Mesh attribute layout array
    size_t layout_size;                    // Size of mesh attribute layout array in bytes
    size_t index_buffer_element_size;      // Size of index data size in bytes
} gs_gfxt_mesh_import_options_t;

typedef struct gs_gfxt_mesh_desc_t {
    gs_gfxt_mesh_raw_data_t* meshes;   // Mesh data array
    size_t size;					   // Size of mesh data array in bytes
} gs_gfxt_mesh_desc_t;

typedef struct gs_gfxt_mesh_primitive_t {
    gs_handle(gs_graphics_vertex_buffer_t) vbo;
    gs_handle(gs_graphics_index_buffer_t) ibo;
    uint32_t count; 
} gs_gfxt_mesh_primitive_t;

typedef struct gs_gfxt_mesh_t {
	gs_dyn_array(gs_gfxt_mesh_primitive_t) primitives;
} gs_gfxt_mesh_t;

// Renderable
typedef struct gs_gfxt_renderable_desc_t {
	gs_gfxt_raw_data_func_desc_t mesh;			// Description for retrieving raw mesh pointer data from handle.
	gs_gfxt_raw_data_func_desc_t material;		// Description for retrieving raw material pointer data from handle.
} gs_gfxt_renderable_desc_t;

typedef struct gs_gfxt_renderable_t {
	gs_gfxt_renderable_desc_t desc;			// Renderable description object
	gs_mat4 model_matrix;					// Model matrix for renderable
} gs_gfxt_renderable_t;

// Graphics scene
typedef struct gs_gfxt_graphics_scene_t {
	gs_slot_array(gs_gfxt_renderable_t) renderables;
} gs_gfxt_graphics_scene_t;

/*==== API =====*/

// Creation/Destruction
GS_API_DECL gs_gfxt_pipeline_t 	    gs_gfxt_pipeline_create(gs_gfxt_pipeline_desc_t* desc);
GS_API_DECL gs_gfxt_material_t 		gs_gfxt_material_create(gs_gfxt_material_desc_t* desc);
GS_API_DECL gs_gfxt_mesh_t 			gs_gfxt_mesh_create(gs_gfxt_mesh_desc_t* desc);
GS_API_DECL gs_gfxt_renderable_t 	gs_gfxt_renderable_create(gs_gfxt_renderable_desc_t* desc);
GS_API_DECL gs_gfxt_uniform_block_t gs_gfxt_uniform_block_create(gs_gfxt_uniform_block_desc_t* desc);

// Material API
GS_API_DECL void gs_gfxt_material_set_uniform(gs_gfxt_material_t* mat, const char* name, void* data);
GS_API_DECL void gs_gfxt_material_bind(gs_command_buffer_t* cb, gs_gfxt_material_t* mat);

// Mesh API
GS_API_DECL void gs_gfxt_mesh_draw(gs_command_buffer_t* cb, gs_gfxt_mesh_t* mesh);
GS_API_DECL gs_gfxt_mesh_t gs_gfxt_mesh_create_from_file(const char* file, gs_gfxt_mesh_import_options_t* options);
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
GS_API_DECL 
gs_gfxt_pipeline_t gs_gfxt_pipeline_create(gs_gfxt_pipeline_desc_t* desc)
{
	gs_gfxt_pipeline_t pip = gs_default_val();

	if (!desc) {
		gs_assert(false);
		return pip;
	}

	pip.hndl = gs_graphics_pipeline_create(&desc->pip_desc);
	pip.ublock = gs_gfxt_uniform_block_create(&desc->ublock_desc);
	return pip;
}

GS_API_DECL 
gs_gfxt_uniform_block_t gs_gfxt_uniform_block_create(gs_gfxt_uniform_block_desc_t* desc)
{
	gs_gfxt_uniform_block_t block = gs_default_val();

	if (!desc) return block;

	// Iterate through layout, construct uniforms, place them into hash table
	uint32_t offset = 0;
	uint32_t ct = desc->layout_size / sizeof(gs_gfxt_uniform_desc_t);
	for (uint32_t i = 0; i < ct; ++i)
	{
		gs_gfxt_uniform_desc_t* ud = &desc->layout[i];

		gs_gfxt_uniform_t u = {
			.hndl = gs_graphics_uniform_create (
		        &(gs_graphics_uniform_desc_t) {
		            .name = ud->name,
		            .layout = &(gs_graphics_uniform_layout_desc_t){.type = ud->type}
		        }
			),
			.binding = ud->binding,
			.offset = offset,
			.type = ud->type
		};

		// Add to data offset based on type
		switch (ud->type) {
			default:
		    case GS_GRAPHICS_UNIFORM_FLOAT:		offset += sizeof(float); break;
		    case GS_GRAPHICS_UNIFORM_INT:		offset += sizeof(int32_t); break;
		    case GS_GRAPHICS_UNIFORM_VEC2:		offset += sizeof(gs_vec2); break;
		    case GS_GRAPHICS_UNIFORM_VEC3:		offset += sizeof(gs_vec3); break;
		    case GS_GRAPHICS_UNIFORM_VEC4:		offset += sizeof(gs_vec4); break;
		    case GS_GRAPHICS_UNIFORM_MAT4:		offset += sizeof(gs_mat4); break;
		    case GS_GRAPHICS_UNIFORM_SAMPLER2D: offset += sizeof(gs_handle(gs_graphics_texture_t)); break;
		}

		// Add uniform to block with name as key
		gs_dyn_array_push(block.uniforms, u);
		gs_hash_table_insert(block.lookup, gs_hash_str64(ud->name), gs_dyn_array_size(block.uniforms) - 1);
	}
	block.size = offset;

	return block;
}

GS_API_DECL 
gs_gfxt_material_t gs_gfxt_material_create(gs_gfxt_material_desc_t* desc)
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

	gs_byte_buffer_resize(&mat.uniform_data, pip->ublock.size);
	gs_byte_buffer_memset(&mat.uniform_data, 0);
	return mat;
}

GS_API_DECL 
gs_gfxt_mesh_t gs_gfxt_mesh_create(gs_gfxt_mesh_desc_t* desc)
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

        for (uint32_t p = 0; p < (uint32_t)m->prim_count; ++p)
        {
            // Construct primitive
            gs_gfxt_mesh_primitive_t prim = gs_default_val();
            prim.count = m->index_sizes[p] / sizeof(uint16_t);

            // Vertex buffer decl
            gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
            vdesc.data = m->vertices[p];
            vdesc.size = m->vertex_sizes[p];

            // Construct vertex buffer for primitive
            prim.vbo = gs_graphics_vertex_buffer_create(&vdesc);

            // Index buffer decl
            gs_graphics_index_buffer_desc_t idesc = gs_default_val();
            idesc.data = m->indices[p];
            idesc.size = m->index_sizes[p];

            // Construct index buffer for primitive
            prim.ibo = gs_graphics_index_buffer_create(&idesc);

            // Add primitive to mesh
            gs_dyn_array_push(mesh.primitives, prim);
        } 
    }

	return mesh;
}

GS_API_DECL 
gs_gfxt_renderable_t gs_gfxt_renderable_create(gs_gfxt_renderable_desc_t* desc)
{
	gs_gfxt_renderable_t rend = gs_default_val();

	if (!desc) {
		return rend;
	}

	rend.model_matrix = gs_mat4_identity();
	rend.desc = *desc;

	return rend;
}

// Material API
GS_API_DECL
void gs_gfxt_material_set_uniform(gs_gfxt_material_t* mat, const char* name, void* data)
{
	if (!mat || !name || !data) return;

	gs_gfxt_pipeline_t* pip = GS_GFXT_RAW_DATA(&mat->desc.pip_func, gs_gfxt_pipeline_t);
	gs_assert(pip);

	u64 hash = gs_hash_str64(name);
	if (!gs_hash_table_key_exists(pip->ublock.lookup, hash)) return;

	// Based on name, need to get uniform
	uint32_t uidx = gs_hash_table_get(pip->ublock.lookup, hash);
	gs_gfxt_uniform_t* u = &pip->ublock.uniforms[uidx];

	// Seek to offset
	gs_byte_buffer_seek_to_beg(&mat->uniform_data);	
	gs_byte_buffer_advance_position(&mat->uniform_data, u->offset);

	switch (u->type)
	{
	    case GS_GRAPHICS_UNIFORM_FLOAT: gs_byte_buffer_write(&mat->uniform_data, float, *(float*)data); 	break;
	    case GS_GRAPHICS_UNIFORM_INT: 	gs_byte_buffer_write(&mat->uniform_data, int32_t, *(int32_t*)data); break;
	    case GS_GRAPHICS_UNIFORM_VEC2:  gs_byte_buffer_write(&mat->uniform_data, gs_vec2, *(gs_vec2*)data); break;
	    case GS_GRAPHICS_UNIFORM_VEC3:  gs_byte_buffer_write(&mat->uniform_data, gs_vec3, *(gs_vec3*)data); break;
	    case GS_GRAPHICS_UNIFORM_VEC4:  gs_byte_buffer_write(&mat->uniform_data, gs_vec4, *(gs_vec4*)data); break;
	    case GS_GRAPHICS_UNIFORM_MAT4:  gs_byte_buffer_write(&mat->uniform_data, gs_mat4, *(gs_mat4*)data); break;
	    case GS_GRAPHICS_UNIFORM_SAMPLER2D: {
	    	gs_byte_buffer_write(&mat->uniform_data, gs_handle(gs_graphics_texture_t), *(gs_handle(gs_graphics_texture_t)*)data);
	    } break;
	}
}

GS_API_DECL 
void gs_gfxt_material_bind(gs_command_buffer_t* cb, gs_gfxt_material_t* mat)
{
	if (!mat) return;

	gs_gfxt_pipeline_t* pip = GS_GFXT_RAW_DATA(&mat->desc.pip_func, gs_gfxt_pipeline_t);
	gs_assert(pip);

	// Grab uniform layout from pipeline
	for (uint32_t i = 0; i < gs_dyn_array_size(pip->ublock.uniforms); ++i) {
		gs_gfxt_uniform_t* u = &pip->ublock.uniforms[i];
		gs_graphics_bind_desc_t bind = {
			.uniforms = {.desc = &(gs_graphics_bind_uniform_desc_t){
				.uniform = u->hndl, .data = (mat->uniform_data.data + u->offset), .binding = u->binding
			}}
		};
		gs_graphics_apply_bindings(cb, &bind);
	}
}

// Mesh API
GS_API_DECL 
void gs_gfxt_mesh_draw(gs_command_buffer_t* cb, gs_gfxt_mesh_t* mp)
{
    // For each primitive in mesh
    for (uint32_t i = 0; i < gs_dyn_array_size(mp->primitives); ++i)
    {
        gs_gfxt_mesh_primitive_t* prim = &mp->primitives[i];

        // Bindings for all buffers: vertex, index, uniform, sampler
        gs_graphics_bind_desc_t binds = {
            .vertex_buffers = {.desc = &(gs_graphics_bind_vertex_buffer_desc_t){.buffer = prim->vbo}},
            .index_buffers = {.desc = &(gs_graphics_bind_index_buffer_desc_t){.buffer = prim->ibo}}
        };

        gs_graphics_apply_bindings(cb, &binds);
        gs_graphics_draw(cb, &(gs_graphics_draw_desc_t){.start = 0, .count = prim->count});
    }
}

// Util API
GS_API_DECL
void* gs_gfxt_raw_data_default_impl(GS_GFXT_HNDL hndl, void* user_data)
{
	return hndl;
}

GS_API_DECL 
gs_gfxt_mesh_t gs_gfxt_mesh_create_from_file(const char* path, gs_gfxt_mesh_import_options_t* options)
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
    else {
        gs_println("Warning:GFXT:MeshLoadFromFile:File extension not supported: %s, file: %s", file_ext, path);
        return mesh;
    }

    // Construct mesh from raw data
    mesh = gs_gfxt_mesh_create(&(gs_gfxt_mesh_desc_t) {
    	.meshes = meshes, 
    	.size = mesh_count * sizeof(gs_gfxt_mesh_raw_data_t)
    });

    return mesh;
}

GS_API_DECL 
bool gs_gfxt_load_gltf_data_from_file(const char* path, gs_gfxt_mesh_import_options_t* options, gs_gfxt_mesh_raw_data_t** out, uint32_t* mesh_count)
{
    // Use cgltf like a boss
    cgltf_options cgltf_options = gs_default_val();
    size_t len = 0;
    char* file_data = gs_platform_read_file_contents(path, "rb", &len);
    gs_println("GFXT:Loading GLTF: %s", path);

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
    gs_dyn_array(gs_color_t) colors = NULL;
    gs_dyn_array(gs_vec2) uvs = NULL;
    gs_dyn_array(gs_gfxt_mesh_layout_t) layouts = NULL;
    gs_byte_buffer_t v_data = gs_byte_buffer_new();
    gs_byte_buffer_t i_data = gs_byte_buffer_new();

    // Allocate memory for buffers
    *mesh_count = data->meshes_count;
    *out = (gs_gfxt_mesh_raw_data_t*)gs_malloc(data->meshes_count * sizeof(gs_gfxt_mesh_raw_data_t));
    memset(*out, 0, sizeof(gs_gfxt_mesh_raw_data_t) * data->meshes_count);

    // Iterate through meshes in data
    for (uint32_t i = 0; i < data->meshes_count; ++i)
    {
        // Initialize mesh data
        gs_gfxt_mesh_raw_data_t* mesh = &((*out)[i]);
        mesh->prim_count = data->meshes[i].primitives_count;
        mesh->vertex_sizes = (size_t*)gs_malloc(sizeof(size_t) * mesh->prim_count);
        mesh->index_sizes = (size_t*)gs_malloc(sizeof(size_t) * mesh->prim_count);
        mesh->vertices = (void**)gs_malloc(sizeof(size_t) * mesh->prim_count);
        mesh->indices = (void**)gs_malloc(sizeof(size_t) * mesh->prim_count);

        // For each primitive in mesh 
        for (uint32_t p = 0; p < data->meshes[i].primitives_count; ++p)
        {
            // Clear temp data from previous use
            gs_dyn_array_clear(positions);
            gs_dyn_array_clear(normals);
            gs_dyn_array_clear(tangents);
            gs_dyn_array_clear(uvs);
            gs_dyn_array_clear(colors);
            gs_dyn_array_clear(layouts);
            gs_byte_buffer_clear(&v_data);
            gs_byte_buffer_clear(&i_data);

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
            for (uint32_t a = 0; a < data->meshes[i].primitives[p].attributes_count; ++a)
            {
                // Accessor for attribute data
                cgltf_accessor* attr = data->meshes[i].primitives[p].attributes[a].data;

                // Switch on type for reading data
                switch (data->meshes[i].primitives[p].attributes[a].type)
                {
                    case cgltf_attribute_type_position: {
                        __GFXT_GLTF_PUSH_ATTR(attr, float, 3, positions, gs_vec3, layouts, GS_GFXT_MESH_ATTRIBUTE_TYPE_POSITION);
                    } break;

                    case cgltf_attribute_type_normal: {
                        __GFXT_GLTF_PUSH_ATTR(attr, float, 3, normals, gs_vec3, layouts, GS_GFXT_MESH_ATTRIBUTE_TYPE_NORMAL);
                    } break;

                    case cgltf_attribute_type_tangent: {
                        __GFXT_GLTF_PUSH_ATTR(attr, float, 3, tangents, gs_vec3, layouts, GS_GFXT_MESH_ATTRIBUTE_TYPE_TANGENT);
                    } break;

                    case cgltf_attribute_type_texcoord: {
                        __GFXT_GLTF_PUSH_ATTR(attr, float, 2, uvs, gs_vec2, layouts, GS_GFXT_MESH_ATTRIBUTE_TYPE_TEXCOORD);
                    } break;

                    case cgltf_attribute_type_color: {
                        __GFXT_GLTF_PUSH_ATTR(attr, uint8_t, 4, colors, gs_color_t, layouts, GS_GFXT_MESH_ATTRIBUTE_TYPE_COLOR);
                    } break;

                    // Not sure what to do with these for now
                    case cgltf_attribute_type_joints: 
                    {
                        // Push into layout
                        gs_gfxt_mesh_layout_t layout = gs_default_val();
                        layout.type = GS_GFXT_MESH_ATTRIBUTE_TYPE_JOINT;
                        gs_dyn_array_push(layouts, layout);
                    } break;

                    case cgltf_attribute_type_weights:
                    {
                        // Push into layout
                        gs_gfxt_mesh_layout_t layout = gs_default_val();
                        layout.type = GS_GFXT_MESH_ATTRIBUTE_TYPE_WEIGHT;
                        gs_dyn_array_push(layouts, layout);
                    } break;

                    // Shouldn't hit here...   
                    default: {
                    } break;
                }
            }

            // Indices for primitive
            cgltf_accessor* acc = data->meshes[i].primitives[p].indices;

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

            bool warnings[gs_enum_count(gs_gfxt_mesh_attribute_type)] = gs_default_val();

            // Grab mesh layout pointer to use
            gs_gfxt_mesh_layout_t* layoutp = options ? options->layout : layouts;
            uint32_t layout_ct = options ? options->layout_size / sizeof(gs_gfxt_mesh_layout_t) : gs_dyn_array_size(layouts);

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
                        /* Grab data at index, if available */\
                        if (IT < gs_dyn_array_size(ARR)) {\
                            gs_byte_buffer_write(&(VDATA), ARR_TYPE, ARR[IT]);\
                        }\
                        else {\
                            /* Write default value and give warning.*/\
                            gs_byte_buffer_write(&(VDATA), ARR_TYPE, ARR_DEF_VAL);\
                            if (!warnings[LAYOUT_TYPE]) {\
                                gs_println("Warning:Mesh:LoadFromFile:%s:Index out of range.", #LAYOUT_TYPE);\
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
                            case GS_GFXT_MESH_ATTRIBUTE_TYPE_POSITION: {
                                __GLTF_WRITE_DATA(it, v_data, positions, gs_vec3, gs_v3(0.f, 0.f, 0.f), GS_GFXT_MESH_ATTRIBUTE_TYPE_POSITION); 
                            } break;

                            case GS_GFXT_MESH_ATTRIBUTE_TYPE_TEXCOORD: {
                                __GLTF_WRITE_DATA(it, v_data, uvs, gs_vec2, gs_v2(0.f, 0.f), GS_GFXT_MESH_ATTRIBUTE_TYPE_TEXCOORD); 
                            } break;

                            case GS_GFXT_MESH_ATTRIBUTE_TYPE_COLOR: {
                                __GLTF_WRITE_DATA(it, v_data, colors, gs_color_t, GS_COLOR_WHITE, GS_GFXT_MESH_ATTRIBUTE_TYPE_COLOR); 
                            } break;

                            case GS_GFXT_MESH_ATTRIBUTE_TYPE_NORMAL: {
                                __GLTF_WRITE_DATA(it, v_data, normals, gs_vec3, gs_v3(0.f, 0.f, 1.f), GS_GFXT_MESH_ATTRIBUTE_TYPE_NORMAL); 
                            } break;

                            case GS_GFXT_MESH_ATTRIBUTE_TYPE_TANGENT: {
                                __GLTF_WRITE_DATA(it, v_data, tangents, gs_vec3, gs_v3(0.f, 1.f, 0.f), GS_GFXT_MESH_ATTRIBUTE_TYPE_TANGENT); 
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
        }
    }

    // Free all data at the end
    cgltf_free(data);
    gs_dyn_array_free(positions);
    gs_dyn_array_free(normals);
    gs_dyn_array_free(tangents);
    gs_dyn_array_free(colors);
    gs_dyn_array_free(uvs);
    gs_dyn_array_free(layouts);
    gs_byte_buffer_free(&v_data);
    gs_byte_buffer_free(&i_data);
    return true;
}

GS_API_DECL 
gs_gfxt_mesh_t gs_gfxt_mesh_unit_quad_generate(gs_gfxt_mesh_import_options_t* options)
{
	gs_gfxt_mesh_t mesh = {0};

	gs_vec3 v_pos[] = {
	    // Positions
	    gs_v3(-0.5f, -0.5f, 0.f), // Top Left
	    gs_v3(0.5f, -0.5f, 0.f),  // Top Right 
	    gs_v3(-0.5f,  0.5f, 0.f), // Bottom Left
	    gs_v3(0.5f,  0.5f, 0.f)   // Bottom Right
	};

	// Vertex data for quad
	gs_vec2 v_uvs[] = {
	    // Positions  UVs
	    gs_v2(0.0f, 0.0f),  // Top Left
	    gs_v2(1.0f, 0.0f),  // Top Right 
	    gs_v2(0.0f, 1.0f),  // Bottom Left
	    gs_v2(1.0f, 1.0f)   // Bottom Right
	};

	gs_vec3 norm = gs_v3(0.f, 0.f, 1.f);
	gs_vec3 tan =  gs_v3(1.f, 0.f, 0.f);
	gs_color_t color = GS_COLOR_WHITE;

	// Index data for quad
	uint32_t i_data[] = {
	    0, 3, 2,    // First Triangle
	    0, 1, 3     // Second Triangle
	};

	// Generate necessary vertex data to push vertex buffer
	gs_gfxt_mesh_primitive_t prim = {0};
	prim.count = 6;

	gs_gfxt_mesh_import_options_t def_options = {
		.layout = (gs_gfxt_mesh_layout_t[]) {
			{.type = GS_GFXT_MESH_ATTRIBUTE_TYPE_POSITION},
			{.type = GS_GFXT_MESH_ATTRIBUTE_TYPE_TEXCOORD}
		},
		.layout_size = 2 * sizeof(gs_gfxt_mesh_layout_t),
		.index_buffer_element_size = sizeof(uint32_t)
	};

	// If no decl, then just use default layout
	gs_gfxt_mesh_import_options_t* moptions = options ? options : &def_options; 
	uint32_t ct = moptions->layout_size / sizeof(gs_asset_mesh_layout_t);
	gs_byte_buffer_t vbuffer = gs_byte_buffer_new();

	// For each vertex
	for (uint32_t v = 0; v < 4; ++v) {
		for (uint32_t i = 0; i < ct; ++i) {
			switch (moptions->layout[i].type) {
				case GS_GFXT_MESH_ATTRIBUTE_TYPE_POSITION:  gs_byte_buffer_write(&vbuffer, gs_vec3, v_pos[v]);  break;
				case GS_GFXT_MESH_ATTRIBUTE_TYPE_NORMAL: 	gs_byte_buffer_write(&vbuffer, gs_vec3, norm); 		break;
				case GS_GFXT_MESH_ATTRIBUTE_TYPE_TEXCOORD:  gs_byte_buffer_write(&vbuffer, gs_vec2, v_uvs[v]);  break;
				case GS_GFXT_MESH_ATTRIBUTE_TYPE_COLOR: 	gs_byte_buffer_write(&vbuffer, gs_color_t, color);  break;
				case GS_GFXT_MESH_ATTRIBUTE_TYPE_TANGENT:   gs_byte_buffer_write(&vbuffer, gs_vec3, tan);  		break;
				default: break;
			}
		}
	}

	// Construct primitive for mesh
	prim.vbo = gs_graphics_vertex_buffer_create(
		&(gs_graphics_vertex_buffer_desc_t) {
			.data = vbuffer.data,
			.size = vbuffer.size
		}
	);

	prim.ibo = gs_graphics_index_buffer_create(
		&(gs_graphics_index_buffer_desc_t) {
			.data = i_data,
			.size = sizeof(i_data)
		}
	);

	// Add primitive
	gs_dyn_array_push(mesh.primitives, prim);

	// Free buffer
	gs_byte_buffer_free(&vbuffer);

	return mesh;
}

gs_handle(gs_graphics_texture_t) gs_gfxt_texture_generate_default()
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

    gs_graphics_texture_desc_t desc = {
        .width = GS_GFXT_ROW_COL_CT,
        .height = GS_GFXT_ROW_COL_CT,
        .format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
        .min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, 
        .mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, 
        .wrap_s = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,
        .wrap_t = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,
        .data = pixels
    };

    // Create dynamic texture
    return gs_graphics_texture_create(&desc);
}

#endif // GS_GFXT_IMPL

#endif // __GS_GFXT_H__



/*
*/






