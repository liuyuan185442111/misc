#ifndef __BYTEORDER_H
#define __BYTEORDER_H

/* Network provides transparent transmission,
 * so if the two sides use the same byte ordering(little_endian or big_endian),
 * there is no need to transform to big_endian for network trasnmission.
 * */

#if defined _SAME_BYTE_ORDER_
	#define byteorder_16(x)	(x)
	#define byteorder_32(x)	(x)
	#define byteorder_64(x)	(x)
#elif defined _BYTE_ORDER_BIG_ENDIAN_
	#define byteorder_16(x)	(x)
	#define byteorder_32(x)	(x)
	#define byteorder_64(x)	(x)
#elif defined __GNUC__
	inline unsigned short byteorder_16(unsigned short x)
	{
		register unsigned short v;
		__asm__ ("xchg %b0, %h0" : "=q"(v) : "0"(x));
		return v;
	}
	#if defined(__x86_64__)
	inline unsigned int byteorder_32(unsigned int x)
	{
		register unsigned int v;
		__asm__ ("bswapl %0" : "=r"(v) : "0"(x));
		return v;
	}
	inline unsigned long byteorder_64(unsigned long x)
	{
		register unsigned long v;
		__asm__("bswapq %0":"=r"(v):"0"(x));
		return v;
	}
	#elif defined(__i386__)
	inline unsigned long byteorder_32(unsigned long x)
	{
		register unsigned long v;
		__asm__ ("bswap %0" : "=r"(v) : "0"(x));
		return v;
	}
	inline unsigned long long byteorder_64(unsigned long long x)
	{
		union
		{
			unsigned long long __ll;
			unsigned long __l[2];
		} i, o;
		i.__ll = x;
		o.__l[0] = byteorder_32(i.__l[1]);
		o.__l[1] = byteorder_32(i.__l[0]);
		return o.__ll;
	}
	#endif
#elif defined WIN32
	inline unsigned __int16 byteorder_16(unsigned __int16 x)
	{
		__asm ror x, 8
		return x;
	}
	inline unsigned __int32 byteorder_32(unsigned __int32 x)
	{
		__asm mov eax, x
		__asm bswap eax
		__asm mov x, eax
		return x;
	}
	#if defined(__x86_64__)
	inline unsigned __int64 byteorder_64(unsigned __int64 x)
	{
		__asm mov rax, x
		__asm bswap rax
		__asm mov x,rax
		return x;
	}
	#elif defined(__i386__)
	inline unsigned __int64 byteorder_64(unsigned __int64 x)
	{
		union
		{
			unsigned __int64 __ll;
			unsigned __int32 __l[2];
		} i, o;
		i.__ll = x;
		o.__l[0] = byteorder_32(i.__l[1]);
		o.__l[1] = byteorder_32(i.__l[0]);
		return o.__ll;
	}
	#endif
#endif

#endif
