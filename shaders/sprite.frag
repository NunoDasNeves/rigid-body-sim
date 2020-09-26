#version 330 core
out vec4 FragColor;

in vec2 tex_coord;

uniform sampler2D sprite_texture;

uniform vec4 color_blend = vec4(1.0, 0.0, 0.0, 0.5);

void main()
{
    vec4 final_color = texture(sprite_texture, tex_coord);
    if(final_color.a < 0.1)
        discard;
    
    final_color.xyz = final_color.xyz * (1 - color_blend.a) + color_blend.xyz * color_blend.a;

    FragColor = final_color;
} 
