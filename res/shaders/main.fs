#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in float TexIndex;
in vec3 Normal;
uniform sampler2DArray arrayTexture;

void main() {
	//FragColor = vec4(texture(arrayTexture, vec3(TexCoord, TexIndex)));
	FragColor = vec4(vec3((Normal + 1) / 2), 1.0f);
}