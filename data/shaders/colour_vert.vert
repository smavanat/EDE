#version 330 core

out vec2 uv;

void main() {
    vec2 positions[4] = vec2[](
        vec2(-1.0, -1.0), // 0
        vec2( 1.0, -1.0), // 1
        vec2(-1.0,  1.0), // 2
        vec2( 1.0,  1.0)  // 3
    );

    vec2 pos = positions[gl_VertexID];
    uv = (pos + 1.0) * 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);
}
