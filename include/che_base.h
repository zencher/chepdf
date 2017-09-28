#ifndef _CHEPDF_BASE_H_
#define _CHEPDF_BASE_H_

#include "che_define.h"

#include <new>
#include <cstdlib>

#ifdef _MAC_OS_X_
#include <malloc/malloc.h>
#include <pthread.h>
#else
#include <malloc.h>
#endif

#ifdef WIN32
#include <windows.h>
#undef GetObject
#endif

#if _MSC_VER>=1800
#define CPP_VARIADIC_TEMPLATE
#endif

namespace chepdf
{
	class Allocator
	{
	public:
		static Allocator * GetDefaultAllocator();

		virtual ~Allocator() {}
		virtual void* Alloc(size_t cb) = 0;
		virtual void Free(void* data) = 0;
		virtual size_t GetSize(void* data) = 0;

		template <class Type>
		inline Type* New()
		{
			void* obj = Alloc(sizeof(Type));
			return new(obj) Type;
		}

		template <class Type>
		inline Type* NewArray(size_t count)
		{
			void* obj = Alloc(sizeof(Type) * count);
			size_t relCount = GetSize(obj) / sizeof(Type);
			Type * pTmp = (Type *)obj;
			for (size_t i = 0; i < relCount; i++)
			{
				new((void *)pTmp) Type;
				pTmp++;
			}
			return (Type *)obj;
		}

		template <class Type>
		inline void Delete(Type * p)
		{
			((Type*)p)->~Type();
			Free(p);
		}

		template <class Type>
		inline void DeleteArray(Type * p)
		{
			size_t count = GetSize(p) / sizeof(Type);
			Type * pTmp = p;
			for (size_t i = 0; i < count; i++)
			{
				((Type*)pTmp)->~Type();
				pTmp++;
			}
			Free(p);
		}

#ifdef  CPP_VARIADIC_TEMPLATE
		template <class Type, typename... Args>
		inline Type * New(Args... args)
		{
			void* obj = Alloc(sizeof(Type));
			return new(obj) Type(args...);
		}
#else
		template <class Type, class Arg1>
		inline Type* New(Arg1 arg1)
		{
			void* obj = Alloc(sizeof(Type));
			return new(obj) Type(arg1);
		}

		template <class Type, class Arg1, class Arg2>
		inline Type* New(Arg1 arg1, Arg2 arg2)
		{
			void* obj = Alloc(sizeof(Type));
			return new(obj) Type(arg1, arg2);
		}

		template <class Type, class Arg1, class Arg2, class Arg3>
		inline Type* New(Arg1 arg1, Arg2 arg2, Arg3 arg3)
		{
			void* obj = Alloc(sizeof(Type));
			return new(obj) Type(arg1, arg2, arg3);
		}

		template <class Type, class Arg1, class Arg2, class Arg3, class Arg4>
		inline Type* New(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
		{
			void* obj = Alloc(sizeof(Type));
			return new(obj) Type(arg1, arg2, arg3, arg4);
		}

		template <class Type, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
		inline Type* New(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5)
		{
			void* obj = Alloc(sizeof(Type));
			return new(obj) Type(arg1, arg2, arg3, arg4, arg5);
		}

		template <class Type, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6>
		inline Type* New(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6)
		{
			void* obj = Alloc(sizeof(Type));
			return new(obj) Type(arg1, arg2, arg3, arg4, arg5, arg6);
		}

		template <class Type, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7>
		inline Type* New(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7)
		{
			void* obj = Alloc(sizeof(Type));
			return new(obj) Type(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
		}

		template <class Type, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8>
		inline Type* New(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8)
		{
			void* obj = Alloc(sizeof(Type));
			return new(obj) Type(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
		}

		template <class Type, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class Arg9>
		inline Type* New(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9)
		{
			void* obj = Alloc(sizeof(Type));
			return new(obj) Type(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
		}

		template <class Type, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class Arg9, class Arg10>
		inline Type* New(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4, Arg5 arg5, Arg6 arg6, Arg7 arg7, Arg8 arg8, Arg9 arg9, Arg10 arg10)
		{
			void* obj = Alloc(sizeof(Type));
			return new(obj) Type(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
		}
#endif
	};


	class BaseObject
	{
	public:
		BaseObject(Allocator * pAllocator);

		Allocator * GetAllocator() const { return pAllocator_; }

	private:
		Allocator * pAllocator_;
	};


	class IWrite : public BaseObject
	{
	public:
		enum FILEWRITE_MODE{
			FILEWRITE_MODE_NEW,
			FILEWRITE_MODE_OPEN
		};

		static IWrite * CreateCrtFileIWrite(char * const filename, FILEWRITE_MODE mode, Allocator * pAllocator);
		static void DestroyIWrite(IWrite * pIWrite);

		IWrite(Allocator * pAllocator) : BaseObject(pAllocator) {};
		virtual ~IWrite() {};

		virtual size_t GetSize() = 0;
		virtual size_t GetCurOffset() = 0;
		virtual size_t Flush() = 0;

		virtual	bool WriteBlock(const void * pData, size_t offset, size_t size) = 0;
		virtual	bool WriteBlock(const void * pData, size_t size) { return WriteBlock(pData, GetSize(), size); }
		virtual bool WriteByte(BYTE byte) { return WriteBlock(&byte, 1); }

		virtual void Release() = 0;
	};


	class IRead : public BaseObject
	{
	public:
		enum FILEREAD_MODE {
			FILEREAD_MODE_DEFAULT,
			FILEREAD_MODE_COPYTOMEMORY
		};

		static IRead * CreateCrtFileIRead(char const * filename, FILEREAD_MODE mode, Allocator * pAllocator);
		static IRead * CreateMemoryIRead(PCBYTE pMemory, size_t size, Allocator * pAllocator);
		static void DestroyIRead(IRead * pIRead);

		IRead(Allocator * pAllocator) : BaseObject(pAllocator) {}
		virtual ~IRead() {};

		virtual size_t GetSize() = 0;
		virtual size_t ReadBlock(void * buffer, size_t offset, size_t size) = 0;
		virtual bool ReadByte(size_t offset, BYTE & byte) = 0;
		virtual void Release() = 0;
	};

	class ReferenceCount
	{
	public:
		ReferenceCount() : referenceCount_(0) {}

		inline operator size_t() { return referenceCount_; }

		inline void	AddRef();
		inline void DecRef();

	private:
#ifdef _MAC_OS_X_
		INT32 referenceCount_;
#else
		long referenceCount_;
#endif
	};


	class MutexLock
	{
	public:
		MutexLock();
		~MutexLock();

		void Lock();
		void UnLock();

#ifdef WIN32
		HANDLE	mutex_;
#endif

#ifdef _MAC_OS_X_
		pthread_mutex_t mutex_;
#endif
	};
};

#endif
