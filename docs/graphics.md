#  Graphics
This document shows off the graphics API provided by gunslinger.

## Functions
### Resource Creation:

The following functions are used for creating resources, such as vertex buffers, textures, uniforms, etc.

* [gs_graphics_texture_create](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_texture_create)
* [gs_graphics_uniform_create](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_uniform_create) 
* [gs_graphics_shader_create](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_shader_create) 
* [gs_graphics_vertex_buffer_create](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_vertex_buffer_create) 
* [gs_graphics_index_buffer_create](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_index_buffer_create)
* [gs_graphics_uniform_buffer_create](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_index_buffer_create) 
* [gs_graphics_storage_buffer_create](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_storage_buffer_create) 
* [gs_graphics_framebuffer_create](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_framebuffer_create) 
* [gs_graphics_render_pass_create](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_render_pass_create) 
* [gs_graphics_pipeline_create](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_pipeline_create) 

<br />

#### gs_graphics_texture_create

Creates a texture resource.

```c
gs_handle(gs_graphics_texture_t)
gs_graphics_texture_create(
    const gs_graphics_texture_desc_t* desc
);
```

**Parameters:**

`desc`

A pointer to a `gs_graphics_texture_desc_t` structure that describes the texture to be created.

**Return Value:**

Returns a handle to a texture resource.

<br />

#### gs_graphics_uniform_create

Creates a uniform resource.

```c
gs_handle(gs_graphics_uniform_t)
gs_graphics_uniform_create(
    const gs_graphics_uniform_desc_t* desc
);
```

**Parameters:**

`desc`

A pointer to a `gs_graphics_uniform_desc_t` structure that describes the uniform to be created.

**Return Value:**

Returns a handle to a uniform resource.  

<br />

#### gs_graphics_shader_create

Creates a shader resource.

```c
gs_handle(gs_graphics_shader_t)
gs_graphics_shader_create(
    const gs_graphics_shader_desc_t* desc
);
```

**Parameters:**

`desc`

A pointer to a `gs_graphics_shader_desc_t` structure that describes the shader to be created.

**Return Value:**

Returns a handle to a shader resource.

<br /> 

#### gs_graphics_vertex_buffer_create

Creates a vertex buffer resource.

```c
gs_handle(gs_graphics_vertex_buffer_t)
gs_graphics_vertex_buffer_create(
    const gs_graphics_vertex_buffer_desc_t* desc
);
```

**Parameters:**

`desc`

A pointer to a `gs_graphics_vertex_buffer_desc_t` structure that describes the vertex buffer to be created.

**Return Value:**

Returns a handle to a vertex buffer resource.  

<br /> 

#### gs_graphics_index_buffer_create

Creates an index buffer resource.

```c
gs_handle(gs_graphics_index_buffer_t)
gs_graphics_index_buffer_create(
    const gs_graphics_index_buffer_desc_t* desc
);
```

**Parameters:**

`desc`

A pointer to a `gs_graphics_index_buffer_desc_t` structure that describes the index buffer to be created.

**Return Value:**

Returns a handle to an index buffer resource.

<br /> 

#### gs_graphics_uniform_buffer_create

Creates a uniform buffer resource.

```c
gs_handle(gs_graphics_uniform_buffer_t)
gs_graphics_uniform_buffer_create(
    const gs_graphics_uniform_buffer_desc_t* desc
);
```

**Parameters:**

`desc`

A pointer to a `gs_graphics_uniform_buffer_desc_t` structure that describes the uniform buffer to be created.

**Return Value:**

Returns a handle to a uniform buffer resource.

<br /> 

#### gs_graphics_framebuffer_create

Creates a framebuffer resource.

```c
gs_handle(gs_graphics_framebuffer_t)
gs_graphics_framebuffer_create(
    const gs_graphics_framebuffer_desc_t* desc
);
```

**Parameters:**

`desc`

A pointer to a `gs_graphics_framebuffer_desc_t` structure that describes the framebuffer to be created.  

**Return Value:**

Returns a handle to a framebuffer resource.  

<br /> 

#### gs_graphics_render_pass_create

Creates a render-pass resource.

```c
gs_handle(gs_graphics_render_pass_t)
gs_graphics_render_pass_create(
    const gs_graphics_render_pass_desc_t* desc
);
```

**Parameters:**

`desc`

A pointer to a `gs_graphics_render_pass_desc_t` structure that describes the render-pass to be created.

**Return Value:**

Returns a handle to a render-pass resource.

<br />

#### gs_graphics_pipeline_create

Creates a pipeline resource.

```c
gs_handle(gs_graphics_pipeline_t)
gs_graphics_pipeline_create(
    const gs_graphics_pipeline_desc_t* desc
);
```

**Parameters:**

`desc`

A pointer to a `gs_graphics_pipeline_desc_t` structure that describes the pipeline to be created

**Return Value:**

Returns a handle to a pipeline resource.

<br />

### Resource Destruction:

The following functions are used for freeing resources that were created with the above functions.

* [gs_graphics_vertex_buffer_destroy](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_vertex_buffer_destroy)
* [gs_graphics_index_buffer_destroy](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_index_buffer_destroy)
* [gs_graphics_texture_destroy](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_texture_destroy)
* [gs_graphics_uniform_destroy](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_uniform_destroy)
* [gs_graphics_uniform_buffer_destroy](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_uniform_buffer_destroy)
* [gs_graphics_storage_buffer_destroy](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_storage_buffer_destroy)
* [gs_graphics_shader_destroy](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_shader_destroy)
* [gs_graphics_render_pass_destroy](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_render_pass_destroy)
* [gs_graphics_pipeline_destroy](https://github.com/MrFrenik/gunslinger/blob/master/docs/graphics.md#gs_graphics_pipeline_destroy)

<br />

#### gs_graphics_texture_destroy

Destroys a texture resource.

```c
void gs_graphics_texture_destroy(
    gs_handle(gs_graphics_texture_t) hndl
);
```
**Parameters:**

`hndl`

A handle to a texture resource.

**Return Value:**

None

<br />

#### gs_graphics_shader_destroy

Destroys a shader resource.

```c 
void gs_graphics_shader_destroy(
    gs_handle(gs_graphics_shader_t) hndl
);
```
**Parameters:**

`hndl`

A handle to a shader resource.

**Return Value:**

None

<br />

#### gs_graphics_render_pass_destroy

Destroys a render-pass resource.

```c
void gs_graphics_render_pass_destroy(
    gs_handle(gs_graphics_render_pass_t) hndl
);
```

**Parameters:**

`hndl`

A handle to a render-pass resource.

**Return Value:**

None

<br />

#### gs_graphics_pipeline_destroy

Destroys a pipeline resource.

```c
void gs_graphics_pipeline_destroy(
    gs_handle(gs_graphics_pipeline_t) hndl
);
```

**Parameters:**

`hndl`

A handle to a pipeline resource.

**Return Value:**

None

<br />

## Structures

#### gs_graphics_shader_source_desc_t

Describes the source data for a shader resource.

```c
typedef struct gs_graphics_shader_source_desc_t
{
    gs_graphics_shader_stage_type       type;
    const char*                         source;
} gs_graphics_shader_source_desc_t;
```

**Members**

`type`

Indicates what stage the shader code is for (ie, vertex, fragment, etc.)

`source`

Source code for GLSL shader, as an ASCII string.

<br />

#### gs_graphics_shader_desc_t

Describes a shader resource.

```c
typedef struct gs_graphics_shader_desc_t
{
    gs_graphics_shader_source_desc_t*       sources;
    size_t                                  size;
    char                                    name[64];
} gs_graphics_shader_desc_t;
```

**Members**

`sources`

Array of `gs_graphics_shader_source_desc_t` containing shader sources.

`size`

Size in bytes of the shader source desc array.

`name`

Optional name for the shader - used internally for logging/debugging.

<br />

#### gs_graphics_texture_desc_t

Describes a texture resource.

```c
typedef struct gs_graphics_texture_desc_t
{
    uint32_t                                width;
    uint32_t                                height;
    gs_graphics_texture_format_type         format;
    gs_graphics_texture_wrapping_type       wrap_s;
    gs_graphics_texture_wrapping_type       wrap_t;
    gs_graphics_texture_filtering_type      min_filter;
    gs_graphics_texture_filtering_type      mag_filter;
    gs_graphics_texture_filtering_type      mip_filter;
    void*                                   data;
    b32                                     render_target;
} gs_graphics_texture_desc_t;
```

**Members**

`width`

Width of texture in pixels.

`height`

Height of texture in pixels.

`format`

Format of texture data (rgba32, rgba8, rgba32f, r8, depth32f, etc...).

`wrap_s`

Wrapping type for s (or u) axis of texture.

`wrap_t`

Wrapping type for t (or v) axis of texture.

`min_filter`

Minification filter for texture.

`mag_filter`

Magnification filter for texture.

`mip_filter`

Mip filter for texture.

`num_mips`

Number of mips to generate (default 0 is disable mip generation).

`data`

Texture data to upload (can be NULL).

`render_target`

Default to false (not a render target).

<br />

#### gs_graphics_uniform_layout_desc_t

DESC

```c
```

**Members**

<br />

#### gs_graphics_uniform_desc_t

DESC

```c
```

**Members**

<br />

#### gs_graphics_buffer_update_desc_t

DESC

```c
```

**Members**

<br />

#### gs_graphics_buffer_base_desc_t

DESC

```c
```

**Members**

<br />

#### gs_graphics_vertex_buffer_desc_t

Describes a vertex buffer resource.

```c
typedef struct gs_graphics_vertex_buffer_desc_t
{
    void*                                   data;
    size_t                                  size;
    gs_graphics_buffer_usage_type           usage;
    gs_graphics_buffer_update_desc_t        update;
} gs_graphics_vertex_buffer_desc_t;
```

**Members**

`data`

Data to fill the vertex buffer with. Optional; can be NULL.

`size`

Size in bytes of the vertex buffer to be created.

`usage`

Specifies how the buffer's contents may be updated over time.
See associated enum.

`update`

Specifies how the buffer's are to be updated with `gs_graphics_vertex_buffer_update()`.
See associated enum.

<br />

#### gs_graphics_index_buffer_desc_t

Describes an index buffer resource.

```c
typedef gs_graphics_vertex_buffer_desc_t gs_graphics_index_buffer_desc_t;
```

**Members**

See definition of `gs_graphics_vertex_buffer_desc_t`.
NOTE: `update` specifies use for `gs_graphics_index_buffer_update()`.

<br />

#### gs_graphics_uniform_buffer_desc_t

Describes a uniform buffer resource.

```c
typedef struct gs_graphics_uniform_buffer_desc_t
{
    void*                                   data;
    size_t                                  size;
    gs_graphics_buffer_usage_type           usage;
    const char*                             name;
    gs_graphics_shader_stage_type           stage;
    gs_graphics_buffer_update_desc_t        update;
} gs_graphics_uniform_buffer_desc_t;
```

**Members**

`data`

Data to fill the vertex buffer with. Optional; can be NULL.

`size`

Size in bytes of the vertex buffer to be created.

`usage`

Specifies how the buffer's contents may be updated over time.
See associated enum.

`name`

Name for the buffer; optional, but required for OpenGL/ES & WebGL.

`stage`

Pipeline stage the buffer is to be bound to (ie, vertex, fragment, etc.)

`update`

Specifies how the buffer is to be updated.

<br />

#### gs_graphics_framebuffer_desc_t

Describes a framebuffer resource.

```c
typedef struct gs_graphics_framebuffer_desc_t
{
    void*       data;
} gs_graphics_framebuffer_desc_t;
```

**Members**

`data`

Data to fill the framebuffer with; optional, can be NULL.

<br />

#### gs_graphics_clear_action_t

Describes the action for clearing the current render target's buffers.

```c
typedef struct gs_graphics_clear_action_t
{
    gs_graphics_clear_flag      flag;   // Flag to be set (clear color, clear depth, clear stencil, clear all)
    union
    {
        float       color[4];            // Clear color value
        float       depth;               // Clear depth value
        int32_t     stencil;           // Clear stencil value
    };
 } gs_graphics_clear_action_t;
```

**Members**

`flag`

Specifies which buffers are to be cleared (color, depth, stencil). Can be OR'd (||) together.

`color`

Value to clear the color buffer with.

`depth`

Value to clear the depth buffer with.

`stencil`

Value to clear the stencil buffer with.

<br />

#### gs_graphics_clear_desc_t

Describes a set of clear actions to be performed.

```c
typedef struct gs_graphics_clear_desc_t
{
    gs_graphics_clear_action_t*     actions;
    size_t                          size;
} gs_graphics_clear_desc_t;
```

**Members**

`actions`

Array of clear actions.

`size`

Size of clear action array in bytes.

<br />

#### gs_graphics_render_pass_desc_t

Describes a render-pass resource.

```c
typedef struct gs_graphics_render_pass_desc_t
{
    gs_handle(gs_graphics_framebuffer_t)        fbo;
    gs_handle(gs_graphics_texture_t)*           color;
    size_t                                      color_size;
    gs_handle(gs_graphics_texture_t)            depth;
    gs_handle(gs_graphics_texture_t)            stencil;
} gs_graphics_render_pass_desc_t;
```

**Members**

`fbo`

Framebuffer to bind for render-pass.

`color`

Array of color attachments to be bound (useful for MRT, if supported).

`color_size`

Size of color attachment array.

`depth`

Depth attachment to be bound.

`stencil`

Stencil attachment to be bound.

<br />

#### gs_graphics_bind_vertex_buffer_desc_t

Describes how a vertex buffer is to be bound to the pipeline.

```c
typedef struct gs_graphics_bind_vertex_buffer_desc_t
{
    gs_handle(gs_graphics_vertex_buffer_t)      buffer;
    size_t                                      offset;
    gs_graphics_vertex_data_type                data_type;
} gs_graphics_bind_vertex_buffer_desc_t;
```

**Members**

`buffer`

Handle to the vertex buffer to be bound.

`offset`

Offset into the buffer to bind from.

`data_type`

Specifies if the data is interleaved or non-interleaved.

<br />

#### gs_graphics_bind_index_buffer_desc_t

Describes how an index buffer is to be bound to the pipeline.

```c
typedef struct gs_graphics_bind_index_buffer_desc_t
{
    gs_handle(gs_graphics_index_buffer_t)       buffer;
} gs_graphics_bind_index_buffer_desc_t;
```

**Members**

`buffer`

Handle to the index buffer to be bound.

<br />

#### gs_graphics_bind_image_buffer_desc_t

Specifies how an image buffer is to be bound to the pipeline.

```c
typedef struct gs_graphics_bind_image_buffer_desc_t
{
    gs_handle(gs_graphics_texture_t)        tex;
    uint32_t                                binding;
    gs_graphics_access_type                 access;
} gs_graphics_bind_image_buffer_desc_t;
```

**Members**

`tex`

Handle to the image buffer's texture to be bound.

`binding`

Slot to bind the texture to.

`access`

Specifies read/write permissions.

<br />

#### gs_graphics_bind_uniform_buffer_desc_t

Specifies how a uniform buffer is to be bound to the pipeline.

```c
typedef struct gs_graphics_bind_uniform_buffer_desc_t
{
    gs_handle(gs_graphics_uniform_buffer_t)     buffer;
    uint32_t                                    binding;
    struct
    {
        size_t                                  offset;
        size_t                                  size;
    } range;
} gs_graphics_bind_uniform_buffer_desc_t;
```

**Members**

`buffer`

Handle to the uniform buffer to be bound.

`binding`

Slot to bind the uniform buffer to.

`offset`

Specify an offset for ranged binds.

`size`

Specify size for ranged binds.

<br />

#### gs_graphics_bind_uniform_desc_t

Specifies how a uniform is to be bound to the pipeline.

```c
typedef struct gs_graphics_bind_uniform_desc_t
{
    gs_handle(gs_graphics_uniform_t)        uniform;
    void*                                   data;
    uint32_t                                binding;   // Base binding for samplers?
} gs_graphics_bind_uniform_desc_t;
```

**Members**

`uniform`

Handle to the uniform to be bound.

`data`

Data to be attached to the uniform.

`binding`

Slot to bind the uniform to. (NOTE: base binding for samplers)?

<br />

#### gs_graphics_bind_desc_t

Specifies a set of bindings to apply to the pipeline.

```c
typedef struct gs_graphics_bind_desc_t
{
    struct {
        gs_graphics_bind_vertex_buffer_desc_t*      desc;
        size_t                                      size;
    } vertex_buffers;

    struct {
        gs_graphics_bind_index_buffer_desc_t*       desc;
        size_t                                      size;
    } index_buffers;

    struct {
        gs_graphics_bind_uniform_buffer_desc_t*     desc;
        size_t                                      size;
    } uniform_buffers;

    struct {
        gs_graphics_bind_uniform_desc_t*            desc;
        size_t                                      size;
    } uniforms;

    struct {
        gs_graphics_bind_image_buffer_desc_t*       desc;
        size_t                                      size;
    } image_buffers;
} gs_graphics_bind_desc_t;
```

**Members**

`desc`

Array of declarations (NULL by default).

`size`

Size of array in bytes (optional if one is specified).

<br />

#### gs_graphics_blend_state_desc_t

Specifies a blend state.

```c
typedef struct gs_graphics_blend_state_desc_t
{
    gs_graphics_blend_equation_type     func;
    gs_graphics_blend_mode_type         src;
    gs_graphics_blend_mode_type         dst;
} gs_graphics_blend_state_desc_t;
```

**Members**

`func`

Equation function to use for blend ops.

`src`

Source blend mode.

`dst`

Destination blend mode.

<br />

#### gs_graphics_depth_state_desc_t

Specifies a depth state.

```c
typedef struct gs_graphics_depth_state_desc_t
{
    gs_graphics_depth_func_type     func;
} gs_graphics_depth_state_desc_t;
```

**Members**

`func`

Function to set for depth test.

<br />

#### gs_graphics_stencil_state_desc_t

Specifies a stencil state.

```c
typedef struct gs_graphics_stencil_state_desc_t
{
    gs_graphics_stencil_func_type       func;
    uint32_t                            ref;
    uint32_t                            comp_mask;
    uint32_t                            write_mask;
    gs_graphics_stencil_op_type         sfail;
    gs_graphics_stencil_op_type         dpfail;
    gs_graphics_stencil_op_type         dppass;
} gs_graphics_stencil_state_desc_t;
```

**Members**

`func`

Function to set for stencil test.

`ref`

Specifies reference val for stencil test.

`comp_mask`

Specifies mask that is ANDed with both ref val and stored stencil val.

`write_mask`

Specifies mask that is ANDed with both ref val and stored stencil val.

`sfail`

Action to take when stencil test fails.

`dpfail`

Action to take when stencil test passes but depth test fails.

`dppass`

Action to take when both stencil test passes and either depth passes or is not enabled.

<br />

#### gs_graphics_raster_state_desc_t

DESC

```c
```

**Members**

<br />

#### gs_graphics_compute_state_desc_t

DESC

```c
```

**Members**

<br />

#### gs_graphics_vertex_attribute_desc_t

DESC

```c
```

**Members**

<br />

#### gs_graphics_vertex_layout_desc_t

DESC

```c
```

**Members**

<br />

#### gs_graphics_pipeline_desc_t

DESC

```c
```

**Members**

<br />

#### gs_graphics_draw_desc_t

DESC

```c
```

**Members**

<br />

