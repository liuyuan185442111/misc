#ifndef _L_GCORE_CONF_H
#define _L_GCORE_CONF_H

#include <string>
namespace lcore {
struct Conf {
//if file is NULL, reload last conf file
static bool load(const char *file = NULL);
static std::string find(const std::string &section, const std::string &key);

//just have a fun
struct helper {
	void *pdata;
	helper(void *p) : pdata(p) { }
	std::string &operator[](const std::string &key);
};
helper operator[](const std::string &section);
};
}
#endif
