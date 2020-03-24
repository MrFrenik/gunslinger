#version 330 core

out vec4 frag_color;

in vec2 tex_coord;
in vec3 normal;
in vec3 frag_pos;

uniform sampler2D s_noise_tex;
uniform vec3 u_view_pos;

vec3 light_pos = vec3( -200.0, 1000.0, 200.0 ); 
vec3 light_col = vec3( 1.0, 1.0, 1.0 );
vec3 amb_col = vec3(0.3, 0.3, 0.45);

void main()
{
    // Tex color
    vec3 tex_color = texture(s_noise_tex, tex_coord).rgb;

     // ambient
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * light_col;

    light_pos = vec3(-200.0, 1000.0, -200.0);
    
    // // diffuse 
    float light_strength = 1.2;
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light_pos - frag_pos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light_col * light_strength;
    
    // // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(u_view_pos - frag_pos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * light_col;  
        
    vec3 result = (ambient + diffuse + specular) * tex_color;
    frag_color = vec4(clamp(result, 0.0, 1.0), 1.0);
} 
//