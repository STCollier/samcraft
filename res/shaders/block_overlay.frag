#version 410 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2DArray blockBreakTexture;
uniform int breakState;

void main() {
    vec4 final = texture(blockBreakTexture, vec3(TexCoord, breakState));
    if (final.a > 0.) final.a = 0.75;

	FragColor = final;
}