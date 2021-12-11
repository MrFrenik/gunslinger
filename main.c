/*================================================================
    * Copyright: 2020 John Jackson
    * simple_triangle

    The purpose of this example is to demonstrate how to explicitly construct
    GPU resources to use for your application to render a basic triangle.

    Included:
        * Construct vertex/index buffers from user defined declarations
        * Construct shaders from source
        * Construct pipelines
        * Rendering via command buffers

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#define GS_GRAPHICS_IMPL_CUSTOM
#include "gs_dx11_impl.h"
#include "gs/gs.h"

#define SCR_WIDTH	1024
#define SCR_HEIGHT	768

// All necessary graphics data for this example (shader source/vertex data)
gs_command_buffer_t						cb			= {0};
gs_handle(gs_graphics_vertex_buffer_t)	vbo     	= {0};
gs_handle(gs_graphics_index_buffer_t)	ibo			= {0};
gs_handle(gs_graphics_shader_t) 		shader     	= {0};
gs_handle(gs_graphics_pipeline_t)		pipe		= {0};
gs_graphics_t *graphics;
gsdx11_data_t *dx11;

void init()
{
	HRESULT 				hr;

	graphics = gs_engine_subsystem(graphics);
	dx11 = (gsdx11_data_t *)graphics->user_data;
	cb = gs_command_buffer_new();

	///////////////////////////////////////////////////////////////////////////
	// Shader Setup

	size_t								sz;
	char								*vs_src,
										*ps_src;
	ID3DBlob							*vsblob,
										*psblob;
	gs_graphics_pipeline_desc_t			pipe_desc = {0};


	vs_src = gs_read_file_contents_into_string_null_term("vertex.hlsl", "rb", &sz);
	ps_src = gs_read_file_contents_into_string_null_term("pixel.hlsl", "rb", &sz);

    shader = gs_graphics_shader_create (
        &(gs_graphics_shader_desc_t) {
            .sources = (gs_graphics_shader_source_desc_t[]) {
                {.type = GS_GRAPHICS_SHADER_STAGE_VERTEX, .source = vs_src},
                {.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = ps_src}
            }, 
            .size = 2 * sizeof(gs_graphics_shader_source_desc_t),
            .name = "triangle"
        }
    );


	vsblob = gs_slot_array_get(dx11->shaders, shader.id).vsblob;
	psblob = gs_slot_array_get(dx11->shaders, shader.id).psblob;

    pipe = gs_graphics_pipeline_create (
        &(gs_graphics_pipeline_desc_t) {
            .raster = {
                .shader = shader
            },
            .layout = {
                .attrs = (gs_graphics_vertex_attribute_desc_t[]){
                    {.stride = 12, .format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3, .name = "POSITION"}    // Named attribute required for lower GL versions / ES2 / ES3
                },
                .size = sizeof(gs_graphics_vertex_attribute_desc_t)
            }
        }
    );
	

	///////////////////////////////////////////////////////////////////////////
	// Buffer Setup

	UINT	 stride = 3 * sizeof(float),
			 offset = 0;
	float v_data[] =
	{
		0.0f, 0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
	};

    vbo = gs_graphics_vertex_buffer_create(
        &(gs_graphics_vertex_buffer_desc_t) {
            .data = v_data,
            .size = sizeof(v_data)
        }
    );

	///////////////////////////////////////////////////////////////////////////
	// Input Layout Setup

	ID3D11InputLayout				*Layout;
	D3D11_INPUT_ELEMENT_DESC		LayoutDesc;

	LayoutDesc.SemanticName = "POSITION";
	LayoutDesc.SemanticIndex = 0;
	LayoutDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	LayoutDesc.InputSlot = 0;
	LayoutDesc.AlignedByteOffset = 0;
	LayoutDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	LayoutDesc.InstanceDataStepRate = 0;

	hr = ID3D11Device_CreateInputLayout(dx11->device, &LayoutDesc, 1, ID3D10Blob_GetBufferPointer(vsblob), ID3D10Blob_GetBufferSize(vsblob), &Layout);
	ID3D11DeviceContext_IASetInputLayout(dx11->context, Layout);
	ID3D11DeviceContext_IASetPrimitiveTopology(dx11->context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// RENDER
	gs_graphics_clear_desc_t clear = {0};

    gs_graphics_bind_desc_t binds = {
        .vertex_buffers = {&(gs_graphics_bind_vertex_buffer_desc_t){.buffer = vbo}}
    };

	clear.actions = &(gs_graphics_clear_action_t){.color = {0.1f, 0.1f, 0.1f, 1.0f}};

	gs_graphics_clear(&cb, &clear);
	gs_graphics_bind_pipeline(&cb, pipe);
    gs_graphics_apply_bindings(&cb, &binds);                                   // Bind all bindings (just vertex buffer)
	gs_graphics_draw(&cb, &(gs_graphics_draw_desc_t){.start = 0, .count = 3});
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC))
		gs_engine_quit();

	gs_graphics_submit_command_buffer(&cb);

	ID3D11DeviceContext_Draw(dx11->context, 3, 0);
	IDXGISwapChain_Present(dx11->swapchain, 0, 0);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t)
	{
		.window_width = 1024,
		.window_height = 768,
		.window_title = "DX11 Test",
        .init = init,
		.update = update
    };
}

