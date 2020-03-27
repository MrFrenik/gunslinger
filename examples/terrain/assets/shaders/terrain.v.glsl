#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texcoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec2 tex_coord;
out vec3 normal;
out vec3 frag_pos;
out float height;

void main()
{
    gl_Position = u_proj * u_view * u_model * vec4(a_pos, 1.0);
    normal = mat3(transpose(inverse(u_model))) * a_normal;
    tex_coord = a_texcoord;
    frag_pos = vec3(u_model * vec4(a_pos, 1.0));
    height = a_pos.y;
}