#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;

out vec3 ourColor;
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 ProjMatrix;
uniform mat4 ViewMatrix;
uniform mat4 WorldMatrix;

void main()
{
   FragPos = vec3(WorldMatrix * vec4(aPos, 1.0));
   Normal = mat3(transpose(inverse(WorldMatrix))) * aNormal;

   ourColor = aColor;

   gl_Position = ProjMatrix * ViewMatrix * WorldMatrix * vec4(aPos, 1.0);
   TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}