#version 410 core

layout (location = 0) in uvec2 data;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 chunk_translation;
uniform mat4 light_space_matrix;
uniform vec3 sun_position;
uniform vec3 camera_position;

struct TangentPos {
	vec3 light, view, frag;
};

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

	TangentPos tangent_pos;
} vs_out;

vec3 NORMALS[6] = vec3[6](
  vec3( 0, 1, 0 ), // Top
  vec3(0, -1, 0 ), // Bottom
  vec3( 1, 0, 0 ), // Right
  vec3( -1, 0, 0 ), // Left
  vec3( 0, 0, 1 ), // Front
  vec3( 0, 0, -1 )	 // Back
);

vec3 BITANGENTS[6] = vec3[6](
	vec3(0, 0, -1),
	vec3(0, 0, 1),
	vec3(0, 0, 1),
	vec3(0, 0, -1),
	vec3(-1, 0, 0),
	vec3(1, 0, 0)
);

vec3 TANGENTS[6] = vec3[6](
	vec3(1, 0, 0),
	vec3(-1, 0, 0),
	vec3(0, 1, 0),
	vec3(0, 1, 0),
	vec3(0, 1, 0),
	vec3(0, 1, 0)
);

void main() {
	float x = float(data.x & 255u);
	float y = float((data.x >> 8) & 255u);
	float z = float((data.x >> 16) & 255u);
	uint u = (data.x >> 24) & 255u;

	uint type = data.y & 262143u;
	uint v = (data.y >> 18) & 255u;
	uint norm = (data.y >> 26) & 7u;
	uint ao = (data.y >> 29) & 3u;
	uint opaque = (data.y >> 31) & 1u;
  
	vs_out.frag_ao = clamp(float(ao) / 3.0, 0.5, 1.0);
	vs_out.frag_pos = vec3(chunk_translation + vec3(x, y, z));
	vs_out.frag_viewspace = view * vec4(vs_out.frag_pos, 1);
	vs_out.frag_normal = NORMALS[norm];
	vs_out.frag_uv = vec2(u, v);
	vs_out.frag_opaque = opaque;
	vs_out.frag_type = type;

	vs_out.sun_position = sun_position;
	vs_out.camera_position = camera_position;

	/*mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * TANGENTS[norm]);
    vec3 N = normalize(normalMatrix * vs_out.frag_normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
	mat3 TBN = transpose(mat3(T, B, N));

	vs_out.tangent_pos.light = TBN * sun_position;
	vs_out.tangent_pos.view = TBN * camera_position;
	vs_out.tangent_pos.frag = TBN * vs_out.frag_pos;*/

	vs_out.frag_light_space = light_space_matrix * vec4(vs_out.frag_pos, 1.0);
	
	gl_Position = projection * vs_out.frag_viewspace;
}