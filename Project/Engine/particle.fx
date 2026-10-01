#ifndef _PARTICLE
#define _PARTICLE

struct TParticle
{
    float4 Color;
    bool bUseTex;

    float3 LocalPosition;
    float3 WorldPosition;
    float3 WorldRotation;
    float3 WorldInitScale;
    float3 WorldCurrentScale;
};

Texture2D g_ParticleTexture : register(t0);
StructuredBuffer<TParticle> g_Particle : register(t1);

SamplerState g_SamPointWrap : register(s0);
SamplerState g_SamPointClamp : register(s1);
SamplerState g_SamLinearWrap : register(s2);
SamplerState g_SamLinearClamp : register(s3);
SamplerState g_SamAnisotropicWrap : register(s4);
SamplerState g_SamAnisotropicClamp : register(s5);

cbuffer OBJECT : register(b0)
{
    row_major matrix g_World;
    row_major matrix g_WorldInvTranspose;
    row_major matrix g_View;
    row_major matrix g_Proj;
    row_major matrix g_ViewProj;
    row_major matrix g_TexTransform;
    
    float2 g_DisplacementMapTexelSize;
    float g_GridSpatialStep;
    float padding_object;
}

struct VertexIn
{
    float3 PosW : POSITION;
    float2 TexCoord : TEXCOORD;
    uint InstID : SV_InstanceID;
};

struct VertexOut
{
    float3 PosW : POSITION;
    float2 TexCoord : TEXCOORD;
    nointerpolation uint InstID : INSTID;
};

struct GeoOut
{
    float4 PosH : SV_Position;
    float2 TexCoord : TEXCOORD;
    nointerpolation uint InstID : INSTID;
};

VertexOut VS(VertexIn _in)
{
    VertexOut vout = (VertexOut) 0.f;
    vout.PosW = _in.PosW;
    vout.TexCoord = _in.TexCoord;
    vout.InstID = _in.InstID;
    return vout;
}

[maxvertexcount(4)]
void GS(point VertexOut gin[1], inout TriangleStream<GeoOut> triStream)
{
    float4 ViewPos = mul(float4(g_Particle[gin[0].InstID].WorldPosition, 1.f), g_View);
    GeoOut output[4] =
    {
        (GeoOut) 0.f, (GeoOut) 0.f, (GeoOut) 0.f, (GeoOut) 0.f
    };
    
    // View Space
    // 0---1
    // | \ |
    // 3---2
    output[0].PosH = float4(-g_Particle[gin[0].InstID].WorldCurrentScale.x * 0.5f,
        g_Particle[gin[0].InstID].WorldCurrentScale.y * 0.5f, 0.f, 0.f);
    output[1].PosH = float4(g_Particle[gin[0].InstID].WorldCurrentScale.x * 0.5f,
        g_Particle[gin[0].InstID].WorldCurrentScale.y * 0.5f, 0.f, 0.f);
    output[2].PosH = float4(g_Particle[gin[0].InstID].WorldCurrentScale.x * 0.5f,
        -g_Particle[gin[0].InstID].WorldCurrentScale.y * 0.5f, 0.f, 0.f);
    output[3].PosH = float4(-g_Particle[gin[0].InstID].WorldCurrentScale.x * 0.5f,
        -g_Particle[gin[0].InstID].WorldCurrentScale.y * 0.5f, 0.f, 0.f);
    
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        output[i].PosH += ViewPos;
        output[i].PosH = mul(output[i].PosH, g_Proj);
    }

    output[0].TexCoord = float2(0.f, 0.f);
    output[1].TexCoord = float2(1.f, 0.f);
    output[2].TexCoord = float2(1.f, 1.f);
    output[3].TexCoord = float2(0.f, 1.f);
    
    triStream.Append(output[0]);
    triStream.Append(output[1]);
    triStream.Append(output[2]);
    triStream.Append(output[3]);
}

float4 PS(GeoOut _in) : SV_Target
{
    float4 Color = g_Particle[_in.InstID].bUseTex ?
        g_ParticleTexture.Sample(g_SamAnisotropicWrap, _in.TexCoord) * g_Particle[_in.InstID].Color :
        g_Particle[_in.InstID].Color;
    return Color;
}

#endif