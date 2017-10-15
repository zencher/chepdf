#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <memory>

#include "../include/che_base_string.h"

//@zctodo:optimize:simpleness:for,class,ByteString:"make ByteStringData a class, maintain the refcount and data by itself";
//@zctodo:optimize:performance:for,class,ByteString:"reduce memory copy times";
//@zctodo:optimize:simpleness:for,class,WideString:"make WideStringData a class, maintain the refcount and data by itself"
//@zctodo:optimize:performance:for,class,WideString:"reduce memory copy times";

namespace chepdf
{

struct ByteStringData
{
	ByteStringData()
	{
		str_ = nullptr;
		length_ = 0;
	}
	char *          str_;
	uint32_t        length_;
	ReferenceCount  reference_;
};

ByteString::ByteString(char ch, Allocator * allocator)
	:BaseObject(allocator)
{
	if (ch == '\0')
	{
		data_ = nullptr;
	}
	else{
		data_ = GetAllocator()->New<ByteStringData>();
		data_->length_ = 1;
		data_->reference_.Increase();
		data_->str_[0] = ch;
		data_->str_ = GetAllocator()->NewArray<char>(2);
		data_->str_[1] = '\0';
	}
}

ByteString::ByteString(char const * pstr, uint32_t length /* = 0 */, Allocator * allocator /*= nullptr*/)
	:BaseObject(allocator)
{
	if (pstr == nullptr)
	{
		data_ = nullptr;
		return;
	}
	if (length == 0)
	{
		length = (uint32_t)strlen(pstr);
	}
	if (length > 0)
	{
		data_ = GetAllocator()->New<ByteStringData>();
		data_->reference_.Increase();
		data_->length_ = length;
		data_->str_ = GetAllocator()->NewArray<char>(length + 1);
		memcpy(data_->str_, pstr, length);
		data_->str_[length] = '\0';
	}
	else{
		data_ = nullptr;
	}
}

ByteString::ByteString(const ByteString & str)
	:BaseObject(str.GetAllocator())
{
	if (str.data_ == nullptr)
	{
		data_ = nullptr;
	}
	else{
		data_ = str.data_;
		str.data_->reference_.Increase();
	}
}

void ByteString::Clear()
{
	if (data_)
	{
		data_->reference_.Decrease();

		if (data_->reference_ == 0 && data_->str_)
		{
			GetAllocator()->DeleteArray<char>(data_->str_);
			data_->str_ = nullptr;
			GetAllocator()->Delete<ByteStringData>(data_);
		}
		data_ = nullptr;
	}
}

ByteString & ByteString::operator=(char ch)
{
	Clear();
	if (ch == '\0')
	{
		return *this;
	}
	data_ = GetAllocator()->New<ByteStringData>();
	data_->length_ = 1;
	data_->reference_.Increase();
	data_->str_ = GetAllocator()->NewArray<char>(2);
	data_->str_[0] = ch;
	data_->str_[1] = '\0';
	return *this;
}

ByteString & ByteString::operator=(char const * pstr)
{
	Clear();
	if (pstr == nullptr)
	{
		return *this;
	}

	uint32_t length = (uint32_t)strlen(pstr);
	if (length == 0)
	{
		return *this;
	}

	data_ = GetAllocator()->New<ByteStringData>();
	data_->reference_.Increase();
	data_->length_ = length;
	data_->str_ = GetAllocator()->NewArray<char>(length + 1);
	strcpy(data_->str_, pstr);
	return *this;
}

ByteString & ByteString::operator=(const ByteString & str)
{
	if (this == &str)
	{
		return *this;
	}
	if (data_ == str.data_)
	{
		return *this;
	}
	Clear();
	if (str.data_)
	{
		data_ = str.data_;
		str.data_->reference_.Increase();
	}
	return *this;
}

bool ByteString::operator==(char ch) const
{
	if (data_ == nullptr || strlen(data_->str_) == 0)
	{
		if (ch == '\0')
		{
			return true;
		}
		else{
			return false;
		}
	}
	else{
		if (data_->length_ != 1)
		{
			return false;
		}
		else{
			if (data_->str_[0] == ch)
			{
				return true;
			}
			else{
				return false;
			}
		}
	}
}

bool ByteString::operator==(char const * pstr) const
{
	if (data_ == nullptr || strlen(data_->str_) == 0)
	{
		if (pstr == nullptr || strlen(pstr) == 0)
		{
			return true;
		}
		else{
			return false;
		}
	}
	else{
		if (pstr == nullptr)
		{
			if (data_->str_ == nullptr)
			{
				return true;
			}
			else{
				return false;
			}
		}
		else{
			if (strcmp(data_->str_, pstr) == 0)
			{
				return true;
			}
			else{
				return false;
			}
		}
	}
}

bool ByteString::operator==(const ByteString & str) const
{
	if (this == &str)
	{
		return true;
	}
	else{
		if (data_ == str.data_)
		{
			return true;
		}
		else{
			if (data_ == nullptr)
			{
				return false;
			}
			else{
				if (strcmp(data_->str_, str.data_->str_) == 0)
				{
					return true;
				}
				else{
					return false;
				}
			}
		}
	}
}

bool ByteString::SetData(uint8_t * pData, uint32_t length)
{
	if (pData == nullptr || length == 0)
	{
		Clear();
		return TRUE;
	}
	if (data_ == nullptr)
	{
		data_ = GetAllocator()->New<ByteStringData>();
		data_->reference_.Increase();
		data_->length_ = length;
		data_->str_ = GetAllocator()->NewArray<char>(length + 1);
		memcpy(data_->str_, pData, length);
		data_->str_[length] = '\0';
	}
	else{
		data_->reference_.Decrease();
		if (data_->reference_ == 0)
		{
			if (data_->str_)
			{
				GetAllocator()->DeleteArray<char>(data_->str_);
			}
			data_->reference_.Increase();
			data_->length_ = length;
			data_->str_ = GetAllocator()->NewArray<char>(length + 1);
			memcpy(data_->str_, pData, length);
			data_->str_[length] = '\0';
		}
		else {
			data_ = GetAllocator()->New<ByteStringData>();
			data_->reference_.Increase();
			data_->length_ = length;
			data_->str_ = GetAllocator()->NewArray<char>(length + 1);
			memcpy(data_->str_, pData, length);
			data_->str_[length] = '\0';
		}
	}
	return TRUE;
}

char const * ByteString::GetData() const
{
	return data_ ? data_->str_ : nullptr;
}

uint32_t ByteString::GetLength() const
{
	if (data_ == nullptr)
	{
		return 0;
	}
	else{
		return data_->length_;
	}
}

char ByteString::operator[](uint32_t index) const
{
	if (data_ != nullptr)
	{
		if (index >= data_->length_)
		{
			return 0;
		}
		else if (data_->str_)
		{
			return data_->str_[index];
		}
		else{
			return 0;
		}
	}
	else{
		return 0;
	}
}

int32_t ByteString::GetInteger() const
{
	return atoi(GetData());
}

FLOAT ByteString::GetFloat() const
{
	return atof(GetData());
}

ByteString ByteString::operator+(char ch)
{
	if (data_ == nullptr || data_->str_ == nullptr)
	{
		return ByteString(ch, GetAllocator());
	}

	uint32_t length = data_->length_ + 2;
	char * pstr = GetAllocator()->NewArray<char>(length);

	strcpy(pstr, data_->str_);
	pstr[length - 2] = ch;
	pstr[length - 1] = '\0';

	ByteString str(pstr, length - 1, GetAllocator());
	GetAllocator()->DeleteArray<char>(pstr);
	return str;
}


ByteString ByteString::operator+(char const * pstr)
{
	if (pstr == nullptr)
	{
		return ByteString(*this);
	}

	if (data_ == nullptr || data_->str_ == nullptr)
	{
		return ByteString(pstr, (uint32_t)strlen(pstr), GetAllocator());
	}

	uint32_t length = data_->length_ + (uint32_t)strlen(pstr) + 1;
	char * pTstr_ = GetAllocator()->NewArray<char>(length);

	strcpy(pTstr_, data_->str_);
	strcat(pTstr_, pstr);

	ByteString str(pTstr_, (uint32_t)strlen(pTstr_), GetAllocator());
	GetAllocator()->DeleteArray<char>(pTstr_);
	return str;
}

ByteString ByteString::operator+(const ByteString & str)
{
	if (str.data_ == nullptr)
	{
		return ByteString(*this);
	}

	if (data_ == nullptr || data_->str_ == nullptr)
	{
		return ByteString(str);
	}

	size_t length = data_->length_ + str.data_->length_ + 1;
	char * pTestr_ = GetAllocator()->NewArray<char>(length);
	strcpy(pTestr_, data_->str_);
	strcat(pTestr_, str.data_->str_);

	ByteString strRet(pTestr_);
	GetAllocator()->DeleteArray<char>(pTestr_);
	return strRet;
}

ByteString & ByteString::operator+=(char ch)
{
	if (ch == 0)
	{
		return *this;
	}

	if (data_ == nullptr)
	{
		data_ = GetAllocator()->New<ByteStringData>();
		data_->reference_.Increase();
		data_->length_ = 1;
		data_->str_ = GetAllocator()->NewArray<char>(2);
		data_->str_[0] = ch;
		data_->str_[1] = '\0';
	}
	else{
		if (data_->str_ == nullptr)
		{
			data_->length_ = 1;
			data_->reference_.Increase();
			data_->str_ = GetAllocator()->NewArray<char>(2);
			data_->str_[0] = ch;
			data_->str_[1] = '\0';
			return *this;
		}
		char * pTstr_ = GetAllocator()->NewArray<char>(strlen(data_->str_) + 2);
		strcpy(pTstr_, data_->str_);
		data_->reference_.Decrease();
		if (data_->reference_ == 0)
		{
			if (data_->str_)
			{
				GetAllocator()->DeleteArray<char>(data_->str_);
				data_->str_ = nullptr;
			}
			data_->length_ = 0;
			data_->reference_.Increase();
		}
		else{
			data_ = GetAllocator()->New<ByteStringData>();
			data_->length_ = 0;
			data_->reference_.Increase();
			data_->str_ = nullptr;
		}

		uint32_t length = (uint32_t)strlen(pTstr_) + 2;
		data_->str_ = GetAllocator()->NewArray<char>(length);
		strcpy(data_->str_, pTstr_);
		data_->str_[length - 2] = ch;
		data_->str_[length - 1] = '\0';
		data_->length_ = length - 1;

		GetAllocator()->DeleteArray<char>(pTstr_);
	}
	return *this;
}

ByteString & ByteString::operator+=(char const * pstr)
{
	if (pstr == nullptr)
	{
		return *this;
	}

	if (strlen(pstr) == 0)
	{
		return *this;
	}

	if (data_ == nullptr)
	{
		uint32_t length = (uint32_t)strlen(pstr);
		data_ = GetAllocator()->New<ByteStringData>();
		data_->reference_.Increase();
		data_->str_ = GetAllocator()->NewArray<char>(length + 1);
		strcpy(data_->str_, pstr);
		data_->length_ = length;
	}
	else{
		if (data_->str_ == nullptr)
		{
			uint32_t length = (uint32_t)strlen(pstr);
			data_->str_ = GetAllocator()->NewArray<char>(length + 1);
			strcpy(data_->str_, pstr);
			data_->length_ = length;
			return *this;
		}

		char * pTstr_ = GetAllocator()->NewArray<char>(strlen(data_->str_) + 1);
		strcpy(pTstr_, data_->str_);

		data_->reference_.Decrease();
		if (data_->reference_ == 0)
		{
			if (data_->str_)
			{
				GetAllocator()->DeleteArray<char>(data_->str_);
				data_->str_ = nullptr;
			}
			data_->length_ = 0;
			data_->reference_.Increase();
		}
		else{
			data_ = GetAllocator()->New<ByteStringData>();
			data_->length_ = 0;
			data_->reference_.Increase();
			data_->str_ = nullptr;
		}

		uint32_t length = (uint32_t)strlen(pTstr_) + (uint32_t)strlen(pstr);
		data_->str_ = GetAllocator()->NewArray<char>(length + 1);
		strcpy(data_->str_, pTstr_);
		strcat(data_->str_, pstr);
		data_->length_ = length;
	}

	return *this;
}

ByteString & ByteString::operator+=(const ByteString & str)
{
	if (str.data_ == nullptr || str.data_->str_ == nullptr)
	{
		return *this;
	}

	if (this == &str)
	{
		char * pTstr_ = GetAllocator()->NewArray<char>(strlen(data_->str_) + 1);
		strcpy(pTstr_, data_->str_);

		data_->reference_.Decrease();
		if (data_->reference_ == 0)
		{
			if (data_->str_)
			{
				GetAllocator()->DeleteArray<char>(data_->str_);
				data_->str_ = nullptr;
			}
			data_->length_ = 0;
			data_->reference_.Increase();
		}
		else{
			data_ = GetAllocator()->New<ByteStringData>();
			data_->length_ = 0;
			data_->reference_.Increase();
			data_->str_ = nullptr;
		}

		uint32_t length = (uint32_t)strlen(pTstr_) * 2 + 1;
		data_->str_ = GetAllocator()->NewArray<char>(length);
		strcpy(data_->str_, pTstr_);
		strcat(data_->str_, pTstr_);

		data_->length_ = length - 1;

		GetAllocator()->DeleteArray<char>(pTstr_);
		pTstr_ = nullptr;

		return *this;
	}
	else{
		if (data_ == str.data_)
		{
			char * pTstr_ = GetAllocator()->NewArray<char>(strlen(data_->str_) + 1);
			strcpy(pTstr_, data_->str_);

			data_->reference_.Decrease();
			data_->reference_.Decrease();
			if (data_->reference_ == 0)
			{
				if (data_->str_)
				{
					GetAllocator()->DeleteArray<char>(data_->str_);
					data_->str_ = nullptr;
				}
				data_->length_ = 0;
				data_->reference_.Increase();
				data_->reference_.Increase();
			}
			else{
				data_ = GetAllocator()->New<ByteStringData>();
				data_->reference_.Increase();
				data_->reference_.Increase();
				data_->str_ = nullptr;
				data_->length_ = 0;
			}

			uint32_t length = (uint32_t)strlen(pTstr_) * 2 + 1;
			data_->str_ = GetAllocator()->NewArray<char>(length);
			strcpy(data_->str_, pTstr_);
			strcat(data_->str_, pTstr_);

			data_->length_ = length - 1;

			GetAllocator()->DeleteArray<char>(pTstr_);
			pTstr_ = nullptr;

			return *this;
		}
		else{
			if (data_->str_ == nullptr)
			{
				size_t length = strlen(str.data_->str_) + 1;
				data_->str_ = GetAllocator()->NewArray<char>(length);
				strcpy(data_->str_, str.data_->str_);
				return *this;
			}

			char * pTstr_ = GetAllocator()->NewArray<char>(strlen(data_->str_) + 1);
			strcpy(pTstr_, data_->str_);

			if (data_->reference_ == 1)
			{
				if (data_->str_)
				{
					GetAllocator()->DeleteArray<char>(data_->str_);
					data_->str_ = nullptr;
					data_->length_ = 0;
				}
			}
			else{
				data_->reference_.Decrease();
				data_ = GetAllocator()->New<ByteStringData>();
				data_->length_ = 0;
				data_->reference_.Increase();
				data_->str_ = nullptr;
			}

			uint32_t length = (uint32_t)strlen(str.data_->str_);
			length += strlen(pTstr_) + 1;
			data_->str_ = GetAllocator()->NewArray<char>(length);
			strcpy(data_->str_, pTstr_);
			strcat(data_->str_, str.data_->str_);

			GetAllocator()->DeleteArray<char>(pTstr_);
			pTstr_ = nullptr;

			data_->length_ = length - 1;

			return *this;
		}
	}
}

bool ByteString::operator!=(char ch) const
{
	if (data_ == nullptr)
	{
		if (ch == 0)
		{
			return false;
		}
		else{
			return true;
		}
	}
	else{
		if (data_->length_ != 1)
		{
			return true;
		}
		else{
			if (data_->str_[0] == ch)
			{
				return false;
			}
			else{
				return true;
			}
		}
	}
}

bool ByteString::operator!=(char const * pstr) const
{
	if (data_ == nullptr)
	{
		if (pstr == nullptr || strlen(pstr) == 0)
		{
			return false;
		}
		else{
			return true;
		}
	}
	else{
		if (pstr == nullptr)
		{
			if (data_->str_ == nullptr)
			{
				return false;
			}
			else{
				return true;
			}
		}
		else{
			if (strcmp(data_->str_, pstr) == 0)
			{
				return false;
			}
			else{
				return true;
			}
		}
	}
}

bool ByteString::operator!=(const ByteString & str) const
{
	if (this == &str)
	{
		return false;
	}
	else{
		if (data_ == str.data_)
		{
			return false;
		}
		else{
			if (data_ == nullptr)
			{
				return true;
			}
			else{
				if (strcmp(data_->str_, str.data_->str_) == 0)
				{
					return false;
				}
				else{
					return true;
				}
			}
		}
	}
}

bool operator==(char ch, ByteString & str)
{
	return (str == ch);
}

bool operator==(char const * pstr, const ByteString & str)
{
	return (str == pstr);
}

ByteString operator+(char ch, const ByteString & str)
{
	ByteString testr_(ch, str.GetAllocator());
	testr_ += str;
	return testr_;
}

ByteString operator+(char const * pstr, const ByteString & str)
{
	ByteString testr_(pstr, (uint32_t)strlen(pstr), str.GetAllocator());
	testr_ += str;
	return testr_;
}

bool operator!=(char ch, const ByteString & str)
{
	return (str != ch);
}

bool operator!=(char const * pstr, const ByteString & str)
{
	return (str != pstr);
}



struct WideStringData
{
	WideStringData()
	{
		str_ = nullptr;
		length_ = 0;
	}
	wchar_t *       str_;
	uint32_t        length_;
	ReferenceCount  reference_;
};

WideString::WideString(wchar_t wch, Allocator * allocator)
	: BaseObject(allocator)
{
	if (wch == '\0')
	{
		data_ = nullptr;
	}
	else{
		data_ = GetAllocator()->New<WideStringData>();
		data_->length_ = 1;
		data_->reference_.Increase();
		data_->str_ = GetAllocator()->NewArray<wchar_t >(2);
		data_->str_[0] = wch;
		data_->str_[1] = '\0';
	}
}

WideString::WideString(wchar_t const * pstr, uint32_t length /* = 0 */, Allocator * allocator)
	: BaseObject(allocator)
{
	if (pstr == nullptr)
	{
		data_ = nullptr;
		return;
	}
	if (length == 0)
	{
		length = (uint32_t)wcslen(pstr);
	}
	if (length > 0)
	{
		data_ = GetAllocator()->New<WideStringData>();
		data_->reference_.Increase();
		data_->length_ = length;
		data_->str_ = GetAllocator()->NewArray<wchar_t >(length + 1);
		wcscpy(data_->str_, pstr);
		data_->str_[length] = '\0';
	}
	else{
		data_ = nullptr;
	}
}

WideString::WideString(const WideString & str) : BaseObject(str.GetAllocator())
{
	if (str.data_ == nullptr)
	{
		data_ = nullptr;
	}
	else{
		data_ = str.data_;
		str.data_->reference_.Increase();
	}
}

void WideString::Clear()
{
	if (data_)
	{
		data_->reference_.Decrease();
		if (data_->reference_ == 0 && data_->str_)
		{
			GetAllocator()->DeleteArray<wchar_t >(data_->str_);
			data_->str_ = nullptr;
			GetAllocator()->Delete<WideStringData>(data_);
		}

		data_ = nullptr;
	}
}

WideString & WideString::operator=(wchar_t  wch)
{
	Clear();

	if (wch == (wchar_t)0x0000)
	{
		return *this;
	}

	data_ = GetAllocator()->New<WideStringData>();
	data_->length_ = 1;
	data_->reference_.Increase();
	data_->str_ = GetAllocator()->NewArray<wchar_t >(2);
	data_->str_[0] = wch;
	data_->str_[1] = (wchar_t)0x0000;

	return *this;
}

WideString& WideString::operator=(wchar_t const * pstr)
{
	Clear();

	if (pstr == nullptr)
	{
		return *this;
	}

	uint32_t length = (uint32_t)wcslen(pstr);
	if (length == 0)
	{
		return *this;
	}

	data_ = GetAllocator()->New<WideStringData>();
	data_->reference_.Increase();
	data_->length_ = length;
	data_->str_ = GetAllocator()->NewArray<wchar_t >(length + 1);
	wcscpy(data_->str_, pstr);
	return *this;
}

WideString & WideString::operator=(const WideString & str)
{
	if (this == &str)
	{
		return *this;
	}

	if (data_ == str.data_)
	{
		return *this;
	}

	Clear();

	if (str.data_)
	{
		data_ = str.data_;
		str.data_->reference_.Increase();
	}

	return *this;
}

bool WideString::operator==(wchar_t wch) const
{
	if (data_ == nullptr || wcslen(data_->str_) == 0)
	{
		if (wch == '\0')
		{
			return true;
		}
		else{
			return false;
		}
	}
	else{
		if (data_->length_ != 1)
		{
			return false;
		}
		else{
			if (data_->str_[0] == wch)
			{
				return true;
			}
			else{
				return false;
			}
		}
	}
}

bool WideString::operator==(wchar_t const * pstr)const
{
	if (data_ == nullptr || wcslen(data_->str_) == 0)
	{
		if (pstr == nullptr || wcslen(pstr) == 0)
		{
			return true;
		}
		else{
			return false;
		}
	}
	else{
		if (pstr == nullptr)
		{
			if (data_->str_ == nullptr)
			{
				return true;
			}
			else{
				return false;
			}
		}
		else{
			if (wcscmp(data_->str_, pstr) == 0)
			{
				return true;
			}
			else{
				return false;
			}
		}
	}
}

bool WideString::operator==(const WideString & str)const
{
	if (this == &str)
	{
		return true;
	}
	else{
		if (data_ == str.data_)
		{
			return true;
		}
		else{
			if (data_ == nullptr)
			{
				return false;
			}
			else{
				if (wcscmp(data_->str_, str.data_->str_) == 0)
				{
					return true;
				}
				else{
					return false;
				}
			}
		}
	}
}

bool WideString::SetData(wchar_t * pData, uint32_t length)
{
	if (pData == nullptr || length == 0)
	{
		Clear();
		return TRUE;
	}
	if (data_ == nullptr)
	{
		data_ = GetAllocator()->New<WideStringData>();
		data_->reference_.Increase();
		data_->length_ = length;
		data_->str_ = GetAllocator()->NewArray<wchar_t >(length + 1);
		memcpy(data_->str_, pData, length * sizeof(wchar_t));
		data_->str_[length] = '\0';
	}
	else{
		data_->reference_.Decrease();
		if (data_->reference_ == 0)
		{
			if (data_->str_)
			{
				GetAllocator()->DeleteArray<wchar_t >(data_->str_);
			}
			data_->reference_.Increase();
			data_->length_ = length;
			data_->str_ = GetAllocator()->NewArray<wchar_t >(length + 1);
			memcpy(data_->str_, pData, length * sizeof(wchar_t));
			data_->str_[length] = '\0';
		}
		else {
			data_ = GetAllocator()->New<WideStringData>();
			data_->reference_.Increase();
			data_->length_ = length;
			data_->str_ = GetAllocator()->NewArray<wchar_t >(length + 1);
			memcpy(data_->str_, pData, length * sizeof(wchar_t));
			data_->str_[length] = '\0';
		}
	}
	return TRUE;
}

wchar_t const * WideString::GetData() const
{
	return (data_) ? data_->str_ : nullptr;
}

uint32_t WideString::GetLength() const
{
	if (data_ == nullptr)
	{
		return 0;
	}
	else{
		return data_->length_;
	}
}

wchar_t WideString::operator[](uint32_t index) const
{
	if (data_ != nullptr)
	{
		if (index >= data_->length_)
		{
			return 0;
		}
		else if (data_->str_)
		{
			return data_->str_[index];
		}
		else{
			return 0;
		}
	}
	else{
		return 0;
	}
}

int32_t WideString::GetInteger() const
{
	if (GetLength() == 0)
	{
		return 0;
	}
	else{
		bool bBegin = TRUE;
		bool bNegative = false;
		uint32_t iValue = 0;
		bool bSign = false;
		wchar_t  tmpChar = 0;
		for (size_t i = 0; i < GetLength(); i++)
		{
			if (bBegin && i > 0)
			{
				bBegin = false;
			}
			tmpChar = GetData()[i];
			switch (tmpChar)
			{
			case '+':
				if (bSign == false && bBegin == TRUE)
				{
					bSign = TRUE;
					bNegative = false;
				}
				else{
					return 0;
				}
				break;
			case '-':
				if (bSign == false && bBegin == TRUE)
				{
					bSign = TRUE;
					bNegative = TRUE;
				}
				else{
					return 0;
				}
				break;
			default:
				if (L'0' > tmpChar || tmpChar > L'9')
				{
					return 0;
				}
				else{
					iValue = iValue * 10 + (tmpChar - L'0');
				}
				break;
			}
		}
		if (bNegative == TRUE)
		{
			return 0 - iValue;
		}
		else{
			return iValue;
		}
	}
}

FLOAT WideString::GetFloat() const
{
	if (GetLength() == 0)
	{
		return 0;
	}
	else{
		bool bNegative = false;
		bool bBegin = TRUE;
		size_t lPointBit = 1;
		FLOAT fValue = 0;
		bool bPoint = false;
		bool bSign = false;
		wchar_t  tmpChar = 0;
		for (size_t i = 0; i < GetLength(); i++)
		{
			if (bBegin && i > 0)
			{
				bBegin = false;
			}
			tmpChar = GetData()[i];
			switch (tmpChar)
			{
			case L'+':
				if (bSign == false && bBegin == TRUE)
				{
					bSign = TRUE;
					bNegative = false;
				}
				else{
					return 0;
				}
				break;
			case L'-':
				if (bSign == false && bBegin == TRUE)
				{
					bSign = TRUE;
					bNegative = TRUE;
				}
				else{
					return 0;
				}
				break;
			case L'.':
				if (bPoint == false)
				{
					bPoint = TRUE;
				}
				else{
					return 0;
				}
				break;
			default:
				if (L'0' > tmpChar || tmpChar > L'9')
				{
					return 0;
				}
				else{
					if (bPoint == false)
					{
						fValue = fValue * 10 + (tmpChar - L'0');
					}
					else{
						fValue = fValue + (tmpChar - L'0') / (lPointBit * 10);
						lPointBit++;
					}
				}
				break;
			}
		}
		if (bNegative == TRUE)
		{
			return 0 - fValue;
		}
		else{
			return fValue;
		}
	}
}

WideString WideString::operator+(wchar_t wch)
{
	if (data_ == nullptr || data_->str_ == nullptr)
	{
		return WideString(wch, GetAllocator());
	}

	size_t length = data_->length_ + 2;
	wchar_t  * pTestr_ = GetAllocator()->NewArray<wchar_t >(length);

	wcscpy(pTestr_, data_->str_);
	pTestr_[length - 2] = wch;
	pTestr_[length - 1] = '\0';

	WideString tstr_ing(pTestr_);
	GetAllocator()->DeleteArray<wchar_t >(pTestr_);
	return tstr_ing;
}

WideString WideString::operator+(wchar_t const * pstr)
{
	if (pstr == nullptr)
	{
		return WideString(*this);
	}

	if (data_ == nullptr || data_->str_ == nullptr)
	{
		return WideString(pstr);
	}

	uint32_t length = data_->length_ + (uint32_t)wcslen(pstr);
	wchar_t  * pTestr_ = GetAllocator()->NewArray<wchar_t >(length + 1);
	wcscpy(pTestr_, data_->str_);
	wcscat(pTestr_, pstr);
	data_->length_ = length;

	WideString tstr_ing(pTestr_);
	GetAllocator()->DeleteArray<wchar_t >(pTestr_);
	return tstr_ing;
}

WideString WideString::operator+(const WideString & str)
{
	if (str.data_ == nullptr)
	{
		return WideString(*this);
	}

	if (data_ == nullptr || data_->str_ == nullptr)
	{
		return WideString(str);
	}

	uint32_t length = data_->length_ + str.data_->length_;
	wchar_t  * pTestr_ = GetAllocator()->NewArray<wchar_t >(length + 1);

	wcscpy(pTestr_, data_->str_);
	wcscat(pTestr_, str.data_->str_);
	data_->length_ = length;

	WideString tstr_ing(pTestr_);
	GetAllocator()->DeleteArray<wchar_t >(pTestr_);
	return tstr_ing;
}

WideString & WideString::operator+=(wchar_t wch)
{
	if (wch == 0)
	{
		return *this;
	}

	if (data_ == nullptr)
	{
		data_ = GetAllocator()->New<WideStringData>();
		data_->reference_.Increase();
		data_->length_ = 1;
		data_->str_ = GetAllocator()->NewArray<wchar_t >(2);
		data_->str_[0] = wch;
		data_->str_[1] = '\0';
	}
	else{
		if (data_->str_ == nullptr)
		{
			data_->length_ = 1;
			data_->reference_.Increase();
			data_->str_ = GetAllocator()->NewArray<wchar_t >(2);
			data_->str_[0] = wch;
			data_->str_[1] = '\0';
			return *this;
		}

		wchar_t *  pTestr_ = GetAllocator()->NewArray<wchar_t >(wcslen(data_->str_) + 2);
		wcscpy(pTestr_, data_->str_);

		data_->reference_.Decrease();
		if (data_->reference_ == 0)
		{
			if (data_->str_)
			{
				GetAllocator()->DeleteArray<wchar_t >(data_->str_);
				data_->str_ = nullptr;
			}
			data_->length_ = 0;
			data_->reference_.Increase();
		}
		else{
			data_ = GetAllocator()->New<WideStringData>();
			data_->length_ = 0;
			data_->reference_.Increase();
			data_->str_ = nullptr;
		}

		uint32_t dwBufferSize = (uint32_t)wcslen(pTestr_) + 2;
		data_->str_ = GetAllocator()->NewArray<wchar_t >(dwBufferSize);
		wcscpy(data_->str_, pTestr_);
		data_->str_[dwBufferSize - 2] = wch;
		data_->str_[dwBufferSize - 1] = '\0';
		data_->length_ = dwBufferSize - 1;

		GetAllocator()->DeleteArray<wchar_t >(pTestr_);
	}
	return *this;
}

WideString & WideString::operator+=(wchar_t const * pstr)
{
	if (pstr == nullptr)
	{
		return *this;
	}

	if (wcslen(pstr) == 0)
	{
		return *this;
	}

	if (data_ == nullptr)
	{
		uint32_t length = (uint32_t)wcslen(pstr);
		data_ = GetAllocator()->New<WideStringData>();
		data_->reference_.Increase();
		data_->str_ = GetAllocator()->NewArray<wchar_t >(length + 1);
		wcscpy(data_->str_, pstr);
		data_->length_ = length;
	}
	else{
		if (data_->str_ == nullptr)
		{
			uint32_t length = (uint32_t)wcslen(pstr);
			data_->str_ = GetAllocator()->NewArray<wchar_t >(length + 1);
			wcscpy(data_->str_, pstr);
			data_->length_ = length;
			data_->reference_.Increase();
			return *this;
		}

		wchar_t * pTestr_ = GetAllocator()->NewArray<wchar_t >(wcslen(data_->str_) + 1);
		wcscpy(pTestr_, data_->str_);

		data_->reference_.Decrease();
		if (data_->reference_ == 0)
		{
			if (data_->str_)
			{
				GetAllocator()->DeleteArray<wchar_t >(data_->str_);
				data_->str_ = nullptr;
			}
			data_->length_ = 0;
			data_->reference_.Increase();
		}
		else{
			data_ = GetAllocator()->New<WideStringData>();
			data_->length_ = 0;
			data_->reference_.Increase();
			data_->str_ = nullptr;
		}

		uint32_t length = (uint32_t)wcslen(pTestr_) + (uint32_t)wcslen(pstr);
		data_->str_ = GetAllocator()->NewArray<wchar_t >(length + 1);
		wcscpy(data_->str_, pTestr_);
		wcscat(data_->str_, pstr);
		data_->length_ = length;
	}

	return *this;
}

WideString & WideString::operator+=(const WideString & str)
{
	if (str.data_ == nullptr || str.data_->str_ == nullptr)
	{
		return *this;
	}

	if (this == &str)
	{
		wchar_t * pTestr_ = GetAllocator()->NewArray<wchar_t >(wcslen(data_->str_) + 1);
		wcscpy(pTestr_, data_->str_);

		data_->reference_.Decrease();
		if (data_->reference_ == 0)
		{
			if (data_->str_)
			{
				GetAllocator()->DeleteArray<wchar_t >(data_->str_);
				data_->str_ = nullptr;
			}
			data_->length_ = 0;
			data_->reference_.Increase();
		}
		else{
			data_ = GetAllocator()->New<WideStringData>();
			data_->length_ = 0;
			data_->reference_.Increase();
			data_->str_ = nullptr;
		}

		uint32_t length = (uint32_t)wcslen(pTestr_) * 2;
		data_->str_ = GetAllocator()->NewArray<wchar_t >(length + 1);
		wcscpy(data_->str_, pTestr_);
		wcscat(data_->str_, pTestr_);

		data_->length_ = length;

		GetAllocator()->DeleteArray<wchar_t >(pTestr_);
		pTestr_ = nullptr;

		return *this;
	}
	else{
		if (data_ == str.data_)
		{
			wchar_t *  pTestr_ = GetAllocator()->NewArray<wchar_t >(wcslen(data_->str_) + 1);
			wcscpy(pTestr_, data_->str_);

			data_->reference_.Decrease();
			data_->reference_.Decrease();
			if (data_->reference_ == 0)
			{
				if (data_->str_)
				{
					GetAllocator()->DeleteArray<wchar_t >(data_->str_);
					data_->str_ = nullptr;
				}
				data_->length_ = 0;
				data_->reference_.Increase();
				data_->reference_.Increase();
			}
			else{
				data_ = GetAllocator()->New<WideStringData>();
				data_->reference_.Increase();
				data_->reference_.Increase();
				data_->str_ = nullptr;
				data_->length_ = 0;
			}

			uint32_t length = (uint32_t)wcslen(pTestr_) * 2;
			data_->str_ = GetAllocator()->NewArray<wchar_t >(length + 1);
			wcscpy(data_->str_, pTestr_);
			wcscat(data_->str_, pTestr_);

			data_->length_ = length;

			GetAllocator()->DeleteArray<wchar_t >(pTestr_);
			pTestr_ = nullptr;

			return *this;
		}
		else{
			if (data_->str_ == nullptr)
			{
				size_t length = wcslen(str.data_->str_);
				data_->str_ = GetAllocator()->NewArray<wchar_t >(length + 1);
				wcscpy(data_->str_, str.data_->str_);
				return *this;
			}

			wchar_t * pTestr_ = GetAllocator()->NewArray<wchar_t >(wcslen(data_->str_) + 1);
			wcscpy(pTestr_, data_->str_);

			if (data_->reference_ == 1)
			{
				if (data_->str_)
				{
					GetAllocator()->DeleteArray<wchar_t >(data_->str_);
					data_->str_ = nullptr;
					data_->length_ = 0;
				}
			}
			else{
				data_->reference_.Decrease();
				data_ = GetAllocator()->New<WideStringData>();
				data_->length_ = 0;
				data_->reference_.Increase();
				data_->str_ = nullptr;
			}

			uint32_t length = (uint32_t)wcslen(str.data_->str_);
			length += wcslen(pTestr_);
			data_->str_ = GetAllocator()->NewArray<wchar_t >(length + 1);
			wcscpy(data_->str_, pTestr_);
			wcscat(data_->str_, str.data_->str_);

			GetAllocator()->DeleteArray<wchar_t >(pTestr_);
			pTestr_ = nullptr;

			data_->length_ = length;

			return *this;
		}
	}
}

bool WideString::operator!=(wchar_t  wch) const
{
	if (data_ == nullptr)
	{
		if (wch == 0)
		{
			return false;
		}
		else{
			return true;
		}
	}
	else{
		if (data_->length_ != 1)
		{
			return true;
		}
		else{
			if (data_->str_[0] == wch)
			{
				return false;
			}
			else{
				return true;
			}
		}
	}
}

bool WideString::operator!=(wchar_t const * pstr) const
{
	if (data_ == nullptr)
	{
		if (pstr == nullptr || wcslen(pstr) == 0)
		{
			return false;
		}
		else{
			return true;
		}
	}
	else{
		if (pstr == nullptr)
		{
			if (data_->str_ == nullptr)
			{
				return false;
			}
			else{
				return true;
			}
		}
		else{
			if (wcscmp(data_->str_, pstr) == 0)
			{
				return false;
			}
			else{
				return true;
			}
		}
	}
}

bool WideString::operator!=(const WideString & str) const
{
	if (this == &str)
	{
		return false;
	}
	else{
		if (data_ == str.data_)
		{
			return false;
		}
		else{
			if (data_ == nullptr)
			{
				return true;
			}
			else{
				if (wcscmp(data_->str_, str.data_->str_) == 0)
				{
					return false;
				}
				else{
					return true;
				}
			}
		}
	}
}

bool operator==(wchar_t  wch, WideString & str)
{
	return (str == wch);
}

bool operator==(wchar_t const * pstr, const WideString & str)
{
	return (str == pstr);
}

WideString operator+(wchar_t wch, const WideString & str)
{
	WideString testr_(wch, str.GetAllocator());
	testr_ += str;
	return testr_;
}

WideString operator+(wchar_t const * pstr, const WideString & str)
{
	WideString testr_(pstr, (uint32_t)wcslen(pstr), str.GetAllocator());
	testr_ += str;
	return testr_;
}

bool operator!=(wchar_t  wch, const WideString & str)
{
	return (str != wch);
}

bool operator!=(wchar_t const * pstr, const WideString & str)
{
	return (str != pstr);
}

uint32_t StringToUINT32(const ByteString & str)
{
	if (str.GetLength() == 0)
	{
		return 0;
	}
	size_t length = 4;
	if (str.GetLength() < length)
	{
		length = str.GetLength();
	}
	uint32_t valRet = 0;
	for (size_t i = length; i > 0; --i)
	{
		valRet = valRet << 8;
		valRet |= str.GetData()[i - 1];
	}
	return valRet;
}

}//end of namespce chelib
