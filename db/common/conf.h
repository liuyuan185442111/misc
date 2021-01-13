#ifndef _L_CONF_H
#define _L_CONF_H

#include <string>
namespace lcore {
struct Conf {
static bool load(const char *file = NULL); //if file is NULL, reload last conf file
static std::string find(const std::string &section, const std::string &key);

class helper {
	void *pdata;
	helper(void *p) : pdata(p){ }
	friend Conf;
public:
	const std::string &operator[](const std::string &key);
};
helper operator[](const std::string &section);
};
}
#endif
