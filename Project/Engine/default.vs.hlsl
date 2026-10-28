#include "default_common.hlsli"

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.f;
    
#ifdef DISPLACEMENT_MAP
    // Sample the displacement map using non-transformed [0,1]^2 tex-coords.
    vin.PosL.y += g_DisplacementMap.SampleLevel(g_SamLinearWrap, vin.TexCoord, 1.0f).r;
	
	// Estimate normal using finite difference.
    float du = g_DisplacementMapTexelSize.x;
    float dv = g_DisplacementMapTexelSize.y;
    float l = g_DisplacementMap.SampleLevel(g_SamPointClamp, vin.TexCoord - float2(du, 0.0f), 0.0f).r;
    float r = g_DisplacementMap.SampleLevel(g_SamPointClamp, vin.TexCoord + float2(du, 0.0f), 0.0f).r;
    float t = g_DisplacementMap.SampleLevel(g_SamPointClamp, vin.TexCoord - float2(0.0f, dv), 0.0f).r;
    float b = g_DisplacementMap.SampleLevel(g_SamPointClamp, vin.TexCoord + float2(0.0f, dv), 0.0f).r;
    vin.NormalL = normalize(float3(-r + l, 2.0f * g_GridSpatialStep, b - t));
#endif    

#ifdef SKINNED
    
    float weights[4] = { 0.f, 0.f, 0.f, 0.f };
    weights[0] = vin.BoneWeights.x;
    weights[1] = vin.BoneWeights.y;
    weights[2] = vin.BoneWeights.z;
    weights[3] = 1 - weights[0] - weights[1] - weights[2];
    
    float3 posL = float3(0.f, 0.f, 0.f);
    float3 normalL = float3(0.f, 0.f, 0.f);
    float3 tangentL = float3(0.f, 0.f, 0.f);
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        posL += weights[i] * mul(float4(vin.PosL, 1.f), g_BoneTransforms[vin.BoneIndices[i]]).xyz;
        normalL += weights[i] * mul(vin.NormalL, (float3x3)g_BoneInvTranspose[vin.BoneIndices[i]]);
        tangentL += weights[i] * mul(vin.TangentU, (float3x3)g_BoneTransforms[vin.BoneIndices[i]]);
    }
    
    vin.PosL = posL;
    vin.NormalL = normalL;
    vin.TangentU = tangentL;
    
#endif
    
    
    float4 posW = mul(float4(vin.PosL, 1.f), g_World);
    vout.PosW = posW.xyz;
    
    vout.NormalW = mul(vin.NormalL, (float3x3) g_WorldInvTranspose);
    
    vout.TangentW = mul(vin.TangentU, (float3x3) g_World);
    
    vout.PosH = mul(posW, g_ViewProj);
    
    // Output vertex attributes for interpolation across triangle.
    float4 texCoord = mul(float4(vin.TexCoord, 0.0f, 1.0f), g_TexTransform);
    vout.TexCoord = mul(texCoord, MATERIAL.MtrlTransform).xy;
    
    vout.ShadowPosH = mul(posW, g_ShadowTransform);
    
    vout.SsaoPosH = mul(posW, g_ViewProjTex);
    
    return vout;
}