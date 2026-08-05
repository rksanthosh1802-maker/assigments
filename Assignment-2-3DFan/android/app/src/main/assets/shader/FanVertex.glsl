#version 300 es

layout(location = 0) in vec4  VertexPosition;
layout(location = 1) in float VertexShade;

uniform mat4 MODELVIEWPROJECTIONMATRIX;

out float Shade;

void main()
{
    gl_Position = MODELVIEWPROJECTIONMATRIX * VertexPosition;
    Shade       = VertexShade;
}