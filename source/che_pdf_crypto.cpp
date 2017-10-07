#include "../include/che_pdf_crypto.h"

#include "../include/che_hash_md5.h"
#include "../include/che_crypto_rc4.h"
#include "../include/che_crypto_aes.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>

namespace chepdf {
    
static uint8_t padding[] = "\x28\xBF\x4E\x5E\x4E\x75\x8A\x41\x64\x00\x4E\x56\xFF\xFA\x01\x08\x2E\x2E\x00\xB6\xD0\x68\x3E\x80\x2F\x0C\xA9\xFE\x64\x53\x69\x7A";

PdfCrypto::PdfCrypto(const ByteString id, uint8_t O[32], uint8_t U[32], uint8_t algorithm,
	uint8_t version, uint8_t revision, uint8_t keyLength, bool bMetaData, uint32_t P,
	Allocator * allocator) : BaseObject(allocator), id_(allocator)
{
	id_ = id;
	for (uint32_t i = 0; i < 32; i++)
	{
		o_[i] = O[i];
		u_[i] = U[i];
	}
	algorithm_ = algorithm;
	version_ = version;
	revision_ = revision;
	key_length_ = keyLength;
	b_meta_data_ = bMetaData;
	p_ = P;
	b_password_ok_ = false;
}

PdfCrypto::PdfCrypto(const ByteString id, uint8_t algorithm, uint8_t version, uint8_t revision, uint8_t keyLength,
	bool bMetaData, uint32_t P, Allocator * allocator) : BaseObject(allocator), id_(allocator)
{
	id_ = id;
	algorithm_ = algorithm;
	version_ = version;
	revision_ = revision;
	key_length_ = keyLength;
	b_meta_data_ = bMetaData;
	p_ = P;
	b_password_ok_ = false;
}

void PdfCrypto::Init(const ByteString userPassword, const ByteString ownerPassword)
{
	uint8_t userPad[32];
	uint8_t ownerPad[32];
	uint8_t encryptionKey[16];
	PadPassword(userPassword, userPad);
	PadPassword(ownerPassword, ownerPad);
	if (ownerPassword.GetLength() == 0)
	{
		ComputeOwnerKey(userPad, userPad, o_, false);
	}
	else{
		ComputeOwnerKey(userPad, ownerPad, o_, false);
	}
	ComputeEncryptionKey(userPad, encryptionKey);
	ComputeUserKey(encryptionKey, u_);
	b_password_ok_ = TRUE;
}

void PdfCrypto::PadPassword(const ByteString & password, unsigned char pswd[32])
{
	size_t m = password.GetLength();

	if (m > 32) m = 32;

	uint32_t j = 0, p = 0;
	for (j = 0; j < m; j++)
	{
		pswd[p++] = password[j];
	}
	for (j = 0; p < 32 && j < 32; j++)
	{
		pswd[p++] = padding[j];
	}
}

void PdfCrypto::ComputeOwnerKey(uint8_t userPad[32], uint8_t ownerPad[32], uint8_t ownerKeyRet[32], bool bAuth)
{
	uint8_t mkey[16];
	uint8_t digest[16];
	uint32_t lengthInByte = key_length_ / 8;

	HashMD5 md5;
	md5.Init();
	md5.Update(ownerPad, 32);
	md5.Final(digest);

	if ((revision_ == 3) || (revision_ == 4))
	{
		for (uint32_t i = 0; i < 50; i++)
		{
			md5.Init();
			md5.Update(digest, lengthInByte);
			md5.Final(digest);
		}
		memcpy(ownerKeyRet, userPad, 32);

		for (uint32_t j = 0; j < 20; j++)
		{
			for (uint32_t k = 0; k < lengthInByte; k++)
			{
				if (bAuth)
				{
					mkey[k] = (uint8_t)(digest[k] ^ (19 - j));
				}
				else
				{
					mkey[k] = (uint8_t)(digest[k] ^ j);
				}
			}
			RC4(mkey, lengthInByte, ownerKeyRet, 32, ownerKeyRet);
		}
	}
	else{
		RC4(digest, 5, userPad, 32, ownerKeyRet);
	}
}

void PdfCrypto::CreateObjKey(uint32_t objNum, uint32_t genNum, uint8_t objkey[16], uint32_t* pObjKeyLengthRet)
{
	uint32_t keyLengthInByte = key_length_ / 8;
	uint32_t objKeyLength = keyLengthInByte + 5;
	uint8_t	tmpkey[16 + 5 + 4];

	for (uint32_t j = 0; j < keyLengthInByte; j++)
	{
		tmpkey[j] = encryption_key_[j];
	}
	tmpkey[keyLengthInByte + 0] = (uint8_t)(0xff & objNum);
	tmpkey[keyLengthInByte + 1] = (uint8_t)(0xff & (objNum >> 8));
	tmpkey[keyLengthInByte + 2] = (uint8_t)(0xff & (objNum >> 16));
	tmpkey[keyLengthInByte + 3] = (uint8_t)(0xff & genNum);
	tmpkey[keyLengthInByte + 4] = (uint8_t)(0xff & (genNum >> 8));

	if (algorithm_ == CRYPTO_ALGORITHM_AESV2)
	{
		objKeyLength += 4;
		tmpkey[keyLengthInByte + 5] = 0x73;
		tmpkey[keyLengthInByte + 6] = 0x41;
		tmpkey[keyLengthInByte + 7] = 0x6c;
		tmpkey[keyLengthInByte + 8] = 0x54;
	}

	HashMD5 md5;
	md5.Init();
	md5.Update(tmpkey, objKeyLength);
	md5.Final(objkey);

	*pObjKeyLengthRet = (keyLengthInByte < 11) ? keyLengthInByte + 5 : 16;
}

bool PdfCrypto::Authenticate(const ByteString & password)
{
	bool bRet = TRUE;
	uint8_t padpswd[32];
	uint8_t userKey[32];
	uint8_t encrypt[16];

	PadPassword(password, padpswd);
	ComputeEncryptionKey(padpswd, encrypt);
	ComputeUserKey(encrypt, userKey);

	uint32_t kmax = (revision_ == 2) ? 32 : 16;
	for (uint32_t k = 0; bRet && k < kmax; k++)
	{
		bRet = bRet && (userKey[k] == u_[k]);
	}
	if (!bRet)
	{
		bRet = TRUE;
		unsigned char userpswd[32];
		ComputeOwnerKey(o_, padpswd, userpswd, TRUE);
		ComputeEncryptionKey(userpswd, encrypt);
		ComputeUserKey(encrypt, userKey);
		kmax = 32;
		for (uint32_t k = 0; bRet && k < kmax; k++)
		{
			bRet = bRet && (userKey[k] == u_[k]);
		}
	}
	if (bRet == TRUE)
	{
		b_password_ok_ = TRUE;
		for (uint32_t i = 0; i < 16; i++)
		{
			encryption_key_[i] = encrypt[i];
		}
	}
	return bRet;
}

void PdfCrypto::ComputeEncryptionKey(uint8_t userPad[32], uint8_t encryptionKeyRet[16])
{
	uint32_t keyLengthInByte = key_length_ / 8;

	HashMD5 md5;
	md5.Init();
	md5.Update(userPad, 32);
	md5.Update(o_, 32);

	uint8_t ext[4];
	ext[0] = (uint8_t)(p_ & 0xff);
	ext[1] = (uint8_t)((p_ >> 8) & 0xff);
	ext[2] = (uint8_t)((p_ >> 16) & 0xff);
	ext[3] = (uint8_t)((p_ >> 24) & 0xff);

	md5.Update(ext, 4);

	uint8_t * docId = nullptr;
	if (id_.GetLength() > 0)
	{
		docId = GetAllocator()->NewArray<uint8_t>(id_.GetLength());
		uint32_t j;
		for (j = 0; j < id_.GetLength(); j++)
		{
			docId[j] = static_cast<unsigned char>(id_[j]);
		}
		md5.Update(docId, (uint32_t)id_.GetLength());
	}

	if (b_meta_data_ == false && revision_ >= 4)
	{
		uint8_t ext[4];
		ext[0] = 0xFF;
		ext[1] = 0xFF;
		ext[2] = 0xFF;
		ext[3] = 0xFF;
		md5.Update(ext, 4);
	}
	md5.Final(encryptionKeyRet);

	if (revision_ >= 3)
	{
		for (uint32_t k = 0; k < 50; k++)
		{
			md5.Init();
			md5.Update(encryptionKeyRet, (uint32_t)keyLengthInByte);
			md5.Final(encryptionKeyRet);
		}
	}
}

void PdfCrypto::ComputeUserKey(uint8_t encryptionKey[16], uint8_t userKeyRet[32])
{
	uint32_t keyLengthInByte = key_length_ / 8;

	if (revision_ >= 3)
	{
		HashMD5 md5;
		md5.Init();
		md5.Update(padding, 32);

		if (id_.GetLength() > 0)
		{
			uint8_t * docId = GetAllocator()->NewArray<uint8_t>(id_.GetLength());
			for (uint32_t j = 0; j < id_.GetLength(); j++)
			{
				docId[j] = static_cast<unsigned char>(id_[j]);
			}
			md5.Update(docId, (uint32_t)id_.GetLength());
			GetAllocator()->DeleteArray<uint8_t>(docId);
		}

		uint8_t digest[16];
		md5.Final(digest);
		uint32_t k;
		for (k = 0; k < 16; k++)
		{
			userKeyRet[k] = digest[k];
		}
		for (k = 16; k < 32; k++)
		{
			userKeyRet[k] = 0;
		}
		for (k = 0; k < 20; k++)
		{
			for (uint32_t j = 0; j < keyLengthInByte; j++)
			{
				digest[j] = static_cast<unsigned char>(encryptionKey[j] ^ k);
			}
			RC4(digest, keyLengthInByte, userKeyRet, 16, userKeyRet);
		}
	}
	else
	{
		RC4(encryptionKey, keyLengthInByte, padding, 32, userKeyRet);
	}
}

uint32_t PdfCrypto::Encrypt(ByteString & str, uint32_t objNum, uint32_t genNum)
{
	uint8_t objKey[16];
	uint32_t objKeyLength = 0;
	CreateObjKey(objNum, genNum, objKey, &objKeyLength);
	return Encrypt(str, objKey, objKeyLength);
}

uint32_t PdfCrypto::Encrypt(uint8_t * pData, uint32_t length, uint32_t objNum, uint32_t genNum)
{
	uint8_t objKey[16];
	uint32_t objKeyLength = 0;
	CreateObjKey(objNum, genNum, objKey, &objKeyLength);
	return Encrypt(pData, length, objKey, objKeyLength);
}

uint32_t PdfCrypto::Encrypt(ByteString & str, uint8_t objKey[16], uint32_t objKeyLen)
{
	uint32_t length = str.GetLength();
	uint8_t * pData = GetAllocator()->NewArray<uint8_t>(length + 16);
	for (uint32_t i = 0; i < length; i++)
	{
		pData[i] = (uint8_t)(str[i]);
	}
	if (algorithm_ == CRYPTO_ALGORITHM_RC4)
	{
		RC4(objKey, objKeyLen, pData, length, pData);
		str.SetData(pData, length);
	}
	else if (algorithm_ == CRYPTO_ALGORITHM_AESV2)
	{
		AESEncrypt(objKey, objKeyLen, pData, length, pData);
		str.SetData(pData, length + 16);
	}
	GetAllocator()->DeleteArray<uint8_t>(pData);
	return str.GetLength();
}

uint32_t PdfCrypto::Encrypt(uint8_t * pData, uint32_t length, uint8_t objKey[16], uint32_t objKeyLen)
{
	if (pData == nullptr || length == 0)
	{
		return 0;
	}
	if (algorithm_ == CRYPTO_ALGORITHM_RC4)
	{
		RC4(objKey, objKeyLen, pData, length, pData);
	}
	else if (algorithm_ == CRYPTO_ALGORITHM_AESV2)
	{
		length = AESEncrypt(objKey, objKeyLen, pData, length, pData);
	}
	return length;
}

uint32_t PdfCrypto::Decrypt(ByteString & str, uint32_t objNum, uint32_t genNum)
{
	uint8_t objKey[16];
	uint32_t objKeyLength = 0;
	CreateObjKey(objNum, genNum, objKey, &objKeyLength);
	return Decrypt(str, objKey, objKeyLength);
}

uint32_t PdfCrypto::Decrypt(uint8_t * pData, uint32_t length, uint32_t objNum, uint32_t genNum)
{
	uint8_t objKey[16];
	uint32_t objKeyLength = 0;
	CreateObjKey(objNum, genNum, objKey, &objKeyLength);
	return Decrypt(pData, length, objKey, objKeyLength);
}

uint32_t PdfCrypto::Decrypt(ByteString & str, uint8_t objKey[16], uint32_t objKeyLen)
{
	uint32_t length = str.GetLength();
	uint8_t * pData = GetAllocator()->NewArray<uint8_t>(length);
	for (uint32_t i = 0; i < length; i++)
	{
		pData[i] = (uint8_t)(str[i]);
	}
	if (algorithm_ == CRYPTO_ALGORITHM_RC4)
	{
		RC4(objKey, objKeyLen, pData, length, pData);
		str.SetData(pData, length);
	}
	else if (algorithm_ == CRYPTO_ALGORITHM_AESV2)
	{
		length = AESDecrypt(objKey, objKeyLen, pData, length, pData);
		if (length > 0)
		{
			str.SetData(pData, length);
		}
	}
	GetAllocator()->DeleteArray<uint8_t>(pData);
	return length;
}

uint32_t PdfCrypto::Decrypt(uint8_t * pData, uint32_t length, uint8_t objKey[16], uint32_t objKeyLen)
{
	if (pData == nullptr || length == 0)
	{
		return 0;
	}
	if (algorithm_ == CRYPTO_ALGORITHM_RC4)
	{
		RC4(objKey, objKeyLen, pData, length, pData);
	}
	else if (algorithm_ == CRYPTO_ALGORITHM_AESV2)
	{
		length = AESDecrypt(objKey, objKeyLen, pData, length, pData);
	}
	return length;
}

void PdfCrypto::RC4(uint8_t * key, uint32_t keyLength, uint8_t * data, uint32_t dataLength, uint8_t * dataRet)
{
	CryptoRC4::Encrypt(key, keyLength, data, dataLength, dataRet);
}

uint32_t PdfCrypto::AESEncrypt(uint8_t * key, uint32_t keyLength, uint8_t * data, uint32_t dataLength, uint8_t * dataRet)
{
	CryptoRijndael aes;
	for (uint32_t i = 0; i < 16; i++)
	{
		dataRet[i] = key[i];
	}
	aes.init(CryptoRijndael::CBC, CryptoRijndael::Encrypt, key, CryptoRijndael::Key16Bytes, dataRet);
	int32_t len = aes.padEncrypt(data, (uint32_t)dataLength, &dataRet[16]);
	if (len < 0)
	{
		return 0;
	}
	else{
		return len;
	}
}

uint32_t PdfCrypto::AESDecrypt(uint8_t * key, uint32_t keyLength, uint8_t * data, uint32_t dataLength, uint8_t * dataRet)
{
	CryptoRijndael aes;
	uint8_t vector[16];
	for (uint32_t i = 0; i < 16; i++)
	{
		vector[i] = data[i];
	}
	aes.init(CryptoRijndael::CBC, CryptoRijndael::Decrypt, key, CryptoRijndael::Key16Bytes, vector);
	int32_t len = aes.padDecrypt(&data[16], (uint32_t)(dataLength - 16), dataRet);
	if (len < 0)
	{
		return 0;
	}
	else{
		return len;
	}
}
    
}//namespace
