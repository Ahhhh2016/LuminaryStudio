#version 330 core

// input
layout(location = 0) in vec3 object_position3d;
layout(location = 1) in vec3 object_normal;
layout(location = 2) in vec2 _texture_uv_coordinate;

// output
out vec3 world_position3d;
out vec3 world_normal;
out vec2 texture_uv_coordinate;

uniform vec4 plane;

// matrixs
//uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main() 
{
    //world_position3d = vec3(model_matrix * vec4(object_position3d, 1.0f));
    //world_normal = normalize(inverse(transpose(mat3(model_matrix))) * normalize(object_normal));
    world_position3d = object_position3d;
    world_normal = object_normal;
    texture_uv_coordinate = _texture_uv_coordinate;

    //gl_ClipDistance[0] = dot(vec4(world_position3d, 1.0f), plane);

    gl_Position = projection_matrix * view_matrix * vec4(world_position3d, 1.0f);
}
