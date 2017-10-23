#include <memory.h>

#include "../include/che_pdf_object.h"
#include "../include/che_pdf_crypto.h"
#include "../include/che_pdf_filter.h"
//#include "../include/che_pdf_parser.h"
//#include "../include/che_pdf_file.h"

namespace chepdf {
    
using namespace std;

PdfObject::PdfObject(PDF_OBJ_TYPE type, Allocator * allocator/*= nullptr*/)
	: BaseObject(allocator), b_modified_(false), type_(type) {}

PdfObjectPointer PdfObject::Clone()
{
	PdfObjectPointer pointer;
	switch (type_)
	{
	case OBJ_TYPE_NULL:
		pointer = GetPdfNull()->Clone();
		break;
	case OBJ_TYPE_BOOLEAN:
		pointer = GetPdfBoolean()->Clone();
		break;
	case OBJ_TYPE_NUMBER:
		pointer = GetPdfNumber()->Clone();
		break;
	case OBJ_TYPE_STRING:
		pointer = GetPdfString()->Clone();
		break;
	case OBJ_TYPE_NAME:
		pointer = GetPdfName()->Clone();
		break;
	case OBJ_TYPE_ARRAY:
		pointer = GetPdfArray()->Clone();
		break;
	case OBJ_TYPE_DICTIONARY:
		pointer = GetPdfDictionary()->Clone();
		break;
	case OBJ_TYPE_REFERENCE:
		pointer = GetPdfReference()->Clone();
		break;
	case OBJ_TYPE_STREAM:
		pointer = GetPdfStream()->Clone();
		break;
	case OBJ_TYPE_INVALID:
	default:
		break;
	}
	return pointer;
}

void PdfObject::Release()
{
	switch (type_)
	{
	case OBJ_TYPE_NULL:
		GetAllocator()->Delete<PdfNull>((PdfNull*)this);
		break;
	case OBJ_TYPE_BOOLEAN:
		GetAllocator()->Delete<PdfBoolean>((PdfBoolean*)this);
		break;
	case OBJ_TYPE_NUMBER:
		GetAllocator()->Delete<PdfNumber>((PdfNumber*)this);
		break;
	case OBJ_TYPE_STRING:
		GetAllocator()->Delete<PdfString>((PdfString*)this);
		break;
	case OBJ_TYPE_NAME:
		GetAllocator()->Delete<PdfName>((PdfName*)this);
		break;
	case OBJ_TYPE_ARRAY:
		GetAllocator()->Delete<PdfArray>((PdfArray*)this);
		break;
	case OBJ_TYPE_DICTIONARY:
		GetAllocator()->Delete<PdfDictionary>((PdfDictionary*)this);
		break;
	case OBJ_TYPE_REFERENCE:
		GetAllocator()->Delete<PdfReference>((PdfReference*)this);
		break;
	case OBJ_TYPE_STREAM:
		GetAllocator()->Delete<PdfStream>((PdfStream*)this);
		break;
	case OBJ_TYPE_INVALID:
	default:
		GetAllocator()->Delete<PdfObject>(this);
		break;
	}
}

void PdfObject::SetModified(bool value)
{
	b_modified_ = value;
}

bool PdfObject::IsModified()
{
	return b_modified_;
}

PdfNullPointer PdfObject::GetPdfNull() const
{
	PdfNullPointer pointer;
	if (type_ == OBJ_TYPE_NULL)
	{
		pointer.Reset((PdfObject*)this);
	}
	return pointer;
}

PdfBooleanPointer PdfObject::GetPdfBoolean() const
{
	PdfBooleanPointer pointer;
	if (type_ == OBJ_TYPE_BOOLEAN)
	{
		pointer.Reset((PdfObject*)this);
	}
	return pointer;
}

PdfNumberPointer PdfObject::GetPdfNumber() const
{
	PdfNumberPointer pointer;
	if (type_ == OBJ_TYPE_NUMBER)
	{
		pointer.Reset((PdfObject*)this);
	}
	return pointer;
}

PdfStringPointer PdfObject::GetPdfString() const
{
	PdfStringPointer pointer;
	if (type_ == OBJ_TYPE_STRING)
	{
		pointer.Reset((PdfObject*)this);
	}
	return pointer;
}

PdfNamePointer PdfObject::GetPdfName() const
{
	PdfNamePointer pointer;
	if (type_ == OBJ_TYPE_NAME)
	{
		pointer.Reset((PdfObject*)this);
	}
	return pointer;
}

PdfArrayPointer PdfObject::GetPdfArray() const
{
	PdfArrayPointer pointer;
	if (type_ == OBJ_TYPE_ARRAY)
	{
		pointer.Reset((PdfObject*)this);
	}
	return pointer;
}

PdfDictionaryPointer PdfObject::GetPdfDictionary() const
{
	PdfDictionaryPointer pointer;
	if (type_ == OBJ_TYPE_DICTIONARY)
	{
		pointer.Reset((PdfObject*)this);
	}
	return pointer;
}

PdfReferencePointer PdfObject::GetPdfReference() const
{
	PdfReferencePointer pointer;
	if (type_ == OBJ_TYPE_REFERENCE)
	{
		pointer.Reset((PdfObject*)this);
	}
	return pointer;
}

PdfStreamPointer PdfObject::GetPdfStream() const
{
	PdfStreamPointer pointer;
	if (type_ == OBJ_TYPE_STREAM)
	{
		pointer.Reset((PdfObject*)this);
	}
	return pointer;
}

PdfObjectPointer::PdfObjectPointer(const PdfObjectPointer & pointer)
	: object_(nullptr)
{
	if (object_ != pointer.object_ && pointer.object_ != nullptr)
	{
		if (object_)
		{
			object_->referenceCount_.Decrease();
			if (pointer->referenceCount_ == 0)
			{
				object_->Release();
			}
		}
		object_ = pointer.object_;
		if (object_)
		{
			object_->referenceCount_.Decrease();
		}
	}
}

PdfObjectPointer::~PdfObjectPointer()
{
	if (object_)
	{
		object_->referenceCount_.Decrease();
		if (object_->referenceCount_ == 0)
		{
			object_->Release();
		}
	}
}

PdfObjectPointer PdfObjectPointer::operator=(const PdfObjectPointer & pointer)
{
	if (object_)
	{
		if (object_ == pointer.object_)
		{
			return *this;
		}
		object_->referenceCount_.Decrease();
		if (object_->referenceCount_ == 0)
		{
			object_->Release();
			object_ = nullptr;
		}
	}
	object_ = pointer.object_;
	if (object_)
	{
		object_->referenceCount_.Increase();
	}
	return *this;
}

void PdfObjectPointer::Reset(PdfObject * object/*= nullptr*/)
{
	if (object_ != object)
	{
		if (object_)
		{
			object_->referenceCount_.Decrease();
			if (object_->referenceCount_ == 0)
			{
				object_->Release();
			}
		}
		object_ = object;
		if (object_)
		{
			object_->referenceCount_.Increase();
		}
	}
}

PdfNullPointer PdfNull::Create(Allocator * allocator/*= nullptr*/)
{
	PdfNullPointer pointer;
	if (allocator == nullptr)
	{
        allocator = Allocator::GetDefaultAllocator();
	}
	pointer.Reset(allocator->New<PdfNull>(allocator));
	return pointer;
}

PdfNullPointer PdfNull::Clone()
{
	PdfNullPointer pointer;
	pointer.Reset(GetAllocator()->New<PdfNull>(GetAllocator()));
	return pointer;
}

PdfBooleanPointer PdfBoolean::Create(bool value, Allocator * allocator/*= nullptr*/)
{
	PdfBooleanPointer pointer;
	if (allocator == nullptr)
	{
        allocator = Allocator::GetDefaultAllocator();
	}
	pointer.Reset(allocator->New<PdfBoolean>(value, allocator));
	return pointer;
}

PdfBooleanPointer PdfBoolean::Clone()
{
	PdfBooleanPointer pointer;
	pointer.Reset(GetAllocator()->New<PdfBoolean>(value_, GetAllocator()));
	return pointer;
}

PdfNumberPointer PdfNumber::Create(int32_t value, Allocator * allocator/*= nullptr*/)
{
	PdfNumberPointer pointer;
	if (allocator == nullptr)
	{
        allocator = Allocator::GetDefaultAllocator();
	}
	pointer.Reset(allocator->New<PdfNumber>(value, allocator));
	return pointer;
}

PdfNumberPointer PdfNumber::Create(FLOAT value, Allocator * allocator/*= nullptr*/)
{
	PdfNumberPointer pointer;
	if (allocator == nullptr)
	{
        allocator = Allocator::GetDefaultAllocator();
	}
	pointer.Reset(allocator->New<PdfNumber>(value, allocator));
	return pointer;
}

PdfNumberPointer PdfNumber::Clone()
{
	PdfNumberPointer pointer;
	if (b_integer_)
	{
		pointer.Reset(GetAllocator()->New<PdfNumber>(interger_, GetAllocator()));
	}else
	{
		pointer.Reset(GetAllocator()->New<PdfNumber>(float_, GetAllocator()));
	}
	return pointer;
}

PdfStringPointer PdfString::Create(const ByteString & str, Allocator * allocator/*= nullptr*/)
{
	PdfStringPointer pointer;
	if ( allocator == nullptr )
	{
		allocator = Allocator::GetDefaultAllocator();
	}
	pointer.Reset(allocator->New<PdfString>(str, allocator));
	return pointer;
}

ByteString & PdfString::GetString()
{
	return string_;
}

void PdfString::SetString(ByteString & str)
{
	string_ = str;
	SetModified(true);
}

PdfStringPointer PdfString::Clone()
{
	PdfStringPointer pointer;
	pointer.Reset(GetAllocator()->New<PdfString>(string_, GetAllocator()));
	return pointer;
}

PdfNamePointer PdfName::Create(const ByteString & str, Allocator * allocator/*= nullptr*/)
{
	PdfNamePointer pointer;
	if (allocator == nullptr)
	{
        allocator = Allocator::GetDefaultAllocator();
	}
	pointer.Reset(allocator->New<PdfName>(str, allocator));
	return pointer;
}

PdfNamePointer PdfName::Clone()
{
	PdfNamePointer pointer;
	pointer.Reset( GetAllocator()->New<PdfName>(name_, GetAllocator()));
	return pointer;
}

PdfReferencePointer PdfReference::Create(uint32_t object_number, uint32_t generate_number, PdfFile * file, Allocator * allocator/*= nullptr*/)
{
	PdfReferencePointer pointer;
	if (allocator == nullptr)
	{
        allocator = Allocator::GetDefaultAllocator();
	}
	pointer.Reset(allocator->New<PdfReference>(object_number, generate_number, file, allocator));
	return pointer;
}

PdfReferencePointer PdfReference::Clone()
{
	PdfReferencePointer pointer;
	pointer.Reset(GetAllocator()->New<PdfReference>(object_number_, generate_number_, file_, GetAllocator()));
	return pointer;
}

PdfObjectPointer PdfReference::GetPdfObject()
{
	if (file_ == nullptr)
	{
		return PdfObjectPointer();
	}
    //todo
	/*PDF_RefInfo refInfo;
	refInfo.object_number = object_number_;
	refInfo.generate_number = generate_number_;
	return file_->GetObject(refInfo);*/
    return PdfObjectPointer();
}

PdfObjectPointer PdfReference::GetPdfObject(PDF_OBJ_TYPE type)
{
	PdfObjectPointer pointer;
	if (file_ == nullptr)
	{
		return PdfObjectPointer();
	}
    /*PDF_RefInfo refInfo;
    refInfo.object_number = object_number_;
    refInfo.generate_number = generate_number_;
	pointer = file_->GetObject(refInfo);*/
	if (!pointer)
	{
		return pointer;
	}
	if (pointer->GetType() == type || type == OBJ_TYPE_INVALID)
	{
		return pointer;
	}

	while (true)
	{
		if (pointer->GetType() == OBJ_TYPE_REFERENCE)
		{
			pointer = pointer->GetPdfReference()->GetPdfObject();
			if (!pointer)
			{
				return PdfReferencePointer();
			}
			if (pointer->GetType() == type)
			{
				return pointer;
			}
			if (pointer->GetType() == OBJ_TYPE_REFERENCE || pointer->GetType() == OBJ_TYPE_ARRAY)
			{
				continue;
			}
			return PdfObjectPointer();
		}else if (pointer->GetType() == OBJ_TYPE_ARRAY)
		{
			PdfObjectPointer pElement;
			uint32_t count = pointer->GetPdfArray()->GetSize();
			bool bNeedContinue = false;
			for (uint32_t index = 0; index < count; ++index)
			{
				pElement = pointer->GetPdfArray()->GetElement(index, type);
				if (!pElement)
				{
					continue;
				}
				if (pElement->GetType() == type)
				{
					return pElement;
				}else if (pElement->GetType() == OBJ_TYPE_REFERENCE || pElement->GetType() == OBJ_TYPE_ARRAY)
				{
					pointer = pElement;
					bNeedContinue = true;
					break;
				}
			}
			if ( bNeedContinue == true )
			{
				continue;
			}
			return PdfObjectPointer();
		}else if (pointer->GetType() == type)
		{
			return pointer;
		}else{
			return PdfObjectPointer();
		}
	}
	return PdfObjectPointer();
}

PdfArrayPointer PdfArray::Create(Allocator * allocator /*= nullptr*/)
{
	PdfArrayPointer pointer;
	if (allocator == nullptr)
	{
        allocator = Allocator::GetDefaultAllocator();
	}
	pointer.Reset( allocator->New<PdfArray>( allocator ) );
	return pointer;
}

PdfObjectPointer PdfArray::GetElement(uint32_t index) const
{
	if (index < GetSize())
	{
		return array_[index];
	}
	return PdfObjectPointer();
}

PdfObjectPointer PdfArray::GetElement(uint32_t index, PDF_OBJ_TYPE type) const
{
	PdfObjectPointer pointer = GetElement(index);
	if (!pointer)
	{
		return pointer;
	}

	if (pointer->GetType() == type || type == OBJ_TYPE_INVALID)
	{
		return pointer;
	}

	if (pointer->GetType() == OBJ_TYPE_REFERENCE)
	{
		return pointer->GetPdfReference()->GetPdfObject(type);
	}
	return PdfObjectPointer();
}

PdfObjectPointer PdfArray::GetElementByType(PDF_OBJ_TYPE type)
{
	PdfObjectPointer pointer;
	uint32_t count = (uint32_t)array_.size();
	for (uint32_t index = 0; index < count; ++index)
	{
		pointer = GetElement(index, type);
		if (pointer)
		{
			return pointer;
		}
	}
	return pointer;
}

bool PdfArray::Append(const PdfObjectPointer & pointer)
{
	if (pointer)
	{
		array_.push_back(pointer);
		SetModified(true);
		return true;
	}
	return false;
}

PdfNullPointer PdfArray::AppendNull()
{
    PdfNullPointer pointer = PdfNull::Create(GetAllocator());
    array_.push_back(pointer);
    SetModified(true);
    return pointer;
}

PdfBooleanPointer PdfArray::AppendBoolean(bool value/*= false*/)
{
    PdfBooleanPointer pointer = PdfBoolean::Create(value, GetAllocator());
    array_.push_back(pointer);
    SetModified(true);
    return pointer;
}

PdfNumberPointer PdfArray::AppendNumber()
{
    PdfNumberPointer pointer = PdfNumber::Create(0, GetAllocator());
    array_.push_back(pointer);
    SetModified(true);
    return pointer;
}

PdfNumberPointer PdfArray::AppendNumber(int32_t value)
{
    PdfNumberPointer pointer = PdfNumber::Create(value, GetAllocator());
    array_.push_back(pointer);
    SetModified(true);
    return pointer;
}

PdfNumberPointer PdfArray::AppendNumber(FLOAT value)
{
    PdfNumberPointer pointer = PdfNumber::Create(value, GetAllocator());
    array_.push_back(pointer);
    SetModified(true);
    return pointer;
}

PdfNamePointer PdfArray::AppendName()
{
    PdfNamePointer pointer = PdfName::Create(ByteString(GetAllocator()), GetAllocator());
    array_.push_back(pointer);
    SetModified(true);
    return pointer;
}

PdfNamePointer PdfArray::AppendName(ByteString & str)
{
    PdfNamePointer pointer = PdfName::Create(str, GetAllocator());
    array_.push_back(pointer);
    SetModified(true);
    return pointer;
}

PdfStringPointer PdfArray::AppendString()
{
    PdfStringPointer pointer = PdfString::Create(ByteString(GetAllocator()), GetAllocator());
    array_.push_back(pointer);
    SetModified(true);
    return pointer;
}

PdfStringPointer PdfArray::AppendString(ByteString & str)
{
    PdfStringPointer pointer = PdfString::Create(str, GetAllocator());
    array_.push_back(pointer);
    SetModified(true);
    return pointer;
}

PdfArrayPointer PdfArray::AppendArray()
{
    PdfArrayPointer pointer = PdfArray::Create();
    array_.push_back(pointer);
    SetModified(true);
    return pointer;
}

PdfDictionaryPointer PdfArray::AppendDictionary()
{
    PdfDictionaryPointer pointer = PdfDictionary::Create(GetAllocator());
    array_.push_back(pointer);
    SetModified(true);
    return pointer;
}

PdfReferencePointer PdfArray::AppendReference(PDF_RefInfo info, PdfFile * file)
{
    PdfReferencePointer pointer;
    if (file)
    {
        pointer = PdfReference::Create(info.object_number, info.generate_number, file);
        array_.push_back(pointer);
        SetModified(true);
    }
    return pointer;
}

PdfReferencePointer PdfArray::AppendReference(uint32_t object_number, uint32_t generate_number, PdfFile * file)
{
    PdfReferencePointer pointer;
    if (file)
    {
        pointer = PdfReference::Create(object_number, generate_number, file);
        array_.push_back(pointer);
        SetModified(true);
    }
    return pointer;
}

PdfReferencePointer PdfArray::AppendReference(PdfReferencePointer & pointer)
{
    if (pointer)
    {
        array_.push_back(pointer);
        SetModified(true);
    }
    return pointer;
}

PdfReferencePointer PdfArray::AppendReference(PdfFile * file)
{
    PdfReferencePointer pointer;
    if (file)
    {
        pointer = PdfReference::Create(0, 0, file);
        array_.push_back(pointer);
        SetModified(true);
    }
    return pointer;
}

bool PdfArray::Replace(uint32_t index, const PdfObjectPointer & pointer)
{
    if (index > array_.size() || !pointer)
    {
        return false;
    }
    array_[index] = pointer;
    SetModified(true);
    return true;
}

PdfNullPointer PdfArray::ReplaceNull(uint32_t index)
{
    PdfNullPointer pointer;
    if (index <= array_.size())
    {
        pointer = PdfNull::Create(GetAllocator());
        array_[index] = pointer;
        SetModified(true);
    }
    return pointer;
}

PdfBooleanPointer PdfArray::ReplaceBoolean(uint32_t index)
{
    PdfBooleanPointer pointer;
    if (index <= array_.size())
    {
        pointer = PdfBoolean::Create(false, GetAllocator());
        array_[index] = pointer;
        SetModified(true);
    }
    return pointer;
}

PdfNumberPointer PdfArray::ReplaceNumber(uint32_t index)
{
    PdfNumberPointer pointer;
    if (index <= array_.size())
    {
        pointer = PdfNumber::Create(0, GetAllocator());
        array_[index] = pointer;
        SetModified(true);
    }
    return pointer;
}

PdfNamePointer PdfArray::ReplaceName(uint32_t index)
{
    PdfNamePointer pointer;
    if (index <= array_.size())
    {
        pointer = PdfName::Create(ByteString(GetAllocator()), GetAllocator());
        array_[index] = pointer;
        SetModified(true);
    }
    return pointer;
}

PdfStringPointer PdfArray::ReplaceString(uint32_t index)
{
    PdfStringPointer pointer;
    if (index <= array_.size())
    {
        pointer = PdfString::Create(ByteString(GetAllocator()), GetAllocator());
        array_[index] = pointer;
        SetModified(true);
    }
    return pointer;
}

PdfArrayPointer PdfArray::ReplaceArray(uint32_t index)
{
    PdfArrayPointer pointer;
    if (index <= array_.size())
    {
        pointer = PdfArray::Create(GetAllocator());
        array_[index] = pointer;
        SetModified(true);
    }
    return pointer;
}


PdfDictionaryPointer PdfArray::ReplaceDictionary(uint32_t index)
{
    PdfDictionaryPointer pointer;
    if (index <= array_.size())
    {
        pointer = PdfDictionary::Create(GetAllocator());
        array_[index] = pointer;
        SetModified(true);
    }
    return pointer;
}


PdfReferencePointer PdfArray::ReplaceReference(uint32_t index, PdfFile * file)
{
    PdfReferencePointer pointer;
    if (index <= array_.size() && file)
    {
        pointer = PdfReference::Create(0, 0, file, GetAllocator());
        array_[index] = pointer;
        SetModified(true);
    }
    return pointer;
}

void PdfArray::Clear()
{
    array_.clear();
    SetModified(true);
}

PdfArrayPointer PdfArray::Clone()
{
	PdfArrayPointer pointer;
	pointer.Reset(GetAllocator()->New<PdfArray>(GetAllocator()));
	if (pointer)
	{
		for (uint32_t index = 0; index < GetSize(); ++index)
		{
			PdfObjectPointer tmp_pointer = GetElement(index);
			if (tmp_pointer)
			{
				tmp_pointer = tmp_pointer->Clone();
                pointer->Append(tmp_pointer);
			}
		}
	}
	return pointer;
}

bool PdfArray::IsModified()
{
	if (b_modified_)
	{
		return true;
	}
    uint32_t count = (uint32_t)array_.size();
	for (uint32_t index = 0; index < count; ++index)
	{
		if (array_[index]->IsModified())
		{
			b_modified_ = true;
			return true;
		}
	}
	return false;
}

bool PdfArray::GetRect(PdfRect & rect) const
{
	if (GetSize() >= 4)
	{
        PdfObjectPointer object;
        PdfNumberPointer number;
		FLOAT llx = 0, lly = 0, rux = 0, ruy = 0;
		object = GetElement(0, OBJ_TYPE_NUMBER);
		if (object)
		{
			number = object->GetPdfNumber();
			llx = number->GetFloat();
		}
		object = GetElement(1, OBJ_TYPE_NUMBER);
		if (object)
		{
			number = object->GetPdfNumber();
			lly = number->GetFloat();
		}
		object = GetElement(2, OBJ_TYPE_NUMBER);
		if (object)
		{
			number = object->GetPdfNumber();
			rux = number->GetFloat();
		}
		object = GetElement(3, OBJ_TYPE_NUMBER);
		if (object)
		{
            number = object->GetPdfNumber();
			ruy = number->GetFloat();
		}
		rect.left = llx;
		rect.bottom = lly;
		rect.width = rux - llx;
		rect.height = ruy - lly;
		return true;
	}
	return false;
}

bool PdfArray::GetMatrix(PdfMatrix & matrix) const
{
	if (GetSize() >= 6)
	{
		FLOAT a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
		PdfObjectPointer object;
		PdfNumberPointer number;
		object = GetElement(0, OBJ_TYPE_NUMBER);
		if (object)
		{
			number = object->GetPdfNumber();
			a = number->GetFloat();
		}
		object = GetElement(1, OBJ_TYPE_NUMBER);
		if (object)
		{
			number = object->GetPdfNumber();
			b = number->GetFloat();
		}
		object = GetElement(2, OBJ_TYPE_NUMBER);
		if (object)
		{
			number = object->GetPdfNumber();
			c = number->GetFloat();
		}
		object = GetElement(3, OBJ_TYPE_NUMBER);
		if (object)
		{
			number = object->GetPdfNumber();
			d = number->GetFloat();
		}
		object = GetElement(4, OBJ_TYPE_NUMBER);
		if (object)
		{
			number = object->GetPdfNumber();
			e = number->GetFloat();
		}
		object = GetElement(5, OBJ_TYPE_NUMBER);
		if (object)
		{
			number = object->GetPdfNumber();
			f = number->GetFloat();
		}
		matrix.a = a;
		matrix.b = b;
		matrix.c = c;
		matrix.d = d;
		matrix.e = e;
		matrix.f = f;
		return true;
	}
	return false;
}

PdfDictionaryPointer PdfDictionary::Create(Allocator * allocator/*= nullptr*/)
{
	PdfDictionaryPointer pointer;
	if (allocator == nullptr)
	{
        allocator = Allocator::GetDefaultAllocator();
	}
	pointer.Reset(allocator->New<PdfDictionary>(allocator));
	return pointer;
}

PdfObjectPointer PdfDictionary::GetElement(const ByteString & key)const
{
    std::unordered_map<string,PdfObjectPointer>::const_iterator it = map_.find(std::string(key.GetData()));
	if (it != map_.end())
	{
		return it->second;
	}
	return PdfObjectPointer();
}

PdfObjectPointer PdfDictionary::GetElement(const ByteString & key, PDF_OBJ_TYPE type)
{
	PdfObjectPointer pointer = GetElement( key );
	if (!pointer)
	{
		return pointer;
	}
	PDF_OBJ_TYPE current_type = pointer->GetType();
	if (current_type == type || type == OBJ_TYPE_INVALID)
	{
		return pointer;
	}
	if (pointer->GetType() == OBJ_TYPE_REFERENCE)
	{
		pointer = pointer->GetPdfReference()->GetPdfObject(type);
		return pointer;
	}
	return PdfObjectPointer();
}

bool PdfDictionary::SetObject(const ByteString & key, const PdfObjectPointer & pointer)
{
	if (key.GetLength() > 0 && pointer)
	{
		map_[string(key.GetData())] = pointer;
		SetModified(true);
        return true;
	}
    return false;
}

PdfNullPointer PdfDictionary::SetNull(const ByteString & key)
{
    PdfNullPointer pointer;
	if (key.GetLength() > 0)
	{
 		pointer = PdfNull::Create(GetAllocator());
		map_[string(key.GetData())] = pointer;
		SetModified(true);
	}
    return pointer;
}

PdfBooleanPointer PdfDictionary::SetBoolean(const ByteString & key, bool value)
{
    PdfBooleanPointer pointer;
	if (key.GetLength() > 0)
	{
 		pointer = PdfBoolean::Create(value, GetAllocator());
		map_[string(key.GetData())] = pointer;
		SetModified(true);
	}
    return pointer;
}

PdfNumberPointer PdfDictionary::SetInteger(const ByteString & key, int32_t value)
{
    PdfNumberPointer pointer;
	if (key.GetLength() > 0)
	{
		pointer = PdfNumber::Create(value, GetAllocator());
		map_[string(key.GetData())] = pointer;
		SetModified(true);
	}
    return pointer;
}

PdfNumberPointer PdfDictionary::SetFloatNumber(const ByteString & key, FLOAT value)
{
    PdfNumberPointer pointer;
	if (key.GetLength() > 0)
	{
		pointer = PdfNumber::Create(value, GetAllocator());
		map_[string(key.GetData())] = pointer;
		SetModified(true);
	}
    return pointer;
}

PdfStringPointer PdfDictionary::SetString(const ByteString & key, const ByteString & str )
{
    PdfStringPointer pointer;
	if ( key.GetLength() > 0 )
	{
 		pointer = PdfString::Create( str, GetAllocator() );
		map_[string(key.GetData())] = pointer;
		SetModified(true);
	}
    return pointer;
}

PdfNamePointer	PdfDictionary::SetName(const ByteString & key, const ByteString & name)
{
    PdfNamePointer pointer;
	if (key.GetLength() > 0)
	{
		pointer = PdfName::Create(name, GetAllocator());
		map_[string(key.GetData())] = pointer;
		SetModified(true);
	}
    return pointer;
}

bool PdfDictionary::SetArray(const ByteString & key, const PdfArrayPointer & pointer)
{
	if (key.GetLength() > 0 && pointer)
	{
		map_[string(key.GetData())] = pointer;
		SetModified(true);
        return true;
	}
    return false;
}

PdfArrayPointer PdfDictionary::SetArray(const ByteString & key)
{
    PdfArrayPointer pointer;
    if (key.GetLength() > 0)
    {
        pointer = PdfArray::Create(GetAllocator());
        map_[string(key.GetData())] = pointer;
        SetModified(true);
    }
    return pointer;
}

bool PdfDictionary::SetDictionary(const ByteString & key, const PdfDictionaryPointer & pointer)
{
	if (key.GetLength() > 0 && pointer)
	{
		map_[string(key.GetData())] = pointer;
		SetModified(true);
        return true;
	}
    return false;
}

PdfDictionaryPointer PdfDictionary::SetDictionary(const ByteString & key)
{
    PdfDictionaryPointer pointer;
    if (key.GetLength() > 0)
    {
        pointer = PdfDictionary::Create(GetAllocator());
        map_[string(key.GetData())] = pointer;
        SetModified(true);
    }
    return pointer;
}

PdfReferencePointer PdfDictionary::SetReference(const ByteString & key, uint32_t object_number, uint32_t generate_number, PdfFile * file)
{
    PdfReferencePointer pointer;
	if (key.GetLength() > 0)
	{
        pointer = PdfReference::Create(object_number, generate_number, file, GetAllocator());
		map_[string(key.GetData())] = pointer;
		SetModified(true);
	}
    return pointer;
}

PdfDictionaryPointer PdfDictionary::Clone()
{
	PdfDictionaryPointer pointer;
	pointer.Reset(GetAllocator()->New<PdfDictionary>(GetAllocator()));
	if (pointer)
	{
		unordered_map<string,PdfObjectPointer>::iterator it;
		for (it = map_.begin(); it != map_.end(); ++it)
		{
			pointer->SetObject(it->first.c_str(), it->second);
		}
	}
	return pointer;
}

void PdfDictionary::Clear()
{
    map_.clear();
    iterator_ = map_.end();
    SetModified(true);
}

bool PdfDictionary::IsModified()
{
	if (b_modified_)
	{
		return true;
	}
	unordered_map<string,PdfObjectPointer>::iterator it;
	for (it = map_.begin(); it != map_.end(); ++it)
	{
		if (it->second->IsModified())
		{
			b_modified_ = true;
			return true;
		}
	}
	return false;
}

bool PdfDictionary::CheckName(const ByteString & key, const ByteString & name, bool b_required/*= true*/)
{
	PdfObjectPointer object = GetElement(key, OBJ_TYPE_NAME);
	if (object)
	{
		PdfNamePointer nameObject = object->GetPdfName();
		if (nameObject->GetString() == name)
		{
			return true;
		}
		return false;
	}else if (!b_required)
	{
		return true;
	}
	return false;
}

void PdfDictionary::MoveToFirst()
{
    iterator_ = map_.begin();
}

bool PdfDictionary::GetKeyAndElement(ByteString & key, PdfObjectPointer & object)
{
    if (iterator_ != map_.end())
    {
        key = iterator_->first.data();
        object = iterator_->second;
		++iterator_;
        return true;
    }
    return false;
}

bool PdfDictionary::Remove(const ByteString & key)
{
    unordered_map<string,PdfObjectPointer>::iterator it = map_.find(key.GetData());
    if (it != map_.end() )
    {
        if (iterator_ == it)
        {
            ++iterator_;
        }
        map_.erase(it);
        return true;
    }
    return false;
}

PdfStreamPointer PdfStream::Create(uint32_t object_number, uint32_t generate_number, PdfCrypto * crypto/*= nullptr*/, Allocator * allocator/*= nullptr*/)
{
	PdfStreamPointer pointer;
	if (allocator == nullptr)
	{
        allocator = Allocator::GetDefaultAllocator();
	}
	pointer.Reset(allocator->New<PdfStream>(object_number, generate_number, crypto, allocator));
	return pointer;
}

PdfStreamPointer PdfStream::Create(uint8_t * data, size_t size, const PdfDictionaryPointer & dictionary, uint32_t object_number,
                                   uint32_t generate_number, PdfCrypto * crypto/*= nullptr*/, Allocator * allocator/*= nullptr*/)
{
	PdfStreamPointer pointer;
	if (allocator == nullptr)
	{
        allocator = Allocator::GetDefaultAllocator();
	}
	pointer.Reset(allocator->New<PdfStream>(data, size, dictionary, object_number, generate_number, crypto, allocator));
	return pointer;
}

PdfStreamPointer PdfStream::Create(IRead* iread, size_t offset, size_t size, const PdfDictionaryPointer & dictionary, uint32_t object_number,
                                   uint32_t generate_number, PdfCrypto * crypto/*= nullptr*/, Allocator * allocator/*= nullptr*/)
{
	PdfStreamPointer pointer;
	if (allocator == nullptr)
	{
        allocator = Allocator::GetDefaultAllocator();
	}
	pointer.Reset(allocator->New<PdfStream>(iread, offset, size, dictionary, object_number, generate_number, crypto, allocator));
	return pointer;
}

PdfStream::PdfStream(uint8_t * data, size_t size, const PdfDictionaryPointer & dictionary,
                     uint32_t object_number, uint32_t generate_number, PdfCrypto * crypto, Allocator * allocator)
    : PdfObject(OBJ_TYPE_STREAM, allocator), crypto_(crypto), b_memory_stream(true),
    data_(nullptr), size_(size), file_offset_(0), object_number_(object_number), generate_number_(generate_number)
{
	if (data != nullptr && size != 0)
	{
		data_ = GetAllocator()->NewArray<uint8_t>(size);
		memcpy(data_, data, size);
	}
	if (dictionary)
	{
		dictionary_ = dictionary;
	}else{
		dictionary_ = PdfDictionary::Create(GetAllocator());
	}
	dictionary_->SetInteger("Length", (int32_t)size);
}

PdfStream::PdfStream(IRead* iread, size_t offset, size_t size, const PdfDictionaryPointer & dictionary,
                     uint32_t object_number, uint32_t genarate_number, PdfCrypto * crypto, Allocator * allocator)
    : PdfObject(OBJ_TYPE_STREAM, allocator), crypto_(crypto), b_memory_stream(false), iread_(nullptr), size_(size),
    file_offset_(offset), object_number_(object_number), generate_number_(genarate_number)
{
	if (dictionary)
	{
		dictionary_ = dictionary;
	}else{
		dictionary_ = PdfDictionary::Create(GetAllocator());
	}
	dictionary_->SetInteger("Length", (uint32_t)size);
}

PdfStream::PdfStream(uint32_t object_number, uint32_t generate_number, PdfCrypto * crypto, Allocator * allocator)
    : PdfObject(OBJ_TYPE_STREAM, allocator), crypto_(crypto), b_memory_stream(true), data_(nullptr), size_(0),
    file_offset_(0), object_number_(object_number), generate_number_(generate_number)
{
	dictionary_ = PdfDictionary::Create(GetAllocator());
}

PdfStream::~PdfStream()
{
 	if (b_memory_stream == true && data_)
 	{
		GetAllocator()->DeleteArray<uint8_t>(data_);
 	}
}

PdfStreamPointer PdfStream::Clone()
{
	PdfStreamPointer stream;
	PdfDictionaryPointer dictionary;
	if (dictionary_)
	{
		dictionary = dictionary_->Clone();
	}
	if (b_memory_stream)
	{
		uint8_t * data = GetAllocator()->NewArray<uint8_t>(size_);
		stream = PdfStream::Create(data, size_, dictionary, object_number_, generate_number_, crypto_, GetAllocator());
	}else
	{
		stream = PdfStream::Create(iread_, file_offset_, size_, dictionary, object_number_, generate_number_, crypto_, GetAllocator());
	}
	return stream;
}

bool PdfStream::IsModified()
{
	if (b_modified_ || dictionary_->IsModified())
	{
		b_modified_ = true;
		return true;
	}
	return false;
}

void PdfStream::SetDictionary(const PdfDictionaryPointer & dictionary)
{
	dictionary_ = dictionary;
	SetModified(true);
}

bool PdfStream::SetRawData(uint8_t * data, size_t size, uint8_t filter/* = STREAM_FILTER_NULL*/)
{
	if (data == nullptr || size == 0)
	{
		return false;
	}
	if (b_memory_stream == false)
	{
		b_memory_stream = true;
		iread_ = nullptr;
	}else{
		if (data_ != nullptr)
		{
			GetAllocator()->DeleteArray<uint8_t>(data_);
			data_ = nullptr;
			size_ = 0;
		}
	}
	if (!dictionary_)
	{
		dictionary_ = PdfDictionary::Create(GetAllocator());
	}
	switch (filter)
	{
	case STREAM_FILTER_NULL:
		{
			data_ = GetAllocator()->NewArray<uint8_t>(size);
			memcpy(data_, data, size);
			size_ = size;
            dictionary_->Remove("Filter");
			break;
		}
	case STREAM_FILTER_HEX:
		{
            Buffer buffer;
            PdfHexFilter filter(GetAllocator());
            filter.Encode(data, size, buffer);
            if (buffer.GetSize() > 0)
            {
                data_ = GetAllocator()->NewArray<uint8_t>(buffer.GetSize());
                size_ = buffer.GetSize();
                memcpy(data_, buffer.GetData(), buffer.GetSize());
                dictionary_->SetName( "Filter", "ASCIIHexDecode" );
            }
			break;
		}
	case STREAM_FILTER_ASCII85:
		{
            Buffer buffer;
            PdfASCII85Filter filter(GetAllocator());
            filter.Encode(data, size, buffer);
            if (buffer.GetSize() > 0)
            {
                data_ = GetAllocator()->NewArray<uint8_t>(buffer.GetSize());
                size_ = buffer.GetSize();
                memcpy(data_, buffer.GetData(), buffer.GetSize());
                dictionary_->SetName( "Filter", "ASCII85Decode" );
            }
			break;
		}
	case STREAM_FILTER_FLATE:
		{
            Buffer buffer;
            PdfFlateFilter filter(GetAllocator());
            filter.Encode(data, size, buffer);
            if (buffer.GetSize() > 0)
            {
                data_ = GetAllocator()->NewArray<uint8_t>(buffer.GetSize());
                size_ = buffer.GetSize();
                memcpy(data_, buffer.GetData(), buffer.GetSize());
                dictionary_->SetName( "Filter", "FlateDecode" );
            }
			break;
		}
	case STREAM_FILTER_LZW:
		{
            Buffer buffer;
            PdfLZWFilter filter(GetAllocator());
            filter.Encode(data, size, buffer);
            if (buffer.GetSize() > 0)
            {
                data_ = GetAllocator()->NewArray<uint8_t>(buffer.GetSize());
                size_ = buffer.GetSize();
                memcpy(data_, buffer.GetData(), buffer.GetSize());
                dictionary_->SetName( "Filter", "LZWDecode" );
            }
			break;
		}
	case STREAM_FILTER_RLE:
		{
            Buffer buffer;
            PdfRLEFileter filter(GetAllocator());
            filter.Encode(data, size, buffer);
            if (buffer.GetSize() > 0)
            {
                data_ = GetAllocator()->NewArray<uint8_t>(buffer.GetSize());
                size_ = buffer.GetSize();
                memcpy(data_, buffer.GetData(), buffer.GetSize());
                dictionary_->SetName( "Filter", "RunLengthDecode" );
            }
			break;
		}
	default:
		break;
	}
	
	dictionary_->SetInteger("Length", (uint32_t)size_);
	if (crypto_ && crypto_->IsPasswordOK())
	{
		crypto_->Decrypt(data_, (uint32_t)size_, GetObjectNumber(), GetGenerateNumber());
	}
	SetModified(true);
	return true;
}

size_t PdfStream::GetRawData(size_t offset, uint8_t * buffer, size_t buffer_size) const
{
	if (buffer == nullptr || buffer_size == 0 || offset >= size_)
 	{
 		return 0;
 	}
    size_t size_return = 0;
	if (b_memory_stream == false)
	{
		size_return = iread_->ReadBlock(buffer, offset + file_offset_, buffer_size);
		if (crypto_ && crypto_->IsPasswordOK())
		{
			crypto_->Decrypt(buffer, (uint32_t)buffer_size, GetObjectNumber(), GetGenerateNumber());
		}
		return size_return;
	}else{
		if (offset + buffer_size > size_)
 		{
			size_return = size_ - offset;
 		}
		memcpy(buffer, data_ + offset, size_return);
		if (crypto_ && crypto_->IsPasswordOK())
		{
			crypto_->Decrypt(buffer, (uint32_t)buffer_size, GetObjectNumber(), GetGenerateNumber());
		}
 		return size_return;
	}
}

PdfStreamAccess::PdfStreamAccess(Allocator * allocator) : BaseObject(allocator), data_(nullptr), size_(0) {}

PdfStreamAccess::~PdfStreamAccess()
{
	if (data_)
	{
		GetAllocator()->DeleteArray<uint8_t>(data_);
	}
	size_ = 0;
	stream_.Reset();
}

bool PdfStreamAccess::Attach(const PdfStreamPointer & stream, PDF_STREAM_DECODE_MODE mode)
{
	if (!stream)
	{
		return false;
	}
	if (stream_)
	{
		Detach();
	}
    stream_ = stream;

    bool result = true;
	PdfDictionaryPointer dictionary = stream->GetDictionary();
	if (dictionary)
	{
		uint32_t filter_count = 0;
		size_t size = stream->GetRawSize();
		PdfObjectPointer filter = dictionary->GetElement("Filter");
		PdfObjectPointer params = dictionary->GetElement("DecodeParms");
		if (!filter)
		{
			size_ = size;
			data_ = GetAllocator()->NewArray<uint8_t>(size_);
			stream->GetRawData(0, data_, size_);
			return true;
		}
		if (filter->GetType() == OBJ_TYPE_ARRAY)
		{
			filter_count = filter->GetPdfArray()->GetSize();
		}else{
			filter_count = 1;
		}
		PdfNamePointer * filter_name_array = nullptr;
		PdfDictionaryPointer * param_dictionary_array = nullptr;
        filter_name_array = GetAllocator()->NewArray<PdfNamePointer>(filter_count);
        param_dictionary_array = GetAllocator()->NewArray<PdfDictionaryPointer>(filter_count);
		if (filter->GetType() == OBJ_TYPE_NAME)
		{
			filter_name_array[0] = filter->GetPdfName();
		}else if (filter->GetType() == OBJ_TYPE_ARRAY)
		{
			PdfArrayPointer tmp_array = filter->GetPdfArray();
			for (uint32_t index = 0; index < filter_count; ++index)
			{
				filter_name_array[index] = tmp_array->GetElement(index)->GetPdfName();
			}
		}else{
			GetAllocator()->DeleteArray<PdfNamePointer>(filter_name_array);
			GetAllocator()->DeleteArray<PdfDictionaryPointer>(param_dictionary_array);
			return false;
		}
		if (params && params->GetType() == OBJ_TYPE_DICTIONARY)
		{
			param_dictionary_array[0] = params->GetPdfDictionary();
		}else if (params && params->GetType() == OBJ_TYPE_ARRAY)
		{
			uint32_t param_count = params->GetPdfArray()->GetSize();
			for (uint32_t index = 0; index < param_count; ++index)
			{
				param_dictionary_array[index] = params->GetPdfArray()->GetElement(index)->GetPdfDictionary();
			}
		}

		size_t buffer_size = (size == 0) ? 1024 : size;
		ByteString filter_name(GetAllocator());
		Buffer buffer(buffer_size * 2, buffer_size, GetAllocator());
		size_t data_size = stream->GetRawSize();
		uint8_t * tmp_data = GetAllocator()->NewArray<uint8_t>(data_size);
		stream->GetRawData(0, tmp_data, data_size);
		for (size_t index = 0; index < filter_count; ++index)
		{
			if (mode == STREAM_DECODE_NOTLASTFILTER)
			{
				if (index + 1 == filter_count)
				{
					break;
				}
			}

			filter_name = filter_name_array[index]->GetPdfName()->GetString();
			if (filter_name == "ASCIIHexDecode" || filter_name == "AHx")
			{
				PdfHexFilter filter(GetAllocator());
				filter.Decode(tmp_data, data_size, buffer);
			}else if (filter_name == "ASCII85Decode" || filter_name == "A85")
			{
                PdfASCII85Filter filter(GetAllocator());
				filter.Decode(tmp_data, data_size, buffer);
			}else if (filter_name == "LZWDecode" || filter_name == "LZW")
			{
				PdfDictionaryPointer decode_params = param_dictionary_array[index];
				if (!decode_params)
				{
					PdfLZWFilter filter(GetAllocator());
					filter.Decode(tmp_data, data_size, buffer);
				}else{
					PdfFilterPredictor predictor(decode_params, GetAllocator());
					PdfLZWFilter filter(GetAllocator());
					filter.Decode(tmp_data, data_size, buffer);
					data_size = buffer.GetSize();
					GetAllocator()->DeleteArray<uint8_t>(tmp_data);
					tmp_data = GetAllocator()->NewArray<uint8_t>(data_size);
					buffer.Read(tmp_data, data_size);
					buffer.Clear();
					predictor.Decode(tmp_data, data_size, buffer);
				}
			}else if (filter_name == "FlateDecode" || filter_name == "Fl")
			{
				PdfDictionaryPointer decode_params = param_dictionary_array[index];
				if (!decode_params)
				{
					PdfFlateFilter filter(GetAllocator());
					filter.Decode(tmp_data, data_size, buffer);
				}else{
					PdfFilterPredictor predictor(decode_params, GetAllocator());
					PdfFlateFilter filter(GetAllocator());
                    filter.Decode(tmp_data, data_size, buffer);
                    data_size = buffer.GetSize();
                    GetAllocator()->DeleteArray<uint8_t>(tmp_data);
                    tmp_data = GetAllocator()->NewArray<uint8_t>(data_size);
                    buffer.Read(tmp_data, data_size);
                    buffer.Clear();
                    predictor.Decode(tmp_data, data_size, buffer);
				}
			}else if (filter_name == "RunLengthDecode" || filter_name == "RL")
			{
				PdfRLEFileter filter(GetAllocator());
				filter.Decode(tmp_data, data_size, buffer);
			}else if (filter_name == "CCITTFaxDecode" || filter_name == "CCF")
			{
				PdfDictionaryPointer decode_params = param_dictionary_array[index];
				PdfFaxDecodeParams params(decode_params);
				PdfFaxFilter filter(&params, GetAllocator());
				filter.Decode(tmp_data, data_size, buffer);
			}else if (filter_name == "JBIG2Decode")
			{
				PdfJBig2Filter filter(GetAllocator());
				filter.Decode(tmp_data, data_size, buffer);
			}else if (filter_name == "DCTDecode" || filter_name == "DCT")
			{
				PdfDCTDFilter filter(GetAllocator());
				filter.Decode(tmp_data, data_size, buffer);
			}else if (filter_name == "JPXDecode")
			{
                PdfJPXFilter filter(GetAllocator());
                filter.Decode(tmp_data, data_size, buffer);
			}else if (filter_name == "Crypt")
			{
				result = false;
			}
			if (result == false)
			{
				GetAllocator()->DeleteArray<uint8_t>(tmp_data);
				GetAllocator()->DeleteArray<PdfNamePointer>(filter_name_array);
				GetAllocator()->DeleteArray<PdfDictionaryPointer>(param_dictionary_array);
				return false;
			}else{
				data_size = buffer.GetSize();
				GetAllocator()->DeleteArray<uint8_t>(tmp_data);
				tmp_data = GetAllocator()->NewArray<uint8_t>(data_size);
				buffer.Read(tmp_data, data_size);
			}
        }
        data_ = tmp_data;
        size_ = data_size;
        
		GetAllocator()->DeleteArray<PdfNamePointer>(filter_name_array);
		GetAllocator()->DeleteArray<PdfDictionaryPointer>(param_dictionary_array);
        return result;
	}else{
		size_ = stream_->GetRawSize();
		data_ = GetAllocator()->NewArray<uint8_t>(size_);
		stream_->GetRawData(0, data_, size_);
		return true;
	}
	return false;
}

void PdfStreamAccess::Detach()
{
	if (data_)
	{
		GetAllocator()->DeleteArray<uint8_t>(data_);
		data_ = nullptr;
	}
	stream_.Reset();
	size_ = 0;
}

bool IsPdfNull(PdfObject * object)
{
	if (object && object->GetType() == OBJ_TYPE_NULL)
	{
		return true;
	}
	return false;
}

bool IsPdfBoolean(PdfObject * object)
{
	if (object && object->GetType() == OBJ_TYPE_BOOLEAN)
	{
		return true;
	}
	return false;
}

bool IsPdfNumber(PdfObject * object)
{
	if (object && object->GetType() == OBJ_TYPE_NUMBER)
	{
		return true;
	}
	return false;
}

bool IsPdfName(PdfObject * object)
{
	if (object && object->GetType() == OBJ_TYPE_NAME)
	{
		return true;
	}
	return false;
}

bool IsPdfString(PdfObject * object)
{
	if (object && object->GetType() == OBJ_TYPE_STRING)
	{
		return true;
	}
	return false;
}

bool IsPdfDict(PdfObject * object)
{
	if (object && object->GetType() == OBJ_TYPE_DICTIONARY)
	{
		return true;
	}
	return false;
}

bool IsPdfArray(PdfObject * object)
{
	if (object && object->GetType() == OBJ_TYPE_ARRAY)
	{
		return true;
	}
	return false;
}

bool IsPdfRef(PdfObject * object)
{
	if (object && object->GetType() == OBJ_TYPE_REFERENCE)
	{
		return true;
	}
	return false;
}

bool IsPdfStream(PdfObject * object)
{
	if (object && object->GetType() == OBJ_TYPE_STREAM)
	{
		return true;
	}
	return false;
}

bool IsPdfNull(const PdfObjectPointer & object)
{
	if (object && object->GetType() == OBJ_TYPE_NULL)
	{
		return true;
	}
	return false;
}

bool IsPdfBoolean(const PdfObjectPointer & object)
{
	if (object && object->GetType() == OBJ_TYPE_BOOLEAN)
	{
		return true;
	}
	return false;
}

bool IsPdfNumber(const PdfObjectPointer & object)
{
	if (object && object->GetType() == OBJ_TYPE_NUMBER)
	{
		return true;
	}
	return false;
}

bool IsPdfName(const PdfObjectPointer & object)
{
	if (object && object->GetType() == OBJ_TYPE_NAME)
	{
		return true;
	}
	return false;
}

bool IsPdfString(const PdfObjectPointer & object)
{
	if (object && object->GetType() == OBJ_TYPE_STRING)
	{
		return true;
	}
	return false;
}

bool IsPdfDictionary(const PdfObjectPointer & object)
{
	if (object && object->GetType() == OBJ_TYPE_DICTIONARY)
	{
		return true;
	}
	return false;
}

bool IsPdfArray(const PdfObjectPointer & object)
{
	if (object && object->GetType() == OBJ_TYPE_ARRAY)
	{
		return true;
	}
	return false;
}

bool IsPdfReference(const PdfObjectPointer & object)
{
	if (object && object->GetType() == OBJ_TYPE_REFERENCE)
	{
		return true;
	}
	return false;
}

bool IsPdfStream(const PdfObjectPointer & object)
{
	if (object && object->GetType() == OBJ_TYPE_STREAM)
	{
		return true;
	}
	return false;
}

}//namespace
