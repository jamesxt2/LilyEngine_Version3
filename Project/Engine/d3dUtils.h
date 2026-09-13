#pragma once

class DxException
{
public:
    DxException() = default;
    DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

    std::wstring ToString()const;

    HRESULT ErrorCode = S_OK;
    std::wstring FunctionName;
    std::wstring Filename;
    int LineNumber = -1;
};

void WStringToString(_In_ const std::wstring& wstr, _Out_ std::string& str);
void StringToWString(_In_ const std::string& str, _Out_ std::wstring& wstr);
std::string WStringToString(_In_ const std::wstring& wstr);
std::wstring StringToWString(_In_ const std::string& str);

ComPtr<ID3D12Resource> CreateDefaultBuffer(const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource>& uploadBuffer);

template<typename T, int size>
void Safe_Del_Array(T* (&Array)[size])
{
	for (int i = 0; i < size; ++i)
	{
		if (Array[i] != nullptr)
		{
			delete Array[i];
			Array[i] = nullptr;
		}
	}
}

template<typename T>
void Safe_Del_Vector(std::vector<T*>& vec)
{
	for (size_t i = 0; i < vec.size(); ++i)
	{
		if (vec[i] != nullptr)
			delete vec[i];
	}
	vec.clear();
}

template<typename T1, typename T2>
void Safe_Del_Map(std::map<T1, T2>& _map)
{
	for (const auto& pair : _map)
	{
		if (pair.second != nullptr)
			delete pair.second;
	}
	_map.clear();
}