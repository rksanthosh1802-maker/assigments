#version 300 es
layout(location = 0) in vec4 VertexPosition;
layout(location = 1) in vec4 VertexColor;
out vec4 VarColor;

void main()
{
    gl_Position = VertexPosition;
    VarColor = VertexColor;
}
