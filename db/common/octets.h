#ifndef _L_OCTETS_H
#define _L_OCTETS_H

#ifdef WIN32
#include <windows.h>
#endif
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

namespace lcore {

class Octets
{
	struct Rep
	{
		size_t cap;
		size_t len;
		size_t ref;

	#ifdef _REENTRANT_
		#ifdef WIN32
		void addref()  { InterlockedIncrement((LONG*)&ref); }
		void release() { if (InterlockedDecrement((LONG*)&ref) == 0) delete this; }
		#else
		void addref()
		{
			__asm__ __volatile__ (
				"lock; add $1, %0	\n"
				: "=m"(ref)
			);
		}
		void release()
		{
			size_t old;
			__asm__ __volatile__ (
				"lock; xadd  %2, %0	\n"
				: "=m"(ref), "=r"(old)
				: "1"(-1) : "memory"
			);

			if (old == 1) delete this;
		}
		#endif
	#else
		void addref() { ++ref; }
		void release() { if (--ref == 0) delete this; }
	#endif

		void *data() { return reinterpret_cast<void*>(this + 1); }
		void *clone()
		{
			Rep *rep = create(cap);
			memcpy(rep->data(), data(), rep->len = len);
			return rep->data();
		}
		void *unique()
		{
			if (ref > 1)
			{
				void* r = clone();
				release();
				return r;
			}
			return data();
		}
		void *reserve(size_t size)
		{
			size = frob_size(size);
			if (size > cap)
			{
				Rep* rep = create(size);
				memcpy(rep->data(), data(), rep->len = len);
				release();
				return rep->data();
			}
			return unique();
		}
		static size_t frob_size(size_t size)
		{
			size_t tmp = 16;
			while (size > tmp) tmp <<= 1;
			return tmp;
		}
		static Rep* create(size_t cap)
		{
			Rep *rep = new (cap) Rep;
			rep->cap = cap;
			rep->len = 0;
			rep->ref = 1;
			return rep;
		}
		static void * operator new(size_t size, size_t extra) { return malloc(size + extra); }
		static void   operator delete(void *p) { free(p); }
		static Rep null;
	};
	void *base;
	Rep *rep() const { return reinterpret_cast<Rep*>(base) - 1; }
	void unique() { base = rep()->unique(); }
public:
	virtual ~Octets() { rep()->release(); }
	Octets() : base(Rep::null.data()) { rep()->addref(); }
	Octets(size_t size) : base(Rep::create(size)->data()) { }
	Octets(const void *x, size_t size) : base(Rep::create(size)->data())
	{
		memcpy(base, x, size);
		rep()->len = size;
	}
	Octets(const char *str)
	{
		size_t size = strlen(str);
		base = Rep::create(size)->data();
		memcpy(base, str, size);
		rep()->len = size;
	}
	Octets(const void *x, const void *y)
	{
		size_t size = (char*)y-(char*)x;
		base = Rep::create(size)->data();
		memcpy(base, x, size);
		rep()->len = size;
	} 
	Octets(const Octets &x) : base(x.base) { rep()->addref(); }
	Octets& operator = (const Octets&x)
	{
		if(&x != this)
		{
			rep()->release();
			base = x.base;
			rep()->addref();
		}
		return *this;
	}
	bool operator == (const Octets &x) const { return size() == x.size() && !memcmp(base, x.base, size()); }
	bool operator != (const Octets &x) const { return ! operator == (x); }
	Octets& swap(Octets &x) { void *tmp = base; base = x.base; x.base = tmp; return *this; }
	void *begin() { unique(); return base; }
	void *end()   { unique(); return (char*)base + rep()->len; }
	const void *begin() const { return base; }
	const void *end()   const { return (char*)base + rep()->len; }
	size_t size()     const { return rep()->len; }
	size_t capacity() const { return rep()->cap; }
	Octets& clear() { unique(); rep()->len = 0; return *this;  }
	Octets& erase(size_t pos, size_t len) { char *x = (char*)begin(); return erase(x + pos, x + pos + len); }
	Octets& erase(void *x, void *y)
	{
		if(x != y)
		{
			void *tmp = base;
			base = rep()->unique();
			ptrdiff_t o = (char*)base - (char*)tmp;
			if(o)
			{
				x = (char*)x + o;
				y = (char*)y + o;
			}
			memmove((char*)x, (char*)y, ((char*)base + rep()->len) - (char*)y);
			rep()->len -= (char*)y - (char*)x;
		}
		return *this;
	}
	Octets& insert(void *pos, const void *x, size_t len)
	{
		ptrdiff_t off = (char*)pos - (char*)base;
		reserve(size() + len);
		pos = (char*)base + off;
		size_t adjust = size() - off;
		if(adjust) memmove((char*)pos + len, pos, adjust);
		memmove(pos, x, len);
		rep()->len += len;
		return *this;
	}
	Octets& insert(void *pos, const void *x, const void *y) { insert(pos, x, (char*)y - (char*)x); return *this; }
	Octets& replace(const void *data, size_t size)
	{
		reserve(size);
		memcpy(base, data, size);
		rep()->len = size;
		return *this;
	}
	Octets& reserve(size_t size)
	{
		base = rep()->reserve(size);
		return *this;
	}
	Octets& setsize(size_t size) { reserve(size); rep()->len = size; return *this; }
};

} //namespace lcore

#endif //_L_OCTETS_H
