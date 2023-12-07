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

uniform float time;

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

// skybox
uniform samplerCube skybox;
uniform bool is_water;

// dudv
uniform sampler2D dudvMap;
const float waveStrength = 0.15;
uniform float moveFactor;

// normalMap
uniform sampler2D normalMap;
const float shineDamper = 20.0f;
const float reflectivity = 0.6f;
const vec3 lightColour = vec3(1.0f, 1.0f, 0.8f);
const vec3 lightPosition = vec3(-100.0, 100.0, 100.0);

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

    if (is_water)
    {
        // // // Calculate reflection vector with distorted normal
        vec3 I = normalize(world_position3d - vec3(camera_pos));
        vec3 R = reflect(I, normalize(world_normal));

        vec2 distortedTexCoords = texture(dudvMap, vec2(texture_uv_coordinate[0] + moveFactor, texture_uv_coordinate[1])).rg*0.1f;
        distortedTexCoords = texture_uv_coordinate + vec2(distortedTexCoords[0], distortedTexCoords[1]+moveFactor);
        vec2 totalDistortion = (texture(dudvMap, distortedTexCoords).rg * 2.0 - 1.0) * waveStrength;

        R.xy += totalDistortion;
        vec4 env_color = vec4(texture(skybox, R).rgb, 1.0f);

        vec4 normalmap_color = texture(normalMap, distortedTexCoords);
        vec3 m_normal = vec3(normalmap_color.r * 2.0f - 1.0f, normalmap_color.b, normalmap_color.g * 2.0f - 1.0f);
        m_normal = normalize(m_normal);

        vec3 fromLightVector = world_position3d - lightPosition;
        vec3 m_reflectedLight = reflect(normalize(fromLightVector), m_normal);
        float m_specular = max(dot(m_reflectedLight, I), 0.0);
        m_specular = pow(m_specular, shineDamper);
        vec3 specularHighlights = lightColour * m_specular * reflectivity;

        frag_color = mix(frag_color, env_color, 0.5);
        frag_color = mix(frag_color, vec4(0.0f, 0.3f, 0.5f, 1.0f), 0.2) + vec4(specularHighlights, 0.0f);
    }

}
