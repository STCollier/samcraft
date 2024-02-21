#version 410 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;

void main() {
    vec4 texColor = texture(image, TexCoords);

    if (texColor.a < 0.1)
        discard;

    color = texColor;
}