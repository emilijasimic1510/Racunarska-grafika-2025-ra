#version 330 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 inColor;   

uniform vec2 uPos;
uniform vec2 uScale;

void main()
{
    vec2 pos = inPos * uScale + uPos;
    gl_Position = vec4(pos, 0.0, 1.0);
}
