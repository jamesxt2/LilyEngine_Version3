#pragma once

template<typename T, typename...Args>
struct TMemberBinding
{
	using FuncType = void(T::*)(Args...);

	T* pObject = nullptr;
	FuncType pFunc = nullptr;

	/*
	void Invoke(Args...args) const
	{
		if (pObject && pFunc)
			(pObject->*pFunc)(args...);
	}
	*/

	void operator()(Args...args) const
	{
		if (pObject && pFunc)
			(pObject->*pFunc)(args...);
	}
};

template<typename...Args>
class MulticastDelegate
{
public:
	using FuncType = std::function<void(Args...)>;

	struct Listener
	{
		void* pObject = nullptr;
		FuncType func;
	};

	template<typename T, typename...Args>
	void AddDynamic(T* obj, void(T::* func)(Args...))
	{

		TMemberBinding<T, Args...> binding{ obj, func };
		Listener l;
		l.pObject = binding.pObject;
		//l.func = [binding](Args...args) {binding.Invoke(args...); };
		l.func = std::move(binding);
		m_Listeners.push_back(std::move(l));
	}

	void Broadcast(Args...args)
	{
		for (auto& listener : m_Listeners)
		{
			if (listener.func)
				listener.func(args...);
		}
	}

	void RemoveAllOf(void* pObject)
	{
		if (pObject == nullptr) return;

		m_Listeners.erase(
			std::remove_if(m_Listeners.begin(), m_Listeners.end(),
				[pObject](const Listener& l) {return l.pObject == pObject; }),
			m_Listeners.end()
		);
	}

private:
	std::vector<Listener> m_Listeners;
};



/*

template<typename...Args>
class MulticastDelegate
{
public:
	using FuncType = std::function<void(Args...)>;

	struct Listener
	{
		void* pObject = nullptr;
		FuncType func;
	};

	void AddDynamic(void* pObject, FuncType func)
	{
		m_Listeners.push_back({ pObject, std::move(func) });
	}

	void Broadcast(Args...args)
	{
		for (auto& listener : m_Listeners)
		{
			if (listener.func)
				listener.func(args...);
		}
	}

	void RemoveAllOf(void* pObject)
	{
		if (pObject == nullptr) return;

		m_Listeners.erase(
			std::remove_if(m_Listeners.begin(), m_Listeners.end(),
				[pObject](const Listener& l) {return l.pObject == pObject; }),
			m_Listeners.end()
		);
	}

private:
	std::vector<Listener> m_Listeners;
};

template<typename T, typename...Args>
auto BindMember(T* obj, void(T::* func)(Args...))
{
	return [obj, func](Args...args) {(obj->*func)(args...); };
}


*/