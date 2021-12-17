#  Graphics
This document shows off the graphics API provided by gunslinger.

## Functions
### Resource Creation:

The following functions are used for creating resources, such as vertex buffers, textures, uniforms, etc.

TODO: add hyperlinking

* gs_graphics_texture_create(const gs_graphics_texture_desc_t* desc)
* gs_graphics_uniform_create(const gs_graphics_uniform_desc_t* desc)
* gs_graphics_shader_create(const gs_graphics_shader_desc_t* desc)
* gs_graphics_vertex_buffer_create(const gs_graphics_vertex_buffer_desc_t* desc)
* gs_graphics_index_buffer_create(const gs_graphics_index_buffer_desc_t* desc)
* gs_graphics_uniform_buffer_create(const gs_graphics_uniform_buffer_desc_t* desc)
* gs_graphics_framebuffer_create(const gs_graphics_framebuffer_desc_t* desc)
* gs_graphics_render_pass_create(const gs_graphics_render_pass_desc_t* desc)
* gs_graphics_pipeline_create(const gs_graphics_pipeline_desc_t* desc)

<br />

#### gs_graphics_texture_create
Creates a texture resource.
```
gs_handle(gs_graphics_texture_t)
gs_graphics_texture_create(const gs_graphics_texture_desc_t* desc)

```
**Parameters:**
 
`desc`

A pointer to a `gs_graphics_texture_desc_t` structure that describes the texture to be created.

**Return Value:**

Returns a handle to a texture resource.

<br />

#### gs_graphics_uniform_create
```

```
**Parameters:**

**Return Value:**



<br />


#### gs_graphics_shader_create
```

```
**Parameters:**

**Return Value:**



<br />


#### gs_graphics_vertex_buffer_create
```

```
**Parameters:**

**Return Value:**



<br />


#### gs_graphics_index_buffer_create
```

```
**Parameters:**

**Return Value:**



<br />


#### gs_graphics_uniform_buffer_create
```

```
**Parameters:**

**Return Value:**



<br />


#### gs_graphics_framebuffer_create
```

```
**Parameters:**

**Return Value:**



<br />


#### gs_graphics_render_pass_create
```

```
**Parameters:**

**Return Value:**



<br />


#### gs_graphics_pipeline_create
```

```
**Parameters:**

**Return Value:**



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

