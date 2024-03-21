#version 410 core

out vec4 frag_color;

in vec4 frag_viewspace;
in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_uv;
in float frag_ao;
flat in uint frag_type;
flat in uint frag_opaque;

uniform sampler2DArray arrayTexture;
uniform vec3 camera_position;
uniform vec3 camera_direction;

float fog_maxdist = 400.0;
float fog_mindist = 100.0;
vec4 fog_colour = vec4(0.4, 0.4, 0.4, 1.0);

void main() {
  bool isOpaque = bool(frag_opaque);
  vec4 final = texture(arrayTexture, vec3(frag_uv, frag_type));

  // Calculate fog
  float dist = length(frag_viewspace.xyz);
  float fog_factor = (fog_maxdist - dist) / (fog_maxdist - fog_mindist);
  fog_factor = clamp(fog_factor, 0.0, 1.0);

  if (isOpaque) {
    float ao = clamp(frag_ao, 0.0, 1.0);
    final *= smoothstep(0.0, 1.0, ao);

    frag_color = vec4(mix(fog_colour, final, fog_factor).xyz, 1.0);
  } else {
    frag_color = mix(fog_colour, final, fog_factor);
  }
}