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

    float4 posW = mul(float4(vin.PosL, 1.f), g_World);
    vout.PosW = posW.xyz;
    
    vout.NormalW = mul(vin.NormalL, (float3x3) g_WorldInvTranspose);
    
    vout.PosH = mul(posW, g_ViewProj);
    
    // Output vertex attributes for interpolation across triangle.
    float4 texCoord = mul(float4(vin.TexCoord, 0.0f, 1.0f), g_TexTransform);
    vout.TexCoord = mul(texCoord, MATERIAL.MtrlTransform).xy;
    
    return vout;
}