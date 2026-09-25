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
	GLOBAL,
	ANIMATION,

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

// PSO
enum class OBJ_PSO_TYPE : uint32
{
	PSO_NONE = 0,
	PSO_DEFAULT = 1 << 0,
	PSO_WIREFRAME = 1 << 1,
	PSO_TRANSPARENT = 1 << 2,
	PSO_ALPHA_TESTED = 1 << 3,
	PSO_MIRRORS = 1 << 4,
	PSO_REFLECTIONS = 1 << 5,
	PSO_SHADOW = 1 << 6,
	PSO_BILLBOARD = 1 << 7,

	//ALL = PSO_DEFAULT | PSO_WIREFRAME | PSO_TRANSPARENT | PSO_ALPHA_TESTED | PSO_MIRRORS | PSO_REFLECTIONS
};

inline OBJ_PSO_TYPE operator|(OBJ_PSO_TYPE a, OBJ_PSO_TYPE b)
{
	return static_cast<OBJ_PSO_TYPE>(static_cast<uint32>(a) | static_cast<uint32>(b));
}

inline OBJ_PSO_TYPE operator&(OBJ_PSO_TYPE a, OBJ_PSO_TYPE b)
{
	return static_cast<OBJ_PSO_TYPE>(static_cast<uint32>(a) & static_cast<uint32>(b));
}

inline OBJ_PSO_TYPE operator~(OBJ_PSO_TYPE a)
{
	return static_cast<OBJ_PSO_TYPE>(~static_cast<uint32>(a));
}

inline OBJ_PSO_TYPE operator|=(OBJ_PSO_TYPE& a, OBJ_PSO_TYPE b)
{
	a = a | b;
	return a;
}

inline OBJ_PSO_TYPE operator&=(OBJ_PSO_TYPE& a, OBJ_PSO_TYPE b)
{
	a = a & b;
	return a;
}