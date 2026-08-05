#version 300 es

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;

uniform mat4 MODELVIEWPROJECTIONMATRIX;
uniform mat4 MODELVIEWMATRIX;
uniform mat3 NORMALMATRIX;

out vec3 EyePosition;
out vec3 EyeNormal;
out vec2 ObjectXZ;   // object-space XZ, used by the fragment shader's checker() (Part 3)

void main()
{
    vec4 eyePos4 = MODELVIEWMATRIX * vec4(VertexPosition, 1.0);
    EyePosition  = eyePos4.xyz;
    EyeNormal    = normalize(NORMALMATRIX * VertexNormal);
    ObjectXZ     = VertexPosition.xz;

    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(VertexPosition, 1.0);
}
