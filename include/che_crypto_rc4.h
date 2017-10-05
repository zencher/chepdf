#ifndef _CHE_CRYPTO_RC4_H_
#define _CHE_CRYPTO_RC4_H_

#include "che_base_object.h"

namespace chepdf {

class CryptoRC4
{
public:
	static void Encrypt(const uint8_t * const key, uint32_t keyLength, uint8_t * data, uint32_t dataLength, uint8_t * dataRet);
};
    
}//namespace

#endif
