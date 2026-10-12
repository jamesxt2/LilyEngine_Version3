#ifndef _BLUR
#define _BLUR

cbuffer cbSettings : register(b0)
{
	// We cannot have an array entry in a constant buffer that gets mapped onto
	// root constants, so list each element.  
	
    int g_BlurRadius;

	// Support up to 11 blur weights.
    float w0;
    float w1;
    float w2;
    float w3;
    float w4;
    float w5;
    float w6;
    float w7;
    float w8;
    float w9;
    float w10;
    
    int g_InputWidth;
    int g_InputHeight;
};

static const int g_MaxBlurRadius = 5;

Texture2D g_Input : register(t0);
RWTexture2D<float4> g_Output : register(u0);

#define N 256
#define CacheSize (N + 2 * g_MaxBlurRadius)
groupshared float4 g_Cache[CacheSize];

[numthreads(N, 1, 1)]
void HorzBlurCS(int3 groupThreadID : SV_GroupThreadID,
                int3 dispatchThreadID : SV_DispatchThreadID)
{
    float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

    if (groupThreadID.x < g_BlurRadius)
    {
        int x = max(dispatchThreadID.x - g_BlurRadius, 0);
        g_Cache[groupThreadID.x] = g_Input[int2(x, dispatchThreadID.y)];
    }
    if (groupThreadID.x >= N - g_BlurRadius)
    {
        int x = min(dispatchThreadID.x + g_BlurRadius, g_InputWidth - 1);
        g_Cache[groupThreadID.x + 2 * g_BlurRadius] = g_Input[int2(x, dispatchThreadID.y)];
    }
    g_Cache[groupThreadID.x + g_BlurRadius] = g_Input[min(dispatchThreadID.xy, int2(g_InputWidth - 1, g_InputHeight - 1))];
    
    GroupMemoryBarrierWithGroupSync();

    float4 blurColor = float4(0.f, 0.f, 0.f, 0.f);
    
    for (int i = -g_BlurRadius; i <= g_BlurRadius; ++i)
    {
        int k = groupThreadID.x + g_BlurRadius + i;
        blurColor += weights[i + g_BlurRadius] * g_Cache[k];
    }

    g_Output[dispatchThreadID.xy] = blurColor;
}

[numthreads(1, N, 1)]
void VertBlurCS(int3 groupThreadID : SV_GroupThreadID,
                int3 dispatchThreadID : SV_DispatchThreadID)
{
    float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };
    
    if (groupThreadID.y < g_BlurRadius)
    {
        int y = max(dispatchThreadID.y - g_BlurRadius, 0);
        g_Cache[groupThreadID.y] = g_Input[int2(dispatchThreadID.x, y)];
    }
    if (groupThreadID.y >= N - g_BlurRadius)
    {
        int y = min(dispatchThreadID.y + g_BlurRadius, g_InputHeight - 1);
        g_Cache[groupThreadID.y + 2 * g_BlurRadius] = g_Input[int2(dispatchThreadID.x, y)];
    }
    g_Cache[groupThreadID.y + g_BlurRadius] = g_Input[min(dispatchThreadID.xy, int2(g_InputWidth - 1, g_InputHeight - 1))];
    
    GroupMemoryBarrierWithGroupSync();

    float4 blurColor = float4(0.f, 0.f, 0.f, 0.f);
    
    for (int i = -g_BlurRadius; i <= g_BlurRadius; ++i)
    {
        int k = groupThreadID.y + g_BlurRadius + i;
        blurColor += weights[i + g_BlurRadius] * g_Cache[k];
    }

    g_Output[dispatchThreadID.xy] = blurColor;
}

#endif