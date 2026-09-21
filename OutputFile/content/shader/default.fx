#ifndef _DEFAULT
#define _DEFAULT


// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 2
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif

// Include structures and functions for lighting.
#include "LightingUtils.fx"

Texture2D g_DiffuseMap : register(t0);

SamplerState g_SamPointWrap : register(s0);
SamplerState g_SamPointClamp : register(s1);
SamplerState g_SamLinearWrap : register(s2);
SamplerState g_SamLinearClamp : register(s3);
SamplerState g_SamAnisotropicWrap : register(s4);
SamplerState g_SamAnisotropicClamp : register(s5);

cbuffer TRANSFORM : register(b0)
{
    row_major matrix g_World;
    row_major matrix g_WorldInvTranspose;
    row_major matrix g_ViewProj;
    row_major matrix g_TexTransform;
}

cbuffer MATERIAL : register(b1)
{
    float4 g_DiffuseAlbedo;
    float3 g_FresnelR0;
    float g_Roughness;
    int g_bUseTexture;
    float3 padding_Material;
    
    row_major matrix g_MtrlTransform;
}

cbuffer GLOBAL : register(b2)
{
    float3 g_EyePosW;
    float padding_Global;
    float4 g_AmbientLight;
    Light g_Lights[MaxLights];
}

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexCoord : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.f;
    
    float4 posW = mul(float4(vin.PosL, 1.f), g_World);
    vout.PosW = posW.xyz;
    
    vout.NormalW = mul(vin.NormalL, (float3x3) g_WorldInvTranspose);
    
    vout.PosH = mul(posW, g_ViewProj);
    
    // Output vertex attributes for interpolation across triangle.
    float4 texCoord = mul(float4(vin.TexCoord, 0.0f, 1.0f), g_TexTransform);
    vout.TexCoord = mul(texCoord, g_MtrlTransform).xy;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    float4 diffuseAlbedo = g_bUseTexture ? 
        (g_DiffuseMap.Sample(g_SamAnisotropicWrap, pin.TexCoord) * g_DiffuseAlbedo) 
        : g_DiffuseAlbedo;
    
    pin.NormalW = normalize(pin.NormalW);
    
    float3 toEyeW = normalize(g_EyePosW - pin.PosW);
    
    // indirect light
    float4 ambient = g_AmbientLight * diffuseAlbedo;
    
    // direct light
    const float shininess = 1.f - g_Roughness;
    Material mat = { diffuseAlbedo, g_FresnelR0, shininess };
    float3 shadowFactor = 1.f;
    float4 directLight = ComputeLighting(g_Lights, mat, pin.PosW, pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;
    
    litColor.a = diffuseAlbedo.a;
    
    return litColor;
}

#endif