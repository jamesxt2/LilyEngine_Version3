#ifndef _DRAW_NORMALS
#define _DRAW_NORMALS

#include "common.hlsli"

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexCoord : TEXCOORD;
    float3 TangentU : TANGENT;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 NormalW : NORMAL;
    float2 TexCoord : TEXCOORD;
    float3 TangentW : TANGENT;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.f;
    
    vout.NormalW = mul(vin.NormalL, (float3x3) g_WorldInvTranspose);
    vout.TangentW = mul(vin.TangentU, (float3x3) g_World);
    
    float4 posW = mul(float4(vin.PosL, 1.f), g_World);
    vout.PosH = mul(posW, g_ViewProj);
    
    float4 texC = mul(float4(vin.TexCoord, 0.f, 1.f), g_TexTransform);
    vout.TexCoord = mul(texC, MATERIAL.MtrlTransform).xy;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    float4 diffuseAlbedo = MATERIAL.DiffuseAlbedo;
    uint texIndex = MATERIAL.TextureIndex;
    uint normalMapIdx = MATERIAL.NormalMapTexIndex;
    
    diffuseAlbedo *= g_Textures2D[texIndex].Sample(g_SamAnisotropicWrap, pin.TexCoord);
    
#ifdef ALPHA_TEST
    clip(diffuseAlbedo.a - 0.1f);
#endif
    
    pin.NormalW = normalize(pin.NormalW);
    
    float3 normalV = mul(pin.NormalW, (float3x3) g_View);
    return float4(normalV, 0.f);
}

#endif