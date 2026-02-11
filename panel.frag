#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D uTex;

void main()
{
    vec2 uv = vec2(TexCoord.x, 1.0 - TexCoord.y);
    FragColor = texture(uTex, uv);
}