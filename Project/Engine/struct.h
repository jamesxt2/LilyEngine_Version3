#pragma once

struct Vertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 TexCoord;
	Vector3 TangentU;
	Vector4 Color;

	Vertex() {}
	Vertex(
		const Vector3& p,
		const Vector3& n,
		const Vector3& t,
		const Vector2& uv) :
		Position(p),
		Normal(n),
		TangentU(t),
		TexCoord(uv) {
	}
	Vertex(
		float px, float py, float pz,
		float nx, float ny, float nz,
		float tx, float ty, float tz,
		float u, float v) :
		Position(px, py, pz),
		Normal(nx, ny, nz),
		TangentU(tx, ty, tz),
		TexCoord(u, v) {
	}
};

struct TTransform
{
	Matrix matModel;
	Matrix matView;
	Matrix matProj;

	Matrix matMV;
	Matrix matMVP;
};

extern TTransform g_Trans;