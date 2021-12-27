#version 330 core
out vec4 FragColor;

in vec3 texCoords;

uniform samplerCube skybox;
uniform float cringe;

void main()
{    
    FragColor = cringe*texture(skybox, texCoords);
}