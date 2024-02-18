#version 410 core

layout (location = 0) in uint data;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 frag_viewspace;
out vec3 frag_pos;
out vec3 frag_normal;
out float frag_ao;
flat out float frag_light;
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
  float x = float(data & 63u);
  float y = float((data >> 6) & 63u);
  float z = float((data >> 12) & 63u);
  uint type = (data >> 18) & 31u;
  uint light = (data >> 23) & 15u;
  uint norm = (data >> 27) & 7u;
  uint ao = (data >> 30) & 3u;
  
  frag_ao = clamp(float(ao) / 3.0, 0.5, 1.0);

  frag_pos = vec3(x, y, z) - vec3(0.5);
  frag_viewspace = view * model * vec4(frag_pos, 1);
  frag_normal = NORMALS[norm];
  frag_light = float(light) / 16.0;
  frag_type = type;
  
  gl_Position = projection * frag_viewspace;
}