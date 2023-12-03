#version 330 core

// Task 16: Create a UV coordinate in variable
in vec2 uv_coordinate;

// Task 8: Add a sampler2D uniform
uniform sampler2D u_texture;

// Task 29: Add a bool on whether or not to filter the texture
uniform bool invert_filter;
uniform bool sharpen_filter;
uniform bool grayscale_filter;
uniform bool blur_filter;

uniform int width;
uniform int height;

out vec4 fragColor;

void main()
{
    mat3 sharpen_kernel = 1.0f / 9.0f * mat3(-1.0f, -1.0f, -1.0f, -1.0f, 17.0f, -1.0f, -1.0f, -1.0f, -1.0f);
    // Task 17: Set fragColor using the sampler2D at the UV coordinate
    fragColor = texture(u_texture, uv_coordinate);

    if (sharpen_filter)
    {
        vec4 temp_color = vec4(0.0f);
        for (int row = 0; row < 3; row++)
        {
            for (int col = 0; col < 3; col++)
            {
                vec2 new_uv = vec2(clamp(uv_coordinate[0] + float(col - 1) / float(width), 0.0f, 1.0f), clamp(uv_coordinate[1] + float(row - 1) / float(height), 0.0f, 1.0f));
                temp_color += sharpen_kernel[col][row] * texture(u_texture, new_uv);
            }
        }
        fragColor = vec4(temp_color[0], temp_color[1], temp_color[2], fragColor[3]);
    }

    if (invert_filter)
    {
        fragColor = vec4(1.0f - fragColor[0], 1.0f - fragColor[1], 1.0f - fragColor[2], fragColor[3]);
    }

    if (blur_filter)
    {
        vec4 temp_color = vec4(0.0f);
        for (int row = 0; row < 5; row++)
        {
            for (int col = 0; col < 5; col++)
            {
                vec2 new_uv = vec2(clamp(uv_coordinate[0] + float(col - 2) / float(width), 0.0f, 1.0f), clamp(uv_coordinate[1] + float(row - 2) / float(height), 0.0f, 1.0f));
                temp_color += 1.0f / 25.0f * texture(u_texture, new_uv);
            }
        }
        fragColor = vec4(temp_color[0], temp_color[1], temp_color[2], fragColor[3]);
    }

    if (grayscale_filter)
    {
        float grayscale = 0.299 * fragColor[0] + 0.587 * fragColor[1] + 0.114 * fragColor[2];
        fragColor = vec4(grayscale, grayscale, grayscale, fragColor[3]);
    }
}
