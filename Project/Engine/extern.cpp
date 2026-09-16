#include "pch.h"

TTransform g_Trans = {};

int g_NumFrameResources = 3;
int g_MaxObjectCount = 40;

Vector3 XAxis(1.f, 0.f, 0.f);
Vector3 YAxis(0.f, 1.f, 0.f);
Vector3 ZAxis(0.f, 0.f, 1.f);

extern const char* ASSET_TYPE_STRING[(UINT)ASSET_TYPE::END] =
{
	"PREFAB",
	"MESH",
	"MESH_DATA",
	"MATERIAL",
	"TEXTURE",
	"SOUND",
	"GRAPHICS_SHADER",
	"COMPUTE_SHADER"
};

extern const char* COMPONENT_TYPE_STRING[(UINT)COMPONENT_TYPE::END] =
{
	"TRANSFORM",
	"CAMERA",
	"COLLISION2D",
	"COLLISION3D",
	"ANIMATOR2D",
	"ANIMATOR3D",
	"LIGHT2D",
	"LIGHT3D",
	"MESHRENDER",
	"DECAL",
	"PARTICLESYSTEM",
	"TILEMAP",
	"LANDSCAPE"
};