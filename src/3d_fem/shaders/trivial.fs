#version 330 core
in vec3 Normal;
in vec3 WorldPos;
in vec2 TexCoords;

uniform vec3  albedo;

out vec4 color;

void main()
{
    
        
    //vec3 rgb = Normal;
    //vec3 rgb = WorldPos;
    //rgb = vec3(1.0, 0.8,0.3) * dot(Normal,vec3(1.0) + WorldPos);
    //color = vec4(ambient + 3*Lo, 1.0);
    color = vec4(albedo, 1.0);
}

