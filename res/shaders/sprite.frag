#version 410 core

in vec2 SpriteUV;
in vec4 SpriteColor;

uniform sampler2D spriteAtlas;

out vec4 color;

void main() {    
    vec4 result = texture(spriteAtlas, SpriteUV);
    color = result * SpriteColor;
}