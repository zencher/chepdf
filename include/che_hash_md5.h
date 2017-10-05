#ifndef _CHE_HASH_MD5_H_
#define _CHE_HASH_MD5_H_

#include "che_base_define.h"

namespace chepdf {

class HashMD5
{
public:
    HashMD5();

    void Init();
    void Update(const uint8_t * buf, uint32_t len);
    void Final(uint8_t digest[16]);

private:
    void Transform(uint32_t buf[4], uint32_t in[16]);

    uint32_t    buf_[4];
    uint32_t    bits_[2];
    uint8_t		in_[64];
};

}//namespace

#endif
