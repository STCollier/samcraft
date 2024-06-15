#version 410 core

layout (location = 0) in uvec2 data;

layout (std140) uniform CameraMatrix {
    mat4 projection_view;
};

uniform mat4 model;
/*uniform mat4 view;
uniform mat4 projection;*/

out vec3 frag_pos;
out vec3 frag_normal;
out vec2 frag_uv;
out float frag_ao;
flat out uint frag_opaque;
flat out uint frag_type;

vec3 NORMALS[6] = vec3[6](
  vec3( 0, 1, 0 ),
  vec3(0, -1, 0 ),
  vec3( 1, 0, 0 ),
  vec3( -1, 0, 0 ),
  vec3( 0, 0, 1 ),
  vec3( 0, 0, -1 )
);

void main() {
	uint x = data.x & 255u;
	uint y = (data.x >> 8) & 255u;
	uint z = (data.x >> 16) & 255u;
	uint u = (data.x >> 24) & 255u;

	uint type = data.y & 262143u;
	uint v = (data.y >> 18) & 255u;
	uint norm = (data.y >> 26) & 7u;
	uint ao = (data.y >> 29) & 3u;
	uint opaque = (data.y >> 31) & 1u;
  
	frag_ao = clamp(float(ao) * 0.33, 0.5, 1.0);

	frag_pos = vec3(x, y, z);
	frag_normal = NORMALS[norm];
	frag_uv = vec2(u, v);
	frag_opaque = opaque;
	frag_type = type;
	
	gl_Position = projection_view * vec4(frag_pos, 1);
}