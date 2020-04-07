#ifndef __GS_GRAPHICS_SUBSYSTEM_H__
#define __GS_GRAPHICS_SUBSYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gs_containers.h"
#include "math/gs_math.h"

typedef enum gs_shader_program_type
{
	gs_vertex_program = 0,
	gs_fragment_program,
	gs_geometry_program,
	gs_compute_program
} gs_shader_program_type;

typedef enum gs_shader_language_type
{
	gs_glsl = 0
} gs_shader_language_type;

typedef struct gs_shader_program_desc
{
	gs_shader_program_type 		type;
	gs_shader_language_type 	language;
	const char* 				source;
} gs_shader_program_desc;

typedef enum gs_uniform_type
{
	gs_uniform_type_float = 0,
	gs_uniform_type_int,
	gs_uniform_type_vec2,
	gs_uniform_type_vec3,
	gs_uniform_type_vec4,
	gs_uniform_type_mat4,
	gs_uniform_type_sampler2d
} gs_uniform_type;

// Want to give a vertex buffer layout description (to set attributes)
typedef enum gs_vertex_attribute_type
{
	gs_vertex_attribute_float4 = 0,
	gs_vertex_attribute_float3,
	gs_vertex_attribute_float2,
	gs_vertex_attribute_float,
	gs_vertex_attribute_uint4,
	gs_vertex_attribute_uint3,
	gs_vertex_attribute_uint2,
	gs_vertex_attribute_uint
} gs_vertex_attribute_type;

/*================
// Texture
=================*/

typedef enum gs_texture_format 
{
	gs_texture_format_rgba8,
	gs_texture_format_rgb8,
	gs_texture_format_rgba16f
} gs_texture_format;

typedef enum gs_texture_wrapping
{
	gs_repeat,
	gs_mirrored_repeat,
	gs_clamp_to_edge,
	gs_clamp_to_border
} gs_texture_wrapping;

typedef enum gs_texture_filtering
{
	gs_nearest,
	gs_linear
} gs_texture_filtering;

typedef struct gs_texture_parameter_desc
{
	gs_texture_wrapping texture_wrap_s;
	gs_texture_wrapping texture_wrap_t;
	gs_texture_filtering min_filter;
	gs_texture_filtering mag_filter;
	gs_texture_filtering mipmap_filter;
	f32 border_color[4];
	b32 generate_mips;
	gs_texture_format texture_format;
	void* data;
	u32 width;
	u32 height;
	u32 num_comps;
} gs_texture_parameter_desc;

typedef struct gs_debug_draw_properties
{
	gs_mat4 view_mat;
	gs_mat4 proj_mat;
} gs_debug_draw_properties;

/*================
// Resource Decls
=================*/

#define gs_resource( type )\
	gs_resource_##type

// Strongly typed declarations for resource handles (slot array handles)
#define gs_declare_resource_type( type )\
	typedef struct gs_resource( type ) {\
		u32 id;\
	} gs_resource( type );\

gs_declare_resource_type( gs_command_buffer );
gs_declare_resource_type( gs_uniform_buffer );
gs_declare_resource_type( gs_vertex_buffer );
gs_declare_resource_type( gs_index_buffer );
gs_declare_resource_type( gs_texture );
gs_declare_resource_type( gs_shader );
gs_declare_resource_type( gs_uniform );
gs_declare_resource_type( gs_vertex_attribute_layout_desc );
gs_declare_resource_type( gs_render_target );
gs_declare_resource_type( gs_frame_buffer );

// This will not be abstracted, however the renderer absolutely could be 
typedef struct gs_graphics_i
{
	/*============================================================
	// Graphics Initilization / De-Initialization
	============================================================*/
	gs_result ( * init )( struct gs_graphics_i* );
	gs_result ( * shutdown )( struct gs_graphics_i* );
	gs_result ( * update )();

	/*============================================================
	// Graphics Command Buffer Ops
	============================================================*/
	void ( * reset_command_buffer )( gs_resource( gs_command_buffer ) );
	void ( * set_depth_enabled )( gs_resource( gs_command_buffer ), b32 );
	void ( * set_view_port)( gs_resource( gs_command_buffer ), u32 width, u32 height );
	void ( * bind_frame_buffer )( gs_resource( gs_command_buffer ), gs_resource( gs_frame_buffer ) );
	void ( * set_frame_buffer_attachment )( gs_resource( gs_command_buffer ), gs_resource( gs_texture ), u32 idx );
	void ( * unbind_frame_buffer )( gs_resource( gs_command_buffer ) );
	void ( * bind_shader )( gs_resource( gs_command_buffer ), gs_resource( gs_shader ) );
	void ( * bind_uniform )( gs_resource( gs_command_buffer ), gs_resource( gs_uniform ), void* );
	void ( * bind_vertex_buffer )( gs_resource( gs_command_buffer ), gs_resource( gs_vertex_buffer ) );
	void ( * bind_index_buffer )( gs_resource( gs_command_buffer ), gs_resource( gs_index_buffer ) );
	void ( * bind_texture )( gs_resource( gs_command_buffer ), gs_resource( gs_uniform ), gs_resource( gs_texture ), u32 );
	void ( * set_view_clear )( gs_resource( gs_command_buffer ), f32* color );
	void ( * draw )( gs_resource( gs_command_buffer ), u32 start, u32 count );
	void ( * draw_indexed )( gs_resource( gs_command_buffer ), u32 count );
	void ( * submit_command_buffer )( gs_resource( gs_command_buffer ) );
	// void ( * set_uniform_buffer_sub_data )( gs_resource( gs_command_buffer ), gs_resource( gs_uniform_buffer ), void*, usize );

	/*============================================================
	// Graphics Resource Construction
	============================================================*/
	gs_resource( gs_vertex_buffer )( * construct_vertex_buffer )( gs_vertex_attribute_type*, u32, void*, usize );
	gs_resource( gs_shader )( * construct_shader )( const char* vert_src, const char* frag_src );
	gs_resource( gs_uniform )( * construct_uniform )( gs_resource( gs_shader ), const char* uniform_name, gs_uniform_type );
	gs_resource( gs_command_buffer )( * construct_command_buffer )();
	gs_resource( gs_render_target )( * construct_render_target )( gs_texture_parameter_desc );	// Will eventually set this so you can have a number of targets for MRT (is this even necessary)?
	gs_resource( gs_frame_buffer )( * construct_frame_buffer )( gs_resource( gs_texture ) );
	void* ( * load_texture_data_from_file )( const char* file_path, b32 flip_vertically_on_load, gs_texture_format,
					s32* width, s32* height, s32* num_comps );

	// Will construct texture resource and let user free data...for now
	gs_resource( gs_texture )( * construct_texture )( gs_texture_parameter_desc );
	gs_resource( gs_texture )( * construct_texture_from_file )( const char* file_path, b32 flip_vertically_on_load, gs_texture_parameter_desc t_desc );
	gs_resource( gs_index_buffer )( * construct_index_buffer )( void*, usize );

	/*============================================================
	// Graphics Resource Free Ops
	============================================================*/
	void ( * free_vertex_attribute_layout_desc )( gs_resource( gs_vertex_attribute_layout_desc ) );
	void ( * free_vertex_buffer )( gs_resource( gs_vertex_buffer ) );
	void ( * free_index_buffer )( gs_resource( gs_index_buffer ) );
	void ( * free_shader )( gs_resource( gs_shader ) );
	// void ( * free_uniform_buffer )( gs_resource( gs_uniform_buffer ) );

	/*============================================================
	// Graphics Update Ops
	============================================================*/
	void ( * update_vertex_buffer_data )( gs_resource( gs_vertex_buffer ), void*, usize );
	void ( * update_texture_data )( gs_resource( gs_texture ), gs_texture_parameter_desc );

	/*============================================================
	// Graphics Debug Rendering Ops
	============================================================*/
	void ( * set_debug_draw_properties )( gs_resource( gs_command_buffer ), gs_debug_draw_properties );
	void ( * draw_line )( gs_resource( gs_command_buffer ), gs_vec3 start, gs_vec3 end, gs_vec3 color );
	void ( * draw_square )( gs_resource( gs_command_buffer ), gs_vec3 origin, f32 width, f32 height, gs_vec3 color, gs_mat4 transform );
	void ( * submit_debug_drawing )( gs_resource( gs_command_buffer ) );

	// Render Data 
	void* data;

} gs_graphics_i;

/*===============================
// Graphics Default Functionality
===============================*/

gs_texture_parameter_desc gs_texture_parameter_desc_default();
void* gs_load_texture_data_from_file( const char* path, b32 flip_vertically_on_load );

/*===============================
// Graphics User Provided Funcs
===============================*/

extern struct gs_graphics_i* gs_graphics_construct();

/*
	What are the responsibilities here? 
	The graphics subsystem will construct a renderer then be responsible for holding graphics scenes, 
	dispatching calls to the renderer?
	Or should there just be a renderer? To make it simple?
	Want to load resources from file using the graphics subsystem, however that is totally dependent on what 
	graphics API is being used (for instanced, a shader in a OGL vs. DX)

	Want some concept of "render passes" that can be ordered with custom jobs the user or renderer can define
	Does this require a simple job system before this can be implemented? Does a job system require a better memory 
	management system? ( I don't think so for the latter )

	Graphics subsystem holds raw resources, such as : 
		* texture ids
		* vertex buffers
		* index buffers
		* uniform buffers
		* uniform handles
		* shader handles
		* render texture handle
		* frame buffer handle

	Asset subsystem will hold engine level assets that will use raw resources from various other subsystems: 
		* mesh
		* texture 
		* shader

	gs_graphics_i* gfx = gs_engine_subsystem()->ctx.graphics;

	// Will load texture into VRAM, will not store texture data in RAM, 
	//	but will keep texture handle in slot array in graphics subsystem
	gs_resource_handle texture = gfx->load_texture( "path/to/image.extension" );	

	// Load shader
	gs_resource_handle shader = gfx->load_shader( gs_compute, "path/to/shader.extension" );
	gs_resource_handle shader = gfx->load_shader( gs_default, "path/to/vertex_shader.extension", "path/to/fragment_shader.extension" );
	gs_resource_handle shader = gfx->load_shader( gs_geometry, "path/to/vertex_shader.extension", "path/to/fragment_shader.extension", "path/to/geo_shader.extension" );

	// Create render target resource
	gs_resource_handle render_target = gfx->create_render_target( width, height, gs_render_target_flags );

	// For render passes, not sure what this could look like...



	gs_resource_handle 




	From: http://www.gijskaerts.com/wordpress/?p=112
	Handles are essentially strongly typed integers. You can implement these in various ways: You could wrap an integer type into a struct, or define an enum class with a sized integer backing. The important thing here is that they’re strongly typed; you don’t want to be able to assign a handle representing a descriptor table to a handle representing a 2D texture, for example.

	// An example of a strongly typed 32-bit handle using an enum class
	// Note: A typedef or using statement won't work here as this won't provide strong typing

	enum class Tex2DHandle : uint32 { Invalid = 0xFFFFFFFF };
	// The same handle type using the struct approach

	struct Tex2DHandle { uint32 m_value; };

	// Recommending NOT making typdefs for resource handles and instead strongly typing them to ensure correctness

	// Could still use a macro to define these types in a "generic" way

	#define gs_resource( type )\
		gs_resource_#type

	#define gs_declare_resource_type( type )\
		typedef struct gs_resource( type ) {\
	\		u32 id;\	
		} gs_resource( type );\

	gs_declare_resource_type( gs_texture );
	gs_declare_resource_type( gs_shader );
	...

	void main()
	{
		// Get handle to texture resource (strongly typed)
		gs_resource( gs_texture ) texture = gfx->construct_texture( ... );
	}


	// Example of what a render packet could look like. Again, this is up to you!
	struct RenderPacket
	{
	  PipelineState m_pipelineState;
	  VertexBufferView m_vertexBuffers[8];
	  IndexBufferView m_indexBuffer;
	  ShaderResourceHandle m_shaderResources[16];
	  PrimitiveTopology m_topology;
	  ...
	};
	// Example draw operation using a render packet
	void DrawIndexed(CommandBuffer& cbuffer, const RenderPacket& packet);

	
	void Draw(CommandBuffer& cb, const RenderPacket& packet);
	void DrawIndex(CommandBuffer& cb, const RenderPacket& packet);
	void DrawIndirect(CommandBuffer& cb, const RenderPacket& packet, BufferHandle argsBuffer);
	void Dispatch(CommandBuffer& cb, uint32 x, uint32 y, uint32 z);
	void CopyResource(commandBuffer& cb, BufferHandle src, BufferHandle dest);
	#if defined (SOME_PLATFORM)
	void ExoticOperationOnlySupportedOnSomePlatform(CommandBuffer& cb);
	#endif

	//////////////////////////////////////////////////////////////////////////////////////////////
	// Example of a graphics pipeline state with a fragment + vertex shader, and enabled blending
	//////////////////////////////////////////////////////////////////////////////////////////////

	// Vertex program GLSL source
	const char* vertProgSrc = R"(
	    layout (binding = 0, std140) uniform GUIParams
	    {
	        mat4 gWorldTransform;
	        float gInvViewportWidth;
	        float gInvViewportHeight;
	        vec4 gTint;
	    };      

	    layout (location = 0) in vec3 bs_position;
	    layout (location = 1) in vec2 bs_texcoord0;

	    layout (location = 0) out vec2 texcoord0;

	    out gl_PerVertex
	    {
	        vec4 gl_Position;
	    };

	    void main()
	    {
	        vec4 tfrmdPos = gWorldTransform * vec4(bs_position.xy, 0, 1);

	        float tfrmdX = -1.0f + (tfrmdPos.x * gInvViewportWidth);
	        float tfrmdY = 1.0f - (tfrmdPos.y * gInvViewportHeight);    

	        gl_Position = vec4(tfrmdX, tfrmdY, 0, 1);
	        texcoord0 = bs_texcoord0;
	    }
	)";

	// Fragment program GLSL source
	const char* fragProgSrc = R"(
	    layout (binding = 0, std140) uniform GUIParams
	    {
	        mat4 gWorldTransform;
	        float gInvViewportWidth;
	        float gInvViewportHeight;
	        vec4 gTint;
	    };  

	    layout (binding = 1) uniform sampler2D gMainTexture;

	    layout (location = 0) in vec2 texcoord0;
	    layout (location = 0) out vec4 fragColor;

	    void main()
	    {
	        vec4 color = texture2D(gMainTexture, texcoord0.st);
	        fragColor = color * gTint;
	    }
	)";

	// Descriptor structures used for creating the GPU programs
	GPU_PROGRAM_DESC vertProgDesc;
	vertProgDesc.type = GPT_VERTEX_PROGRAM;
	vertProgDesc.entryPoint = "main";
	vertProgDesc.language = "GLSL";
	vertProgDesc.source = vertProgSrc;

	GPU_PROGRAM_DESC fragProgDesc;
	fragProgDesc.type = GPT_FRAGMENT_PROGRAM;
	fragProgDesc.entryPoint = "main";
	fragProgDesc.language = "GLSL";
	fragProgDesc.source = fragProgSrc;

	// Descriptor structures used for setting blend and depth-stencil states
	BLEND_STATE_DESC blendDesc;
	blendDesc.renderTargetDesc[0].blendEnable = true;
	blendDesc.renderTargetDesc[0].renderTargetWriteMask = 0b0111; // RGB, don't write to alpha
	blendDesc.renderTargetDesc[0].blendOp = BO_ADD;
	blendDesc.renderTargetDesc[0].srcBlend = BF_SOURCE_ALPHA;
	blendDesc.renderTargetDesc[0].dstBlend = BF_INV_SOURCE_ALPHA;

	DEPTH_STENCIL_STATE_DESC depthStencilDesc;
	depthStencilDesc.depthWriteEnable = false;
	depthStencilDesc.depthReadEnable = false;

	// Create pipeline state descriptor
	PIPELINE_STATE_DESC pipelineDesc;
	pipelineDesc.blendState = BlendState::create(blendDesc);
	pipelineDesc.depthStencilState = DepthStencilState::create(depthStencilDesc);
	pipelineDesc.vertexProgram = GpuProgram::create(vertProgDesc);
	pipelineDesc.fragmentProgram = GpuProgram::create(fragProgDesc);

	// And finally, create the pipeline
	SPtr<GraphicsPipelineState> pipelineState = GraphicsPipelineState::create(pipelineDesc);

	//////////////////////////////////////////////////////////////////////////////////////////////
	// Creating and setting GPU parameters for the pipeline we created above
	////////////////////////////////////////////////////////////////////////////////////////////// 

	// Create a container object to hold the parameters for all GPU programs in the pipeline state
	SPtr<GpuParams> gpuParams = GpuParams::create(pipelineState);

	// Create a structure that will hold our uniform block variables
	struct UniformBlock
	{
	    Matrix4 gWorldTransform;
	    float gInvViewportWidth;
	    float gInvViewportHeight;
	    Color gTint;
	};

	// Fill out the uniform block variables
	UniformBlock uniformBlock;
	uniformBlock.gWorldTransform = Matrix4::IDENTITY;
	uniformBlock.gInvViewportWidth = 1.0f / 1920.0f;
	uniformBlock.gInvViewportHeight = 1.0f / 1080.0f;
	uniformBlock.gTint = Color::White;

	// Create a uniform block buffer for holding the uniform variables
	SPtr<GpuParamBlockBuffer> uniformBuffer = GpuParamBlockBuffer::create(sizeof(UniformBlock));
	uniformBuffer->write(0, &uniformBlock, sizeof(uniformBuffer));

	// Assign the uniform buffer to set 0, binding 0
	gpuParams->setParamBlockBuffer(0, 0, uniformBuffer);

	// Import a texture to assign to the gMainTexture parameter
	HTexture texture = gImporter().import<Texture>("myTexture.png");

	// Assign the texture to set 0, binding 1
	gpuParams->setTexture(0, 1, texture);

	// Bind the GPU parameters for use
	RenderAPI::setGpuParams(gpuParams);

	//////////////////////////////////////////////////////////////////////////////////////////////
	// Example for executing 8000 draw calls distributed over eight threads
	////////////////////////////////////////////////////////////////////////////////////////////// 

	// Retrieve the core thread's render API interface
	RenderAPI& rapi = RenderAPI::instance();

	// Create eight command buffers we'll use for parallel command submission
	SPtr<CommandBuffer> commandBuffers[8];
	for (UINT32 i = 0; i < 8; i++)
	    commandBuffers[i] = CommandBuffer::create(GQT_GRAPHICS); // Command buffers running on the graphics queue

	// Worker that queues 1000 different draw calls on a command buffer with the provided index
	// (For simplicity, assuming you have created relevant pipeline states, GPU parameters, index/vertex buffers
	// and vertex declarations earlier)
	auto renderWorker = [&](UINT32 idx)
	{
	    SPtr<CommandBuffer> cb = commandBuffers[idx];
	    for(UINT32 i = 0; i < 1000; i++)
	    {
	        UINT32 entryIdx = idx * 1000 + i;

	        rapi.setGraphicsPipeline(pipelineStates[entryIdx], cb);
	        rapi.setGpuParams(gpuParams[entryIdx], cb);
	        rapi.setVertexBuffers(0, &vertexBuffers[entryIdx], 1, cb);
	        rapi.setIndexBuffer(indexBuffers[entryIdx], cb);
	        rapi.setVertexDeclaration(vertexDeclarations[entryIdx], cb);
	        rapi.setDrawOperation(DOT_TRIANGLE_LIST, cb);
	        rapi.drawIndexed(0, numIndices[entryIdx], 0, numVertices[entryIdx], 0, cb);
	    }
	};

	// Run all of our worker threads
	SPtr<Task> tasks[8];
	for(UINT32 i = 0; i < 8; i++)
	{
	    tasks[i] = Task::create("Render", std::bind(&renderWorker, i));
	    TaskScheduler::instance().addTask(tasks[i]);
	}

	// Block this thread until all workers are done populating the command buffers
	for (UINT32 i = 0; i < 8; i++)
	    tasks[i]->wait();

	// Submit all the command buffers for execution
	for (UINT32 i = 0; i < 8; i++)
	    rapi.submitCommandBuffer(commandBuffers[i]);


	Command queue -> Command Buffers -> Commands	

	Have command buffers for graphics tasks, compute tasks, transfer tasks

	typedef gs_graphics_i 
	{
		gs_resource_handle( gs_texture ) ( * load_texture_from_file )( const char* path ); 	
	} gs_graphics_i;

	Graphics resources are entirely render backend dependent

	What would a simple forward shader look like? 

	// Load shader source from file
	const char* vs = platform->load_from_file( "shader.vs" );
	const char* fs = platform->load_from_file( "shader.fs" );

	gs_resource( gs_shader ) shader = gfx->load_shader( gs_render, vs, fs );

	// Simple triangle
	f32 vert_data[] = 
	{
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 
		1.0f, 1.0f, 0.0f
	};

	u32 index_data[] = 
	{
		0, 1, 2, 2, 3, 0
	};

	typedef enum gs_uniform_type
	{
		gs_uniform_type_float;
		gs_uniform_type_int;
		gs_uniform_type_vec2;
		gs_uniform_type_vec3;
		gs_uniform_type_vec4;
		gs_uniform_type_mat4;
		gs_uniform_type_texture_2d;	
		gs_uniform_type_texture_3d;
	} gs_uniform_type;

	typedef struct gs_uniform_desc 
	{
		gs_uniform_type type;
		gs_resource( gs_uniform ) uniform_handle; 
	} gs_uniform_desc;

	typedef struct gs_uniform_buffer 
	{
		gs_dyn_array( gs_uniform_desc ) uniforms;
 	} gs_uniform_buffer;

	typedef struct gs_pipeline_state_desc
	{
		gs_resource( gs_pipeline_state ) 	shader;
		gs_resource( gs_vertex_buffer ) 	vertex_buffer;
		gs_resource( gs_index_buffer ) 		index_buffer;
		gs_resource( gs_uniform_buffer ) 	uniform_buffer;	
	} gs_pipeline_state_desc;

	gs_resource( gs_vertex_buffer ) vertex_buffer = gfx->create_vertex_buffer( &vert_data, 4 );
	gs_resource( gs_index_buffer ) index_buffer = gfx->create_index_buffer( &index_data, 6 );

	// Not sure how to create a uniform buffer object though...or how I would like to create individual uniforms?
	gs_resource( gs_uniform ) projMatrixHandle = gfx->create_uniform_handle( shader, "projMatrix" );
	gs_resource( gs_uniform ) viewMatrixHandle = gfx->create_uniform_handle( shader, "viewMatrix" );
	gs_resource( gs_uniform ) modelMatrixHandle = gfx->create_uniform_handle( shader, "modelMatrix" );

	gs_graphics_params_desc g_desc = {0}; 
	g_desc.blend_state 				= gs_blend_state_default;
	g_desc.frame_buffer 			= gs_back_buffer;
	g_desc.stencil_state 			= gs_stencil_default;

	// Create a pipeline state object ( as a resource? )
	gs_pipeline_state_desc p_desc 	= {0};
	p_desc.shader 					= shader;
	p_desc.vertex_buffer 			= vertex_buffer;
	p_desc.index_buffer 			= index_buffer;

	// Create pipeline state object passing in descriptor and get resource handle back
	gs_resource( gs_pipeline_state ) ps = gfx->create_pipeline_state( p_desc );

	// Another possible api for setting parameters for the state object
	gs_resource( gs_pipeline_state ) pso = gfx->create_pipeline_state(); 
	gfx->set_pipeline_state_shader( pso, shader );
	gfx->set_pipeline_state_vertex_buffer( pso, vertex_buffer );
	gfx->set_pipeline_state_index_buffer( pso, index_buffer );
	
	// Construct command buffer
	gs_render_command_buffer cb = gfx->create_command_buffer( gs_render );		// The memory for these command buffers will be recycled internally

	// Set pipeline state
	gfx->set_pipeline_state( &cb, pso );

	// Set global graphics params for pass ( whatever these are, usually blend states )
	gfx->set_gfx_params( &cb, gparams );

	// Bind uniforms? These should be held within the gpu params? Or should I allow individual uniform binding such as this?
	// Not sure about these...since they don't really make sense in explicit graphcis apis
	gfx->bind_uniform( &cb, proj_mat_handle, gs_uniform_type_mat4, &proj_mat )
	gfx->bind_uniform( &cb, view_mat_handle, gs_uniform_type_mat4, &view_mat );
	gfx->bind_uniform( &cb, model_mat_handle, gs_uniform_type_mat4, &model_mat );
	gfx->bind_uniform( &cb, texture_handle, gs_uniform_type_sampler, NULL );

	// Draw
    gfx->draw_indexed( &cb, 0, num_indices, 0, num_verts );

	// Submit buffer	
	gfx->submit_command_buffer( &cb );

	// Free buffer
	gfx->free_command_buffer( &cb );

*/

	// GPU_PROGRAM_DESC vertProgDesc;
	// vertProgDesc.type = GPT_VERTEX_PROGRAM;
	// vertProgDesc.entryPoint = "main";
	// vertProgDesc.language = "GLSL";
	// vertProgDesc.source = vertProgSrc;

	// // Descriptor structures used for setting blend and depth-stencil states
	// BLEND_STATE_DESC blendDesc;
	// blendDesc.renderTargetDesc[0].blendEnable = true;
	// blendDesc.renderTargetDesc[0].renderTargetWriteMask = 0b0111; // RGB, don't write to alpha
	// blendDesc.renderTargetDesc[0].blendOp = BO_ADD;
	// blendDesc.renderTargetDesc[0].srcBlend = BF_SOURCE_ALPHA;
	// blendDesc.renderTargetDesc[0].dstBlend = BF_INV_SOURCE_ALPHA;

	// DEPTH_STENCIL_STATE_DESC depthStencilDesc;
	// depthStencilDesc.depthWriteEnable = false;
	// depthStencilDesc.depthReadEnable = false;

// typedef struct gs_render_pipeline_state_desc
// {
// 	gs_blend_state 			blend_state;
// 	gs_depth_stencil_state 	depth_stencil_state;
// 	gs_shader_program 		vertex_program;
// 	gs_shader_program 		fragment_program;
// } gs_pipeline_state_desc;

// Something simple to get something on the fucking screen! Then I can continue to shape it. 

	// // Create pipeline state descriptor
	// PIPELINE_STATE_DESC pipelineDesc;
	// pipelineDesc.blendState = BlendState::create(blendDesc);
	// pipelineDesc.depthStencilState = DepthStencilState::create(depthStencilDesc);
	// pipelineDesc.vertexProgram = GpuProgram::create(vertProgDesc);
	// pipelineDesc.fragmentProgram = GpuProgram::create(fragProgDesc);

/*
	typedef struct gs_renderer_i 
	{
		void ( * render )();
	} gs_renderer_i;

	// Some implementation of a renderer? 
	struct gs_deferred_renderer
	{
		_base( gs_renderer_i );	
	} gs_deferred_renderer;

	// Record INTO the pass, right? Not sure how to architect this worth a damn...
	void GBufferPass( gs_command_buffer* cb )
	{
		// Want some way to order passes as well eventually, if not manually? 

		// Set a pipeline state for this pass? Where is that stored?

		// Grab all renderables to be drawn from the scene
	}

	Alright, want some method for allowing the user to create as many render passes as they want, control the order, control dependencies, 
	then set this thing up to go as wide as possible

	Some simple update loop for the renderer

	# Can't do deferred render pass if on mobile due to lack of MRT
	What do you need to do a gbuffer render pass?
		- Frame buffer with MRT ( this is already making assumptions about the rendering capabilities, btw )
		- blend settings for state
		- depth settings for state

		For each renderable, bind resource descriptors, like program, uniform, etc.
		Sort renderable by hashed key

	// These pipeline states are for renderables...so does that mean when you attach a new material or mesh to an object, you have to 
	// recompile its pipeline state?
	gs_resource( gs_pipeline_state )

	void renderer_init()
	{
		// Initialize gpu params, pipeline states, etc.

		// Create pipeline state object passing in descriptor and get resource handle back
		gs_resource( gs_pipeline_state ) ps = gfx->create_pipeline_state( p_desc );

		// I suppose each material could have its own pipeline description... 
		gs_pipeline_state_desc gbuffer_pipeline_desc;

		// PIPELINE_STATE_DESC pipelineDesc;
		// pipelineDesc.blendState = BlendState::create(blendDesc);
		// pipelineDesc.depthStencilState = DepthStencilState::create(depthStencilDesc);
		// pipelineDesc.vertexProgram = GpuProgram::create(vertProgDesc);
		// pipelineDesc.fragmentProgram = GpuProgram::create(fragProgDesc);

	}

	void renderer_update()
	{
		gs_command_buffer cb_gbuffer = gfx->construct_command_buffer();
		gs_command_buffer cb_lighting = gfx->construct_command_buffer();

		gbuffer_pass();
		lighting_pass();		// Need information from the gbuffer for this pass
	}

	// Just some example of a USER DEFINED specific uniform block to be used for this particular rendering pass
	typedef struct gs_uniform_block	
	{
		gs_mat4 proj_mat;
		gs_mat4 view_mat;
	} gs_uniform_block;

	gs_resource( gs_gpu_params ) gpu_params;
	gs_resource( gs_render_target ) render_target;

	void gbuffer_pass( gs_command_buffer* cb )
	{
		// Fill out the uniform block variables
		gs_uniform_block ub;
		ub.proj_mat = cam->get_projection_matrix();
		ub.view_mat = cam->get_view_matrix();

		// Bind render surface & clear it
		gfx->setRenderTarget( render_target, false, RT_NONE, cb );
		gfx->clearRenderTarget(FBT_COLOR | FBT_DEPTH, Color::Blue, 1, 0, 0xFF, cb );

		// Create a uniform block buffer for holding the uniform variables
		gs_resource( gs_uniform_buffer ) uniform_buffer = gfx->create_param_block_buffer(sizeof(gs_uniform_block));
		gfx->write_param_block(0, &ub, sizeof(gs_uniform_block));

		// Grab renderables from scene ( not sure who manages this )
		renderables = gfx->get_static_mesh_renderables_from_somewhere();

		// Assign the uniform buffer & texture
		gfx->set_gpu_params_block_buffer( gpu_params, gs_fragment, "params", ub );
		gfx->set_gpu_params_block_buffer( gpu_params, gs_vertex, "params", ub );

		// psuedo code
		for each ( renderable in renderables )
		{
			// Bind program for renderable if different from previous
			// Bind required textures for renderable


			// pipeline state object
		}	

		// Another possible api for setting parameters for the state object
		gs_resource( gs_pipeline_state ) pso = gfx->create_pipeline_state(); 
		gfx->set_pipeline_state_shader( pso, shader );
		gfx->set_pipeline_state_vertex_buffer( pso, vertex_buffer );
		gfx->set_pipeline_state_index_buffer( pso, index_buffer );
		
		// Construct command buffer
		gs_render_command_buffer cb = gfx->create_command_buffer( gs_render );		// The memory for these command buffers will be recycled internally

		// Set pipeline state
		gfx->set_pipeline_state( &cb, pso );

		// Set global graphics params for pass ( whatever these are, usually blend states )
		gfx->set_gfx_params( &cb, gparams );

		// Bind uniforms? These should be held within the PSO
		// Not sure about these...since they don't really make sense in explicit graphcis apis
		gfx->bind_uniform( &cb, proj_mat_handle, gs_uniform_type_mat4, &proj_mat )
		gfx->bind_uniform( &cb, view_mat_handle, gs_uniform_type_mat4, &view_mat );
		gfx->bind_uniform( &cb, model_mat_handle, gs_uniform_type_mat4, &model_mat );
		gfx->bind_uniform( &cb, texture_handle, gs_uniform_type_sampler, NULL );

		// Draw
	    gfx->draw_indexed( &cb, 0, num_indices, 0, num_verts );

		// Submit buffer	
		gfx->submit_command_buffer( &cb );
	}

	// Want to give a vertex buffer layout description (to set attributes)
	typedef enum gs_vertex_attribute_type
	{
		gs_attribute_float4,
		gs_attribute_float3,
		gs_attribute_float2,
		gs_attribute_float,
		gs_attribute_uint4,
		gs_attribute_uint3,
		gs_attribute_uint2,
		gs_attribute_uint
	} gs_vertex_attribute_format_type;

	typedef struct gs_vertex_attribute_layout_desc
	{
		usize byte_size;
		gs_dyn_array( gs_vertex_attribute_type ) decl;	
	} gs_vertex_attribute_layout_desc;

	1---3
	|\	|
	| \	|
	|  \|
	0---2

	f32 vert_data[] = 
	{
		// Positions
		0.0f, 0.0f, 0.0f, 
		0.0f, 1.0f, 0.0f, 
		1.0f, 0.0f, 0.0f, 
		1.0f, 1.0f, 0.0f
	};

	f32 index_data[] = 
	{
		0, 2, 1, 1, 2, 3
	};

	const char* vs_source = "\
	    layout (location = 0) in vec3 gs_position;\
	    layout (location = 1) in vec2 gs_texcoord0;\
	    layout (location = 0) out vec2 texcoord0;\

	    layout (std140) uniform ub_matrices\
	    {\
	    	mat4 g_proj;
	        mat4 g_view;\
	    };\     
\
	    void main()\
	    {\
	        vec4 world_pos = g_proj * g_view * vec4(gs_position.xy, 0, 1);\
	        gl_Position = vec4(world_pos.x, world_pos.y, 0, 1);\
	        texcoord0 = gs_texcoord0;\
	    }";

	const char* gs_source = "\
	    layout (std140) uniform ub_matrices\
	    {\
	    	mat4 g_proj;
	        mat4 g_view;\
	    };
\
	    layout (location = 0) in vec2 texcoord0;\
	    layout (location = 0) out vec4 fragColor;\
\
	    uniform float4 u_color;\
\
	    void main()\
	    {\
	        fragColor = u_color;\
	    }\
	)";

	// Use this as our data block to upload every frame
	typedef struct uniform_block
	{
		gs_mat4 proj;	
		gs_mat4 view;
	} uniform_block;

	gs_resource( gs_vertex_buffer ) 		g_vertex_buffer = {0};
	gs_resource( gs_shader ) 				g_shader = {0};
	gs_resource( gs_render_command_buffer ) g_cb = {0};
	gs_resource( gs_uniform_buffer ) 		g_ub = {0};
	gs_resource( gs_uniform )				g_uniform = {0};

	// Should this be set as a callback to the renderer?
	// Should user be in charge of update loop completely?
	void render()
	{
		// Graphics graphics API
		gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

		// Reset render command buffer
		gfx->reset_command_buffer( g_cb );
		
		// Bind shader
		gfx->set_shader( g_cb, g_shader );

		// Set data for uniform block
		uniform_block ub = (uniform_block){ cam->projection_matrix(), cam->view_matrix() };

		// Upload data into uniform buffer
		gfx->set_uniform_buffer_sub_data( g_cb, g_ub, &ub, sizeof(uniform_block) );

		// Bind constant uniform
		f32 red[4] = { 1.0f, 0.f, 0.f, 1.f };
		gfx->set_uniform( g_cb, g_uniform, &red );

		// Bind vertex buffer
		gfx->set_vertex_buffer( g_cb, g_vertex_buffer );

		// Draw
		gfx->draw( g_cb, 0, 3 );

		// Submit/Flush command buffer
		gfx->submit_command_buffer( g_cb );
	}

	void app_init()
	{
		init_graphics_resources();
	}

	void init_graphics_resources()
	{
		gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

		// Construct layout description format for our mesh data
		gs_vertex_buffer_layout_desc v_desc;
		gfx->push_vertex_attribute( &v_desc, gs_attribute_float3 );

		// Construct vertex buffer
		g_vertex_buffer = gfx->construct_buffer( gs_vertex_buffer, v_desc, vert_data, sizeof(vert_data) );

		// This is a little ehh...We'll just do it for now ( eventually, this will go into a pipeline state object to emulate explicit api ) 
		g_shader = gfx->construct_shader( gs_render, vs_source, fs_source );

		// Construct command buffer to use for rendering
		g_cb = gfx->construct_command_buffer( gs_render_command_buffer );

		// Construct uniform buffer binding handle
		g_ub = gfx->construct_uniform_buffer( g_shader, "ub_matrices" );

		// Construct uniform handle for individual/constant/push uniform
		g_uniform = gfx->construct_uniform( g_shader, "u_color" );
	}

	// Example program showing rendering a simple triangle to screen
	void main()
	{
		// Make engine and init...
	}


	// Rendering fonts

	Fonts are NOT a graphics level resource - the texture for the atlas, however, is. Fonts are an asset level resource, 
	which might specifically be a game/application layer concept.

	typedef struct gs_font
	{
		// Glyph data for font
		...

		// Handle to internal texture atlas resource
		gs_resource( gs_texture ) atlas;
	} gs_font;


	// Debug drawing ( for lines / shapes / primitives / etc. )

	// Should add this into a vertex buffer. Use some internal rendering mechanism for displaying all of this data.
	// It'll get drawn on top of all other passes? Or the user can determine when to submit? I suppose it doesn't really matter...
	// Or, could just draw this as a single draw call? I don't like that option.
	gfx->draw_line( start, end, color );

	...

	// Could this be what I want?
	gfx->submit_debug_drawing( cb_handle );

	// Want to throw all debug drawing ops into a vertex buffer for a single draw call
	// Want the ability to do 3d rendering as well as 2d rendering for it
	// Want the user to have ability to render these shapes into whatever they want, and then set whatever 
	// parameters they want for debug rendering, such as view, projection

	// Internally
	struct debug_drawing_internal_data {
		gs_mat4 view_matrix;
		gs_mat4 proj_matrix;
		gs_resource( gs_vertex_buffer ) vbo;
		gs_dyn_array( f32 ) vertex_data; 
	};

	struct debug_properties props = {
		view_matrix, 
		proj_matrix
	};

	// I mean, what's the point of this, though? Provide simple utilities for adding vertex data into a buffer 
	// for primitive drawing. Abstracting the bore of having to code all of that yourself. A particular shader 
	// is needed for drawing, the vertex layout needs to be understood correctly.

	gfx->set_debug_properties( cb_handle, props );
	gfx->draw_line( cb_handle, start, end, color );
	gfx->draw_cube( cb_handle, aabb, model_matrix, color );
	gfx->draw_square( cb_handle, origin, extents, color );
	gfx->submit_debug_rendering( cb_handle );

	// Graphics update
	Debug Rendering...

	// Alright, so forget 3d rendering...
	// Let's do simple 2d rendering solely...how do? I can't access a global buffer if I want this to scale...
	// Wait, yes I can...I'm being dumb. In drawing lines, it'll add in the submit phase. This'll work.
	// But...if I want these things to be quads, that's where the lines will be weird. Unless I keep this entire 
	// implementation 2d. Or have two different vertex buffers - one for lines, one for quads. Then all
	// debug drawing will come down to two separate calls. I think I'm okay with that...maybe?
	// When would the actual drawing take place? In a command buffer submission? Or at end of frame? What if user 
	// wants to render debug data into a particular render target? The user can't control the way the debug drawing looks, 
	// but he can control where it is drawn into.

	void graphics_update()
	{
		// Submit all command buffers...

		// Do debug drawing
	}

	struct gs_render_pass
	{
		gs_resource( gs_command_buffer ) command_buffer;
	}

	What would a deferred rendering pass look like? Would be implicit, I believe...
*/


#ifdef __cplusplus
}
#endif 	// c++


#endif // __GS_GRAPHICS_SUBSYSTEM_H__


