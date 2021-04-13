#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
   gl_Position = projection * view * model * vec4(aPos, 1.0f);
   WorldPos = (model * vec4(aPos, 1.0f)).xyz;
   //Normal = mat3(transpose(inverse(model))) * aNormal;
   Normal =  aNormal;
   TexCoords = aTexCoords;
}