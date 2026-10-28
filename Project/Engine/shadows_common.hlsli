#ifndef _SHADOW
#define _SHADOW

#include "common.hlsli"

struct VertexIn
{
    float3 PosL : POSITION;
    float2 TexCoord : TEXCOORD;
#ifdef SKINNED
    float3 BoneWeights : WEIGHTS;
    uint4 BoneIndices  : BONEINDICES;
#endif
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

#endif