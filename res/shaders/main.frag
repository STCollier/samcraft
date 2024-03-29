#version 410 core

out vec4 frag_color;

in vec4 frag_viewspace;
in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_uv;
in float frag_ao;
flat in uint frag_type;
flat in uint frag_opaque;

uniform float fog_min;
uniform float fog_max;

uniform sampler2DArray arrayTexture;
uniform vec3 camera_position;
uniform vec3 camera_direction;

vec4 fog_color = vec4(0.588, 0.784, 1.0, 1.0);

vec4 layer(vec4 foreground, vec4 background) {
  return foreground * foreground.a + background * (1.0 - foreground.a);
}

void main() {
  bool isOpaque = bool(frag_opaque);
  vec4 final = texture(arrayTexture, vec3(frag_uv, frag_type));
  //vec4 breakTexture = texture(blockBreakTexture, vec3(frag_uv, 2)) * vec4(1.0, 1.0, 1.0, 0.8);

  // Calculate fog
  float dist = length(frag_viewspace.xyz);
  float fog_factor = (fog_max - dist) / (fog_max - fog_min);
  fog_factor = clamp(fog_factor, 0.0, 1.0);

  if (isOpaque) {
    float ao = clamp(frag_ao, 0.0, 1.0);
    final *= smoothstep(0.0, 1.0, ao);

    /*layer(breakTexture, vec4(final.xyz, 1.0))*/

    frag_color = vec4(mix(fog_color, final, fog_factor).xyz, 1.0);
  } else {
    frag_color = mix(fog_color, final, fog_factor);
  }
}