#version 330 core

in vec3 vertexColor;
in vec2 TexCoord;
uniform sampler2D texture1;
out vec4 FragColor;

void main()
{    
    FragColor = texture(texture1, TexCoord) * vec4(vertexColor, 1.0);
}