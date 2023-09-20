#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2DArray arrayTexture;

void main() {
	FragColor = vec4(texture(arrayTexture, vec3(TexCoord, 2)));
}