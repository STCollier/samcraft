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
  vec3 final = texture(arrayTexture, vec3(frag_uv, frag_type)).xyz;

  float ao = clamp(frag_ao, 0.0, 1.0);
  final *= float(15.0 / 16.0) * smoothstep(0.0, 1.0, ao);

  frag_color = vec4(final, 1.0);
}