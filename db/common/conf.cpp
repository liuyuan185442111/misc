#include "conf.h"
#include <sys/stat.h>
#include <unistd.h>
#include <map>
#include <cstring>
#include <fstream>

namespace lcore {
namespace {
using std::string;
struct stringcasecmp
{
	bool operator() (const string &x, const string &y) const { return strcasecmp(x.c_str(), y.c_str()) < 0; }
};
typedef std::map<string, string, stringcasecmp> SECTION_MAP;
typedef std::map<string, SECTION_MAP, stringcasecmp> CONF_MAP;

class _Conf
{
	CONF_MAP confmap;
	time_t mtime;
	string filename;
	string lastfile;
	string null;
	_Conf() : mtime(0) { }
	bool reload()
	{
		struct stat st;
		if(stat(filename.c_str(), &st) == -1) return false;
		for(; lastfile != filename || mtime != st.st_mtime; stat(filename.c_str(), &st))
		{
			lastfile = filename;
			mtime = st.st_mtime;
			std::ifstream ifs(filename.c_str());
			string line;
			string section;
			SECTION_MAP secmap;
			if(!confmap.empty()) confmap.clear();
			while(std::getline(ifs, line))
			{
				if(line[0] == '#') continue;
				if(line[0] == '[')
				{
					string::size_type start = line.find_first_not_of(" \t", 1);
					if(start == string::npos) continue;
					string::size_type end   = line.find_first_of(" \t]", start);
					if(end   == string::npos) continue;
					if(!section.empty()) confmap[section] = secmap;
					section = string(line, start, end - start);
					secmap.clear();
				}
				else
				{
					string::size_type key_start = line.find_first_not_of(" \t");
					if(key_start == string::npos) continue;
					string::size_type key_end   = line.find_first_of(" \t=", key_start);
					if(key_end   == string::npos) continue;
					string::size_type val_start = line.find_first_of("=", key_end);
					if(val_start == string::npos) continue;
					val_start = line.find_first_not_of(" \t", val_start + 1);
					if(val_start == string::npos) continue;
					string::size_type val_end = line.find_last_not_of(" \t\r\n");
					if(val_end   == string::npos) continue;
					if(val_end < val_start) continue;
					secmap[string(line, key_start, key_end-key_start)] = string(line, val_start, val_end-val_start+1);
				}
			}
			if(!section.empty()) confmap[section] = secmap;
		}
		return true;
	}
public:
	bool load(const char *file)
	{
		if(file)
		{
			if(access(file, R_OK) == -1) return false;
			instance.filename = file;
			return instance.reload();
		}
		return !instance.lastfile.empty() && (access(instance.lastfile.c_str(), R_OK) == 0) && instance.reload();
	}
	const string &find(const string &section, const string &key) const
	{
		if(instance.lastfile.empty()) return null;
		CONF_MAP::const_iterator it = confmap.find(section);
		if(it != confmap.end())
		{
			SECTION_MAP::const_iterator it2 = it->second.find(key);
			if(it2 != it->second.end()) return it2->second;
		}
		return null;
	}
	const SECTION_MAP &operator[](const string &section){ return confmap[section]; }
	static _Conf instance;
};
_Conf _Conf::instance;
}

bool Conf::load(const char *file)
{
	return _Conf::instance.load(file);
}
string Conf::find(const string &section, const string &key)
{
	return _Conf::instance.find(section, key);
}

Conf::helper Conf::operator[](const string &section)
{
	return helper((void*)(&_Conf::instance[section]));
}
const string &Conf::helper::operator[](const string &key)
{
	return (*(SECTION_MAP*)pdata)[key];
}
}
