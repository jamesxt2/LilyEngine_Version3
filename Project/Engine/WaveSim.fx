#ifndef _WAVESIM
#define _WAVESIM

cbuffer WAVE_UPDATE : register(b0)
{
    float g_WaveConstant0;
    float g_WaveConstant1;
    float g_WaveConstant2;
    
    float g_DisturbMag;
    int2 g_DisturbIndex;
    
    float2 padding;
};

RWTexture2D<float> g_PrevSolInput : register(u0);
RWTexture2D<float> g_CurrSolInput : register(u1);
RWTexture2D<float> g_Output : register(u2);

[numthreads(16, 16, 1)]
void UpdateWavesCS(int3 dispatchThreadID : SV_DispatchThreadID)
{
    // We do not need to do bounds checking because:
	//	 *out-of-bounds reads return 0, which works for us--it just means the boundary of 
	//    our water simulation is clamped to 0 in local space.
	//   *out-of-bounds writes are a no-op.
    int x = dispatchThreadID.x;
    int y = dispatchThreadID.y;
    
    g_Output[int2(x, y)] = g_WaveConstant0 * g_PrevSolInput[int2(x, y)].r +
            g_WaveConstant1 * g_CurrSolInput[int2(x, y)].r +
            g_WaveConstant2 * (g_CurrSolInput[int2(x, y + 1)].r +
            g_CurrSolInput[int2(x, y - 1)].r + g_CurrSolInput[int2(x + 1, y)].r 
            + g_CurrSolInput[int2(x - 1, y)].r);
}

[numthreads(1, 1, 1)]
void DisturbWavesCS(int3 groupThreadID : SV_GroupThreadID,
                    int3 dispatchThreadID : SV_DispatchThreadID)
{
    int x = g_DisturbIndex.x;
    int y = g_DisturbIndex.y;
    
    float halfMag = 0.5f * g_DisturbMag;
    
    // Buffer is RW so operator += is well defined.
    g_Output[int2(x, y)] += g_DisturbMag;
    g_Output[int2(x + 1, y)] += halfMag;
    g_Output[int2(x - 1, y)] += halfMag;
    g_Output[int2(x, y + 1)] += halfMag;
    g_Output[int2(x, y - 1)] += halfMag;
}

#endif