#pragma once

#include <memory>
#include <cstddef>
#include <assert.h>
#include <mutex>

template <typename T>
class cached_object
{
public:
	virtual ~cached_object() {}

	void* operator new(size_t size)
	{
		std::lock_guard<std::mutex> l(lock);
		assert(size == sizeof(T));
		if (freestore == nullptr)
		{
			T* a = alloc.allocate(chunk);
			for (int i = 0; i < chunk; ++i)
				add_to_freelist(a + i);
		}

		T* p = freestore;
		freestore = freestore->cached_object<T>::_next;
		return p;
	}

	void operator delete(void* p, size_t size)
	{
		if (p)
		{
			std::lock_guard<std::mutex> l(lock);
			add_to_freelist((T*)p);
		}

	}
protected:
	T *_next;
private:
	static void add_to_freelist(T* p)	
	{
		p->cached_object<T>::_next = freestore;
		freestore = p;
	}

	static T* freestore;
	static std::allocator<T> alloc;
	static const std::size_t chunk;
	static std::mutex lock;
};

template <typename T> std::allocator<T> cached_object<T>::alloc;
template <typename T> T *cached_object<T>::freestore = nullptr;
template <typename T> const size_t cached_object<T>::chunk = 24;
template <typename T> std::mutex cached_object<T>::lock;

