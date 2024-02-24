#version 410 core

layout (location = 0) in uint xyz_type;
layout (location = 1) in uint uv;
layout (location = 2) in uint norm_ao;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 frag_viewspace;
out vec3 frag_pos;
out vec3 frag_normal;
out vec2 frag_uv;
out float frag_ao;
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
  float x = float(xyz_type & 255u);
  float y = float((xyz_type >> 8) & 255u);
  float z = float((xyz_type >> 16) & 255u);
  uint type = (xyz_type >> 24) & 255u;

  uint u = (uv & 255u);
  uint v = (uv >> 8) & 255u;

  uint norm = (norm_ao & 31u);
  uint ao = (norm_ao >> 5) & 3u;
  
  frag_ao = clamp(float(ao) / 3.0, 0.5, 1.0);

  frag_pos = vec3(x, y, z);
  frag_viewspace = view * model * vec4(frag_pos, 1);
  frag_normal = NORMALS[norm];
  frag_uv = vec2(u, v);
  frag_type = type;
  
  gl_Position = projection * frag_viewspace;
}