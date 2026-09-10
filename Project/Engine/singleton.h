#pragma once

template<typename T>
class CSingleton
{
public:
	static T* GetInst()
	{
		if (m_This == nullptr)
			m_This = new T;
		return m_This;
	}

	static void Destroy()
	{
		if (m_This != nullptr)
		{
			delete m_This;
			m_This = nullptr;
		}
	}


protected:
	CSingleton()
	{
		atexit((DESTROY)&CSingleton<T>::Destroy);
	}
	CSingleton(const CSingleton& _other) = delete;
	~CSingleton() = default;


private:
	static T* m_This;
	typedef void(*DESTROY)(void);
};

template<typename T>
T* CSingleton<T>::m_This = nullptr;