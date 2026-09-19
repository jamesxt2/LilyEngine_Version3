#ifndef _DEFAULT
#define _DEFAULT


// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 1
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif

// Include structures and functions for lighting.
#include "LightingUtils.fx"

cbuffer TRANSFORM : register(b0)
{
    row_major matrix g_World;
    row_major matrix g_WorldInvTranspose;
    row_major matrix g_ViewProj;
}

cbuffer MATERIAL : register(b1)
{
    float4 g_DiffuseAlbedo;
    float3 g_FresnelR0;
    float g_Roughness;
}

cbuffer GLOBAL : register(b2)
{
    float3 g_EyePosW;
    float padding;
    float4 g_AmbientLight;
    Light g_Lights[MaxLights];
}

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.f;
    
    float4 posW = mul(float4(vin.PosL, 1.f), g_World);
    vout.PosW = posW.xyz;
    
    vout.NormalW = mul(vin.NormalL, (float3x3) g_WorldInvTranspose);
    
    vout.PosH = mul(posW, g_ViewProj);
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    pin.NormalW = normalize(pin.NormalW);
    
    float3 toEyeW = normalize(g_EyePosW - pin.PosW);
    
    // indirect light
    float4 ambient = g_AmbientLight * g_DiffuseAlbedo;
    
    // direct light
    const float shininess = 1.f - g_Roughness;
    Material mat = { g_DiffuseAlbedo, g_FresnelR0, shininess };
    float3 shadowFactor = 1.f;
    float4 directLight = ComputeLighting(g_Lights, mat, pin.PosW, pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;
    
    litColor.a = g_DiffuseAlbedo.a;
    
    return litColor;
}

#endif