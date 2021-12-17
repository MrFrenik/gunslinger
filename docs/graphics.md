#  Graphics
This document shows off the graphics API provided by gunslinger.

## Functions
### Resource Creation:

The following functions are used for creating resources, such as vertex buffers, textures, uniforms, etc.

TODO: add hyperlinking

* gs_graphics_texture_create
* gs_graphics_uniform_create
* gs_graphics_shader_create
* gs_graphics_vertex_buffer_create
* gs_graphics_index_buffer_create
* gs_graphics_uniform_buffer_create
* gs_graphics_framebuffer_create
* gs_graphics_render_pass_create
* gs_graphics_pipeline_create

<br />

#### gs_graphics_texture_create
Creates a texture resource.
```
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
```
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
```
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
```
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
```
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
```
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
```
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
```
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
```
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

* gs_graphics_texture_destroy(gs_handle(gs_graphics_texture_t) hndl)
* gs_graphics_shader_destroy(gs_handle(gs_graphics_shader_t) hndl)
* gs_graphics_render_pass_destroy(gs_handle(gs_graphics_render_pass_t) hndl)
* gs_graphics_pipeline_destroy(gs_handle(gs_graphics_pipeline_t) hndl)

<br />

#### gs_graphics_texture_destroy
Destroys a texture resource.
```
void
gs_graphics_texture_destroy(
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
```
void
gs_graphics_shader_destroy(
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
```
void
gs_graphics_render_pass_destroy(
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
```
void
gs_graphics_pipeline_destroy(
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

#### gs_graphics_texture_desc_t
Describes a texture resource.
```
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

