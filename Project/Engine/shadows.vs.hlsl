#include "shadows_common.hlsli"

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.f;
    
    float4 posW = mul(float4(vin.PosL, 1.f), g_World);
    vout.PosH = mul(posW, g_ViewProj);
    
    float4 texCoord = mul(float4(vin.TexCoord, 0.f, 1.f), g_TexTransform);
    vout.TexCoord = mul(texCoord, MATERIAL.MtrlTransform).xy;
    
    return vout;
}