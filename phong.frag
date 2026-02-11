#version 330 core
out vec4 FragColor;

in vec3 vFragPos;
in vec3 vNormal;

struct Light
{
    vec3 pos;
    vec3 kA;
    vec3 kD;
    vec3 kS;
};

uniform Light uLight;
uniform vec3 uViewPos;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uViewPos - vFragPos);
    vec3 L = normalize(uLight.pos - vFragPos);

    float diff = max(dot(N, L), 0.0);

    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V, R), 0.0), 32.0);

    vec3 ambient  = uLight.kA;
    vec3 diffuse  = uLight.kD * diff;
    vec3 specular = uLight.kS * spec * 0.4;

    vec3 color = ambient + diffuse + specular;

    FragColor = vec4(color, 1.0);
}
