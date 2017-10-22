#ifndef _CHE_PDF_OBJECT_H_
#define _CHE_PDF_OBJECT_H_

#include <string>
#include <vector>
#include <cfloat>
#include <unordered_map>

#include "che_base_string.h"

namespace chepdf {
    
enum PDF_OBJ_TYPE
{
    OBJ_TYPE_INVALID		= 0x00,
    OBJ_TYPE_NULL			= 0x01,
    OBJ_TYPE_BOOLEAN		= 0x02,
    OBJ_TYPE_NUMBER			= 0x03,
    OBJ_TYPE_STRING			= 0x04,
    OBJ_TYPE_NAME			= 0x05,
    OBJ_TYPE_ARRAY			= 0x06,
    OBJ_TYPE_DICTIONARY		= 0x07,
    OBJ_TYPE_STREAM			= 0x08,
    OBJ_TYPE_REFERENCE		= 0x09
};


enum PDF_STREAM_FILTER
{
    STREAM_FILTER_NULL		= 0x00,
    STREAM_FILTER_HEX		= 0x01,
    STREAM_FILTER_ASCII85	= 0x02,
    STREAM_FILTER_FLATE		= 0x03,
    STREAM_FILTER_LZW		= 0x04,
    STREAM_FILTER_RLE		= 0x05
};


class PdfObject;
class PdfNull;
class PdfBoolean;
class PdfNumber;
class PdfString;
class PdfName;
class PdfReference;
class PdfArray;
class PdfDictionary;
class PdfStream;
class PdfFile;
class PdfCrypto;

class PdfObjectPointer;
class PdfNullPointer;
class PdfBooleanPointer;
class PdfNumberPointer;
class PdfStringPointer;
class PdfNamePointer;
class PdfReferencePointer;
class PdfArrayPointer;
class PdfDictionaryPointer;
class PdfStreamPointer;


bool IsPdfNull( PdfObject * pObj );
bool IsPdfBoolean( PdfObject * pObj );
bool IsPdfNumber( PdfObject * pObj );
bool IsPdfName( PdfObject * pObj );
bool IsPdfString( PdfObject * pObj );
bool IsPdfDict( PdfObject * pObj );
bool IsPdfArray( PdfObject * pObj );
bool IsPdfRef( PdfObject * pObj );
bool IsPdfStream( PdfObject * pObj );
bool IsPdfNull( const PdfObjectPointer & objPtr );
bool IsPdfBoolean( const PdfObjectPointer & objPtr );
bool IsPdfNumber( const PdfObjectPointer & objPtr );
bool IsPdfName( const PdfObjectPointer & objPtr );
bool IsPdfString( const PdfObjectPointer & objPtr );
bool IsPdfDictionary( const PdfObjectPointer & objPtr );
bool IsPdfArrayPointer( const PdfObjectPointer & objPtr );
bool IsPdfRefPointer( const PdfObjectPointer & objPtr );
bool IsPdfStreamPointer( const PdfObjectPointer & objPtr );


class PdfObject : public BaseObject
{
public:
    PDF_OBJ_TYPE        GetType() const { return type_; };
    
    PdfObjectPointer	Clone();
    void                Release();
    
    void                SetModified(bool value);
    virtual	bool		IsModified();
    
    PdfNullPointer        GetPdfNull() const;
    PdfBooleanPointer     GetPdfBoolean() const;
    PdfNumberPointer      GetPdfNumber() const;
    PdfStringPointer      GetPdfString() const;
    PdfNamePointer        GetPdfName() const;
    PdfArrayPointer       GetPdfArray() const;
    PdfDictionaryPointer  GetPdfDictionary() const;
    PdfReferencePointer   GetPdfReference() const;
    PdfStreamPointer      GetPdfStream() const;
    
protected:
    PdfObject( PDF_OBJ_TYPE type, Allocator * allocator = nullptr );
    virtual ~PdfObject() {}
    
    bool                b_modified_;
    PDF_OBJ_TYPE        type_;
    ReferenceCount      referenceCount_;
    
    friend class Allocator;
    friend class PdfObjectPointer;
};

class PdfObjectPointer
{
public:
    PdfObjectPointer() : object_( nullptr ) {}
    PdfObjectPointer( const PdfObjectPointer & pointer );
    ~PdfObjectPointer();
    
    PdfObjectPointer operator=( const PdfObjectPointer & pointer );
    bool operator!() const { return object_ ? false : true; }
    operator bool() const { return object_ ? true : false; }
    PdfObject * operator->() const { return object_; }
    
    void Reset( PdfObject * object = nullptr );
    
protected:
    PdfObject * object_;
};

class PdfNullPointer : public PdfObjectPointer
{
public:
    PdfNull * operator->() const { return (PdfNull*)( object_ ); }
};

class PdfBooleanPointer : public PdfObjectPointer
{
public:
    PdfBoolean * operator->() const { return (PdfBoolean*)( object_ ); }
};

class PdfNumberPointer : public PdfObjectPointer
{
public:
    PdfNumber * operator->() const { return (PdfNumber*)( object_ ); }
};

class PdfStringPointer : public PdfObjectPointer
{
public:
    PdfString * operator->() const { return (PdfString*)( object_ ); }
};

class PdfNamePointer : public PdfObjectPointer
{
public:
    PdfName * operator->() const { return (PdfName*)( object_ ); }
};

class PdfReferencePointer : public PdfObjectPointer
{
public:
    PdfReference * operator->() const { return (PdfReference*)( object_ ); }
};

class PdfArrayPointer : public PdfObjectPointer
{
public:
    PdfArray * operator->() const { return (PdfArray*)( object_ ); }
};

class PdfDictionaryPointer : public PdfObjectPointer
{
public:
    PdfDictionary * operator->() const { return (PdfDictionary*)( object_ ); }
};

class PdfStreamPointer : public PdfObjectPointer
{
public:
    PdfStream * operator->() const { return (PdfStream*)( object_ ); }
};

class PdfNull : public PdfObject
{
public:
    static PdfNullPointer Create( Allocator * allocator = nullptr );
    
    PdfNullPointer Clone();
    
private:
    PdfNull( Allocator * allocator = nullptr ) : PdfObject(OBJ_TYPE_NULL, allocator) {}
    
    friend class Allocator;
    friend class PdfObject;
};



class PdfPoint
{
public:
    PdfPoint() : x(0), y(0) {}
    PdfPoint(FLOAT vx, FLOAT vy) : x(vx), y(vy) {}
    
    FLOAT x;
    FLOAT y;
};

class PdfRect
{
public:
    PdfRect() : left(0), bottom(0), width(0), height(0) {}
    PdfRect(FLOAT vleft, FLOAT vbottom, FLOAT vwidth, FLOAT vheight)
    : left(vleft), bottom(vbottom), width(vwidth), height(vheight) {}
    
    bool IsEmpty() const
    {
        return (fabs(width) < FLT_EPSILON || fabs(height) < FLT_EPSILON);
    }
    
    bool IsUnion(const PdfRect & rect)
    {
        PdfPoint p1;
        PdfPoint p2;
        p1.x = left + width / 2;
        p1.y = bottom + height / 2;
        p2.x = rect.left + rect.width / 2;
        p2.y = rect.bottom + rect.height / 2;
        FLOAT xDis = fabsf( p1.x - p2.x );
        FLOAT yDis = fabsf( p1.y - p2.y );
        if ( ( xDis < ( rect.width + width ) / 2 ) && ( yDis < ( rect.height + height ) / 2 ) )
        {
            return true;
        }
        return false;
    }
    
    void Union(const PdfRect & rect)
    {
        if (&rect != this)
        {
            if (IsEmpty())
            {
                *this = rect;
                return;
            }
            if (!rect.IsEmpty())
            {
                PdfRect tmpRect;
                tmpRect.left = ( left <= rect.left ) ? left : rect.left;
                tmpRect.bottom = ( bottom <= rect.bottom ) ? bottom : rect.bottom;
                tmpRect.width = ( left + width >= rect.left + rect.width ) ? left + width : rect.left + rect.width;
                tmpRect.width -= tmpRect.left;
                tmpRect.height = ( bottom + height >= rect.bottom + rect.height ) ? bottom + height : rect.bottom + rect.height;
                tmpRect.height -= tmpRect.bottom;
                *this = tmpRect;
            }
        }
    }
    
    bool operator==(const PdfRect & rect) const
    {
        if (fabsf( left - rect.left ) <= FLT_EPSILON &&
            fabsf( bottom - rect.bottom ) <= FLT_EPSILON &&
            fabsf( width - rect.width ) <= FLT_EPSILON &&
            fabsf( height - rect.height ) <= FLT_EPSILON )
        {
            return true;
        }
        return false;
    }
    
    bool operator!=( const PdfRect & rect ) const
    {
        return ! operator==( rect );
    }
    
    FLOAT	left;
    FLOAT	bottom;
    FLOAT	width;
    FLOAT	height;
};

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FMAX4(a,b,c,d) fmax(fmax(a,b), fmax(c,d))
#define FMIN4(a,b,c,d) fmin(fmin(a,b), fmin(c,d))

static inline float fmin(float a, float b)
{
    return (a < b ? a : b);
}
static inline float fmax(float a, float b)
{
    return (a > b ? a : b);
}

class PdfMatrix
{
public:
    PdfMatrix() : a(1), b(0), c(0), d(1), e(0), f(0) {}
    PdfMatrix(float va, float vb, float vc, float vd, float ve, float vf)
    : a(va), b(vb), c(vc), d(vd), e(ve), f(vf) {}
    
    static PdfMatrix TranslateMatrix(FLOAT tx, FLOAT ty)
    {
        PdfMatrix matrixRet;
        matrixRet.e = tx;
        matrixRet.f = ty;
        return matrixRet;
    }
    
    static PdfMatrix RotateMatrix(FLOAT theta)
    {
        float s;
        float c;
        PdfMatrix matrixRet;
        
        while (theta < 0)
            theta += 360;
        while (theta >= 360)
            theta -= 360;
        
        if (fabsf(0 - theta) < FLT_EPSILON)
        {
            s = 0;
            c = 1;
        }
        else if (fabsf(90.0f - theta) < FLT_EPSILON)
        {
            s = 1;
            c = 0;
        }
        else if (fabsf(180.0f - theta) < FLT_EPSILON)
        {
            s = 0;
            c = -1;
        }
        else if (fabsf(270.0f - theta) < FLT_EPSILON)
        {
            s = -1;
            c = 0;
        }
        else
        {
            s = sinf(theta * (float)M_PI / 180);
            c = cosf(theta * (float)M_PI / 180);
        }
        
        matrixRet.a = c; matrixRet.b = s;
        matrixRet.c = -s; matrixRet.d = c;
        matrixRet.e = 0; matrixRet.f = 0;
        return matrixRet;
    }
    
    static PdfMatrix ScaleMatrix(FLOAT sx, FLOAT sy)
    {
        PdfMatrix matrixRet;
        matrixRet.a = sx;
        matrixRet.d = sy;
        return matrixRet;
    }
    
    static PdfMatrix SkewMatrix(FLOAT a, FLOAT b)
    {
        PdfMatrix matrixRet;
        matrixRet.b = tanf(a);
        matrixRet.c = tanf(b);
        return matrixRet;
    }
    
    void Concat(const PdfMatrix & matrix)
    {
        PdfMatrix tmpMatrix = *this;
        a = tmpMatrix.a * matrix.a +tmpMatrix. b * matrix.c;
        b = tmpMatrix.a * matrix.b + tmpMatrix.b * matrix.d;
        c = tmpMatrix.c * matrix.a + tmpMatrix.d * matrix.c;
        d = tmpMatrix.c * matrix.b + tmpMatrix.d * matrix.d;
        e = tmpMatrix.e * matrix.a + tmpMatrix.f * matrix.c + matrix.e;
        f = tmpMatrix.e * matrix.b + tmpMatrix.f * matrix.d + matrix.f;
    }
    
    void Invert(const PdfMatrix & matirx)
    {
        float rdet = matirx.AbsValue();
        a = matirx.d * rdet;
        b = - matirx.b * rdet;
        c = - matirx.c * rdet;
        d = matirx.a * rdet;
        e = - matirx.e * a - matirx.f * c;
        f = - matirx.e * b - matirx.f * d;
    }
    
    FLOAT AbsValue() const
    {
        FLOAT val = 1 / ( a * d - b * c );
        return val;
    }
    
    PdfPoint Transform(const PdfPoint & point) const
    {
        PdfPoint pointRet;
        pointRet.x = point.x * a + point.y * c + e;
        pointRet.y = point.x * b + point.y * d + f;
        return pointRet;
    }
    
    PdfRect Transform(const PdfRect & rect) const
    {
        PdfPoint s, t, u, v;
        PdfRect retRect;
        
        s.x = rect.left;
        s.y = rect.bottom;
        t.x = rect.left;
        t.y = rect.bottom + rect.height;
        u.x = rect.left + rect.width;
        u.y = rect.bottom + rect.height;
        v.x = rect.left + rect.width;
        v.y = rect.bottom;
        s = Transform( s );
        t = Transform( t );
        u = Transform( u );
        v = Transform( v );
        
        retRect.left = FMIN4(s.x, t.x, u.x, v.x);
        retRect.bottom = FMIN4(s.y, t.y, u.y, v.y);
        retRect.width = FMAX4(s.x, t.x, u.x, v.x) - FMIN4(s.x, t.x, u.x, v.x);
        retRect.height = FMAX4(s.y, t.y, u.y, v.y) - FMIN4(s.y, t.y, u.y, v.y);
        return retRect;
    }
    
    bool operator==(const PdfMatrix & matrix) const
    {
        if (fabsf( a - matrix.a ) <= FLT_EPSILON &&
            fabsf( b - matrix.b ) <= FLT_EPSILON &&
            fabsf( c - matrix.c ) <= FLT_EPSILON &&
            fabsf( d - matrix.d ) <= FLT_EPSILON &&
            fabsf( e - matrix.e ) <= FLT_EPSILON &&
            fabsf( f - matrix.f ) <= FLT_EPSILON)
        {
            return true;
        }
        return false;
    }
    
    bool operator!=(const PdfMatrix & matrix) const
    {
        return !operator==(matrix);
    }
    
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;
};


class PdfBoolean : public PdfObject
{
public:
    static PdfBooleanPointer Create(bool value, Allocator * allocator = nullptr);
    
    PdfBooleanPointer Clone();
    
    bool GetValue() { return value_; }
    void SetValue(bool value) { value_ = value; SetModified( true ); }
    
private:
    PdfBoolean(Allocator * allocator = nullptr)
    : PdfObject(OBJ_TYPE_BOOLEAN, allocator), value_(false) {}
    
    PdfBoolean(bool value, Allocator * allocator = nullptr)
    : PdfObject(OBJ_TYPE_BOOLEAN, allocator), value_( value ) {}
    
    bool value_;
    
    friend class Allocator;
    friend class PdfObject;
};

class PdfNumber : public PdfObject
{
public:
    static PdfNumberPointer Create(int32_t value,  Allocator * allocator = nullptr);
    static PdfNumberPointer Create(FLOAT value, Allocator * allocator = nullptr);
    
    PdfNumberPointer Clone();
    
    bool IsInteger() const { return b_integer_; }
    
    int32_t GetInteger() const { return b_integer_ ? interger_ : (int32_t)float_; }
    FLOAT GetFloat() const { return b_integer_ ? (FLOAT)interger_ : float_; }
    
    void SetValue(int32_t value) { b_integer_ = true; interger_ = value; SetModified(true); }
    void SetValue(FLOAT value) { b_integer_ = false; float_ = value; SetModified(true); }
    
private:
    PdfNumber(int32_t value, Allocator * allocator = nullptr)
    : PdfObject(OBJ_TYPE_NUMBER, allocator ), b_integer_(true), interger_(value) {}
    
    PdfNumber(FLOAT value, Allocator * allocator = nullptr)
    : PdfObject(OBJ_TYPE_NUMBER, allocator), b_integer_(false), float_(value) {}
    
    bool b_integer_;
    union {
        int32_t interger_;
        FLOAT float_;
    };
    
    friend class Allocator;
    friend class PdfObject;
};

class PdfString : public PdfObject
{
public:
    static PdfStringPointer Create(const ByteString & str, Allocator * allocator = nullptr);
    
    PdfStringPointer Clone();
    
    ByteString & GetString();
    void SetString(ByteString & str);
    
private:
    PdfString(Allocator * allocator = nullptr)
    : PdfObject(OBJ_TYPE_STRING, allocator), string_(allocator ) {};
    
    PdfString(const ByteString & str, Allocator * allocator = nullptr)
    : PdfObject(OBJ_TYPE_STRING, allocator), string_(str) {}
    
    ByteString string_;
    
    friend class Allocator;
    friend class PdfObject;
};

class PdfName : public PdfObject
{
public:
    static PdfNamePointer Create(const ByteString & str, Allocator * allocator = nullptr);
    
    PdfNamePointer Clone();
    
    ByteString GetString() const { return name_; }
    void SetString(ByteString & name) { name_ = name; SetModified(true); }
    
private:
    PdfName(const ByteString & str,  Allocator * allocator = nullptr)
    : PdfObject(OBJ_TYPE_NAME, allocator), name_(str) {}
    
    ByteString name_;
    
    friend class Allocator;
    friend class PdfObject;
};

struct PDF_RefInfo
{
    uint32_t object_number;
    uint32_t generate_number;
};

class PdfReference : public PdfObject
{
public:
    static PdfReferencePointer Create(uint32_t object_number, uint32_t generate_number, PdfFile * file, Allocator * allocator = nullptr);
    
    PdfReferencePointer Clone();
    
    uint32_t GetReferenceNumber() const { return object_number_; }
    void SetReferenceNumber(uint32_t object_number) { object_number_ = object_number; SetModified(true); }
    uint32_t GetGenerateNumber() const { return generate_number_; }
    void SetGenerateNumber(uint32_t generate_number) { generate_number_ = generate_number; SetModified(true); }
    PDF_RefInfo GetRefInfo() { PDF_RefInfo refInfo; refInfo.object_number = object_number_; refInfo.generate_number = generate_number_; return refInfo; }
    void SetRefInfo(PDF_RefInfo refInfo) { object_number_ = refInfo.object_number; generate_number_ = refInfo.generate_number; SetModified(true); }
    
    PdfObjectPointer GetPdfObject();
    PdfObjectPointer GetPdfObject(PDF_OBJ_TYPE type);
    
    PdfFile * GetFile() const { return file_; }
    
private:
    PdfReference(uint32_t object_number, uint32_t generate_number, PdfFile * file, Allocator * allocator = nullptr )
    : PdfObject(OBJ_TYPE_REFERENCE, allocator), object_number_(object_number), generate_number_(generate_number), file_(file) {}
    
    uint32_t object_number_;
    uint32_t generate_number_;
    PdfFile * file_;
    
    friend class Allocator;
    friend class PdfObject;
};

class PdfArray : public PdfObject
{
public:
    static PdfArrayPointer Create(Allocator * allocator = nullptr);
    
    PdfArrayPointer Clone();
    
    bool IsModified();
    
    bool Append(const PdfObjectPointer & object);
    bool Replace(uint32_t index, const PdfObjectPointer & object);
    void Clear();
    
    PdfNullPointer        AppendNull();
    PdfBooleanPointer     AppendBoolean(bool val = false);
    PdfNumberPointer      AppendNumber();
    PdfNumberPointer      AppendNumber(int32_t val);
    PdfNumberPointer      AppendNumber(FLOAT val);
    PdfNamePointer        AppendName();
    PdfNamePointer        AppendName(ByteString & str);
    PdfStringPointer      AppendString();
    PdfStringPointer      AppendString(ByteString & str);
    PdfArrayPointer       AppendArray();
    PdfDictionaryPointer  AppendDictionary();
    PdfReferencePointer   AppendReference(PDF_RefInfo info, PdfFile * pFile);
    PdfReferencePointer   AppendReference(uint32_t objNum, uint32_t genNum, PdfFile * pFile);
    PdfReferencePointer   AppendReference(PdfReferencePointer & ref);
    PdfReferencePointer   AppendReference(PdfFile * pFile);
    
    PdfNullPointer        ReplaceNull(uint32_t index);
    PdfBooleanPointer     ReplaceBoolean(uint32_t index);
    PdfNumberPointer      ReplaceNumber(uint32_t index);
    PdfNamePointer        ReplaceName(uint32_t index);
    PdfStringPointer      ReplaceString(uint32_t index);
    PdfArrayPointer       ReplaceArray(uint32_t index);
    PdfDictionaryPointer  ReplaceDictionary(uint32_t index);
    PdfReferencePointer   ReplaceReference(uint32_t index, PdfFile * file);
    
    uint32_t GetSize() const { return (uint32_t)array_.size(); }
    PdfObjectPointer GetElement(uint32_t index ) const;
    PdfObjectPointer GetElement(uint32_t index, PDF_OBJ_TYPE type) const;
    PdfObjectPointer GetElementByType(PDF_OBJ_TYPE type);
    
    bool GetRect(PdfRect & rect) const;
    bool GetMatrix(PdfMatrix & maxtrix) const;
    
private:
    PdfArray(Allocator * allocator = nullptr) : PdfObject(OBJ_TYPE_ARRAY, allocator) {}
    
    std::vector<PdfObjectPointer> array_;
    
    friend class Allocator;
    friend class PdfObject;
};

class PdfDictionary : public PdfObject
{
public:
    static PdfDictionaryPointer Create(Allocator * allocator = nullptr);
    
    PdfDictionaryPointer Clone();
    void Clear();
    
    bool IsModified();
    
    bool                    SetObject(const ByteString & key, const PdfObjectPointer & object);
    PdfNullPointer          SetNull(const ByteString & key);
    PdfBooleanPointer       SetBoolean(const ByteString & key, bool value);
    PdfNumberPointer        SetInteger(const ByteString & key, int32_t value);
    PdfNumberPointer        SetFloatNumber(const ByteString & key, FLOAT value);
    PdfStringPointer        SetString(const ByteString & key, const ByteString & string);
    PdfNamePointer          SetName(const ByteString & key, const ByteString & name);
    bool                    SetArray(const ByteString & key, const PdfArrayPointer & array);
    PdfArrayPointer         SetArray(const ByteString & key);
    bool                    SetDictionary(const ByteString & key, const PdfDictionaryPointer & dictionary);
    PdfDictionaryPointer    SetDictionary(const ByteString & key);
    PdfReferencePointer     SetReference(const ByteString & key, uint32_t object_number, uint32_t generate_number, PdfFile * file);
    
    uint32_t GetCount() { return (uint32_t)map_.size(); }
    PdfObjectPointer GetElement(const ByteString & key) const;
    PdfObjectPointer GetElement(const ByteString & key, PDF_OBJ_TYPE type);
    
    bool Remove(const ByteString & key);
    
    void MoveToFirst();
    
    bool GetKeyAndElement(ByteString & key, PdfObjectPointer & object);
    
    bool CheckName(const ByteString & key, const ByteString & name, bool b_required = true);
    
private:
    PdfDictionary(Allocator * allocator = nullptr) : PdfObject(OBJ_TYPE_DICTIONARY, allocator) {}
    
    std::unordered_map<std::string,PdfObjectPointer>::iterator iterator_;
    std::unordered_map<std::string,PdfObjectPointer> map_;
    
    friend class Allocator;
    friend class PdfObject;
};

class PdfStream : public PdfObject
{
public:
    static PdfStreamPointer Create(uint32_t object_number, uint32_t generate_number,
                                   PdfCrypto * crypto = nullptr, Allocator * allocator = nullptr);
    
    static PdfStreamPointer Create(uint8_t * data, size_t size,
                                   const PdfDictionaryPointer & dictionary,
                                   uint32_t object_number, uint32_t generate_number,
                                   PdfCrypto * crypto = nullptr,
                                   Allocator * allocator = nullptr);
    
    static PdfStreamPointer Create(IRead* iread, size_t offset, size_t size,
                                   const PdfDictionaryPointer & dictionary,
                                   uint32_t object_number, uint32_t genarate_number,
                                   PdfCrypto * crypto = nullptr,
                                   Allocator * allocator = nullptr);
    
    PdfStreamPointer Clone();
    
    bool IsModified();
    
    uint32_t GetObjectNumber() const { return object_number_; }
    uint32_t GetGenerateNumber() const { return generate_number_; }
    
    void SetDictionary(const PdfDictionaryPointer & dictionary);
    PdfDictionaryPointer GetDictionary() const { return dictionary_; }
    
    size_t GetRawSize() const { return size_; }
    size_t GetRawData(size_t offset, uint8_t * buffer, size_t buffer_size) const;
    bool SetRawData(uint8_t * data, size_t data_size, uint8_t filter = STREAM_FILTER_NULL);
    
private:
    PdfStream(uint8_t * data, size_t size, const PdfDictionaryPointer & dictionary,
              uint32_t object_number, uint32_t generate_number, PdfCrypto * crypto = nullptr,
              Allocator * allocator = nullptr);
    
    PdfStream(IRead* iread, size_t offset, size_t size, const PdfDictionaryPointer & dictionary,
              uint32_t object_number, uint32_t generate_number, PdfCrypto * crypto = nullptr,
              Allocator * allocator = nullptr);
    
    PdfStream(uint32_t object_number, uint32_t generate_number, PdfCrypto * crypto = nullptr,
              Allocator * allocator = nullptr);

    ~PdfStream();
    
    PdfCrypto * crypto_;
    PdfDictionaryPointer dictionary_;
    
    bool b_memory_stream;
    union {
        uint8_t *   data_;
        IRead *     iread_;
    };
    size_t size_;
    size_t file_offset_;
    uint32_t object_number_;
    uint32_t generate_number_;
    
    friend class Allocator;
    friend class PdfObject;
    friend class PdfStreamAccess;
};

enum PDF_STREAM_DECODE_MODE
{
    STREAM_DECODE_NORMAL,
    STREAM_DECODE_NOTLASTFILTER
};

class PdfStreamAccess : public BaseObject
{
public:
    PdfStreamAccess(Allocator * allocator = nullptr);
    ~PdfStreamAccess();
    
    bool Attach(const PdfStreamPointer & stream, PDF_STREAM_DECODE_MODE mode = STREAM_DECODE_NORMAL);
    void Detach();
    
    PdfStreamPointer GetStream() const { return stream_; }
    
    uint8_t * GetData() const { return data_; }
    size_t GetSize() const { return size_; }
    
private:
    uint8_t * data_;
    size_t size_;
    PdfStreamPointer stream_;
};
    
}//namespace

#endif
