#ifndef _CHE_PDF_CRYPTO_H_
#define _CHE_PDF_CRYPTO_H_

#include "che_base_string.h"

namespace chepdf {

#define CRYPTO_KEYLENGTH_40		40
#define CRYPTO_KEYLENGTH_56		56
#define CRYPTO_KEYLENGTH_80		80
#define CRYPTO_KEYLENGTH_96		96
#define CRYPTO_KEYLENGTH_128    128

#define CRYPTO_PERMISSIONS_PRINT		0x00000004
#define CRYPTO_PERMISSIONS_EDIT         0x00000008
#define CRYPTO_PERMISSIONS_COPY         0x00000010
#define CRYPTO_PERMISSIONS_EDITNOTES	0x00000020
#define CRYPTO_PERMISSIONS_FILLANDSIGN	0x00000100
#define CRYPTO_PERMISSIONS_ACCESSIBLE	0x00000200
#define CRYPTO_PERMISSIONS_DOCACCEMBLY	0x00000400
#define CRYPTO_PERMISSIONS_HighPrint	0x00000800

#define CRYPTO_ALGORITHM_RC4	1
#define CRYPTO_ALGORITHM_AESV2	2

class PdfCrypto : public BaseObject
{
public:
	PdfCrypto(const ByteString id, uint8_t o[32], uint8_t u[32], uint8_t algorithm,
                uint8_t version, uint8_t revision, uint8_t keyLength, bool bMetaData,
                uint32_t p, Allocator * allocator = nullptr);

	PdfCrypto(const ByteString id, uint8_t algorithm, uint8_t version, uint8_t revision,
              uint8_t key_length, bool b_meta_data, uint32_t p, Allocator * allocator = nullptr);
    
    virtual ~PdfCrypto() {}

	void Init(const ByteString userPassword, const ByteString ownerPassword);

	bool Authenticate(const ByteString & password);

	bool IsPasswordOK() { return b_password_ok_; }

	uint32_t Encrypt(ByteString & str, uint32_t objNum, uint32_t genNum);
	uint32_t Encrypt(uint8_t * pData, uint32_t length, uint32_t objNum, uint32_t genNum);
	uint32_t Encrypt(ByteString & str, uint8_t objKey[16], uint32_t objKeyLen);
	uint32_t Encrypt(uint8_t * pData, uint32_t length, uint8_t objKey[16], uint32_t objKeyLen);
	uint32_t Decrypt(ByteString & str, uint32_t objNum, uint32_t genNum);
	uint32_t Decrypt(uint8_t * pData, uint32_t length, uint32_t objNum, uint32_t genNum);
	uint32_t Decrypt(ByteString & str, uint8_t objKey[16], uint32_t objKeyLen);
	uint32_t Decrypt(uint8_t * pData, uint32_t length, uint8_t objKey[16], uint32_t objKeyLen);

private:
	void ComputeEncryptionKey(uint8_t userPad[32], uint8_t encryptionKeyRet[16]);
	void ComputeOwnerKey(uint8_t userPad[32], uint8_t ownerPad[32], uint8_t ownerKeyRet[32], bool bAuth = false);
	void ComputeUserKey(uint8_t encryptionKey[16], uint8_t userKeyRet[32]);
	void CreateObjKey(uint32_t objNum, uint32_t genNum, uint8_t objkey[16], uint32_t * pObjKeyLengthRet);
	void PadPassword(const ByteString & password, uint8_t pswd[32]);
	void RC4(uint8_t * key, uint32_t keyLength, uint8_t * data, uint32_t dataLength, uint8_t * dataRet);
	
	uint32_t AESEncrypt(uint8_t * key, uint32_t keyLength, uint8_t * data, uint32_t dataLength, uint8_t * dataRet);
	uint32_t AESDecrypt(uint8_t * key, uint32_t keyLength, uint8_t * data, uint32_t dataLength, uint8_t * dataRet);

private:
    bool            b_password_ok_;
	bool            b_meta_data_;
	uint8_t         algorithm_;
	uint8_t         version_;
	uint8_t         revision_;
	uint8_t         key_length_;
	uint32_t        p_;
	uint8_t         o_[32];
	uint8_t         u_[32];
	uint8_t         encryption_key_[16];
    ByteString      id_;
};
    
}//namespace

#endif
