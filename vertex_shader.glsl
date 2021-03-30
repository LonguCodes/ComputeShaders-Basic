#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 vColor;
out vec3 color;

void main()
{
    gl_Position = vec4(pos,1.0);
    color = vec3(1.0,1.0,1.0);
    color = vColor;
}