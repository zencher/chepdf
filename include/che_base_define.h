#ifndef _CHE_BASE_DEFINE_
#define _CHE_BASE_DEFINE_

#include <stdint.h>

#include "config.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

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
