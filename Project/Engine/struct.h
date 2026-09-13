#pragma once

struct Vertex
{
	Vector3 Pos;
	Vector4 Color;
	Vector2 UV;
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