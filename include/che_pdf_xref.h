#ifndef _CHE_PDF_XREF_H_
#define _CHE_PDF_XREF_H_

#include <vector>
#include <map>

#include "che_pdf_object.h"

namespace chepdf {

enum PDF_XREF_ENTRY_TYPE
{
    XREF_ENTRY_TYPE_FREE		= 0x00,
    XREF_ENTRY_TYPE_COMMON		= 0x01,
    XREF_ENTRY_TYPE_COMPRESSED	= 0x02,
    XREF_ENTRY_TYPE_NEW			= 0x03
};

class PdfXRefEntry
{
public:
    PdfXRefEntry();
    PdfXRefEntry(PDF_XREF_ENTRY_TYPE type, uint32_t objNum, uint32_t field1, uint32_t field2);

	PDF_XREF_ENTRY_TYPE GetType() const { return type_; }
    
    uint32_t GetObjectNumber() const { return object_number_; }
    uint32_t GetOffset() const { return field1_; }
    uint32_t GetGenerateNumber() const { return field2_; }
    uint32_t GetParentObjectNumber() const { return field1_; }
    uint32_t GetIndex() const { return field2_; }

    PDF_XREF_ENTRY_TYPE type_;
    uint32_t object_number_;
    uint32_t field1_;
    uint32_t field2_;
};


class PdfXRefTable : public BaseObject
{
public:
	PdfXRefTable(Allocator * allocator = nullptr);
	~PdfXRefTable();

	bool Add(const PdfXRefEntry & entry);
	void NewEntry(PdfXRefEntry & entry_out);
	bool AddTrailerDictionary(const PdfDictionaryPointer & dictionary);

	void Clean();

	bool Get(uint32_t object_number, PdfXRefEntry & entry_out);
    
	uint32_t GetTrailerCount() const { return (uint32_t)trailer_dictionaries_.size(); }
	PdfDictionaryPointer GetTrailer(uint32_t index = 0) const;
	
    uint32_t GetMaxObjectNumber() const { return max_object_number_; }

	bool IsEOF() { return iterator_ == map_.end(); }
	void First() { iterator_ = map_.begin(); }
	void Next() { ++iterator_; }
	bool GetCurrentNode(PdfXRefEntry & entry_out);
	bool Update(uint32_t object_number, const PdfXRefEntry & entry_out);

private:
	uint32_t                                        max_object_number_;
	std::vector<PdfDictionaryPointer>               trailer_dictionaries_;
    std::map<uint32_t, PdfXRefEntry>                map_;
    std::map<uint32_t, PdfXRefEntry>::iterator      iterator_;
};

}//namespace

#endif
