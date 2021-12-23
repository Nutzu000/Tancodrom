#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNor;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aCol;

out vec3 vertexColor;
out vec2 TexCoord;

uniform mat4 ProjMatrix;
uniform mat4 ViewMatrix;
uniform mat4 WorldMatrix;

void main()
{
    gl_Position = ProjMatrix * ViewMatrix * WorldMatrix * vec4(aPos, 1.0f);
    vertexColor = aCol;
    TexCoord = vec2(aTex.x, aTex.y);
} 