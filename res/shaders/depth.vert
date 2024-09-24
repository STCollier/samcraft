#version 410 core

layout (location = 0) in uvec2 data;

uniform mat4 lightSpaceMatrix;
uniform vec3 chunk_translation;

void main() {
    float x = float(data.x & 63u);
	float y = float((data.x >> 6) & 63u);
	float z = float((data.x >> 12) & 63u);

    vec4 position = vec4(vec3(x, y, z) + chunk_translation, 1.0);

    gl_Position = lightSpaceMatrix * position;
}