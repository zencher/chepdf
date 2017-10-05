#ifndef _CHE_BASE_STRING_
#define _CHE_BASE_STRING_

#include "che_base_object.h"

namespace chepdf {

struct ByteStringData;

class ByteString : public BaseObject
{
public:
	ByteString(Allocator * allocator) : BaseObject(allocator), data_(nullptr) {}

	ByteString() : BaseObject(nullptr)
	{
		Clear();
	}

	ByteString(char ch, Allocator * allocator = nullptr);
	ByteString(const char * pstr, uint32_t length = 0, Allocator * allocator = nullptr);
	ByteString(const ByteString& str);

	ByteString & operator=(char ch);
	ByteString & operator=(const char * pstr);
	ByteString & operator=(const ByteString& str);

	bool operator==(char ch) const;
	bool operator==(char const * pstr) const;
	bool operator==(const ByteString & str) const;
	friend bool operator==(char ch, ByteString & str);
	friend bool operator==(char * pstr, ByteString & str);

	ByteString operator+(char ch);
	ByteString operator+(const char * pstr);
	ByteString operator+(const ByteString & str);
	friend ByteString operator+(char ch, ByteString & str);
	friend ByteString operator+(const char * pstr, ByteString & str);

	ByteString & operator+=(char ch);
	ByteString & operator+=(const char * pstr);
	ByteString & operator+=(const ByteString & str);

	bool operator!=(char ch) const;
	bool operator!=(const char * pstr) const;
	bool operator!=(const ByteString& str) const;
	friend bool operator!=(char ch, ByteString & str);
	friend bool operator!=(const char * pstr, ByteString & str);

	char operator[](uint32_t index) const;

	uint32_t GetLength() const;

	bool SetData(uint8_t * data, uint32_t length);
	char const * GetData() const;

	INT32 GetInteger() const;
	FLOAT GetFloat() const;

	void Clear();

private:
	ByteStringData * data_;
};

bool operator==(char ch, ByteString & str);
bool operator==(const char * pstr, ByteString & str);

ByteString operator+(char ch, ByteString & str);
ByteString operator+(const char * pstr, ByteString & str);

bool operator!=(char ch, ByteString & str);
bool operator!=(const char * pstr, ByteString & str);


struct WideStringData;

class WideString : public BaseObject
{
public:
	WideString(Allocator * allocator) : BaseObject(allocator), data_(nullptr) {}

	WideString() : BaseObject(nullptr)
	{
		Clear();
	}

	WideString(wchar_t wch, Allocator * allocator = nullptr);
	WideString(const wchar_t * pstr, uint32_t nStrSize = 0, Allocator * allocator = nullptr);
	WideString(const WideString & str);

	WideString & operator=(wchar_t wch);
	WideString & operator=(const wchar_t * pstr);
	WideString & operator=(const WideString & str);

	bool operator==(wchar_t wch) const;
	bool operator==(const wchar_t * pstr) const;
	bool operator==(const WideString & str) const;
	friend bool operator==(wchar_t wch, WideString & str);
	friend bool operator==(const wchar_t * pstr, WideString & str);

	WideString operator+(const WideString & str);
	WideString operator+(wchar_t wch);
	WideString operator+(const wchar_t * pstr);
	friend WideString operator+(wchar_t wch, WideString & str);
	friend WideString operator+(const wchar_t * pstr, WideString & str);

	WideString & operator+=(const WideString & str);
	WideString & operator+=(wchar_t wch);
	WideString & operator+=(const wchar_t * pstr);

	bool operator!=(const WideString & wstr) const;
	bool operator!=(wchar_t wch) const;
	bool operator!=(const wchar_t * pstr) const;
	friend bool operator!=(wchar_t wch, WideString & str);
	friend bool operator!=(const wchar_t * pstr, WideString & str);

	wchar_t operator[](uint32_t index) const;

	uint32_t GetLength() const;

	bool SetData(wchar_t * pData, uint32_t length);
	wchar_t const *  GetData() const;

	int32_t GetInteger() const;
	FLOAT GetFloat() const;

	void Clear();

private:
	WideStringData * data_;
};

bool operator==(wchar_t wch, WideString & str);
bool operator==(wchar_t * pstr, WideString & str);

WideString operator+(wchar_t wch, WideString & str);
WideString operator+(const wchar_t * pstr, WideString & str);

bool operator!=(wchar_t wch, WideString & str);
bool operator!=(const wchar_t *  lstr, WideString & str);

#define A(a)		(a)
#define B(a,b)		(a | b<<8)
#define C(a,b,c)	(a | b<<8 | c<<16)
#define D(a,b,c,d)	(a | b<<8 | c<<16 | d<<24)

UINT32 StringToUINT32(const ByteString & str);

}//namespace

#endif
