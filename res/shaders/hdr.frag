#version 410 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform bool hdr;
uniform float exposure;

void main() {
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    
    if (hdr) {
        // reinhard
        vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
        FragColor = vec4(result, 1.0);
    }
    else {
        vec3 result = hdrColor;
        FragColor = vec4(result, 1.0);
    }
}