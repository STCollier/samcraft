#version 410 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2DArray fontArrayTexture;
uniform int char;

void main() {
    vec4 texColor = texture(fontArrayTexture, vec3(TexCoords, char)) * vec4(vec3(0.0), 1.0);
    color = texColor;
}