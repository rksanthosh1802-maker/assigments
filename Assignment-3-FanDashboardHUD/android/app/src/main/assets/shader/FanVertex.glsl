#version 300 es

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;

uniform mat4 MODELVIEWPROJECTIONMATRIX;
uniform mat4 MODELVIEWMATRIX;
uniform mat3 NORMALMATRIX;   // inverse-transpose of (View*Model) upper 3x3 (Part 2 gotcha)

out vec3 EyePosition;
out vec3 EyeNormal;

void main()
{
    vec4 eyePos4 = MODELVIEWMATRIX * vec4(VertexPosition, 1.0);
    EyePosition  = eyePos4.xyz;

    // Transform the normal with the normal matrix, NOT the model matrix
    // directly - otherwise non-uniform scale (every stretched "bone" in the
    // fan hierarchy) skews it, and lighting goes subtly wrong once the fan
    // spins even though it looked fine standing still.
    EyeNormal = normalize(NORMALMATRIX * VertexNormal);

    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(VertexPosition, 1.0);
}
