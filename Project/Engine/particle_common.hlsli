#ifndef _PARTICLE
#define _PARTICLE

#include "struct.hlsli"

struct TParticle
{
    float4 Color;

    float3 RelativePosition;
    float3 RelativeRotation;
    float3 WorldInitScale;
    float3 WorldCurrentScale;
    
    float3 Velocity;
    
    int IsActive;
    float Life;
    float Age;
    float NormalizedAge;
    
    float Mass;
    float3 Force;
    float NoiseForceAccTime;
    float3 NoiseForceDir;
    
    uint VelocityAlignment;
    //float padding_Particle;
};

StructuredBuffer<TParticle> g_Particle : register(t0, space3);

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

#endif