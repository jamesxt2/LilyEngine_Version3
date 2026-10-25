#ifndef _SSAO
#define _SSAO

cbuffer SSAO : register(b0)
{
    row_major matrix g_Proj;
    row_major matrix g_InvProj;
    row_major matrix g_ProjTex;
    float4 g_OffsetVectors[14];
    
    float4 g_BlurWeights[3];
    
    float2 g_InvRenderTargetSize;
    
    float g_OcclusionRadius;
    float g_OcclusionFadeStart;
    float g_OcclusionFadeEnd;
    float g_SurfaceEpsilon;
}

cbuffer ROOT_CONSTANTS : register(b1)
{
    bool g_HorizontalBlur;
}

Texture2D g_NormalMap : register(t0);
Texture2D g_DepthMap : register(t1);

SamplerState g_SamPointClamp : register(s0);
SamplerState g_SamLinearClamp : register(s1);
SamplerState g_SamDepthMap : register(s2);
SamplerState g_SamLinearWrap : register(s3);

static const float2 g_TexCoords[6] =
{
    float2(0.f, 1.f),
    float2(0.f, 0.f),
    float2(1.f, 0.f),
    float2(0.f, 1.f),
    float2(1.f, 0.f),
    float2(1.f, 1.f)
};

float NdcDepthToViewDepth(float z_ndc)
{
    // z_ndc = A + B / viewZ
    return g_Proj[3][2] / (z_ndc - g_Proj[2][2]);
}

#endif