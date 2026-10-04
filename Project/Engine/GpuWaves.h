#pragma once

class GpuWaves
{
public:
	// Note that m,n should be divisible by 16 so there is no 
	// remainder when we divide into thread groups.
	GpuWaves(int m, int n, float dx, float dt, float speed, float damping);
	GpuWaves(const GpuWaves& rhs) = delete;
	GpuWaves& operator=(const GpuWaves& rhs) = delete;
	virtual ~GpuWaves() = default;

	void Update(ID3D12GraphicsCommandList* cmdlist);

	void Disturb(ID3D12GraphicsCommandList* cmdlist, UINT i, UINT j, float magnitude);

protected:

	void BuildResources();
	void BuildDescriptors();

	UINT m_NumRows;
	UINT m_NumCols;

	UINT m_VertexCount;
	UINT m_TriangleCount;

	// Simulation constants we can precompute.
	float m_K[3];

	float m_TimeStep;
	float m_SpatialStep;

	CD3DX12_GPU_DESCRIPTOR_HANDLE m_PrevSolSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_CurrSolSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_NextSolSrv;

	CD3DX12_GPU_DESCRIPTOR_HANDLE m_PrevSolUav;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_CurrSolUav;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_NextSolUav;

	// Two for ping-ponging the textures.
	ComPtr<ID3D12Resource> m_PrevSol = nullptr;
	ComPtr<ID3D12Resource> m_CurrSol = nullptr;
	ComPtr<ID3D12Resource> m_NextSol = nullptr;

	ComPtr<ID3D12Resource> m_PrevUploadBuffer = nullptr;
	ComPtr<ID3D12Resource> m_CurrUploadBuffer = nullptr;

	bool m_IsResourceUploaded;
	void UploadResources(ID3D12GraphicsCommandList* cmdlist);

public:
	inline UINT GetDescriptorCount() const { return 6; }
};

