#version 450

// PSP geometry. Transformed vertices arrive in object space and are multiplied
// by the combined world-view-projection matrix; "through" vertices are already
// in screen pixels and are mapped to clip space with the viewport size.
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec2 in_texcoord;
layout(location = 2) in vec4 in_color;
layout(location = 3) in vec3 in_normal;

layout(location = 0) out vec2 frag_texcoord;
layout(location = 1) out vec4 frag_color;
layout(location = 2) out vec3 frag_specular;
layout(location = 3) out float frag_fog;
// Through-mode tiles: the texture coordinates the tile may sample, as min.xy,
// max.xy. The vertex carries them in texels, in in_normal.xy and
// (in_normal.z, in_position.w), which through-mode vertices do not use
// otherwise; see clamp_through_quads() in vulkan_renderer.cpp.
layout(location = 4) flat out vec4 frag_uv_rect;

layout(push_constant) uniform Push {
    mat4 transform;      // WVP, or identity for through vertices
    vec4 viewport;       // xy: target size in PSP pixels, z: through, w: 1 fog + 2 lighting
    vec4 texture_params; // x: texture enabled, y: texture function, z: alpha ref, w: alpha func
    vec4 uv_transform;   // xy: scale, zw: offset
    vec4 view_z;         // row of view * world that gives view-space z
} push;

// The lighting environment, shared by every draw until the game changes it;
// the layout matches EnvironmentBlock on the host. Colours are 0..1; small
// integers are stored as floats.
layout(set = 1, binding = 0) uniform Environment {
    vec4 ambient;           // global ambient light, rgba
    vec4 fog;               // x: end, y: scale
    vec4 fog_color;
    vec4 light_position[4];    // xyz; w: enabled
    vec4 light_direction[4];   // xyz; w: type (0 directional, 1 point, 2 spot)
    vec4 light_attenuation[4]; // xyz: constant, linear, quadratic; w: kind
    vec4 light_spot[4];        // x: exponent, y: cutoff
    vec4 light_ambient[4];
    vec4 light_diffuse[4];
    vec4 light_specular[4];
} lighting;

// A lit draw's world matrix and material; the layout matches ObjectBlock.
layout(set = 1, binding = 1) uniform Object {
    mat4 world;
    vec4 flags;             // y: vertex has a colour, w: material update mask
    vec4 emissive;          // rgb; w: specular power
    vec4 material_ambient;  // rgba
    vec4 material_diffuse;  // rgb; w: 1 keeps specular apart
    vec4 material_specular; // rgb; w: reverse normals
} object;

// The GE's per-vertex lighting, evaluated in world space: emissive, plus the
// global ambient light times the material ambient, plus for each enabled light
// its ambient, diffuse and specular terms, scaled by distance attenuation and
// the spot cone. The material update mask makes the vertex colour stand in for
// the ambient (bit 0), diffuse (bit 1) and specular (bit 2) material colours;
// a vertex without a colour keeps the material ones.
void light_vertex(out vec4 color, out vec3 separate_specular) {
    int mask = int(object.flags.w + 0.5);
    bool has_color = object.flags.y > 0.5;
    vec4 ambient_material = (has_color && (mask & 1) != 0) ? in_color : object.material_ambient;
    vec3 diffuse_material = (has_color && (mask & 2) != 0) ? in_color.rgb : object.material_diffuse.rgb;
    vec3 specular_material = (has_color && (mask & 4) != 0) ? in_color.rgb : object.material_specular.rgb;
    float power = object.emissive.w;

    vec3 world_position = (object.world * vec4(in_position.xyz, 1.0)).xyz;
    // Skinned normals come out of the bone matrices far from unit length, so
    // the GE's normalisation after the transform matters.
    vec3 normal = mat3(object.world) * in_normal;
    float length_squared = dot(normal, normal);
    normal = length_squared > 0.0 ? normal * inversesqrt(length_squared) : vec3(0.0, 0.0, 1.0);
    if (object.material_specular.w > 0.5) normal = -normal;

    vec3 sum = object.emissive.rgb + lighting.ambient.rgb * ambient_material.rgb;
    vec3 specular = vec3(0.0);
    for (int i = 0; i < 4; ++i) {
        if (lighting.light_position[i].w < 0.5) continue;
        int type = int(lighting.light_direction[i].w + 0.5);
        int kind = int(lighting.light_attenuation[i].w + 0.5);
        vec3 to_light = lighting.light_position[i].xyz;
        float scale = 1.0;
        if (type != 0) {
            to_light -= world_position;
            float distance = length(to_light);
            vec3 k = lighting.light_attenuation[i].xyz;
            scale = clamp(1.0 / max(k.x + k.y * distance + k.z * distance * distance, 1e-20), 0.0, 1.0);
        }
        to_light = dot(to_light, to_light) > 0.0 ? normalize(to_light) : vec3(0.0, 0.0, 1.0);
        if (type == 2) {
            vec3 axis = lighting.light_direction[i].xyz;
            axis = dot(axis, axis) > 0.0 ? normalize(axis) : vec3(0.0, 0.0, 1.0);
            float angle = dot(axis, -to_light);
            scale *= angle >= lighting.light_spot[i].y ? pow(max(angle, 0.0), lighting.light_spot[i].x) : 0.0;
        }
        float n_dot_l = dot(normal, to_light);
        float diffuse = max(n_dot_l, 0.0);
        if (kind == 2) diffuse = pow(diffuse, power);
        sum += (lighting.light_ambient[i].rgb * ambient_material.rgb +
                lighting.light_diffuse[i].rgb * diffuse_material * diffuse) * scale;
        if (kind == 1 && n_dot_l >= 0.0) {
            // The viewer is taken to look down z, as the GE does.
            vec3 half_vector = normalize(to_light + vec3(0.0, 0.0, 1.0));
            specular += lighting.light_specular[i].rgb * specular_material *
                        pow(max(dot(normal, half_vector), 0.0), power) * scale;
        }
    }
    float alpha = lighting.ambient.a * ambient_material.a;
    if (object.material_diffuse.w > 0.5) {
        separate_specular = clamp(specular, 0.0, 1.0);
    } else {
        sum += specular;
        separate_specular = vec3(0.0);
    }
    color = clamp(vec4(sum, alpha), 0.0, 1.0);
}

void main() {
    frag_texcoord = in_texcoord * push.uv_transform.xy + push.uv_transform.zw;
    frag_color = in_color;
    frag_specular = vec3(0.0);
    frag_fog = 1.0;
    frag_uv_rect = vec4(-1e30, -1e30, 1e30, 1e30);
    if (push.viewport.z > 0.5) {
        vec4 rect = vec4(in_normal.xy, in_normal.z, in_position.w);
        frag_uv_rect = vec4(rect.xy * push.uv_transform.xy + push.uv_transform.zw,
                            rect.zw * push.uv_transform.xy + push.uv_transform.zw);
        // Screen-space vertices: pixels to clip space.
        vec2 ndc = vec2(in_position.x / push.viewport.x, in_position.y / push.viewport.y) * 2.0 - 1.0;
        gl_Position = vec4(ndc, clamp(in_position.z / 65535.0, 0.0, 1.0), 1.0);
    } else {
        int enables = int(push.viewport.w + 0.5);
        if ((enables & 2) != 0) light_vertex(frag_color, frag_specular);
        // Fog runs linearly from 1 (clear) to 0 (fogged) with view-space z,
        // which is negative in front of the camera: (z + end) * scale.
        if ((enables & 1) != 0)
            frag_fog = (dot(push.view_z, vec4(in_position.xyz, 1.0)) + lighting.fog.x) * lighting.fog.y;
        vec4 clip = push.transform * vec4(in_position.xyz, 1.0);
        // PSP clip space follows OpenGL with z in [-w, w]; Vulkan clips against
        // [0, w], so without this remap the near half of every frustum is lost.
        // The PSP viewport's z scale and offset are folded into the Vulkan
        // viewport's min/max depth, which expects this [0, 1] device z.
        clip.z = (clip.z + clip.w) * 0.5;
        gl_Position = clip;
    }
}
