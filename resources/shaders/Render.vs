#version 330 core
layout (location = 0)in vec3 position;
layout (location = 1)in float size;
layout (location = 2)in float Alpha;

out vec3 pos;
out float alpha;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 dsUniform;
void main()
{

        pos[0] = position[0] + dsUniform[0];
        pos[1] = position[1] + dsUniform[1];
        pos[2] = position[2] + dsUniform[2];
        alpha = Alpha;
	gl_PointSize = size;
	gl_Position = projection * view * model * vec4(position,1.0f);

}
