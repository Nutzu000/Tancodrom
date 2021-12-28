#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
  
uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform float Ka;
uniform float Kd;
uniform float Ks;
uniform float Kspec;
void main()
{
    vec3 ambient = lightColor * Ka;

    vec3 LightDir = normalize(lightPos - FragPos);
    vec3 diffuse = lightColor * Kd * max(dot(Normal, LightDir), 0.0);

    float specFactor = pow(max(dot(viewPos, LightDir), 0.0), Kspec);
    vec3 specular = Ks * specFactor * lightColor;

    FragColor = vec4(ambient + diffuse + specular, 1.0) * vec4(objectColor, 1.0);
}  