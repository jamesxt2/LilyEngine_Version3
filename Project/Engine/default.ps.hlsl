#include "default_common.hlsli"

float4 PS(VertexOut pin) : SV_Target
{
    float4 diffuseAlbedo = MATERIAL.bUseTexture ?
        (g_Textures2D[MATERIAL.TextureIndex].Sample(g_SamAnisotropicWrap, pin.TexCoord) * MATERIAL.DiffuseAlbedo)
        : MATERIAL.DiffuseAlbedo;
    
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
    const float shininess = 1.f - MATERIAL.Roughness;
    Material mat = { diffuseAlbedo, MATERIAL.FresnelR0, shininess };
    float3 shadowFactor = 1.f;
    float4 directLight = ComputeLighting(g_Lights, mat, pin.PosW, pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;
    
    if (MATERIAL.bUseCubeReflection)
    {
        float3 r = reflect(-toEyeW, pin.NormalW);
        float4 reflectionColor = g_TexturesCube[0].Sample(g_SamLinearWrap, r);
        float3 fresnelFactor = SchlickFresnel(MATERIAL.FresnelR0, pin.NormalW, r);
        litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;
    }
    
#ifdef FOG
    float fogAmount = saturate((distToEye - g_FogStart) / g_FogRange);
    litColor = lerp(litColor, g_FogColor, fogAmount);
#endif
    
    litColor.a = diffuseAlbedo.a;
    
    return litColor;
}