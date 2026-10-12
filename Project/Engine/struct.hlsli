#ifndef _STRUCT
#define _STRUCT

#include "material.hlsli"

cbuffer OBJECT : register(b0)
{
    row_major matrix g_World;
    row_major matrix g_WorldInvTranspose;
    row_major matrix g_View;
    row_major matrix g_Proj;
    row_major matrix g_ViewProj;
    row_major matrix g_TexTransform;
    
    float2 g_DisplacementMapTexelSize;
    float g_GridSpatialStep;
    
    uint g_MaterialIndex;
    
    //float padding_object;
}

#define MATERIAL g_Materials[g_MaterialIndex]

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

#define PI 3.1415927f

cbuffer GLOBAL : register(b1)
{
    float3 g_EyePosW;
    float padding_Global1;
    float4 g_AmbientLight;
    Light g_Lights[MaxLights];
    
    float4 g_FogColor;
    float g_FogStart;
    float g_FogRange;
    
    float g_DeltaTime;
    float g_TotalTime;
}


#endif