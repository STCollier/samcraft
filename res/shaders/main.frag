#version 410 core

out vec4 frag_color;

in vec4 frag_viewspace;
in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_uv;
in float frag_ao;
flat in uint frag_type;

uniform vec3 eye_position;
uniform sampler2DArray arrayTexture;

void main() {
  vec3 final_color = vec3(frag_uv, 0.0);

  /*float ao = clamp(frag_ao, 0.0, 1.0);
  final_color *= 1.0 * smoothstep(0.0, 1.0, ao);*/

  //frag_color = vec4(texture(arrayTexture, vec3(frag_uv, frag_type)).xyz, 1.0f);
  frag_color = vec4(final_color, 1.0);
}