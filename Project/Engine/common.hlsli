#ifndef _COMMON
#define _COMMON

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
    int bUseNormalMap;
    uint NormalMapTexIndex;
    float2 padding_MD;

    row_major matrix MtrlTransform;
};

Texture2D g_Textures2D[MAX_TEXTURES_2D] : register(t0, space0);
Texture2DArray g_TexturesArr[MAX_TEXTURES_ARR] : register(t0, space1);
TextureCube g_TexturesCube[MAX_TEXTURES_CUBE] : register(t0, space4);
Texture2D g_ShadowMap : register(t0, space5);
Texture2D g_SsaoMap : register(t1, space5);

StructuredBuffer<MaterialData> g_Materials : register(t0, space2);

SamplerState g_SamPointWrap : register(s0);
SamplerState g_SamPointClamp : register(s1);
SamplerState g_SamLinearWrap : register(s2);
SamplerState g_SamLinearClamp : register(s3);
SamplerState g_SamAnisotropicWrap : register(s4);
SamplerState g_SamAnisotropicClamp : register(s5);
SamplerComparisonState g_SamShadow : register(s6);


cbuffer OBJECT : register(b0)
{
    row_major matrix g_World;
    row_major matrix g_WorldInvTranspose;
    
    row_major matrix g_TexTransform;
    
    float2 g_DisplacementMapTexelSize;
    float g_GridSpatialStep;
    
    uint g_MaterialIndex;
    
    //float padding_object;
};

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
    row_major matrix g_View;
    row_major matrix g_Proj;
    row_major matrix g_ViewProj;
    row_major matrix g_ShadowTransform;
    row_major matrix g_ViewProjTex;
    
    float3 g_EyePosW;
    float padding_Global1;
    float4 g_AmbientLight;
    Light g_Lights[MaxLights];
    
    float4 g_FogColor;
    float g_FogStart;
    float g_FogRange;
    
    float g_DeltaTime;
    float g_TotalTime;
};

cbuffer SKINNED_DATA : register(b2)
{
    row_major matrix g_BoneTransforms[96];
    row_major matrix g_BoneInvTranspose[96];
};

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, 
                                float3 tangentW)
{
    float3 normalT = 2.f * normalMapSample - 1.f;
    
    float3 N = unitNormalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);
    
    float3x3 TBN = float3x3(T, B, N);
    
    float3 bumpedNormalW = mul(normalT, TBN);
    
    return bumpedNormalW;
}

float CalcShadowFactor(float4 shadowPosH)
{
    shadowPosH.xyz /= shadowPosH.w;
    float depth = shadowPosH.z;
    
    uint width, height, numMips;
    g_ShadowMap.GetDimensions(0, width, height, numMips);
    
    float dx = 1.f / (float) width;
    
    float percentLit = 0.f;
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.f, -dx), float2(dx, -dx),
        float2(-dx, 0.f), float2(0.f, 0.f), float2(dx, 0.f),
        float2(-dx, dx), float2(0.f, dx), float2(dx, dx)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += g_ShadowMap.SampleCmpLevelZero(g_SamShadow,
                shadowPosH.xy + offsets[i], depth).r;
    }

    return percentLit / 9.f;
}

#endif