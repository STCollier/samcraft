#version 410 core

layout (location = 0) in uint vertexData;

out vec2 TexCoord;
out float TexIndex;
out vec3 Normal;
flat out uint NormalIndex;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

vec3 normals[6] = vec3[6](
	vec3(0.0f, 0.0f, -1.0f), // Back
	vec3(0.0f, 0.0f, 1.0f),  // Front
	vec3(1.0f, 0.0f, 0.0f),  // Left
	vec3(-1.0f, 0.0f, 0.0f), // Right
	vec3(0.0f, -1.0f, 0.0f), // Bottom
	vec3(0.0f, 1.0f, 0.0f)   // Top
);

void main() {
	// Positions
	uint x = (vertexData >> 0) & 63u;
	uint y = (vertexData >> 6) & 63u;
	uint z = (vertexData >> 12) & 63u;

	// Texture Coords
	uint u = (vertexData >> 18) & 1u;
	uint v = (vertexData >> 19) & 1u;

	// Array Texture Index
	uint i = (vertexData >> 20) & 255u;

	// Normal Index	
	uint n = (vertexData >> 28) & 15u;

	vec3 aPos = vec3(x, y, z);
	vec2 aTexCoord = vec2(u, v);
	uint aTexIndex = i;
	vec3 aNormal = normals[n];

	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	FragPos = vec3(model * vec4(aPos, 1.0f));
	TexCoord = aTexCoord;
	TexIndex = aTexIndex;
	Normal = aNormal;
	NormalIndex = n;
}