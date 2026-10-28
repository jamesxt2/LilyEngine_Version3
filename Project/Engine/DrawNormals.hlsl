#ifndef _DRAW_NORMALS
#define _DRAW_NORMALS

#include "common.hlsli"

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexCoord : TEXCOORD;
    float3 TangentU : TANGENT;
#ifdef SKINNED
    float3 BoneWeights : WEIGHTS;
    uint4 BoneIndices  : BONEINDICES;
#endif
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
    
#ifdef SKINNED
    float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    weights[0] = vin.BoneWeights.x;
    weights[1] = vin.BoneWeights.y;
    weights[2] = vin.BoneWeights.z;
    weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

    float3 posL = float3(0.0f, 0.0f, 0.0f);
    float3 normalL = float3(0.0f, 0.0f, 0.0f);
    float3 tangentL = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 4; ++i)
    {
        // Assume no nonuniform scaling when transforming normals, so 
        // that we do not have to use the inverse-transpose.

        posL += weights[i] * mul(float4(vin.PosL, 1.0f), g_BoneTransforms[vin.BoneIndices[i]]).xyz;
        normalL += weights[i] * mul(vin.NormalL, (float3x3) g_BoneTransforms[vin.BoneIndices[i]]);
        tangentL += weights[i] * mul(vin.TangentU.xyz, (float3x3) g_BoneTransforms[vin.BoneIndices[i]]);
    }

    vin.PosL = posL;
    vin.NormalL = normalL;
    vin.TangentU.xyz = tangentL;
#endif
    
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