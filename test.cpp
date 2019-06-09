#include <string>
#include <list>
#include <algorithm>
#include <vector>
#include <iostream>
#include <iterator>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stddef.h>
#include <assert.h>
#include <functional>
#include <map>
#include <array>
#include <thread>
#include <unordered_map>
#include <leveldb/db.h>
#include <cinttypes>
#include <utility>
using namespace std;


namespace nstd {
template<class T, T v>
struct integral_constant {
	static constexpr T value = v;
};
using true_type = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;
}


template <typename T, typename>
struct has_tostring : nstd::false_type {};

template <typename T>
struct has_tostring<T, decltype(std::declval<T>().tostring())> : nstd::true_type {};


struct A {
	std::string tostring();
};

struct B {
	const char* tostring();
};

int main(int argc, char ** argv)
{
	std::cout << boolalpha;
	std::cout << has_tostring<A, std::string>::value << std::endl;
	std::cout << has_tostring<B, std::string>::value << std::endl;
	std::cout << has_tostring<B, const char *>::value << std::endl;
	return 0;
}

//https://www.jianshu.com/p/007c041f43ab
