#pragma once

struct Vertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 TexCoord;

	Vertex() = default;

	Vertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
		: Position(px, py, pz), Normal(nx, ny, nz), TexCoord(u, v)
	{ }
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
	Matrix View;
	Matrix Proj;
	Matrix ViewProj;

	Matrix TexTransform;

	Vector2 DisplacementMapTexelSize;
	float GridSpatialStep;
	float padding;
};

extern TObject g_Object;

struct TMaterial
{
	Vector4 DiffuseAlbedo = { 1.f, 1.f, 1.f, 1.f };
	Vector3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;
	int bUseTecture = 0;
	Vector3 padding;

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
	Vector3 EyePosW;
	float padding1;
	Vector4 AmbientLight;
	TLight Lights[MaxLights];

	Vector4 FogColor;
	float FogStart;
	float FogRange;
	
	float DeltaTime;
	float TotalTime;
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

	float padding{ 0.f };
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

	//Vector3 padding;
};