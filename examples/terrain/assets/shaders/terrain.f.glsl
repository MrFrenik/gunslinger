#version 330 core

in vec2 tex_coord;
in vec3 normal;
in vec3 frag_pos;
in float height;

out vec4 frag_color;

uniform sampler2D u_noise_tex;
uniform vec3 u_view_pos;

float _HeightblendFactor = 10.0;

vec3 light_pos = vec3( -200.0, 1000.0, 200.0 ); 
vec3 light_col = vec3( 1.0, 1.0, 1.0 );
vec3 amb_col = vec3(0.3, 0.3, 0.45);

// {0.3f, {10, 20, 150, 255}},     // Deep Water
// {0.5f, {10, 50, 250, 255}},     // Shallow Water
// {0.53f, {255, 255, 153, 255}},  // Sand/Beach
// {0.6f, {100, 170, 40, 255}},    // Grass
// {0.65f, {100, 140, 30, 255}},   // Grass2
// {0.8f, {153, 102, 10, 255}},    // Rock
// {0.85f, {51, 26, 0, 255}},      // Rock2
// {1.0f, {200, 190, 210, 255}}    // Snow

vec3 deep_water = vec3(20.0, 40.0, 100.0) / 255.0;
vec3 shallow_water = vec3(10.0, 50.0, 200.0) / 255.0;
vec3 sand      = vec3(130.0, 130.0, 70.0) / 255.0;
vec3 grass      = vec3(60.0, 100.0, 30.0) / 255.0;
vec3 rock      = vec3(153.0, 102.0, 10.0) / 255.0;
vec3 rock2       = vec3(51.0, 26.0, 0.0) / 255.0;
vec3 snow       = vec3(200.0, 190.0, 210.0) / 255.0;

float max_height = 20.0;

vec3 heightblend(vec3 input1, float height1, vec3 input2, float height2)
{
    float height_start = max(height1, height2) - _HeightblendFactor;
    float level1 = max(height1 - height_start, 0.0);
    float level2 = max(height2 - height_start, 0.0);
    return ((input1 * level1) + (input2 * level2)) / (level1 + level2);
}

vec3 mix_cols(vec3 c0, vec3 c1, float v0, float v1, float height)
{
    float blendAmount = (height - v0) * ( 1.0 / (v1 - v0 ) );
    return mix(c0, c1, blendAmount);
}

// Want to mix colors as they ramp upward
vec3 color_from_height( float height )
{
    // Normalize height between 0 and 1
    height /= max_height;
    height = clamp( height, 0.0, 1.0 );
    float blendAmount = 1.0;

    vec3 col = vec3(0.0, 0.0, 0.0);
    if ( height < 0.07 ) 
    {
        blendAmount = height / 0.07;
        col = mix(deep_water, shallow_water, blendAmount);
    } 
    else if ( height >= 0.07 && height < 0.075 )  col = mix_cols(shallow_water, sand, 0.07, 0.075, height);
    else if ( height >= 0.075 && height < 0.53 ) col = mix_cols(sand, grass, 0.35, 0.53, height);
    else if ( height >= 0.53 && height < 0.8 ) col = mix_cols(grass, rock, 0.53, 0.8, height);
    else if ( height >= 0.8 && height < 0.9 )  col = mix_cols(rock, rock2, 0.8, 0.9, height);
    else if ( height >= 0.9 && height <= 1.0 )  col = mix_cols(rock2, snow, 0.9, 1.0, height);

    return clamp( col, 0.0, 1.0 );

    // if ( height < 1.0 ) return mix( deep_water, rock2, height );
    // vec3 col = vec3(0.0, 0.0, 0.0);
    // col = mix( deep_water, grass, height / max_height );    // Not sure how to mix over ranges...
    // col = mix( col, rock2, height / max_height );
    // return col;
    // return mix( deep_water, rock2, height / max_height );
    // Want to blend between regions...
    // if ( height <= 2.0 ) return deep_water; 
    // else if ( height <= 3.0 ) return mix(deep_water, grass, height / max_height ); 
    // else if ( height <= 3.5 ) return mix(deep_water, grass, height); 
    // else return rock2;
}

// float distance(vec2 v0, vec2 v1)
// {
//     float dx = v1.x - v0.x;
//     float dy = v1.y - v0.y;
//     return sqrt((dx * dx) + (dy * dy));
// }

float
map_range( float input_start, float input_end, float output_start, float output_end, float val )
{
    float slope = ( output_end - output_start ) / ( input_end - input_start );
    return ( output_start + ( slope * ( val - input_start ) ) );
}

void main()
{
    // Tex color
    // vec3 tex_color = texture2D(s_noise_tex, tex_coord).rgb;
    // tex_color = mix( tex_color, texture2D( s_noise_tex, tex_coord + vec2(-1.0, -1.0) ).rgb, 0.5 );
    // tex_color = mix( tex_color, texture2D( s_noise_tex, tex_coord + vec2(1.0, -1.0) ).rgb, 0.5 );
    // tex_color = mix( tex_color, texture2D( s_noise_tex, tex_coord + vec2(1.0, 1.0) ).rgb, 0.5 );
    // tex_color = mix( tex_color, texture2D( s_noise_tex, tex_coord + vec2(1.0, 0.0) ).rgb, 0.5 );
    // tex_color = mix( tex_color, texture2D( s_noise_tex, tex_coord + vec2(0.0, 1.0) ).rgb, 0.5 );

    // mix the alpha of the fragment from the center? (want it to blend away from center)
    vec3 tex_color = color_from_height( height );

    float alpha = 1.0;

    float r = 0.4;
    float dist = distance(tex_coord, vec2(0.5, 0.5));
    // tex_color = vec3(1.0, 1.0, 1.0);
    if ( dist > r )
    {
        // tex_color = vec3(dist, dist, dist) * 2.0;
        float blend = 1.0 - map_range( 0.4, 0.6, 0.0, 1.0, dist );
        alpha = mix( 0.0, 1.0, blend );
    }

    /*
        dist = 0.1

        0.1 - 0.2 = -0.1 / 0.8 = -0.125 

        want to map range 0 -> 0.2 onto 0.0 to 1.0

        dist - 
    */

    // if ( tex_coord.x <= 0.15 ) {
    //     float blendAmount = (tex_coord.x - 0.0) / ( 0.15 - 0.0 );
    //     alpha = mix( 0.0, 1.0, blendAmount );
    // }
    // if ( tex_coord.x > 0.9 ) {
    //     float blendAmount = 1.0 - (tex_coord.x - 0.9) / ( 1.0 - 0.9 );
    //     alpha = mix( 0.0, 1.0, blendAmount );
    // }
    // if ( tex_coord.y <= 0.1 ) {
    //     float blendAmount = (tex_coord.y - 0.0) / ( 0.1 - 0.0 );
    //     alpha = mix( 0.0, 1.0, blendAmount );
    // }
    // if ( tex_coord.y > 0.9 ) {
    //     float blendAmount = 1.0 - (tex_coord.y - 0.9) / ( 1.0 - 0.9 );
    //     alpha = mix( 0.0, 1.0, blendAmount );
    // }

     // ambient
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * light_col;

    light_pos = vec3(-200.0, 1000.0, -200.0);
    
    // // diffuse 
    float light_strength = 1.0;
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light_pos - frag_pos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light_col * light_strength;
    
    // // specular
    float specularStrength = 0.1;
    vec3 viewDir = normalize(u_view_pos - frag_pos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * light_col;  
        
    vec3 result = (ambient + diffuse + specular) * tex_color;
    frag_color = vec4(clamp(result, 0.0, 1.0), alpha);
}