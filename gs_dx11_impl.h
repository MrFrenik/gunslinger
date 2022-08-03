/*
   This file will contain the actual implementation details for the graphics
   layer, in Direct3D 11. For now we'll follow the GL implementation and try
   to follow a similar approach here as best we can. Big strokes first,
   smaller stuff later.

   TODO(matthew): in gs.h, need to expose a 'nopresent' flag/command.
   TODO(matthew): proper thread safety for resource creation (critical sections)
   TODO(matthew): currently using TLS to handle the command buffer system; maybe
    later we should try to use the actual gs_command_buffer_t object itself by
    using a cast for instance? It's a hacky alternative but at least it actually
    makes use of the command buffer object, whereas TLS completely ignores it.
   TODO(matthew): TEST THE THREAD-SAFETY CHANGES
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

static CRITICAL_SECTION		shaders_cs,
							vertex_buffers_cs,
							index_buffers_cs,
							uniform_buffers_cs,
							uniforms_cs,
							textures_cs,
							pipelines_cs;

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
    // NOTE(matthew): couple Texture2D + SamplerState as one
    GSDX11_UNIFORMTYPE_TEXTURE2D,
    /* GSDX11_UNIFORMTYPE_TEXTURE_SAMPLER, */
    GSDX11_UNIFORMTYPE_BUFFER,
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

typedef struct _TAG_gsdx11_texture
{
    ID3D11Texture2D                 *tex;
    gs_graphics_texture_desc_t      desc;
    ID3D11SamplerState              *sampler;
    ID3D11ShaderResourceView        *srv;
    /* ID3D11UnorderedAccessView        *uav; */
} gsdx11_texture_t;

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
    gsdx11_buffer_t                             ibo;
} gsdx11_data_cache_t;

typedef struct _TAG_gsdx11_uniform_buffer
{
    gsdx11_buffer_t cbo;
    size_t size;
    gs_graphics_shader_stage_type stage;
} gsdx11_uniform_buffer_t;

// TODO(matthew): uniforms will serve as bindable shader resources, such as
// Texture2D, SamplerState, Buffer, RWBuffer, etc.
typedef struct _TAG_gsdx11_uniform
{
    gsdx11_uniform_type type;
    gs_graphics_shader_stage_type stage; // NOTE(matthew): assume only bindable to one stage for now
    uint32_t slot; // is this needed?
    size_t size;
} gsdx11_uniform_t;

/* typedef struct _TAG_gsdx11_uniform_list */
/* { */
/*  char name[64]; */
/*  size_t size; */
/*  gs_dyn_array(gsdx11_uniform_t) uniforms; */
/* } gsdx11_uniform_list_t; */

typedef struct _TAG_gsdx11_command_buffer
{
    ID3D11DeviceContext     *def_context;
    ID3D11CommandList       *cmd_list;
} gsdx11_command_buffer_t;

// Internal DX11 data
typedef struct _TAG_gsdx11_data
{
    gs_slot_array(gsdx11_shader_t)      shaders;
    gs_slot_array(gsdx11_buffer_t)      vertex_buffers;
    gs_slot_array(gsdx11_buffer_t)      index_buffers;
    gs_slot_array(gsdx11_uniform_buffer_t) uniform_buffers;
    gs_slot_array(gsdx11_uniform_t)     uniforms;
    gs_slot_array(gsdx11_texture_t)     textures;
    gs_slot_array(gsdx11_pipeline_t)    pipelines;
    DWORD                               tls_index;

    // Global data that I'll just put here since it's appropriate
    ID3D11Device                        *device;
    ID3D11DeviceContext                 *context; // immediate context
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



/*=============================
// Utility Functions
=============================*/

DXGI_FORMAT     gsdx11_vertex_attrib_to_dxgi_format(gs_graphics_vertex_attribute_type type);
DXGI_FORMAT     gsdx11_tex_format_to_dxgi_format(gs_graphics_texture_format_type type);
uint32_t        gsdx11_texture_wrap_to_dx11_texture_wrap(gs_graphics_texture_wrapping_type type);
uint32_t        gsdx11_texture_filter_to_dx11_texture_filter(gs_graphics_texture_filtering_type min_filter, gs_graphics_texture_filtering_type max_filter, gs_graphics_texture_filtering_type mip_filter);
size_t           gsdx11_uniform_data_size_in_bytes(gs_graphics_uniform_type type);

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
/* uint32_t         gsdx11_blend_equation_to_dx11_blend_eq(gs_graphics_blend_equation_type eq); */
/* uint32_t         gsdx11_shader_stage_to_dx11_stage(gs_graphics_shader_stage_type type); */
/* uint32_t         gsdx11_primitive_to_dx11_primitive(gs_graphics_primitive_type type); */
/* uint32_t         gsdx11_blend_mode_to_dx11_blend_mode(gs_graphics_blend_mode_type type, uint32_t def); */
/* uint32_t         gsdx11_cull_face_to_dx11_cull_face(gs_graphics_face_culling_type type); */
/* uint32_t         gsdx11_winding_order_to_dx11_winding_order(gs_graphics_winding_order_type type); */
/* uint32_t         gsdx11_depth_func_to_dx11_depth_func(gs_graphics_depth_func_type type); */
/* uint32_t         gsdx11_stencil_func_to_dx11_stencil_func(gs_graphics_stencil_func_type type); */
/* uint32_t         gsdx11_stencil_op_to_dx11_stencil_op(gs_graphics_stencil_op_type type); */
/* uint32_t         gsdx11_index_buffer_size_to_dx11_index_type(size_t sz); */
/* size_t           gsdx11_get_byte_size_of_vertex_attribute(gs_graphics_vertex_attribute_type type); */
/* size_t           gsdx11_calculate_vertex_size_in_bytes(gs_graphics_vertex_attribute_desc_t* layout, uint32_t count); */
/* size_t           gsdx11_get_vertex_attr_byte_offest(gs_dyn_array(gs_graphics_vertex_attribute_desc_t) layout, uint32_t idx); */

/*=============================
// Graphics Initialization
=============================*/

/*=============================
// Resource Creation
=============================*/

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

DXGI_FORMAT
gsdx11_tex_format_to_dxgi_format(gs_graphics_texture_format_type type)
{
    DXGI_FORMAT     format;


    // TODO(matthew): deal with unspecified formats
    switch (type)
    {
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA8:                  format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
        /* case GS_GRAPHICS_TEXTURE_FORMAT_RGB8:                    format = DXGI_FORMAT_; break; */
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA16F:                format = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA32F:                format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_R8:                     format = DXGI_FORMAT_R8_UNORM; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_A8:                     format = DXGI_FORMAT_A8_UNORM; break;
        /* case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH8:                  format = DXGI_FORMAT_; break; */
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH16:                format = DXGI_FORMAT_D16_UNORM; break;
        /* case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH24:             format = DXGI_FORMAT_; break; */
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:               format = DXGI_FORMAT_D32_FLOAT; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:       format = DXGI_FORMAT_D24_UNORM_S8_UINT; break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:      format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT; break;
        /* case GS_GRAPHICS_TEXTURE_FORMAT_STENCIL8:                format = DXGI_FORMAT_; break; */
    }

    return format;
}

uint32_t
gsdx11_texture_wrap_to_dx11_texture_wrap(gs_graphics_texture_wrapping_type type)
{
    uint32_t        wrap;


    switch (type)
    {
        default:
        case GS_GRAPHICS_TEXTURE_WRAP_REPEAT:               wrap = D3D11_TEXTURE_ADDRESS_WRAP; break;
        case GS_GRAPHICS_TEXTURE_WRAP_MIRRORED_REPEAT:      wrap = D3D11_TEXTURE_ADDRESS_MIRROR; break;
        case GS_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE:        wrap = D3D11_TEXTURE_ADDRESS_CLAMP; break;
    }

    return wrap;
}

uint32_t
gsdx11_texture_filter_to_dx11_texture_filter(gs_graphics_texture_filtering_type min_filter,
                                             gs_graphics_texture_filtering_type max_filter,
                                             gs_graphics_texture_filtering_type mip_filter)
{
    uint32_t        filter = 0;


    if (min_filter == GS_GRAPHICS_TEXTURE_FILTER_LINEAR)
    {
        if (max_filter == GS_GRAPHICS_TEXTURE_FILTER_LINEAR)
        {
            if (mip_filter == GS_GRAPHICS_TEXTURE_FILTER_LINEAR)
            {
            }
        }
    }
    else
    {
        // ... dont have a 'nearest' sampler in dx11?
    }

    return filter;
}



/*=============================
// Graphics Initialization
=============================*/

gs_graphics_t *
gs_graphics_create()
{
    gs_graphics_t       *gfx = gs_malloc_init(gs_graphics_t);
    gsdx11_data_t       *dx11;

    gfx->user_data = gs_malloc_init(gsdx11_data_t);
    dx11 = (gsdx11_data_t *)gfx->user_data;

	// NOTE(matthew): should this go in gs_graphics_init()?
    dx11->tls_index = TlsAlloc();
    if (dx11->tls_index == TLS_OUT_OF_INDEXES)
    {
        gs_assert(false);
    }
    printf("allocated tls: %d\r\n", dx11->tls_index);

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

    TlsFree(dx11->tls_index);
    printf("freed tls\r\n");

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
    gsdx11_uniform_buffer_t     ub = {0}; // ^^^
    gsdx11_uniform_t            u = {0}; // ^^
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
    gs_slot_array_insert(dx11->uniform_buffers, ub);
    gs_slot_array_insert(dx11->uniforms, u);

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

    dx11->viewport.Width = window_width;
    dx11->viewport.Height = window_height;
    dx11->viewport.MaxDepth = 1.0f;

    raster_state_desc.FillMode = D3D11_FILL_SOLID;
    raster_state_desc.CullMode = D3D11_CULL_NONE;
    hr = ID3D11Device_CreateRasterizerState(dx11->device, &raster_state_desc, &dx11->raster_state);

	// Critical section setup
	InitializeCriticalSection(&shaders_cs);
	InitializeCriticalSection(&vertex_buffers_cs);
	InitializeCriticalSection(&index_buffers_cs);
	InitializeCriticalSection(&uniform_buffers_cs);
	InitializeCriticalSection(&uniforms_cs);
	InitializeCriticalSection(&textures_cs);
	InitializeCriticalSection(&pipelines_cs);
}

void
gs_graphics_shutdown(gs_graphics_t *graphics)
{
	// TODO(matthew): actually fill this thing out!
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
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    if (desc->usage == GS_GRAPHICS_BUFFER_USAGE_DYNAMIC)
    {
        buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
        buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    }
    else
    {
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    }
    buffer_data.pSysMem = desc->data;

    if (desc->data)
        hr = ID3D11Device_CreateBuffer(dx11->device, &buffer_desc, &buffer_data, &buffer);
    else
        hr = ID3D11Device_CreateBuffer(dx11->device, &buffer_desc, NULL, &buffer);

	EnterCriticalSection(&vertex_buffers_cs);
		hndl = gs_handle_create(gs_graphics_vertex_buffer_t, gs_slot_array_insert(dx11->vertex_buffers, buffer));
	LeaveCriticalSection(&vertex_buffers_cs);

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
    if (desc->usage == GS_GRAPHICS_BUFFER_USAGE_DYNAMIC)
    {
        buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
        buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    }
    else
    {
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    }
    buffer_data.pSysMem = desc->data;

    if (desc->data)
        hr = ID3D11Device_CreateBuffer(dx11->device, &buffer_desc, &buffer_data, &buffer);
    else
        hr = ID3D11Device_CreateBuffer(dx11->device, &buffer_desc, NULL, &buffer);

	EnterCriticalSection(&index_buffers_cs);
    	hndl = gs_handle_create(gs_graphics_index_buffer_t, gs_slot_array_insert(dx11->index_buffers, buffer));
	LeaveCriticalSection(&index_buffers_cs);

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
                if  (err_blob)
                {
                    printf("\nDX11 VS COMPILE ERROR...!:\n%s\n%s\n\n", shader_src, (char *)ID3D10Blob_GetBufferPointer(err_blob));
                    gs_assert(false);
                }
                else
                {
                    hr = ID3D11Device_CreateVertexShader(dx11->device, ID3D10Blob_GetBufferPointer(shader.vsblob),
                            ID3D10Blob_GetBufferSize(shader.vsblob), 0, &shader.vs);
                    shader.tag |= GS_DX11_SHADER_TYPE_VERTEX;
                }
            } break;

            case GS_GRAPHICS_SHADER_STAGE_FRAGMENT:
            {
                hr = D3DCompile(shader_src, shader_len, NULL, NULL, NULL, "main", "ps_5_0",
                        0, 0, &shader.psblob, &err_blob);
                if  (err_blob)
                {
                    printf("\nDX11 PS COMPILE ERROR...!:\n%s\n%s\n\n", shader_src, (char *)ID3D10Blob_GetBufferPointer(err_blob));
                    gs_assert(false);
                }
                else
                {
                    hr = ID3D11Device_CreatePixelShader(dx11->device, ID3D10Blob_GetBufferPointer(shader.psblob),
                            ID3D10Blob_GetBufferSize(shader.psblob), 0, &shader.ps);
                    shader.tag |= GS_DX11_SHADER_TYPE_PIXEL;
                }
            } break;
        }
    }

	EnterCriticalSection(&shaders_cs);
    	hndl = gs_handle_create(gs_graphics_shader_t, gs_slot_array_insert(dx11->shaders, shader));
	LeaveCriticalSection(&shaders_cs);

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

	EnterCriticalSection(&pipelines_cs);
    	hndl = gs_handle_create(gs_graphics_pipeline_t, gs_slot_array_insert(dx11->pipelines, pipe));
	LeaveCriticalSection(&pipelines_cs);

    return hndl;
}

gs_handle(gs_graphics_uniform_buffer_t)
gs_graphics_uniform_buffer_create(const gs_graphics_uniform_buffer_desc_t *desc)
{
    HRESULT                                     hr;
    gsdx11_data_t                               *dx11;
    gsdx11_uniform_buffer_t                     ub = gs_default_val();
    D3D11_BUFFER_DESC                           buffer_desc = {0};
    D3D11_SUBRESOURCE_DATA                      buffer_data = {0};
    gs_handle(gs_graphics_uniform_buffer_t)     hndl;


    dx11 = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;

    // TODO(matthew): compare these flags to what's specified in the desc (ie,
    // dynamic, CPU access)
    buffer_desc.ByteWidth = desc->size;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    if (desc->usage == GS_GRAPHICS_BUFFER_USAGE_DYNAMIC)
    {
        buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
        buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    }
    else
    {
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    }
    buffer_data.pSysMem = desc->data;

    ub.size = desc->size;
    ub.stage = desc->stage;

    if (desc->data)
        hr = ID3D11Device_CreateBuffer(dx11->device, &buffer_desc, &buffer_data,  &ub.cbo);
    else
        hr = ID3D11Device_CreateBuffer(dx11->device, &buffer_desc, NULL,  &ub.cbo);

	EnterCriticalSection(&uniform_buffers_cs);
    	hndl = gs_handle_create(gs_graphics_uniform_buffer_t, gs_slot_array_insert(dx11->uniform_buffers, ub));
	LeaveCriticalSection(&uniform_buffers_cs);

    return hndl;
}

gs_handle(gs_graphics_texture_t)
gs_graphics_texture_create(const gs_graphics_texture_desc_t* desc)
{
    gsdx11_data_t                           *dx11;
    gsdx11_texture_t                        tex;
    gs_handle(gs_graphics_texture_t)        hndl;


    dx11 = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;
    tex = dx11_texture_create_internal(desc);

	EnterCriticalSection(&textures_cs);
		hndl = gs_handle_create(gs_graphics_texture_t, gs_slot_array_insert(dx11->textures, tex));
	LeaveCriticalSection(&textures_cs);

    return hndl;
}

gsdx11_texture_t
dx11_texture_create_internal(const gs_graphics_texture_desc_t* desc)
{
    HRESULT                             hr;
    gsdx11_data_t                       *dx11;
    gsdx11_texture_t                    tex = gs_default_val();
    D3D11_TEXTURE2D_DESC                tex_desc;
    D3D11_SUBRESOURCE_DATA              tex_data;
    D3D11_SAMPLER_DESC                  sampler_desc = gs_default_val();
    D3D11_SHADER_RESOURCE_VIEW_DESC     srv_desc;


    dx11 = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;

    // NOTE(matthew): not everything here is checked in the desc!
    // Create texture
    tex_desc.Width = desc->width;
    tex_desc.Height = desc->height;
    tex_desc.Format = gsdx11_tex_format_to_dxgi_format(desc->format);
    tex_desc.Usage = D3D11_USAGE_DEFAULT;
    tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    tex_desc.CPUAccessFlags = 0;
    tex_desc.MiscFlags = 0;
    tex_data.pSysMem = desc->data;
    tex_data.SysMemPitch = desc->width * 4; // NOTE(matthew): assuming RGBA8 for now
    tex_data.SysMemSlicePitch = 0;
    // NOTE(matthew): we're ignoring mipmaps for now
    tex_desc.MipLevels = 1;
    tex_desc.ArraySize = 1;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.SampleDesc.Quality = 0;

    hr = ID3D11Device_CreateTexture2D(dx11->device, &tex_desc, &tex_data, &tex.tex);

    // Create sampler
    // NOTE(matthew): fuck it, just use point for everything for now (since that's what the example app needs)
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampler_desc.AddressU = gsdx11_texture_wrap_to_dx11_texture_wrap(desc->wrap_s);
    sampler_desc.AddressV = gsdx11_texture_wrap_to_dx11_texture_wrap(desc->wrap_t);
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.MipLODBias = 0.0f;
    sampler_desc.MaxAnisotropy = 8;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampler_desc.MinLOD = 0.0f;
    sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = ID3D11Device_CreateSamplerState(dx11->device, &sampler_desc, &tex.sampler);

    // Create SRV
    srv_desc.Format = tex_desc.Format;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;
    srv_desc.Texture2D.MostDetailedMip = 0;

    hr = ID3D11Device_CreateShaderResourceView(dx11->device, tex.tex, &srv_desc, &tex.srv);

    return tex;
}

gs_handle(gs_graphics_uniform_t)
gs_graphics_uniform_create(const gs_graphics_uniform_desc_t* desc)
{
    gsdx11_data_t                           *dx11;
    gsdx11_uniform_t                        uniform;
    gs_handle(gs_graphics_uniform_t)        hndl;


    dx11 = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;

    // NOTE(matthew): assuming only 2D texture for now
    uniform.type = GSDX11_UNIFORMTYPE_TEXTURE2D;
    uniform.size = sizeof(gsdx11_texture_t);
    uniform.stage = desc->stage;

	EnterCriticalSection(&uniforms_cs);
    	hndl = gs_handle_create(gs_graphics_uniform_t, gs_slot_array_insert(dx11->uniforms, uniform));
	LeaveCriticalSection(&uniforms_cs);

    return hndl;
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
    HRESULT                             hr;
    gsdx11_data_t                       *dx11;
    gsdx11_command_buffer_t             *cmdbuffer;
    gsdx11_uniform_buffer_t             ub;
    uint32_t                            id = hndl.id;
    gs_graphics_buffer_usage_type       usage = desc->usage;
    size_t                              sz = desc->size,
                                        offset = desc->update.offset;
    gs_graphics_buffer_update_type      update_type = desc->update.type;
    void                                *data = desc->data;
    D3D11_MAPPED_SUBRESOURCE            resource = {0};


    dx11 = (gsdx11_data_t*)gs_engine_subsystem(graphics)->user_data;
    cmdbuffer = TlsGetValue(dx11->tls_index);

    ub = gs_slot_array_get(dx11->uniform_buffers, hndl.id);

    switch (update_type)
    {
        case GS_GRAPHICS_BUFFER_UPDATE_SUBDATA:
        {
            // Map resource to CPU side and sub data
            // TODO(matthew): BAD! Better to use UpdateSubresource() instead,
            // but there are constraints on the data that can be filled (ie,
            // US() does not take an offset parameter, and GS does not specify whether
            // the data ptr has to fully overlap the underlying type structure.
            hr = ID3D11DeviceContext_Map(cmdbuffer->def_context, ub.cbo, 0,
                    D3D11_MAP_WRITE_DISCARD, 0, &resource);
            memcpy((char *)resource.pData + offset, data, sz);
            ID3D11DeviceContext_Unmap(dx11->context, ub.cbo, 0);
        } break;
    }
}



/*=============================
// Command Buffers Ops
=============================*/

// TODO(matthew): The solution to ensuring thread safety when doing multithreaded
// rendering is quite simple. Each gs_command_buffer_t will have to (internally)
// contain a deferred context and a command list. Each time we call a gs_graphics_XXX()
// function that takes a command buffer, we need to simply make the API calls using the
// deferred context inside that command buffer. Then, when we call to submit the command
// buffer, we just execute the command list from the global immediate context.
// This means that the op-code system we've been using is no longer necessary. All we need
// to do is replace all the __dx11_push_command(...) calls inside each of these functions
// with the appropriate API calls through the command buffer's deferred context.
// We'll use TLS to create a per-thread command buffer.

void
gs_graphics_begin_render_pass(gs_command_buffer_t *cb,
                              gs_handle(gs_graphics_render_pass_t) hndl)
{
    HRESULT                     hr;
    gsdx11_data_t               *dx11;
    gsdx11_command_buffer_t     *cmdbuffer;


    dx11 = (gsdx11_data_t*)gs_engine_subsystem(graphics)->user_data;

    // get cmdbuffer
    cmdbuffer = TlsGetValue(dx11->tls_index);

    // if it hasn't been created yet
    if (!cmdbuffer)
    {
        cmdbuffer = (gsdx11_command_buffer_t *)LocalAlloc(LPTR, sizeof(*cmdbuffer));
        if (!TlsSetValue(dx11->tls_index, cmdbuffer))
        {
            gs_assert(false);
        }

        hr = ID3D11Device_CreateDeferredContext(dx11->device, 0, &cmdbuffer->def_context);
    }

    ID3D11DeviceContext_OMSetRenderTargets(cmdbuffer->def_context, 1, &dx11->rtv, dx11->dsv);
    ID3D11DeviceContext_RSSetState(cmdbuffer->def_context, dx11->raster_state);
}

void
gs_graphics_end_render_pass(gs_command_buffer_t *cb)
{
    HRESULT                     hr;
    gsdx11_data_t               *dx11;
    gsdx11_command_buffer_t     *cmdbuffer;


    dx11 = (gsdx11_data_t*)gs_engine_subsystem(graphics)->user_data;

    cmdbuffer = TlsGetValue(dx11->tls_index);
    if (!cmdbuffer)
    {
        gs_assert(false);
    }

    hr = ID3D11DeviceContext_FinishCommandList(cmdbuffer->def_context, false, &cmdbuffer->cmd_list);
}

void
gs_graphics_clear(gs_command_buffer_t *cb,
                  gs_graphics_clear_desc_t *desc)
{
    uint32_t                        count = !desc->actions ? 0 : !desc->size ? 1 : (uint32_t)((size_t)desc->size / (size_t)sizeof(gs_graphics_clear_action_t));
    uint32_t                        bit = 0x00;
    gsdx11_data_t                   *dx11;
    gsdx11_command_buffer_t         *cmdbuffer;
    gs_graphics_clear_action_t      action;


    dx11 = (gsdx11_data_t*)gs_engine_subsystem(graphics)->user_data;
    cmdbuffer = TlsGetValue(dx11->tls_index);

    for (uint32_t i = 0; i < count; i++)
    {
        action = desc->actions[i];

        if (action.flag & GS_GRAPHICS_CLEAR_NONE)
            continue;

        if (action.flag & GS_GRAPHICS_CLEAR_COLOR || action.flag == 0x00)
        {
            float       bgcolor[4] = {action.color[0], action.color[1], action.color[2], action.color[3]};
            ID3D11DeviceContext_ClearRenderTargetView(cmdbuffer->def_context, dx11->rtv, bgcolor);
        }
        if (action.flag & GS_GRAPHICS_CLEAR_DEPTH || action.flag == 0x00)
        {
            bit |= D3D11_CLEAR_DEPTH;
        }
        if (action.flag & GS_GRAPHICS_CLEAR_STENCIL || action.flag == 0x00)
        {
            bit |= D3D11_CLEAR_STENCIL;
        }

        ID3D11DeviceContext_ClearDepthStencilView(cmdbuffer->def_context, dx11->dsv, bit, 1.0f, 0);
    }
}

void
gs_graphics_set_viewport(gs_command_buffer_t *cb,
                         uint32_t x,
                         uint32_t y,
                         uint32_t w,
                         uint32_t h)
{
    gsdx11_data_t               *dx11;
    gsdx11_command_buffer_t     *cmdbuffer;


    dx11 = (gsdx11_data_t*)gs_engine_subsystem(graphics)->user_data;
    cmdbuffer = TlsGetValue(dx11->tls_index);

    dx11->viewport.TopLeftX = x;
    dx11->viewport.TopLeftY = y;
    dx11->viewport.Width = w;
    dx11->viewport.Height = h;

    ID3D11DeviceContext_RSSetViewports(cmdbuffer->def_context, 1, &dx11->viewport);
}

void
gs_graphics_apply_bindings(gs_command_buffer_t *cb,
                           gs_graphics_bind_desc_t *binds)
{
    gsdx11_data_t               *dx11;
    gsdx11_command_buffer_t     *cmdbuffer;
    gsdx11_pipeline_t           *pipe;
    uint32_t                    vcnt,   // vertex buffers to bind
                                icnt,   // index buffers to bind
                                ubcnt,  // uniform buffers to bind
                                ucnt,   // uniforms to bind
                                cnt;    // total objects to bind
    int                         layout_cnt,
                                layout_idx = 0;


    dx11 = (gsdx11_data_t*)gs_engine_subsystem(graphics)->user_data;
    cmdbuffer = TlsGetValue(dx11->tls_index);
    pipe  = gs_slot_array_getp(dx11->pipelines, dx11->cache.pipeline.id);
    layout_cnt = gs_dyn_array_size(pipe->layout);

    vcnt = binds->vertex_buffers.desc ? binds->vertex_buffers.size ? binds->vertex_buffers.size / sizeof(gs_graphics_bind_vertex_buffer_desc_t) : 1 : 0;
    icnt = binds->index_buffers.desc ? binds->index_buffers.size ? binds->index_buffers.size / sizeof(gs_graphics_bind_index_buffer_desc_t) : 1 : 0;
    ubcnt = binds->uniform_buffers.desc ? binds->uniform_buffers.size ? binds->uniform_buffers.size / sizeof(gs_graphics_bind_uniform_buffer_desc_t) : 1 : 0;
    ucnt = binds->uniforms.desc ? binds->uniforms.size ? binds->uniforms.size / sizeof(gs_graphics_bind_uniform_desc_t) : 1 : 0;

    // vertex buffers
    for (int i = 0; i < vcnt; i++)
    {
        uint32_t                                stride = 0,
                                                vbo_slot,
                                                vbo_offset;
        gsdx11_buffer_t                         vbo;
        int                                     prev_slot = 0;
        gs_graphics_bind_vertex_buffer_desc_t   *decl = &binds->vertex_buffers.desc[i];

        vbo_offset = decl->offset;

        do
        {
            uint32_t                        offset = pipe->layout[layout_idx].offset;
            D3D11_INPUT_ELEMENT_DESC        layout_desc = gs_default_val();

            stride += pipe->layout[layout_idx].stride;
            vbo = gs_slot_array_get(dx11->vertex_buffers, decl->buffer.id);
            vbo_slot = pipe->layout[layout_idx].buffer_idx;

            layout_desc.SemanticName = pipe->layout[layout_idx].name;
            layout_desc.Format = gsdx11_vertex_attrib_to_dxgi_format(pipe->layout[layout_idx].format);
            layout_desc.InputSlot = vbo_slot;
            layout_desc.AlignedByteOffset = offset;
            if (pipe->layout[layout_idx].divisor)
            {
                layout_desc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
                layout_desc.InstanceDataStepRate = pipe->layout[layout_idx].divisor;
            }

            gs_dyn_array_push(dx11->cache.layout_descs, layout_desc);

            prev_slot = vbo_slot;
            layout_idx++;
        } while ((pipe->layout[layout_idx].buffer_idx == prev_slot) && (layout_idx < layout_cnt));

        ID3D11DeviceContext_IASetVertexBuffers(cmdbuffer->def_context, vbo_slot, 1, &vbo, &stride, &vbo_offset);
    }

    // index buffers
    for (int i = 0; i < icnt; i++)
    {
        gsdx11_buffer_t                             ibo;
        gs_graphics_bind_index_buffer_desc_t        *decl;

        decl = &binds->index_buffers.desc[i];
        ibo = gs_slot_array_get(dx11->index_buffers, decl->buffer.id);
        dx11->cache.ibo = ibo;

        ID3D11DeviceContext_IASetIndexBuffer(cmdbuffer->def_context, ibo, DXGI_FORMAT_R32_UINT, 0);
    }

    // uniforms
    for (int i = 0; i < ucnt; i++)
    {
        gs_graphics_bind_uniform_desc_t     *decl;
        gsdx11_uniform_t                    *u;
        uint32_t                            binding;
        size_t                              sz;

        decl = &binds->uniforms.desc[i];
        sz = 1;
        binding = decl->binding;

        // Check buffer id. If invalid, then we can't operate, and instead just need to pass over the data.
        if (!decl->uniform.id || !gs_slot_array_exists(dx11->uniforms, decl->uniform.id)) {
            gs_timed_action(60, {
                gs_println("Warning:Bind Uniform:Uniform %d does not exist.", decl->uniform.id);
            });
            continue;
        }

        u = gs_slot_array_getp(dx11->uniforms, decl->uniform.id);

        switch (u->type)
        {
            case GSDX11_UNIFORMTYPE_TEXTURE2D:
            {
                gsdx11_texture_t                                *tex;
                gs_handle(gs_graphics_texture_t)        hndl;

                hndl = *(gs_handle(gs_graphics_texture_t) *)decl->data;
                tex = gs_slot_array_getp(dx11->textures, hndl.id);

                ID3D11DeviceContext_PSSetShaderResources(cmdbuffer->def_context, binding, 1, &tex->srv);
                ID3D11DeviceContext_PSSetSamplers(cmdbuffer->def_context, binding, 1, &tex->sampler);
            } break;
        }
    }

    // uniform buffers
    for (int i = 0; i < ubcnt; i++)
    {
        gsdx11_uniform_buffer_t                     *ub;
        gs_graphics_bind_uniform_buffer_desc_t      *decl;

        decl = &binds->uniform_buffers.desc[i];
        ub = gs_slot_array_getp(dx11->uniform_buffers, decl->buffer.id);

        if (ub->stage == GS_GRAPHICS_SHADER_STAGE_VERTEX)
        {
            ID3D11DeviceContext_VSSetConstantBuffers(cmdbuffer->def_context, decl->binding, 1, &ub->cbo);
        }
        else if (ub->stage == GS_GRAPHICS_SHADER_STAGE_FRAGMENT)
        {
            ID3D11DeviceContext_PSSetConstantBuffers(cmdbuffer->def_context, decl->binding, 1, &ub->cbo);
        }
    }

    // create input layout
    if (!dx11->cache.layout)
    {
        ID3D11Device_CreateInputLayout(dx11->device, dx11->cache.layout_descs,
                gs_dyn_array_size(dx11->cache.layout_descs),
                ID3D10Blob_GetBufferPointer(dx11->cache.shader.vsblob),
                ID3D10Blob_GetBufferSize(dx11->cache.shader.vsblob),
                &dx11->cache.layout);
    }
    ID3D11DeviceContext_IASetInputLayout(cmdbuffer->def_context, dx11->cache.layout);
    ID3D11DeviceContext_IASetPrimitiveTopology(cmdbuffer->def_context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void
gs_graphics_bind_pipeline(gs_command_buffer_t *cb,
                          gs_handle(gs_graphics_pipeline_t) hndl)
{
    gsdx11_data_t               *dx11;
    gsdx11_command_buffer_t     *cmdbuffer;
    gsdx11_pipeline_t           *pipe;
    gsdx11_shader_t             shader;


    dx11 = (gsdx11_data_t*)gs_engine_subsystem(graphics)->user_data;
    cmdbuffer = TlsGetValue(dx11->tls_index);

    dx11->cache.pipeline = gs_handle_create(gs_graphics_pipeline_t, hndl.id);
    pipe = gs_slot_array_getp(dx11->pipelines, hndl.id);

    if (pipe->raster.shader.id && gs_slot_array_exists(dx11->shaders, pipe->raster.shader.id))
    {
        shader = gs_slot_array_get(dx11->shaders, pipe->raster.shader.id);

        if (shader.tag & GS_DX11_SHADER_TYPE_VERTEX)
            ID3D11DeviceContext_VSSetShader(cmdbuffer->def_context, shader.vs, 0, 0);
        if (shader.tag & GS_DX11_SHADER_TYPE_PIXEL)
            ID3D11DeviceContext_PSSetShader(cmdbuffer->def_context, shader.ps, 0, 0);

        dx11->cache.shader = shader;
    }
}

void
gs_graphics_draw(gs_command_buffer_t *cb,
                 gs_graphics_draw_desc_t *desc)
{
    gsdx11_data_t               *dx11;
    gsdx11_command_buffer_t     *cmdbuffer;


    dx11 = (gsdx11_data_t*)gs_engine_subsystem(graphics)->user_data;
    cmdbuffer = TlsGetValue(dx11->tls_index);

    if (dx11->cache.ibo)
    {
        // TODO(matthew): check other params in the draw_desc
        if (desc->instances)
            ID3D11DeviceContext_DrawIndexedInstanced(cmdbuffer->def_context, desc->count, desc->instances, desc->start, 0, 0);
        else
            ID3D11DeviceContext_DrawIndexed(cmdbuffer->def_context, desc->count, desc->start, 0);
    }
    else
    {
        if (desc->instances)
            ID3D11DeviceContext_DrawInstanced(cmdbuffer->def_context, desc->count, desc->instances, desc->start, 0);
        else
            ID3D11DeviceContext_Draw(cmdbuffer->def_context, desc->count, desc->start);
    }
}


/* Submission (Main Thread) */
void
gs_graphics_submit_command_buffer(gs_command_buffer_t *cb)
{
    HRESULT             hr;
    gsdx11_data_t       *dx11;
    gsdx11_command_buffer_t     *cmdbuffer;


    dx11 = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;
    cmdbuffer = TlsGetValue(dx11->tls_index);

    ID3D11DeviceContext_ExecuteCommandList(dx11->context, cmdbuffer->cmd_list, false);

    hr = IDXGISwapChain_Present(dx11->swapchain, 0, 0);
}

#endif // GS_DX11_IMPL_H

