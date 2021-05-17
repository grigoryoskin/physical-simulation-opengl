#version 330 core
in vec3 Normal;
in vec3 WorldPos;
in vec2 TexCoords;


out vec4 color;

struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};
#define PI 3.14159265359

#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform vec3 camPos;
uniform vec3  albedo;
uniform float metallic;
uniform float roughness;

float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
    
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
    
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return nom / denom;
}
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
    
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    vec3 Lo = vec3(0.0);
    vec3 N = normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);

    float NdotV  = max(dot(N, V), 0.0);

    for(int i = 0; i < NR_POINT_LIGHTS; ++i)
    {
        vec3 L = normalize(pointLights[i].position - WorldPos);
        vec3 H = normalize(V + L);
        
        float NdotH  = max(dot(N, H), 0.0);
        float NdotL  = max(dot(N, L), 0.0);
        
        float D = DistributionGGX(N, H, roughness);
        
        float k = (roughness + 1)*(roughness + 1)/8;
        float G = GeometrySmith(N, V, L, k);
        
        vec3 F0 = vec3(0.04);
        F0 = mix(F0, albedo, metallic);
        vec3 F = fresnelSchlick(NdotH, F0);
        
        vec3 numerator    = F * D * G;
        float denominator = 4.0 * NdotV * NdotL;
        vec3 specular     = numerator / max(denominator, 0.001);
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
          
        
        kD *= 1.0 - metallic;
        
        Lo += (kD * albedo / PI + specular) * NdotL;
    }
    
    //vec3 rgb = Normal;
    //vec3 rgb = WorldPos;
    //rgb = vec3(1.0, 0.8,0.3) * dot(Normal,vec3(1.0) + WorldPos);
    vec3 ambient = 0.7 * albedo;
    //color = vec4(ambient + 3*Lo, 1.0);
    Lo *= 3;
    //Lo = Lo / (Lo + vec3(1.0));
    //Lo = pow(Lo, vec3(1.0/2.2));

    color = vec4(Lo, 1.0);
}
