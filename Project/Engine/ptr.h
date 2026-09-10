#pragma once

template<typename T>
class Ptr
{
public:
	Ptr()
		: m_Asset(nullptr)
	{}

	Ptr(T* pAsset)
		: m_Asset(pAsset)
	{
		if (m_Asset != nullptr)
			m_Asset->AddRef();
	}

	Ptr(const Ptr<T>& _other)
		: m_Asset(_other.m_Asset)
	{
		if (m_Asset != nullptr)
			m_Asset->AddRef();
	}

	~Ptr()
	{
		if (m_Asset != nullptr)
			m_Asset->Release();
	}

	T* Get() const { return m_Asset; }
	T** GetAddressOf() const { return &m_Asset; }

	T** operator&() const { return &m_Asset; }

	T* operator->()
	{
		return m_Asset;
	}

	Ptr<T>& operator=(T* asset)
	{
		if (m_Asset != nullptr)
			m_Asset->Release();

		m_Asset = asset;

		if (m_Asset != nullptr)
			m_Asset->AddRef();

		return *this;
	}

	Ptr<T>& operator=(const Ptr<T>& asset)
	{
		if (m_Asset != nullptr)
			m_Asset->Release();

		m_Asset = asset.m_Asset;

		if (m_Asset != nullptr)
			m_Asset->AddRef();

		return *this;
	}

	bool operator==(T* asset)
	{
		return m_Asset == asset;
	}

	bool operator!=(T* asset)
	{
		return m_Asset != asset;
	}

	bool operator==(const Ptr<T>& asset)
	{
		return m_Asset == asset.m_Asset;
	}

	bool operator!=(const Ptr<T>& asset)
	{
		return m_Asset != asset.m_Asset;
	}

	friend bool operator==(T* asset1, const Ptr<T>& asset2);
	friend bool operator!=(T* asset1, const Ptr<T>& asset2);

private:
	T* m_Asset;
};

template<typename T>
inline bool operator==(T* asset1, const Ptr<T>& asset2)
{
	return asset1 == asset2.Get();
}

template<typename T>
inline bool operator!=(T* asset1, const Ptr<T>& asset2)
{
	return asset1 != asset2.Get();;
}
