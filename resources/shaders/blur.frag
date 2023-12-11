#version 330 core

// Task 16: Create a UV coordinate in variable
in vec2 uv_coordinate;

// Task 8: Add a sampler2D uniform
uniform sampler2D screenTexture;
uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

// Task 29: Add a bool on whether or not to filter the texture
//uniform bool isFilter;

out vec4 fragColor;

void main()
{
    vec2 tex_offset = 1.0 / textureSize(screenTexture, 0); // gets size of single texel
    vec4 result = texture(screenTexture, uv_coordinate) * weight[0]; // current fragment's contribution
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(screenTexture, uv_coordinate + vec2(tex_offset.x * i, 0.0)) * weight[i];
            result += texture(screenTexture, uv_coordinate - vec2(tex_offset.x * i, 0.0)) * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(screenTexture, uv_coordinate + vec2(0.0, tex_offset.y * i)) * weight[i];
            result += texture(screenTexture, uv_coordinate - vec2(0.0, tex_offset.y * i)) * weight[i];
        }
    }
    fragColor = vec4(result.rgb, 0.5);
}
