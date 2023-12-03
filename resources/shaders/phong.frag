#version 330 core

// structure
struct Light{
    vec3 function;
    vec4 color;
    vec4 pos;
    int type; // 1 dir, 2 point, 3 spot
    vec4 direction;
    float penumbra;
    float angle;
};

// input
in vec3 world_position3d;
in vec3 world_normal;
uniform vec4 material_ambient;
uniform vec4 material_diffuse;
uniform vec4 material_specular;

// Texture
in vec2 texture_uv_coordinate;
uniform float blend;
uniform sampler2D object_texture;
uniform bool use_texture;

// output
out vec4 frag_color;

// global
uniform float ka;
uniform float kd;
uniform float ks;

// light
uniform int num_lights;
uniform Light lights[8];
uniform int shininess;

// camera
uniform vec4 camera_pos;

void main() {
    frag_color = vec4(0.0f);

    // Ambient
    frag_color += ka * material_ambient;

    for (int i = 0; i < num_lights; i++)
    {
        Light light = lights[i];
        vec4 n, L, R, E;
        float temp, fatt = 1.0f, dis;
        float outer, inner, x, intensity = 1.0f, falloff;

        n = normalize(vec4(world_normal, 0.0f));
        E = normalize(camera_pos - vec4(world_position3d, 1.0f));

        switch (light.type)
        {
        case 1: // directional
            L = normalize(-light.direction);
            R = -normalize(reflect(L, n));
            break;
        case 2: // point
            dis = max(0, distance(light.pos, vec4(world_position3d, 1.0f)));
            fatt = min(1.0f, 1.0f / (light.function[0] + light.function[1] * dis + light.function[2] * dis * dis));
            L = normalize(light.pos - vec4(world_position3d, 1.0f));
            R = -normalize(reflect(L, n));
            break;
        case 3: // spot
            dis = max(0,distance(light.pos, vec4(world_position3d, 1.0f)));
            fatt = min(1.0f, 1.0f / (light.function[0] + light.function[1] * dis + light.function[2] * dis * dis));
            L = normalize(light.pos - vec4(world_position3d, 1.0f));
            R = -normalize(reflect(L, n));

            outer = light.angle;
            inner = outer - light.penumbra;
            x = acos(dot(normalize(light.direction), -L));
            intensity = (x < inner) ? 1.0f : 1.0f - ((x > outer) ? 1.0f : (-2.0f * pow((x - inner) / (outer - inner), 3) + 3.0f * pow((x - inner) / (outer - inner), 2)));
            break;
        default:
            break;
        }

        vec4 diffuse = kd * material_diffuse;
        if (use_texture)
        {
            diffuse = (1.0f - blend) * diffuse + blend * texture(object_texture, texture_uv_coordinate);
        }

        frag_color += fatt * light.color * intensity * diffuse * clamp(dot(n, L), 0.0f, 1.0f);
        temp = clamp(dot(R, E), 0.0f, 1.0f);
        if (temp != 0)
        {
            frag_color += fatt * light.color * intensity * ks * material_specular * pow(temp, shininess);
        }

    }
}
