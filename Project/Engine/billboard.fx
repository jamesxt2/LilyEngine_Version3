#ifndef _BILLBOARD
#define _BILLBOARD

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 2
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif

// Include structures and functions for lighting.
#include "LightingUtils.fx"

Texture2DArray g_BillboardMapArray : register(t0);

SamplerState g_SamPointWrap : register(s0);
SamplerState g_SamPointClamp : register(s1);
SamplerState g_SamLinearWrap : register(s2);
SamplerState g_SamLinearClamp : register(s3);
SamplerState g_SamAnisotropicWrap : register(s4);
SamplerState g_SamAnisotropicClamp : register(s5);

cbuffer TRANSFORM : register(b0)
{
    row_major matrix g_World;
    row_major matrix g_WorldInvTranspose;
    row_major matrix g_ViewProj;
    row_major matrix g_TexTransform;
}

cbuffer MATERIAL : register(b1)
{
    float4 g_DiffuseAlbedo;
    float3 g_FresnelR0;
    float g_Roughness;
    int g_bUseTexture;
    float3 padding_Material;
    
    row_major matrix g_MtrlTransform;
}

cbuffer GLOBAL : register(b2)
{
    float3 g_EyePosW;
    float padding_Global1;
    float4 g_AmbientLight;
    Light g_Lights[MaxLights];
    
    float4 g_FogColor;
    float g_FogStart;
    float g_FogRange;
    float2 padding_Global2;
}

struct VertexIn
{
    float3 PosW : POSITION;
    float2 SizeW : SIZE;
};

struct VertexOut
{
    float3 CenterW : POSITION;
    float2 SizeW : SIZE;
};

struct GeoOut
{
    float4 PosH : SV_Position;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexCoord : TEXCOORD;
    uint PrimID : SV_PrimitiveID;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.f;
    
    vout.CenterW = vin.PosW;
    vout.SizeW = vin.SizeW;
    
    return vout;
}

[maxvertexcount(4)]
void GS(point VertexOut gin[1],
        uint primID : SV_PrimitiveID,
        inout TriangleStream<GeoOut> triStream)
{
    float3 up = float3(0.f, 1.f, 0.f);
    float3 look = g_EyePosW - gin[0].CenterW;
    look.y = 0.f; // y-axis aligned, project to xz plane
    look = normalize(look);
    float3 right = cross(up, look);
    
    float halfWidth = 0.5f * gin[0].SizeW.x;
    float halfHeight = 0.5f * gin[0].SizeW.y;
    
    float4 v[4];
    v[0] = float4(gin[0].CenterW + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(gin[0].CenterW + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(gin[0].CenterW - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(gin[0].CenterW - halfWidth * right + halfHeight * up, 1.0f);
    
    float2 texCoord[4] =
    {
        float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
    };
    
    GeoOut gout;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        gout.PosH = mul(v[i], g_ViewProj);
        gout.PosW = v[i].xyz;
        gout.NormalW = look;
        gout.TexCoord = texCoord[i];
        gout.PrimID = primID;
        
        triStream.Append(gout);
    }
}

float4 PS(GeoOut pin) : SV_Target
{
    float3 uvw = float3(pin.TexCoord, pin.PrimID % 3);
    float4 diffuseAlbedo = g_BillboardMapArray.Sample(g_SamAnisotropicWrap, uvw) * g_DiffuseAlbedo;
    
#ifdef ALPHA_TEST
	// Discard pixel if texture alpha < 0.1.  We do this test as soon 
	// as possible in the shader so that we can potentially exit the
	// shader early, thereby skipping the rest of the shader code.
	clip(diffuseAlbedo.a - 0.1f);
#endif
    
    pin.NormalW = normalize(pin.NormalW);
    
    float3 toEyeW = g_EyePosW - pin.PosW;
    float distToEye = length(toEyeW);
    toEyeW /= distToEye;
    
    // indirect light
    float4 ambient = g_AmbientLight * diffuseAlbedo;
    
    // direct light
    const float shininess = 1.f - g_Roughness;
    Material mat = { diffuseAlbedo, g_FresnelR0, shininess };
    float3 shadowFactor = 1.f;
    float4 directLight = ComputeLighting(g_Lights, mat, pin.PosW, pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;
    
#ifdef FOG
    float fogAmount = saturate((distToEye - g_FogStart) / g_FogRange);
    litColor = lerp(litColor, g_FogColor, fogAmount);
#endif
    
    litColor.a = diffuseAlbedo.a;
    
    return litColor;
}

#endif