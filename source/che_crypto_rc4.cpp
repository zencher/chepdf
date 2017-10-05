#include "../include/che_crypto_rc4.h"

namespace chepdf {

void CryptoRC4::Encrypt(const uint8_t * const key, uint32_t keyLength, uint8_t * data, uint32_t dataLength, uint8_t * dataRet)
{
	uint8_t rc4[256];
	uint8_t t = 0;
	uint32_t j = 0;
	for (uint32_t i = 0; i < 256; ++i)
	{
		rc4[i] = static_cast<uint8_t>(i);
	}
	for (uint32_t l = 0; l < 256; ++l)
	{
		t = rc4[l];
		j = (j + static_cast<uint32_t>(t)+static_cast<uint32_t>(key[l % keyLength])) % 256;
		rc4[l] = rc4[j];
		rc4[j] = t;
	}
	uint32_t a = 0;
	uint32_t b = 0;
	uint8_t k = 0;
	for (uint32_t m = 0; m < dataLength; ++m)
	{
		a = (a + 1) % 256;
		t = rc4[a];
		b = (b + static_cast<uint32_t>(t)) % 256;
		rc4[a] = rc4[b];
		rc4[b] = t;
		k = rc4[(static_cast<uint32_t>(rc4[a]) + static_cast<uint32_t>(rc4[b])) % 256];
		dataRet[m] = data[m] ^ k;
	}
}

}//namespace
