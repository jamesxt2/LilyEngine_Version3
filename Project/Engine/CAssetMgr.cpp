#include "pch.h"
#include "CAssetMgr.h"

#include "d3dUtils.h"
#include "CDevice.h"
#include "CTimeMgr.h"
#include "BlurFilter.h"

CAssetMgr::CAssetMgr()
	: m_Waves(nullptr), m_BlurFilter(nullptr),
	m_CbvSrvUavDescriptorHeap(nullptr), m_TextureDescriptorCount(0)
{
	
}

CAssetMgr::~CAssetMgr()
{

}

void CAssetMgr::GetAssetNames(ASSET_TYPE type, _Out_ std::vector<std::string>& vecNames)
{
	vecNames.clear();
	for (const auto& pair : m_AssetMap[(UINT)type])
	{
		std::string strName;
		WStringToString(pair.first, strName);
		vecNames.push_back(strName);
	}
}

void CAssetMgr::Init()
{
	CDevice::GetInst()->Reset();
	BuildRootSignatures();
	BuildDescriptorHeaps();
	CreateMeshes();
	CreateTextures();
	CreateMaterials();
	CDevice::GetInst()->Close();
}

void CAssetMgr::PostInit()
{
	CreateGraphicsShaders();
	CreateComputeShaders();
	BuildPSO();
}

void CAssetMgr::BuildDescriptorHeaps()
{
	m_BlurFilter = std::make_unique<BlurFilter>();
	m_WaveObject = new CWaveObject(256, 256, 0.25f, 0.03f, 2.0f, 0.2f);

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = g_TextureDescriptorsCount + g_BlurDescriptorCount + g_WavesGPUDescriptorCount;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(DEVICE->CreateDescriptorHeap(
		&srvHeapDesc, IID_PPV_ARGS(&m_CbvSrvUavDescriptorHeap)));

	m_BlurFilter->BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), g_TextureDescriptorsCount, CDevice::GetInst()->m_CbvSrvUavDescriptorSize),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), g_TextureDescriptorsCount, CDevice::GetInst()->m_CbvSrvUavDescriptorSize),
		CDevice::GetInst()->m_CbvSrvUavDescriptorSize);

	//m_GpuWaves->BuildDescriptors(
	//	CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), textureDescriptorsCount + blurDescriptorCount, CDevice::GetInst()->m_CbvSrvUavDescriptorSize),
	//	CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), textureDescriptorsCount + blurDescriptorCount, CDevice::GetInst()->m_CbvSrvUavDescriptorSize),
	//	CDevice::GetInst()->m_CbvSrvUavDescriptorSize
	//)
	m_WaveObject->BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), g_TextureDescriptorsCount + g_BlurDescriptorCount, CDevice::GetInst()->m_CbvSrvUavDescriptorSize),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), g_TextureDescriptorsCount + g_BlurDescriptorCount, CDevice::GetInst()->m_CbvSrvUavDescriptorSize),
		CDevice::GetInst()->m_CbvSrvUavDescriptorSize);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE CAssetMgr::GetCPUDescriptorHandle(int Offset) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), Offset, CDevice::GetInst()->m_CbvSrvUavDescriptorSize);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE CAssetMgr::GetGPUDescriptorHandle(int Offset) const
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), Offset, CDevice::GetInst()->m_CbvSrvUavDescriptorSize);
}

void CAssetMgr::BuildRootSignatures()
{
	/***************************************************************/
	// Default
	/***************************************************************/
	CD3DX12_DESCRIPTOR_RANGE texTable_Default;
	texTable_Default.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE displacementMapTable;
	displacementMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter_Default[5];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter_Default[0].InitAsDescriptorTable(1, &texTable_Default, D3D12_SHADER_VISIBILITY_ALL);
	slotRootParameter_Default[1].InitAsConstantBufferView(0);
	slotRootParameter_Default[2].InitAsConstantBufferView(1);
	slotRootParameter_Default[3].InitAsConstantBufferView(2);
	slotRootParameter_Default[4].InitAsDescriptorTable(1, &displacementMapTable, D3D12_SHADER_VISIBILITY_ALL);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc_Default(5, slotRootParameter_Default,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc_Default, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ComPtr<ID3D12RootSignature> defaultRootSig;
	ThrowIfFailed(DEVICE->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(defaultRootSig.GetAddressOf())));
	m_RootSignatureMap[L"Default"] = defaultRootSig;


	/***************************************************************/
	// PostProcess
	/***************************************************************/
	CD3DX12_DESCRIPTOR_RANGE srvTable_PostProcess;
	srvTable_PostProcess.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE uavTable_PostProcess;
	uavTable_PostProcess.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_ROOT_PARAMETER slotRootParameter_PostProcess[3];

	slotRootParameter_PostProcess[0].InitAsConstants(14, 0);
	slotRootParameter_PostProcess[1].InitAsDescriptorTable(1, &srvTable_PostProcess);
	slotRootParameter_PostProcess[2].InitAsDescriptorTable(1, &uavTable_PostProcess);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc_PostProcess(3, slotRootParameter_PostProcess,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	serializedRootSig = nullptr;
	errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&rootSigDesc_PostProcess, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ComPtr<ID3D12RootSignature> postprocessRootSig;
	ThrowIfFailed(DEVICE->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(postprocessRootSig.GetAddressOf())));
	m_RootSignatureMap[L"PostProcess"] = postprocessRootSig;

	/***************************************************************/
	// Waves
	/***************************************************************/
	CD3DX12_DESCRIPTOR_RANGE uavTable0_Waves;
	uavTable0_Waves.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE uavTable1_Waves;
	uavTable1_Waves.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

	CD3DX12_DESCRIPTOR_RANGE uavTable2_Waves;
	uavTable2_Waves.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter_Waves[4];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter_Waves[0].InitAsConstants(6, 0);
	slotRootParameter_Waves[1].InitAsDescriptorTable(1, &uavTable0_Waves);
	slotRootParameter_Waves[2].InitAsDescriptorTable(1, &uavTable1_Waves);
	slotRootParameter_Waves[3].InitAsDescriptorTable(1, &uavTable2_Waves);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc_Waves(4, slotRootParameter_Waves,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_NONE);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	serializedRootSig = nullptr;
	errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&rootSigDesc_Waves, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ComPtr<ID3D12RootSignature> wavesRootSig;
	ThrowIfFailed(DEVICE->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(wavesRootSig.GetAddressOf())));
	m_RootSignatureMap[L"Waves"] = wavesRootSig;
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> CAssetMgr::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}

ComPtr<ID3D12RootSignature> CAssetMgr::GetRootSignature(const std::wstring& key) const
{
	std::unordered_map<std::wstring, ComPtr<ID3D12RootSignature>>::const_iterator iter = m_RootSignatureMap.find(key);
	if (iter == m_RootSignatureMap.end())
		return nullptr;
	return iter->second;
}

void CAssetMgr::BuildPSO()
{
	ComPtr<ID3D12RootSignature> rootSignature(GetRootSignature(L"Default"));
	ComPtr<ID3D12RootSignature> postProcessRootSignature(GetRootSignature(L"PostProcess"));
	ComPtr<ID3D12RootSignature> wavesRootSignature(GetRootSignature(L"Waves"));

	assert(rootSignature);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	Ptr<CGraphicsShader> shader = CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"DefaultShader");
	psoDesc.InputLayout = { shader->GetInputLayout().data(), (UINT)shader->GetInputLayout().size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(shader->GetVsByteCode(L"DefaultVS")->GetBufferPointer()),
		shader->GetVsByteCode(L"DefaultVS")->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(shader->GetPsByteCode(L"FogPS")->GetBufferPointer()),
		shader->GetPsByteCode(L"FogPS")->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = CDevice::GetInst()->EnableMSAA() ? 4 : 1;
	psoDesc.SampleDesc.Quality = CDevice::GetInst()->EnableMSAA() ? CDevice::GetInst()->Get4xMSAAQuality() - 1 : 0;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSOGroup[OBJ_PSO_TYPE::PSO_DEFAULT])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC wireframePSODesc = psoDesc;
	wireframePSODesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&wireframePSODesc, IID_PPV_ARGS(&m_PSOGroup[OBJ_PSO_TYPE::PSO_WIREFRAME])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparencyPSODesc = psoDesc;
	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	transparencyPSODesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&transparencyPSODesc, IID_PPV_ARGS(&m_PSOGroup[OBJ_PSO_TYPE::PSO_TRANSPARENT])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphatestedPSODesc = transparencyPSODesc;
	alphatestedPSODesc.PS =
	{
		reinterpret_cast<BYTE*>(shader->GetPsByteCode(L"AlphaTestedPS")->GetBufferPointer()),
		shader->GetPsByteCode(L"AlphaTestedPS")->GetBufferSize()
	};
	alphatestedPSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&alphatestedPSODesc, IID_PPV_ARGS(&m_PSOGroup[OBJ_PSO_TYPE::PSO_ALPHA_TESTED])));

	// Mirror PSO
	CD3DX12_BLEND_DESC mirrorBlendState(D3D12_DEFAULT);
	mirrorBlendState.RenderTarget[0].RenderTargetWriteMask = 0;

	D3D12_DEPTH_STENCIL_DESC mirrorDSS;
	mirrorDSS.DepthEnable = true;
	mirrorDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	mirrorDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	mirrorDSS.StencilEnable = true;
	mirrorDSS.StencilReadMask = 0xff;
	mirrorDSS.StencilWriteMask = 0xff;

	mirrorDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirrorDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	mirrorDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirrorDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC markMirrorPSODesc = psoDesc;
	markMirrorPSODesc.BlendState = mirrorBlendState;
	markMirrorPSODesc.DepthStencilState = mirrorDSS;
	ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&markMirrorPSODesc, IID_PPV_ARGS(&m_PSOGroup[OBJ_PSO_TYPE::PSO_MIRRORS])));

	D3D12_DEPTH_STENCIL_DESC reflectionsDSS;
	reflectionsDSS.DepthEnable = true;
	reflectionsDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	reflectionsDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	reflectionsDSS.StencilEnable = true;
	reflectionsDSS.StencilReadMask = 0xff;
	reflectionsDSS.StencilWriteMask = 0xff;

	reflectionsDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	reflectionsDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC drawReflectionPSODesc = psoDesc;
	drawReflectionPSODesc.DepthStencilState = reflectionsDSS;
	drawReflectionPSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	drawReflectionPSODesc.RasterizerState.FrontCounterClockwise = true;
	ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&drawReflectionPSODesc, IID_PPV_ARGS(&m_PSOGroup[OBJ_PSO_TYPE::PSO_REFLECTIONS])));

	D3D12_DEPTH_STENCIL_DESC shadowDSS;
	shadowDSS.DepthEnable = true;
	shadowDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	shadowDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	shadowDSS.StencilEnable = true;
	shadowDSS.StencilReadMask = 0xff;
	shadowDSS.StencilWriteMask = 0xff;

	shadowDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	shadowDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	shadowDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	shadowDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowPSODesc = transparencyPSODesc;
	shadowPSODesc.DepthStencilState = shadowDSS;
	ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&shadowPSODesc, IID_PPV_ARGS(&m_PSOGroup[OBJ_PSO_TYPE::PSO_SHADOW])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC billboardPSODesc = psoDesc;
	shader = CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"BillboardShader");
	billboardPSODesc.VS =
	{
		reinterpret_cast<BYTE*>(shader->GetVsByteCode(L"BillboardVS")->GetBufferPointer()),
		shader->GetVsByteCode(L"BillboardVS")->GetBufferSize()
	};
	billboardPSODesc.InputLayout = { shader->GetInputLayout().data(), (UINT)shader->GetInputLayout().size() };
	billboardPSODesc.GS =
	{
		reinterpret_cast<BYTE*>(shader->GetGsByteCode(L"BillboardGS")->GetBufferPointer()),
		shader->GetGsByteCode(L"BillboardGS")->GetBufferSize()
	};
	billboardPSODesc.PS =
	{
		reinterpret_cast<BYTE*>(shader->GetPsByteCode(L"AlphaTestedPS")->GetBufferPointer()),
		shader->GetPsByteCode(L"AlphaTestedPS")->GetBufferSize()
	};
	billboardPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&billboardPSODesc, IID_PPV_ARGS(&m_PSOGroup[OBJ_PSO_TYPE::PSO_BILLBOARD])));

	// CS
	D3D12_COMPUTE_PIPELINE_STATE_DESC horzBlurPSODesc = {};
	horzBlurPSODesc.pRootSignature = postProcessRootSignature.Get();
	Ptr<CComputeShader> pCS = CAssetMgr::GetInst()->FindAsset<CComputeShader>(L"HorzBlurCS");
	horzBlurPSODesc.CS =
	{
		reinterpret_cast<BYTE*>(pCS->GetCsByteCode()->GetBufferPointer()),
		pCS->GetCsByteCode()->GetBufferSize()
	};
	horzBlurPSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(DEVICE->CreateComputePipelineState(&horzBlurPSODesc, IID_PPV_ARGS(&m_PSOGroup[OBJ_PSO_TYPE::PSO_HORIZONTAL_BLUR])));

	D3D12_COMPUTE_PIPELINE_STATE_DESC vertBlurPSODesc = {};
	vertBlurPSODesc.pRootSignature = postProcessRootSignature.Get();
	pCS = CAssetMgr::GetInst()->FindAsset<CComputeShader>(L"VertBlurCS");
	vertBlurPSODesc.CS =
	{
		reinterpret_cast<BYTE*>(pCS->GetCsByteCode()->GetBufferPointer()),
		pCS->GetCsByteCode()->GetBufferSize()
	};
	vertBlurPSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(DEVICE->CreateComputePipelineState(&vertBlurPSODesc, IID_PPV_ARGS(&m_PSOGroup[OBJ_PSO_TYPE::PSO_VERTICAL_BLUR])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC waveRenderPSODesc = transparencyPSODesc;
	shader = CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"WaveShader");
	waveRenderPSODesc.VS =
	{
		reinterpret_cast<BYTE*>(shader->GetVsByteCode(L"WaveVS")->GetBufferPointer()),
		shader->GetVsByteCode(L"WaveVS")->GetBufferSize()
	};
	ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&waveRenderPSODesc, IID_PPV_ARGS(&m_PSOGroup[OBJ_PSO_TYPE::PSO_WAVE_RENDER])));

	D3D12_COMPUTE_PIPELINE_STATE_DESC waveUpdatePSODesc = {};
	waveUpdatePSODesc.pRootSignature = wavesRootSignature.Get();
	pCS = CAssetMgr::GetInst()->FindAsset<CComputeShader>(L"UpdateWavesCS");
	waveUpdatePSODesc.CS =
	{
		reinterpret_cast<BYTE*>(pCS->GetCsByteCode()->GetBufferPointer()),
		pCS->GetCsByteCode()->GetBufferSize()
	};
	waveUpdatePSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(DEVICE->CreateComputePipelineState(&waveUpdatePSODesc, IID_PPV_ARGS(&m_PSOGroup[OBJ_PSO_TYPE::PSO_WAVE_UPDATE])));

	D3D12_COMPUTE_PIPELINE_STATE_DESC waveDisturbPSODesc = {};
	waveDisturbPSODesc.pRootSignature = wavesRootSignature.Get();
	pCS = CAssetMgr::GetInst()->FindAsset<CComputeShader>(L"DisturbWavesCS");
	waveDisturbPSODesc.CS =
	{
		reinterpret_cast<BYTE*>(pCS->GetCsByteCode()->GetBufferPointer()),
		pCS->GetCsByteCode()->GetBufferSize()
	};
	waveDisturbPSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(DEVICE->CreateComputePipelineState(&waveDisturbPSODesc, IID_PPV_ARGS(&m_PSOGroup[OBJ_PSO_TYPE::PSO_WAVE_DISTURB])));
}

ComPtr<ID3D12PipelineState> CAssetMgr::GetPSO(OBJ_PSO_TYPE type) const
{
	std::unordered_map<OBJ_PSO_TYPE, ComPtr<ID3D12PipelineState>>::const_iterator iter = m_PSOGroup.find(type);
	if (iter == m_PSOGroup.end())
		return nullptr;
	return iter->second;
}

void CAssetMgr::Tick()
{
	UpdateWaves();
	AnimateMaterials();
}

void CAssetMgr::UpdateWaves()
{
	// Every quarter second, generate a random wave.
	static float t_base = 0.0f;

	if ((CTimeMgr::GetInst()->TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		int i = Rand(4, m_Waves->RowCount() - 5);
		int j = Rand(4, m_Waves->ColumnCount() - 5);

		float r = RandF(0.2f, 0.5f);

		m_Waves->Disturb(i, j, r);
	}

	// Update the wave simulation.
	m_Waves->Update(CTimeMgr::GetInst()->DeltaTime());

	// Update the wave vertex buffer with the new solution.
	auto currWavesVB = CDevice::GetInst()->GetCurrFrameResource()->m_WavesVB.get();
	for (int i = 0; i < m_Waves->VertexCount(); ++i)
	{
		Vertex v;

		v.Position = m_Waves->Position(i);
		v.Normal = m_Waves->Normal(i);
		
		// Derive tex-coords from position by 
		// mapping [-w/2,w/2] --> [0,1]
		v.TexCoord.x = 0.5f + v.Position.x / m_Waves->Width();
		v.TexCoord.y = 0.5f - v.Position.z / m_Waves->Depth();

		currWavesVB->CopyData(i, (const void*)&v);
	}

	// Set the dynamic VB of the wave renderitem to the current frame VB.
	FindAsset<CMesh>(L"WaveMesh")->m_VertexBufferGPU = currWavesVB->Resource();
}

void CAssetMgr::AnimateMaterials()
{
	Ptr<CMaterial> pWaterMtrl = FindAsset<CMaterial>(L"WaterMaterial");
	if (pWaterMtrl == nullptr) return;

	float tu = pWaterMtrl->m_MtrlTransform(3, 0);
	float tv = pWaterMtrl->m_MtrlTransform(3, 1);

	tu += 0.01f * CTimeMgr::GetInst()->DeltaTime();
	tv += 0.002f * CTimeMgr::GetInst()->DeltaTime();

	if (tu >= 1.f)
		tu -= 1.f;
	if (tv >= 1.f)
		tv -= 1.f;

	pWaterMtrl->m_MtrlTransform(3, 0) = tu;
	pWaterMtrl->m_MtrlTransform(3, 1) = tv;

	pWaterMtrl->m_NumFramesDirty = g_NumFrameResources;
}

void CAssetMgr::CreateMeshes()
{
	CreateDefaultMeshes();
	CreateSceneMeshes();
	CreateWaveMeshes();
	CreateSkullMesh();
	CreateRoomMeshes();
	CreateBillboardMesh();
}

void CAssetMgr::CreateDefaultMeshes()
{
	/***************/
	// Rect Mesh
	/***************/
	std::vector<Vertex> vecVtx;
	Vertex v;
	
	v.Position = Vector3(-0.5f, 0.5f, 0.f);
	v.TexCoord = Vector2(0.f, 0.f);
	vecVtx.push_back(v);

	v.Position = Vector3(0.5f, 0.5f, 0.f);
	v.TexCoord = Vector2(1.f, 0.f);
	vecVtx.push_back(v);

	v.Position = Vector3(0.5f, -0.5f, 0.f);
	v.TexCoord = Vector2(1.f, 1.f);
	vecVtx.push_back(v);

	v.Position = Vector3(-0.5f, -0.5f, 0.f);
	v.TexCoord = Vector2(0.f, 1.f);
	vecVtx.push_back(v);

	std::vector<uint16> vecIdx;

	vecIdx.push_back(0);
	vecIdx.push_back(3);
	vecIdx.push_back(2);

	vecIdx.push_back(0);
	vecIdx.push_back(1);
	vecIdx.push_back(2);

	Ptr<CMesh> pMesh = new CMesh;
	pMesh->CreateVertexBuffer(vecVtx.data(), (UINT)vecVtx.size());
	pMesh->CreateIndexBuffer16(vecIdx.data(), (UINT)vecIdx.size());

	AddAsset<CMesh>(L"RectMesh", pMesh);
}

void CAssetMgr::CreateSceneMeshes()
{
	/*********************************************************************/
	// Scene
	/*********************************************************************/
	CreateBox(L"BoxMeshData", 1.5f, 1.5f, 1.5f, 3);
	CreateGrid(L"GridMeshData", 20.f, 30.f, 60, 40);
	CreateSphere(L"SphereMeshData", 0.5f, 20, 20);
	CreateCylinder(L"CylinderMeshData", 0.5f, 0.3f, 3.f, 20, 20);

	MeshData* box = &(m_MeshDataMap.find(L"BoxMeshData")->second);
	MeshData* grid = &(m_MeshDataMap.find(L"GridMeshData")->second);
	MeshData* sphere = &(m_MeshDataMap.find(L"SphereMeshData")->second);
	MeshData* cylinder = &(m_MeshDataMap.find(L"CylinderMeshData")->second);

	//
	// We are concatenating all the geometry into one big vertex/index buffer.  So
	// define the regions in the buffer each submesh covers.
	//

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box->Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid->Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere->Vertices.size();

	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box->Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid->Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere->Indices32.size();

	// Define the SubmeshGeometry that cover different 
	// regions of the vertex/index buffers.

	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box->Indices32.size();
	boxSubmesh.StartIndexLocation = boxIndexOffset;
	boxSubmesh.BaseVertexLocation = boxVertexOffset;

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = (UINT)grid->Indices32.size();
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = (UINT)sphere->Indices32.size();
	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = (UINT)cylinder->Indices32.size();
	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//
	auto totalVtxCount = box->Vertices.size() + grid->Vertices.size() + sphere->Vertices.size() + cylinder->Vertices.size();
	std::vector<Vertex> vertices(totalVtxCount);

	UINT k = 0;

	for (size_t i = 0; i < box->Vertices.size(); ++i, ++k)
	{
		vertices[k].Position = box->Vertices[i].Position;
		vertices[k].Normal = box->Vertices[i].Normal;
		vertices[k].TexCoord = box->Vertices[i].TexCoord;
	}
	for (size_t i = 0; i < grid->Vertices.size(); ++i, ++k)
	{
		vertices[k].Position = grid->Vertices[i].Position;
		vertices[k].Normal = grid->Vertices[i].Normal;
		vertices[k].TexCoord = grid->Vertices[i].TexCoord;
	}
	for (size_t i = 0; i < sphere->Vertices.size(); ++i, ++k)
	{
		vertices[k].Position = sphere->Vertices[i].Position;
		vertices[k].Normal = sphere->Vertices[i].Normal;
		vertices[k].TexCoord = sphere->Vertices[i].TexCoord;
	}
	for (size_t i = 0; i < cylinder->Vertices.size(); ++i, ++k)
	{
		vertices[k].Position = cylinder->Vertices[i].Position;
		vertices[k].Normal = cylinder->Vertices[i].Normal;
		vertices[k].TexCoord = cylinder->Vertices[i].TexCoord;
	}

	std::vector<uint16> indices;
	indices.insert(indices.end(), std::begin(box->GetIndices16()), std::end(box->GetIndices16()));
	indices.insert(indices.end(), std::begin(grid->GetIndices16()), std::end(grid->GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere->GetIndices16()), std::end(sphere->GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder->GetIndices16()), std::end(cylinder->GetIndices16()));

	Ptr<CMesh> pMesh = new CMesh;
	pMesh->CreateVertexBuffer(vertices.data(), (UINT)vertices.size());
	pMesh->CreateIndexBuffer16(indices.data(), (UINT)indices.size());

	pMesh->m_DrawArgs.emplace("box", std::move(boxSubmesh));
	pMesh->m_DrawArgs.emplace("grid", std::move(gridSubmesh));
	pMesh->m_DrawArgs.emplace("sphere", std::move(sphereSubmesh));
	pMesh->m_DrawArgs.emplace("cylinder", std::move(cylinderSubmesh));

	AddAsset<CMesh>(L"SceneGeoMesh", pMesh);
	/*********************************************************************/
	// Scene
	/*********************************************************************/
}

void CAssetMgr::CreateWaveMeshes()
{
	/*********************************************************************/
	// Land Grid
	/*********************************************************************/
	CreateGrid(L"WaveGridMeshData", 160.f, 160.f, 50, 50);
	MeshData* pLandGridMeshData = &m_MeshDataMap.find(L"WaveGridMeshData")->second;

	std::vector<Vertex> landGridVertices(pLandGridMeshData->Vertices.size());
	for (size_t i = 0; i < pLandGridMeshData->Vertices.size(); ++i)
	{
		auto& p = pLandGridMeshData->Vertices[i].Position;
		landGridVertices[i].Position = p;
		landGridVertices[i].Position.y = 0.3f * (p.z * sinf(0.1f * p.x) + p.x * cosf(0.1f * p.z));

		// n = (-df/dx, 1, -df/dz)
		Vector3 n(
			-0.03f * p.z * cosf(0.1f * p.x) - 0.3f * cosf(0.1f * p.z),
			1.0f,
			-0.3f * sinf(0.1f * p.x) + 0.03f * p.x * sinf(0.1f * p.z));
		n.Normalize();
		landGridVertices[i].Normal = n;

		landGridVertices[i].TexCoord = pLandGridMeshData->Vertices[i].TexCoord;
	}

	std::vector<uint16> waveGridIndices = pLandGridMeshData->GetIndices16();

	Ptr<CMesh> pLandGridMesh = new CMesh;
	pLandGridMesh->CreateVertexBuffer(landGridVertices.data(), (UINT)landGridVertices.size());
	pLandGridMesh->CreateIndexBuffer16(waveGridIndices.data(), (UINT)waveGridIndices.size());

	SubmeshGeometry submeshLandGrid;
	submeshLandGrid.IndexCount = (UINT)waveGridIndices.size();
	submeshLandGrid.StartIndexLocation = 0;
	submeshLandGrid.BaseVertexLocation = 0;

	pLandGridMesh->m_DrawArgs.emplace("grid", std::move(submeshLandGrid));

	AddAsset<CMesh>(L"LandMesh", pLandGridMesh);
	/*********************************************************************/
	// Land Grid
	/*********************************************************************/


	/*********************************************************************/
	// Wave
	/*********************************************************************/
	m_Waves = std::make_unique<Waves>(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);

	std::vector<uint16> waveIndices(3 * m_Waves->TriangleCount()); // 3 indices per face
	assert(m_Waves->VertexCount() < 0x0000ffff);

	// Iterate over each quad.
	int m = m_Waves->RowCount();
	int n = m_Waves->ColumnCount();
	int k = 0;
	for (int i = 0; i < m - 1; ++i)
	{
		for (int j = 0; j < n - 1; ++j)
		{
			waveIndices[k] = i * n + j;
			waveIndices[k + 1] = i * n + j + 1;
			waveIndices[k + 2] = (i + 1) * n + j;

			waveIndices[k + 3] = (i + 1) * n + j;
			waveIndices[k + 4] = i * n + j + 1;
			waveIndices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	UINT vbByteSizeWave = m_Waves->VertexCount() * sizeof(Vertex);

	Ptr<CMesh> pWave = new CMesh;

	// Set dynamically.
	pWave->m_VertexBufferCPU = nullptr;
	pWave->m_VertexBufferGPU = nullptr;

	pWave->CreateIndexBuffer16(waveIndices.data(), (UINT)waveIndices.size());

	pWave->m_VertexByteStride = sizeof(Vertex);
	pWave->m_VertexBufferByteSize = vbByteSizeWave;

	SubmeshGeometry submeshWave;
	submeshWave.IndexCount = (UINT)waveIndices.size();
	submeshWave.StartIndexLocation = 0;
	submeshWave.BaseVertexLocation = 0;

	pWave->m_DrawArgs.emplace("grid", std::move(submeshWave));

	AddAsset<CMesh>(L"WaveMesh", pWave);
	/*********************************************************************/
	// Wave
	/*********************************************************************/
}

void CAssetMgr::CreateSkullMesh()
{
	std::wstring strPath = CPathMgr::GetInst()->GetContentPath();
	std::ifstream fin(strPath + L"models\\skull.txt");

	if (!fin)
	{
		MessageBox(0, L"models\\skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	std::vector<Vertex> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Position.x >> vertices[i].Position.y >> vertices[i].Position.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	std::vector<uint32> indices(3 * tcount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::int32_t);

	Ptr<CMesh> pSkullMesh = new CMesh;
	pSkullMesh->CreateVertexBuffer(vertices.data(), (UINT)vertices.size());
	pSkullMesh->CreateIndexBuffer32(indices.data(), (UINT)indices.size());

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	pSkullMesh->m_DrawArgs["skull"] = submesh;

	AddAsset<CMesh>(L"SkullMesh", pSkullMesh);
}

void CAssetMgr::CreateRoomMeshes()
{
	// Create and specify geometry.  For this sample we draw a floor
	// and a wall with a mirror on it.  We put the floor, wall, and
	// mirror geometry in one vertex buffer.
	//
	//   |--------------|
	//   |              |
	//   |----|----|----|
	//   |Wall|Mirr|Wall|
	//   |    | or |    |
	//   /--------------/
	//  /   Floor      /
	// /--------------/

	std::array<Vertex, 20> vertices =
	{
		// Floor: Observe we tile texture coordinates.
		Vertex(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f), // 0 
		Vertex(-3.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f),
		Vertex(7.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f),
		Vertex(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f),

		// Wall: Observe we tile texture coordinates, and that we
		// leave a gap in the middle for the mirror.
		Vertex(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f), // 4
		Vertex(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f),
		Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f),

		Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f), // 8 
		Vertex(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f),
		Vertex(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f),

		Vertex(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f), // 12
		Vertex(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f),
		Vertex(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f),

		// Mirror
		Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f), // 16
		Vertex(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f),
		Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f)
	};

	std::array<uint16, 30> indices =
	{
		// Floor
		0, 1, 2,
		0, 2, 3,

		// Walls
		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		// Mirror
		16, 17, 18,
		16, 18, 19
	};

	SubmeshGeometry floorSubmesh;
	floorSubmesh.IndexCount = 6;
	floorSubmesh.StartIndexLocation = 0;
	floorSubmesh.BaseVertexLocation = 0;

	SubmeshGeometry wallSubmesh;
	wallSubmesh.IndexCount = 18;
	wallSubmesh.StartIndexLocation = 6;
	wallSubmesh.BaseVertexLocation = 0;

	SubmeshGeometry mirrorSubmesh;
	mirrorSubmesh.IndexCount = 6;
	mirrorSubmesh.StartIndexLocation = 24;
	mirrorSubmesh.BaseVertexLocation = 0;

	Ptr<CMesh> pMesh = new CMesh;
	pMesh->CreateVertexBuffer(vertices.data(), (UINT)vertices.size());
	pMesh->CreateIndexBuffer16(indices.data(), (UINT)indices.size());

	pMesh->m_DrawArgs.emplace("floor", std::move(floorSubmesh));
	pMesh->m_DrawArgs.emplace("wall", std::move(wallSubmesh));
	pMesh->m_DrawArgs.emplace("mirror", std::move(mirrorSubmesh));
	
	AddAsset<CMesh>(L"RoomMesh", pMesh);
}

void CAssetMgr::CreateBillboardMesh()
{
	static const int treeCount = 16;
	std::array<BillboardVertex, 16> vertices;
	for (UINT i = 0; i < treeCount; ++i)
	{
		float x = RandF(-45.0f, 45.0f);
		float z = RandF(-45.0f, 45.0f);
		float y = 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));

		// Move tree slightly above land height.
		y += 8.0f;

		vertices[i].Position = Vector3(x, y, z);
		vertices[i].Size = Vector2(20.0f, 20.0f);
	}

	std::array<uint16, 16> indices =
	{
		0, 1, 2, 3, 4, 5, 6, 7,
		8, 9, 10, 11, 12, 13, 14, 15
	};

	Ptr<CMesh> pMesh = new CMesh;
	pMesh->CreateVertexBuffer(vertices.data(), (UINT)vertices.size());
	pMesh->CreateIndexBuffer16(indices.data(), (UINT)indices.size());

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	pMesh->m_DrawArgs["points"] = submesh;

	AddAsset<CMesh>(L"TreeBillboardsMesh", pMesh);
}

void CAssetMgr::CreateTextures()
{
	Ptr<CTexture> pTexture = new CTexture;
	pTexture->CreateFromFile(L"textures\\WoodCrate01.dds", m_TextureDescriptorCount++);
	AddAsset<CTexture>(L"WoodCrate01Texure", pTexture);

	pTexture = new CTexture;
	pTexture->CreateFromFile(L"textures\\grass.dds", m_TextureDescriptorCount++);
	AddAsset<CTexture>(L"GrassTexure", pTexture);

	pTexture = new CTexture;
	pTexture->CreateFromFile(L"textures\\water1.dds", m_TextureDescriptorCount++);
	AddAsset<CTexture>(L"Water1Texure", pTexture);

	pTexture = new CTexture;
	pTexture->CreateFromFile(L"textures\\bricks.dds", m_TextureDescriptorCount++);
	AddAsset<CTexture>(L"BricksTexture", pTexture);

	pTexture = new CTexture;
	pTexture->CreateFromFile(L"textures\\stone.dds", m_TextureDescriptorCount++);
	AddAsset<CTexture>(L"StoneTexture", pTexture);

	pTexture = new CTexture;
	pTexture->CreateFromFile(L"textures\\tile.dds", m_TextureDescriptorCount++);
	AddAsset<CTexture>(L"TileTexture", pTexture);

	pTexture = new CTexture;
	pTexture->CreateFromFile(L"textures\\WireFence.dds", m_TextureDescriptorCount++);
	AddAsset<CTexture>(L"WireFenceTexture", pTexture);

	pTexture = new CTexture;
	pTexture->CreateFromFile(L"textures\\ice.dds", m_TextureDescriptorCount++);
	AddAsset<CTexture>(L"IceTexture", pTexture);

	pTexture = new CTexture;
	pTexture->CreateFromFile(L"textures\\treeArray2.dds", m_TextureDescriptorCount++, true);
	AddAsset<CTexture>(L"TreeArrayTexture", pTexture);  // 9

	pTexture = new CTexture;
	pTexture->CreateFromFile(L"textures\\particle\\AlphaCircle.dds", m_TextureDescriptorCount++);
	AddAsset<CTexture>(L"AlphaCircleArray", pTexture);
}

void CAssetMgr::CreateMaterials()
{
	Ptr<CMaterial> pMaterial = new CMaterial;
	pMaterial->m_DiffuseAlbedo = Vector4(0.2f, 0.6f, 0.2f, 1.f);
	pMaterial->m_FresnelR0 = Vector3(0.01f, 0.01f, 0.01f);
	pMaterial->m_Roughness = 0.125f;
	pMaterial->m_MtrlCBIndex = 0;
	AddAsset<CMaterial>(L"GreenMaterial", pMaterial);

	pMaterial = new CMaterial;
	pMaterial->m_DiffuseAlbedo = Vector4(0.f, 0.2f, 0.6f, 1.f);
	pMaterial->m_FresnelR0 = Vector3(0.1f, 0.1f, 0.1f);
	pMaterial->m_Roughness = 0.f;
	pMaterial->m_MtrlCBIndex = 1;
	AddAsset<CMaterial>(L"BlueMaterial", pMaterial);

	pMaterial = new CMaterial;
	pMaterial->m_DiffuseAlbedo = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	pMaterial->m_FresnelR0 = Vector3(0.03f, 0.03f, 0.03f);
	pMaterial->m_Roughness = 0.3f;
	pMaterial->m_MtrlCBIndex = 2;
	AddAsset<CMaterial>(L"SkullMaterial", pMaterial);

	pMaterial = new CMaterial;
	pMaterial->m_MtrlCBIndex = 3;
	pMaterial->m_DiffuseAlbedo = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	pMaterial->m_FresnelR0 = Vector3(0.1f, 0.1f, 0.1f);
	pMaterial->m_Roughness = 0.25f;
	pMaterial->m_Texture = FindAsset<CTexture>(L"WoodCrate01Texure");
	AddAsset<CMaterial>(L"WoodBoxMaterial", pMaterial);

	pMaterial = new CMaterial;
	pMaterial->m_MtrlCBIndex = 4;
	pMaterial->m_DiffuseAlbedo = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	pMaterial->m_FresnelR0 = Vector3(0.1f, 0.1f, 0.1f);
	pMaterial->m_Roughness = 0.25f;
	pMaterial->m_Texture = FindAsset<CTexture>(L"WireFenceTexture");
	AddAsset<CMaterial>(L"WireFenceBoxMaterial", pMaterial);

	pMaterial = new CMaterial;
	pMaterial->m_MtrlCBIndex = 5;
	pMaterial->m_DiffuseAlbedo = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	pMaterial->m_FresnelR0 = Vector3(0.01f, 0.01f, 0.01f);
	pMaterial->m_Roughness = 0.125f;
	pMaterial->m_Texture = FindAsset<CTexture>(L"GrassTexure");
	AddAsset<CMaterial>(L"GrassMaterial", pMaterial);

	pMaterial = new CMaterial;
	pMaterial->m_MtrlCBIndex = 6;
	pMaterial->m_DiffuseAlbedo = Vector4(1.0f, 1.0f, 1.0f, 0.5f);
	pMaterial->m_FresnelR0 = Vector3(0.2f, 0.2f, 0.2f);
	pMaterial->m_Roughness = 0.f;
	pMaterial->m_Texture = FindAsset<CTexture>(L"Water1Texure");
	AddAsset<CMaterial>(L"WaterMaterial", pMaterial);

	pMaterial = new CMaterial;
	pMaterial->m_MtrlCBIndex = 7;
	pMaterial->m_DiffuseAlbedo = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	pMaterial->m_FresnelR0 = Vector3(0.08f, 0.08f, 0.08f);
	pMaterial->m_Roughness = 0.01f;
	pMaterial->m_Texture = FindAsset<CTexture>(L"BricksTexture");
	AddAsset<CMaterial>(L"BricksMaterial", pMaterial);

	pMaterial = new CMaterial;
	pMaterial->m_MtrlCBIndex = 8;
	pMaterial->m_DiffuseAlbedo = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	pMaterial->m_FresnelR0 = Vector3(0.05f, 0.05f, 0.05f);
	pMaterial->m_Roughness = 0.3f;
	pMaterial->m_Texture = FindAsset<CTexture>(L"StoneTexture");
	AddAsset<CMaterial>(L"StoneMaterial", pMaterial);

	pMaterial = new CMaterial;
	pMaterial->m_MtrlCBIndex = 9;
	pMaterial->m_DiffuseAlbedo = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	pMaterial->m_FresnelR0 = Vector3(0.02f, 0.02f, 0.02f);
	pMaterial->m_Roughness = 0.3f;
	pMaterial->m_Texture = FindAsset<CTexture>(L"TileTexture");
	AddAsset<CMaterial>(L"TileMaterial", pMaterial);

	pMaterial = new CMaterial;
	pMaterial->m_MtrlCBIndex = 10;
	pMaterial->m_DiffuseAlbedo = Vector4(1.f, 1.f, 1.f, 0.3f);
	pMaterial->m_FresnelR0 = Vector3(0.1f);
	pMaterial->m_Roughness = 0.5f;
	pMaterial->m_Texture = FindAsset<CTexture>(L"IceTexture");
	AddAsset<CMaterial>(L"MirrorMaterial", pMaterial);

	pMaterial = new CMaterial;
	pMaterial->m_MtrlCBIndex = 11;
	pMaterial->m_DiffuseAlbedo = Vector4(0.f, 0.f, 0.f, 0.5f);
	pMaterial->m_FresnelR0 = Vector3(0.001f);
	pMaterial->m_Roughness = 0.f;
	AddAsset<CMaterial>(L"ShadowMaterial", pMaterial);

	pMaterial = new CMaterial;
	pMaterial->m_MtrlCBIndex = 12;
	pMaterial->m_DiffuseAlbedo = Vector4(1.f, 1.f, 1.f, 1.f);
	pMaterial->m_FresnelR0 = Vector3(0.01f);
	pMaterial->m_Roughness = 0.125f;
	pMaterial->m_Texture = FindAsset<CTexture>(L"TreeArrayTexture");
	AddAsset<CMaterial>(L"TreeBillboardMaterial", pMaterial);

	pMaterial = new CMaterial;
	pMaterial->m_MtrlCBIndex = 13;
	pMaterial->m_DiffuseAlbedo = Vector4(1.f, 1.f, 1.f, 1.f);
	pMaterial->m_FresnelR0 = Vector3(0.01f);
	pMaterial->m_Roughness = 0.125f;
	pMaterial->m_Texture = FindAsset<CTexture>(L"AlphaCircleTexture");
	AddAsset<CMaterial>(L"AlphaCircleMaterial", pMaterial);
}

void CAssetMgr::CreateGraphicsShaders()
{
	std::wstring strPath = CPathMgr::GetInst()->GetContentPath();

	Ptr<CGraphicsShader> pShader = nullptr;

	const D3D_SHADER_MACRO opaqueDefines[] =
	{
		"FOG", "1",
		NULL, NULL
	};
	pShader = new CGraphicsShader;
	pShader->BuildVertexShader(L"DefaultVS", strPath + L"shader\\default.fx", nullptr, "VS");
	pShader->m_InputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	pShader->BuildPixelShader(L"FogPS", strPath + L"shader\\default.fx", opaqueDefines, "PS");

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};
	pShader->BuildPixelShader(L"AlphaTestedPS", strPath + L"shader\\default.fx", alphaTestDefines, "PS");

	AddAsset<CGraphicsShader>(L"DefaultShader", pShader);

	pShader = new CGraphicsShader;
	pShader->BuildVertexShader(L"BillboardVS", strPath + L"shader\\billboard.fx", nullptr, "VS");
	pShader->m_InputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	pShader->BuildGeometryShader(L"BillboardGS", strPath + L"shader\\billboard.fx", nullptr, "GS");
	pShader->BuildPixelShader(L"AlphaTestedPS", strPath + L"shader\\billboard.fx", alphaTestDefines, "PS");

	AddAsset<CGraphicsShader>(L"BillboardShader", pShader);

	const D3D_SHADER_MACRO waveDefines[] =
	{
		"DISPLACEMENT_MAP", "1",
		NULL, NULL
	};
	pShader = new CGraphicsShader;
	pShader->BuildVertexShader(L"WaveVS", strPath + L"shader\\default.fx", waveDefines, "VS");
	pShader->m_InputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	pShader->BuildPixelShader(L"AlphaTestedPS", strPath + L"shader\\default.fx", alphaTestDefines, "PS");

	AddAsset<CGraphicsShader>(L"WaveShader", pShader);
}

void CAssetMgr::CreateComputeShaders()
{
	std::wstring strPath = CPathMgr::GetInst()->GetContentPath();

	Ptr<CComputeShader> pShader = new CComputeShader;
	pShader->BuildComputeShader(strPath + L"shader\\blur.fx", nullptr, "HorzBlurCS");
	AddAsset<CComputeShader>(L"HorzBlurCS", pShader);

	pShader = new CComputeShader;
	pShader->BuildComputeShader(strPath + L"shader\\blur.fx", nullptr, "VertBlurCS");
	AddAsset<CComputeShader>(L"VertBlurCS", pShader);

	pShader = new CComputeShader;
	pShader->BuildComputeShader(strPath + L"shader\\WaveSim.fx", nullptr, "UpdateWavesCS");
	AddAsset<CComputeShader>(L"UpdateWavesCS", pShader);

	pShader = new CComputeShader;
	pShader->BuildComputeShader(strPath + L"shader\\WaveSim.fx", nullptr, "DisturbWavesCS");
	AddAsset<CComputeShader>(L"DisturbWavesCS", pShader);
}



void CAssetMgr::CreateBox(const std::wstring& name, float width, float height, float depth, uint32 numSubdivisions)
{
	MeshData meshData;

	//
	// Create the vertices.
	//

	VertexMesh v[24];

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;

	// Position Normal TangentU TexCoord
	// Fill in the front face vertex data.
	v[0] = VertexMesh(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[1] = VertexMesh(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[2] = VertexMesh(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[3] = VertexMesh(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = VertexMesh(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[5] = VertexMesh(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[6] = VertexMesh(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[7] = VertexMesh(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8] = VertexMesh(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[9] = VertexMesh(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[10] = VertexMesh(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[11] = VertexMesh(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = VertexMesh(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[13] = VertexMesh(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[14] = VertexMesh(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[15] = VertexMesh(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = VertexMesh(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[17] = VertexMesh(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[18] = VertexMesh(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[19] = VertexMesh(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = VertexMesh(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[21] = VertexMesh(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[22] = VertexMesh(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[23] = VertexMesh(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	// 0~23 [ , )
	meshData.Vertices.assign(&v[0], &v[24]);

	//
	// Create the indices.
	//

	uint32 i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	meshData.Indices32.assign(&i[0], &i[36]);

	// Put a cap on the number of subdivisions.
	numSubdivisions = std::min<uint32>(numSubdivisions, 6u);

	for (uint32 i = 0; i < numSubdivisions; ++i)
		Subdivide(meshData);

	m_MeshDataMap.emplace(name, std::move(meshData));
}

void CAssetMgr::CreateSphere(const std::wstring& name, float radius, uint32 sliceCount, uint32 stackCount)
{
	MeshData meshData;

	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	VertexMesh topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	VertexMesh bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	meshData.Vertices.push_back(topVertex);

	float phiStep = XM_PI / stackCount;
	float thetaStep = 2.0f * XM_PI / sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (uint32 i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// Vertices of ring.
		for (uint32 j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			VertexMesh v;

			// spherical to cartesian
			v.Position.x = radius * sinf(phi) * cosf(theta);
			v.Position.y = radius * cosf(phi);
			v.Position.z = radius * sinf(phi) * sinf(theta);

			// Partial derivative of P with respect to theta
			v.TangentU.x = -radius * sinf(phi) * sinf(theta);
			v.TangentU.y = 0.0f;
			v.TangentU.z = +radius * sinf(phi) * cosf(theta);

			v.TangentU.Normalize();

			Vector3 p = v.Position;
			v.Normal = p.Normalize();

			v.TexCoord.x = theta / XM_2PI;
			v.TexCoord.y = phi / XM_PI;

			meshData.Vertices.push_back(v);
		}
	}

	meshData.Vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (uint32 i = 1; i <= sliceCount; ++i)
	{
		meshData.Indices32.push_back(0);
		meshData.Indices32.push_back(i + 1);
		meshData.Indices32.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	uint32 baseIndex = 1;
	uint32 ringVertexCount = sliceCount + 1;
	for (uint32 i = 0; i < stackCount - 2; ++i)
	{
		for (uint32 j = 0; j < sliceCount; ++j)
		{
			meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j);
			meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			meshData.Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	uint32 southPoleIndex = (uint32)meshData.Vertices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (uint32 i = 0; i < sliceCount; ++i)
	{
		meshData.Indices32.push_back(southPoleIndex);
		meshData.Indices32.push_back(baseIndex + i);
		meshData.Indices32.push_back(baseIndex + i + 1);
	}

	m_MeshDataMap.emplace(name, std::move(meshData));
}

void CAssetMgr::CreateGeosphere(const std::wstring& name, float radius, uint32 numSubdivisions)
{
	MeshData meshData;

	// Put a cap on the number of subdivisions.
	numSubdivisions = std::min<uint32>(numSubdivisions, 6u);

	// Approximate a sphere by tessellating an icosahedron.

	const float X = 0.525731f;
	const float Z = 0.850651f;

	Vector3 pos[12] =
	{
		Vector3(-X, 0.0f, Z),  Vector3(X, 0.0f, Z),
		Vector3(-X, 0.0f, -Z), Vector3(X, 0.0f, -Z),
		Vector3(0.0f, Z, X),   Vector3(0.0f, Z, -X),
		Vector3(0.0f, -Z, X),  Vector3(0.0f, -Z, -X),
		Vector3(Z, X, 0.0f),   Vector3(-Z, X, 0.0f),
		Vector3(Z, -X, 0.0f),  Vector3(-Z, -X, 0.0f)
	};

	uint32 k[60] =
	{
		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
	};

	meshData.Vertices.resize(12);
	meshData.Indices32.assign(&k[0], &k[60]);

	for (uint32 i = 0; i < 12; ++i)
		meshData.Vertices[i].Position = pos[i];

	for (uint32 i = 0; i < numSubdivisions; ++i)
		Subdivide(meshData);

	// Project vertices onto sphere and scale.
	for (uint32 i = 0; i < meshData.Vertices.size(); ++i)
	{
		// Project onto unit sphere.
		Vector3 pos = meshData.Vertices[i].Position;
		Vector3 n = pos.Normalize();

		// Project onto sphere.
		Vector3 p = radius * n;

		meshData.Vertices[i].Position = p;
		meshData.Vertices[i].Normal = n;

		// Derive texture coordinates from spherical coordinates.
		float theta = atan2f(meshData.Vertices[i].Position.z, meshData.Vertices[i].Position.x);

		// Put in [0, 2pi].
		if (theta < 0.0f)
			theta += XM_2PI;

		float phi = acosf(meshData.Vertices[i].Position.y / radius);

		meshData.Vertices[i].TexCoord.x = theta / XM_2PI;
		meshData.Vertices[i].TexCoord.y = phi / XM_PI;

		// Partial derivative of P with respect to theta
		meshData.Vertices[i].TangentU.x = -radius * sinf(phi) * sinf(theta);
		meshData.Vertices[i].TangentU.y = 0.0f;
		meshData.Vertices[i].TangentU.z = +radius * sinf(phi) * cosf(theta);

		meshData.Vertices[i].TangentU.Normalize();
	}

	m_MeshDataMap.emplace(name, std::move(meshData));
}

void CAssetMgr::CreateCylinder(const std::wstring& name, float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount)
{
	MeshData meshData;

	float stackHeight = height / stackCount;

	float radiusStep = (topRadius - bottomRadius) / stackCount;
	uint32 ringCount = stackCount + 1;

	for (uint32 i = 0; i < ringCount; ++i)
	{
		float y = -0.5f * height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;
		float dTheta = 2 * XM_PI / sliceCount;
		for (uint32 j = 0; j <= sliceCount; ++j)
		{
			VertexMesh vertex;
			
			float c = cosf(j * dTheta);
			float s = sinf(j * dTheta);

			vertex.Position = Vector3(r * c, y, r * s);

			// In DirectX, v = 0 is the top of the image and v = 1 is bottom
			vertex.TexCoord.x = (float)j / sliceCount;
			vertex.TexCoord.y = 1.0f - (float)i / stackCount;

			// Cylinder can be parameterized as follows, where we introduce v
			// parameter that goes in the same direction as the v tex-coord
			// so that the bitangent goes in the same direction as the v tex-coord.
			//   Let r0 be the bottom radius and let r1 be the top radius.
			//   y(v) = h - hv for v in [0,1].
			//   r(v) = r1 + (r0-r1)v
			//
			//   x(t, v) = r(v)*cos(t)
			//   y(t, v) = h - hv
			//   z(t, v) = r(v)*sin(t)
			// 
			//  dx/dt = -r(v)*sin(t)
			//  dy/dt = 0
			//  dz/dt = +r(v)*cos(t)
			//
			//  dx/dv = (r0-r1)*cos(t)
			//  dy/dv = -h
			//  dz/dv = (r0-r1)*sin(t)

			// This is unit length.
			vertex.TangentU = Vector3(-s, 0.0f, c);

			float dr = bottomRadius - topRadius;
			Vector3 bitangent(dr * c, -height, dr * s);

			//Vector3 T = XMLoadFloat3(&vertex.TangentU);
			//Vector3 B = XMLoadFloat3(&bitangent);
			//Vector3 N = XMVector3Normalize(XMVector3Cross(T, B));
			//XMStoreFloat3(&vertex.Normal, N);

			vertex.Normal = (vertex.TangentU.Cross(bitangent)).Normalize();

			meshData.Vertices.push_back(std::move(vertex));
		}
	}

	// Add one because we duplicate the first and last vertex per ring
	// since the texture coordinates are different.
	uint32 ringVertexCount = sliceCount + 1;

	// Compute indices for each stack.
	for (uint32 i = 0; i < stackCount; ++i)
	{
		for (uint32 j = 0; j < sliceCount; ++j)
		{
			meshData.Indices32.push_back(i * ringVertexCount + j);
			meshData.Indices32.push_back((i + 1) * ringVertexCount + j);
			meshData.Indices32.push_back((i + 1) * ringVertexCount + j + 1);

			meshData.Indices32.push_back(i * ringVertexCount + j);
			meshData.Indices32.push_back((i + 1) * ringVertexCount + j + 1);
			meshData.Indices32.push_back(i * ringVertexCount + j + 1);
		}
	}


	BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
	BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);

	m_MeshDataMap.emplace(name, std::move(meshData));
}

void CAssetMgr::BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData)
{
	uint32 baseIndex = (uint32)meshData.Vertices.size();

	float y = 0.5f * height;
	float dTheta = 2.0f * XM_PI / sliceCount;

	// Duplicate cap ring vertices because the texture coordinates and normals differ.
	for (uint32 i = 0; i <= sliceCount; ++i)
	{
		float x = topRadius * cosf(i * dTheta);
		float z = topRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back(VertexMesh(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center vertex.
	meshData.Vertices.push_back(VertexMesh(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Index of center vertex.
	uint32 centerIndex = (uint32)meshData.Vertices.size() - 1;

	for (uint32 i = 0; i < sliceCount; ++i)
	{
		meshData.Indices32.push_back(centerIndex);
		meshData.Indices32.push_back(baseIndex + i + 1);
		meshData.Indices32.push_back(baseIndex + i);
	}
}

void CAssetMgr::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData)
{
	// 
	// Build bottom cap.
	//

	uint32 baseIndex = (uint32)meshData.Vertices.size();
	float y = -0.5f * height;

	// vertices of ring
	float dTheta = 2.0f * XM_PI / sliceCount;
	for (uint32 i = 0; i <= sliceCount; ++i)
	{
		float x = bottomRadius * cosf(i * dTheta);
		float z = bottomRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back(VertexMesh(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center vertex.
	meshData.Vertices.push_back(VertexMesh(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Cache the index of center vertex.
	uint32 centerIndex = (uint32)meshData.Vertices.size() - 1;

	for (uint32 i = 0; i < sliceCount; ++i)
	{
		meshData.Indices32.push_back(centerIndex);
		meshData.Indices32.push_back(baseIndex + i);
		meshData.Indices32.push_back(baseIndex + i + 1);
	}
}

void CAssetMgr::CreateGrid(const std::wstring& name, float width, float depth, uint32 m, uint32 n)
{
	MeshData meshData;

	uint32 vertexCount = m * n;
	uint32 faceCount = (m - 1) * (n - 1) * 2;

	//
	// Create the vertices.
	//

	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	meshData.Vertices.resize(vertexCount);
	for (uint32 i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for (uint32 j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			meshData.Vertices[i * n + j].Position = Vector3(x, 0.0f, z);
			meshData.Vertices[i * n + j].Normal = Vector3(0.0f, 1.0f, 0.0f);
			meshData.Vertices[i * n + j].TangentU = Vector3(1.0f, 0.0f, 0.0f);

			// Stretch texture over grid.
			meshData.Vertices[i * n + j].TexCoord.x = j * du;
			meshData.Vertices[i * n + j].TexCoord.y = i * dv;

			meshData.Vertices[i * n + j].TexCoord.x = j * du;
			meshData.Vertices[i * n + j].TexCoord.y = i * dv;
		}
	}

	//
	// Create the indices.
	//

	meshData.Indices32.resize(faceCount * 3); // 3 indices per face

	// Iterate over each quad and compute indices.
	uint32 k = 0;
	for (uint32 i = 0; i < m - 1; ++i)
	{
		for (uint32 j = 0; j < n - 1; ++j)
		{
			meshData.Indices32[k] = i * n + j;
			meshData.Indices32[k + 1] = i * n + j + 1;
			meshData.Indices32[k + 2] = (i + 1) * n + j;

			meshData.Indices32[k + 3] = (i + 1) * n + j;
			meshData.Indices32[k + 4] = i * n + j + 1;
			meshData.Indices32[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	m_MeshDataMap.emplace(name, std::move(meshData));
}

void CAssetMgr::CreateQuad(const std::wstring& name, float x, float y, float w, float h, float depth)
{
	MeshData meshData;

	meshData.Vertices.resize(4);
	meshData.Indices32.resize(6);

	// Position coordinates specified in NDC space.
	meshData.Vertices[0] = VertexMesh(
		x, y - h, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f);

	meshData.Vertices[1] = VertexMesh(
		x, y, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f);

	meshData.Vertices[2] = VertexMesh(
		x + w, y, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f);

	meshData.Vertices[3] = VertexMesh(
		x + w, y - h, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f);

	meshData.Indices32[0] = 0;
	meshData.Indices32[1] = 1;
	meshData.Indices32[2] = 2;

	meshData.Indices32[3] = 0;
	meshData.Indices32[4] = 2;
	meshData.Indices32[5] = 3;

	m_MeshDataMap.emplace(name, std::move(meshData));
}

VertexMesh CAssetMgr::MidPoint(const VertexMesh& v0, const VertexMesh& v1)
{
	Vector3 p0 = v0.Position;
	Vector3 p1 = v1.Position;

	Vector3 n0 = v0.Normal;
	Vector3 n1 = v1.Normal;

	Vector3 tan0 = v0.TangentU;
	Vector3 tan1 = v1.TangentU;

	Vector2 tex0 = v0.TexCoord;
	Vector2 tex1 = v1.TexCoord;

	// Compute the midpoints of all the attributes.  Vectors need to be normalized
	// since linear interpolating can make them not unit length.  
	VertexMesh v;
	v.Position = 0.5f * (p0 + p1);
	v.Normal = (0.5f * (n0 + n1)).Normalize();
	v.TangentU = (0.5f * (tan0 + tan1)).Normalize();
	v.TexCoord = 0.5f * (tex0 + tex1);

	return v;
}

void CAssetMgr::Subdivide(MeshData& meshData)
{
	// Save a copy of the input geometry.
	MeshData inputCopy = meshData;


	meshData.Vertices.resize(0);
	meshData.Indices32.resize(0);

	//       v1
	//       *
	//      / \
	//     /   \
	//  m0*-----*m1
	//   / \   / \
	//  /   \ /   \
	// *-----*-----*
	// v0    m2     v2

	uint32 numTris = (uint32)inputCopy.Indices32.size() / 3;
	for (uint32 i = 0; i < numTris; ++i)
	{
		VertexMesh v0 = inputCopy.Vertices[inputCopy.Indices32[i * 3 + 0]];
		VertexMesh v1 = inputCopy.Vertices[inputCopy.Indices32[i * 3 + 1]];
		VertexMesh v2 = inputCopy.Vertices[inputCopy.Indices32[i * 3 + 2]];

		//
		// Generate the midpoints.
		//

		VertexMesh m0 = MidPoint(v0, v1);
		VertexMesh m1 = MidPoint(v1, v2);
		VertexMesh m2 = MidPoint(v0, v2);

		//
		// Add new geometry.
		//

		meshData.Vertices.push_back(v0); // 0
		meshData.Vertices.push_back(v1); // 1
		meshData.Vertices.push_back(v2); // 2
		meshData.Vertices.push_back(m0); // 3
		meshData.Vertices.push_back(m1); // 4
		meshData.Vertices.push_back(m2); // 5

		meshData.Indices32.push_back(i * 6 + 0);
		meshData.Indices32.push_back(i * 6 + 3);
		meshData.Indices32.push_back(i * 6 + 5);

		meshData.Indices32.push_back(i * 6 + 3);
		meshData.Indices32.push_back(i * 6 + 4);
		meshData.Indices32.push_back(i * 6 + 5);

		meshData.Indices32.push_back(i * 6 + 5);
		meshData.Indices32.push_back(i * 6 + 4);
		meshData.Indices32.push_back(i * 6 + 2);

		meshData.Indices32.push_back(i * 6 + 3);
		meshData.Indices32.push_back(i * 6 + 1);
		meshData.Indices32.push_back(i * 6 + 4);
	}
}

