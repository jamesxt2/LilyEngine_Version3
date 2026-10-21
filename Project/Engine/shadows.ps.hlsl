#include "shadows_common.hlsli"

void PS(VertexOut pin)
{
    float4 diffuseAlbedo = MATERIAL.DiffuseAlbedo;
    uint textureIndex = MATERIAL.TextureIndex;
    
    diffuseAlbedo *= g_Textures2D[textureIndex].Sample(g_SamAnisotropicWrap, pin.TexCoord);
    
#ifdef ALPHA_TEST
    clip(diffuseAlbedo.a - 0.1f);
#endif
}