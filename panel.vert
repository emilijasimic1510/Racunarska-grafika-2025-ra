#version 330 core

layout (location = 0) in vec3 aPos;

out vec2 TexCoord;

uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;

void main()
{
    gl_Position = uP * uV * uM * vec4(aPos, 1.0);

    // mapiranje kocke na 2D teksturu
    TexCoord = aPos.xy + vec2(0.5);
}
