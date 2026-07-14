#version 300 es
precision mediump float;

in float Shade;

uniform vec3 PARTCOLOR;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(PARTCOLOR * Shade, 1.0);
}