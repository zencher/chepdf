#include <cstdlib>
#include <cstdio>
#include <memory>

#ifdef WIN32
#include <windows.h>
#include <intrin.h>
#endif

#ifdef _MAC_OS_X_
#include <libkern/OSAtomic.h>
#endif

#include "../include/base.h"

namespace chepdf {

	class DefaultAllocator : public Allocator
	{
	public:
		inline void* Alloc(size_t cb) { return malloc(cb); }

		inline void Free(void* data) { free(data); }

		inline size_t GetSize(void * data)
		{
#ifdef WIN32
			return _msize(data);
#endif
#ifdef _LINUX_
			return malloc_usable_size(data);
#endif
#ifdef _MAC_OS_X_
			return malloc_size(data);
#endif
		}
	};

	DefaultAllocator gDefaultAllocator;

	BaseObject::BaseObject(Allocator * pAllocator)
	{
		if (pAllocator == nullptr)
		{
			pAllocator = Allocator::GetDefaultAllocator();
		}
		pAllocator_ = pAllocator;
	}
}