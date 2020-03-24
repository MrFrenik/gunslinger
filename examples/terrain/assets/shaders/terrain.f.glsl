#version 330 core

out vec4 frag_color;

in vec2 tex_coord;

uniform sampler2D s_noise_tex;

void main()
{
    frag_color = texture(s_noise_tex, tex_coord);
}