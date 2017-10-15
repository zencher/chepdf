#ifndef _CHE_PDF_FILTER_H_
#define _CHE_PDF_FILTER_H_

#include "che_pdf_object.h"

namespace chepdf {

class PdfFilter : public BaseObject
{
public:
	PdfFilter(Allocator * allocator = nullptr) : BaseObject(allocator) {}
	virtual	~PdfFilter() {};

	virtual void Encode(uint8_t * data, size_t size, Buffer & buffer ) = 0;
	virtual void Decode(uint8_t * data, size_t size, Buffer & buffer ) = 0;
};

static inline uint32_t getcomponent(uint8_t * line, uint32_t x, uint32_t bpc)
{
	switch (bpc)
	{
	case 1: return (line[x >> 3] >> ( 7 - (x & 7) ) ) & 1;
	case 2: return (line[x >> 2] >> ( ( 3 - (x & 3) ) << 1 ) ) & 3;
	case 4: return (line[x >> 1] >> ( ( 1 - (x & 1) ) << 2 ) ) & 15;
	case 8: return line[x];
	case 16: return (line[x << 1] << 8) + line[(x << 1) + 1];
	}
	return 0;
}

static inline void putcomponent(uint8_t * buffer, uint32_t x, uint32_t bpc, uint32_t value)
{
	switch (bpc)
	{
	case 1: buffer[x >> 3] |= value << (7 - (x & 7)); break;
	case 2: buffer[x >> 2] |= value << ((3 - (x & 3)) << 1); break;
	case 4: buffer[x >> 1] |= value << ((1 - (x & 1)) << 2); break;
	case 8: buffer[x] = value; break;
	case 16: buffer[x << 1] = value >> 8; buffer[(x << 1) + 1] = value; break;
	}
}

static inline int fz_absi(int i)
{
	return (i < 0 ? -i : i);
}

static inline int paeth(int a, int b, int c)
{
	/* The definitions of ac and bc are correct, not a typo. */
	int ac = b - c, bc = a - c, abcc = ac + bc;
	int pa = fz_absi(ac);
	int pb = fz_absi(bc);
	int pc = fz_absi(abcc);
	return pa <= pb && pa <= pc ? a : pb <= pc ? b : c;
}


class PdfFilterPredictor : public BaseObject
{
public:
	PdfFilterPredictor(PdfDictionaryPointer & dictionary, Allocator * allocator = nullptr)
		: BaseObject(allocator), predictor_(1), bpc_(8), bpp_((8+7)/8), early_change_(1),
		colors_(1), columns_(1), stride_((8 + 7) / 8), output_(nullptr), ref_(nullptr)
	{
		if (dictionary)
		{
			PdfObjectPointer object = dictionary->GetElement( "Predictor" );
			if (object && object->GetType() == OBJ_TYPE_NUMBER )
			{	
				predictor_ = object->GetPdfNumber()->GetInteger();
			}
			object = dictionary->GetElement( "colors_" );
			if (object && object->GetType() == OBJ_TYPE_NUMBER )
			{
				colors_ = object->GetPdfNumber()->GetInteger();
			}
			object = dictionary->GetElement( "BitsPerComponent" );
			if (object && object->GetType() == OBJ_TYPE_NUMBER )
			{
				bpc_ = object->GetPdfNumber()->GetInteger();
			}
			object = dictionary->GetElement( "columns_" );
			if (object && object->GetType() == OBJ_TYPE_NUMBER )
			{
				columns_ = object->GetPdfNumber()->GetInteger();
			}
			object = dictionary->GetElement( "early_change_" );
			if (object && object->GetType() == OBJ_TYPE_NUMBER )
			{
				early_change_ = object->GetPdfNumber()->GetInteger();
			}
			bpp_ = ((bpc_ * colors_ + 7) / 8);
			stride_ = ((bpc_ * colors_ * columns_ + 7) / 8);
			output_ = GetAllocator()->NewArray<uint8_t>(stride_ + 1);
			memset(output_, 0, stride_ + 1);
			ref_ = GetAllocator()->NewArray<uint8_t>(stride_ + 1);
			memset(ref_, 0, stride_ + 1);
		}
	}

    PdfFilterPredictor(uint8_t predictor = 1, uint8_t colors = 1, uint8_t bits_per_component = 8,
                       uint32_t columns = 1, uint8_t early_change = 1, Allocator * allocator = nullptr)
		: BaseObject(allocator), predictor_(predictor), bpc_(bits_per_component), bpp_((bits_per_component * colors + 7) / 8),
		early_change_(early_change), colors_(colors), columns_(columns), stride_((bits_per_component * colors * columns + 7) / 8),
		output_(nullptr), ref_(nullptr)
	{
		output_ = GetAllocator()->NewArray<uint8_t>(stride_ + 1);
		ref_ = GetAllocator()->NewArray<uint8_t>(stride_ + 1);
	}

    ~PdfFilterPredictor()
    {
		if (output_)
		{
			GetAllocator()->DeleteArray(output_);
		}
		if (ref_)
		{
			GetAllocator()->DeleteArray(ref_);
		}
    }

	void PredirectTiff(uint8_t * data, Buffer & buffer)
	{
		int left[32];
		int i, k;
		const int mask = (1 << bpc_) - 1;

		for (k = 0; k < colors_; ++k)
		{
			left[k] = 0;
		}
		memset(output_, 0, stride_);
		for (i = 0; i < columns_; ++i)
		{
			for (k = 0; k < colors_; ++k)
			{
				int a = getcomponent(data, i * colors_ + k, bpc_);
				int b = a + left[k];
				int c = b & mask;
				putcomponent(output_, i * colors_ + k, bpc_, c);
				left[k] = c;
			}
		}
	}

	void PredirectPng(uint8_t * pData, Buffer & buffer, uint8_t predictor)
	{
		size_t i = 0;
		uint8_t * p = pData;
		uint8_t * pOut = output_;
		uint8_t * pRef = ref_;
		switch (predictor)
		{
		case 0:
			{
				memcpy( output_, p, stride_ );
				buffer.Write( output_, stride_ );
				break;
			}
		case 1:
			{
				for (i = bpp_; i > 0; --i)
				{
					*pOut = *p;
					++pOut;
					++p;
				}
				for (i = stride_ - bpp_; i > 0; --i)
				{
					*pOut = *p + pOut[-bpp_];
					++pOut;
					++p;
				}
				buffer.Write( output_, stride_ );
				break;
			}
		case 2:
			{
				for (i = bpp_; i > 0; --i)
				{
					*pOut = *p + *pRef;
					++p;
					++pOut;
					++pRef;
				}
				for (i = stride_ - bpp_; i > 0; --i)
				{
					*pOut = *p + *pRef;
					++p;
					++pOut;
					++pRef;
				}
				buffer.Write(output_, stride_);
				break;
			}
		case 3:
			{
				for (i = bpp_; i > 0; --i)
				{
					*pOut = *p + *pRef/2;
					++p;
					++pOut;
					++pRef;
				}
				for (i = stride_ - bpp_; i > 0; --i)
				{
					*pOut = *p + (pOut[-bpp_] + *pRef) / 2;
					++p;
					++pOut;
					++pRef;
				}
				buffer.Write(output_, stride_);
				break;
			}
		case 4:
			{
				for (i = bpp_; i > 0; --i)
				{
					*pOut = *p + paeth(0, *pRef, 0);
					++p;
					++pOut;
					++pRef;
				}
				for (i = stride_ - bpp_; i > 0; --i)
				{
					*pOut = *p + paeth(pOut[-bpp_], *pRef, pRef[-bpp_]);
					++p;
					++pOut;
					++pRef;
				}
				buffer.Write(output_, stride_);
				break;
			}
		}
		memcpy(ref_, output_, stride_);
	}

    void Decode(uint8_t * data, size_t size, Buffer & buffer)
    {
		if (data == nullptr || size == 0)
		{
			return;
		}
		if ( predictor_ != 1 && predictor_ != 2 &&
			 predictor_ != 10 && predictor_ != 11 &&
			 predictor_ != 12 && predictor_ != 13 &&
			 predictor_ != 14 && predictor_ != 15 )
		{
			return;
		}
		uint8_t * p	 = data;
		uint8_t * ep = data + size;
		while (p < ep)
		{
			if (predictor_ == 1)
			{
				buffer.Write(p, stride_);
				p += stride_;
			}
			else if (predictor_ == 2)
			{
				PredirectTiff(p, buffer);
				p += stride_;
			}
            else{
				PredirectPng(p + 1, buffer, *p);
				p += stride_ + 1;
			}
		}
    }

private:
    uint8_t predictor_;
    uint8_t bpc_;
	uint8_t bpp_;
	uint8_t early_change_;
	uint32_t colors_;
    uint32_t columns_;
	uint32_t stride_;
	uint8_t * output_;
	uint8_t * ref_;
};

class PdfHexFilter : public PdfFilter
{
public:
	PdfHexFilter(Allocator * allocator = nullptr) : PdfFilter(allocator) {}
	~PdfHexFilter() {};
	
	void Encode(uint8_t * data, size_t size, Buffer & buffer);
	void Decode(uint8_t * data, size_t size, Buffer & buffer);
};

class PdfASCII85Filter : public PdfFilter
{
public:
	PdfASCII85Filter(Allocator * allocator = nullptr) : PdfFilter(allocator) {}
	~PdfASCII85Filter() {};

    void Encode(uint8_t * data, size_t size, Buffer & buffer);
    void Decode(uint8_t * data, size_t size, Buffer & buffer);

private:
	void EncodeTuple(size_t tuple, size_t count,  Buffer & buffer);
	void WidePut(size_t tuple, size_t bytes, Buffer & buffer);

	size_t count_;
    size_t tuple_;
};

class PdfRLEFileter : public PdfFilter
{
public:
	PdfRLEFileter(Allocator * allocator = nullptr) : PdfFilter(allocator) {}
	~PdfRLEFileter() {};

    void Encode(uint8_t * data, size_t size, Buffer & buffer);
    void Decode(uint8_t * data, size_t size, Buffer & buffer);
};

class PdfFlateFilter : public PdfFilter
{
public:
	PdfFlateFilter( Allocator * allocator = nullptr) : PdfFilter( allocator ) {}
	~PdfFlateFilter() {}

    void Encode(uint8_t * data, size_t size, Buffer & buffer);
    void Decode(uint8_t * data, size_t size, Buffer & buffer);
};

struct TLzwItem {
	std::vector<uint8_t> value;
};

typedef std::vector<TLzwItem> TLzwTable;
typedef TLzwTable::iterator TILzwTable;
typedef TLzwTable::const_iterator TCILzwTable;

class PdfLZWFilter : public PdfFilter
{
public:
	PdfLZWFilter(Allocator * allocator = nullptr) : PdfFilter(allocator) {}
	~PdfLZWFilter() {};

    void Encode(uint8_t * data, size_t size, Buffer & buffer);
    void Decode(uint8_t * data, size_t size, Buffer & buffer);
	
private:
	void InitTable();

    static const unsigned short s_masks_[4];
    static const unsigned short s_clear_;
    static const unsigned short s_eod_;
	
    TLzwTable   table_;
    uint32_t    mask_;
    uint32_t    code_len_;
    uint8_t     character_;
    bool        b_first_;
};

class PdfFaxDecodeParams
{
public:
	PdfFaxDecodeParams(PdfDictionaryPointer dictionary);

	int32_t k;
	int32_t columns;
	int32_t rows;
	bool eol;
	bool eba;
	bool eob;
	bool bi1;
};

class PdfFaxFilter : public PdfFilter
{
public:
	PdfFaxFilter(PdfFaxDecodeParams * params = nullptr, Allocator * allocator = nullptr);
	~PdfFaxFilter();

    void Encode(uint8_t * data, size_t size, Buffer & buffer);
    void Decode(uint8_t * data, size_t size, Buffer & buffer);

private:
	PdfFaxDecodeParams * params_;
};

class PdfDCTDFilter : public PdfFilter
{
public:
    PdfDCTDFilter(Allocator * allocator = nullptr)
        : PdfFilter(allocator) {};
    ~PdfDCTDFilter() {};
    
    void Encode(uint8_t * data, size_t size, Buffer & buffer);
    void Decode(uint8_t * data, size_t size, Buffer & buffer);
};

class PdfJPXFilter : public PdfFilter
{
public:
    PdfJPXFilter(Allocator * allocator = nullptr);
    ~PdfJPXFilter();
    
    void Encode(uint8_t * data, size_t size, Buffer & buffer);
    void Decode(uint8_t * data, size_t size, Buffer & buffer);
};

class PdfJBig2Filter : public PdfFilter
{
public:
	PdfJBig2Filter(Allocator * allocator = nullptr);
	~PdfJBig2Filter();

	void SetGlobals(uint8_t * data, size_t size);

   void Encode(uint8_t * data, size_t size, Buffer & buffer);
   void Decode(uint8_t * data, size_t size, Buffer & buffer);

private:
	uint8_t *	globals_param_;
    size_t      globals_param_size_;
};

}//namespace

#endif
