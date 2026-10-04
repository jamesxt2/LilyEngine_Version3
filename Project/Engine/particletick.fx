#ifndef _PARTICLE_TICK
#define _PARTICLE_TICK

#include "struct.fx"

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
    
    float padding_Particle;
};

struct TSpawnCount
{
    int SpawnCount;
    float3 padding;
};

struct TParticleModule
{
    // Spawn
    uint SpawnRate;
    float4 SpawnColor;
    float3 SpawnMinScale;
    float3 SpawnMaxScale;
    float MinLife;
    float MaxLife;
    uint SpawnShape;
    float3 SpawnShapeScale;
    uint BlockSpawnShape;
    float3 BlockSpawnShapeScale;
    
    // Add Velocity
    uint AddVelocityType; // 0: Random, 1: FromCenter, 2: ToCenter, 3: Fixed
    float3 AddVelocityFixedDir;
    float AddMinSpeed;
    float AddMaxSpeed;
    
    // Noise Force
    float NoiseForceTerm;
    float NoiseForceScale;
    
    int Module[3];
    
    //float3 padding_ParticleModule;
};

RWStructuredBuffer<TParticle> ParticleBuffer : register(u0);
RWStructuredBuffer<TSpawnCount> SpawnCountBuffer : register(u1);
Texture2D NoiseTex : register(t0);
StructuredBuffer<TParticleModule> Module : register(t1);

SamplerState g_SamPointWrap : register(s0);
SamplerState g_SamPointClamp : register(s1);
SamplerState g_SamLinearWrap : register(s2);
SamplerState g_SamLinearClamp : register(s3);
SamplerState g_SamAnisotropicWrap : register(s4);
SamplerState g_SamAnisotropicClamp : register(s5);

cbuffer GLOBAL : register(b0)
{
    float3 g_EyePosW;
    float padding_Global1;
    float4 g_AmbientLight;
    Light g_Lights[MaxLights];
    
    float4 g_FogColor;
    float g_FogStart;
    float g_FogRange;
    
    float g_DeltaTime;
    float g_TotalTime;
}

cbuffer PARTICLE : register(b1)
{
    int MaxParticleCount;
}

// Module Check
#define SpawnModule Module[0].Module[0]
#define AddVelocityModule Module[0].Module[1]
#define NoiseForceModule Module[0].Module[2]

float3 GetRandom(in Texture2D noise, float normalizedThreadID)
{
    float2 vUV = (float2) 0.f;
    vUV.x = normalizedThreadID + g_TotalTime * 0.1f;
    vUV.y = sin((vUV.x - g_TotalTime) * 20 * PI) * 0.5f + g_TotalTime * 0.2f;

    float3 vNoise = noise.SampleLevel(g_SamLinearWrap, vUV, 0).xyz;
    return vNoise;
}

[numthreads(32, 1, 1)]
void CS_ParticleTick(int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= MaxParticleCount)
        return;
    if (ParticleBuffer[dispatchThreadID.x].IsActive == 0)
    {
        if (SpawnModule)
        {
            int CurSpawnCount = SpawnCountBuffer[0].SpawnCount;
            while (CurSpawnCount > 0)
            {
                int originValue = 0;
                InterlockedCompareExchange(SpawnCountBuffer[0].SpawnCount,
                    CurSpawnCount, SpawnCountBuffer[0].SpawnCount - 1, originValue);
                if (CurSpawnCount == originValue)
                {
                    float3 random = GetRandom(NoiseTex, (float) dispatchThreadID.x / (float) (MaxParticleCount - 1));
                    float3 random1 = GetRandom(NoiseTex, (float) (dispatchThreadID.x + 1) / (float) (MaxParticleCount - 1));
                    float3 random2 = GetRandom(NoiseTex, (float) (dispatchThreadID.x + 2) / (float) (MaxParticleCount - 1));
                    
                    float3 SpawnPosition = (float3) 0.f;
                    
                    if (Module[0].SpawnShape == 0)
                    {
                        if (Module[0].BlockSpawnShape == 1)
                        {
                            float3 differScale = Module[0].SpawnShapeScale - Module[0].BlockSpawnShapeScale;
                            SpawnPosition.x = random.x * differScale.x + Module[0].BlockSpawnShapeScale.x;
                            SpawnPosition.y = random.y * differScale.y + Module[0].BlockSpawnShapeScale.y;
                            SpawnPosition.z = random.z * differScale.z + Module[0].BlockSpawnShapeScale.z;
                        }
                        else
                        {
                            SpawnPosition.x = random.x * Module[0].SpawnShapeScale.x - Module[0].SpawnShapeScale.x / 2.f;
                            SpawnPosition.y = random.y * Module[0].SpawnShapeScale.y - Module[0].SpawnShapeScale.y / 2.f;
                            SpawnPosition.z = random.z * Module[0].SpawnShapeScale.z - Module[0].SpawnShapeScale.z / 2.f;
                        }
                    }
                    else if (Module[0].SpawnShape == 1)
                    {
                        float radius = Module[0].SpawnShapeScale.x;
                        
                        if (Module[0].BlockSpawnShape == 1)
                        {
                            float blockRadius = Module[0].BlockSpawnShapeScale.x;
                            float differRadius = radius - blockRadius;
                            SpawnPosition = normalize(random1 - 0.5f) * differRadius * random2.x
                                            + normalize(random1 - 0.5f) * blockRadius;
                        }
                        else
                            SpawnPosition = normalize(random1 - 0.5f) * radius * random2.x;
                    }
                    
                    ParticleBuffer[dispatchThreadID.x].Velocity = (float3) 0.f;
                    if (AddVelocityModule)
                    {
                        float speed = Module[0].AddMinSpeed + (Module[0].AddMaxSpeed - Module[0].AddMinSpeed) * random1.x;
                        // Random
                        if (Module[0].AddVelocityType == 0)
                            ParticleBuffer[dispatchThreadID.x].Velocity = normalize(random2 - 0.5f) * speed;
                        // From Center
                        else if (Module[0].AddVelocityType == 1)
                            ParticleBuffer[dispatchThreadID.x].Velocity = normalize(SpawnPosition) * speed;
                        // To Center
                        else if (Module[0].AddVelocityType == 2)
                            ParticleBuffer[dispatchThreadID.x].Velocity = -normalize(SpawnPosition) * speed;
                        // Fixed
                        else if (Module[0].AddVelocityType == 3)
                            ParticleBuffer[dispatchThreadID.x].Velocity = normalize(Module[0].AddVelocityFixedDir) * speed;
                    }
                    
                    ParticleBuffer[dispatchThreadID.x].RelativePosition = SpawnPosition;
                    ParticleBuffer[dispatchThreadID.x].WorldInitScale = (Module[0].SpawnMaxScale - Module[0].SpawnMinScale) * random.x + Module[0].SpawnMinScale;
                
                    ParticleBuffer[dispatchThreadID.x].Color = Module[0].SpawnColor;
                    
                    ParticleBuffer[dispatchThreadID.x].IsActive = 1;
                    ParticleBuffer[dispatchThreadID.x].Age = 0.f;
                    ParticleBuffer[dispatchThreadID.x].Life = (Module[0].MaxLife - Module[0].MinLife) * random1.y + Module[0].MinLife;
                    ParticleBuffer[dispatchThreadID.x].NormalizedAge = 0.f;
                
                    break;
                }
                CurSpawnCount = SpawnCountBuffer[0].SpawnCount;
            }
        }
    }
    else
    {
        ParticleBuffer[dispatchThreadID.x].Force = (float3) 0.f;
        if (NoiseForceModule)
        {
            if (ParticleBuffer[dispatchThreadID.x].NoiseForceAccTime >= Module[0].NoiseForceTerm)
            {
                ParticleBuffer[dispatchThreadID.x].NoiseForceAccTime = 0.f;
                float3 random = GetRandom(NoiseTex, ((float) dispatchThreadID.x / (float) (MaxParticleCount - 1)));
                ParticleBuffer[dispatchThreadID.x].NoiseForceDir = normalize(random - 0.5f);
            }
            
            ParticleBuffer[dispatchThreadID.x].Force += ParticleBuffer[dispatchThreadID.x].NoiseForceDir * Module[0].NoiseForceScale;
            ParticleBuffer[dispatchThreadID.x].NoiseForceAccTime += g_DeltaTime;
        }
        
        float3 accelerate = ParticleBuffer[dispatchThreadID.x].Force / ParticleBuffer[dispatchThreadID.x].Mass;
        ParticleBuffer[dispatchThreadID.x].Velocity += accelerate * g_DeltaTime;
        
        ParticleBuffer[dispatchThreadID.x].Age += g_DeltaTime;
        ParticleBuffer[dispatchThreadID.x].NormalizedAge = ParticleBuffer[dispatchThreadID.x].Age / ParticleBuffer[dispatchThreadID.x].Life;
        
        ParticleBuffer[dispatchThreadID.x].RelativePosition += ParticleBuffer[dispatchThreadID.x].Velocity * g_DeltaTime;
        
        ParticleBuffer[dispatchThreadID.x].WorldCurrentScale = ParticleBuffer[dispatchThreadID.x].WorldInitScale;
        
        if (ParticleBuffer[dispatchThreadID.x].Age >= ParticleBuffer[dispatchThreadID.x].Life)
            ParticleBuffer[dispatchThreadID.x].IsActive = 0;
    }
}

#endif