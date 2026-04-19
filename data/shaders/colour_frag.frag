#version 430
in vec2 v_uv;
out vec4 fc;

uniform sampler2D canvas_tex;

void main(){
    fc = texture(canvas_tex, v_uv);
}
