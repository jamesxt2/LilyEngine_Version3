#include "sky_common.hlsli"

float4 PS(VertexOut pin) : SV_Target
{
    return g_TexturesCube[MATERIAL.TextureIndex].Sample(g_SamLinearWrap, pin.PosL);
}