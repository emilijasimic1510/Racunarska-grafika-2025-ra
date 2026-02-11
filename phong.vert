#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 vFragPos;
out vec3 vNormal;

uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;

void main()
{
    vec4 worldPos = uM * vec4(aPos, 1.0);
    vFragPos = worldPos.xyz;

    vNormal = mat3(transpose(inverse(uM))) * aNormal;

    gl_Position = uP * uV * worldPos;
}
