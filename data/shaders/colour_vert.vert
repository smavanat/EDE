#version 430
out vec2 v_uv;
uniform bool flip_y;

const vec2 V[4] = vec2[4](vec2(-1, -1), vec2(1, -1), vec2(-1, 1), vec2(1, 1));
const vec2 U[4] = vec2[4](vec2(0, 0), vec2(1, 0), vec2(0, 1), vec2(1, 1));

void main(){
    vec2 uv = U[gl_VertexID];
    if(flip_y) uv.y = 1.0-uv.y;

    v_uv = uv;
    gl_Position = vec4(V[gl_VertexID],0,1);
}
