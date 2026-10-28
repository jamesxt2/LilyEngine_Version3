#include "pch.h"

TObject g_Object = {};
TGlobal g_Global = {};
TSSAO g_SSAO = {};

int g_NumFrameResources = 3;
int g_MaxObjectCount = 100;
int g_MaxMaterialCount = 100;

int g_MaxTexture2DCount = 4000;
int g_MaxTextureArrayCount = 96;
int g_MaxTextureCubeCount = 96;

Vector3 XAxis(1.f, 0.f, 0.f);
Vector3 YAxis(0.f, 1.f, 0.f);
Vector3 ZAxis(0.f, 0.f, 1.f);
