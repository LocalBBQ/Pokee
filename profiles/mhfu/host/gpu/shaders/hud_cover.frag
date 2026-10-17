#version 450

// The game's HUD drawn again over the interface's marks (gather icon, damage
// numbers, lock-on, hunt portraits). `finished` is the game's frame as shown;
// `world` is the same frame copied just before its first HUD draw. Where the
// two differ, the HUD (or something drawn after it) covered the pixel, and
// the finished frame is drawn there again; elsewhere the marks stay.

layout(set = 0, binding = 0) uniform sampler2D finished;
layout(set = 0, binding = 1) uniform sampler2D world;

layout(push_constant) uniform Picture {
    vec2 origin; // the game's picture in the window, in pixels
    vec2 size;
} picture;

layout(location = 0) out vec4 colour;

void main() {
    vec2 uv = (gl_FragCoord.xy - picture.origin) / picture.size;
    if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) discard;
    vec3 shown = texture(finished, uv).rgb;
    vec3 under = texture(world, uv).rgb;
    vec3 change = abs(shown - under);
    // Half a step of an 8-bit channel: the two copies of an untouched pixel
    // are the same bytes, filtered the same way.
    if (max(change.r, max(change.g, change.b)) < 0.5 / 255.0) discard;
    colour = vec4(shown, 1.0);
}
