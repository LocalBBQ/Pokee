#version 450

// The game's HUD over the interface's marks: one triangle covering the
// window. The scissor keeps it to the game's picture.

void main() {
    // (0,0), (2,0), (0,2): the triangle that covers the whole target.
    vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(uv * 2.0 - 1.0, 0.0, 1.0);
}
