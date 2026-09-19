#pragma once

struct VertexMesh
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 TexCoord;
	Vector3 TangentU;
	Vector4 Color;

	VertexMesh() {}
	VertexMesh(
		const Vector3& p,
		const Vector3& n,
		const Vector3& t,
		const Vector2& uv) :
		Position(p),
		Normal(n),
		TangentU(t),
		TexCoord(uv) {
	}
	VertexMesh(
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


struct MeshData
{
	std::vector<VertexMesh> Vertices;
	std::vector<uint32> Indices32;

	std::vector<uint16>& GetIndices16()
	{
		if (mIndices16.empty())
		{
			mIndices16.resize(Indices32.size());
			for (size_t i = 0; i < Indices32.size(); ++i)
				mIndices16[i] = static_cast<uint16>(Indices32[i]);
		}

		return mIndices16;
	}

private:
	std::vector<uint16> mIndices16;
};


// Defines a subrange of geometry in a MeshGeometry.  This is for when multiple
// geometries are stored in one vertex and index buffer.  It provides the offsets
// and data needed to draw a subset of geometry stores in the vertex and index 
// buffers so that we can implement the technique described by Figure 6.3.
struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	// Bounding box of the geometry defined by this submesh. 
	// This is used in later chapters of the book.
	DirectX::BoundingBox Bounds;
};