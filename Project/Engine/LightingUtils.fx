#ifndef _LIGHTING_UTILS
#define _LIGHTING_UTILS

#define MaxLights 16

struct Light
{
    float3 Strength;
    float FalloffStart;
    float3 Direction;
    float FalloffEnd;
    float3 Position;
    float SpotPower;
};

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    float cosIncidentAngle = saturate(dot(normal, lightVec));
    
    float f0 = 1.f - cosIncidentAngle;
    return R0 + (1.f - R0) * (f0 * f0 * f0 * f0 * f0);
}

struct Material
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float Shininess;
};

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye, Material mat)
{
    const float m = mat.Shininess * 256.f;
    float3 halfVec = normalize(toEye + lightVec);
    
    float ks = pow(max(dot(halfVec, normal), 0.f), m);
    //if (ks <= 0.1f)
    //    ks = 0.f;
    //else if (ks <= 0.8f)
    //    ks = 0.5f;
    //else
    //    ks = 0.8f;
    
    float roughnessFactor = (m + 8.f) * ks / 8.f;
    float3 fresnelFactor = SchlickFresnel(mat.FresnelR0, halfVec, lightVec);
    float3 specAlbedo = fresnelFactor * roughnessFactor;
    specAlbedo = specAlbedo / (specAlbedo + 1.f); // tone mapping
    
    return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

float3 ComputeDirectionalLight(Light L, Material mat, float3 normal, float3 toEye)
{
    float3 lightVec = -L.Direction;
    
    float kd = max(dot(lightVec, normal), 0.f);
    //if (kd <= 0.f)
    //    kd = 0.4f;
    //else if (kd <= 0.5f)
    //    kd = 0.6f;
    //else
    //    kd = 1.f;
    
    float3 lightStrength = L.Strength * kd;
    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    float3 lightVec = L.Position - pos;
    float d = length(lightVec);
    if (d > L.FalloffEnd)
        return 0.f;
    
    lightVec /= d;
    float3 lightStrength = L.Strength * max(dot(lightVec, normal), 0.f);
    lightStrength *= CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    
    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    float3 lightVec = L.Position - pos;
    float d = length(lightVec);
    if (d > L.FalloffEnd)
        return 0.f;
    
    lightVec /= d;
    float3 lightStrength = L.Strength * max(dot(lightVec, normal), 0.f);
    lightStrength *= CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    
    float spotFactor = pow(max(dot(-lightVec, L.Direction), 0.f), L.SpotPower);
    lightStrength *= spotFactor;
    
    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float4 ComputeLighting(Light gLights[MaxLights], Material mat,
                       float3 pos, float3 normal, float3 toEye,
                       float3 shadowFactor)
{
    float3 result = 0.0f;

    int i = 0;

#if (NUM_DIR_LIGHTS > 0)
    for(i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
        result += shadowFactor[i] * ComputeDirectionalLight(gLights[i], mat, normal, toEye);
    }
#endif

#if (NUM_POINT_LIGHTS > 0)
    for(i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS+NUM_POINT_LIGHTS; ++i)
    {
        result += ComputePointLight(gLights[i], mat, pos, normal, toEye);
    }
#endif

#if (NUM_SPOT_LIGHTS > 0)
    for(i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; ++i)
    {
        result += ComputeSpotLight(gLights[i], mat, pos, normal, toEye);
    }
#endif 

    return float4(result, 0.0f);
}



#endif