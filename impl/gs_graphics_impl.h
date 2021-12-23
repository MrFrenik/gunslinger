
/*================================================================
    * Copyright: 2020 John Jackson 
    * File: gs_graphics_impl.h
    All Rights Reserved
=================================================================*/

#ifndef GS_GRAPHICS_IMPL_H
#define GS_GRAPHICS_IMPL_H

#ifndef GS_GRAPHICS_IMPL_CUSTOM
    #define GS_GRAPHICS_IMPL_DEFAULT
#endif

#ifdef GS_GRAPHICS_IMPL_DEFAULT

// Default stuff here, if any...

/* Graphics Info Object Query */
gs_graphics_info_t* gs_graphics_info()
{
    return &gs_engine_subsystem(graphics)->info;
}

#endif

#if (defined GS_GRAPHICS_IMPL_OPENGL_CORE || defined GS_GRAPHICS_IMPL_OPENGL_ES)

#ifdef GS_GRAPHICS_IMPL_OPENGL_CORE
    #define CHECK_GL_CORE(...) __VA_ARGS__
#else
    #define CHECK_GL_CORE(...) gs_empty_instruction(void)
#endif

typedef enum gsgl_uniform_type
{
    GSGL_UNIFORMTYPE_FLOAT,
    GSGL_UNIFORMTYPE_INT,
    GSGL_UNIFORMTYPE_VEC2,
    GSGL_UNIFORMTYPE_VEC3,
    GSGL_UNIFORMTYPE_VEC4,
    GSGL_UNIFORMTYPE_MAT4,
    GSGL_UNIFORMTYPE_SAMPLER2D
} gsgl_uniform_type;

/* Uniform (stores samplers as well as primitive uniforms) */
typedef struct gsgl_uniform_t {
    char name[64];               // Name of uniform to find location
    gsgl_uniform_type type;         // Type of uniform data
    uint32_t location;              // Location of uniform
    size_t size;                    // Total data size of uniform
    uint32_t sid;                   // Shader id (should probably inverse this, but I don't want to force a map lookup)
    uint32_t count;                 // Count (used for arrays)
} gsgl_uniform_t;

// When a user passes in a uniform layout, that handle could then pass to a WHOLE list of uniforms (if describing a struct)
typedef struct gsgl_uniform_list_t {
    size_t size;                                // Total size of uniform data
    char name[64];                           // Base name of uniform
    gs_dyn_array(gsgl_uniform_t) uniforms;      // Individual uniforms in list
} gsgl_uniform_list_t;

typedef struct gsgl_uniform_buffer_t {
    char name[64];
    uint32_t location;
    size_t size;
    uint32_t ubo;
    uint32_t sid;
} gsgl_uniform_buffer_t;

/* Pipeline */
typedef struct gsgl_pipeline_t {
    gs_graphics_blend_state_desc_t blend;
    gs_graphics_depth_state_desc_t depth;
    gs_graphics_raster_state_desc_t raster;
    gs_graphics_stencil_state_desc_t stencil;
    gs_graphics_compute_state_desc_t compute;
    gs_dyn_array(gs_graphics_vertex_attribute_desc_t) layout;
} gsgl_pipeline_t;

/* Render Pass */
typedef struct gsgl_render_pass_t {
    gs_handle(gs_graphics_framebuffer_t) fbo;                        
    gs_dyn_array(gs_handle(gs_graphics_texture_t)) color;
    gs_handle(gs_graphics_texture_t) depth; 
    gs_handle(gs_graphics_texture_t) stencil;
} gsgl_render_pass_t;

/* Shader */
typedef uint32_t gsgl_shader_t;

/* Gfx Buffer */
typedef uint32_t gsgl_buffer_t;

/* Texture */
typedef struct gsgl_texture_t {
    uint32_t id;
    gs_graphics_texture_desc_t desc;
} gsgl_texture_t;

typedef struct gsgl_vertex_buffer_decl_t {
    gsgl_buffer_t vbo;
    gs_graphics_vertex_data_type data_type;
    size_t offset;
} gsgl_vertex_buffer_decl_t;

/* Cached data between draws */
typedef struct gsgl_data_cache_t
{
    gsgl_buffer_t vao;
    gsgl_buffer_t ibo;
    size_t ibo_elem_sz;
    gs_dyn_array(gsgl_vertex_buffer_decl_t) vdecls;
    gs_handle(gs_graphics_pipeline_t) pipeline;
} gsgl_data_cache_t;

/* Internal Opengl Data */
typedef struct gsgl_data_t
{
    gs_slot_array(gsgl_shader_t)        shaders;
    gs_slot_array(gsgl_texture_t)       textures;
    gs_slot_array(gsgl_buffer_t)        vertex_buffers;
    gs_slot_array(gsgl_uniform_buffer_t) uniform_buffers;
    gs_slot_array(gsgl_buffer_t)        index_buffers;
    gs_slot_array(gsgl_buffer_t)        frame_buffers;
    gs_slot_array(gsgl_uniform_list_t)  uniforms;
    gs_slot_array(gsgl_pipeline_t)      pipelines;
    gs_slot_array(gsgl_render_pass_t)   render_passes;

    // All the required uniform data for strict aliasing.
    struct {
        gs_dyn_array(uint32_t)  ui32; 
        gs_dyn_array(int32_t)   i32; 
        gs_dyn_array(float)     flt; 
        gs_dyn_array(gs_vec2)   vec2; 
        gs_dyn_array(gs_vec3)   vec3; 
        gs_dyn_array(gs_vec4)   vec4; 
        gs_dyn_array(gs_mat4)   mat4; 
    } uniform_data;

    // Cached data between draw calls (to minimize state changes)
    gsgl_data_cache_t cache;

} gsgl_data_t;

/* Internal OGL Command Buffer Op Code */
typedef enum gs_opengl_op_code_type
{
    GS_OPENGL_OP_BEGIN_RENDER_PASS = 0x00,
    GS_OPENGL_OP_END_RENDER_PASS,
    GS_OPENGL_OP_SET_VIEWPORT,
    GS_OPENGL_OP_SET_VIEW_SCISSOR,
    GS_OPENGL_OP_CLEAR,
    GS_OPENGL_OP_REQUEST_BUFFER_UPDATE,
    GS_OPENGL_OP_REQUEST_TEXTURE_UPDATE,
    GS_OPENGL_OP_BIND_PIPELINE,
    GS_OPENGL_OP_APPLY_BINDINGS,
    GS_OPENGL_OP_DISPATCH_COMPUTE,
    GS_OPENGL_OP_DRAW,
} gs_opengl_op_code_type;

void gsgl_reset_data_cache(gsgl_data_cache_t* cache)
{
    cache->ibo = 0;
    cache->ibo_elem_sz = 0;
    cache->pipeline = gs_handle_invalid(gs_graphics_pipeline_t);
    gs_dyn_array_clear(cache->vdecls);
}

void gsgl_pipeline_state()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);

    CHECK_GL_CORE(
        gs_graphics_info_t* info = gs_graphics_info();
        if (info->compute.available) {
            glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        }
    )
}

/* GS/OGL Utilities */
int32_t gsgl_buffer_usage_to_gl_enum(gs_graphics_buffer_usage_type type)
{
    int32_t mode = GL_STATIC_DRAW;
    switch (type) {
        default:
        case GS_GRAPHICS_BUFFER_USAGE_STATIC: mode = GL_STATIC_DRAW; break;
        case GS_GRAPHICS_BUFFER_USAGE_STREAM: mode = GL_STREAM_DRAW; break;
        case GS_GRAPHICS_BUFFER_USAGE_DYNAMIC: mode = GL_DYNAMIC_DRAW; break;
    }   
    return mode;
}

uint32_t gsgl_access_type_to_gl_access_type(gs_graphics_access_type type)
{
    CHECK_GL_CORE(
        uint32_t access = GL_WRITE_ONLY;
        switch (type)
        {
            case GS_GRAPHICS_ACCESS_WRITE_ONLY:  access = GL_WRITE_ONLY;  break;
            case GS_GRAPHICS_ACCESS_READ_ONLY:   access = GL_READ_ONLY;   break;
            case GS_GRAPHICS_ACCESS_READ_WRITE:  access = GL_READ_WRITE;  break;
            default: break;
        }
        return access;
    )
    return 0;
}

uint32_t gsgl_texture_wrap_to_gl_texture_wrap(gs_graphics_texture_wrapping_type type)
{
    uint32_t wrap = GL_REPEAT;
    switch (type) {
        default:
        case GS_GRAPHICS_TEXTURE_WRAP_REPEAT:           wrap = GL_REPEAT;           break;
        case GS_GRAPHICS_TEXTURE_WRAP_MIRRORED_REPEAT:  wrap = GL_MIRRORED_REPEAT;  break;
        case GS_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE:    wrap = GL_CLAMP_TO_EDGE;    break;
        CHECK_GL_CORE(case GS_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_BORDER:  wrap = GL_CLAMP_TO_BORDER;  break;)
    };

    return wrap;
}

uint32_t gsgl_texture_format_to_gl_data_type(gs_graphics_texture_format_type type)
{
    uint32_t format = GL_UNSIGNED_BYTE;
    switch (type) {
        default:
        case GS_GRAPHICS_TEXTURE_FORMAT_A8:                 format = GL_UNSIGNED_BYTE; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_R8:                 format = GL_UNSIGNED_BYTE; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGB8:               format = GL_UNSIGNED_BYTE; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA8:              format = GL_UNSIGNED_BYTE; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA16F:            format = GL_FLOAT; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA32F:            format = GL_FLOAT; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH8:             format = GL_FLOAT; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH16:            format = GL_FLOAT; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH24:            format = GL_FLOAT; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:           format = GL_FLOAT; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:   format = GL_UNSIGNED_INT_24_8; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:  format = GL_FLOAT_32_UNSIGNED_INT_24_8_REV; break;
    }
    return format;
}

uint32_t gsgl_texture_format_to_gl_texture_format(gs_graphics_texture_format_type type)
{
    uint32_t dt = GL_RGBA;
    switch (type)
    {
        default:
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA8:              dt = GL_RGBA; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_A8:                 dt = GL_ALPHA; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_R8:                 dt = GL_RED; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGB8:               dt = GL_RGB; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA16F:            dt = GL_RGBA; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA32F:            dt = GL_RGBA; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH8:             dt = GL_DEPTH_COMPONENT; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH16:            dt = GL_DEPTH_COMPONENT; break;              
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH24:            dt = GL_DEPTH_COMPONENT; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:           dt = GL_DEPTH_COMPONENT; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:   dt = GL_DEPTH_STENCIL;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:  dt = GL_DEPTH_STENCIL; break;
    }
    return dt;
}


uint32_t gsgl_texture_format_to_gl_texture_internal_format(gs_graphics_texture_format_type type)
{
    uint32_t format = GL_UNSIGNED_BYTE;
    switch (type) {
        case GS_GRAPHICS_TEXTURE_FORMAT_A8:                 format = GL_ALPHA; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_R8:                 format = GL_RED; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGB8:               format = GL_RGB8; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA8:              format = GL_RGBA8; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA16F:            format = GL_RGBA16F; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA32F:            format = GL_RGBA32F; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH8:             format = GL_DEPTH_COMPONENT; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH16:            format = GL_DEPTH_COMPONENT16; break;              
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH24:            format = GL_DEPTH_COMPONENT24; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:           format = GL_DEPTH_COMPONENT32F; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:   format = GL_DEPTH24_STENCIL8; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:  format = GL_DEPTH32F_STENCIL8; break;
    }
    return format;
}

uint32_t gsgl_shader_stage_to_gl_stage(gs_graphics_shader_stage_type type)
{
    uint32_t stage = GL_VERTEX_SHADER;
    switch (type) {
        default:
        case GS_GRAPHICS_SHADER_STAGE_VERTEX: stage = GL_VERTEX_SHADER; break;
        case GS_GRAPHICS_SHADER_STAGE_FRAGMENT: stage = GL_FRAGMENT_SHADER; break;
        CHECK_GL_CORE(case GS_GRAPHICS_SHADER_STAGE_COMPUTE: stage = GL_COMPUTE_SHADER; break;)
    }
    return stage;
}

uint32_t gsgl_primitive_to_gl_primitive(gs_graphics_primitive_type type)
{
    uint32_t prim = GL_TRIANGLES;   
    switch (type) {
        default:
        case GS_GRAPHICS_PRIMITIVE_TRIANGLES: prim = GL_TRIANGLES; break;
        case GS_GRAPHICS_PRIMITIVE_LINES: prim = GL_LINES; break;
        CHECK_GL_CORE(case GS_GRAPHICS_PRIMITIVE_QUADS: prim = GL_QUADS; break;)
    }
    return prim;
}

uint32_t gsgl_blend_equation_to_gl_blend_eq(gs_graphics_blend_equation_type eq)
{
    uint32_t beq = GL_FUNC_ADD; 
    switch (eq) {
        default:
        case GS_GRAPHICS_BLEND_EQUATION_ADD:                beq = GL_FUNC_ADD; break;
        case GS_GRAPHICS_BLEND_EQUATION_SUBTRACT:           beq = GL_FUNC_SUBTRACT; break;
        case GS_GRAPHICS_BLEND_EQUATION_REVERSE_SUBTRACT:   beq = GL_FUNC_REVERSE_SUBTRACT; break;
        case GS_GRAPHICS_BLEND_EQUATION_MIN:                beq = GL_MIN; break;
        case GS_GRAPHICS_BLEND_EQUATION_MAX:                beq = GL_MAX; break;
    };

    return beq;
}

uint32_t gsgl_blend_mode_to_gl_blend_mode(gs_graphics_blend_mode_type type, uint32_t def)
{
    uint32_t mode = def;    
    switch (type) {
        case GS_GRAPHICS_BLEND_MODE_ZERO: mode = GL_ZERO; break;
        case GS_GRAPHICS_BLEND_MODE_ONE: mode = GL_ONE; break;
        case GS_GRAPHICS_BLEND_MODE_SRC_COLOR: mode = GL_SRC_COLOR; break;
        case GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_COLOR: mode = GL_ONE_MINUS_SRC_COLOR; break;
        case GS_GRAPHICS_BLEND_MODE_DST_COLOR: mode = GL_DST_COLOR; break;
        case GS_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_COLOR: mode = GL_ONE_MINUS_DST_COLOR; break;
        case GS_GRAPHICS_BLEND_MODE_SRC_ALPHA: mode = GL_SRC_ALPHA; break;
        case GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA: mode = GL_ONE_MINUS_SRC_ALPHA; break;
        case GS_GRAPHICS_BLEND_MODE_DST_ALPHA: mode = GL_DST_ALPHA; break;
        case GS_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_ALPHA: mode = GL_ONE_MINUS_DST_ALPHA; break;
        case GS_GRAPHICS_BLEND_MODE_CONSTANT_COLOR: mode = GL_CONSTANT_COLOR; break;
        case GS_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_COLOR: mode = GL_ONE_MINUS_CONSTANT_COLOR; break;
        case GS_GRAPHICS_BLEND_MODE_CONSTANT_ALPHA: mode = GL_CONSTANT_ALPHA; break;
        case GS_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA: mode = GL_ONE_MINUS_CONSTANT_ALPHA; break;
    }   
    return mode;
}

uint32_t gsgl_cull_face_to_gl_cull_face(gs_graphics_face_culling_type type)
{
    uint32_t fc = GL_BACK;
    switch (type) {
        default:
        case GS_GRAPHICS_FACE_CULLING_BACK: fc = GL_BACK; break;
        case GS_GRAPHICS_FACE_CULLING_FRONT: fc = GL_FRONT; break;
        case GS_GRAPHICS_FACE_CULLING_FRONT_AND_BACK: fc = GL_FRONT_AND_BACK; break;
    }
    return fc;
}   

uint32_t gsgl_winding_order_to_gl_winding_order(gs_graphics_winding_order_type type)
{
    uint32_t wo = GL_CCW;   
    switch (type)
    {
        case GS_GRAPHICS_WINDING_ORDER_CCW: wo = GL_CCW; break;
        case GS_GRAPHICS_WINDING_ORDER_CW: wo = GL_CW; break;
    }
    return wo;
}

uint32_t gsgl_depth_func_to_gl_depth_func(gs_graphics_depth_func_type type)
{
    uint32_t func = GL_LESS;
    switch (type) {
        default:
        case GS_GRAPHICS_DEPTH_FUNC_LESS: func = GL_LESS; break;
        case GS_GRAPHICS_DEPTH_FUNC_NEVER: func = GL_NEVER; break; 
        case GS_GRAPHICS_DEPTH_FUNC_EQUAL: func = GL_EQUAL; break;
        case GS_GRAPHICS_DEPTH_FUNC_LEQUAL: func = GL_LEQUAL; break;
        case GS_GRAPHICS_DEPTH_FUNC_GREATER: func = GL_GREATER; break;
        case GS_GRAPHICS_DEPTH_FUNC_NOTEQUAL: func = GL_NOTEQUAL; break;
        case GS_GRAPHICS_DEPTH_FUNC_GEQUAL: func = GL_GEQUAL; break;
        case GS_GRAPHICS_DEPTH_FUNC_ALWAYS: func = GL_ALWAYS; break;
    }
    return func;
}

uint32_t gsgl_stencil_func_to_gl_stencil_func(gs_graphics_stencil_func_type type)
{
    uint32_t func = GL_ALWAYS;
    switch (type) {
        default:
        case GS_GRAPHICS_STENCIL_FUNC_LESS: func = GL_LESS; break;
        case GS_GRAPHICS_STENCIL_FUNC_NEVER: func = GL_NEVER; break; 
        case GS_GRAPHICS_STENCIL_FUNC_EQUAL: func = GL_EQUAL; break;
        case GS_GRAPHICS_STENCIL_FUNC_LEQUAL: func = GL_LEQUAL; break;
        case GS_GRAPHICS_STENCIL_FUNC_GREATER: func = GL_GREATER; break;
        case GS_GRAPHICS_STENCIL_FUNC_NOTEQUAL: func = GL_NOTEQUAL; break;
        case GS_GRAPHICS_STENCIL_FUNC_GEQUAL: func = GL_GEQUAL; break;
        case GS_GRAPHICS_STENCIL_FUNC_ALWAYS: func = GL_ALWAYS; break;
    }
    return func;
}

uint32_t gsgl_stencil_op_to_gl_stencil_op(gs_graphics_stencil_op_type type)
{
    uint32_t op = GL_KEEP;
    switch (type) {
        default:
        case GS_GRAPHICS_STENCIL_OP_KEEP: op = GL_KEEP; break;
        case GS_GRAPHICS_STENCIL_OP_ZERO: op = GL_ZERO; break;
        case GS_GRAPHICS_STENCIL_OP_REPLACE: op = GL_REPLACE; break;
        case GS_GRAPHICS_STENCIL_OP_INCR: op = GL_INCR; break;
        case GS_GRAPHICS_STENCIL_OP_INCR_WRAP: op = GL_INCR_WRAP; break;
        case GS_GRAPHICS_STENCIL_OP_DECR: op = GL_DECR; break;
        case GS_GRAPHICS_STENCIL_OP_DECR_WRAP: op = GL_DECR_WRAP; break;
        case GS_GRAPHICS_STENCIL_OP_INVERT: op = GL_INVERT; break;
    }
    return op;  
}

gsgl_uniform_type gsgl_uniform_type_to_gl_uniform_type(gs_graphics_uniform_type gstype)
{
    gsgl_uniform_type type = GSGL_UNIFORMTYPE_FLOAT;
    switch (gstype) {
        default:
        case GS_GRAPHICS_UNIFORM_FLOAT: type = GSGL_UNIFORMTYPE_FLOAT; break;
        case GS_GRAPHICS_UNIFORM_INT: type = GSGL_UNIFORMTYPE_INT; break;
        case GS_GRAPHICS_UNIFORM_VEC2: type = GSGL_UNIFORMTYPE_VEC2; break;
        case GS_GRAPHICS_UNIFORM_VEC3: type = GSGL_UNIFORMTYPE_VEC3; break;
        case GS_GRAPHICS_UNIFORM_VEC4: type = GSGL_UNIFORMTYPE_VEC4; break;
        case GS_GRAPHICS_UNIFORM_MAT4: type = GSGL_UNIFORMTYPE_MAT4; break;
        case GS_GRAPHICS_UNIFORM_SAMPLER2D: type = GSGL_UNIFORMTYPE_SAMPLER2D; break;
    }
    return type;
} 

uint32_t gsgl_index_buffer_size_to_gl_index_type(size_t sz)
{
    uint32_t type = GL_UNSIGNED_INT;
    switch (sz) {
        default:
        case 4: type = GL_UNSIGNED_INT; break; 
        case 2: type = GL_UNSIGNED_SHORT; break;
        case 1: type = GL_UNSIGNED_BYTE; break;
    }
    return type;
}

size_t gsgl_get_byte_size_of_vertex_attribute(gs_graphics_vertex_attribute_type type)
{
    size_t byte_size = 0; 
    switch (type) {
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4:   { byte_size = sizeof(float32_t) * 4; } break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3:   { byte_size = sizeof(float32_t) * 3; } break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2:   { byte_size = sizeof(float32_t) * 2; } break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT:    { byte_size = sizeof(float32_t) * 1; } break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT4:    { byte_size = sizeof(uint32_t) * 4; } break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT3:    { byte_size = sizeof(uint32_t) * 3; } break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT2:    { byte_size = sizeof(uint32_t) * 2; } break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT:     { byte_size = sizeof(uint32_t) * 1; } break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4:    { byte_size = sizeof(uint8_t) * 4; } break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE3:    { byte_size = sizeof(uint8_t) * 3; } break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE2:    { byte_size = sizeof(uint8_t) * 2; } break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE:     { byte_size = sizeof(uint8_t) * 1; } break;
    } 

    return byte_size;
}


size_t gsgl_calculate_vertex_size_in_bytes(gs_graphics_vertex_attribute_desc_t* layout, uint32_t count)
{
    // Iterate through all formats in delcarations and calculate total size
    size_t sz = 0;
    for (uint32_t i = 0; i < count; ++i) {
        gs_graphics_vertex_attribute_type type = layout[i].format;
        sz += gsgl_get_byte_size_of_vertex_attribute(type);
    }

    return sz;
}

size_t  gsgl_get_vertex_attr_byte_offest(gs_dyn_array(gs_graphics_vertex_attribute_desc_t) layout, uint32_t idx)
{
    // Recursively calculate offset
    size_t total_offset = 0;

    // Base case
    if (idx == 0) {
        return total_offset;
    } 

    // Calculate total offset up to this point
    for (uint32_t i = 0; i < idx; ++i) {
        total_offset += gsgl_get_byte_size_of_vertex_attribute(layout[i].format);
    } 

    return total_offset;
}

size_t gsgl_uniform_data_size_in_bytes(gs_graphics_uniform_type type)
{
    size_t sz = 0;
    switch (type) {
        case GS_GRAPHICS_UNIFORM_FLOAT: sz = sizeof(float); break;
        case GS_GRAPHICS_UNIFORM_INT:   sz = sizeof(int32_t); break;
        case GS_GRAPHICS_UNIFORM_VEC2:  sz = 2 * sizeof(float); break;
        case GS_GRAPHICS_UNIFORM_VEC3:  sz = 3 * sizeof(float); break;
        case GS_GRAPHICS_UNIFORM_VEC4:  sz = 4 * sizeof(float); break;
        case GS_GRAPHICS_UNIFORM_MAT4:  sz = 16 * sizeof(float); break;
        case GS_GRAPHICS_UNIFORM_SAMPLER2D:  sz = sizeof(gs_handle(gs_graphics_texture_t)); break;  // handle size
        default: {
            sz = 0;
        } break;
    }
    return sz;
}

/* Graphics Interface Creation / Initialization / Shutdown / Destruction */
gs_graphics_t* gs_graphics_create()
{
    // Construct new graphics interface
    gs_graphics_t* gfx = gs_malloc_init(gs_graphics_t);

    // Construct internal data for opengl
    gfx->user_data = gs_malloc_init(gsgl_data_t);

    return gfx;
}

void gs_graphics_destroy(gs_graphics_t* graphics)
{
    // Free all resources (assuming they've been freed from the GPU already)
    if (graphics == NULL) return;

    gsgl_data_t* ogl = (gsgl_data_t*)graphics->user_data;

#define OGL_FREE_DATA(SA, T, FUNC)\
    do {\
        for (\
            gs_slot_array_iter it = 1;\
            gs_slot_array_iter_valid(SA, it);\
            gs_slot_array_iter_advance(SA, it)\
        )\
        {\
            gs_handle(T) hndl = gs_default_val();\
            hndl.id = it;\
            FUNC(hndl);\
        }\
    } while (0)

    // Free all gl data 
    if (ogl->pipelines)         OGL_FREE_DATA(ogl->pipelines, gs_graphics_pipeline_t, gs_graphics_pipeline_destroy); 
    if (ogl->shaders)           OGL_FREE_DATA(ogl->shaders, gs_graphics_shader_t, gs_graphics_shader_destroy); 
    if (ogl->vertex_buffers)    OGL_FREE_DATA(ogl->vertex_buffers, gs_graphics_vertex_buffer_t, gs_graphics_vertex_buffer_destroy);
    if (ogl->index_buffers)     OGL_FREE_DATA(ogl->index_buffers, gs_graphics_index_buffer_t, gs_graphics_index_buffer_destroy); 
    if (ogl->render_passes)     OGL_FREE_DATA(ogl->render_passes, gs_graphics_render_pass_t, gs_graphics_render_pass_destroy); 
    if (ogl->frame_buffers)     OGL_FREE_DATA(ogl->frame_buffers, gs_graphics_framebuffer_t, gs_graphics_framebuffer_destroy); 
    if (ogl->textures)          OGL_FREE_DATA(ogl->textures, gs_graphics_texture_t, gs_graphics_texture_destroy); 
    if (ogl->uniforms)          OGL_FREE_DATA(ogl->uniforms, gs_graphics_uniform_t, gs_graphics_uniform_destroy); 
    if (ogl->uniform_buffers)   OGL_FREE_DATA(ogl->uniform_buffers, gs_graphics_uniform_buffer_t, gs_graphics_uniform_buffer_destroy); 

    gs_slot_array_free(ogl->shaders);
    gs_slot_array_free(ogl->vertex_buffers);
    gs_slot_array_free(ogl->index_buffers);
    gs_slot_array_free(ogl->frame_buffers);
    gs_slot_array_free(ogl->uniforms);
    gs_slot_array_free(ogl->textures);
    gs_slot_array_free(ogl->pipelines);
    gs_slot_array_free(ogl->render_passes);
    gs_slot_array_free(ogl->uniform_buffers);

    // Free uniform data array
    gs_dyn_array_free(ogl->uniform_data.mat4);
    gs_dyn_array_free(ogl->uniform_data.vec4);
    gs_dyn_array_free(ogl->uniform_data.vec3);
    gs_dyn_array_free(ogl->uniform_data.vec2);
    gs_dyn_array_free(ogl->uniform_data.flt);
    gs_dyn_array_free(ogl->uniform_data.i32);
    gs_dyn_array_free(ogl->uniform_data.ui32);

    gs_free(graphics);
    graphics = NULL;
}

void gs_graphics_init(gs_graphics_t* graphics)
{
    // Push back 0 handles into slot arrays (for 0 init validation)
    gsgl_data_t* ogl = (gsgl_data_t*)graphics->user_data;

    gs_slot_array_insert(ogl->shaders, 0);  
    gs_slot_array_insert(ogl->vertex_buffers, 0);   
    gs_slot_array_insert(ogl->index_buffers, 0);    
    gs_slot_array_insert(ogl->frame_buffers, 0);    

    gsgl_uniform_list_t ul = gs_default_val();
    gsgl_uniform_buffer_t ub = gs_default_val();
    gsgl_pipeline_t pip = gs_default_val();
    gsgl_render_pass_t rp = gs_default_val();
    gsgl_texture_t tex = gs_default_val();

    gs_slot_array_insert(ogl->uniforms, ul);
    gs_slot_array_insert(ogl->pipelines, pip);
    gs_slot_array_insert(ogl->render_passes, rp);
    gs_slot_array_insert(ogl->uniform_buffers, ub);
    gs_slot_array_insert(ogl->textures, tex);

    // Construct vao then bind
    glGenVertexArrays(1, &ogl->cache.vao);      
    glBindVertexArray(ogl->cache.vao);

    // Reset data cache for rendering ops
    gsgl_reset_data_cache(&ogl->cache);

    // Init info object
    gs_graphics_info_t* info = &gs_engine_subsystem(graphics)->info;

    // Major/Minor version
    glGetIntegerv(GL_MAJOR_VERSION, (GLint*)&info->major_version);
    glGetIntegerv(GL_MINOR_VERSION, (GLint*)&info->minor_version);

    // Compute shader info
    CHECK_GL_CORE(
        info->compute.available = info->major_version >= 4 && info->minor_version >= 3;
        if (info->compute.available)
        {
            // Work group counts
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, (int32_t*)&info->compute.max_work_group_count[0]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, (int32_t*)&info->compute.max_work_group_count[1]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, (int32_t*)&info->compute.max_work_group_count[2]);
            // Work group sizes
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, (int32_t*)&info->compute.max_work_group_size[0]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, (int32_t*)&info->compute.max_work_group_size[1]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, (int32_t*)&info->compute.max_work_group_size[2]);
            // Work group invocations
            glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, (int32_t*)&info->compute.max_work_group_invocations);
        }
    )
}

void gs_graphics_shutdown(gs_graphics_t* graphics)
{
}

gsgl_texture_t gl_texture_create_internal(const gs_graphics_texture_desc_t* desc)
{
     gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;

    gsgl_texture_t tex = gs_default_val();
    uint32_t width = desc->width;
    uint32_t height = desc->height;
    void* data = desc->data;

    // TODO(john): allow for user to specify explicitly whether to allocate texture as 'render buffer storage'

    // Construct 'normal' texture
    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    // Construct texture based on appropriate format
    switch(desc->format) 
    {
        case GS_GRAPHICS_TEXTURE_FORMAT_A8:                 glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_R8:                 glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGB8:               glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA8:              glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA16F:            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA32F:            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH8:             glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH16:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;              
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH24:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:           glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, data); break;

        // NOTE(john): Because Apple is a shit company, I have to section this off and provide support for 4.1 only features.
        // case GS_GRAPHICS_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
        default: break;
    }

    int32_t mag_filter = desc->mag_filter == GS_GRAPHICS_TEXTURE_FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;
    int32_t min_filter = desc->min_filter == GS_GRAPHICS_TEXTURE_FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;

    if (desc->num_mips) {
        if (desc->min_filter == GS_GRAPHICS_TEXTURE_FILTER_NEAREST) {
            min_filter = desc->mip_filter == GS_GRAPHICS_TEXTURE_FILTER_NEAREST ? GL_NEAREST_MIPMAP_NEAREST : 
                GL_NEAREST_MIPMAP_LINEAR;
        } 
        else {
            min_filter = desc->mip_filter == GS_GRAPHICS_TEXTURE_FILTER_NEAREST ? GL_LINEAR_MIPMAP_NEAREST : 
                GL_NEAREST_MIPMAP_LINEAR;
        }
    }

    uint32_t texture_wrap_s = gsgl_texture_wrap_to_gl_texture_wrap(desc->wrap_s); 
    uint32_t texture_wrap_t = gsgl_texture_wrap_to_gl_texture_wrap(desc->wrap_t);

    if (desc->num_mips) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // float aniso = 0.0f;
    // glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap_t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);

    // Unbind buffers
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Set description
    tex.desc = *desc;

    // Add texture to internal resource pool and return handle
    return tex;
}

/* Resource Creation */
gs_handle(gs_graphics_texture_t) gs_graphics_texture_create(const gs_graphics_texture_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    gsgl_texture_t tex = gl_texture_create_internal(desc);
    // Add texture to internal resource pool and return handle
    return (gs_handle_create(gs_graphics_texture_t, gs_slot_array_insert(ogl->textures, tex)));
}

gs_handle(gs_graphics_uniform_t) gs_graphics_uniform_create(const gs_graphics_uniform_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;

    // Assert if data isn't named
    if (desc->name == NULL) {
        gs_println("Warning: Uniform must be named for OpenGL.");
        return gs_handle_invalid(gs_graphics_uniform_t);
    }

    uint32_t ct = !desc->layout ? 0 : !desc->layout_size ? 1 : (uint32_t)desc->layout_size / (uint32_t)sizeof(gs_graphics_uniform_layout_desc_t);
    if (ct < 1) {
        gs_println("Warning: Uniform layout description must not be empty for: %s.", desc->name);
        return gs_handle_invalid(gs_graphics_uniform_t);
    }

    // Construct list for uniform handles
    gsgl_uniform_list_t ul = gs_default_val();
    memcpy(ul.name, desc->name, 64);

    // Iterate layout, construct individual handles
    for (uint32_t i = 0; i < ct; ++i)
    {
        // Uniform to fill out
        gsgl_uniform_t u = gs_default_val();
        // Cache layout
        gs_graphics_uniform_layout_desc_t* layout = &desc->layout[i];

        memcpy(u.name, layout->fname, 64);
        u.type = gsgl_uniform_type_to_gl_uniform_type(layout->type);
        u.size = gsgl_uniform_data_size_in_bytes(layout->type);
        u.count = layout->count ? layout->count : 1;
        u.location = UINT32_MAX;

        // Add to size of ul
        ul.size += u.size * u.count;

        // Push uniform into list
        gs_dyn_array_push(ul.uniforms, u);
    }

    return gs_handle_create(gs_graphics_uniform_t, gs_slot_array_insert(ogl->uniforms, ul));
}

gs_handle(gs_graphics_vertex_buffer_t) gs_graphics_vertex_buffer_create(const gs_graphics_vertex_buffer_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    gs_handle(gs_graphics_vertex_buffer_t) hndl = gs_default_val();
    gsgl_buffer_t buffer = 0;

    // Assert if data isn't filled for vertex data when static draw enabled
    if (desc->usage == GS_GRAPHICS_BUFFER_USAGE_STATIC && !desc->data) {
        gs_println("Error: Vertex buffer desc must contain data when GS_GRAPHICS_BUFFER_USAGE_STATIC set.");
        gs_assert(false);
    } 

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, desc->size, desc->data, gsgl_buffer_usage_to_gl_enum(desc->usage));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    hndl = gs_handle_create(gs_graphics_vertex_buffer_t, gs_slot_array_insert(ogl->vertex_buffers, buffer));

    return hndl;
}

gs_handle(gs_graphics_index_buffer_t) gs_graphics_index_buffer_create(const gs_graphics_index_buffer_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    gs_handle(gs_graphics_index_buffer_t) hndl = gs_default_val();
    gsgl_buffer_t buffer = 0;

     // Assert if data isn't filled for vertex data when static draw enabled
    if (desc->usage == GS_GRAPHICS_BUFFER_USAGE_STATIC && !desc->data) {
        gs_println("Error: Index buffer desc must contain data when GS_GRAPHICS_BUFFER_USAGE_STATIC set.");
        gs_assert(false);
    }

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, desc->size, desc->data, gsgl_buffer_usage_to_gl_enum(desc->usage));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    hndl = gs_handle_create(gs_graphics_index_buffer_t, gs_slot_array_insert(ogl->index_buffers, buffer));

    return hndl;
}

gs_handle(gs_graphics_uniform_buffer_t) gs_graphics_uniform_buffer_create(const gs_graphics_uniform_buffer_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    gs_handle(gs_graphics_uniform_buffer_t) hndl = gs_default_val();
    gsgl_buffer_t buffer = 0;

     // Assert if data isn't named
    if (desc->name == NULL) {
        gs_println("Warning: Uniform buffer must be named for OpenGL.");
    }

    gsgl_uniform_buffer_t u = gs_default_val();
    memcpy(u.name, desc->name, 64);
    u.size = desc->size;
    u.location = UINT32_MAX;

    // Generate buffer (if needed)
    glGenBuffers(1, &u.ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, u.ubo);
    glBufferData(GL_UNIFORM_BUFFER, u.size, 0, gsgl_buffer_usage_to_gl_enum(desc->usage));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    hndl = gs_handle_create(gs_graphics_uniform_buffer_t, gs_slot_array_insert(ogl->uniform_buffers, u));

    return hndl;
}

gs_handle(gs_graphics_framebuffer_t) gs_graphics_framebuffer_create(const gs_graphics_framebuffer_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    gs_handle(gs_graphics_framebuffer_t) hndl = gs_default_val();
    gsgl_buffer_t buffer = 0;
    glGenFramebuffers(1, &buffer);
    hndl = gs_handle_create(gs_graphics_framebuffer_t, gs_slot_array_insert(ogl->frame_buffers, buffer));
    return hndl;
}

#define GSGL_GRAPHICS_SHADER_PIPELINE_GFX       0x01
#define GSGL_GRAPHICS_SHADER_PIPELINE_COMPUTE   0x02
#define GSGL_GRAPHICS_MAX_SID                   128

gs_handle(gs_graphics_shader_t) gs_graphics_shader_create(const gs_graphics_shader_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    gsgl_shader_t shader = 0;
    uint32_t pip = 0x00;

    uint32_t sid_ct = 0;
    uint32_t sids[GSGL_GRAPHICS_MAX_SID] = gs_default_val();

    // Create shader program
    shader = glCreateProgram();

    uint32_t ct = (uint32_t)desc->size / (uint32_t)sizeof(gs_graphics_shader_source_desc_t);
    for (uint32_t i = 0; i < ct; ++i) 
    {
        if (desc->sources[i].type == GS_GRAPHICS_SHADER_STAGE_VERTEX) pip |= GSGL_GRAPHICS_SHADER_PIPELINE_GFX;
        if (desc->sources[i].type == GS_GRAPHICS_SHADER_STAGE_COMPUTE) pip |= GSGL_GRAPHICS_SHADER_PIPELINE_COMPUTE;

        // Validity Check: Desc must have vertex source if compute not selected. All other source is optional.
        if ((pip & GSGL_GRAPHICS_SHADER_PIPELINE_COMPUTE) && ((pip & GSGL_GRAPHICS_SHADER_PIPELINE_GFX))) {
            gs_println("Error: Cannot have compute and graphics stages for shader program.");
            gs_assert(false);
        }

        uint32_t stage = gsgl_shader_stage_to_gl_stage(desc->sources[i].type);
        uint32_t sid = glCreateShader(stage);

        if (!sid) {
            gs_println ("Error: Failed to allocate memory for shader: '%s': stage: {put stage id here}", desc->name);
            gs_assert(sid);
        }

        // Set source
        glShaderSource(sid, 1, &desc->sources[i].source, NULL);

        // Compile shader
        glCompileShader(sid);

        //Check for errors
        GLint success = 0;
        glGetShaderiv(sid, GL_COMPILE_STATUS, &success);

        if (success == GL_FALSE)
        {
            GLint max_len = 0;
            glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &max_len);

            char* log = (char*)gs_malloc(max_len);
            memset(log, 0, max_len);

            //The max_len includes the NULL character
            glGetShaderInfoLog(sid, max_len, &max_len, log);
            
            // Delete shader.
            glDeleteShader(shader);

            //Provide the infolog
            gs_println("Opengl::opengl_compile_shader::shader: '%s'\nFAILED_TO_COMPILE: %s\n %s", desc->name, log, desc->sources[i].source);

            free(log);
            log = NULL;

            gs_assert(false);
        }

        // Attach shader to program
         glAttachShader(shader, sid);

        // Add to shader array
         sids[sid_ct++] = sid;
    }

    // Link shaders into final program
    glLinkProgram(shader);

    //Create info log for errors
    s32 is_linked = 0;
    glGetProgramiv(shader, GL_LINK_STATUS, (s32*)&is_linked);
    if (is_linked == GL_FALSE)
    {
        GLint max_len = 0;
        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &max_len);

        char* log = (char*)gs_malloc(max_len);
        memset(log, 0, max_len);
        glGetProgramInfoLog(shader, max_len, &max_len, log); 

        // Print error
        gs_println("Error: Fail To Link::opengl_link_shaders::shader: '%s', \n%s", desc->name, log);

        // //We don't need the program anymore.
        glDeleteProgram(shader);

        free(log);
        log = NULL;

        // Just assert for now
        gs_assert(false);
    }

    // Free shaders after use
    for (uint32_t i = 0; i < sid_ct; ++i) {
        glDeleteShader(sids[i]);
    }

    // Iterate over uniforms
    /*
    {
        char tmp_name[256] = gs_default_val();
        int32_t count = 0;
        glGetProgramiv(shader, GL_ACTIVE_UNIFORMS, &count);
        gs_println("Active Uniforms: %d\n", count);

        for (uint32_t i = 0; i < count; i++) {
            int32_t sz = 0;
            uint32_t type;
            glGetActiveUniform(shader, (GLuint)i, 256, NULL, &sz, &type, tmp_name);
            gs_println("Uniform #%d Type: %u Name: %s\n", i, type, tmp_name);
        }
    }
    */

    // Add to pool and return handle
    return (gs_handle_create(gs_graphics_shader_t, gs_slot_array_insert(ogl->shaders, shader)));
}

gs_handle(gs_graphics_render_pass_t) gs_graphics_render_pass_create(const gs_graphics_render_pass_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;

    gsgl_render_pass_t pass = gs_default_val();

    // Set fbo
    pass.fbo = desc->fbo;

    // Set color attachments
    uint32_t ct = (uint32_t)desc->color_size / (uint32_t)sizeof(gs_handle(gs_graphics_texture_t));
    for (uint32_t i = 0; i < ct; ++i) 
    {
        gs_dyn_array_push(pass.color, desc->color[i]);
    }
    // Set depth attachment
    pass.depth = desc->depth;

    // Create handle and return
    return (gs_handle_create(gs_graphics_render_pass_t, gs_slot_array_insert(ogl->render_passes, pass)));
}

gs_handle(gs_graphics_pipeline_t) gs_graphics_pipeline_create(const gs_graphics_pipeline_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;

    gsgl_pipeline_t pipe = gs_default_val();

    // Add states
    pipe.blend = desc->blend;
    pipe.depth = desc->depth;
    pipe.raster = desc->raster;
    pipe.stencil = desc->stencil;
    pipe.compute = desc->compute;

    // Add layout
    uint32_t ct = (uint32_t)desc->layout.size / (uint32_t)sizeof(gs_graphics_vertex_attribute_desc_t);
    gs_dyn_array_reserve(pipe.layout, ct);
    for (uint32_t i = 0; i < ct; ++i) {
        gs_dyn_array_push(pipe.layout, desc->layout.attrs[i]);
    }

    // Create handle and return
    return (gs_handle_create(gs_graphics_pipeline_t, gs_slot_array_insert(ogl->pipelines, pipe)));
}

// Resource Destruction
GS_API_DECL void gs_graphics_texture_destroy(gs_handle(gs_graphics_texture_t) hndl)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    gsgl_texture_t* tex = gs_slot_array_getp(ogl->textures, hndl.id);
    glDeleteTextures(1, &tex->id);
    gs_slot_array_erase(ogl->textures, hndl.id);
}

GS_API_DECL void gs_graphics_uniform_destroy(gs_handle(gs_graphics_uniform_t) hndl)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data; 
    gsgl_uniform_list_t* ul = gs_slot_array_getp(ogl->uniforms, hndl.id);
    gs_dyn_array_free(ul->uniforms);
    gs_slot_array_erase(ogl->uniforms, hndl.id);
}

GS_API_DECL void gs_graphics_shader_destroy(gs_handle(gs_graphics_shader_t) hndl)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    glDeleteProgram(gs_slot_array_get(ogl->shaders, hndl.id));
    gs_slot_array_erase(ogl->shaders, hndl.id);
}

GS_API_DECL void gs_graphics_vertex_buffer_destroy(gs_handle(gs_graphics_vertex_buffer_t) hndl)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    gsgl_buffer_t buffer = gs_slot_array_get(ogl->vertex_buffers, hndl.id); 
    glDeleteBuffers(1, &buffer);
    gs_slot_array_erase(ogl->vertex_buffers, hndl.id);
}

GS_API_DECL void gs_graphics_index_buffer_destroy(gs_handle(gs_graphics_index_buffer_t) hndl)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    gsgl_buffer_t buffer = gs_slot_array_get(ogl->index_buffers, hndl.id); 
    glDeleteBuffers(1, &buffer);
    gs_slot_array_erase(ogl->index_buffers, hndl.id);
}

GS_API_DECL void gs_graphics_uniform_buffer_destroy(gs_handle(gs_graphics_uniform_buffer_t) hndl)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    gsgl_uniform_buffer_t* u = gs_slot_array_getp(ogl->uniform_buffers, hndl.id);

    // Delete buffer (if needed)
    glDeleteBuffers(1, &u->ubo);

    // Delete from slot array
    gs_slot_array_erase(ogl->uniform_buffers, hndl.id);
}

GS_API_DECL void gs_graphics_framebuffer_destroy(gs_handle(gs_graphics_framebuffer_t) hndl)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    gsgl_buffer_t buffer = gs_slot_array_get(ogl->frame_buffers, hndl.id);
    glDeleteFramebuffers(1, &buffer);
    gs_slot_array_erase(ogl->frame_buffers, hndl.id);
}

GS_API_DECL void gs_graphics_render_pass_destroy(gs_handle(gs_graphics_render_pass_t) hndl)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    gs_slot_array_erase(ogl->render_passes, hndl.id);
}

GS_API_DECL void gs_graphics_pipeline_destroy(gs_handle(gs_graphics_pipeline_t) hndl)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    gsgl_pipeline_t* pip = gs_slot_array_getp(ogl->pipelines, hndl.id);

    // Free layout
    gs_dyn_array_free(pip->layout);

    // Erase handles from slot arrays
    gs_slot_array_erase(ogl->pipelines, hndl.id);
}

// Resource Query
GS_API_DECL void gs_graphics_pipeline_desc_query(gs_handle(gs_graphics_pipeline_t) hndl, gs_graphics_pipeline_desc_t* out)
{
    if (!out) return;

    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data; 
    gsgl_pipeline_t* pip =  gs_slot_array_getp(ogl->pipelines, hndl.id); 

    // Add states
    out->blend = pip->blend;
    out->depth = pip->depth;
    out->raster = pip->raster;
    out->stencil = pip->stencil;
    out->compute = pip->compute;

    // Add layout
    uint32_t ct = gs_dyn_array_size(pip->layout);
    for (uint32_t i = 0; i < ct; ++i) {
        gs_dyn_array_push(out->layout.attrs, pip->layout[i]);
    } 
}

// Resource Updates (main thread only) 
//
GS_API_DECL void gs_graphics_vertex_buffer_update(gs_handle(gs_graphics_vertex_buffer_t) hndl, gs_graphics_vertex_buffer_desc_t* desc)
{ 
    /*
    void __gs_graphics_update_buffer_internal(gs_command_buffer_t* cb, 
        uint32_t id, 
        gs_graphics_buffer_type type,
        gs_graphics_buffer_usage_type usage, 
        size_t sz, 
        size_t offset, 
        gs_graphics_buffer_update_type update_type,
        void* data)
    {
        // Write command
        gs_byte_buffer_write(&cb->commands, u32, (u32)GS_OPENGL_OP_REQUEST_BUFFER_UPDATE);
        cb->num_commands++;

        // Write handle id
        gs_byte_buffer_write(&cb->commands, uint32_t, id);
        // Write type
        gs_byte_buffer_write(&cb->commands, gs_graphics_buffer_type, type);
        // Write usage
        gs_byte_buffer_write(&cb->commands, gs_graphics_buffer_usage_type, usage);
        // Write data size
        gs_byte_buffer_write(&cb->commands, size_t, sz);
        // Write data offset
        gs_byte_buffer_write(&cb->commands, size_t, offset);
        // Write data update type
        gs_byte_buffer_write(&cb->commands, gs_graphics_buffer_update_type, update_type);
        // Write data
        gs_byte_buffer_write_bulk(&cb->commands, data, sz);
    } 
    __gs_graphics_update_buffer_internal(cb, hndl.id, GS_GRAPHICS_BUFFER_VERTEX, desc->usage, desc->size, desc->update.offset, desc->update.type, desc->data);
    */

    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    gsgl_buffer_t buffer = gs_slot_array_get(ogl->vertex_buffers, hndl.id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer); 
    int32_t glusage = gsgl_buffer_usage_to_gl_enum(desc->usage);
    switch (desc->update.type) 
    {
        case GS_GRAPHICS_BUFFER_UPDATE_SUBDATA: glBufferSubData(GL_ARRAY_BUFFER, desc->update.offset, desc->size, desc->data); break;
        default:                                glBufferData(GL_ARRAY_BUFFER, desc->size, desc->data, glusage); break;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GS_API_DECL void gs_graphics_index_buffer_update(gs_handle(gs_graphics_index_buffer_t) hndl, gs_graphics_index_buffer_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    gsgl_buffer_t buffer = gs_slot_array_get(ogl->index_buffers, hndl.id);
    int32_t glusage = gsgl_buffer_usage_to_gl_enum(desc->usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    switch (desc->update.type) {
l:
        case GS_GRAPHICS_BUFFER_UPDATE_SUBDATA: glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, desc->update.offset, desc->size, desc->data); break;
        default:                                glBufferData(GL_ELEMENT_ARRAY_BUFFER, desc->size, desc->data, glusage); break;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
} 

/* Resource Update*/
void gs_graphics_texture_update(gs_handle(gs_graphics_texture_t) hndl, gs_graphics_texture_desc_t* desc)
{
}

// void gs_graphics_buffer_update(gs_handle(gs_graphics_buffer_t) hndl, gs_graphics_buffer_desc_t* desc)
// {
// }

#define __ogl_push_command(CB, OP_CODE, ...)\
do {\
    gsgl_data_t* DATA = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;\
    gs_byte_buffer_write(&CB->commands, u32, (u32)OP_CODE);\
    __VA_ARGS__\
    CB->num_commands++;\
} while (0)

/* Command Buffer Ops: Pipeline / Pass / Bind / Draw */
void gs_graphics_begin_render_pass(gs_command_buffer_t* cb, gs_handle(gs_graphics_render_pass_t) hndl)
{
    __ogl_push_command(cb, GS_OPENGL_OP_BEGIN_RENDER_PASS, {
        gs_byte_buffer_write(&cb->commands, uint32_t, hndl.id);
    });
}

void gs_graphics_end_render_pass(gs_command_buffer_t* cb)
{
    __ogl_push_command(cb, GS_OPENGL_OP_END_RENDER_PASS, {
        // Nothing...
    });
}

void gs_graphics_clear(gs_command_buffer_t* cb, gs_graphics_clear_desc_t* desc)
{
    __ogl_push_command(cb, GS_OPENGL_OP_CLEAR, {
        uint32_t count = !desc->actions ? 0 : !desc->size ? 1 : (uint32_t)((size_t)desc->size / (size_t)sizeof(gs_graphics_clear_action_t));
        gs_byte_buffer_write(&cb->commands, uint32_t, count);
        for (uint32_t i = 0; i < count; ++i) {
            gs_byte_buffer_write(&cb->commands, gs_graphics_clear_action_t, desc->actions[i]);
        }
    });
}

void gs_graphics_set_viewport(gs_command_buffer_t* cb, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    __ogl_push_command(cb, GS_OPENGL_OP_SET_VIEWPORT, {
        gs_byte_buffer_write(&cb->commands, uint32_t, x);
        gs_byte_buffer_write(&cb->commands, uint32_t, y);
        gs_byte_buffer_write(&cb->commands, uint32_t, w);
        gs_byte_buffer_write(&cb->commands, uint32_t, h);
    });
}

void gs_graphics_set_view_scissor(gs_command_buffer_t* cb, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{ 
    __ogl_push_command(cb, GS_OPENGL_OP_SET_VIEW_SCISSOR, {
        gs_byte_buffer_write(&cb->commands, uint32_t, x);
        gs_byte_buffer_write(&cb->commands, uint32_t, y);
        gs_byte_buffer_write(&cb->commands, uint32_t, w);
        gs_byte_buffer_write(&cb->commands, uint32_t, h);
    }); 
}

void gs_graphics_texture_request_update(gs_command_buffer_t* cb, gs_handle(gs_graphics_texture_t) hndl, gs_graphics_texture_desc_t* desc)
{
    // Write command
    gs_byte_buffer_write(&cb->commands, uint32_t, (uint32_t)GS_OPENGL_OP_REQUEST_TEXTURE_UPDATE);
    cb->num_commands++;

    uint32_t num_comps = 0;
    size_t data_type_size = 0;
    size_t total_size = 0;
    switch(desc->format) 
    {
        default:
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA8:              num_comps = 4; data_type_size = sizeof(uint8_t); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGB8:               num_comps = 3; data_type_size = sizeof(uint8_t); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_A8:                 num_comps = 1; data_type_size = sizeof(uint8_t); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_R8:                 num_comps = 1; data_type_size = sizeof(uint8_t); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA16F:            num_comps = 4; data_type_size = sizeof(float); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA32F:            num_comps = 4; data_type_size = sizeof(float); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH8:             num_comps = 1; data_type_size = sizeof(float); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH16:            num_comps = 1; data_type_size = sizeof(float); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH24:            num_comps = 1; data_type_size = sizeof(float); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:           num_comps = 1; data_type_size = sizeof(float); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:   num_comps = 1; data_type_size = sizeof(uint32_t); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:  num_comps = 1; data_type_size = sizeof(float) + sizeof(uint8_t); break;

        // NOTE(john): Because Apple is a shit company, I have to section this off and provide support for 4.1 only features.
        // case GS_GRAPHICS_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
    }
    total_size = desc->width * desc->height * num_comps * data_type_size;
    gs_byte_buffer_write(&cb->commands, uint32_t, hndl.id);
    gs_byte_buffer_write(&cb->commands, gs_graphics_texture_desc_t, *desc);
    gs_byte_buffer_write(&cb->commands, size_t, total_size);
    gs_byte_buffer_write_bulk(&cb->commands, desc->data, total_size);
}

void __gs_graphics_update_buffer_internal(gs_command_buffer_t* cb, 
    uint32_t id, 
    gs_graphics_buffer_type type,
    gs_graphics_buffer_usage_type usage, 
    size_t sz, 
    size_t offset, 
    gs_graphics_buffer_update_type update_type,
    void* data)
{
    // Write command
    gs_byte_buffer_write(&cb->commands, u32, (u32)GS_OPENGL_OP_REQUEST_BUFFER_UPDATE);
    cb->num_commands++;

    // Write handle id
    gs_byte_buffer_write(&cb->commands, uint32_t, id);
    // Write type
    gs_byte_buffer_write(&cb->commands, gs_graphics_buffer_type, type);
    // Write usage
    gs_byte_buffer_write(&cb->commands, gs_graphics_buffer_usage_type, usage);
    // Write data size
    gs_byte_buffer_write(&cb->commands, size_t, sz);
    // Write data offset
    gs_byte_buffer_write(&cb->commands, size_t, offset);
    // Write data update type
    gs_byte_buffer_write(&cb->commands, gs_graphics_buffer_update_type, update_type);
    // Write data
    gs_byte_buffer_write_bulk(&cb->commands, data, sz);
}

void gs_graphics_vertex_buffer_request_update(gs_command_buffer_t* cb, gs_handle(gs_graphics_vertex_buffer_t) hndl, gs_graphics_vertex_buffer_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;

    // Return if handle not valid
    if (!hndl.id) return;

    __gs_graphics_update_buffer_internal(cb, hndl.id, GS_GRAPHICS_BUFFER_VERTEX, desc->usage, desc->size, desc->update.offset, desc->update.type, desc->data);
}

void gs_graphics_index_buffer_request_update(gs_command_buffer_t* cb, gs_handle(gs_graphics_index_buffer_t) hndl, gs_graphics_index_buffer_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;

    // Return if handle not valid
    if (!hndl.id) return;

    __gs_graphics_update_buffer_internal(cb, hndl.id, GS_GRAPHICS_BUFFER_INDEX, desc->usage, desc->size, desc->update.offset, desc->update.type, desc->data);
}

void gs_graphics_uniform_buffer_request_update(gs_command_buffer_t* cb, gs_handle(gs_graphics_uniform_buffer_t) hndl, gs_graphics_uniform_buffer_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;

    // Return if handle not valid
    if (!hndl.id) return;

    __gs_graphics_update_buffer_internal(cb, hndl.id, GS_GRAPHICS_BUFFER_UNIFORM, desc->usage, desc->size, desc->update.offset, desc->update.type, desc->data);
}

void gs_graphics_apply_bindings(gs_command_buffer_t* cb, gs_graphics_bind_desc_t* binds)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;

    // Increment commands
    gs_byte_buffer_write(&cb->commands, u32, (u32)GS_OPENGL_OP_APPLY_BINDINGS);
    cb->num_commands++;
 
    // __ogl_push_command(cb, GS_OPENGL_OP_APPLY_BINDINGS,
    {
        // Get counts from buffers
        uint32_t vct = binds->vertex_buffers.desc ? binds->vertex_buffers.size ? binds->vertex_buffers.size / sizeof(gs_graphics_bind_vertex_buffer_desc_t) : 1 : 0;
        uint32_t ict = binds->index_buffers.desc ? binds->index_buffers.size ? binds->index_buffers.size / sizeof(gs_graphics_bind_index_buffer_desc_t) : 1 : 0;
        uint32_t uct = binds->uniform_buffers.desc ? binds->uniform_buffers.size ? binds->uniform_buffers.size / sizeof(gs_graphics_bind_uniform_buffer_desc_t) : 1 : 0;
        uint32_t pct = binds->uniforms.desc ? binds->uniforms.size ? binds->uniforms.size / sizeof(gs_graphics_bind_uniform_desc_t) : 1 : 0;
        uint32_t ibc = binds->image_buffers.desc ? binds->image_buffers.size ? binds->image_buffers.size / sizeof(gs_graphics_bind_image_buffer_desc_t) : 1 : 0;

        // Determine total count to write into command buffer
        uint32_t ct = vct + ict + uct + ibc + pct;
        gs_byte_buffer_write(&cb->commands, uint32_t, ct);

        // Determine if need to clear any previous vertex buffers (if vct != 0)
        gs_byte_buffer_write(&cb->commands, bool, (vct != 0));

        // Vertex buffers
        for (uint32_t i = 0; i < vct; ++i)
        {
            gs_graphics_bind_vertex_buffer_desc_t* decl = &binds->vertex_buffers.desc[i];
            gs_byte_buffer_write(&cb->commands, gs_graphics_bind_type, GS_GRAPHICS_BIND_VERTEX_BUFFER);
            gs_byte_buffer_write(&cb->commands, uint32_t, decl->buffer.id);
            gs_byte_buffer_write(&cb->commands, size_t, decl->offset);
            gs_byte_buffer_write(&cb->commands, gs_graphics_vertex_data_type, decl->data_type);
        }

        // Index buffers
        for (uint32_t i = 0; i < ict; ++i)
        {
            gs_graphics_bind_index_buffer_desc_t* decl = &binds->index_buffers.desc[i];
            gs_byte_buffer_write(&cb->commands, gs_graphics_bind_type, GS_GRAPHICS_BIND_INDEX_BUFFER);
            gs_byte_buffer_write(&cb->commands, uint32_t, decl->buffer.id);
        }

        // Uniform buffers
        for (uint32_t i = 0; i < uct; ++i)
        {
            gs_graphics_bind_uniform_buffer_desc_t* decl = &binds->uniform_buffers.desc[i];

            uint32_t id = decl->buffer.id;
            size_t sz = (size_t)(gs_slot_array_getp(ogl->uniform_buffers, id))->size;
            gs_byte_buffer_write(&cb->commands, gs_graphics_bind_type, GS_GRAPHICS_BIND_UNIFORM_BUFFER);
            gs_byte_buffer_write(&cb->commands, uint32_t, decl->buffer.id);
            gs_byte_buffer_write(&cb->commands, uint32_t, decl->binding);
            gs_byte_buffer_write(&cb->commands, size_t, decl->range.offset);
            gs_byte_buffer_write(&cb->commands, size_t, decl->range.size);
        }

        // Image buffers
        for (uint32_t i = 0; i < ibc; ++i)
        {
            gs_graphics_bind_image_buffer_desc_t* decl = &binds->image_buffers.desc[i];
            gs_byte_buffer_write(&cb->commands, gs_graphics_bind_type, GS_GRAPHICS_BIND_IMAGE_BUFFER);
            gs_byte_buffer_write(&cb->commands, uint32_t, decl->tex.id);
            gs_byte_buffer_write(&cb->commands, uint32_t, decl->binding);
            gs_byte_buffer_write(&cb->commands, gs_graphics_access_type, decl->access);
        }

        // Uniforms
        for (uint32_t i = 0; i < pct; ++i)
        {
            gs_graphics_bind_uniform_desc_t* decl = &binds->uniforms.desc[i];

            // Get size from uniform list
            size_t sz = gs_slot_array_getp(ogl->uniforms, decl->uniform.id)->size;
            gs_byte_buffer_write(&cb->commands, gs_graphics_bind_type, GS_GRAPHICS_BIND_UNIFORM);
            gs_byte_buffer_write(&cb->commands, uint32_t, decl->uniform.id);
            gs_byte_buffer_write(&cb->commands, size_t, sz);
            gs_byte_buffer_write(&cb->commands, uint32_t, decl->binding);
            gs_byte_buffer_write_bulk(&cb->commands, decl->data, sz);
        }
    };
}

void gs_graphics_bind_pipeline(gs_command_buffer_t* cb, gs_handle(gs_graphics_pipeline_t) hndl)
{
    // NOTE(john): Not sure if this is safe in the future, since the data for pipelines is on the main thread and MIGHT be tampered with on a separate thread.
    __ogl_push_command(cb, GS_OPENGL_OP_BIND_PIPELINE, {
        gs_byte_buffer_write(&cb->commands, uint32_t, hndl.id);
    });
}

void gs_graphics_draw(gs_command_buffer_t* cb, gs_graphics_draw_desc_t* desc)
{
    __ogl_push_command(cb, GS_OPENGL_OP_DRAW, {
        gs_byte_buffer_write(&cb->commands, uint32_t, desc->start);
        gs_byte_buffer_write(&cb->commands, uint32_t, desc->count);
        gs_byte_buffer_write(&cb->commands, uint32_t, desc->instances);
        gs_byte_buffer_write(&cb->commands, uint32_t, desc->base_vertex);
        gs_byte_buffer_write(&cb->commands, uint32_t, desc->range.start);
        gs_byte_buffer_write(&cb->commands, uint32_t, desc->range.end);
    });
}

void gs_graphics_dispatch_compute(gs_command_buffer_t* cb, uint32_t num_x_groups, uint32_t num_y_groups, uint32_t num_z_groups)
{
    __ogl_push_command(cb, GS_OPENGL_OP_DISPATCH_COMPUTE, {
        gs_byte_buffer_write(&cb->commands, uint32_t, num_x_groups);
        gs_byte_buffer_write(&cb->commands, uint32_t, num_y_groups);
        gs_byte_buffer_write(&cb->commands, uint32_t, num_z_groups);
    });
}

/* Submission (Main Thread) */
void gs_graphics_submit_command_buffer(gs_command_buffer_t* cb)
{
    /*
        // Structure of command: 
            - Op code
            - Data packet
    */

    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;

    // Set read position of buffer to beginning
    gs_byte_buffer_seek_to_beg(&cb->commands);

    // For each command in buffer
    gs_for_range(cb->num_commands)
    {
        // Read in op code of command
        gs_byte_buffer_readc(&cb->commands, gs_opengl_op_code_type, op_code);

        switch (op_code)
        {
            case GS_OPENGL_OP_BEGIN_RENDER_PASS:
            {
                // Bind render pass stuff
                gs_byte_buffer_readc(&cb->commands, uint32_t, rpid);

                // If render pass exists, then we'll bind frame buffer and attachments 
                if (rpid && gs_slot_array_exists(ogl->render_passes, rpid)) 
                {
                    gsgl_render_pass_t* rp = gs_slot_array_getp(ogl->render_passes, rpid);

                    // Bind frame buffer since it actually exists
                    if (rp->fbo.id && gs_slot_array_exists(ogl->frame_buffers, rp->fbo.id)) 
                    {
                        // Bind frame buffer
                        glBindFramebuffer(GL_FRAMEBUFFER, gs_slot_array_get(ogl->frame_buffers, rp->fbo.id));

                        // Bind color attachments
                        for (uint32_t r = 0; r < gs_dyn_array_size(rp->color); ++r)
                        {
                            uint32_t cid = rp->color[r].id;
                            if (cid && gs_slot_array_exists(ogl->textures, cid)) 
                            {
                                gsgl_texture_t* rt = gs_slot_array_getp(ogl->textures, cid);

                                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + r, GL_TEXTURE_2D, rt->id, 0);
                            }
                        }

                        // Bind depth attachment
                        {
                            uint32_t depth_id = rp->depth.id;
                            if (depth_id && gs_slot_array_exists(ogl->textures, depth_id))
                            {
                                gsgl_texture_t* rt = gs_slot_array_getp(ogl->textures, depth_id);
                                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rt->id, 0);
                            }
                        }
                    }
                }
            } break;

            case GS_OPENGL_OP_END_RENDER_PASS:
            {
                gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
                gsgl_reset_data_cache(&ogl->cache);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                glDisable(GL_SCISSOR_TEST);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_STENCIL_TEST);
                glDisable(GL_BLEND);
            } break;

            case GS_OPENGL_OP_CLEAR:
            {
                // Actions
                gs_byte_buffer_readc(&cb->commands, uint32_t, action_count);
                for (uint32_t j = 0; j < action_count; ++j)
                {
                    gs_byte_buffer_readc(&cb->commands, gs_graphics_clear_action_t, action);

                    // No clear
                    if (action.flag & GS_GRAPHICS_CLEAR_NONE) {
                        continue;
                    }

                    uint32_t bit = 0x00;

                    if (action.flag & GS_GRAPHICS_CLEAR_COLOR || action.flag == 0x00) {
                        glClearColor(action.color[0], action.color[1], action.color[2], action.color[3]);
                        bit |= GL_COLOR_BUFFER_BIT;
                    }
                    if (action.flag & GS_GRAPHICS_CLEAR_DEPTH || action.flag == 0x00) {
                        bit |= GL_DEPTH_BUFFER_BIT;
                    }
                    if (action.flag & GS_GRAPHICS_CLEAR_STENCIL || action.flag == 0x00) {
                        bit |= GL_STENCIL_BUFFER_BIT;
                    }

                    glClear(bit);
                }
            } break;

            case GS_OPENGL_OP_SET_VIEWPORT:
            {
                gs_byte_buffer_readc(&cb->commands, uint32_t, x);
                gs_byte_buffer_readc(&cb->commands, uint32_t, y);
                gs_byte_buffer_readc(&cb->commands, uint32_t, w);
                gs_byte_buffer_readc(&cb->commands, uint32_t, h);

                glViewport(x, y, w, h);
            } break;

            case GS_OPENGL_OP_SET_VIEW_SCISSOR:
            {
                gs_byte_buffer_readc(&cb->commands, uint32_t, x);
                gs_byte_buffer_readc(&cb->commands, uint32_t, y);
                gs_byte_buffer_readc(&cb->commands, uint32_t, w);
                gs_byte_buffer_readc(&cb->commands, uint32_t, h); 

                glEnable(GL_SCISSOR_TEST);
                glScissor(x, y, w, h);
            } break;

            case GS_OPENGL_OP_APPLY_BINDINGS:
            {
                gs_byte_buffer_readc(&cb->commands, uint32_t, ct);

                // Determine if need to clear any previous vertex buffers here
                gs_byte_buffer_readc(&cb->commands, bool, clear_vertex_buffers);

                // Clear previous vertex decls if necessary
                if (clear_vertex_buffers) {
                    gs_dyn_array_clear(ogl->cache.vdecls);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                }

                for (uint32_t i = 0; i < ct; ++i) 
                {
                    gs_byte_buffer_readc(&cb->commands, gs_graphics_bind_type, type);
                    switch (type)
                    {
                        case GS_GRAPHICS_BIND_VERTEX_BUFFER:
                        {
                            gs_byte_buffer_readc(&cb->commands, uint32_t, id);
                            gs_byte_buffer_readc(&cb->commands, size_t, offset);
                            gs_byte_buffer_readc(&cb->commands, gs_graphics_vertex_data_type, data_type);

                            if (!id || !gs_slot_array_exists(ogl->vertex_buffers, id)) 
                            {
                                gs_timed_action(60, 
                                {
                                    gs_println("Warning:Opengl:BindBindings:VertexBuffer %d does not exist.", id);
                                    continue;
                                });
                            }

                            // Grab vbo to bind
                            gsgl_buffer_t vbo = gs_slot_array_get(ogl->vertex_buffers, id);

                            // If the data type is non-interleaved, then push size into vertex buffer decl
                            gsgl_vertex_buffer_decl_t vbo_decl = gs_default_val();
                            vbo_decl.vbo = vbo;
                            vbo_decl.data_type = data_type;
                            vbo_decl.offset = offset;

                            // Cache vertex buffer for later use
                            gs_dyn_array_push(ogl->cache.vdecls, vbo_decl);

                        } break;

                        case GS_GRAPHICS_BIND_INDEX_BUFFER:
                        {
                            gs_byte_buffer_readc(&cb->commands, uint32_t, id);

                            if (!gs_slot_array_exists(ogl->index_buffers, id)) 
                            {
                                gs_timed_action(60, 
                                {
                                    gs_println("Warning:Opengl:BindBindings:IndexBuffer %d does not exist.", id);
                                });
                            } 
                            else 
                            {
                                gsgl_buffer_t ibo = gs_slot_array_get(ogl->index_buffers, id);

                                // Store in cache
                                ogl->cache.ibo = id;
                            }
                        } break;

                        case GS_GRAPHICS_BIND_UNIFORM:
                        {
                            // Get size from uniform list
                            gs_byte_buffer_readc(&cb->commands, uint32_t, id);
                            // Read data size for uniform list 
                            gs_byte_buffer_readc(&cb->commands, size_t, sz);
                            // Read binding from uniform list (could make this a binding list? not sure how to handle this)
                            gs_byte_buffer_readc(&cb->commands, uint32_t, binding);

                            // Check buffer id. If invalid, then we can't operate, and instead just need to pass over the data.
                            if (!id || !gs_slot_array_exists(ogl->uniforms, id)) {
                                gs_timed_action(60, {
                                    gs_println("Warning:Bind Uniform:Uniform %d does not exist.", id);
                                });
                                gs_byte_buffer_advance_position(&cb->commands, sz);
                                continue;
                            }

                            // Grab currently bound pipeline (TODO(john): assert if this isn't valid)
                            if (!ogl->cache.pipeline.id || !gs_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)){
                                gs_timed_action(60, {
                                    gs_println("Warning:Bind Uniform Buffer:Pipeline %d does not exist.", ogl->cache.pipeline.id);
                                });
                                gs_byte_buffer_advance_position(&cb->commands, sz);
                                continue;
                            }

                            gsgl_pipeline_t* pip = gs_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                            // Get uniform
                            gsgl_uniform_list_t* ul = gs_slot_array_getp(ogl->uniforms, id);

                            // Get bound shader from pipeline (either compute or raster)
                            uint32_t sid = pip->compute.shader.id ? pip->compute.shader.id : pip->raster.shader.id;

                            // Check uniform location. If UINT32_T max, then must construct and place location
                            for (uint32_t ui = 0; ui < gs_dyn_array_size(ul->uniforms); ++ui)
                            {
                                gsgl_uniform_t* u = &ul->uniforms[ui];

                                // Searching for location if not bound or sid doesn't match previous use
                                if ((u->location == UINT32_MAX && u->location != UINT32_MAX - 1) || u->sid != pip->raster.shader.id) 
                                {
                                    if (!sid || !gs_slot_array_exists(ogl->shaders, sid)) {
                                        gs_timed_action(60, {
                                            gs_println("Warning:Bind Uniform:Shader %d does not exist.", sid);
                                        });

                                        // Advance by size of uniform
                                        gs_byte_buffer_advance_position(&cb->commands, sz);
                                        continue;
                                    }

                                    gsgl_shader_t shader = gs_slot_array_get(ogl->shaders, sid);

                                    // Construct temp name, concat with base name + uniform field name
                                    char name[256] = gs_default_val();
                                    memcpy(name, ul->name, 256);
                                    if (u->name)
                                    {
                                        gs_snprintfc(UTMP, 256, "%s%s", ul->name, u->name);
                                        memcpy(name, UTMP, 256);
                                    }

                                    // Grab location of uniform based on name
                                    u->location = glGetUniformLocation(shader, name ? name : "__EMPTY_UNIFORM_NAME");

                                    if (u->location >= UINT32_MAX) {
                                        gs_println("Warning: Bind Uniform: Uniform not found: \"%s\"", name);
                                        u->location = UINT32_MAX - 1;
                                    }

                                    u->sid = pip->raster.shader.id;
                                }

                                // Switch on uniform type to upload data
                                switch (u->type) 
                                {
                                    case GSGL_UNIFORMTYPE_FLOAT: 
                                    {
                                        // Need to read bulk data for array.  
                                        gs_assert(u->size == sizeof(float)); 
                                        gs_dyn_array_clear(ogl->uniform_data.flt);
                                        uint32_t ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        gs_for_range(ct) {
                                            gs_byte_buffer_readc(&cb->commands, float, v);
                                            gs_dyn_array_push(ogl->uniform_data.flt, v);
                                        }
                                        glUniform1fv(u->location, ct, ogl->uniform_data.flt); 
                                    } break;

                                    case GSGL_UNIFORMTYPE_INT: 
                                    {
                                        gs_assert(u->size == sizeof(int32_t)); 
                                        gs_dyn_array_clear(ogl->uniform_data.i32);
                                        uint32_t ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        gs_for_range(ct) {
                                            gs_byte_buffer_readc(&cb->commands, int32_t, v);
                                            gs_dyn_array_push(ogl->uniform_data.i32, v);
                                        }
                                        glUniform1iv(u->location, ct, ogl->uniform_data.i32); 
                                    } break;

                                    case GSGL_UNIFORMTYPE_VEC2: 
                                    {
                                        gs_assert(u->size == sizeof(gs_vec2)); 
                                        gs_dyn_array_clear(ogl->uniform_data.vec2);
                                        uint32_t ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        gs_for_range(ct) {
                                            gs_byte_buffer_readc(&cb->commands, gs_vec2, v);
                                            gs_dyn_array_push(ogl->uniform_data.vec2, v);
                                        }
                                        glUniform2fv(u->location, ct, (float*)ogl->uniform_data.vec2); 
                                    } break;

                                    case GSGL_UNIFORMTYPE_VEC3: 
                                    {
                                        gs_assert(u->size == sizeof(gs_vec3));
                                        gs_dyn_array_clear(ogl->uniform_data.vec3);
                                        uint32_t ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        gs_for_range(ct) {
                                            gs_byte_buffer_readc(&cb->commands, gs_vec3, v);
                                            gs_dyn_array_push(ogl->uniform_data.vec3, v);
                                        }
                                        glUniform3fv(u->location, ct, (float*)ogl->uniform_data.vec3); 
                                    } break;

                                    case GSGL_UNIFORMTYPE_VEC4: 
                                    { 
                                        gs_assert(u->size == sizeof(gs_vec4));
                                        gs_dyn_array_clear(ogl->uniform_data.vec4);
                                        uint32_t ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        gs_for_range(ct) {
                                            gs_byte_buffer_readc(&cb->commands, gs_vec4, v);
                                            gs_dyn_array_push(ogl->uniform_data.vec4, v);
                                        }
                                        glUniform4fv(u->location, ct, (float*)ogl->uniform_data.vec4); 
                                    } break;

                                    case GSGL_UNIFORMTYPE_MAT4: 
                                    { 
                                        gs_assert(u->size == sizeof(gs_mat4)); 
                                        gs_dyn_array_clear(ogl->uniform_data.mat4);
                                        uint32_t ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size; 
                                        gs_for_range(ct) {
                                            gs_byte_buffer_readc(&cb->commands, gs_mat4, v);
                                            gs_dyn_array_push(ogl->uniform_data.mat4, v);
                                        }
                                        glUniformMatrix4fv(u->location, ct, false, (float*)ogl->uniform_data.mat4); 
                                    } break;

                                    case GSGL_UNIFORMTYPE_SAMPLER2D:
                                    {
                                        gs_assert(u->size == sizeof(gs_handle(gs_graphics_texture_t)));
                                        uint32_t ct = u->count ? u->count : 1; 
                                        int32_t binds[128] = gs_default_val();
                                        for (uint32_t i = 0; (i < ct && i < 128); ++i)    // Max of 128 texture binds. Get real.
                                        {
                                            gs_byte_buffer_read_bulkc(&cb->commands, gs_handle(gs_graphics_texture_t), v, u->size); 

                                            // Get texture, also need binding, but will worry about that in a bit
                                            gsgl_texture_t* tex = gs_slot_array_getp(ogl->textures, v.id);

                                            // Activate texture slot
                                            glActiveTexture(GL_TEXTURE0 + binding);

                                            // Bind texture
                                            glBindTexture(GL_TEXTURE_2D, tex->id);

                                            binds[i] = (int32_t)binding++;
                                        } 

                                        // Bind uniforms
                                        glUniform1iv(u->location, ct, (int32_t*)binds);

                                    } break;

                                    default: {
                                        // Shouldn't hit here
                                        gs_println("Assert: Bind Uniform: Invalid uniform type specified.");
                                        gs_assert(false);
                                    } break;
                                }
                            }

                        } break;

                        case GS_GRAPHICS_BIND_UNIFORM_BUFFER:
                        {
                            // Read slot id of uniform buffer
                            gs_byte_buffer_readc(&cb->commands, uint32_t, id);
                            // Read binding
                            gs_byte_buffer_readc(&cb->commands, uint32_t, binding);
                            // Read range offset
                            gs_byte_buffer_readc(&cb->commands, size_t, range_offset);
                            // Read range size
                            gs_byte_buffer_readc(&cb->commands, size_t, range_size);

                            // Check buffer id. If invalid, then we can't operate, and instead just need to pass over the data.
                            if (!id || !gs_slot_array_exists(ogl->uniforms, id)) {
                                gs_timed_action(60, {
                                    gs_println("Warning:Bind Uniform Buffer:Uniform %d does not exist.", id);
                                });
                                continue;
                            }

                            // Grab currently bound pipeline (TODO(john): assert if this isn't valid)
                            if (!ogl->cache.pipeline.id || !gs_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)){
                                gs_timed_action(60, {
                                    gs_println("Warning:Bind Uniform Buffer:Pipeline %d does not exist.", ogl->cache.pipeline.id);
                                });
                                continue;
                            }

                            gsgl_pipeline_t* pip = gs_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                            // Get uniform
                            gsgl_uniform_buffer_t* u = gs_slot_array_getp(ogl->uniform_buffers, id);

                            // Get bound shader from pipeline (either compute or raster)
                            uint32_t sid = pip->compute.shader.id ? pip->compute.shader.id : pip->raster.shader.id;

                            // Check uniform location. 
                            // If UINT32_T max, then must construct and place location, or if shader id doesn't match previously used shader with this uniform
                            // TODO(john): To avoid constant lookups in this case, allow for shaders to hold uniform handles references instead.
                            if ((u->location == UINT32_MAX && u->location != UINT32_MAX - 1) || u->sid != pip->raster.shader.id) 
                            {
                                if (!sid || !gs_slot_array_exists(ogl->shaders, sid)) {
                                    gs_timed_action(60, {
                                        gs_println("Warning:Bind Uniform Buffer:Shader %d does not exist.", sid);
                                    });
                                    continue;
                                }

                                gsgl_shader_t shader = gs_slot_array_get(ogl->shaders, sid);

                                // Get uniform location based on name and bound shader
                                u->location = glGetUniformBlockIndex(shader, u->name ? u->name : "__EMPTY_UNIFORM_NAME");

                                // Set binding for uniform block
                                glUniformBlockBinding(shader, u->location, binding); 

                                if (u->location >= UINT32_MAX) {
                                    gs_println("Warning: Bind Uniform Buffer: Uniform not found: \"%s\"", u->name);
                                    u->location = UINT32_MAX - 1;
                                }

                                u->sid = pip->raster.shader.id;
                            }

                            glBindBufferRange(GL_UNIFORM_BUFFER, binding, u->ubo, range_offset, range_size ? range_size : u->size);

                        } break;

                        case GS_GRAPHICS_BIND_IMAGE_BUFFER:
                        {
                            gs_byte_buffer_readc(&cb->commands, uint32_t, tex_slot_id);
                            gs_byte_buffer_readc(&cb->commands, uint32_t, binding);
                            gs_byte_buffer_readc(&cb->commands, gs_graphics_access_type, access);

                            // Grab texture from sampler id
                            if (!tex_slot_id || !gs_slot_array_exists(ogl->textures, tex_slot_id)) {
                                gs_timed_action(60, {
                                    gs_println("Warning:Bind Image Buffer:Texture %d does not exist.", tex_slot_id);
                                });
                                continue;
                            }

                            gsgl_texture_t* tex = gs_slot_array_getp(ogl->textures, tex_slot_id);
                            uint32_t gl_access = gsgl_access_type_to_gl_access_type(access);
                            uint32_t gl_format = gsgl_texture_format_to_gl_texture_internal_format(tex->desc.format);

                            // Bind image texture
                            CHECK_GL_CORE(glBindImageTexture(0, tex->id, 0, GL_FALSE, 0, gl_access, gl_format);)
                        } break;

                        default: gs_assert(false); break;
                    }
                }

            } break;

            case GS_OPENGL_OP_BIND_PIPELINE:
            {
                // Bind pipeline stuff
                gs_byte_buffer_readc(&cb->commands, uint32_t, pipid);

                // Make sure pipeline exists
                if (!pipid || !gs_slot_array_exists(ogl->pipelines, pipid)) {
                    gs_println("Warning: Pipeline %d does not exist.", pipid);
                    continue;
                }
                
                // Reset cache
                gsgl_reset_data_cache(&ogl->cache);

               // Reset state as well
                gsgl_pipeline_state();

                /* Cache pipeline id */
                ogl->cache.pipeline = gs_handle_create(gs_graphics_pipeline_t, pipid);

                gsgl_pipeline_t* pip = gs_slot_array_getp(ogl->pipelines, pipid);

                /* Compute */ 
                // Early out if compute, since we're not doing a rasterization stage
                if (pip->compute.shader.id)
                {
                    /* Shader */
                    if (pip->compute.shader.id && gs_slot_array_exists(ogl->shaders, pip->compute.shader.id)) {
                        glUseProgram(gs_slot_array_get(ogl->shaders, pip->compute.shader.id));
                    } 
                    else {
                        gs_timed_action(60, {
                            gs_println("Warning:Opengl:BindPipeline:Compute:Shader %d does not exist.", pip->compute.shader.id);
                        });
                    }

                    continue;
                }

                /* Depth */
                if (!pip->depth.func) {
                    // If no depth function (default), then disable
                    glDisable(GL_DEPTH_TEST);
                }
                else {
                    glEnable(GL_DEPTH_TEST);    
                    glDepthFunc(gsgl_depth_func_to_gl_depth_func(pip->depth.func));
                }

                /* Stencil */
                if (!pip->stencil.func) {
                    // If no stencil function (default), then disable
                    glDisable(GL_STENCIL_TEST); 
                } else {
                    glEnable(GL_STENCIL_TEST);  
                    uint32_t func = gsgl_stencil_func_to_gl_stencil_func(pip->stencil.func);
                    uint32_t sfail = gsgl_stencil_op_to_gl_stencil_op(pip->stencil.sfail);
                    uint32_t dpfail = gsgl_stencil_op_to_gl_stencil_op(pip->stencil.dpfail);
                    uint32_t dppass = gsgl_stencil_op_to_gl_stencil_op(pip->stencil.dppass);
                    glStencilFunc(func, pip->stencil.ref, pip->stencil.comp_mask);
                    glStencilMask(pip->stencil.write_mask);
                    glStencilOp(sfail, dpfail, dppass);
                }

                /* Blend */
                if (!pip->blend.func) {
                    glDisable(GL_BLEND);
                } else {
                    glEnable(GL_BLEND);
                    glBlendEquation(gsgl_blend_equation_to_gl_blend_eq(pip->blend.func));
                    glBlendFunc(gsgl_blend_mode_to_gl_blend_mode(pip->blend.src, GL_ONE), 
                        gsgl_blend_mode_to_gl_blend_mode(pip->blend.dst, GL_ZERO));
                }

                /* Raster */
                // Face culling
                if (!pip->raster.face_culling) {
                    glDisable(GL_CULL_FACE);    
                } else {
                    glEnable(GL_CULL_FACE); 
                    glCullFace(gsgl_cull_face_to_gl_cull_face(pip->raster.face_culling));   
                }

                // Winding order
                glFrontFace(gsgl_winding_order_to_gl_winding_order(pip->raster.winding_order));

                /* Shader */
                if (pip->raster.shader.id && gs_slot_array_exists(ogl->shaders, pip->raster.shader.id)) {
                    glUseProgram(gs_slot_array_get(ogl->shaders, pip->raster.shader.id));
                } 
                else {
                    gs_timed_action(60, {
                        gs_println("Warning:Opengl:BindPipeline:Shader %d does not exist.", pip->raster.shader.id);
                    });
                }
            } break;

            case GS_OPENGL_OP_DISPATCH_COMPUTE:
            {
                gs_byte_buffer_readc(&cb->commands, uint32_t, num_x_groups);
                gs_byte_buffer_readc(&cb->commands, uint32_t, num_y_groups);
                gs_byte_buffer_readc(&cb->commands, uint32_t, num_z_groups);

                // Grab currently bound pipeline (TODO(john): assert if this isn't valid)
                if (ogl->cache.pipeline.id == 0 || !gs_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                    gs_timed_action(60, {
                        gs_println("Warning:Opengl:DispatchCompute:Compute Pipeline not bound.");
                    });
                    continue;
                }

                gsgl_pipeline_t* pip = gs_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                // If pipeline does not have a compute state bound, then leave
                if (!pip->compute.shader.id) {
                    gs_timed_action(60, {
                        gs_println("Warning:Opengl:DispatchCompute:Compute Pipeline not bound.");
                    });
                    continue;
                }

                // Dispatch shader 
                CHECK_GL_CORE(
                    glDispatchCompute(num_x_groups, num_y_groups, num_z_groups); 
                    // Memory barrier (TODO(john): make this specifically set in the pipeline state)
                    glMemoryBarrier(GL_ALL_BARRIER_BITS);
                )
            } break;

            case GS_OPENGL_OP_DRAW:
            {
                // Grab currently bound pipeline (TODO(john): assert if this isn't valid)
                gsgl_pipeline_t* pip = gs_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                // Must have a vertex buffer bound to draw
                if (gs_dyn_array_empty(ogl->cache.vdecls)) {
                    gs_println("Error:Opengl:Draw: No vertex buffer bound.");
                    gs_assert(false);
                }

                // Keep track whether or not the data is to be instanced
                bool is_instanced = false;

                for (uint32_t i = 0; i < gs_dyn_array_size(pip->layout); ++i)
                {
                    // Vertex buffer to bind
                    uint32_t vbo_idx = pip->layout[i].buffer_idx;
                    gsgl_vertex_buffer_decl_t vdecl = vbo_idx < gs_dyn_array_size(ogl->cache.vdecls) ? ogl->cache.vdecls[vbo_idx] : ogl->cache.vdecls[0];
                    gsgl_buffer_t vbo = vdecl.vbo;

                    // Manual override. If you manually set divisor/stride/offset, then will not automatically calculate any of those.
                    bool is_manual = pip->layout[i].stride | pip->layout[i].divisor | pip->layout[i].offset | vdecl.data_type == GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED;

                    // Bind buffer
                    glBindBuffer(GL_ARRAY_BUFFER, vbo);

                    // Stride of vertex attribute
                    size_t stride = is_manual ? pip->layout[i].stride : 
                                        gsgl_calculate_vertex_size_in_bytes(pip->layout, gs_dyn_array_size(pip->layout));

                    // Byte offset of vertex attribute (if non-interleaved data, then grab offset from decl instead)
                    size_t offset = vdecl.data_type == GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED ? vdecl.offset : is_manual ? pip->layout[i].offset : 
                                        gsgl_get_vertex_attr_byte_offest(pip->layout, i);

                    // If there is a vertex divisor for this layout, then we'll draw instanced
                    is_instanced |= (pip->layout[i].divisor != 0);

                    // Enable the vertex attribute pointer
                    glEnableVertexAttribArray(i);

                    switch (pip->layout[i].format)
                    {
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4: glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3: glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2: glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT:  glVertexAttribPointer(i, 1, GL_FLOAT, GL_FALSE, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT4:  glVertexAttribIPointer(i, 4, GL_UNSIGNED_INT, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT3:  glVertexAttribIPointer(i, 3, GL_UNSIGNED_INT, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT2:  glVertexAttribIPointer(i, 2, GL_UNSIGNED_INT, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT:   glVertexAttribIPointer(i, 1, GL_UNSIGNED_INT, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE:   glVertexAttribPointer(i, 1, GL_UNSIGNED_BYTE, GL_TRUE, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE2:  glVertexAttribPointer(i, 2, GL_UNSIGNED_BYTE, GL_TRUE, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE3:  glVertexAttribPointer(i, 3, GL_UNSIGNED_BYTE, GL_TRUE, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4:  glVertexAttribPointer(i, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, gs_int2voidp(offset)); break;

                        // Shouldn't get here
                        default: {
                            gs_assert(false);
                        } break;
                    }
                    // Set up divisor (for instancing)
                    glVertexAttribDivisor(i, pip->layout[i].divisor);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                } 

                // Bind all vertex buffers after setting up data and pointers
                for (uint32_t i = 0; i < gs_dyn_array_size(ogl->cache.vdecls); ++i) {
                    glBindBuffer(GL_ARRAY_BUFFER, ogl->cache.vdecls[i].vbo);
                } 

                // Draw based on bound primitive type in raster 
                gs_byte_buffer_readc(&cb->commands, uint32_t, start);
                gs_byte_buffer_readc(&cb->commands, uint32_t, count);
                gs_byte_buffer_readc(&cb->commands, uint32_t, instance_count);
                gs_byte_buffer_readc(&cb->commands, uint32_t, base_vertex);
                gs_byte_buffer_readc(&cb->commands, uint32_t, range_start);
                gs_byte_buffer_readc(&cb->commands, uint32_t, range_end);

                range_end = (range_end && range_end < range_start) ? range_end : count;

                // Bind element buffer ranged
                if (ogl->cache.ibo) {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gs_slot_array_get(ogl->index_buffers, ogl->cache.ibo));
                }

                // If instance count > 1, do instanced drawing
                is_instanced |= (instance_count > 1);

                uint32_t prim = gsgl_primitive_to_gl_primitive(pip->raster.primitive);
                uint32_t itype = gsgl_index_buffer_size_to_gl_index_type(pip->raster.index_buffer_element_size);

                // Draw
                if (ogl->cache.ibo) { 
                    #ifdef GS_GRAPHICS_IMPL_OPENGL_CORE
                        if (is_instanced)   glDrawElementsInstancedBaseVertex(prim, count, itype, gs_int2voidp(start), instance_count, base_vertex);
                        else                glDrawRangeElementsBaseVertex(prim, range_start, range_end, count, itype, gs_int2voidp(start), base_vertex);
                    #else
                        if (is_instanced)   glDrawElementsInstanced(prim, count, itype, gs_int2voidp(start), instance_count);
                        else                glDrawElements(prim, count, itype, gs_int2voidp(start));
                    #endif
                } 
                else {
                    if (is_instanced)   glDrawArraysInstanced(prim, start, count, instance_count);
                    else                glDrawArrays(prim, start, count);
                }

            } break;

            case GS_OPENGL_OP_REQUEST_TEXTURE_UPDATE:
            {
                gs_byte_buffer_readc(&cb->commands, uint32_t, tex_slot_id);
                gs_byte_buffer_readc(&cb->commands, gs_graphics_texture_desc_t, desc);
                gs_byte_buffer_readc(&cb->commands, size_t, data_size);

                // Update texture with data, depending on update type (for now, just stream new data)

                // Grab texture from sampler id
                if (!tex_slot_id || !gs_slot_array_exists(ogl->textures, tex_slot_id)) {
                    gs_timed_action(60, {
                        gs_println("Warning:Bind Image Buffer:Texture %d does not exist.", tex_slot_id);
                    });
                    gs_byte_buffer_advance_position(&cb->commands, data_size);
                }

                gsgl_texture_t* tex = gs_slot_array_getp(ogl->textures, tex_slot_id);
                uint32_t int_format = gsgl_texture_format_to_gl_texture_internal_format(desc.format);
                uint32_t format = gsgl_texture_format_to_gl_texture_format(desc.format);
                uint32_t dt = gsgl_texture_format_to_gl_data_type(desc.format);
                desc.data = (cb->commands.data + cb->commands.position);
                *tex = gl_texture_create_internal(&desc);

                // Bind texture
                // glBindTexture(GL_TEXTURE_2D, tex->id);
                // // Update texture data
                // // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, desc.width, desc.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (cb->commands.data, cb->commands.position));
                // // glTexImage2D(GL_TEXTURE_2D, 0, int_format, desc.width, desc.height, 0, format, dt, (cb->commands.data, cb->commands.position));
                // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, desc.width, desc.height, format, dt, (cb->commands.data + cb->commands.position));
                // glBindTexture(GL_TEXTURE_2D, 0);
                gs_byte_buffer_advance_position(&cb->commands, data_size);
            } break;

            case GS_OPENGL_OP_REQUEST_BUFFER_UPDATE:
            {
                gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;

                // Read handle id
                gs_byte_buffer_readc(&cb->commands, uint32_t, id);
                // Read type
                gs_byte_buffer_readc(&cb->commands, gs_graphics_buffer_type, type);
                // Read usage
                gs_byte_buffer_readc(&cb->commands, gs_graphics_buffer_usage_type, usage);
                // Read data size
                gs_byte_buffer_readc(&cb->commands, size_t, sz);
                // Read data offset
                gs_byte_buffer_readc(&cb->commands, size_t, offset);
                // Read update type
                gs_byte_buffer_readc(&cb->commands, gs_graphics_buffer_update_type, update_type);

                int32_t glusage = gsgl_buffer_usage_to_gl_enum(usage);

                switch (type)
                {
                    // Vertex Buffer
                    default:
                    case GS_GRAPHICS_BUFFER_VERTEX:
                    {
                        gsgl_buffer_t buffer = gs_slot_array_get(ogl->vertex_buffers, id);
                        glBindBuffer(GL_ARRAY_BUFFER, buffer);
                        switch (update_type) {
                            case GS_GRAPHICS_BUFFER_UPDATE_SUBDATA: glBufferSubData(GL_ARRAY_BUFFER, offset, sz, (cb->commands.data + cb->commands.position)); break;
                            default:                                glBufferData(GL_ARRAY_BUFFER, sz, (cb->commands.data + cb->commands.position), glusage); break;
                        }
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                    } break;

                    case GS_GRAPHICS_BUFFER_INDEX:
                    {
                        gsgl_buffer_t buffer = gs_slot_array_get(ogl->index_buffers, id);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
                        switch (update_type) {
                            case GS_GRAPHICS_BUFFER_UPDATE_SUBDATA: glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, sz, (cb->commands.data + cb->commands.position)); break;
                            default:                                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, (cb->commands.data + cb->commands.position), glusage); break;
                        }
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                    } break;

                    case GS_GRAPHICS_BUFFER_UNIFORM:
                    {
                        // Have to 
                        gsgl_uniform_buffer_t* u = gs_slot_array_getp(ogl->uniform_buffers, id);

                        glBindBuffer(GL_UNIFORM_BUFFER, u->ubo);

                        switch (update_type) {
                            case GS_GRAPHICS_BUFFER_UPDATE_SUBDATA: {
                                glBufferSubData(GL_UNIFORM_BUFFER, offset, sz, (cb->commands.data + cb->commands.position));
                            } break;
                            default: {
                                // Reset uniform size
                                u->size = sz;
                                // Recreate buffer
                                glBufferData(GL_UNIFORM_BUFFER, sz, (cb->commands.data + cb->commands.position), glusage);
                            } break;
                        }

                        glBindBuffer(GL_UNIFORM_BUFFER, 0);
                    } break;
                }

                // Advance past data
                gs_byte_buffer_advance_position(&cb->commands, sz);

            } break;

            default:
            {
                // Op code not supported yet!
                gs_println("Op code not supported yet: %zu", (uint32_t)op_code);
                gs_assert(false);
            }
        }
    }
    
    // Clear byte buffer of commands
    gs_byte_buffer_clear(&cb->commands);

    // Set num commands to 0
    cb->num_commands = 0;
}

#endif // GS_GRAPHICS_IMPL_OPENGL
#endif // GS_GRAPHICS_IMPL_H

