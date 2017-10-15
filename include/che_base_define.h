#ifndef _CHE_BASE_DEFINE_
#define _CHE_BASE_DEFINE_

#include <stdint.h>

/*typedef uint8_t			BYTE;
typedef uint8_t *		PBYTE;
typedef uint8_t const *	PCBYTE;
typedef int16_t			INT16;
typedef uint16_t		UINT16;
typedef int32_t			INT32;
typedef uint32_t		UINT32;
typedef int64_t			INT64;
typedef uint64_t		UINT64;*/

#ifdef _MSC_VER
	#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
	#endif
#endif

typedef wchar_t			WCHAR;
typedef wchar_t *		PWSTR;
typedef wchar_t const *	PCWSTR;
typedef float			FLOAT;
typedef double			DOUBLE;
typedef uint32_t    	ARGB;

#endif
