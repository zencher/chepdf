#include <setjmp.h>

#include "../include/che_pdf_filter.h"

#include "zlib.h"
#include "/usr/local/include/jpeglib.h"
#include "/usr/local/include/jbig2.h"
#include "/usr/local/include/openjpeg-2.3/openjpeg.h"

namespace chepdf {

void PdfHexFilter::Encode(uint8_t * data, size_t size, Buffer & buffer)
{
	if (data == nullptr || size == 0)
	{
		return;
	}
    uint8_t bytes[2];
    while(size--)
    {
        bytes[0]  = (*data & 0xF0) >> 4;
        bytes[0] += (bytes[0] > 9 ? 'A' - 10 : '0');
        bytes[1]  = (*data & 0x0F);
        bytes[1] += (bytes[1] > 9 ? 'A' - 10 : '0');
		buffer.Write(data, 2);
        ++data;
    }
	data[0] = '>';
	buffer.Write(data, 1);
}

void PdfHexFilter::Decode(uint8_t * data, size_t size, Buffer & buffer)
{
	if (data == nullptr || size == 0)
	{
		return;
	}
	bool b_low = false;
    uint8_t byte = 0;
	uint8_t value = 0;
    while(size--)
    {
        byte  = *data;
		if (byte == '>')
		{
			break;
		}
		if (byte >= '0' && byte <= '9')
		{
			value += (byte-'0');
			if (b_low == true)
			{
				buffer.Write( &value, 1 );
				value = 0;
				b_low = false;
			}else{
				value *= 16;
				b_low = true;
			}
		}else if (byte >= 'a' && byte <= 'f')
		{
			value += (byte-'a'+10);
			if (b_low == true)
			{
				buffer.Write(&value, 1);
				value = 0;
				b_low = false;
			}else{
				value *= 16;
				b_low = true;
			}
		}else if (byte >= 'A' && byte <= 'F')
		{
			value += (byte - 'A' + 10);
			if ( b_low == true )
			{
				buffer.Write(&value, 1);
				value = 0;
				b_low = false;
			}else{
				value *= 16;
				b_low = true;
			}
		}
		++data;
    }
}

void PdfASCII85Filter::Encode(uint8_t * data, size_t size, Buffer & buffer)
{
	count_ = 0;
    tuple_ = 0;
	size_t c;
    const char * z = "z";
    while (size)
    {
        c = *data & 0xff;
        switch (count_++) {
		case 0: tuple_ |= ( c << 24); break;
		case 1: tuple_ |= ( c << 16); break;
		case 2: tuple_ |= ( c <<  8); break;
		case 3:
			tuple_ |= c;
			if(0 == tuple_)
			{
				buffer.Write((unsigned char*)z, 1);
			}
			else
			{
				this->EncodeTuple(tuple_, count_, buffer);
			}
			tuple_ = 0;
			count_ = 0;
			break;
        }
        --size;
        ++data;
    }
	if (count_ > 0)
	{
		this->EncodeTuple(tuple_, count_, buffer);
	}
	buffer.Write((unsigned char*)("~>"), 2);
}

void PdfASCII85Filter::Decode(uint8_t * data, size_t size, Buffer & buffer)
{
	count_ = 0;
    tuple_ = 0;
    bool foundEndMarker = false;
	const size_t sPowers85[] = { 85*85*85*85, 85*85*85, 85*85, 85, 1 };
    while(size && !foundEndMarker)
    {
        switch (*data)
        {
		default:
			if (*data < '!' || *data > 'u')
			{
				//error
				return;
			}
			tuple_ += ( *data - '!') * sPowers85[count_++];
			if( count_ == 5)
			{
				WidePut( tuple_, count_-1, buffer);
				count_ = 0;
				tuple_ = 0;
			}
			break;
		case 'z':
			if (count_ != 0)
			{
				//error
				return;
			}
			this->WidePut(0, 4, buffer);
			break;
		case '~':
			++data;
			--size;
			if(size && *data != '>')
			{
				//error
				return;
			}
			foundEndMarker = true;
			break;
		case '\n': case '\r': case '\t': case ' ':
		case '\0': case '\f': case '\b': case 0177:
			break;
        }
        --size;
        ++data;
	}
	if(count_ > 1)
	{
		WidePut(tuple_, count_-1, buffer);
		count_ = 0;
		tuple_ = 0;
	}
}

void PdfASCII85Filter::EncodeTuple(size_t tuple, size_t count, Buffer & buffer)
{
	size_t i = 5;
	size_t z = 0;
	char buf[5];
	char out[5];
	char* start = buf;
	
	do 
	{
		*start++ = (char)(tuple % 85);
		tuple /= 85;
	} 
	while (--i > 0);
	
	i = count;
	do 
	{
		out[z++] = (char)(*--start) + '!';
	} 
	while( i-- > 0 );

	buffer.Write((unsigned char*)out, z);
}
	
void PdfASCII85Filter::WidePut(size_t tuple, size_t bytes, Buffer & buffer)
{
    char data[4];

	data[0] = static_cast<char>(tuple >> 24);
	data[1] = static_cast<char>(tuple >> 16);
	data[2] = static_cast<char>(tuple >>  8);
	data[3] = static_cast<char>(tuple);

	if (bytes != 4)
	{
		switch (bytes)
		{
		case 3:
			if (data[3] != 0)
			{
				data[2]++;
			}
			break;
		case 2:
			if (data[2] != 0 || data[3] != 0)
			{
				data[1]++;
			}
			break;
		case 1:
			if (data[1] != 0 || data[2] != 0 || data[3] != 0)
			{
				data[0]++;
			}
			break;
		default:
			break;
		}
	}

	buffer.Write((unsigned char*)data, bytes);
}

void PdfFlateFilter::Encode(uint8_t * data, size_t size, Buffer & buffer)
{
	if (data == nullptr || size == 0)
	{
		return;
	}

	uint8_t tmpBuffer[4096];
	size_t nWrittenData = 0;
	z_stream stream;
	int32_t param = Z_NO_FLUSH;

	stream.zalloc = nullptr;
	stream.zfree = nullptr;
    stream.avail_in = (int32_t)size;
    stream.next_in  = data;
	deflateInit(&stream, Z_DEFAULT_COMPRESSION);
	
    while (true)
	{
        stream.avail_out = 4096;
        stream.next_out  = tmpBuffer;
		switch (deflate(&stream, param))
		{
		case Z_OK:
			if (stream.avail_out != 0)
			{
				param = Z_FINISH;
			}
			break;
		case Z_BUF_ERROR:
		case Z_STREAM_END:
			nWrittenData = 4096 - stream.avail_out;
			buffer.Write(tmpBuffer, nWrittenData);
			deflateEnd(&stream);
			return;
			break;
		default:
			deflateEnd(&stream);
			return;
			break;
		}
        nWrittenData = 4096 - stream.avail_out;
		buffer.Write(tmpBuffer, nWrittenData);
    }
}

void PdfFlateFilter::Decode(uint8_t * data, size_t size, Buffer & buffer)
{
	if (data == nullptr || size == 0)
	{
		return;
	}
	uint8_t tmpBuffer[4096];
	size_t nWrittenData = 0;
	z_stream stream;
	int32_t param = Z_NO_FLUSH;

	stream.zalloc = nullptr;
	stream.zfree = nullptr;
	stream.avail_in = (int32_t)size;
	stream.next_in  = data;
	inflateInit(&stream);

	while(true)
	{
		stream.avail_out = 4096;
		stream.next_out  = tmpBuffer;
        int i = inflate(&stream, param);
		switch (i)
		{
		case Z_OK:
			if (stream.avail_out != 0)
			{
				param = Z_FINISH;
			}
			break;
		case Z_STREAM_END:
			nWrittenData = 4096 - stream.avail_out;
			buffer.Write(tmpBuffer, nWrittenData);
			inflateEnd(&stream);
			return;
			break;
		//case Z_BUF_ERROR:
		//	break;
		default:
			inflateEnd(&stream);
			return;
		}   
		nWrittenData = 4096 - stream.avail_out;
		buffer.Write(tmpBuffer, nWrittenData);
	}
}

const unsigned short PdfLZWFilter::s_masks_[] = {	0x01FF,
                                                    0x03FF,
                                                    0x07FF,
                                                    0x0FFF };

const unsigned short PdfLZWFilter::s_clear_  = 0x0100;      // clear table
const unsigned short PdfLZWFilter::s_eod_    = 0x0101;      // end of data

void PdfLZWFilter::Encode(uint8_t * data, size_t size, Buffer & buffer)
{
	return;
}

void PdfLZWFilter::Decode(uint8_t * data, size_t size, Buffer & out_buffer)
{
	mask_ = 0;
    code_len_ = 9;
    character_ = 0;
    b_first_ = true;
	uint32_t buffer_size = 0;
    uint32_t buffer_max = 24;
    size_t old = 0;
    size_t code = 0;
    size_t buffer = 0;
    TLzwItem item;
    std::vector<uint8_t> bytes;
    if (b_first_)
    {
        character_ = *data;
        b_first_ = false;
    }
    while (size)
    {
        // Fill the buffer
        while (buffer_size <= (buffer_max - 8) && size)
        {
            buffer <<= 8;
            buffer |= static_cast<size_t>(static_cast<uint8_t>(*data));
            buffer_size += 8;
            ++data;
            size--;
        }
        // read from the buffer
        while (buffer_size >= code_len_)
        {
            code = (buffer >> (buffer_size - code_len_)) & PdfLZWFilter::s_masks_[mask_];
            buffer_size -= code_len_;
            if (code == PdfLZWFilter::s_clear_)
            {
                mask_ = 0;
                code_len_ = 9;
                InitTable();
            }
            else if(code == PdfLZWFilter::s_eod_)
            {
                size = 0;
                break;
            }
            else 
            {
                if(code >= table_.size())
                {
//                     if (old >= table_.size())
//                     {
//                          error, out of range
//                     }
                    bytes = table_[old].value;
                    bytes.push_back(character_);
                }else{
                    bytes = table_[code].value;
                }
                // Write data to the output device
				out_buffer.Write( (uint8_t *)(&(bytes[0])), bytes.size() );
				
                character_ = bytes[0];
                if(old < table_.size()) // fix the first loop
                    bytes = table_[old].value;
                bytes.push_back(character_);
				
                item.value = bytes;
                table_.push_back(item);
                
                old = code;

                switch(table_.size())
                {
				case 511:
				case 1023:
				case 2047:
					++code_len_;
					++mask_;
				default:
					break;
                }
            }
        }
    }
	return;
}

void PdfLZWFilter::InitTable()
{
	int      i;
	TLzwItem item;
	
	table_.clear();
	table_.reserve( 4096 );
	
	for( i=0;i<=255;i++ )
	{
		item.value.clear();
		item.value.push_back( static_cast<unsigned char>(i) );
		table_.push_back( item );
	}
	
	// Add dummy entry, which is never used by decoder
	item.value.clear();
	table_.push_back( item );
}

void PdfRLEFileter::Encode( uint8_t * data, size_t size, Buffer & buffer )
{
	if ( data == nullptr || size == 0 )
	{
		return;
	}

	Buffer buf( size, size, GetAllocator() );
	uint8_t byte = *data;
	data++;
	size_t lCount = 1;
	uint8_t countByte = 0;
	while ( size > 1 )
	{
		if ( byte == *data )
		{
			if ( buf.GetSize() > 1 )
			{
				countByte = (uint8_t)(buf.GetSize() - 1);
				buffer.Write( &countByte, 1 );
				buffer.Write( buf );
				buf.Clear();
			}
			lCount++;
			if ( lCount == 128 )
			{
				countByte = (uint8_t)(lCount + 127);
				buffer.Write( &countByte, 1 );
				buffer.Write( &byte, 1 );
				lCount = 1;
				byte = *(++data);
			}
		}else if ( byte != *data )
		{
			if ( lCount >= 2 )
			{
				countByte = (uint8_t)(lCount + 127);
				buffer.Write( &countByte, 1 );
				buffer.Write( &byte, 1 );
				lCount = 1;
				byte = *data;
			}else{
				buf.Write( &byte, 1 );
				byte = *data;
				if ( buf.GetSize() == 128 )
				{
					countByte = (uint8_t)(buf.GetSize() - 1);
					buffer.Write( &countByte, 1 );
					buffer.Write( buf );
					buf.Clear();
				}
			}
		}
		size--;
		data++;
	}

	if ( buf.GetSize() > 0 )
	{
		countByte = (uint8_t)(buf.GetSize() - 1);
		buffer.Write( &countByte, 1 );
		buffer.Write( buf );
		buf.Clear();
	}
	if ( lCount == 1 )
	{
		countByte = (uint8_t)(lCount - 1);
		buffer.Write( &countByte, 1 );
		buffer.Write( &byte, 1 );
	}else if ( lCount > 1 )
	{
		countByte = (uint8_t)(lCount + 127);
		buffer.Write( &countByte, 1 );
		buffer.Write( &byte, 1 );
	}
	countByte = 128;
	buffer.Write( &countByte, 1 );
}

void PdfRLEFileter::Decode( uint8_t * data, size_t size, Buffer & buffer )
{
	if ( data == nullptr || size == 0 )
	{
		return;
	}

	uint8_t	byteLen = 0;
	while ( size > 0 )
	{
		byteLen = *data;
		data++;
		size--;
		if ( byteLen == 128 )
		{
			return;
		}else if ( byteLen < 128 )
		{
			buffer.Write( data, byteLen+1 );
			size-= byteLen+1;
			data+= byteLen+1;
		}else if ( byteLen > 128 )
		{
			for ( uint8_t i = 0; i < byteLen - 127; i++ )
			{
				buffer.Write( data, 1 );
			}
			data++;
			size--;
		}
	}
}


PdfFaxDecodeParams::PdfFaxDecodeParams( PdfDictionaryPointer dictionary )
	: k(0), columns(1728), rows(0), eol(false), eba(false), eob(true), bi1(false)
{
	if ( dictionary )
	{
		PdfObjectPointer object;
		object = dictionary->GetElement( "K", OBJ_TYPE_NUMBER );
		if ( object )
		{
			k = object->GetPdfNumber()->GetInteger();
		}
		object = dictionary->GetElement( "EndOfLine", OBJ_TYPE_BOOLEAN );
		if ( object )
		{
			eol = object->GetPdfBoolean()->GetValue();
		}
		object = dictionary->GetElement( "EncodedByteAlign", OBJ_TYPE_BOOLEAN );
		if ( object )
		{
			eba = object->GetPdfBoolean()->GetValue();
		}
		object = dictionary->GetElement( "EndOfBlock", OBJ_TYPE_BOOLEAN );
		if ( object )
		{
			eob = object->GetPdfBoolean()->GetValue();
		}
		object = dictionary->GetElement( "BlackIs1", OBJ_TYPE_BOOLEAN );
		if ( object )
		{
			bi1 = object->GetPdfBoolean()->GetValue();
		}
		object = dictionary->GetElement( "Columns", OBJ_TYPE_NUMBER );
		if ( object )
		{
			columns = object->GetPdfNumber()->GetInteger();
		}
		object = dictionary->GetElement( "Rows", OBJ_TYPE_NUMBER );
		if ( object )
		{
			rows = object->GetPdfNumber()->GetInteger();
		}
	}
}



typedef struct cfd_node_s cfd_node;

struct cfd_node_s
{
	short value;
	short nbits;
};


#define 	cfd_white_initial_bits			8
#define		cfd_black_initial_bits			7
#define		cfd_2d_initial_bits				7
#define		cfd_uncompressed_initial_bits	6

#define 	FAXERROR			-1
#define		ZEROS			-2
#define		UNCOMPRESSED	-3

/* semantic codes for cf_2d_decode */

#define	P -4
#define	H -5
#define	VR3 0
#define	VR2 1
#define	VR1 2
#define	V0 3
#define	VL1 4
#define	VL2 5
#define	VL3 6

/* White decoding table. */
static const cfd_node cf_white_decode[] = {
	{256,12},{272,12},{29,8},{30,8},{45,8},{46,8},{22,7},{22,7},
	{23,7},{23,7},{47,8},{48,8},{13,6},{13,6},{13,6},{13,6},{20,7},
	{20,7},{33,8},{34,8},{35,8},{36,8},{37,8},{38,8},{19,7},{19,7},
	{31,8},{32,8},{1,6},{1,6},{1,6},{1,6},{12,6},{12,6},{12,6},{12,6},
	{53,8},{54,8},{26,7},{26,7},{39,8},{40,8},{41,8},{42,8},{43,8},
	{44,8},{21,7},{21,7},{28,7},{28,7},{61,8},{62,8},{63,8},{0,8},
	{320,8},{384,8},{10,5},{10,5},{10,5},{10,5},{10,5},{10,5},{10,5},
	{10,5},{11,5},{11,5},{11,5},{11,5},{11,5},{11,5},{11,5},{11,5},
	{27,7},{27,7},{59,8},{60,8},{288,9},{290,9},{18,7},{18,7},{24,7},
	{24,7},{49,8},{50,8},{51,8},{52,8},{25,7},{25,7},{55,8},{56,8},
	{57,8},{58,8},{192,6},{192,6},{192,6},{192,6},{1664,6},{1664,6},
	{1664,6},{1664,6},{448,8},{512,8},{292,9},{640,8},{576,8},{294,9},
	{296,9},{298,9},{300,9},{302,9},{256,7},{256,7},{2,4},{2,4},{2,4},
	{2,4},{2,4},{2,4},{2,4},{2,4},{2,4},{2,4},{2,4},{2,4},{2,4},{2,4},
	{2,4},{2,4},{3,4},{3,4},{3,4},{3,4},{3,4},{3,4},{3,4},{3,4},{3,4},
	{3,4},{3,4},{3,4},{3,4},{3,4},{3,4},{3,4},{128,5},{128,5},{128,5},
	{128,5},{128,5},{128,5},{128,5},{128,5},{8,5},{8,5},{8,5},{8,5},
	{8,5},{8,5},{8,5},{8,5},{9,5},{9,5},{9,5},{9,5},{9,5},{9,5},{9,5},
	{9,5},{16,6},{16,6},{16,6},{16,6},{17,6},{17,6},{17,6},{17,6},
	{4,4},{4,4},{4,4},{4,4},{4,4},{4,4},{4,4},{4,4},{4,4},{4,4},{4,4},
	{4,4},{4,4},{4,4},{4,4},{4,4},{5,4},{5,4},{5,4},{5,4},{5,4},{5,4},
	{5,4},{5,4},{5,4},{5,4},{5,4},{5,4},{5,4},{5,4},{5,4},{5,4},
	{14,6},{14,6},{14,6},{14,6},{15,6},{15,6},{15,6},{15,6},{64,5},
	{64,5},{64,5},{64,5},{64,5},{64,5},{64,5},{64,5},{6,4},{6,4},
	{6,4},{6,4},{6,4},{6,4},{6,4},{6,4},{6,4},{6,4},{6,4},{6,4},{6,4},
	{6,4},{6,4},{6,4},{7,4},{7,4},{7,4},{7,4},{7,4},{7,4},{7,4},{7,4},
	{7,4},{7,4},{7,4},{7,4},{7,4},{7,4},{7,4},{7,4},{-2,3},{-2,3},
	{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},
	{-1,0},{-1,0},{-1,0},{-1,0},{-3,4},{1792,3},{1792,3},{1984,4},
	{2048,4},{2112,4},{2176,4},{2240,4},{2304,4},{1856,3},{1856,3},
	{1920,3},{1920,3},{2368,4},{2432,4},{2496,4},{2560,4},{1472,1},
	{1536,1},{1600,1},{1728,1},{704,1},{768,1},{832,1},{896,1},
	{960,1},{1024,1},{1088,1},{1152,1},{1216,1},{1280,1},{1344,1},
	{1408,1}
};

/* Black decoding table. */
static const cfd_node cf_black_decode[] = {
	{128,12},{160,13},{224,12},{256,12},{10,7},{11,7},{288,12},{12,7},
	{9,6},{9,6},{8,6},{8,6},{7,5},{7,5},{7,5},{7,5},{6,4},{6,4},{6,4},
	{6,4},{6,4},{6,4},{6,4},{6,4},{5,4},{5,4},{5,4},{5,4},{5,4},{5,4},
	{5,4},{5,4},{1,3},{1,3},{1,3},{1,3},{1,3},{1,3},{1,3},{1,3},{1,3},
	{1,3},{1,3},{1,3},{1,3},{1,3},{1,3},{1,3},{4,3},{4,3},{4,3},{4,3},
	{4,3},{4,3},{4,3},{4,3},{4,3},{4,3},{4,3},{4,3},{4,3},{4,3},{4,3},
	{4,3},{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},
	{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},
	{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},{3,2},
	{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},
	{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},
	{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},
	{-2,4},{-2,4},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},
	{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-3,5},{1792,4},
	{1792,4},{1984,5},{2048,5},{2112,5},{2176,5},{2240,5},{2304,5},
	{1856,4},{1856,4},{1920,4},{1920,4},{2368,5},{2432,5},{2496,5},
	{2560,5},{18,3},{18,3},{18,3},{18,3},{18,3},{18,3},{18,3},{18,3},
	{52,5},{52,5},{640,6},{704,6},{768,6},{832,6},{55,5},{55,5},
	{56,5},{56,5},{1280,6},{1344,6},{1408,6},{1472,6},{59,5},{59,5},
	{60,5},{60,5},{1536,6},{1600,6},{24,4},{24,4},{24,4},{24,4},
	{25,4},{25,4},{25,4},{25,4},{1664,6},{1728,6},{320,5},{320,5},
	{384,5},{384,5},{448,5},{448,5},{512,6},{576,6},{53,5},{53,5},
	{54,5},{54,5},{896,6},{960,6},{1024,6},{1088,6},{1152,6},{1216,6},
	{64,3},{64,3},{64,3},{64,3},{64,3},{64,3},{64,3},{64,3},{13,1},
	{13,1},{13,1},{13,1},{13,1},{13,1},{13,1},{13,1},{13,1},{13,1},
	{13,1},{13,1},{13,1},{13,1},{13,1},{13,1},{23,4},{23,4},{50,5},
	{51,5},{44,5},{45,5},{46,5},{47,5},{57,5},{58,5},{61,5},{256,5},
	{16,3},{16,3},{16,3},{16,3},{17,3},{17,3},{17,3},{17,3},{48,5},
	{49,5},{62,5},{63,5},{30,5},{31,5},{32,5},{33,5},{40,5},{41,5},
	{22,4},{22,4},{14,1},{14,1},{14,1},{14,1},{14,1},{14,1},{14,1},
	{14,1},{14,1},{14,1},{14,1},{14,1},{14,1},{14,1},{14,1},{14,1},
	{15,2},{15,2},{15,2},{15,2},{15,2},{15,2},{15,2},{15,2},{128,5},
	{192,5},{26,5},{27,5},{28,5},{29,5},{19,4},{19,4},{20,4},{20,4},
	{34,5},{35,5},{36,5},{37,5},{38,5},{39,5},{21,4},{21,4},{42,5},
	{43,5},{0,3},{0,3},{0,3},{0,3}
};

/* 2-D decoding table. */
static const cfd_node cf_2d_decode[] = {
	{128,11},{144,10},{6,7},{0,7},{5,6},{5,6},{1,6},{1,6},{-4,4},
	{-4,4},{-4,4},{-4,4},{-4,4},{-4,4},{-4,4},{-4,4},{-5,3},{-5,3},
	{-5,3},{-5,3},{-5,3},{-5,3},{-5,3},{-5,3},{-5,3},{-5,3},{-5,3},
	{-5,3},{-5,3},{-5,3},{-5,3},{-5,3},{4,3},{4,3},{4,3},{4,3},{4,3},
	{4,3},{4,3},{4,3},{4,3},{4,3},{4,3},{4,3},{4,3},{4,3},{4,3},{4,3},
	{2,3},{2,3},{2,3},{2,3},{2,3},{2,3},{2,3},{2,3},{2,3},{2,3},{2,3},
	{2,3},{2,3},{2,3},{2,3},{2,3},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},
	{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},
	{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},
	{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},
	{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},
	{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},{3,1},
	{3,1},{3,1},{3,1},{-2,4},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},
	{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},
	{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-1,0},{-3,3}
};

/* Uncompressed decoding table. */
/*static const cfd_node cf_uncompressed_decode[] = {
	{64,12},{5,6},{4,5},{4,5},{3,4},{3,4},{3,4},{3,4},{2,3},{2,3},
	{2,3},{2,3},{2,3},{2,3},{2,3},{2,3},{1,2},{1,2},{1,2},{1,2},{1,2},
	{1,2},{1,2},{1,2},{1,2},{1,2},{1,2},{1,2},{1,2},{1,2},{1,2},{1,2},
	{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},
	{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},
	{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},{0,1},
	{-1,0},{-1,0},{8,6},{9,6},{6,5},{6,5},{7,5},{7,5},{4,4},{4,4},
	{4,4},{4,4},{5,4},{5,4},{5,4},{5,4},{2,3},{2,3},{2,3},{2,3},{2,3},
	{2,3},{2,3},{2,3},{3,3},{3,3},{3,3},{3,3},{3,3},{3,3},{3,3},{3,3},
	{0,2},{0,2},{0,2},{0,2},{0,2},{0,2},{0,2},{0,2},{0,2},{0,2},{0,2},
	{0,2},{0,2},{0,2},{0,2},{0,2},{1,2},{1,2},{1,2},{1,2},{1,2},{1,2},
	{1,2},{1,2},{1,2},{1,2},{1,2},{1,2},{1,2},{1,2},{1,2},{1,2}
};*/

/* bit magic */

static inline int getbit(const unsigned char *buf, int x)
{
	return ( buf[x >> 3] >> ( 7 - (x & 7) ) ) & 1;
}

static int
	find_changing(const unsigned char *line, int x, int w)
{
	int a, b;

	if (!line)
		return w;

	if (x == -1)
	{
		a = 0;
		x = 0;
	}
	else
	{
		a = getbit(line, x);
		x++;
	}

	while (x < w)
	{
		b = getbit(line, x);
		if (a != b)
			break;
		x++;
	}

	return x;
}

static int
	find_changing_color(const unsigned char *line, int x, int w, int color)
{
	if (!line)
		return w;

	x = find_changing(line, (x > 0 || !color) ? x : -1, w);

	if (x < w && getbit(line, x) != color)
		x = find_changing(line, x, w);

	return x;
}

static const unsigned char lm[8] = {
	0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01
};

static const unsigned char rm[8] = {
	0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE
};

static inline void setbits(unsigned char *line, int x0, int x1)
{
	int a0, a1, b0, b1, a;

	a0 = x0 >> 3;
	a1 = x1 >> 3;

	b0 = x0 & 7;
	b1 = x1 & 7;

	if (a0 == a1)
	{
		if (b1)
			line[a0] |= lm[b0] & rm[b1];
	}
	else
	{
		line[a0] |= lm[b0];
		for (a = a0 + 1; a < a1; a++)
			line[a] = 0xFF;
		if (b1)
			line[a1] |= rm[b1];
	}
}

typedef struct fz_faxd_s fz_faxd;

enum
{
	STATE_NORMAL,	/* neutral state, waiting for any code */
	STATE_MAKEUP,	/* got a 1d makeup code, waiting for terminating code */
	STATE_EOL,		/* at eol, needs output buffer space */
	STATE_H1, STATE_H2,	/* in H part 1 and 2 (both makeup and terminating codes) */
	STATE_DONE		/* all done */
};

struct fz_faxd_s
{
// 	fz_context *ctx;
// 	fz_stream *chain;

	uint8_t * data;
	size_t index;
	size_t size;

	int k;
	int end_of_line;
	int encoded_byte_align;
	int columns;
	int rows;
	int end_of_block;
	int black_is_1;

	int stride;
	int ridx;

	int bidx;
	uint32_t word;

	int stage;

	int a, c, dim, eolc;
	unsigned char *ref;
	unsigned char *dst;
	unsigned char *rp, *wp;
};

static inline void eat_bits(fz_faxd *fax, int nbits)
{
	fax->word <<= nbits;
	fax->bidx += nbits;
}

static int fill_bits(fz_faxd *fax)
{
	while (fax->bidx >= 8)
	{
		int c = *(fax->data + fax->index); //;fz_read_byte(fax->chain);
		fax->index++;
		if (c == EOF)
			return EOF;
		fax->bidx -= 8;
		fax->word |= c << fax->bidx;
	}
	return 0;
}

static int get_code(fz_faxd *fax, const cfd_node *table, int initialbits)
{
	uint32_t word = fax->word;
	int tidx = word >> (32 - initialbits);
	int value = table[tidx].value;
	int nbits = table[tidx].nbits;

	if (nbits > initialbits)
	{
		int mask = (1 << (32 - initialbits)) - 1;
		tidx = value + ((word & mask) >> (32 - nbits));
		value = table[tidx].value;
		nbits = initialbits + table[tidx].nbits;
	}

	eat_bits(fax, nbits);

	return value;
}

/* decode one 1d code */
static void dec1d(/*fz_context *ctx,*/ fz_faxd *fax)
{
	int code;

	if (fax->a == -1)
		fax->a = 0;

	if (fax->c)
		code = get_code(fax, cf_black_decode, cfd_black_initial_bits);
	else
		code = get_code(fax, cf_white_decode, cfd_white_initial_bits);

// 	if (code == UNCOMPRESSED)
// 		fz_throw(ctx, "uncompressed data in faxd");
// 
// 	if (code < 0)
// 		fz_throw(ctx, "negative code in 1d faxd");
// 
// 	if (fax->a + code > fax->columns)
// 		fz_throw(ctx, "overflow in 1d faxd");

	if (fax->c)
		setbits(fax->dst, fax->a, fax->a + code);

	fax->a += code;

	if (code < 64)
	{
		fax->c = !fax->c;
		fax->stage = STATE_NORMAL;
	}
	else
		fax->stage = STATE_MAKEUP;
}

/* decode one 2d code */
static void dec2d(/*fz_context *ctx, */fz_faxd *fax)
{
	int code, b1, b2;

	if (fax->stage == STATE_H1 || fax->stage == STATE_H2)
	{
		if (fax->a == -1)
			fax->a = 0;

		if (fax->c)
			code = get_code(fax, cf_black_decode, cfd_black_initial_bits);
		else
			code = get_code(fax, cf_white_decode, cfd_white_initial_bits);

// 		if (code == UNCOMPRESSED)
// 			fz_throw(ctx, "uncompressed data in faxd");
// 
// 		if (code < 0)
// 			fz_throw(ctx, "negative code in 2d faxd");
// 
// 		if (fax->a + code > fax->columns)
// 			fz_throw(ctx, "overflow in 2d faxd");

		if (fax->c)
			setbits(fax->dst, fax->a, fax->a + code);

		fax->a += code;

		if (code < 64)
		{
			fax->c = !fax->c;
			if (fax->stage == STATE_H1)
				fax->stage = STATE_H2;
			else if (fax->stage == STATE_H2)
				fax->stage = STATE_NORMAL;
		}

		return;
	}

	code = get_code(fax, cf_2d_decode, cfd_2d_initial_bits);

	switch (code)
	{
	case H:
		fax->stage = STATE_H1;
		break;

	case P:
		b1 = find_changing_color(fax->ref, fax->a, fax->columns, !fax->c);
		if (b1 >= fax->columns)
			b2 = fax->columns;
		else
			b2 = find_changing(fax->ref, b1, fax->columns);
		if (fax->c) setbits(fax->dst, fax->a, b2);
		fax->a = b2;
		break;

	case V0:
		b1 = find_changing_color(fax->ref, fax->a, fax->columns, !fax->c);
		if (fax->c) setbits(fax->dst, fax->a, b1);
		fax->a = b1;
		fax->c = !fax->c;
		break;

	case VR1:
		b1 = 1 + find_changing_color(fax->ref, fax->a, fax->columns, !fax->c);
		if (b1 >= fax->columns) b1 = fax->columns;
		if (fax->c) setbits(fax->dst, fax->a, b1);
		fax->a = b1;
		fax->c = !fax->c;
		break;

	case VR2:
		b1 = 2 + find_changing_color(fax->ref, fax->a, fax->columns, !fax->c);
		if (b1 >= fax->columns) b1 = fax->columns;
		if (fax->c) setbits(fax->dst, fax->a, b1);
		fax->a = b1;
		fax->c = !fax->c;
		break;

	case VR3:
		b1 = 3 + find_changing_color(fax->ref, fax->a, fax->columns, !fax->c);
		if (b1 >= fax->columns) b1 = fax->columns;
		if (fax->c) setbits(fax->dst, fax->a, b1);
		fax->a = b1;
		fax->c = !fax->c;
		break;

	case VL1:
		b1 = -1 + find_changing_color(fax->ref, fax->a, fax->columns, !fax->c);
		if (b1 < 0) b1 = 0;
		if (fax->c) setbits(fax->dst, fax->a, b1);
		fax->a = b1;
		fax->c = !fax->c;
		break;

	case VL2:
		b1 = -2 + find_changing_color(fax->ref, fax->a, fax->columns, !fax->c);
		if (b1 < 0) b1 = 0;
		if (fax->c) setbits(fax->dst, fax->a, b1);
		fax->a = b1;
		fax->c = !fax->c;
		break;

	case VL3:
		b1 = -3 + find_changing_color(fax->ref, fax->a, fax->columns, !fax->c);
		if (b1 < 0) b1 = 0;
		if (fax->c) setbits(fax->dst, fax->a, b1);
		fax->a = b1;
		fax->c = !fax->c;
		break;

// 	case UNCOMPRESSED:
// 		fz_throw(ctx, "uncompressed data in faxd");
// 
// 	case ERROR:
// 		fz_throw(ctx, "invalid code in 2d faxd");
// 
// 	default:
// 		fz_throw(ctx, "invalid code in 2d faxd (%d)", code);
	}
}


PdfFaxFilter::PdfFaxFilter( PdfFaxDecodeParams * params, Allocator * allocator /*= nullptr*/ )
    : PdfFilter(allocator), params_(params)
{
}

PdfFaxFilter::~PdfFaxFilter()
{
}

void PdfFaxFilter::Encode( uint8_t * data, size_t size, Buffer & buffer )
{

}

void PdfFaxFilter::Decode( uint8_t * data, size_t size, Buffer & buffer )
{
	if ( data == nullptr || size == 0 )
	{
		return;
	}

	fz_faxd faxs;
	if ( params_ )
	{
		faxs.ref = nullptr;
		faxs.dst = nullptr;

		faxs.data = data;
		faxs.index = 0;
		faxs.size = size;

		faxs.k = params_->k;
		faxs.end_of_line = params_->eol;
		faxs.encoded_byte_align = params_->eba;
		faxs.columns = params_->columns;
		faxs.rows = params_->rows;
		faxs.end_of_block = params_->eob;
		faxs.black_is_1 = params_->bi1;

		faxs.stride = ((faxs.columns - 1) >> 3) + 1;
		faxs.ridx = 0;
		faxs.bidx = 32;
		faxs.word = 0;

		faxs.stage = STATE_NORMAL;
		faxs.a = -1;
		faxs.c = 0;
		faxs.dim = faxs.k < 0 ? 2 : 1;
		faxs.eolc = 0;

		faxs.ref = GetAllocator()->NewArray<uint8_t>( faxs.stride );
		faxs.dst = GetAllocator()->NewArray<uint8_t>( faxs.stride );
		faxs.rp = faxs.dst;
		faxs.wp = faxs.dst + faxs.stride;

		memset(faxs.ref, 0, faxs.stride);
		memset(faxs.dst, 0, faxs.stride);
	}
	
	fz_faxd * fax = &faxs;

	unsigned char * tmp = nullptr;

	if (fax->stage == STATE_DONE)
	{
		if ( !fax->black_is_1 )
		{
			uint8_t * pByte = buffer.GetData();
			for ( size_t i = 0; i < buffer.GetSize(); ++i )
			{
				*pByte = *pByte ^ 0xFF;
				++pByte;
			}
		}
		return;
	}

	if (fax->stage == STATE_EOL)
		goto eol;

loop:

	if (fill_bits(fax))
	{
		if (fax->bidx > 31)
		{
			if (fax->a > 0)
				goto eol;
			goto rtc;
		}
	}

	if ((fax->word >> (32 - 12)) == 0)
	{
		eat_bits(fax, 1);
		goto loop;
	}

	if ((fax->word >> (32 - 12)) == 1)
	{
		eat_bits(fax, 12);
		fax->eolc ++;

		if (fax->k > 0)
		{
			if (fax->a == -1)
				fax->a = 0;
			if ((fax->word >> (32 - 1)) == 1)
				fax->dim = 1;
			else
				fax->dim = 2;
			eat_bits(fax, 1);
		}
	}
	else if (fax->k > 0 && fax->a == -1)
	{
		fax->a = 0;
		if ((fax->word >> (32 - 1)) == 1)
			fax->dim = 1;
		else
			fax->dim = 2;
		eat_bits(fax, 1);
	}
	else if (fax->dim == 1)
	{
		fax->eolc = 0;
		dec1d(/*stm->ctx, */fax);
	}
	else if (fax->dim == 2)
	{
		fax->eolc = 0;
		dec2d(/*stm->ctx, */fax);
	}

	/* no eol check after makeup codes nor in the middle of an H code */
	if (fax->stage == STATE_MAKEUP || fax->stage == STATE_H1 || fax->stage == STATE_H2)
		goto loop;

	/* check for eol conditions */
	if (fax->eolc || fax->a >= fax->columns)
	{
		if (fax->a > 0)
			goto eol;
		if (fax->eolc == (fax->k < 0 ? 2 : 6))
			goto rtc;
	}

	goto loop;

eol:
	fax->stage = STATE_EOL;

	buffer.Write( fax->rp, fax->wp - fax->rp );
	tmp = fax->ref;
	fax->ref = fax->dst;
	fax->dst = tmp;
	memset(fax->dst, 0, fax->stride);

	fax->rp = fax->dst;
	fax->wp = fax->dst + fax->stride;

	fax->stage = STATE_NORMAL;
	fax->c = 0;
	fax->a = -1;
	fax->ridx ++;

	if (!fax->end_of_block && fax->rows)
	{
		if (fax->ridx >= fax->rows)
			goto rtc;
	}

	/* we have not read dim from eol, make a guess */
	if (fax->k > 0 && !fax->eolc && fax->a == -1)
	{
		if (fax->ridx % fax->k == 0)
			fax->dim = 1;
		else
			fax->dim = 2;
	}

	/* if end_of_line & encoded_byte_align, EOLs are *not* optional */
	if (fax->encoded_byte_align)
	{
		if (fax->end_of_line)
			eat_bits(fax, (12 - fax->bidx) & 7);
		else
			eat_bits(fax, (8 - fax->bidx) & 7);
	}

	/* no more space in output, don't decode the next row yet */
// 	if (p == buf + len)
// 		return p - buf;

	goto loop;

rtc:
	fax->stage = STATE_DONE;
	/*return p - buf;*/

	if ( !fax->black_is_1 )
	{
		uint8_t * pByte = buffer.GetData();
		for ( size_t i = 0; i < buffer.GetSize(); ++i )
		{
			*pByte = *pByte ^ 0xFF;
			++pByte;
		}
	}
}



struct jpeg_error_mgr_jmp
{
	struct jpeg_error_mgr super;
	jmp_buf env;
	char msg[JMSG_LENGTH_MAX];
};

static void error_exit(j_common_ptr cinfo)
{
	struct jpeg_error_mgr_jmp *err = (struct jpeg_error_mgr_jmp *)cinfo->err;
	cinfo->err->format_message(cinfo, err->msg);
	longjmp(err->env, 1);
}

static void init_source(j_decompress_ptr cinfo)
{
	/* nothing to do */
}

static void term_source(j_decompress_ptr cinfo)
{
	/* nothing to do */
}

static boolean fill_input_buffer(j_decompress_ptr cinfo)
{
	static unsigned char eoi[2] = { 0xFF, JPEG_EOI };
	struct jpeg_source_mgr *src = cinfo->src;
	src->next_input_byte = eoi;
	src->bytes_in_buffer = 2;
	return 1;
}

static void skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
	struct jpeg_source_mgr *src = cinfo->src;
	if (num_bytes > 0)
	{
		src->next_input_byte += num_bytes;
		src->bytes_in_buffer -= num_bytes;
	}
}


void PdfDCTDFilter::Decode( uint8_t * data, size_t size, Buffer &buffer )
{
    jpeg_decompress_struct cinfo;
	jpeg_error_mgr_jmp err;
	struct jpeg_source_mgr src;
	uint8_t * p = nullptr;
	uint8_t * row[1];
    
	if (setjmp(err.env))
	{
        // 		if (image)
        // 			fz_drop_pixmap(ctx, image);
        // 		fz_throw(ctx, "jpeg error: %s", err.msg);
	}
    
 	cinfo.err = jpeg_std_error(&err.super);
 	err.super.error_exit = error_exit;
    
	jpeg_create_decompress(&cinfo);
    
	cinfo.src = &src;
	src.init_source = init_source;
	src.fill_input_buffer = fill_input_buffer;
	src.skip_input_data = skip_input_data;
	src.resync_to_restart = jpeg_resync_to_restart;
	src.term_source = term_source;
	src.next_input_byte = data;
	src.bytes_in_buffer = size;
    
	jpeg_read_header(&cinfo, 1);
	jpeg_start_decompress(&cinfo);

	buffer.Clear();
    
	p = GetAllocator()->NewArray<uint8_t>( cinfo.output_components * cinfo.output_width );
	row[0] = p;
	while (cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines(&cinfo, row, 1);
        buffer.Write( p, cinfo.output_width * cinfo.output_components );
	}
	GetAllocator()->DeleteArray( p );
    
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
}



/*static void fz_opj_error_callback(const char *msg, void *client_data)
{
	//fz_context *ctx = (fz_context *)client_data;
	//fz_warn(ctx, "openjpeg error: %s", msg);
}

static void fz_opj_warning_callback(const char *msg, void *client_data)
{
	//fz_context *ctx = (fz_context *)client_data;
	//fz_warn(ctx, "openjpeg warning: %s", msg);
}

static void fz_opj_info_callback(const char *msg, void *client_data)
{
	//fz_warn("openjpeg info: %s", msg);
}*/

PdfJPXFilter::PdfJPXFilter( Allocator * allocator /*= nullptr*/ )
    : PdfFilter(allocator)
{
}

PdfJPXFilter::~PdfJPXFilter()
{
}

void	PdfJPXFilter::Encode( uint8_t * data, size_t size, Buffer & buffer )
{
}
    
void	PdfJPXFilter::Decode( uint8_t * data, size_t size, Buffer & buffer )
{
    if ( !data || size <= 2 )
    {
        return;
    }
    
    //todo, add some error handle please!
	opj_dparameters_t   params;
    opj_codec_t *       info = nullptr;
    opj_stream_t *      input_stream = nullptr;
	opj_image_t *       jpx = nullptr;
    OPJ_CODEC_FORMAT format;

	int a, n, w, h, depth, sgnd;
	int x, y, k, v;
    
	/* Check for SOC marker -- if found we have a bare J2K stream */
	if ( data[0] == 0xFF && data[1] == 0x4F )
    {
		format = OPJ_CODEC_J2K;
    }else{
		format = OPJ_CODEC_JP2;
    }
    
	//memset(&evtmgr, 0, sizeof(evtmgr));
	//evtmgr.error_handler = fz_opj_error_callback;
	//evtmgr.warning_handler = fz_opj_warning_callback;
	//evtmgr.info_handler = fz_opj_info_callback;
    
	opj_set_default_decoder_parameters( &params );
	
    //if (indexed) ???
	{
        params.flags |= OPJ_DPARAMETERS_IGNORE_PCLR_CMAP_CDEF_FLAG;
    }
    
	info = opj_create_decompress( format );
    
	//opj_set_event_mgr( (opj_common_ptr)info, &evtmgr, nullptr );
	opj_setup_decoder(info, &params );
    
    input_stream = opj_stream_default_create(true);
    opj_stream_set_user_data(input_stream, data, nullptr);
    opj_stream_set_user_data_length(input_stream, size);
    
	//cio = opj_cio_open( (opj_common_ptr)info, data, (uint32_t)size );
    
    opj_decode(info, input_stream, jpx);
    
    opj_stream_destroy(input_stream);
	opj_destroy_codec(info);
    
	if ( !jpx )
    {
        //decode error!!!
        return;
    }
    
	for (k = 1; k < jpx->numcomps; k++)
	{
		if (jpx->comps[k].w != jpx->comps[0].w)
		{
			opj_image_destroy(jpx);
			//fz_throw(ctx, "image components have different width");
		}
		if (jpx->comps[k].h != jpx->comps[0].h)
		{
			opj_image_destroy(jpx);
			//fz_throw(ctx, "image components have different height");
		}
		if (jpx->comps[k].prec != jpx->comps[0].prec)
		{
			opj_image_destroy(jpx);
			//fz_throw(ctx, "image components have different precision");
		}
	}
    
	n = jpx->numcomps;
	w = jpx->comps[0].w;
	h = jpx->comps[0].h;
	depth = jpx->comps[0].prec;
	sgnd = jpx->comps[0].sgnd;
    
	if (jpx->color_space == OPJ_CLRSPC_SRGB && n == 4) { n = 3; a = 1; }
	else if (jpx->color_space == OPJ_CLRSPC_SYCC && n == 4) { n = 3; a = 1; }
	else if (n == 2) { n = 1; a = 1; }
	else if (n > 4) { n = 4; a = 1; }
	else { a = 0; }
    
    buffer.Alloc( n * w * h );
	uint8_t * pByte = buffer.GetData();
    
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			for (k = 0; k < n + a; k++)
			{
				v = jpx->comps[k].data[y * w + x];
				if (sgnd)
					v = v + (1 << (depth - 1));
				if (depth > 8)
					v = v >> (depth - 8);
				*pByte++ = v;
				//buffer.Write( (uint8_t *)&v, 1 );
			}
// 			if (!a)
//                 v = 255;
//                 buffer.Write( (uint8_t *)&v, 1 );
		}
	}
 
	opj_image_destroy( jpx );
}

PdfJBig2Filter::PdfJBig2Filter( Allocator * allocator /*= nullptr*/)
	: PdfFilter(allocator), globals_param_(nullptr), globals_param_size_(0)
{
}

PdfJBig2Filter::~PdfJBig2Filter()
{
}

void PdfJBig2Filter::SetGlobals( uint8_t * data, size_t size )
{
	globals_param_ = data;
	globals_param_size_ = size;
}

void PdfJBig2Filter::Encode( uint8_t * data, size_t size, Buffer & buffer )
{
}

void PdfJBig2Filter::Decode( uint8_t * data, size_t size, Buffer & buffer )
{
	if ( data == nullptr || size == 0 )
	{
		return;
	}

	Jbig2Ctx *ctx = jbig2_ctx_new( nullptr, JBIG2_OPTIONS_EMBEDDED, nullptr, nullptr, nullptr );
	Jbig2GlobalCtx *gctx = nullptr;
	Jbig2Image *page = nullptr;

	if ( globals_param_ )
	{
		jbig2_data_in( ctx, globals_param_, globals_param_size_ );
		gctx = jbig2_make_global_ctx( ctx );
		ctx = jbig2_ctx_new( nullptr, JBIG2_OPTIONS_EMBEDDED, gctx, nullptr, nullptr );
	}
    
	if ( !page )
	{
        jbig2_data_in( ctx, data, size );
		jbig2_complete_page( ctx );
		page = jbig2_page_out( ctx );
        //if page = nullptr error happened!
	}
    
    uint8_t * p = page->data;
	uint8_t * s = page->data;
	int32_t w = page->height * page->stride;
	int32_t x = 0;
 	while (x < w)
 	{
 		*p++ = ~ s[x++];
		//*p++ = s[x++] ^ 0xff;
 	}
    
    buffer.Clear();
    buffer.Write( page->data, page->height * page->stride );


    if ( page )
    {
		jbig2_release_page( ctx, page );
    }
	if ( gctx )
	{
		jbig2_global_ctx_free( gctx );
	}
	if ( ctx )
	{
		jbig2_ctx_free( ctx );
	}
}

}//namespace
