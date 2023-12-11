#version 330 core

// Task 16: Create a UV coordinate in variable
in vec2 uv_coordinate;

// Task 8: Add a sampler2D uniform
uniform sampler2D basic_Texture;
uniform sampler2D bloom_Texture;

out vec4 fragColor;
vec4 bloomColor;

void main()
{
    fragColor = texture(basic_Texture, uv_coordinate);
//    bloomColor = texture(bloom_Texture, uv_coordinate);
//    fragColor = bloomColor;
//    fragColor += bloomColor;
//    const float gamma = 1.8;
//    float exposure = 1.0f;
//     // tone mapping
//    vec3 result = vec3(1.0) - exp(-fragColor.rbg * exposure);
//    // also gamma correct while we're at it
//    fragColor.rbg = pow(result, vec3(1.0 / gamma));
}
