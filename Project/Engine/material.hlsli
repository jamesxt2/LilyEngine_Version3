#ifndef _VALUE
#define _VALUE

#define MAX_TEXTURES_2D 4000
#define MAX_TEXTURES_ARR 96

struct MaterialData
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float Roughness;
    int bUseTexture;
    uint DiffuseMapIndex;
    int bIsTexArray;
    float padding_Mtrl;

    matrix MtrlTransform;
};

Texture2D g_Textures2D[MAX_TEXTURES_2D] : register(t0, space0);
Texture2DArray g_TexturesArr[MAX_TEXTURES_ARR] : register(t0, space1);

StructuredBuffer<MaterialData> g_Materials : register(t0, space2);

SamplerState g_SamPointWrap : register(s0);
SamplerState g_SamPointClamp : register(s1);
SamplerState g_SamLinearWrap : register(s2);
SamplerState g_SamLinearClamp : register(s3);
SamplerState g_SamAnisotropicWrap : register(s4);
SamplerState g_SamAnisotropicClamp : register(s5);


#endif