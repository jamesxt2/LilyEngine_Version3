#include "particle_common.hlsli"

float4 PS(GeoOut _in) : SV_Target
{
    float4 Color = float4(0.f, 0.f, 0.f, 0.f);
    if (MATERIAL.bUseTexture)
        Color = g_Textures2D[MATERIAL.DiffuseMapIndex].Sample(g_SamAnisotropicWrap, _in.TexCoord) * g_Particle[_in.InstID].Color;
    else
        Color = g_Particle[_in.InstID].Color;
    return Color;
}
