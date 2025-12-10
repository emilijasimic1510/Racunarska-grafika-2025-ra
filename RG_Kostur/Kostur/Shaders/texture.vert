#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;

uniform vec2 uPos;      // centar
uniform vec2 uScale;    // širina/visina

out vec2 TexCoord;

void main() {
    vec2 pos = aPos * uScale + uPos;
    gl_Position = vec4(pos, 0.0, 1.0);
    TexCoord = aTex;
}

