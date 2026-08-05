#version 300 es
precision mediump float;

in vec3 EyePosition;
in vec3 EyeNormal;

uniform vec3 PARTCOLOR;
uniform vec3 LIGHTPOS_EYE;   // light position, already transformed into eye space by Scene3D
uniform vec3 LIGHTCOLOR;

layout(location = 0) out vec4 FragColor;

void main()
{
    vec3 N = normalize(EyeNormal);
    vec3 L = normalize(LIGHTPOS_EYE - EyePosition);
    vec3 V = normalize(-EyePosition);   // eye is at the origin in eye space
    vec3 R = reflect(-L, N);

    float ambientStrength = 0.15;
    vec3 ambient = ambientStrength * LIGHTCOLOR;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * LIGHTCOLOR;

    float specularStrength = 0.6;
    float shininess = 32.0;
    float spec = pow(max(dot(V, R), 0.0), shininess);
    vec3 specular = specularStrength * spec * LIGHTCOLOR;

    vec3 result = (ambient + diffuse) * PARTCOLOR + specular;
    FragColor = vec4(result, 1.0);
}
