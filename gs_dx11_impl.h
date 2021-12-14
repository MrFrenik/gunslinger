/*
   This file will contain the actual implementation details for the graphics
   layer, in Direct3D 11. For now we'll follow the GL implementation and try
   to follow a similar approach here as best we can. Big strokes first,
   smaller stuff later.
*/

#ifndef GS_DX11_IMPL_H
#define GS_DX11_IMPL_H

/*=============================
// Headers
=============================*/

#include "gs_dx11.h"
#define CINTERFACE
#define COBJMACROS
#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <stdio.h>

/*=============================
// Globals
=============================*/

/*=============================
// Enums
=============================*/

typedef enum gs_dx11_op_code_type
{
    GS_DX11_OP_BEGIN_RENDER_PASS = 0x00,
    GS_DX11_OP_END_RENDER_PASS,
    GS_DX11_OP_SET_VIEWPORT,
    GS_DX11_OP_CLEAR,
	GS_DX11_OP_REQUEST_BUFFER_UPDATE,
    GS_DX11_OP_BIND_PIPELINE,
    GS_DX11_OP_APPLY_BINDINGS,
    GS_DX11_OP_DRAW,
    GS_DX11_OP_COUNT
} gs_dx11_op_code_type;

// NOTE(matthew): Created these here instead of using the shader stage type
// in gs.h because I need to be able to | and & them together. This is so that
// the shader object (gsdx11_shader_t) can hold what shader(s) it contains so
// that I can set the appropriate shader when binding the pipeline.
typedef enum _TAG_gsdx11_shader_type
{
    GS_DX11_SHADER_TYPE_VERTEX = 0x01,
    GS_DX11_SHADER_TYPE_PIXEL = 0x02
} gsdx11_shader_type;

typedef enum _TAG_gsdx11_uniform_type
{
	GSDX11_UNIFORMTYPE_FLOAT,
	GSDX11_UNIFORMTYPE_INT,
	GSDX11_UNIFORMTYPE_VEC2,
	GSDX11_UNIFORMTYPE_VEC3,
	GSDX11_UNIFORMTYPE_VEC4,
	GSDX11_UNIFORMTYPE_MAT4,
	GSDX11_UNIFORMTYPE_COUNT
} gsdx11_uniform_type;

/*=============================
// Structures
=============================*/

typedef ID3D11Buffer    *gsdx11_buffer_t;

// Full set of shaders we bind to the pipeline
typedef struct _TAG_gsdx11_shader
{
    ID3D11VertexShader      *vs;
    ID3D11PixelShader       *ps;

    // need the bytecode for creating input layouts
    ID3DBlob                *vsblob,
                            *psblob;

    // For identifying what's in the shader object, like a discriminated union
    gsdx11_shader_type      tag;
} gsdx11_shader_t;

typedef struct _TAG_gsdx11_pipeline
{
    /* gs_graphics_blend_state_desc_t blend; */
    /* gs_graphics_depth_state_desc_t depth; */
    gs_graphics_raster_state_desc_t raster;
    /* gs_graphics_stencil_state_desc_t stencil; */
    /* gs_graphics_compute_state_desc_t compute; */
    gs_dyn_array(gs_graphics_vertex_attribute_desc_t) layout;
} gsdx11_pipeline_t;

typedef struct _TAG_gsdx11_vertex_buffer_decl
{
    gsdx11_buffer_t                     vbo;
    gs_graphics_vertex_data_type        data_type;
    size_t                              offset;
} gsdx11_vertex_buffer_decl_t;

// Data used by the DRAW op. We bind this data in BIND_PIPELINE and APPLY_BINDINGS
typedef struct _TAG_gsdx11_data_cache
{
    // NOTE(matthew): do we still need this??
    gs_dyn_array(gsdx11_vertex_buffer_decl_t)   vdecls;

    gs_handle(gs_graphics_pipeline_t)           pipeline;
    gs_dyn_array(D3D11_INPUT_ELEMENT_DESC)      layout_descs;
    ID3D11InputLayout                           *layout;
    gsdx11_shader_t                             shader;
} gsdx11_data_cache_t;

typedef struct _TAG_gsdx11_uniform_buffer
{
	gsdx11_buffer_t cbo;
	size_t size;
	gs_graphics_shader_stage_type stage;
} gsdx11_uniform_buffer_t;

// Internal DX11 data
typedef struct _TAG_gsdx11_data
{
    gs_slot_array(gsdx11_shader_t)      shaders;
    gs_slot_array(gsdx11_buffer_t)      vertex_buffers;
    gs_slot_array(gsdx11_buffer_t)      index_buffers;
	gs_slot_array(gsdx11_uniform_buffer_t) uniform_buffers;
    gs_slot_array(gsdx11_pipeline_t)    pipelines;

    // Global data that I'll just put here since it's appropriate
    ID3D11Device                        *device;
    ID3D11DeviceContext                 *context;
    IDXGISwapChain                      *swapchain;
    ID3D11RenderTargetView              *rtv;
    ID3D11DepthStencilView              *dsv;

    // NOTE(matthew): putting these here for now, although doing so is probably
    // unnecesary. The raster_state will likely turn into an array of raster
    // states if we add functionality for doing multiple render passes.
    // At the end of the day, we need to set up these two as part of DX11
    // initialization, so might as well save them here.
    ID3D11RasterizerState               *raster_state;
    D3D11_VIEWPORT                      viewport;

    // Cached data between draw / state change calls
    gsdx11_data_cache_t                 cache;
} gsdx11_data_t;

// TODO(matthew): implement these after we figure out uniform buffers, since
// they're basically cbuffers.
/* typedef struct _TAG_gsdx11_uniform */
/* { */
/* 	gsdx11_uniform_type type; */
/* 	uint32_t location; // is this needed? */
/* 	size_t size; */
/* } gsdx11_uniform_t; */

/* typedef struct _TAG_gsdx11_uniform_list */
/* { */
/* 	char name[64]; */
/* 	size_t size; */
/* 	gs_dyn_array(gsdx11_uniform_t) uniforms; */
/* } gsdx11_uniform_list_t; */

/*=============================
// Utility Functions
=============================*/

DXGI_FORMAT     gsdx11_vertex_attrib_to_dxgi_format(gs_graphics_vertex_attribute_type type);

// NOTE(matthew): Putting these here for organization purposes (have them all
// in one place without needing to look back at the ogl implementation). These
// will be filled out on a needs basis, many are not required at the time of
// writing this. Also note that some of these may be invalid for DX11, in
// which case they just get scraped, or we determine their DX11 equivalents
// (ie, no uniforms in DX11... what then??).
/* int32_t          gsdx11_buffer_usage_to_dx11_enum(gs_graphics_buffer_usage_type type); */
/* uint32_t         gsdx11_access_type_to_dx11_access_type(gs_graphics_access_type type); */
/* uint32_t         gsdx11_texture_wrap_to_dx11_texture_wrap(gs_graphics_texture_wrapping_type type); */
/* uint32_t         gsdx11_texture_format_to_dx11_data_type(gs_graphics_texture_format_type type); */
/* uint32_t         gsdx11_texture_format_to_dx11_texture_format(gs_graphics_texture_format_type type); */
/* uint32_t         gsdx11_texture_format_to_dx11_texture_internal_format(gs_graphics_texture_format_type type); */
/* uint32_t         gsdx11_shader_stage_to_dx11_stage(gs_graphics_shader_stage_type type); */
/* uint32_t         gsdx11_primitive_to_dx11_primitive(gs_graphics_primitive_type type); */
/* uint32_t         gsdx11_blend_equation_to_dx11_blend_eq(gs_graphics_blend_equation_type eq); */
/* uint32_t         gsdx11_blend_mode_to_dx11_blend_mode(gs_graphics_blend_mode_type type, uint32_t def); */
/* uint32_t         gsdx11_cull_face_to_dx11_cull_face(gs_graphics_face_culling_type type); */
/* uint32_t         gsdx11_winding_order_to_dx11_winding_order(gs_graphics_winding_order_type type); */
/* uint32_t         gsdx11_depth_func_to_dx11_depth_func(gs_graphics_depth_func_type type); */
/* uint32_t         gsdx11_stencil_func_to_dx11_stencil_func(gs_graphics_stencil_func_type type); */
/* uint32_t         gsdx11_stencil_op_to_dx11_stencil_op(gs_graphics_stencil_op_type type); */
/* void             gsdx11_uniform_type_to_dx11_uniform_type(gs_graphics_uniform_type gstype); // NOTE(matthew): DX11 doesn't have uniforms! */
/* uint32_t         gsdx11_index_buffer_size_to_dx11_index_type(size_t sz); */
/* size_t           gsdx11_get_byte_size_of_vertex_attribute(gs_graphics_vertex_attribute_type type); */
/* size_t           gsdx11_calculate_vertex_size_in_bytes(gs_graphics_vertex_attribute_desc_t* layout, uint32_t count); */
/* size_t           gsdx11_get_vertex_attr_byte_offest(gs_dyn_array(gs_graphics_vertex_attribute_desc_t) layout, uint32_t idx); */
/* size_t           gsdx11_uniform_data_size_in_bytes(gs_graphics_uniform_type type); */

/*=============================
// Graphics Initialization
=============================*/

/* // Initialize DX11 specific data (device, context, swapchain  etc.) */
/* void gs_graphics_init(gs_graphics_t *graphics); */

/* // Shutdown */
/* void gs_graphics_shutdown(gs_graphics_t* graphics); */

/*=============================
// Resource Creation
=============================*/

/* // Create a vertex buffer */
/* gs_handle(gs_graphics_vertex_buffer_t) gs_graphics_vertex_buffer_create(const gs_graphics_vertex_buffer_desc_t *desc); */

/* // Create an index buffer */
/* gs_handle(gs_graphics_index_buffer_t) gs_graphics_index_buffer_create(const gs_graphics_index_buffer_desc_t* desc) */

/* // Create a shader */
/* gs_handle(gs_graphics_shader_t) gs_graphics_shader_create(const gs_graphics_shader_desc_t *desc); */

/* // Create a pipeline object */
/* gs_handle(gs_graphics_pipeline_t) gs_graphics_pipeline_create(const gs_graphics_pipeline_desc_t* desc) */

/*=============================================================================
// ===== DX11 Implementation =============================================== //
=============================================================================*/

/*=============================
// Utility Functions
=============================*/

DXGI_FORMAT
gsdx11_vertex_attrib_to_dxgi_format(gs_graphics_vertex_attribute_type type)
{
    DXGI_FORMAT format;


    switch (type)
    {
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4:       format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3:       format = DXGI_FORMAT_R32G32B32_FLOAT; break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2:       format = DXGI_FORMAT_R32G32_FLOAT; break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT:        format = DXGI_FORMAT_R32_FLOAT; break;
        // TODO(matthew): should these be UINT or UNORM??
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT4:        format = DXGI_FORMAT_R32G32B32A32_UINT; break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT3:        format = DXGI_FORMAT_R32G32B32_UINT; break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT2:        format = DXGI_FORMAT_R32G32_UINT; break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT:         format = DXGI_FORMAT_R32_UINT; break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4:        format = DXGI_FORMAT_R8G8B8A8_UINT; break;
        // TODO(matthew): fix this case! no such format in DXGI
        /* case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE3:     format = DXGI_FORMAT_R8G8B8_UINT; break; */
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE2:        format = DXGI_FORMAT_R8G8_UINT; break;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE:         format = DXGI_FORMAT_R8_UINT; break;
    }

    return format;
}



/*=============================
// Graphics Initialization
=============================*/

gs_graphics_t *
gs_graphics_create()
{
    gs_graphics_t       *gfx = gs_malloc_init(gs_graphics_t);


    gfx->user_data = gs_malloc_init(gsdx11_data_t);

    return gfx;
}

void
gs_graphics_destroy(gs_graphics_t *graphics)
{
    gsdx11_data_t       *dx11;


    if (!graphics)
        return;

    dx11 = (gsdx11_data_t *)graphics->user_data;

    // free data
    gs_slot_array_free(dx11->shaders);
    gs_slot_array_free(dx11->vertex_buffers);
    gs_slot_array_free(dx11->index_buffers);
    gs_slot_array_free(dx11->pipelines);

    gs_free(graphics);
}

void
gs_graphics_init(gs_graphics_t *graphics)
{
    HRESULT                     hr;
    ID3D11Texture2D             *backbuffer,
                                *ds_buffer;
    DXGI_MODE_DESC              buffer_desc = {0};
    DXGI_SWAP_CHAIN_DESC        swapchain_desc = {0};
    D3D11_TEXTURE2D_DESC        ds_desc = {0};
    gsdx11_data_t               *dx11;
    void                        *gs_window;
    HWND                        hwnd;
    gsdx11_shader_t             s = {0}; // TODO(matthew): bulletproof this, empty struct for now
    gsdx11_pipeline_t           p = {0}; // ^^^
    gsdx11_uniform_buffer_t		u = {0}; // ^^^
    D3D11_RASTERIZER_DESC       raster_state_desc = {0};
    uint32_t                    window_width = gs_engine_subsystem(app).window_width,
                                window_height  = gs_engine_subsystem(app).window_height;


    dx11 = (gsdx11_data_t *)graphics->user_data;
    gs_window = gs_slot_array_get(gs_engine_subsystem(platform)->windows, 1);
    hwnd = glfwGetWin32Window(gs_window);

    // TODO(matthew): could add a 'render targets' field eventually
    gs_slot_array_insert(dx11->shaders, s);
    gs_slot_array_insert(dx11->vertex_buffers, 0);
    gs_slot_array_insert(dx11->index_buffers, 0);
    gs_slot_array_insert(dx11->pipelines, p);
    gs_slot_array_insert(dx11->uniform_buffers, u);

    // TODO(matthew): we're filling out many of these with predetermined
    // values for now. This is only for development purpsoes. In the future,
    // we'll want to map gunslinger application options to these. For
    // instance, we are forcing the window to be windowed by default; later,
    // we should check the gs_app_desc_t to see if the window should be
    // windowed or fullscreen.
    buffer_desc.Width = window_width;
    buffer_desc.Height = window_height;
    buffer_desc.RefreshRate.Denominator = 1;
    buffer_desc.RefreshRate.Numerator = 60;
    buffer_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    buffer_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    buffer_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    swapchain_desc.BufferDesc = buffer_desc;
    swapchain_desc.SampleDesc.Count = 1;
    swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchain_desc.BufferCount = 1;
    swapchain_desc.OutputWindow = hwnd;
    swapchain_desc.Windowed = TRUE;
    swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    ds_desc.Width = window_width;
    ds_desc.Height = window_height;
    ds_desc.MipLevels = 1;
    ds_desc.ArraySize = 1;
    ds_desc.Format = DXGI_FORMAT_D32_FLOAT;
    ds_desc.SampleDesc.Count = 1;
    ds_desc.Usage = D3D11_USAGE_DEFAULT;
    ds_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
            D3D11_CREATE_DEVICE_DEBUG, 0, 0, D3D11_SDK_VERSION, &swapchain_desc,
            &dx11->swapchain, &dx11->device,NULL, &dx11->context);
    hr = IDXGISwapChain_GetBuffer(dx11->swapchain, 0, &IID_ID3D11Texture2D, (void **)&backbuffer);
    hr = ID3D11Device_CreateRenderTargetView(dx11->device, backbuffer, NULL, &dx11->rtv);
    hr = ID3D11Device_CreateTexture2D(dx11->device, &ds_desc, 0, &ds_buffer);
    hr = ID3D11Device_CreateDepthStencilView(dx11->device, ds_buffer, NULL, &dx11->dsv);
    ID3D11DeviceContext_OMSetRenderTargets(dx11->context, 1, &dx11->rtv, dx11->dsv);

    dx11->viewport.Width = window_width;
    dx11->viewport.Height = window_height;
    dx11->viewport.MaxDepth = 1.0f;
    ID3D11DeviceContext_RSSetViewports(dx11->context, 1, &dx11->viewport);

    raster_state_desc.FillMode = D3D11_FILL_SOLID;
    raster_state_desc.CullMode = D3D11_CULL_NONE;
    hr = ID3D11Device_CreateRasterizerState(dx11->device, &raster_state_desc, &dx11->raster_state);
    ID3D11DeviceContext_RSSetState(dx11->context, dx11->raster_state);
}

void
gs_graphics_shutdown(gs_graphics_t *graphics)
{
}



/*=============================
// Resource Creation
=============================*/

gs_handle(gs_graphics_vertex_buffer_t)
gs_graphics_vertex_buffer_create(const gs_graphics_vertex_buffer_desc_t *desc)
{
    HRESULT                                     hr;
    ID3D11Buffer                                *buffer;
    D3D11_BUFFER_DESC                           buffer_desc = {0};
    D3D11_SUBRESOURCE_DATA                      buffer_data = {0};
    gsdx11_data_t                               *dx11;
    gs_handle(gs_graphics_vertex_buffer_t)      hndl = gs_default_val();


    dx11 = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;

    // TODO(matthew): Later, we need to map more of these fields according to
    // the data in the desc. Will need to create functions that map GS enums
    // to DX11 enums.
    buffer_desc.ByteWidth = desc->size;
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer_data.pSysMem = desc->data;

    if (desc->data)
        hr = ID3D11Device_CreateBuffer(dx11->device, &buffer_desc, &buffer_data, &buffer);
    else
        hr = ID3D11Device_CreateBuffer(dx11->device, &buffer_desc, NULL, &buffer);

    hndl = gs_handle_create(gs_graphics_vertex_buffer_t, gs_slot_array_insert(dx11->vertex_buffers, buffer));
    /* printf("1: %p\r\n", buffer); */
    /* printf("2: %p\n", (gs_slot_array_getp(dx11->vertex_buffers, 1))); */

    return hndl;
}

gs_handle(gs_graphics_index_buffer_t)
gs_graphics_index_buffer_create(const gs_graphics_index_buffer_desc_t* desc)
{
    HRESULT                                     hr;
    ID3D11Buffer                                *buffer;
    D3D11_BUFFER_DESC                           buffer_desc = {0};
    D3D11_SUBRESOURCE_DATA                      buffer_data = {0};
    gsdx11_data_t                               *dx11;
    gs_handle(gs_graphics_index_buffer_t)       hndl = gs_default_val();


    dx11 = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;
    buffer_desc.ByteWidth = desc->size;
    buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_data.pSysMem = desc->data;

    if (desc->data)
        hr = ID3D11Device_CreateBuffer(dx11->device, &buffer_desc, &buffer_data, &buffer);
    else
        hr = ID3D11Device_CreateBuffer(dx11->device, &buffer_desc, NULL, &buffer);

    hndl = gs_handle_create(gs_graphics_index_buffer_t, gs_slot_array_insert(dx11->index_buffers, buffer));

    return hndl;
}

gs_handle(gs_graphics_shader_t)
gs_graphics_shader_create(const gs_graphics_shader_desc_t *desc)
{
    HRESULT                             hr;
    gsdx11_data_t                       *dx11;
    ID3DBlob                            *err_blob;
    void                                *shader_src;
    size_t                              shader_len;
    gsdx11_shader_t                     shader;
    uint32_t                            shader_type;
    gs_handle(gs_graphics_shader_t)     hndl;
    uint32_t                            cnt;


    dx11 = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;
    cnt = (uint32_t)desc->size / (uint32_t)sizeof(gs_graphics_shader_source_desc_t);

    // TODO(matthew): Check the error blob
    for (uint32_t i = 0; i < cnt; i++)
    {
        shader_src = desc->sources[i].source;
        shader_len = strlen(shader_src) + 1;
        shader_type = desc->sources[i].type;

        switch (shader_type)
        {
            case GS_GRAPHICS_SHADER_STAGE_VERTEX:
            {
                hr = D3DCompile(shader_src, shader_len, NULL, NULL, NULL, "main", "vs_5_0",
                        0, 0, &shader.vsblob, &err_blob);
                hr = ID3D11Device_CreateVertexShader(dx11->device, ID3D10Blob_GetBufferPointer(shader.vsblob),
                        ID3D10Blob_GetBufferSize(shader.vsblob), 0, &shader.vs);
                shader.tag |= GS_DX11_SHADER_TYPE_VERTEX;
                /* printf("v: %p\n", shader.vs); */
            } break;
            case GS_GRAPHICS_SHADER_STAGE_FRAGMENT:
            {
                hr = D3DCompile(shader_src, shader_len, NULL, NULL, NULL, "main", "ps_5_0",
                        0, 0, &shader.psblob, &err_blob);
                hr = ID3D11Device_CreatePixelShader(dx11->device, ID3D10Blob_GetBufferPointer(shader.psblob),
                        ID3D10Blob_GetBufferSize(shader.psblob), 0, &shader.ps);
                shader.tag |= GS_DX11_SHADER_TYPE_PIXEL;
                /* printf("p: %p\n", shader.ps); */
            } break;
        }
    }

    hndl = gs_handle_create(gs_graphics_shader_t, gs_slot_array_insert(dx11->shaders, shader));

    return hndl;
}

gs_handle(gs_graphics_pipeline_t)
gs_graphics_pipeline_create(const gs_graphics_pipeline_desc_t* desc)
{
    gsdx11_data_t                           *dx11;
    gsdx11_pipeline_t                       pipe = gs_default_val();
    gs_handle(gs_graphics_pipeline_t)       hndl;
    uint32_t                                cnt;

    dx11 = (gsdx11_data_t*)gs_engine_subsystem(graphics)->user_data;

    // add state
    pipe.raster = desc->raster;

    // add layout
    cnt = (uint32_t)desc->layout.size / (uint32_t)sizeof(gs_graphics_vertex_attribute_desc_t);
    gs_dyn_array_reserve(pipe.layout, cnt);
    for (uint32_t i = 0; i < cnt; i++)
    {
        gs_dyn_array_push(pipe.layout, desc->layout.attrs[i]);
    }

    hndl = gs_handle_create(gs_graphics_pipeline_t, gs_slot_array_insert(dx11->pipelines, pipe));

    return hndl;
}

gs_handle(gs_graphics_uniform_buffer_t)
gs_graphics_uniform_buffer_create(const gs_graphics_uniform_buffer_desc_t *desc)
{
    HRESULT                                     hr;
	gsdx11_data_t								*dx11;
	gsdx11_uniform_buffer_t						ub = gs_default_val();
    D3D11_BUFFER_DESC                           buffer_desc = {0};
	D3D11_SUBRESOURCE_DATA						buffer_data = {0};
	gs_handle(gs_graphics_uniform_buffer_t)		hndl;


	dx11 = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;

	// TODO(matthew): compare these flags to what's specified in the desc (ie,
	// dynamic, CPU access)
    buffer_desc.ByteWidth = desc->size;
    buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_data.pSysMem = desc->data;

	ub.size = desc->size;
	ub.stage = desc->stage;

	if (desc->data)
		hr = ID3D11Device_CreateBuffer(dx11->device, &buffer_desc, &buffer_data,  &ub.cbo);
	else
		hr = ID3D11Device_CreateBuffer(dx11->device, &buffer_desc, NULL,  &ub.cbo);

	hndl = gs_handle_create(gs_graphics_uniform_buffer_t, gs_slot_array_insert(dx11->uniform_buffers, ub));

	return hndl;
}

gs_handle(gs_graphics_texture_t)
gs_graphics_texture_create(const gs_graphics_texture_desc_t* desc)
{
    // TODO(matthew): empty for now, just putting this here for the compile to work
}



/*=============================
// Resource Updates
=============================*/

void
__gs_graphics_update_buffer_internal(gs_command_buffer_t* cb, 
									 uint32_t id, 
									 gs_graphics_buffer_type type,
									 gs_graphics_buffer_usage_type usage, 
									 size_t sz, 
									 size_t offset, 
									 gs_graphics_buffer_update_type update_type,
									 void* data)
{
    // Write command
    gs_byte_buffer_write(&cb->commands, u32, (u32)GS_DX11_OP_REQUEST_BUFFER_UPDATE);
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

void
gs_graphics_uniform_buffer_request_update(gs_command_buffer_t *cb,
										  gs_handle(gs_graphics_uniform_buffer_t) hndl,
										  gs_graphics_uniform_buffer_desc_t *desc)
{
	gsdx11_data_t		*dx11;


	dx11 = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;

	// Return if we receive an invalid handle
	if (!hndl.id)
		return;

    __gs_graphics_update_buffer_internal(cb, hndl.id, GS_GRAPHICS_BUFFER_UNIFORM,
			desc->usage, desc->size, desc->update.offset, desc->update.type, desc->data);
}



/*=============================
// Command Buffers Ops
=============================*/
/*
    // Structure of command:
        - Op code
        - Data packet
*/

// TODO(matthew): This system works fine for now, however it's probably not
// thread safe if we start using multiple command buffers. We can fix this by
// using DX11 command lists. This would involve creating deferred contexts in
// the internal DX11 data for use on multiple threads, as well as having a
// unique command list for each thread that calls into gunslinger's command
// buffer API (need to think of how this could be done).
// We could call ID3D11DeviceContext::FinishCommandList in begin_render_pass()
// to clear the context state, and then again in end_render_pass() to record
// all the commands we wish to batch together into a single command buffer.
// Then, in submit_buffer(), we simply execute the given command list (again,
// need a way to store these for different threads so that they can be accessed
// later on, namely NOW).
// Ignore this for now, as it is a long term thing.

#define __dx11_push_command(__cb, __op_code, ...)                                               \
do                                                                                              \
{                                                                                               \
    gsdx11_data_t *__gsdx11_data = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;   \
    gs_byte_buffer_write(&__cb->commands, u32, (u32)__op_code);                                 \
    __VA_ARGS__                                                                                 \
    __cb->num_commands++;                                                                       \
} while (0)

void
gs_graphics_begin_render_pass(gs_command_buffer_t *cb,
                              gs_handle(gs_graphics_render_pass_t) hndl)
{
    __dx11_push_command(cb, GS_DX11_OP_BEGIN_RENDER_PASS,
    {
        gs_byte_buffer_write(&cb->commands, uint32_t, hndl.id);
    });
}

void
gs_graphics_end_render_pass(gs_command_buffer_t *cb)
{
    __dx11_push_command(cb, GS_DX11_OP_END_RENDER_PASS,
    {
        // Nothing...
    });
}

void
gs_graphics_clear(gs_command_buffer_t *cb,
                  gs_graphics_clear_desc_t *desc)
{

    __dx11_push_command(cb, GS_DX11_OP_CLEAR,
    {
        uint32_t count = !desc->actions ? 0 : !desc->size ? 1 : (uint32_t)((size_t)desc->size / (size_t)sizeof(gs_graphics_clear_action_t));
        gs_byte_buffer_write(&cb->commands, uint32_t, count);
        for (uint32_t i = 0; i < count; ++i)
        {
            gs_byte_buffer_write(&cb->commands, gs_graphics_clear_action_t, desc->actions[i]);
        }
    });
}

void
gs_graphics_set_viewport(gs_command_buffer_t *cb,
                         uint32_t x,
                         uint32_t y,
                         uint32_t w,
                         uint32_t h)
{
    __dx11_push_command(cb, GS_DX11_OP_SET_VIEWPORT,
    {
        gs_byte_buffer_write(&cb->commands, uint32_t, x);
        gs_byte_buffer_write(&cb->commands, uint32_t, y);
        gs_byte_buffer_write(&cb->commands, uint32_t, w);
        gs_byte_buffer_write(&cb->commands, uint32_t, h);
    });
}

void
gs_graphics_apply_bindings(gs_command_buffer_t *cb,
                           gs_graphics_bind_desc_t *binds)
{
    gsdx11_data_t       *dx11;
    uint32_t            vcnt,	// vertex buffers to bind
						ubcnt,	// uniform buffers to bind
                        cnt; 	// total objects to bind


    dx11 = (gsdx11_data_t*)gs_engine_subsystem(graphics)->user_data;
    vcnt = binds->vertex_buffers.desc ? binds->vertex_buffers.size ? binds->vertex_buffers.size / sizeof(gs_graphics_bind_vertex_buffer_desc_t) : 1 : 0;
    ubcnt = binds->uniform_buffers.desc ? binds->uniform_buffers.size ? binds->vertex_buffers.size / sizeof(gs_graphics_bind_uniform_buffer_desc_t) : 1 : 0;
    cnt = vcnt + ubcnt;

    // increment commands
    gs_byte_buffer_write(&cb->commands, u32, (u32)GS_DX11_OP_APPLY_BINDINGS);
    cb->num_commands++;

    // just vertex buffers for now
    gs_byte_buffer_write(&cb->commands, uint32_t, cnt);

    // vertex buffers
    for (uint32_t i = 0; i < vcnt; i++)
    {
        gs_graphics_bind_vertex_buffer_desc_t *decl = &binds->vertex_buffers.desc[i];
        gs_byte_buffer_write(&cb->commands, gs_graphics_bind_type, GS_GRAPHICS_BIND_VERTEX_BUFFER);
        gs_byte_buffer_write(&cb->commands, uint32_t, decl->buffer.id);
        gs_byte_buffer_write(&cb->commands, size_t, decl->offset);
        gs_byte_buffer_write(&cb->commands, gs_graphics_vertex_data_type, decl->data_type);
    }

	// uniform buffers
    for (uint32_t i = 0; i < ubcnt; i++)
    {
        gs_graphics_bind_uniform_buffer_desc_t *decl = &binds->uniform_buffers.desc[i];

		uint32_t id = decl->buffer.id;
		size_t sz = (size_t)(gs_slot_array_getp(dx11->uniform_buffers, id))->size;
        gs_byte_buffer_write(&cb->commands, gs_graphics_bind_type, GS_GRAPHICS_BIND_UNIFORM_BUFFER);
        gs_byte_buffer_write(&cb->commands, uint32_t, decl->buffer.id);
        gs_byte_buffer_write(&cb->commands, size_t, decl->binding);
        gs_byte_buffer_write(&cb->commands, size_t, decl->range.offset);
        gs_byte_buffer_write(&cb->commands, size_t, decl->range.size);
    }
}

void
gs_graphics_bind_pipeline(gs_command_buffer_t *cb,
                          gs_handle(gs_graphics_pipeline_t) hndl)
{
    // NOTE(matthew): See John's note in the GL impl.
    __dx11_push_command(cb, GS_DX11_OP_BIND_PIPELINE,
    {
        gs_byte_buffer_write(&cb->commands, uint32_t, hndl.id);
    });
}

void
gs_graphics_draw(gs_command_buffer_t *cb,
                 gs_graphics_draw_desc_t *desc)
{
    __dx11_push_command(cb, GS_DX11_OP_DRAW, {
        gs_byte_buffer_write(&cb->commands, uint32_t, desc->start);
        gs_byte_buffer_write(&cb->commands, uint32_t, desc->count);
        gs_byte_buffer_write(&cb->commands, uint32_t, desc->instances);
        gs_byte_buffer_write(&cb->commands, uint32_t, desc->base_vertex);
        gs_byte_buffer_write(&cb->commands, uint32_t, desc->range.start);
        gs_byte_buffer_write(&cb->commands, uint32_t, desc->range.end);
    });
}


/* Submission (Main Thread) */
void
gs_graphics_submit_command_buffer(gs_command_buffer_t *cb)
{
    gsdx11_data_t       *dx11;


    dx11 = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;

    // Set read pos to beginning
    gs_byte_buffer_seek_to_beg(&cb->commands);

    // For each command in buffer
    gs_for_range(cb->num_commands)
    {
        // Read op code
        gs_byte_buffer_readc(&cb->commands, gs_dx11_op_code_type, op_code);

        switch (op_code)
        {
            // TODO(matthew): Create the input layout here, then cache it?
            case GS_DX11_OP_BEGIN_RENDER_PASS:
            {} break;

            // TODO(matthew): Destroy the cached input layout here
            case GS_DX11_OP_END_RENDER_PASS:
            {
                // TODO(matthew): figure out why this isn't being deleted
                ID3D11InputLayout_Release(dx11->cache.layout);
                printf("released!...\r\n");
            } break;

            case GS_DX11_OP_CLEAR:
            {
                gs_byte_buffer_readc(&cb->commands, uint32_t, action_count);
                for (uint32_t i = 0; i < action_count; i++)
                {
                    uint32_t        bit = 0x00;

                    gs_byte_buffer_readc(&cb->commands, gs_graphics_clear_action_t, action);

                    // No clear
                    if (action.flag & GS_GRAPHICS_CLEAR_NONE)
                        continue;

                    if (action.flag & GS_GRAPHICS_CLEAR_COLOR || action.flag == 0x00)
                    {
                        float       bgcolor[4] = {action.color[0], action.color[1], action.color[2], action.color[3]};
                        ID3D11DeviceContext_ClearRenderTargetView(dx11->context, dx11->rtv, bgcolor);
                    }
                    if (action.flag & GS_GRAPHICS_CLEAR_DEPTH || action.flag == 0x00)
                    {
                        bit |= D3D11_CLEAR_DEPTH;
                    }
                    if (action.flag & GS_GRAPHICS_CLEAR_STENCIL || action.flag == 0x00)
                    {
                        bit |= D3D11_CLEAR_STENCIL;
                    }

                    ID3D11DeviceContext_ClearDepthStencilView(dx11->context, dx11->dsv, bit, 1.0f, 0);
                }
            } break;

            case GS_DX11_OP_REQUEST_BUFFER_UPDATE:
			{
				gsdx11_data_t		*dx11;

				dx11 = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;

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

				switch (type)
				{
					case GS_GRAPHICS_BUFFER_UNIFORM:
					{
						gsdx11_uniform_buffer_t		*ub;

						if (!id || !gs_slot_array_exists(dx11->uniform_buffers, id)) {
							gs_timed_action(60, {
								gs_println("Warning:Bind Uniform Buffer:Uniform %d does not exist.", id);
							});
							continue;
						}

						ub = gs_slot_array_getp(dx11->uniform_buffers, id);

						switch (update_type)
						{
							// TODO(matthew): handle the pure recreate case (should be as simple as
							// a UpdateSubresource(...)).
							case GS_GRAPHICS_BUFFER_UPDATE_SUBDATA:
							{
								D3D11_MAPPED_SUBRESOURCE		resource = {0};

								// Map resource to CPU side and sub data
								ID3D11DeviceContext_Map(dx11->context, ub->cbo, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
								memcpy(resource.pData,
									(cb->commands.data + cb->commands.position) + offset,
									sz);
								ID3D11DeviceContext_Unmap(dx11->context, ub->cbo, 0);
							} break;
						}
					} break;
				}
			} break;

            case GS_DX11_OP_SET_VIEWPORT:
            {
                gs_byte_buffer_readc(&cb->commands, uint32_t, x);
                gs_byte_buffer_readc(&cb->commands, uint32_t, y);
                gs_byte_buffer_readc(&cb->commands, uint32_t, w);
                gs_byte_buffer_readc(&cb->commands, uint32_t, h);

                dx11->viewport.TopLeftX = x;
                dx11->viewport.TopLeftY = y;
                dx11->viewport.Width = w;
                dx11->viewport.Height = h;

                ID3D11DeviceContext_RSSetViewports(dx11->context, 1, &dx11->viewport);
            };

            case GS_DX11_OP_BIND_PIPELINE:
            {
                gsdx11_pipeline_t       *pipe;

                gs_byte_buffer_readc(&cb->commands, uint32_t, pipe_id);

                // cache the pipeline since we'll need it when binding data
                dx11->cache.pipeline = gs_handle_create(gs_graphics_pipeline_t, pipe_id);

                pipe = gs_slot_array_getp(dx11->pipelines, pipe_id);

                // Raster (shader)
                // (check that the shader is valid)
                if (pipe->raster.shader.id && gs_slot_array_exists(dx11->shaders, pipe->raster.shader.id))
                {
                    gsdx11_shader_t     shader = gs_slot_array_get(dx11->shaders, pipe->raster.shader.id);

                    if (shader.tag & GS_DX11_SHADER_TYPE_VERTEX)
                        ID3D11DeviceContext_VSSetShader(dx11->context, shader.vs, 0, 0);
                    if (shader.tag & GS_DX11_SHADER_TYPE_PIXEL)
                        ID3D11DeviceContext_PSSetShader(dx11->context, shader.ps, 0, 0);

                    dx11->cache.shader = shader;
                }
            } break;

            case GS_DX11_OP_APPLY_BINDINGS:
            {
                gsdx11_pipeline_t       *pipe = gs_slot_array_getp(dx11->pipelines, dx11->cache.pipeline.id);
                gs_byte_buffer_readc(&cb->commands, uint32_t, cnt);

                for (uint32_t i = 0; i < cnt; i++)
                {
                    gs_byte_buffer_readc(&cb->commands, gs_graphics_bind_type, type);

                    switch (type)
                    {
                        case GS_GRAPHICS_BIND_VERTEX_BUFFER:
                        {
                            gsdx11_buffer_t                 vbo;
                            UINT                            vbo_slot = pipe->layout[i].buffer_idx;
                            // TODO(matthew): Handle the other case, we're only considering no stride for now.
                            UINT                            stride = pipe->layout[i].stride;
                            D3D11_INPUT_ELEMENT_DESC        layout_desc = gs_default_val();

                            gs_byte_buffer_readc(&cb->commands, uint32_t, id);
                            gs_byte_buffer_readc(&cb->commands, size_t, offset);
                            gs_byte_buffer_readc(&cb->commands, gs_graphics_vertex_data_type, data_type);

                            vbo = gs_slot_array_get(dx11->vertex_buffers, id);

                            layout_desc.SemanticName = pipe->layout[i].name;
                            layout_desc.Format = gsdx11_vertex_attrib_to_dxgi_format(pipe->layout[i].format);
                            layout_desc.InputSlot = vbo_slot;
                            layout_desc.AlignedByteOffset = offset;
                            // layout_desc.InputSlotClass TODO(matthew): handle instancing case!

                            gs_dyn_array_push(dx11->cache.layout_descs, layout_desc);

                            ID3D11DeviceContext_IASetVertexBuffers(dx11->context, vbo_slot, 1, &vbo, &stride, &offset);
                        } break;

						case GS_GRAPHICS_BIND_UNIFORM_BUFFER:
						{
							gsdx11_uniform_buffer_t		ub;

							gs_byte_buffer_readc(&cb->commands, uint32_t, id);
							gs_byte_buffer_readc(&cb->commands, uint32_t, binding);
							gs_byte_buffer_readc(&cb->commands, size_t, range_offset);
							gs_byte_buffer_readc(&cb->commands, size_t, range_size);

                            if (!id || !gs_slot_array_exists(dx11->uniform_buffers, id)) {
                                gs_timed_action(60, {
                                    gs_println("Warning:Bind Uniform Buffer:Uniform %d does not exist.", id);
                                });
                                continue;
                            }

							ub = gs_slot_array_get(dx11->uniform_buffers, id);	

							if (ub.stage == GS_GRAPHICS_SHADER_STAGE_VERTEX)
								ID3D11DeviceContext_VSSetConstantBuffers(dx11->context, binding, 1, &ub.cbo);
							else if (ub.stage == GS_GRAPHICS_SHADER_STAGE_FRAGMENT)
								ID3D11DeviceContext_PSSetConstantBuffers(dx11->context, binding, 1, &ub.cbo);
							// TODO(matthew): add compute binding
						} break;
                    }

                    if (!dx11->cache.layout)
                    {
                        ID3D11Device_CreateInputLayout(dx11->device, dx11->cache.layout_descs,
                                gs_dyn_array_size(dx11->cache.layout_descs),
                               ID3D10Blob_GetBufferPointer(dx11->cache.shader.vsblob),
                               ID3D10Blob_GetBufferSize(dx11->cache.shader.vsblob),
                               &dx11->cache.layout);
                        ID3D11DeviceContext_IASetInputLayout(dx11->context, dx11->cache.layout);
                        // NOTE(matthew): set this as the default for now
                        ID3D11DeviceContext_IASetPrimitiveTopology(dx11->context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    }
                }
            } break;

            case GS_DX11_OP_DRAW:
            {
                /* gsdx11_pipeline_t *pipe = gs_slot_array_getp(dx11->pipelines, dx11->cache.pipeline.id); */
                /* bool is_instanced = false; */

                /* if (gs_dyn_array_empty(dx11->cache.vdecls)) { */
                    /* gs_println("Error:DX11:Draw: No vertex buffer bound."); */
                    /* gs_assert(false); */
                /* } */

                /* for (uint32_t i = 0; i < gs_dyn_array_size(pipe->layout); i++) */
                /* { */
                /*  uint32_t vbo_slot = pipe->layout[i].buffer_idx; */
                /*  gsdx11_vertex_buffer_decl_t vdecl = dx11->cache.vdecls[vbo_slot]; */
                /*  gsdx11_buffer_t vbo = vdecl.vbo; */
                /*  size_t stride, offset; */

                /*  // Manual override for setting divisor(step rate)/stride/offset */
                    /* bool is_manual = pipe->layout[i].stride | pipe->layout[i].divisor | pipe->layout[i].offset | vdecl.data_type == GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED; */
                /*  stride = pipe->layout[i].stride; // TODO(matthew): Handle the other case, we're only considering no stride for now. */
                /*  offset = vdecl.offset; // TODO(matthew): Again, handle the other case (see L2051). */

                /*  // TODO(matthew): make this take the buffer slot instead of 'i' */
                /*  ID3D11DeviceContext_IASetVertexBuffers(dx11->context, i, 1, &vbo, &stride, &offset); */
                /* } */

                // TODO(matthew): handle instancing + indexed rendering eventually
                gs_byte_buffer_readc(&cb->commands, uint32_t, start);
                gs_byte_buffer_readc(&cb->commands, uint32_t, count);

                ID3D11DeviceContext_Draw(dx11->context, count, start);
                IDXGISwapChain_Present(dx11->swapchain, 0, 0);
            } break;
        }
    }
}

#endif // GS_DX11_IMPL_H

