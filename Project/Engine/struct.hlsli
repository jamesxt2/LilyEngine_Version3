#ifndef _STRUCT
#define _STRUCT

#define MAX_TEXTURES_2D 4000
#define MAX_TEXTURES_ARR 96
#define MAX_TEXTURES_CUBE 96

struct MaterialData
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float Roughness;
    int bUseTexture;
    uint TextureIndex;
    int bIsTexArray;
    int bUseCubeReflection;

    matrix MtrlTransform;
};

Texture2D g_Textures2D[MAX_TEXTURES_2D] : register(t0, space0);
Texture2DArray g_TexturesArr[MAX_TEXTURES_ARR] : register(t0, space1);
TextureCube g_TexturesCube[MAX_TEXTURES_CUBE] : register(t0, space4);

StructuredBuffer<MaterialData> g_Materials : register(t0, space2);

SamplerState g_SamPointWrap : register(s0);
SamplerState g_SamPointClamp : register(s1);
SamplerState g_SamLinearWrap : register(s2);
SamplerState g_SamLinearClamp : register(s3);
SamplerState g_SamAnisotropicWrap : register(s4);
SamplerState g_SamAnisotropicClamp : register(s5);



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