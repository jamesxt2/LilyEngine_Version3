#include "pch.h"
#include "CParticleSystem.h"

#include "CDevice.h"
#include "CStructuredBuffer.h"
#include "CTimeMgr.h"
#include "CAssetMgr.h"

CParticleSystem::CParticleSystem()
	: CRenderComponent(COMPONENT_TYPE::PARTICLESYSTEM),
	m_MaxParticleCount(1000), m_ParticleBuffer(nullptr), m_Time(0.f),
	m_SpawnCountBuffer(nullptr), m_ModuleBuffer(nullptr)
{
	m_ParticleBuffer = new CStructuredBuffer(sizeof(TParticle), m_MaxParticleCount);

	std::vector<TParticle> particles(m_MaxParticleCount);
	m_ParticleBuffer->UploadData((const void*)particles.data());

	m_SpawnCountBuffer = new CStructuredBuffer(sizeof(TParticleSpawnCount), 1);
	m_ModuleBuffer = new CStructuredBuffer(sizeof(TParticleModule), 1);

	// Spawn Module
	m_Module.Module[(UINT)PARTICLE_MODULE::SPAWN] = 1;
	m_Module.SpawnRate = 100;
	m_Module.SpawnShape = 0;
	m_Module.SpawnShapeScale = Vector3(200.f, 2.f, 200.f);
	m_Module.BlockSpawnShape = 0;
	m_Module.BlockSpawnShapeScale = Vector3(0.f);
	m_Module.MinLife = 5.f;
	m_Module.MaxLife = 10.f;
	m_Module.SpawnMinScale = Vector3(0.5f);
	m_Module.SpawnMaxScale = Vector3(1.f);
	m_Module.SpawnColor = Vector4(1.f, 1.f, 1.f, 1.f);

	// Add Velocity Module
	m_Module.Module[(UINT)PARTICLE_MODULE::ADD_VELOCITY] = 1;
	m_Module.AddVelocityType = 3;
	m_Module.AddVelocityFixedDir = Vector3(0.f, -1.f, 0.f);
	m_Module.AddMinSpeed = 1.f;
	m_Module.AddMaxSpeed = 5.f;

	// Noise Force Module
	m_Module.Module[(UINT)PARTICLE_MODULE::NOISE_FORCE] = 1;
	m_Module.NoiseForceTerm = 0.1f;
	m_Module.NoiseForceScale = 10.f;

	m_ModuleBuffer->UploadData((const void*)&m_Module);
}

CParticleSystem::CParticleSystem(const CParticleSystem& _other)
	: CRenderComponent(_other),
	m_MaxParticleCount(_other.m_MaxParticleCount), m_ParticleBuffer(nullptr), m_Time(0.f),
	m_SpawnCountBuffer(nullptr), m_ModuleBuffer(nullptr)
{
	assert(_other.m_ParticleBuffer);
	m_ParticleBuffer = new CStructuredBuffer(sizeof(TParticle), m_MaxParticleCount);
	m_SpawnCountBuffer = new CStructuredBuffer(sizeof(TParticleSpawnCount), 1);
	m_ModuleBuffer = new CStructuredBuffer(sizeof(TParticleModule), 1);
}

CParticleSystem::~CParticleSystem()
{
	if (m_ParticleBuffer != nullptr)
		delete m_ParticleBuffer;
	if (m_SpawnCountBuffer != nullptr)
		delete m_SpawnCountBuffer;
	if (m_ModuleBuffer != nullptr)
		delete m_ModuleBuffer;
}

void CParticleSystem::FinalTick()
{
	CalculateSpawnCount();

	CDevice::GetInst()->UploadResourceAsync([&](ID3D12GraphicsCommandList* cmdlist) {
		ID3D12DescriptorHeap* descriptorHeaps[] = { CAssetMgr::GetInst()->GetDescriptorHeap().Get() };
		cmdlist->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		cmdlist->SetPipelineState(CAssetMgr::GetInst()->GetPSO(OBJ_PSO_TYPE::PSO_PARTICLE_TICK).Get());
		cmdlist->SetComputeRootSignature(CAssetMgr::GetInst()->GetRootSignature(L"ParticleTick").Get());

		CDevice::GetInst()->GetCurrFrameResource()->GetConstantBuffer(CB_TYPE::GLOBAL)->Bind_Compute(cmdlist, 0);
		m_ParticleBuffer->Bind_Compute_UAV_Table(cmdlist, 1);
		m_SpawnCountBuffer->Bind_Compute_UAV_Table(cmdlist, 2);
		m_ModuleBuffer->Bind_Compute_SRV_Table(cmdlist, 3);
		cmdlist->SetComputeRoot32BitConstants(4, 1, &m_MaxParticleCount, 0);
		CAssetMgr::GetInst()->FindAsset<CTexture>(L"NoiseTexture")->Bind_Compute_Table(cmdlist, 5);

		UINT GroupX = m_ParticleBuffer->GetElementCount() / 32;
		if (m_ParticleBuffer->GetElementCount() % 32)
			++GroupX;

		cmdlist->Dispatch(GroupX, 1, 1);
		});

	CDevice::GetInst()->WaitForAllUploads();
}

void CParticleSystem::CalculateSpawnCount()
{
	m_Time += CTimeMgr::GetInst()->DeltaTime();
	TParticleSpawnCount count = {};

	if (m_Module.Module[(UINT)PARTICLE_MODULE::SPAWN])
	{
		float term = 1.f / (float)m_Module.SpawnRate;
		int spawnCount = 0;

		if (m_Time > term)
		{
			float value = m_Time / term;
			spawnCount = (int)floor(value);
			m_Time -= (float)spawnCount * term;
		}

		count.SpawnCount += spawnCount;
	}

	m_SpawnCountBuffer->UploadData((const void*)&count);
}

void CParticleSystem::Render()
{
	if (GetMesh() == nullptr || GetMaterial() == nullptr || m_ParticleBuffer == nullptr) 
		return;

	m_ParticleBuffer->Bind_Graphics_SRV_Table(CMDLIST.Get(), 0);

	GetMaterial()->Bind(3);

	GetMesh()->Bind();

	CMDLIST->DrawIndexedInstanced(GetMesh()->GetIndexCount(), m_MaxParticleCount, 0, 0, 0);
}

