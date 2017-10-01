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

#include "../include/che_base_object.h"

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

Allocator * Allocator::GetDefaultAllocator()
{
    return &gDefaultAllocator;
}

BaseObject::BaseObject(Allocator * allocator)
{
    if (allocator == nullptr)
    {
        allocator = Allocator::GetDefaultAllocator();
    }
    allocator_ = allocator;
}



IWrite * IWrite::CreateCrtFileIWrite(char * const filename, FILEWRITE_MODE mode, Allocator * allocator)
{
    return nullptr;
}

void IWrite::DestroyIWrite(IWrite * pIWrite)
{
    if (pIWrite != nullptr)
    {
        pIWrite->Release();
        pIWrite->GetAllocator()->Delete<IWrite>(pIWrite);
    }
}

class ICrtFileWrite : public IWrite
{
public:
    ICrtFileWrite(char * const filename, FILEWRITE_MODE mode, Allocator * allocator);
    virtual ~ICrtFileWrite();

    virtual size_t GetSize();
    virtual size_t GetCurOffset();
    virtual size_t Flush();
    virtual	bool WriteBlock(const void * pData, size_t offset, size_t size);
    virtual void Release();

private:
    FILE * pFile_;
};

ICrtFileWrite::ICrtFileWrite(char * const filename, FILEWRITE_MODE mode, Allocator * allocator)
    : IWrite(allocator), pFile_(nullptr)
{
    if (filename != nullptr)
    {
        if (mode == FILEWRITE_MODE_NEW)
        {
            pFile_ = fopen(filename, "wb+");
        }
        else if (mode == FILEWRITE_MODE_OPEN) {
            pFile_ = fopen(filename, "rb+");
        }
    }
}

ICrtFileWrite::~ICrtFileWrite()
{
    if (pFile_ != nullptr)
    {
        fclose(pFile_);
        pFile_ = nullptr;
    }
}

size_t ICrtFileWrite::GetSize()
{
    if (pFile_)
    {
        fseek(pFile_, 0, SEEK_END);
        return ftell(pFile_);
    } else {
        return 0;
    }
}

size_t ICrtFileWrite::GetCurOffset()
{
    return GetSize();
}

size_t ICrtFileWrite::Flush()
{
    if (pFile_)
    {
        return fflush(pFile_);
    } else {
        return 0;
    }
}

void ICrtFileWrite::Release()
{
    if (pFile_)
    {
        fflush(pFile_);
        fclose(pFile_);
        pFile_ = nullptr;
    }
}

bool ICrtFileWrite::WriteBlock(const void * pData, size_t offset, size_t size)
{
    if (pData == nullptr || size == 0)
    {
        return false;
    }
    if (pFile_)
    {
        fseek(pFile_, offset, SEEK_SET);
        size_t ret = fwrite(pData, 1, size, pFile_);
        if (ret > 0)
        {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}


class ICrtFileReadDefault : public IRead
{
public:
    ICrtFileReadDefault(char const * filename, Allocator * allocator);
    virtual ~ICrtFileReadDefault();

    virtual size_t GetSize();
    virtual size_t ReadBlock(void * buffer, size_t offset, size_t size);
    virtual bool ReadByte(size_t offset, BYTE & byte);
    virtual void Release();

private:
    FILE * pFile_;
};

class ICrtFileReadMemoryCopy : public IRead
{
public:
    ICrtFileReadMemoryCopy(char const * filename, Allocator * allocator);
    virtual ~ICrtFileReadMemoryCopy();

    virtual size_t GetSize();
    virtual size_t ReadBlock(void * buffer, size_t offset, size_t size);
    virtual bool ReadByte(size_t offset, BYTE & byte);
    virtual void Release();

private:
    PBYTE pBuf_;
    size_t bufSize_;
};

ICrtFileReadDefault::ICrtFileReadDefault(char const * filename, Allocator * allocator)
    : IRead(allocator), pFile_(nullptr)
{
    if (filename != nullptr)
    {
        pFile_ = fopen(filename, "rb");
    }
}

ICrtFileReadDefault::~ICrtFileReadDefault()
{
    if (pFile_)
    {
        fclose(pFile_);
        pFile_ = nullptr;
    }
}

size_t ICrtFileReadDefault::GetSize()
{
    if (pFile_)
    {
        fseek(pFile_, 0, SEEK_END);
        return ftell(pFile_);
    } else {
        return 0;
    }
}

size_t ICrtFileReadDefault::ReadBlock(void * buffer, size_t offset, size_t size)
{
    if (buffer == nullptr)
    {
        return 0;
    }
    if (pFile_)
    {
        fseek(pFile_, offset, SEEK_SET);
        size_t ret = fread(buffer, 1, size, pFile_);
        return ret;
    }
    return 0;
}

bool ICrtFileReadDefault::ReadByte(size_t offset, BYTE & byte)
{
    if (pFile_)
    {
        if (fseek(pFile_, offset, SEEK_SET) == -1)
        {
            return false;
        } else {
            size_t ret = fread(&byte, 1, 1, pFile_);
            if (ret == 1)
            {
                return true;
            }
        }
    }
    return false;
}

void ICrtFileReadDefault::Release()
{
    if (pFile_)
    {
        fclose(pFile_);
        pFile_ = nullptr;
    }
}


ICrtFileReadMemoryCopy::ICrtFileReadMemoryCopy(char const * filename, Allocator * allocator)
    : IRead(allocator), pBuf_(nullptr), bufSize_(0)
{
    FILE * pFile = fopen(filename, "rb");
    if (pFile)
    {
        fseek(pFile, 0, SEEK_END);
        bufSize_ = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);
        pBuf_ = GetAllocator()->NewArray<BYTE>(bufSize_);
        fread(pBuf_, 1, bufSize_, pFile);
        fclose(pFile);
    }
}

ICrtFileReadMemoryCopy::~ICrtFileReadMemoryCopy()
{
    if (pBuf_)
    {
        GetAllocator()->DeleteArray<BYTE>(pBuf_);
    }
}

size_t ICrtFileReadMemoryCopy::ReadBlock(void * buffer, size_t offset, size_t size)
{
    if (offset + size < bufSize_)
    {
        memcpy(buffer, pBuf_ + offset, size);
        return size;
    } else {
        size = bufSize_ - offset;
        memcpy(buffer, pBuf_ + offset, size);
        return size;
    }
    return 0;
}

bool ICrtFileReadMemoryCopy::ReadByte(size_t offset, BYTE & byte)
{
    if (offset < bufSize_)
    {
        byte = pBuf_[offset];
        return true;
    }
    return false;
}

class IMemoryRead : public IRead
{
public:
    IMemoryRead(PCBYTE pBuf, size_t size, Allocator * allocator)
        : IRead(allocator), pBuf_(nullptr), bufSize_(0) {}
    virtual ~IMemoryRead() {};

    virtual size_t GetSize() { return bufSize_; }
    virtual size_t ReadBlock(void * buffer, size_t offset, size_t size);
    virtual bool ReadByte(size_t offset, BYTE & byte);
    virtual void Release() { pBuf_ = nullptr; bufSize_ = 0; }

private:
    PCBYTE	pBuf_;
    size_t	bufSize_;
};

size_t IMemoryRead::ReadBlock(void * buffer, size_t offset, size_t size)
{
    if (buffer == NULL || pBuf_ == NULL || size == 0)
    {
        return 0;
    }
    if (offset < bufSize_)
    {
        if (offset + size > bufSize_)
        {
            size = bufSize_ - offset;
        }
        memcpy(buffer, pBuf_ + offset, size);
        return size;
    }
    return 0;
}

bool IMemoryRead::ReadByte(size_t offset, BYTE & byte)
{
    if (offset < bufSize_)
    {
        byte = pBuf_[offset];
        return true;
    }
    return false;
}

IRead * IRead::CreateCrtFileIRead(char const * filename, FILEREAD_MODE mode, Allocator * allocator)
{
    if (allocator == nullptr)
    {
        allocator = Allocator::GetDefaultAllocator();
    }
    if (filename != nullptr)
    {
        switch (mode)
        {
        case FILEREAD_MODE_DEFAULT:
            return allocator->New<ICrtFileReadDefault>(filename, allocator);
        case FILEREAD_MODE_COPYTOMEMORY:
            return allocator->New<ICrtFileReadMemoryCopy>(filename, allocator);
        default:
            break;
        }
    }
    return nullptr;
}

IRead * IRead::CreateMemoryIRead(PCBYTE pMemory, size_t size, Allocator * pAllocator)
{
    if (pAllocator == nullptr)
    {
        pAllocator = Allocator::GetDefaultAllocator();
    }
    if (pMemory != nullptr || size == 0)
    {
        return pAllocator->New<IMemoryRead>(pMemory, size, pAllocator);
    }
    return nullptr;
}

void IRead::DestroyIRead(IRead * pIRead)
{
    if (pIRead != nullptr)
    {
        pIRead->GetAllocator()->Delete<IRead>(pIRead);
    }
}


inline void ReferenceCount::Increase()
{
#ifdef WIN32
    _InterlockedIncrement(&referenceCount_);
#endif

#ifdef _MAC_OS_X_
    OSAtomicIncrement32(&referenceCount_);
#endif
}

inline void ReferenceCount::Decrease()
{
#ifdef WIN32
    _InterlockedDecrement(&referenceCount_);
#endif

#ifdef _MAC_OS_X_
    OSAtomicDecrement32(&referenceCount_);
#endif
}


MutexLock::MutexLock()
{
#ifdef WIN32
    mutex_ = CreateMutex(NULL, FALSE, NULL);
#endif

#ifdef _MAC_OS_X_
    pthread_mutex_init(&mutex_, nullptr);
#endif
}

MutexLock::~MutexLock()
{
#ifdef WIN32
    if (mutex_)
    {
        CloseHandle(mutex_);
        mutex_ = NULL;
    }
#endif

#ifdef _MAC_OS_X_
    pthread_mutex_destroy(&mutex_);
#endif
}

void MutexLock::Lock()
{
#ifdef WIN32
    WaitForSingleObject(mutex_, INFINITE);
#endif

#ifdef _MAC_OS_X_
    pthread_mutex_lock(&mutex_);
#endif
}

void MutexLock::UnLock()
{
#ifdef WIN32
    ReleaseMutex(mutex_);
#endif

#ifdef _MAC_OS_X_
    pthread_mutex_unlock(&mutex_);
#endif
}
    
}//namespace
