#include "shadows_common.hlsli"

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
    [unroll]
    for(int i = 0; i < 4; ++i)
    {
        // Assume no nonuniform scaling when transforming normals, so 
        // that we do not have to use the inverse-transpose.

        posL += weights[i] * mul(float4(vin.PosL, 1.0f), g_BoneTransforms[vin.BoneIndices[i]]).xyz;
    }

    vin.PosL = posL;
#endif
    
    float4 posW = mul(float4(vin.PosL, 1.f), g_World);
    vout.PosH = mul(posW, g_ViewProj);
    
    float4 texCoord = mul(float4(vin.TexCoord, 0.f, 1.f), g_TexTransform);
    vout.TexCoord = mul(texCoord, MATERIAL.MtrlTransform).xy;
    
    return vout;
}