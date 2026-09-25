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

struct TTransform
{
	Matrix World;
	Matrix WorldInvTranspose;
	Matrix ViewProj;

	Matrix TexTransform;
};

extern TTransform g_Trans;

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
	Vector2 padding2;
};
extern TGlobal g_Global;