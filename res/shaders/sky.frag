#version 410 core

out vec4 FragColor;
in vec2 t; // TexCoords if ever needed

uniform vec2 viewport_size;
uniform vec3 sun_position;
uniform mat4 inv_view_projection;
uniform mat4 view;

void main() {

    vec4 fog_color = vec4(0.4, 0.4, 0.4, 1.0);

    FragColor = fog_color;
}