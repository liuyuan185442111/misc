#ifndef _L_DATA_CODER_H
#define _L_DATA_CODER_H

#include "octets.h"
#include "snappy.h"

namespace lcore {

enum {
	DB_OK,
	DB_NOOVERWRITE,
	DB_OVERWRITE,
	DB_NOTFOUND,
	DB_KEYSIZEZERO,
	DB_UNCOMPRESSERR,
};
class DbException : public std::exception
{
	int reason;
public:
	DbException(int e) : reason(e) { }
	virtual ~DbException() throw() { }
	virtual const char *what() const throw()
	{
		switch(reason)
		{
			case DB_NOTFOUND:		return "NOTFOUND";
			case DB_OVERWRITE:		return "OVERWRITE";
			case DB_KEYSIZEZERO:	return "KEYSIZEZERO";
			case DB_UNCOMPRESSERR:	return "UNCOMPRESSERR";
			default:				return "UNKNOWN";
		}
	}
};


struct DataCoder
{
	virtual ~DataCoder() { }
	virtual Octets Compress(const Octets &os) = 0;
	virtual Octets Uncompress(const Octets &os) = 0;
};

struct NullCoder : public DataCoder
{
	Octets Compress(const Octets &os) { return os; }
	Octets Uncompress(const Octets &os) { return os; }
};

struct SnappyCoder : public DataCoder
{
	Octets Compress(const Octets &os)
	{
		Octets val;
		val.reserve(snappy::MaxCompressedLength(os.size()));
		size_t len;
		snappy::RawCompress((const char *)os.begin(), os.size(), (char *)val.begin(), &len);
		val.setsize(len);
		return val;
	}
	Octets Uncompress(const Octets &os)
	{
		Octets val;
		size_t len;
		if(!snappy::GetUncompressedLength((const char *)os.begin(), os.size(), &len))
			throw DbException(DB_UNCOMPRESSERR);
		val.reserve(len);
		val.setsize(len);
		if(!snappy::RawUncompress((const char *)os.begin(), os.size(), (char *)val.begin()))
			throw DbException(DB_UNCOMPRESSERR);
		return val;
	}
};

}

#endif
