#version 410 core

layout (location = 0) in uvec2 data;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main() {
    float x = float(data.x & 255u);
	float y = float((data.x >> 8) & 255u);
	float z = float((data.x >> 16) & 255u);

    gl_Position = lightSpaceMatrix * model * vec4(x, y, z, 1.0);
}  