#ifndef _CHE_CRYPTO_AES_H_
#define _CHE_CRYPTO_AES_H_

#include "che_base_object.h"

namespace chepdf {

// Error codes
#define RIJNDAEL_SUCCESS                     0
#define RIJNDAEL_UNSUPPORTED_MODE           -1
#define RIJNDAEL_UNSUPPORTED_DIRECTION      -2
#define RIJNDAEL_UNSUPPORTED_KEY_LENGTH     -3
#define RIJNDAEL_BAD_KEY                    -4
#define RIJNDAEL_NOT_INITIALIZED            -5
#define RIJNDAEL_BAD_DIRECTION              -6
#define RIJNDAEL_CORRUPTED_DATA             -7


#define _MAX_KEY_COLUMNS (256/32)
#define _MAX_ROUNDS      14
#define MAX_IV_SIZE      16


class CryptoRijndael
{	
public:
	enum Direction { Encrypt , Decrypt };
	enum Mode { ECB , CBC , CFB1 };
	enum KeyLength { Key16Bytes , Key24Bytes , Key32Bytes };
	
	CryptoRijndael();
	~CryptoRijndael();

protected:
	enum State { Valid , Invalid };
	State       state_;
	Mode        mode_;
	Direction   direction_;
    uint32_t    rounds_;
	uint8_t     init_vector_[MAX_IV_SIZE];
	uint8_t     expanded_key_[_MAX_ROUNDS+1][4][4];

public:
	int32_t init(Mode mode, Direction dir, const uint8_t * key, KeyLength keyLen, uint8_t * initVector = 0);
	int32_t blockEncrypt(const uint8_t * input, int32_t inputLen, uint8_t * outBuffer);
	int32_t padEncrypt(const uint8_t * input, int32_t inputOctets, uint8_t * outBuffer);
	int32_t blockDecrypt(const uint8_t * input, int32_t inputLen, uint8_t * outBuffer);
	int32_t padDecrypt(const uint8_t * input, int32_t inputOctets, uint8_t * outBuffer);

protected:
	void keySched(uint8_t key[_MAX_KEY_COLUMNS][4]);
	void keyEncToDec();
	void encrypt(const uint8_t a[16], uint8_t b[16]);
	void decrypt(const uint8_t a[16], uint8_t b[16]);
};

}//namespace

#endif
