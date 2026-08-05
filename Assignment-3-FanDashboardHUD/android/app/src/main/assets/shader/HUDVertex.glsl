#version 300 es

layout(location = 0) in vec2 VertexPosition; // pixel coords, top-left origin
layout(location = 1) in vec3 VertexColor;

uniform mat4 PROJECTIONMATRIX; // orthographic, built with TransformOrtho(0,w,h,0,-1,1)

out vec3 Color;

void main()
{
    Color = VertexColor;
    gl_Position = PROJECTIONMATRIX * vec4(VertexPosition, 0.0, 1.0);
}
