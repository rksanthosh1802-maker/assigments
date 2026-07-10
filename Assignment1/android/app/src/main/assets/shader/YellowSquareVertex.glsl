#version 300 es
layout(location = 0) in vec4 VertexPosition;

void main()
{
    gl_Position = VertexPosition;
}
