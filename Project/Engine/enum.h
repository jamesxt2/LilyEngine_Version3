#pragma once

enum class ASSET_TYPE
{
	PREFAB,
	MESH,
	SUBMESH_GEO,
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
	OBJECT,
	GLOBAL,
	SSAO,

	END
};

// Structured Buffer Types
enum class SB_TYPE
{
	MATERIAL,
	END
};

enum class DIR_TYPE
{
	RIGHT,
	UP,
	FORWARD
};

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
enum class PSO_TYPE : uint32
{
	PSO_NONE = 0,
	PSO_DEFAULT = 1 << 0,
	PSO_WIREFRAME = 1 << 1,
	PSO_TRANSPARENT = 1 << 2,
	PSO_ALPHA_TESTED = 1 << 3,
	PSO_BILLBOARD = 1 << 4,
	PSO_WAVE_RENDER = 1 << 5,
	PSO_WAVE_UPDATE = 1 << 6,
	PSO_WAVE_DISTURB = 1 << 7,
	PSO_PARTICLE_RENDER = 1 << 8,
	PSO_PARTICLE_TICK = 1 << 9,
	PSO_TESS_QUAD_PATCH = 1 << 10,
	PSO_SKY = 1 << 11,
	PSO_SHADOW_MAP = 1 << 12,
	PSO_DRAW_NORMALS = 1 << 13,
	PSO_SSAO = 1 << 14,
	PSO_SSAO_BLUR = 1 << 15,
	PSO_FXAA = 1 << 16,
};

inline PSO_TYPE operator|(PSO_TYPE a, PSO_TYPE b)
{
	return static_cast<PSO_TYPE>(static_cast<uint32>(a) | static_cast<uint32>(b));
}

inline PSO_TYPE operator&(PSO_TYPE a, PSO_TYPE b)
{
	return static_cast<PSO_TYPE>(static_cast<uint32>(a) & static_cast<uint32>(b));
}

inline PSO_TYPE operator~(PSO_TYPE a)
{
	return static_cast<PSO_TYPE>(~static_cast<uint32>(a));
}

inline PSO_TYPE operator|=(PSO_TYPE& a, PSO_TYPE b)
{
	a = a | b;
	return a;
}

inline PSO_TYPE operator&=(PSO_TYPE& a, PSO_TYPE b)
{
	a = a & b;
	return a;
}

// Particle
enum class PARTICLE_MODULE
{
	SPAWN,
	ADD_VELOCITY,
	NOISE_FORCE,

	END
};