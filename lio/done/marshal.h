#ifndef __MARSHAL_H
#define __MARSHAL_H

#include "byteorder.h"
#include "octets.h"
#include <exception>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <map>

#ifdef __GNUC__
#define STATIC_ASSERT_NO_WARNING __attribute__((unused))
#else
#define STATIC_ASSERT_NO_WARNING
#endif
template<bool x> struct STATIC_ASSERTION_FAILURE;
template<> struct STATIC_ASSERTION_FAILURE<true>{};
template<int x> struct static_assert_test{};
#define static_assert(B) typedef static_assert_test<sizeof(STATIC_ASSERTION_FAILURE<(bool)(B)>)>\
    static_assert_typedef_##__LINE__ STATIC_ASSERT_NO_WARNING

namespace
{
template <typename U, typename V>
union TypeAliasing
{
	U _u;
	V _v;
	TypeAliasing(V v) : _v(v) { }
	operator U() const { return _u; }
};
}
template <typename U, typename V>
inline U aliasing_cast(V v)
{
    return TypeAliasing<U, V>(v);
}

namespace GNET
{

class OctetsStream;
struct Marshal
{
	virtual OctetsStream& marshal(OctetsStream &) const = 0;
	virtual const OctetsStream& unmarshal(const OctetsStream &) = 0;
	virtual ~Marshal() { }
};

template <typename T>
inline T& remove_const(const T &t) { return const_cast<T&>(t); }

template <typename Container>
class STLContainer;
template <typename Container>
inline STLContainer<Container> MarshalContainer(const Container &c)
{
	return STLContainer<Container>(remove_const(c));
}

class OctetsStream
{
	class Exception : public std::exception
	{
		std::string msg;
	public:
		Exception(const std::string &arg) throw() : msg(arg){}
		virtual ~Exception() throw() { }
		virtual const char* what() throw() { return msg.c_str(); }
	};

	enum { MAXSPARE = 16384};
	Octets data;
	mutable size_t pos;
	mutable size_t tranpos;

	template<typename T> OctetsStream& push_byte(T t)
	{
		data.insert(data.end(), &t, sizeof(t));
		return *this;
	}
	template<typename T> void pop_byte(T &t) const
	{
		if(pos + sizeof(t) > data.size()) throw Exception("pop_byte T out_of_range");
		t = *(T *)((char*)data.begin() + pos);
		pos += sizeof(t);
	}
	unsigned char pop_byte_8() const
	{
		unsigned char c;
		pop_byte(c);
		return c;
	}
	unsigned short pop_byte_16() const
	{
		unsigned short s;
		pop_byte(s);
		return byteorder_16(s);
	}
	unsigned int pop_byte_32() const
	{
		unsigned int i;
		pop_byte(i);
		return byteorder_32(i);
	}
	unsigned long long pop_byte_64() const
	{
		unsigned long long ll;
		pop_byte(ll);
		return byteorder_64(ll);
	}

	friend class CompactUINT;
	friend class CompactSINT;

	OctetsStream& compact_uint32(unsigned int x)
	{
		if(x < 0x80) return push_byte((unsigned char)x);
		else if(x < 0x4000) return push_byte(byteorder_16(x|0x8000));
		else if(x < 0x20000000) return push_byte(byteorder_32(x|0xC0000000));
		push_byte((unsigned char)0xE0);
		return push_byte(byteorder_32(x));
	}
	OctetsStream& compact_sint32(int x)
	{
		if(x >= 0)
		{
			if(x < 0x40) return push_byte((unsigned char)x);
			else if(x < 0x2000) return push_byte(byteorder_16(x|0x4000));
			else if(x < 0x10000000) return push_byte(byteorder_32(x|0x60000000));
			push_byte((unsigned char)0x70);
			return push_byte(byteorder_32(x));
		}
		x = -x;
		if(x < 0x40) return push_byte((unsigned char)(x|0x80));
		else if(x < 0x2000) return push_byte(byteorder_16(x|0xC000));
		else if(x < 0x10000000) return push_byte(byteorder_32(x|0xE0000000));
		push_byte((unsigned char)0xF0);
		return push_byte(byteorder_32(x));
	}
	const OctetsStream& uncompact_uint32(const unsigned int &x) const
	{
		switch(*((unsigned char *)data.begin()+pos) & 0xE0)
		{
			case 0xE0:
				pop_byte_8();
				remove_const(x) = pop_byte_32();
				break;
			case 0xC0:
				remove_const(x) = pop_byte_32() & ~0xC0000000;
				break;
			case 0xA0:
			case 0x80:
				remove_const(x) = pop_byte_16() & ~0x8000;
				break;
			default:
				remove_const(x) = pop_byte_8();
				break;
		}
		return *this;
	}
	const OctetsStream& uncompact_sint32(const int &x) const
	{
		unsigned char header = *((unsigned char *)data.begin()+pos);
		int abs;
		switch(header & 0x70)
		{
			case 0x70:
				pop_byte_8();
				abs = pop_byte_32();
				break;
			case 0x60:
				abs = pop_byte_32() & ~0xE0000000;
				break;
			case 0x50:
			case 0x40:
				abs = pop_byte_16() & ~0xC000;
				break;
			default:
				abs = pop_byte_8() & ~0x80;
				break;
		}
		if(header & 0x80) remove_const(x) = -abs;
		else remove_const(x) = abs;
		return *this;
	}

public:
	enum Transaction { Begin, Commit, Rollback };
	OctetsStream() : pos(0) {}
	OctetsStream(const Octets &o) : data(o), pos(0) {}
	OctetsStream(const OctetsStream &os) : data(os.data), pos(0) {}
	OctetsStream& operator = (const OctetsStream &os) 
	{
		if(&os != this)
		{
			pos  = os.pos;
			data = os.data;
		}
		return *this;
	}

	bool operator == (const OctetsStream &os) const { return data == os.data; }
	bool operator != (const OctetsStream &os) const { return data != os.data; }
	size_t size() const { return data.size(); }
	void swap(OctetsStream &os) { data.swap(os.data); }
	operator Octets& () { return data; }
	operator const Octets& () const { return data; }

	void *begin() { return data.begin(); }
	void *end()   { return data.end(); }
	const void *begin() const { return data.begin(); }
	const void *end()   const { return data.end();   }
	void insert(void *pos, const void *x, size_t len) { data.insert(pos, x, len); }
	void insert(void *pos, const void *x, const void *y) { data.insert(pos, x, y); }
	void erase(void *x, void *y) { data.erase(x, y); }
	void clear() { data.clear(); pos = 0; }
	bool eos() const { return pos == data.size(); }

	OctetsStream& operator << (char x)               { return push_byte(x); }
	OctetsStream& operator << (unsigned char x)      { return push_byte(x); }
	OctetsStream& operator << (bool x)               { return push_byte(static_cast<char>(x)); }
	OctetsStream& operator << (short x)              { return push_byte(byteorder_16(x)); }
	OctetsStream& operator << (unsigned short x)     { return push_byte(byteorder_16(x)); }
	OctetsStream& operator << (int x)                { return push_byte(byteorder_32(x)); }
	OctetsStream& operator << (unsigned int x)       { return push_byte(byteorder_32(x)); }
	OctetsStream& operator << (int64_t x)            { return push_byte(byteorder_64(x)); }
	OctetsStream& operator << (float x)              { return push_byte(byteorder_32(aliasing_cast<int>(x))); }
	OctetsStream& operator << (double x)             { return push_byte(byteorder_64(aliasing_cast<unsigned long long>(x))); }
	OctetsStream& operator << (const Marshal &x)     { return x.marshal(*this); }
	OctetsStream& operator << (const Octets &x)
	{
		compact_uint32(static_cast<unsigned int>(x.size()));
		data.insert(data.end(), x.begin(), x.end());
		return *this;
	}
	template<typename T>
	OctetsStream& operator << (const std::basic_string<T> &x)
	{
		static_assert(sizeof(T) == 1);//when needed, release other sizeof
		unsigned int bytes = x.length()*sizeof(T);
		compact_uint32(bytes);
		insert(end(), (void*)x.c_str(), bytes);
		return *this;
	}
	OctetsStream& push_byte(const char *x, size_t len)
	{
		data.insert(data.end(), x, len);
		return *this;
	}
	template<typename T1, typename T2>
	OctetsStream& operator << (const std::pair<T1, T2> &x)
	{
		return *this << x.first << x.second;
	}
	template<typename T>
	OctetsStream& operator << (const std::vector<T> &x) 
	{
		return *this << (MarshalContainer(x));
	}
	template<typename T>
	OctetsStream& operator << (const std::list<T> &x) 
	{
		return *this << (MarshalContainer(x));
	}
	template<typename T>
	OctetsStream& operator << (const std::deque<T> &x) 
	{
		return *this << (MarshalContainer(x));
	}
	template<typename T1, typename T2>
	OctetsStream& operator << (const std::map<T1, T2> &x) 
	{
		return *this << (MarshalContainer(x));
	}

	const OctetsStream& operator >> (Transaction trans) const
	{
		switch (trans)
		{
			case Begin:
				tranpos = pos;
				break;
			case Rollback:
				pos = tranpos;
				break;
			case Commit:
				if(pos >= MAXSPARE)
				{
					remove_const(*this).data.erase((char*)data.begin(), (char*)data.begin()+pos);	
					pos = 0;
				}
		}
		return *this;
	}
	const OctetsStream& operator >> (const char &x) const
	{
		remove_const(x) = pop_byte_8();
		return *this;
	}
	const OctetsStream& operator >> (const unsigned char &x) const
	{
		remove_const(x) = pop_byte_8();
		return *this;
	}
	const OctetsStream& operator >> (const bool &x) const
	{
		remove_const(x) = pop_byte_8() != 0;
		return *this;
	}
	const OctetsStream& operator >> (const short &x) const
	{
		remove_const(x) = pop_byte_16();
		return *this;
	}
	const OctetsStream& operator >> (const unsigned short &x) const
	{
		remove_const(x) = pop_byte_16();
		return *this;
	}
	const OctetsStream& operator >> (const int &x) const
	{
		remove_const(x) = pop_byte_32();
		return *this;
	}
	const OctetsStream& operator >> (const unsigned int &x) const
	{
		remove_const(x) = pop_byte_32();
		return *this;
	}
	const OctetsStream& operator >> (const int64_t &x) const
	{
		remove_const(x) = pop_byte_64();
		return *this;
	}
	const OctetsStream& operator >> (const float &x) const
	{
		remove_const(x) = aliasing_cast<float>(pop_byte_32());
		return *this;
	}
	const OctetsStream& operator >> (const double &x) const
	{
		remove_const(x) = aliasing_cast<double>(pop_byte_64());
		return *this;
	}
	const OctetsStream& operator >> (const Marshal &x) const
	{
		return remove_const(x).unmarshal(*this);
	}
	const OctetsStream& operator >> (const Octets &x) const
	{
		unsigned int len;
		uncompact_uint32(len);
		if(len > data.size() - pos) throw Exception("invalid length for Octets");
		remove_const(x).replace((char*)data.begin()+pos, len);
		pos += len;
		return *this;
	}
	template<typename T>
	const OctetsStream& operator >> (const std::basic_string<T> &x) const
	{
		static_assert(sizeof(T) == 1);//when needed, release other sizeof
		unsigned int bytes;
		uncompact_uint32(bytes);
		if(bytes % sizeof(T)) throw Exception("operator >> size_error");
		if(bytes > data.size() - pos) throw Exception("operator >> out_of_range");
		remove_const(x).assign((T*)((char*)data.begin()+pos), bytes/sizeof(T));
		pos += bytes;
		return *this;
	}
	void pop_byte(char *x, size_t len) const
	{
		if(pos + len > data.size()) throw Exception("pop_byte str out_of_range");
		memcpy(x, (char*)data.begin()+pos, len);
		pos += len;
	}
	template<typename T1, typename T2>
	const OctetsStream& operator >> (const std::pair<T1, T2> &x) const
	{
		return *this >> remove_const(x.first) >> remove_const(x.second);
	}
	template<typename T>
	const OctetsStream& operator >> (const std::vector<T> &x) const
	{
		return *this >> (MarshalContainer(x));			
	}
	template<typename T>
	const OctetsStream& operator >> (const std::deque<T> &x) const
	{
		return *this >> (MarshalContainer(x));			
	}
	template<typename T>
	const OctetsStream& operator >> (const std::list<T> &x) const
	{
		return *this >> (MarshalContainer(x));			
	}
	template<typename T1, typename T2>
	const OctetsStream& operator >> (const std::map<T1, T2> &x) const
	{
		return *this >> (MarshalContainer(x));			
	}
};

class CompactUINT : public Marshal
{
	unsigned int *pi;
public:
	explicit CompactUINT(const unsigned int &i): pi(&remove_const(i)) { }
	OctetsStream& marshal(OctetsStream &os) const
	{
		return os.compact_uint32(*pi);
	}
	const OctetsStream& unmarshal(const OctetsStream &os)
	{
		return os.uncompact_uint32(*pi);
	}
};

class CompactSINT : public Marshal
{
	int *pi;
public:
	explicit CompactSINT(const int &i): pi(&remove_const(i)) { }
	OctetsStream& marshal(OctetsStream &os) const
	{
		return os.compact_sint32(*pi);
	}
	const OctetsStream& unmarshal(const OctetsStream &os)
	{
		return os.uncompact_sint32(*pi);
	}
};

template <typename Container>
class STLContainer : public Marshal
{
	Container *pc;
public:
	explicit STLContainer(Container &c) : pc(&c) { }
	OctetsStream& marshal(OctetsStream &os) const
	{
		os << CompactUINT(pc->size());
		for(typename Container::const_iterator i = pc->begin(), e = pc->end(); i != e; ++i)
			os << *i;
		return os;
	}
	const OctetsStream& unmarshal(const OctetsStream &os)
	{
		pc->clear();
		unsigned int size;
		for(os >> CompactUINT(size); size > 0; --size)
		{
			typename Container::value_type tmp;
			os >> tmp;
			pc->insert(pc->end(), tmp);
		}
		return os;
	}
};

}//namespace GNET

#endif//__MARSHAL_H
