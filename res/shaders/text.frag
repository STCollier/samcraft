#version 410 core

in vec2 TexCoords;
in vec4 TextColor;

uniform sampler2D text;

out vec4 color;

void main() {    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = TextColor * sampled;
}