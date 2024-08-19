#version 410 core

layout (location = 0) in uvec2 data;

uniform mat4 lightSpaceMatrix;
uniform vec3 chunk_translation;

void main() {
    float x = float(data.x & 255u);
	float y = float((data.x >> 8) & 255u);
	float z = float((data.x >> 16) & 255u);

    vec4 position = vec4(vec3(x, y, z) + chunk_translation, 1.0);

    gl_Position = lightSpaceMatrix * position;
}