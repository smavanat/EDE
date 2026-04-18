#version 430

layout(local_size_x=8, local_size_y=8) in;

layout(rgba32f, binding = 0) uniform image2D grid;

uniform vec2 brushPos;
uniform float radius;

void main() {
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
    float d = distance(vec2(coord), brushPos);

    if(d < radius) {
        imageStore(grid, coord, vec4(1,0,0,1));
    }
}
