#include <memory.h>

#include "../include/che_pdf_xref.h"

namespace chepdf {
    
using namespace std;

PdfXRefEntry::PdfXRefEntry()
{
	type_ = XREF_ENTRY_TYPE_FREE;
	object_number_ = 0;
	field1_ = 0;
	field2_ = 0;
}

PdfXRefEntry::PdfXRefEntry(PDF_XREF_ENTRY_TYPE type, uint32_t object_number, uint32_t field1, uint32_t field2)
{
	type_ = type;
	object_number_ = object_number;
	field1_ = field1;
	field2_ = field2;
}

PdfXRefTable::PdfXRefTable(Allocator * allocator)
: BaseObject(allocator), max_object_number_(0) {}

PdfXRefTable::~PdfXRefTable()
{
	Clean();
}

void PdfXRefTable::Clean()
{
	trailer_dictionaries_.clear();
	map_.clear();
}

bool PdfXRefTable::Add(const PdfXRefEntry & entry)
{
	map_[entry.object_number_] = entry;
	if (entry.object_number_ > max_object_number_)
	{
		max_object_number_ = entry.object_number_;
		return TRUE;
	}
	return false;
}

void PdfXRefTable::NewEntry(PdfXRefEntry & entry_out)
{
	entry_out.object_number_ = ++max_object_number_;
	entry_out.type_ = XREF_ENTRY_TYPE_NEW;
	entry_out.field1_ = 0;
	entry_out.field2_ = 0;
	map_[entry_out.object_number_] = entry_out;
}

bool PdfXRefTable::AddTrailerDictionary(const PdfDictionaryPointer & dictionary)
{
	if (dictionary)
	{
		trailer_dictionaries_.push_back(dictionary);
		return TRUE;
	}
	return false;
}

PdfDictionaryPointer PdfXRefTable::GetTrailer(uint32_t index /*= 0*/) const
{
	if (index >= trailer_dictionaries_.size())
	{
		return PdfDictionaryPointer();
	}
	return trailer_dictionaries_[index];
}

bool PdfXRefTable::Get(uint32_t object_number, PdfXRefEntry & entry_out)
{
	map<uint32_t, PdfXRefEntry>::iterator it;
	it = map_.find(object_number);
	if (it != map_.end())
	{
		entry_out = it->second;
		return TRUE;
	}
	return false;
}

bool PdfXRefTable::GetCurrentNode(PdfXRefEntry & entry_out)
{
	if (iterator_ != map_.end())
	{
		entry_out = iterator_->second;
		return TRUE;
	}
	return false;
}

bool PdfXRefTable::Update(uint32_t object_number, const PdfXRefEntry & entry)
{
	map<uint32_t, PdfXRefEntry>::iterator it;
	it = map_.find(object_number);
	if (it != map_.end())
	{
		it->second = entry;
		return TRUE;
	}
	return false;
}

}//namespace
