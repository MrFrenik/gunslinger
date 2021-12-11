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

// All necessary graphics data for this example (shader source/vertex data)
gs_command_buffer_t						cb			= {0};
gs_handle(gs_graphics_vertex_buffer_t)	vbo     	= {0};
gs_handle(gs_graphics_index_buffer_t)	ibo			= {0};
gs_handle(gs_graphics_shader_t) 		shaders[2] 	= {0};
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

	ID3D11VertexShader					*vs;
	ID3D11PixelShader					*ps;
	size_t								sz;
	char								*vs_src,
										*ps_src;
	ID3DBlob							*vsblob,
										*psblob;
	gs_graphics_shader_desc_t 			shader_desc = {0};
	gs_graphics_shader_source_desc_t	src_desc = {0};

	vs_src = gs_read_file_contents_into_string_null_term("vertex.hlsl", "rb", &sz);
	ps_src = gs_read_file_contents_into_string_null_term("pixel.hlsl", "rb", &sz);

	src_desc.type = GS_GRAPHICS_SHADER_STAGE_VERTEX;
	src_desc.source = vs_src;
	shader_desc.sources = &src_desc;
	shader_desc.size = 1;
	shaders[0] = gs_graphics_shader_create(&shader_desc);

	src_desc.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT;
	src_desc.source = ps_src;
	shaders[1] = gs_graphics_shader_create(&shader_desc);

	vs = gs_slot_array_get(dx11->shaders, 1).vs;
	ps = gs_slot_array_get(dx11->shaders, 2).ps;
	vsblob = gs_slot_array_get(dx11->shaders, 1).blob;
	psblob = gs_slot_array_get(dx11->shaders, 2).blob;

	ID3D11DeviceContext_VSSetShader(dx11->context, vs, 0, 0);
	ID3D11DeviceContext_PSSetShader(dx11->context, ps, 0, 0);

	///////////////////////////////////////////////////////////////////////////
	// Buffer Setup

	UINT	 stride = 3 * sizeof(float),
			 offset = 0;
	gs_graphics_vertex_buffer_desc_t desc = {0};
	float v_data[] =
	{
		0.0f, 0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
		0.8f, 0.5f, 0.5f
	};
	UINT idx_data[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	desc.data = v_data;
	desc.size = sizeof(v_data);
	vbo = gs_graphics_vertex_buffer_create(&desc);

	desc.data = idx_data;
	desc.size = sizeof(idx_data);
	ibo = gs_graphics_index_buffer_create(&desc);

	ID3D11DeviceContext_IASetVertexBuffers(dx11->context, 0, 1, (gs_slot_array_getp(dx11->vertex_buffers, 1)), &stride, &offset);
	ID3D11DeviceContext_IASetIndexBuffer(dx11->context, gs_slot_array_get(dx11->index_buffers, 1), DXGI_FORMAT_R32_UINT, 0);

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
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC))
		gs_engine_quit();

	// RENDER
	float bgcolor[] = {0.1, 0.1, 0.1, 1.0};
	gs_graphics_clear_desc_t clear = {0};

	clear.actions = &(gs_graphics_clear_action_t){.color = {0.1f, 0.3f, 0.9f, 1.0f}};

	gs_graphics_clear(&cb, &clear);
	gs_graphics_submit_command_buffer(&cb);

	ID3D11DeviceContext_DrawIndexed(dx11->context, 6, 0, 0);
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
