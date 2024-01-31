#version 410 core

out vec4 FragColor;

in vec2 TexCoord;
in float TexIndex;
in vec3 Normal;
flat in uint NormalIndex;
in vec3 FragPos;
uniform sampler2DArray arrayTexture;


void main() {
  vec3 lightValue;

  switch (NormalIndex) {
    case 0u:
      lightValue = vec3(0.86f); // Back
      break;
    case 1u:
      lightValue = vec3(0.86f); // Front
      break; 
    case 2u:
      lightValue = vec3(0.8f); // Left
      break; 
    case 3u:
      lightValue = vec3(0.8f); // Right
      break;
    case 4u:
      lightValue = vec3(0.65f); // Bottom
      break;
    case 5u:
      lightValue = vec3(1.0f); // Top
      break;
  }

	FragColor = vec4(texture(arrayTexture, vec3(TexCoord, TexIndex)).xyz * lightValue, 1.0f);
}