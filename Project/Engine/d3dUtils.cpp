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

namespace Utilities
{
	void WStringToString(_In_ const std::wstring & wstr, _Out_ std::string & str)
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

	void StringToWString(_In_ const std::string & str, _Out_ std::wstring & wstr)
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

	std::string WStringToString(_In_ const std::wstring & wstr)
	{
		std::string result = {};
		if (wstr.empty())
			return result;

		int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
		result.resize((const size_t)len - 1, '\0');
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, result.data(), len, nullptr, nullptr);
		return result;
	}

	std::wstring StringToWString(_In_ const std::string & str)
	{
		std::wstring result = {};
		if (str.empty())
			return result;

		int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
		result.resize((const size_t)len - 1, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, result.data(), len);
		return result;
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

	Vector4 SphericalToCartesian(float radius, float theta, float phi)
	{
		return Vector4(
			radius * sinf(phi) * cosf(theta),
			radius * cosf(phi),
			radius * sinf(phi) * sinf(theta),
			1.0f);
	}
}
