#include "octets.h"
namespace lcore { Octets::Rep Octets::Rep::null = { 0, 0, 1 }; }
//如果release函数报警告, 将null变成指针
//namespace lcore { Octets::Rep *Octets::Rep::null = Rep::create(0); }
