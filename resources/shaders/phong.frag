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
uniform bool apply_reflection;

// dudv
uniform sampler2D dudvMap;
const float waveStrength = 0.05;
uniform float moveFactor;

// normalMap
uniform sampler2D normalMap;
const float shineDamper = 20.0f;
const float reflectivity = 0.6f;
const vec3 lightColour = vec3(1.0f, 0.7f, 0.3f);
const vec3 lightPosition = vec3(-100.0, 100.0, -100.0);

// refraction
const float eta = 1.0f / 1.333f; // air to water

//move texture
uniform float radius_light;
uniform vec2 light_offset;

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
            vec4 color = texture(object_texture, texture_uv_coordinate);
            vec2 center[3] = vec2[3](vec2(0.5, 0.1),  vec2(0.4, 0.15), vec2(0.6, 0.1));
            for(int j = 0; j < 3; j++) {
                if(j % 2 == 1) center[j] += light_offset;
                else center[j] -= light_offset;
                float dis = distance(center[j], texture_uv_coordinate);
                if(dis > radius_light) continue;
                color += (radius_light - dis) / radius_light * vec4(1.5, 1.5, 0.0, 1.0) * (1 - color);
            }
            diffuse = (1.0f - blend) * diffuse + blend * color;
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
        //reflection vector
        vec3 I_reflection = normalize(world_position3d - vec3(camera_pos));
        vec3 R_reflection = reflect(I_reflection, normalize(world_normal));

        // refraction vector
        vec3 newcamera_pos = vec3(camera_pos[0], camera_pos[1], camera_pos[2]);
        vec3 I_refraction = normalize(world_position3d - vec3(newcamera_pos));
        vec3 T = refract(I_refraction, normalize(world_normal), eta);
        float cosTheta = dot(-I_refraction, normalize(world_normal));
        float R0 = pow((1.0 - 1.333) / (1.0 + 1.333), 2.0);
        float refractiveFactor = R0 + (1.0 - R0) * pow(1.0 - cosTheta, 5.0);

        // distortion
        vec2 distortedTexCoords = texture(dudvMap, vec2(texture_uv_coordinate[0] + moveFactor, texture_uv_coordinate[1])).rg*0.1f;
        distortedTexCoords = texture_uv_coordinate + vec2(distortedTexCoords[0], distortedTexCoords[1]+moveFactor);
        vec2 totalDistortion = (texture(dudvMap, distortedTexCoords).rg * 2.0 - 1.0) * waveStrength;

        R_reflection.xy += totalDistortion;
        T.xy += totalDistortion;
        vec4 reflection_color = vec4(texture(skybox, R_reflection).rgb, 1.0f);
        vec4 refraction_color = vec4(texture(skybox, T).rgb, 1.0f);

        // normal map
        vec4 normalmap_color = texture(normalMap, distortedTexCoords);
        vec3 m_normal = vec3(normalmap_color.r * 2.0f - 1.0f, normalmap_color.b, normalmap_color.g * 2.0f - 1.0f);
        m_normal = normalize(m_normal);

        // add a sun
        vec3 fromLightVector = world_position3d - lightPosition;
        vec3 m_reflectedLight = reflect(normalize(fromLightVector), m_normal);
        float m_specular = max(dot(m_reflectedLight, I_reflection), 0.0);
        m_specular = pow(m_specular, shineDamper);
        vec3 specularHighlights = lightColour * m_specular * reflectivity;

        // final color
        frag_color = mix(reflection_color, refraction_color, clamp(refractiveFactor, 0.6f, 1.0f));
        frag_color = mix(frag_color, vec4(0.0f, 0.3f, 0.5f, 1.0f), 0.2) + vec4(specularHighlights * 1.2f, 0.0f);
    }
    else if (apply_reflection)
    {
        //reflection vector
        vec3 I_reflection = normalize(world_position3d - vec3(camera_pos));
        vec3 R_reflection = reflect(I_reflection, normalize(world_normal));

        // // refraction vector
        // vec3 newcamera_pos = vec3(camera_pos[0], camera_pos[1], camera_pos[2]);
        // vec3 I_refraction = normalize(world_position3d - vec3(newcamera_pos));
        // vec3 T = refract(I_refraction, normalize(world_normal), eta);
        // float cosTheta = dot(-I_refraction, normalize(world_normal));
        // float R0 = pow((1.0 - 1.333) / (1.0 + 1.333), 2.0);
        // float refractiveFactor = R0 + (1.0 - R0) * pow(1.0 - cosTheta, 5.0);

        // tecxture color
        vec4 reflection_color = vec4(texture(skybox, R_reflection).rgb, 1.0f);
        //vec4 refraction_color = vec4(texture(skybox, T).rgb, 1.0f);

        // final color
        frag_color = mix(frag_color, reflection_color, 0.7f);
    }

}
