#version 330 core
out vec4 FragColor;

in vec3 texCoords;

uniform samplerCube skybox;
uniform float darkeningFactor;

void main()
{    
    FragColor = darkeningFactor*texture(skybox, texCoords);
}