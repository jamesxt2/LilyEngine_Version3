#include "sky_common.hlsli"

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    vout.PosL = vin.PosL;
    
    float4 posW = mul(float4(vin.PosL, 1.f), g_World);
    posW.xyz += g_EyePosW;
    vout.PosH = mul(posW, g_ViewProj).xyww;
    
    return vout;
}