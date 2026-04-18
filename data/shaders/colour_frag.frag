#version 330 core

in vec2 uv;
out vec4 FragColour;

uniform sampler2D screenTex;

void main() {
    FragColour = texture(screenTex, uv);
}
