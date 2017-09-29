#include "../include/che_base_string.h"

#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <memory>

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
        _str = nullptr;
        _length = 0;
    }
    char *          _str;
    size_t          _length;
    ReferenceCount  _reference;
};

ByteString::ByteString( char ch, Allocator * pAllocator )
  : BaseObject( pAllocator )
{
	if ( ch == '\0' )
	{
		_data = nullptr;
	}else{
		_data = GetAllocator()->New<ByteStringData>();
		_data->_length = 1;
		_data->_reference.AddRef();
		_data->_str[0] = ch;
		_data->_str = GetAllocator()->NewArray<char>( 2 );
		_data->_str[1] = '\0';
	}
}

ByteString::ByteString( char const * pstr, size_t length /* = 0 */, Allocator * pAllocator /*= nullptr*/ )
  : BaseObject( pAllocator )
{
	if ( pstr == nullptr )
	{
		_data = nullptr;
		return;
	}
	if ( length == 0 )
	{
        length = strlen( pstr );
	}
	if ( length > 0 )
	{
		_data = GetAllocator()->New<ByteStringData>();
		_data->_reference.AddRef();
		_data->_length = length;
		_data->_str = GetAllocator()->NewArray<char>( length+1 );
		memcpy( _data->_str, pstr, length );
		_data->_str[length] = '\0';
	}else{
		_data = nullptr;
	}
}

ByteString::ByteString( const ByteString & str )
  : BaseObject( str.GetAllocator() )
{
	if ( str._data == nullptr )
	{
		_data = nullptr;
	}else{
		_data = str._data;
		str._data->_reference.AddRef();
	}
}

void ByteString::Clear()
{
	if ( _data )
	{
		_data->_reference.DecRef();

		if ( _data->_reference == 0 && _data->_str )
		{
			GetAllocator()->DeleteArray<char>( _data->_str );
			_data->_str = nullptr;
			GetAllocator()->Delete<ByteStringData>( _data );
		}
		_data = nullptr;
	}
}

ByteString & ByteString::operator=( char ch )
{
	Clear();

	if ( ch == '\0' )
	{
		return *this;
	}
	
	_data = GetAllocator()->New<ByteStringData>();
	_data->_length = 1;
	_data->_reference.AddRef();
	_data->_str = GetAllocator()->NewArray<char>( 2 );
	_data->_str[0] = ch;
	_data->_str[1] = '\0';

	return *this;
}

ByteString & ByteString::operator=( char const * pstr )
{
	Clear();

	if ( pstr == nullptr )
	{
		return *this;
	}
    
	size_t length = strlen( pstr );
	if ( length == 0 )
	{
		return *this;
	}

	_data = GetAllocator()->New<ByteStringData>();
	_data->_reference.AddRef();
	_data->_length = length;
	_data->_str = GetAllocator()->NewArray<char>( length+1 );
	strcpy( _data->_str, pstr );

	return *this;
}

ByteString & ByteString::operator=( const ByteString & str )
{
	if ( this == &str )
	{
		return *this;
	}

	if ( _data == str._data )
	{
		return *this;
	}

	Clear();

	if ( str._data )
	{
		_data = str._data;
		str._data->_reference.AddRef();
	}

	return *this;
}

bool ByteString::operator==( char ch ) const
{
	if ( _data == nullptr || strlen( _data->_str ) == 0 )
	{
		if ( ch == '\0' )
		{
			return true;
		}else{
			return false;
		}
	}else{
		if ( _data->_length != 1 )
		{
			return false;
		}else{
			if ( _data->_str[0] == ch )
			{
				return true;
			}else{
				return false;
			}
		}
	}
}

bool ByteString::operator==( char const * pstr ) const
{
	if ( _data == nullptr || strlen( _data->_str ) == 0 )
	{
		if ( pstr == nullptr || strlen( pstr ) == 0 )
		{
			return true;
		}else{
			return false;
		}
	}else{
		if ( pstr == nullptr )
		{
			if ( _data->_str == nullptr )
			{
				return true;
			}else{
				return false;
			}
		}else{
			if ( strcmp( _data->_str, pstr ) == 0 )
			{
				return true;
			}else{
				return false;
			}	
		}
	}
}

bool ByteString::operator==( const ByteString & str ) const
{
	if ( this == &str )
	{
		return true;
	}else{
		if ( _data == str._data )
		{
			return true;
		}else{
			if ( _data == nullptr )
			{
				return false;
			}else{
				if ( strcmp( _data->_str, str._data->_str ) == 0 )
				{
					return true;
				}else{
					return false;
				}
			}
		}
	}
}

bool ByteString::SetData( BYTE * pData, size_t length )
{
	if ( pData == nullptr || length == 0 )
	{
		Clear();
		return TRUE;
	}
	if ( _data == nullptr )
	{
		_data = GetAllocator()->New<ByteStringData>();
		_data->_reference.AddRef();
		_data->_length = length;
		_data->_str = GetAllocator()->NewArray<char>( length+1 );
		memcpy( _data->_str, pData, length );
		_data->_str[length] = '\0';
	}else{
		_data->_reference.DecRef();
		if ( _data->_reference == 0 )
		{
			if ( _data->_str )
			{
				GetAllocator()->DeleteArray<char>( _data->_str );
			}
			_data->_reference.AddRef();
			_data->_length = length;
			_data->_str= GetAllocator()->NewArray<char>( length+1 );
			memcpy( _data->_str, pData, length );
			_data->_str[length] = '\0';
		}else {
			_data = GetAllocator()->New<ByteStringData>();
			_data->_reference.AddRef();
			_data->_length = length;
			_data->_str= GetAllocator()->NewArray<char>( length+1 );
			memcpy( _data->_str, pData, length );
			_data->_str[length] = '\0';
		}
	}
	return TRUE;
}
    
char const * ByteString::GetData() const
{
    return _data ? _data->_str : nullptr;
}

size_t ByteString::GetLength() const
{
	if ( _data == nullptr )
	{
		return 0;
	}else{
		return _data->_length;
	}
}

char ByteString::operator[]( size_t index ) const
{
	if ( _data != nullptr )
	{
		if ( index >= _data->_length )
		{
			return 0;
		}else if ( _data->_str )
		{
			return _data->_str[index];
		}else{
			return 0;
		}
	}else{
		return 0;
	}
}

INT32 ByteString::GetInteger() const
{
	return atoi( GetData() );
}

FLOAT ByteString::GetFloat() const
{
	return atof( GetData() );
}

ByteString ByteString::operator+( char ch )
{
	if ( _data == nullptr || _data->_str == nullptr )
	{
		return ByteString( ch, GetAllocator() );
	}

	size_t length = _data->_length + 2;
	char * pstr = GetAllocator()->NewArray<char>( length );

	strcpy( pstr, _data->_str );
	pstr[length-2] = ch;
	pstr[length-1] = '\0';

	ByteString str( pstr, length-1, GetAllocator() );
	GetAllocator()->DeleteArray<char>( pstr );
	return str;
}


ByteString ByteString::operator+( char const * pstr )
{
	if ( pstr == nullptr )
	{
		return ByteString( *this );
	}

	if ( _data == nullptr || _data->_str == nullptr )
	{
		return ByteString( pstr, strlen(pstr), GetAllocator() );
	}

	size_t length = _data->_length + strlen(pstr) + 1;
	char * pT_str = GetAllocator()->NewArray<char>( length );

	strcpy( pT_str, _data->_str );
	strcat( pT_str, pstr );

	ByteString str( pT_str, strlen(pT_str), GetAllocator() );
	GetAllocator()->DeleteArray<char>( pT_str );
	return str;
}

ByteString ByteString::operator+( const ByteString & str )
{
	if ( str._data == nullptr )
	{
		return ByteString( *this );
	}
	
	if ( _data == nullptr || _data->_str == nullptr )
	{
		return ByteString( str );
	}
	
	size_t length = _data->_length + str._data->_length + 1;
	char * pTe_str = GetAllocator()->NewArray<char>( length );
	strcpy( pTe_str, _data->_str );
	strcat( pTe_str, str._data->_str );
	
	ByteString strRet(pTe_str);
	GetAllocator()->DeleteArray<char>( pTe_str );
	return strRet;
}

ByteString & ByteString::operator+=( char ch )
{
	if ( ch == 0 )
	{
		return *this;
	}

	if ( _data == nullptr )
	{
		_data = GetAllocator()->New<ByteStringData>();
		_data->_reference.AddRef();
		_data->_length = 1;
		_data->_str = GetAllocator()->NewArray<char>( 2 );
		_data->_str[0] = ch;
		_data->_str[1] = '\0';
	}else{
		if ( _data->_str == nullptr )
		{
			_data->_length = 1;
			_data->_reference.AddRef();
			_data->_str = GetAllocator()->NewArray<char>( 2 );
			_data->_str[0] = ch;
			_data->_str[1] = '\0';
			return *this;
		}
		char * pT_str = GetAllocator()->NewArray<char>( strlen(_data->_str)+2 );
		strcpy( pT_str, _data->_str );
		_data->_reference.DecRef();
		if ( _data->_reference == 0 )
		{
			if ( _data->_str )
			{
				GetAllocator()->DeleteArray<char>( _data->_str );
				_data->_str = nullptr;
			}
			_data->_length = 0;
			_data->_reference.AddRef();
		}else{
			_data = GetAllocator()->New<ByteStringData>();
			_data->_length = 0;
			_data->_reference.AddRef();
			_data->_str = nullptr;
		}
		
		size_t length = strlen( pT_str )+2;
		_data->_str = GetAllocator()->NewArray<char>( length );
		strcpy( _data->_str, pT_str );
		_data->_str[length-2] = ch;
		_data->_str[length-1] = '\0';
		_data->_length = length-1;
		
		GetAllocator()->DeleteArray<char>( pT_str );
	}
	return *this;
}

ByteString & ByteString::operator+=( char const * pstr )
{
	if ( pstr == nullptr )
	{
		return *this;
	}

	if ( strlen( pstr ) == 0 )
	{
		return *this;
	}

	if ( _data == nullptr )
	{
		size_t length = strlen(pstr);
		_data = GetAllocator()->New<ByteStringData>();
		_data->_reference.AddRef();
		_data->_str = GetAllocator()->NewArray<char>( length+1 );
		strcpy( _data->_str, pstr );
		_data->_length = length;
	}else{
		if ( _data->_str == nullptr )
		{
			size_t length = strlen(pstr);
			_data->_str = GetAllocator()->NewArray<char>( length+1 );
			strcpy( _data->_str, pstr );
			_data->_length = length;
			return *this;
		}

		char * pT_str = GetAllocator()->NewArray<char>( strlen(_data->_str)+1 );
		strcpy( pT_str, _data->_str );

		_data->_reference.DecRef();
		if ( _data->_reference == 0 )
		{
			if ( _data->_str )
			{
				GetAllocator()->DeleteArray<char>( _data->_str );
				_data->_str = nullptr;
			}
			_data->_length = 0;
			_data->_reference.AddRef();
		}else{
			_data = GetAllocator()->New<ByteStringData>();
			_data->_length = 0;
			_data->_reference.AddRef();
			_data->_str = nullptr;
		}

		size_t length = strlen(pT_str) + strlen(pstr);
		_data->_str = GetAllocator()->NewArray<char>( length+1 );
		strcpy( _data->_str, pT_str );
		strcat( _data->_str, pstr );
		_data->_length = length;
	}
	
	return *this;
}

ByteString & ByteString::operator+=( const ByteString & str )
{
	if ( str._data == nullptr || str._data->_str == nullptr )
	{
		return *this;
	}

	if ( this == &str )
	{
		char * pT_str = GetAllocator()->NewArray<char>( strlen(_data->_str)+1 );
		strcpy( pT_str, _data->_str );

		_data->_reference.DecRef();
		if ( _data->_reference == 0 )
		{
			if ( _data->_str )
			{
				GetAllocator()->DeleteArray<char>( _data->_str );
				_data->_str = nullptr;
			}
			_data->_length = 0;
			_data->_reference.AddRef();
		}else{
			_data = GetAllocator()->New<ByteStringData>();
			_data->_length = 0;
			_data->_reference.AddRef();
			_data->_str = nullptr;
		}

		size_t length = strlen(pT_str) * 2 + 1;
		_data->_str = GetAllocator()->NewArray<char>( length );
		strcpy( _data->_str, pT_str );
		strcat( _data->_str, pT_str );

		_data->_length = length-1;

		GetAllocator()->DeleteArray<char>( pT_str );
		pT_str = nullptr;
		
		return *this;
	}else{
		if ( _data == str._data )
		{
			char * pT_str = GetAllocator()->NewArray<char>( strlen(_data->_str)+1 );
			strcpy( pT_str, _data->_str );

			_data->_reference.DecRef();
			_data->_reference.DecRef();
			if ( _data->_reference == 0 )
			{
				if ( _data->_str )
				{
					GetAllocator()->DeleteArray<char>( _data->_str );
					_data->_str = nullptr;
				}
				_data->_length = 0;
				_data->_reference.AddRef();
				_data->_reference.AddRef();
			}else{
				_data = GetAllocator()->New<ByteStringData>();
				_data->_reference.AddRef();
				_data->_reference.AddRef();
				_data->_str = nullptr;
				_data->_length = 0;
			}

			size_t length = strlen(pT_str) * 2 + 1;
			_data->_str = GetAllocator()->NewArray<char>( length );
			strcpy( _data->_str, pT_str );
			strcat( _data->_str, pT_str );
			
			_data->_length = length-1;
			
			GetAllocator()->DeleteArray<char>( pT_str );
			pT_str = nullptr;
			
			return *this;
		}else{
			if ( _data->_str == nullptr )
			{
				size_t length = strlen(str._data->_str)+1;
				_data->_str = GetAllocator()->NewArray<char>( length );
				strcpy( _data->_str, str._data->_str );
				return *this;
			}

			char * pT_str = GetAllocator()->NewArray<char>( strlen(_data->_str)+1 );
			strcpy( pT_str, _data->_str );

			if ( _data->_reference == 1 )
			{
				if ( _data->_str )
				{
					GetAllocator()->DeleteArray<char>( _data->_str );
					_data->_str = nullptr;
					_data->_length = 0;
				}
			}else{
				_data->_reference.DecRef();
				_data = GetAllocator()->New<ByteStringData>();
				_data->_length = 0;
				_data->_reference.AddRef();
				_data->_str = nullptr;
			}

			size_t length = strlen( str._data->_str );
			length += strlen( pT_str )+1;
			_data->_str = GetAllocator()->NewArray<char>( length );
			strcpy( _data->_str, pT_str );
			strcat( _data->_str, str._data->_str );

			GetAllocator()->DeleteArray<char>( pT_str );
			pT_str = nullptr;
			
			_data->_length = length-1;
			
			return *this;
		}
	}
}

bool ByteString::operator!=( char ch )const
{
	if ( _data == nullptr )
	{
		if ( ch == 0 )
		{
			return false;
		}else{
			return true;
		}
	}else{
		if ( _data->_length != 1 )
		{
			return true;
		}else{
			if ( _data->_str[0] == ch )
			{
				return false;
			}else{
				return true;
			}
		}
	}
}

bool ByteString::operator!=( char const * pstr )const
{
	if ( _data == nullptr )
	{
		if ( pstr == nullptr || strlen( pstr ) == 0 )
		{
			return false;
		}else{
			return true;
		}
	}else{
		if ( pstr == nullptr )
		{
			if ( _data->_str == nullptr )
			{
				return false;
			}else{
				return true;
			}
		}else{
			if ( strcmp( _data->_str, pstr ) == 0 )
			{
				return false;
			}else{
				return true;
			}	
		}
	}
}

bool ByteString::operator!=( const ByteString & str )const
{
	if ( this == &str )
	{
		return false;
	}else{
		if ( _data == str._data )
		{
			return false;
		}else{
			if ( _data == nullptr )
			{
				return true;
			}else{
				if ( strcmp( _data->_str, str._data->_str ) == 0 )
				{
					return false;
				}else{
					return true;
				}
			}
		}
	}	
}

bool operator==( char ch, ByteString & str )
{
	return ( str == ch );
}

bool operator==( char const * pstr, const ByteString & str )
{
	return ( str == pstr );
}

ByteString operator+( char ch, const ByteString & str )
{
	ByteString te_str( ch, str.GetAllocator() );
	te_str+=str;
	return te_str;
}

ByteString operator+( char const * pstr, const ByteString & str )
{
	ByteString te_str( pstr, strlen(pstr), str.GetAllocator() );
	te_str+= str;
	return te_str;
}

bool operator!=( char ch, const ByteString & str )
{
	return ( str != ch );
}

bool operator!=( char const * pstr, const ByteString & str )
{
	return ( str != pstr );
}



struct WideStringData
{
    WideStringData()
    {
        _str = nullptr;
        _length = 0;
    }
    wchar_t *       _str;
    size_t          _length;
    ReferenceCount  _reference;
};
    
WideString::WideString( wchar_t wch, Allocator * pAllocator ) : BaseObject( pAllocator )
{
	if ( wch == '\0' )
	{
		_data = nullptr;
	}else{
		_data = GetAllocator()->New<WideStringData>();
		_data->_length = 1;
		_data->_reference.AddRef();
		_data->_str = GetAllocator()->NewArray<wchar_t >(2);
		_data->_str[0] = wch;
		_data->_str[1] = '\0';
	}
}

WideString::WideString( wchar_t const * pstr, size_t length /* = 0 */,  Allocator * pAllocator) : BaseObject( pAllocator )
{
	if ( pstr == nullptr )
	{
		_data = nullptr;
		return;
	}
	if ( length == 0 )
	{
		length = wcslen( pstr );
	}
	if ( length > 0 )
	{
		_data = GetAllocator()->New<WideStringData>();
		_data->_reference.AddRef();
		_data->_length = length;
		_data->_str = GetAllocator()->NewArray<wchar_t >( length+1 );
		wcscpy( _data->_str, pstr );
		_data->_str[length] = '\0';
	}else{
		_data = nullptr;
	}
}

WideString::WideString( const WideString & str ) : BaseObject( str.GetAllocator() )
{
	if ( str._data == nullptr )
	{
		_data = nullptr;
	}else{
		_data = str._data;
		str._data->_reference.AddRef();
	}
}

void WideString::Clear()
{
	if ( _data )
	{
		_data->_reference.DecRef();
		if ( _data->_reference == 0 && _data->_str )
		{
			GetAllocator()->DeleteArray<wchar_t >( _data->_str );
			_data->_str = nullptr;
			GetAllocator()->Delete<WideStringData>( _data );
		}

		_data = nullptr;
	}
}

WideString & WideString::operator=( wchar_t  wch )
{
	Clear();

	if ( wch == (wchar_t )0x0000 )
	{
		return *this;
	}

	_data = GetAllocator()->New<WideStringData>();
	_data->_length = 1;
	_data->_reference.AddRef();
	_data->_str = GetAllocator()->NewArray<wchar_t >( 2 );
	_data->_str[0] = wch;
	_data->_str[1] = (wchar_t)0x0000;

	return *this;
}

WideString& WideString::operator=( wchar_t const * pstr )
{
	Clear();

	if ( pstr  == nullptr )
	{
		return *this;
	}

	size_t length = wcslen( pstr );
	if ( length == 0 )
	{
		return *this;
	}

	_data = GetAllocator()->New<WideStringData>();
	_data->_reference.AddRef();
	_data->_length = length;
	_data->_str = GetAllocator()->NewArray<wchar_t >( length+1 );
	wcscpy( _data->_str, pstr );
	return *this;
}

WideString & WideString::operator=( const WideString & str )
{
	if ( this == &str )
	{
		return *this;
	}
	
	if ( _data == str._data )
	{
		return *this;
	}

	Clear();

	if ( str._data )
	{
		_data = str._data;
		str._data->_reference.AddRef();
	}

	return *this;
}

bool WideString::operator==( wchar_t wch )const
{
	if ( _data == nullptr || wcslen( _data->_str ) == 0 )
	{
		if ( wch == '\0' )
		{
			return true;
		}else{
			return false;
		}
	}else{
		if ( _data->_length != 1 )
		{
			return false;
		}else{
			if ( _data->_str[0] == wch )
			{
				return true;
			}else{
				return false;
			}
		}
	}
}

bool WideString::operator==( wchar_t const * pstr )const
{
	if ( _data == nullptr || wcslen( _data->_str ) == 0 )
	{
		if ( pstr == nullptr || wcslen( pstr ) == 0 )
		{
			return true;
		}else{
			return false;
		}
	}else{
		if ( pstr == nullptr )
		{
			if ( _data->_str == nullptr )
			{
				return true;
			}else{
				return false;
			}
		}else{
			if ( wcscmp( _data->_str, pstr ) == 0 )
			{
				return true;
			}else{
				return false;
			}	
		}
	}
}

bool WideString::operator==( const WideString & str )const
{
	if ( this == &str )
	{
		return true;
	}else{
		if ( _data == str._data )
		{
			return true;
		}else{
			if ( _data == nullptr )
			{
				return false;
			}else{
				if ( wcscmp( _data->_str, str._data->_str ) == 0 )
				{
					return true;
				}else{
					return false;
				}
			}
		}
	}
}

bool WideString::SetData( wchar_t * pData, size_t length )
{
	if ( pData == nullptr || length == 0 )
	{
		Clear();
		return TRUE;
	}
	if ( _data == nullptr )
	{
		_data = GetAllocator()->New<WideStringData>();
		_data->_reference.AddRef();
		_data->_length = length;
		_data->_str = GetAllocator()->NewArray<wchar_t >( length+1 );
		memcpy( _data->_str, pData, length * sizeof(wchar_t) );
		_data->_str[length] = '\0';
	}else{
		_data->_reference.DecRef();
		if ( _data->_reference == 0 )
		{
			if ( _data->_str )
			{
				GetAllocator()->DeleteArray<wchar_t >( _data->_str );
			}
			_data->_reference.AddRef();
			_data->_length = length;
			_data->_str = GetAllocator()->NewArray<wchar_t >( length+1 );
			memcpy( _data->_str, pData, length * sizeof(wchar_t) );
			_data->_str[length] = '\0';
		}else {
			_data = GetAllocator()->New<WideStringData>();
			_data->_reference.AddRef();
			_data->_length = length;
			_data->_str = GetAllocator()->NewArray<wchar_t >( length+1 );
			memcpy( _data->_str, pData, length * sizeof(wchar_t) );
			_data->_str[length] = '\0';
		}
	}
	return TRUE;
}
    
wchar_t const * WideString::GetData() const
{
    return ( _data ) ? _data->_str : nullptr;
}

size_t WideString::GetLength() const
{
	if ( _data == nullptr )
	{
		return 0;
	}else{
		return _data->_length;
	}
}

wchar_t WideString::operator[]( size_t index )const
{
	if ( _data != nullptr )
	{
		if ( index >= _data->_length )
		{
			return 0;
		}else if ( _data->_str )
		{
			return _data->_str[index];
		}else{
			return 0;
		}
	}else{
		return 0;
	}
}

INT32 WideString::GetInteger() const
{
	if ( GetLength() == 0 )
	{
		return 0;
	}else{
		bool bBegin = TRUE;
		bool bNegative = false;
		UINT32 iValue = 0;
		bool bSign = false;
		wchar_t  tmpChar = 0;
		for ( size_t i = 0; i < GetLength(); i++ )
		{
			if ( bBegin && i > 0 )
			{
				bBegin = false;
			}
			tmpChar = GetData()[i];
			switch( tmpChar )
			{
			case '+':
				if ( bSign == false && bBegin == TRUE )
				{
					bSign = TRUE;
					bNegative = false;
				}else{
					return 0;
				}
				break;
			case '-':
				if ( bSign == false && bBegin == TRUE )
				{
					bSign = TRUE;
					bNegative = TRUE;
				}else{
					return 0;
				}
				break;
			default:
				if ( L'0' > tmpChar || tmpChar > L'9' )
				{
					return 0;
				}else{
					iValue = iValue * 10 + ( tmpChar - L'0' ); 
				}
				break;
			}
		}
		if( bNegative == TRUE )
		{
			return 0 - iValue;
		}else{
			return iValue;
		}
	}
}

FLOAT WideString::GetFloat() const
{
	if ( GetLength() == 0 )
	{
		return 0;
	}else{
		bool bNegative = false;
		bool bBegin = TRUE;
		size_t lPointBit = 1;
		FLOAT fValue = 0;
		bool bPoint = false;
		bool bSign = false;
		wchar_t  tmpChar = 0;
		for ( size_t i = 0; i < GetLength(); i++ )
		{
			if ( bBegin && i > 0 )
			{
				bBegin = false;
			}
			tmpChar = GetData()[i];
			switch( tmpChar )
			{
			case L'+':
				if ( bSign == false && bBegin == TRUE )
				{
					bSign = TRUE;
					bNegative = false;
				}else{
					return 0;
				}
				break;
			case L'-':
				if ( bSign == false && bBegin == TRUE )
				{
					bSign = TRUE;
					bNegative = TRUE;
				}else{
					return 0;
				}
				break;
			case L'.':
				if ( bPoint == false )
				{
					bPoint = TRUE;
				}else{
					return 0;
				}
				break;
			default:
				if ( L'0' > tmpChar || tmpChar > L'9' )
				{
					return 0;
				}else{
					if ( bPoint == false )
					{
						fValue = fValue * 10 + ( tmpChar - L'0' );
					}else{
						fValue = fValue + ( tmpChar - L'0' ) / ( lPointBit * 10 );
						lPointBit++;
					}
				}
				break;
			}
		}
		if ( bNegative == TRUE )
		{
			return 0 - fValue;
		}else{
			return fValue;
		}
	}
}

WideString WideString::operator+( wchar_t wch )
{
	if ( _data == nullptr || _data->_str == nullptr )
	{
		return WideString( wch, GetAllocator() );
	}

	size_t length = _data->_length + 2;
	wchar_t  * pTe_str = GetAllocator()->NewArray<wchar_t >(length);

	wcscpy( pTe_str, _data->_str );
	pTe_str[length-2] = wch;
	pTe_str[length-1] = '\0';

	WideString t_string(pTe_str);
	GetAllocator()->DeleteArray<wchar_t >( pTe_str );
	return t_string;
}

WideString WideString::operator+( wchar_t const * pstr )
{
	if ( pstr == nullptr )
	{
		return WideString(*this);
	}

	if ( _data == nullptr || _data->_str == nullptr )
	{
		return WideString( pstr );
	}
	
	size_t length = _data->_length + wcslen(pstr);
	wchar_t  * pTe_str = GetAllocator()->NewArray<wchar_t >( length+1 );
	wcscpy( pTe_str, _data->_str );
	wcscat( pTe_str, pstr );
    _data->_length = length;

	WideString t_string(pTe_str);
	GetAllocator()->DeleteArray<wchar_t >( pTe_str );
	return t_string;
}

WideString WideString::operator+( const WideString & str )
{
	if ( str._data == nullptr )
	{
		return WideString(*this);
	}
	
	if ( _data == nullptr || _data->_str == nullptr )
	{
		return WideString( str );
	}
	
	size_t length = _data->_length + str._data->_length;
	wchar_t  * pTe_str = GetAllocator()->NewArray<wchar_t >( length+1 );
	
	wcscpy( pTe_str, _data->_str );
	wcscat( pTe_str, str._data->_str );
    _data->_length = length;
	
	WideString t_string(pTe_str);
	GetAllocator()->DeleteArray<wchar_t >( pTe_str );
	return t_string;
}

WideString & WideString::operator+=( wchar_t wch )
{
	if ( wch == 0 )
	{
		return *this;
	}

	if ( _data == nullptr )
	{
		_data = GetAllocator()->New<WideStringData>();
		_data->_reference.AddRef();
		_data->_length = 1;
		_data->_str = GetAllocator()->NewArray<wchar_t >(2);
		_data->_str[0] = wch;
		_data->_str[1] = '\0';
	}else{
		if ( _data->_str == nullptr )
		{
			_data->_length = 1;
			_data->_reference.AddRef();
			_data->_str = GetAllocator()->NewArray<wchar_t >(2);
			_data->_str[0] = wch;
			_data->_str[1] = '\0';
			return *this;
		}
		
		wchar_t *  pTe_str = GetAllocator()->NewArray<wchar_t >( wcslen(_data->_str)+2 );
		wcscpy( pTe_str, _data->_str );
		
		_data->_reference.DecRef();
		if ( _data->_reference == 0 )
		{
			if ( _data->_str )
			{
				GetAllocator()->DeleteArray<wchar_t >( _data->_str );
				_data->_str = nullptr;
			}
			_data->_length = 0;
			_data->_reference.AddRef();
		}else{
			_data = GetAllocator()->New<WideStringData>();
			_data->_length = 0;
			_data->_reference.AddRef();
			_data->_str = nullptr;
		}
		
		size_t dwBufferSize = wcslen( pTe_str )+2;
		_data->_str = GetAllocator()->NewArray<wchar_t >( dwBufferSize );
		wcscpy( _data->_str, pTe_str );
		_data->_str[dwBufferSize-2] = wch;
		_data->_str[dwBufferSize-1] = '\0';
		_data->_length = dwBufferSize-1;
		
		GetAllocator()->DeleteArray<wchar_t >( pTe_str );
	}
	return *this;
}

WideString & WideString::operator+=( wchar_t const * pstr )
{
	if ( pstr == nullptr )
	{
		return *this;
	}

	if ( wcslen( pstr ) == 0 )
	{
		return *this;
	}

	if ( _data == nullptr )
	{
		size_t length = wcslen(pstr);
		_data = GetAllocator()->New<WideStringData>();
		_data->_reference.AddRef();
		_data->_str = GetAllocator()->NewArray<wchar_t >( length+1 );
		wcscpy( _data->_str, pstr );
		_data->_length = length;
	}else{
		if ( _data->_str == nullptr )
		{
			size_t length = wcslen(pstr);
			_data->_str = GetAllocator()->NewArray<wchar_t >( length+1 );
			wcscpy( _data->_str, pstr );
			_data->_length = length;
			_data->_reference.AddRef();
			return *this;
		}
		
		wchar_t * pTe_str = GetAllocator()->NewArray<wchar_t >( wcslen(_data->_str)+1 );
		wcscpy( pTe_str, _data->_str );
		
		_data->_reference.DecRef();
		if ( _data->_reference == 0 )
		{
			if ( _data->_str )
			{
				GetAllocator()->DeleteArray<wchar_t >( _data->_str );
				_data->_str = nullptr;
			}
			_data->_length = 0;
			_data->_reference.AddRef();
		}else{
			_data = GetAllocator()->New<WideStringData>();
			_data->_length = 0;
			_data->_reference.AddRef();
			_data->_str = nullptr;
		}
		
		size_t length = wcslen(pTe_str) + wcslen(pstr);
		_data->_str = GetAllocator()->NewArray<wchar_t >( length+1 );
		wcscpy( _data->_str, pTe_str );
		wcscat( _data->_str, pstr );
		_data->_length = length;
	}
	
	return *this;
}

WideString & WideString::operator+=( const WideString & str )
{
	if ( str._data == nullptr || str._data->_str == nullptr )
	{
		return *this;
	}

	if ( this == &str )
	{
		wchar_t * pTe_str = GetAllocator()->NewArray<wchar_t >( wcslen(_data->_str)+1 );
		wcscpy( pTe_str, _data->_str );

		_data->_reference.DecRef();
		if ( _data->_reference == 0 )
		{
			if ( _data->_str )
			{
				GetAllocator()->DeleteArray<wchar_t >( _data->_str );
				_data->_str = nullptr;
			}
			_data->_length = 0;
			_data->_reference.AddRef();
		}else{
			_data = GetAllocator()->New<WideStringData>();
			_data->_length = 0;
			_data->_reference.AddRef();
			_data->_str = nullptr;
		}

		size_t length = wcslen(pTe_str) * 2;
		_data->_str = GetAllocator()->NewArray<wchar_t >( length+1 );
		wcscpy( _data->_str, pTe_str );
		wcscat( _data->_str, pTe_str );

		_data->_length = length;

		GetAllocator()->DeleteArray<wchar_t >( pTe_str );
		pTe_str = nullptr;
		
		return *this;
	}else{
		if ( _data == str._data )
		{
			wchar_t *  pTe_str = GetAllocator()->NewArray<wchar_t >( wcslen(_data->_str)+1 );
			wcscpy( pTe_str, _data->_str );

			_data->_reference.DecRef();
			_data->_reference.DecRef();
			if ( _data->_reference == 0 )
			{
				if ( _data->_str )
				{
					GetAllocator()->DeleteArray<wchar_t >( _data->_str );
					_data->_str = nullptr;
				}
				_data->_length = 0;
				_data->_reference.AddRef();
				_data->_reference.AddRef();
			}else{
				_data = GetAllocator()->New<WideStringData>();
				_data->_reference.AddRef();
				_data->_reference.AddRef();
				_data->_str = nullptr;
				_data->_length = 0;
			}

			size_t length = wcslen(pTe_str) * 2;
			_data->_str = GetAllocator()->NewArray<wchar_t >( length+1 );
			wcscpy( _data->_str, pTe_str );
			wcscat( _data->_str, pTe_str );
			
			_data->_length = length;
			
			GetAllocator()->DeleteArray<wchar_t >( pTe_str );
			pTe_str = nullptr;
			
			return *this;
		}else{
			if ( _data->_str == nullptr )
			{
				size_t length = wcslen(str._data->_str);
				_data->_str = GetAllocator()->NewArray<wchar_t >( length+1 );
				wcscpy( _data->_str, str._data->_str );
				return *this;
			}

			wchar_t * pTe_str = GetAllocator()->NewArray<wchar_t >( wcslen(_data->_str)+1 );
			wcscpy( pTe_str, _data->_str );

			if ( _data->_reference == 1 )
			{
				if ( _data->_str )
				{
					GetAllocator()->DeleteArray<wchar_t >( _data->_str );
					_data->_str = nullptr;
					_data->_length = 0;
				}
			}else{
				_data->_reference.DecRef();
				_data = GetAllocator()->New<WideStringData>();
				_data->_length = 0;
				_data->_reference.AddRef();
				_data->_str = nullptr;
			}

			size_t length = wcslen( str._data->_str );
			length += wcslen( pTe_str );
			_data->_str = GetAllocator()->NewArray<wchar_t >( length+1 );
			wcscpy( _data->_str, pTe_str );
			wcscat( _data->_str, str._data->_str );

			GetAllocator()->DeleteArray<wchar_t >( pTe_str );
			pTe_str = nullptr;
			
			_data->_length = length;
			
			return *this;
		}
	}
}

bool WideString::operator!=( wchar_t  wch )const
{
	if ( _data == nullptr )
	{
		if ( wch == 0 )
		{
			return false;
		}else{
			return true;
		}
	}else{
		if ( _data->_length != 1 )
		{
			return true;
		}else{
			if ( _data->_str[0] == wch )
			{
				return false;
			}else{
				return true;
			}
		}
	}
}

bool WideString::operator!=( wchar_t const * pstr  )const
{
	if ( _data == nullptr )
	{
		if ( pstr == nullptr || wcslen( pstr ) == 0 )
		{
			return false;
		}else{
			return true;
		}
	}else{
		if ( pstr == nullptr )
		{
			if ( _data->_str == nullptr )
			{
				return false;
			}else{
				return true;
			}
		}else{
			if ( wcscmp( _data->_str, pstr ) == 0 )
			{
				return false;
			}else{
				return true;
			}	
		}
	}
}

bool WideString::operator!=( const WideString & str )const
{
	if ( this == &str )
	{
		return false;
	}else{
		if ( _data == str._data )
		{
			return false;
		}else{
			if ( _data == nullptr )
			{
				return true;
			}else{
				if ( wcscmp( _data->_str, str._data->_str ) == 0 )
				{
					return false;
				}else{
					return true;
				}
			}
		}
	}	
}

bool operator==( wchar_t  wch, WideString & str )
{
	return ( str == wch );
}

bool operator==( wchar_t const * pstr, const WideString & str )
{
	return ( str == pstr );
}

WideString operator+( wchar_t wch, const WideString & str )
{
	WideString te_str( wch, str.GetAllocator() );
	te_str+=str;
	return te_str;
}

WideString operator+( wchar_t const * pstr, const WideString & str )
{
	WideString te_str( pstr, wcslen(pstr), str.GetAllocator() );
	te_str+= str;
	return te_str;
}

bool operator!=( wchar_t  wch, const WideString & str )
{
	return ( str != wch );
}

bool operator!=( wchar_t const * pstr, const WideString & str )
{
	return ( str != pstr );
}

UINT32 StringToUINT32( const ByteString & str )
{
    if ( str.GetLength() == 0 )
    {
        return 0;
    }
    size_t length = 4;
    if ( str.GetLength() < length )
    {
        length = str.GetLength();
    }
    UINT32 valRet = 0;
    for ( size_t i = length; i > 0; --i )
    {
        valRet = valRet<<8;
        valRet |= str.GetData()[i-1];
    }
    return valRet;
}
    
}//end of namespce chelib
