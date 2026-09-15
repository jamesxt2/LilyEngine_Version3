#pragma once

enum class ASSET_TYPE
{
	PREFAB,
	MESH,
	MESH_DATA,
	MATERIAL,
	TEXTURE,
	SOUND,
	GRAPHICS_SHADER,
	COMPUTE_SHADER,

	END
};

extern const char* ASSET_TYPE_STRING[(UINT)ASSET_TYPE::END];

enum class COMPONENT_TYPE
{
	TRANSFORM,
	CAMERA,
	COLLISION2D,
	COLLISION3D,
	ANIMATOR2D,
	ANIMATOR3D,
	LIGHT2D,
	LIGHT3D,

	MESHRENDER,
	DECAL,
	PARTICLESYSTEM,
	TILEMAP,
	LANDSCAPE,

	END,

	SCRIPT
};

extern const char* COMPONENT_TYPE_STRING[(UINT)COMPONENT_TYPE::END];

// Constant Buffer Types
enum class CB_TYPE
{
	TRANSFORM,
	MATERIAL,
	ANIMATION,
	GLOBAL,

	END
};

enum class DIR_TYPE
{
	RIGHT,
	UP,
	FORWARD
};

extern int g_NumFrameResources;
extern int g_MaxObjectCount;

extern Vector3 XAxis;
extern Vector3 YAxis;
extern Vector3 ZAxis;

enum class PROJ_TYPE
{
	ORTHOGRAPHIC,
	PERSPECTIVE
};

// Level
enum class LEVEL_STATE
{
	PLAY,
	STOP,
	PAUSE
};