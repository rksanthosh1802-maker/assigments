#version 300 es
precision mediump float;

in vec3 EyePosition;
in vec3 EyeNormal;
in vec2 ObjectXZ;

uniform vec3 LIGHTPOS_EYE;
uniform vec3 LIGHTCOLOR;

layout(location = 0) out vec4 FragColor;

// Same cell-parity technique the handout's Part 3 snippet describes:
// floor(objectXZ / tileSize), then mod(cell.x + cell.y, 2.0) picks one of
// two colours. Repeats with mod() so a single finite quad still reads as an
// infinite tiled floor.
float checker(vec2 objectXZ, float tileSize)
{
    vec2 cell = floor(objectXZ / tileSize);
    float parity = mod(cell.x + cell.y, 2.0);
    return parity;
}

void main()
{
    vec3 colorA = vec3(0.9, 0.9, 0.9);
    vec3 colorB = vec3(0.15, 0.15, 0.15);
    float t = checker(ObjectXZ, 1.0);
    vec3 materialColor = mix(colorA, colorB, t);

    vec3 N = normalize(EyeNormal);
    vec3 L = normalize(LIGHTPOS_EYE - EyePosition);
    vec3 V = normalize(-EyePosition);
    vec3 R = reflect(-L, N);

    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * LIGHTCOLOR;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * LIGHTCOLOR;

    float specularStrength = 0.25;
    float shininess = 16.0;
    float spec = pow(max(dot(V, R), 0.0), shininess);
    vec3 specular = specularStrength * spec * LIGHTCOLOR;

    vec3 result = (ambient + diffuse) * materialColor + specular;
    FragColor = vec4(result, 1.0);
}
