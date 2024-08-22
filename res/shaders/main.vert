#version 410 core

layout (location = 0) in uvec2 data;

struct MaterialIndices {
    int diffuseIndex, normalIndex;
    int reserved01, reserved02;
};

layout (std140) uniform BlockData {
	MaterialIndices materialIndices[256 * 6];
};

uniform mat4 view;
uniform mat4 projection;
uniform vec3 chunk_translation;
uniform mat4 light_space_matrix;
uniform vec3 sun_position;
uniform vec3 camera_position;

out VS_OUT {
	vec4 frag_viewspace;
	vec4 frag_light_space;
	vec3 frag_pos;
	vec3 frag_normal;
	vec2 frag_uv;
	flat uint frag_opaque;
	flat uint frag_type;
	float frag_ao;

	vec3 sun_position;
	vec3 camera_position;

	flat uint diffuse_index;
	flat uint normal_index;

	mat3 TBN;
} vs_out;

const vec3 NORMALS[6] = vec3[6](
  vec3( 0, 1, 0 ), // Top
  vec3(0, -1, 0 ), // Bottom
  vec3( 1, 0, 0 ), // Right
  vec3( -1, 0, 0 ), // Left
  vec3( 0, 0, 1 ), // Front
  vec3( 0, 0, -1 )	 // Back
);

const vec3 TANGENTS[6] = vec3[6](
	vec3(0, 0, 1),
	vec3(0, 0, -1),
	vec3(0, 1, 0),
	vec3(0, 1, 0),
	vec3(0, 1, 0),
	vec3(0, 1, 0)
);

const vec3 BITANGENTS[6] = vec3[6](
	vec3(1, 1, 0),
	vec3(-1, 0, 0),
	vec3(0, 0, 1),
	vec3(0, 0, -1),
	vec3(1, 0, 0),
	vec3(-1, 0, 0)
);

const mat3 TBNS[6] = mat3[6](
	mat3(TANGENTS[0], TANGENTS[0], NORMALS[0]),
	mat3(TANGENTS[1], TANGENTS[1], NORMALS[1]),
	mat3(TANGENTS[2], TANGENTS[2], NORMALS[2]),
	mat3(TANGENTS[3], TANGENTS[3], NORMALS[3]),
	mat3(TANGENTS[4], TANGENTS[4], NORMALS[4]),
	mat3(TANGENTS[5], TANGENTS[5], NORMALS[5])
);

void main() {
	/*float x = float(bitfieldExtract(data.x, 0, 6));
	float y = float(bitfieldExtract(data.x, 6, 6));
	float z = float(bitfieldExtract(data.x, 12, 6));
	uint u = bitfieldExtract(data.x, 18, 6);
	uint v = bitfieldExtract(data.x, 24, 6);
	uint ao = bitfieldExtract(data.x, 30, 2);

	uint face = bitfieldExtract(data.y, 0, 3); 
	uint rotation = bitfieldExtract(data.y, 3, 5);
	uint opaque = bitfieldExtract(data.y, 8, 1);
	uint light = bitfieldExtract(data.y, 9, 5);
	uint type = bitfieldExtract(data.y, 14, 18);*/

    float x = float(data.x & 63u);
    float y = float((data.x >> 6) & 63u);
    float z = float((data.x >> 12) & 63u);
	uint u = (data.x >> 18) & 63u;
	uint v = (data.x >> 24) & 63u; 
	uint ao = (data.x >> 30) & 3u;

	uint face = data.y & 7u; 
	uint rotation = (data.y >> 3) & 31u;
	uint opaque = (data.y >> 8) & 1u;
	uint light = (data.y >> 9) & 31u;
	uint type = (data.y >> 14) & 262143u;

	vs_out.frag_ao = clamp(float(ao) / 3.0, 0.5, 1.0);
	vs_out.frag_pos = vec3(chunk_translation + vec3(x, y, z));
	vs_out.frag_viewspace = view * vec4(vs_out.frag_pos, 1);
	vs_out.frag_normal = NORMALS[face];
	vs_out.frag_uv = vec2(u, v);
	vs_out.frag_opaque = opaque;
	vs_out.frag_type = type;

	uint materialIndex = 6 * type + face;
	vs_out.diffuse_index = materialIndices[materialIndex].diffuseIndex;
	vs_out.normal_index = materialIndices[materialIndex].normalIndex;

	vs_out.sun_position = sun_position;
	vs_out.camera_position = camera_position;

	vs_out.TBN = TBNS[face];

	vs_out.frag_light_space = light_space_matrix * vec4(vs_out.frag_pos, 1.0);
	
	gl_Position = projection * vs_out.frag_viewspace;
}