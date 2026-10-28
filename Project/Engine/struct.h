#pragma once

struct Vertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 TexCoord;
	Vector3 TangentU;

	Vertex() = default;

	Vertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v, float tx, float ty, float tz)
		: Position(px, py, pz), Normal(nx, ny, nz), TexCoord(u, v), TangentU(tx, ty, tz)
	{ }
};

struct SkinnedVertex
{
	Vector3 Pos;
	Vector3 Normal;
	Vector2 TexC;
	Vector3 TangentU;
	Vector3 BoneWeights;
	BYTE BoneIndices[4] = {};
};

struct BillboardVertex
{
	Vector3 Position;
	Vector2 Size;
};

struct TObject
{
	Matrix World;
	Matrix WorldInvTranspose;
	

	Matrix TexTransform;

	Vector2 DisplacementMapTexelSize;
	float GridSpatialStep;
	
	UINT MaterialIndex;

	//float padding;
};

extern TObject g_Object;

struct TMaterial
{
	Vector4 DiffuseAlbedo = { 1.f, 1.f, 1.f, 1.f };
	Vector3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;
	int bUseTexture = 0;
	UINT TextureIndex = 0;
	int bIsTexArray = 0;
	int bUseCubeReflection = 0;
	int bUseNormalMap = 0;
	UINT NormalMapTexIndex = 0;
	Vector2 padding;

	Matrix MtrlTransform;
};

struct TLight
{
	Vector3 Strength = { 0.5f, 0.5f, 0.5f };
	float FalloffStart = 1.f; // point and spot light
	Vector3 Direction = { 0.f, -1.f, 0.f }; // directional and spot light
	float FalloffEnd = 10.f; // point and spot light
	Vector3 Position = { 0.f, 0.f, 0.f }; // point and spot light
	float SpotPower = 64.f; // spot light
};

#define MaxLights 16

struct TGlobal
{
	Matrix View;
	Matrix Proj;
	Matrix ViewProj;
	Matrix ShadowTransform;
	Matrix ViewProjTex;

	Vector3 EyePosW;
	float padding1{ 0.f };
	Vector4 AmbientLight;
	TLight Lights[MaxLights];

	Vector4 FogColor;
	float FogStart{ 0.f };
	float FogRange{ 0.f };
	
	float DeltaTime{ 0.f };
	float TotalTime{ 0.f };
};
extern TGlobal g_Global;

struct TParticle
{
	Vector4 Color;

	Vector3 RelativePosition;
	Vector3 RelativeRotation;
	Vector3 WorldInitScale;
	Vector3 WorldCurrentScale;

	Vector3 Velocity;

	int IsActive{ 0 };
	float Life{ 0.f };
	float Age{ 0.f };
	float NormalizedAge{ 0.f };

	float Mass{ 1.f };
	Vector3 Force;
	float NoiseForceAccTime{ 0.f };
	Vector3 NoiseForceDir;

	UINT VelocityAlignment{ 0 };

	//float padding{ 0.f };
};

struct TParticleSpawnCount
{
	int SpawnCount{ 0 };
	Vector3 padding;
};

struct TParticleModule
{
	// Spawn
	UINT SpawnRate{ 0 };

	Vector4 SpawnColor;
	Vector3 SpawnMinScale;
	Vector3 SpawnMaxScale;

	float MinLife{ 0.f };
	float MaxLife{ 0.f };

	UINT SpawnShape{ 1 }; // 0: Box, 1: Sphere
	Vector3 SpawnShapeScale; // x == Radius

	UINT BlockSpawnShape{ 1 }; // 0: Box, 1: Sphere
	Vector3 BlockSpawnShapeScale; // x == Radius

	UINT VelocityAlignment{ 0 };

	// Add Velocity
	UINT AddVelocityType{ 0 }; // 0: Random, 1: FromCenter, 2: ToCenter, 3: Fixed
	Vector3 AddVelocityFixedDir;
	float AddMinSpeed{ 0.f };
	float AddMaxSpeed{ 0.f };

	// Noise Force
	float NoiseForceTerm{ 0.f };
	float NoiseForceScale{ 0.f };

	// Module on/off
	int Module[(UINT)PARTICLE_MODULE::END] = {};

	Vector3 padding;
};

struct TSSAO
{
	Matrix Proj;
	Matrix InvProj;
	Matrix ProjTex;
	Vector4 OffsetVectors[14];

	Vector4 BlurWeights[3];

	Vector2 InvRenderTargetSize;

	float OcclusionRadius = 0.f;
	float OcclusionFadeStart = 0.f;
	float OcclusionFadeEnd = 0.f;
	float SurfaceEpsilon = 0.f;
};
extern TSSAO g_SSAO;

struct TSkinned
{
	Matrix BoneTransforms[96];
	Matrix BoneInvTranspose[96];
};