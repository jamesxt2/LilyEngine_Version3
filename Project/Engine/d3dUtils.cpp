#include "pch.h"
#include "d3dUtils.h"

#include "CDevice.h"

DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber)
	: ErrorCode(hr),
    FunctionName(functionName),
    Filename(filename),
    LineNumber(lineNumber)
{
}

std::wstring DxException::ToString() const
{
    // Get the string description of the error code.
    _com_error err(ErrorCode);
    std::wstring msg = err.ErrorMessage();

    return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
}

void WStringToString(_In_ const std::wstring& wstr, _Out_ std::string& str)
{
	if (wstr.empty())
	{
		str = std::string();
		return;
	}
	int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	str.resize((const size_t)len - 1, '\0');
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, str.data(), len, nullptr, nullptr);
}

void StringToWString(_In_ const std::string& str, _Out_ std::wstring& wstr)
{
	if (str.empty())
	{
		wstr = std::wstring();
		return;
	}
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	wstr.resize((const size_t)len - 1, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr.data(), len);
}

std::string WStringToString(_In_ const std::wstring& wstr)
{
	std::string result = {};
	if (wstr.empty())
		return result;

	int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	result.resize((const size_t)len - 1, '\0');
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, result.data(), len, nullptr, nullptr);
	return result;
}

std::wstring StringToWString(_In_ const std::string& str)
{
	std::wstring result = {};
	if (str.empty())
		return result;

	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	result.resize((const size_t)len - 1, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, result.data(), len);
	return result;
}

ComPtr<ID3D12Resource> CreateDefaultBuffer(const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource>& uploadBuffer)
{
	ComPtr<ID3D12Resource> defaultBuffer;

	// Create actual default buffer resource
	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC desc(CD3DX12_RESOURCE_DESC::Buffer(byteSize));
	ThrowIfFailed(DEVICE->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf())
	));

	// To copy the data from CPU memory to default buffer, we need to create an intermediate upload buffer
	heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	ThrowIfFailed(DEVICE->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())
	));

	// describe the data we want to copy to the default buffer
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = byteSize;

	// copy
	CD3DX12_RESOURCE_BARRIER barrier1(CD3DX12_RESOURCE_BARRIER::Transition(
		defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST
	));
	CMDLIST->ResourceBarrier(1, &barrier1);
	UpdateSubresources<1>(
		CMDLIST.Get(), defaultBuffer.Get(), uploadBuffer.Get(),
		0, 0, 1, &subResourceData
	);
	CD3DX12_RESOURCE_BARRIER barrier2(CD3DX12_RESOURCE_BARRIER::Transition(
		defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_GENERIC_READ
	));
	CMDLIST->ResourceBarrier(1, &barrier2);

	return defaultBuffer;
}

int Rand(int a, int b)
{
	return a + rand() % ((b - a) + 1);
}

float RandF()
{
	return (float)(rand()) / (float)RAND_MAX;
}

float RandF(float a, float b)
{
	return a + RandF() * (b - a);
}
