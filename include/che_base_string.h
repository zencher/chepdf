#ifndef _CHE_BASE_STRING_
#define _CHE_BASE_STRING_

#include "che_base_object.h"

namespace chepdf {

struct ByteStringData;

class ByteString : public BaseObject
{
public:
	ByteString(Allocator * allocator = nullptr) : BaseObject(allocator), data_(nullptr) {}

	ByteString() : BaseObject(nullptr)
	{
		Clear();
	}

	ByteString(char ch, Allocator * allocator = nullptr);
	ByteString(char const * pstr, size_t length = 0, Allocator * allocator = nullptr);
	ByteString(const ByteString& str);

	ByteString & operator=(char ch);
	ByteString & operator=(char const * pstr);
	ByteString & operator=(const ByteString& str);

	bool operator==(char ch) const;
	bool operator==(char const * pstr) const;
	bool operator==(const ByteString & str) const;
	friend bool operator==(char ch, ByteString & str);
	friend bool operator==(char * pstr, ByteString & str);

	ByteString operator+(char ch);
	ByteString operator+(char const * pstr);
	ByteString operator+(const ByteString & str);
	friend ByteString operator+(char ch, ByteString & str);
	friend ByteString operator+(char const * pstr, ByteString & str);

	ByteString & operator+=(char ch);
	ByteString & operator+=(char const * pstr);
	ByteString & operator+=(const ByteString & str);

	bool operator!=(char ch) const;
	bool operator!=(char const * pstr) const;
	bool operator!=(const ByteString& str) const;
	friend bool operator!=(char ch, ByteString & str);
	friend bool operator!=(char const * pstr, ByteString & str);

	char operator[](size_t index) const;

	size_t GetLength() const;

	bool SetData(BYTE * data, size_t length);
	char const * GetData() const;

	INT32 GetInteger() const;
	FLOAT GetFloat() const;

	void Clear();

private:
	ByteStringData * data_;
};

bool operator==(char ch, ByteString & str);
bool operator==(char const * pstr, ByteString & str);

ByteString operator+(char ch, ByteString & str);
ByteString operator+(char const * pstr, ByteString & str);

bool operator!=(char ch, ByteString & str);
bool operator!=(char const * pstr, ByteString & str);


struct WideStringData;

class WideString : public BaseObject
{
public:
	WideString(Allocator * allocator = nullptr) : BaseObject(allocator), data_(nullptr) {}

	WideString() : BaseObject(nullptr)
	{
		Clear();
	}

	WideString(wchar_t wch, Allocator * allocator = nullptr);
	WideString(wchar_t const * pstr, size_t nStrSize = 0, Allocator * allocator = nullptr);
	WideString(const WideString & str);

	WideString & operator=(wchar_t wch);
	WideString & operator=(wchar_t const * pstr);
	WideString & operator=(const WideString & str);

	bool operator==(wchar_t wch) const;
	bool operator==(wchar_t const * pstr) const;
	bool operator==(const WideString & str) const;
	friend bool operator==(wchar_t wch, WideString & str);
	friend bool operator==(wchar_t const * pstr, WideString & str);

	WideString operator+(const WideString & str);
	WideString operator+(wchar_t wch);
	WideString operator+(wchar_t const * pstr);
	friend WideString operator+(wchar_t wch, WideString & str);
	friend WideString operator+(wchar_t const * pstr, WideString & str);

	WideString & operator+=(const WideString & str);
	WideString & operator+=(wchar_t wch);
	WideString & operator+=(wchar_t const * pstr);

	bool operator!=(const WideString & wstr) const;
	bool operator!=(wchar_t wch) const;
	bool operator!=(wchar_t const * pstr) const;
	friend bool operator!=(wchar_t wch, WideString & str);
	friend bool operator!=(wchar_t const * pstr, WideString & str);

	wchar_t operator[](size_t index) const;

	size_t GetLength() const;

	bool SetData(wchar_t * pData, size_t length);
	wchar_t const *  GetData() const;

	INT32 GetInteger() const;
	FLOAT GetFloat() const;

	void Clear();

private:
	WideStringData * data_;
};

bool operator==(wchar_t wch, WideString & str);
bool operator==(wchar_t * pstr, WideString & str);

WideString operator+(wchar_t wch, WideString & str);
WideString operator+(wchar_t const * pstr, WideString & str);

bool operator!=(wchar_t wch, WideString & str);
bool operator!=(wchar_t const *  lstr, WideString & str);

#define A(a)		(a)
#define B(a,b)		(a | b<<8)
#define C(a,b,c)	(a | b<<8 | c<<16)
#define D(a,b,c,d)	(a | b<<8 | c<<16 | d<<24)

UINT32 StringToUINT32(const ByteString & str);

}//namespace

#endif
