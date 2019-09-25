[《C++11 标准库源代码剖析：连载之二》](https://www.jianshu.com/p/007c041f43ab)结尾提出了一个例子，想了很久才想明白，这个例子类似：
```cpp
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
	std::cout << has_tostring<A, std::string>::value << std::endl;//语句1
	std::cout << has_tostring<B, std::string>::value << std::endl;//语句2
	std::cout << has_tostring<B, const char *>::value << std::endl;
	std::cout << has_tostring<int, int>::value << std::endl;//语句4
	return 0;
}
```
has_tostring可以判断出某个类型是否有tostring函数。
对于“语句1”，先尝试匹配has_tostring的偏特化版本，第二个模板参数推断为A::tostring的返回类型std::string，特化出has_string<A, std::string>的版本，正好需要的也是这个版本。
对于“语句2”，也是先看has_tostring的偏特化版本，第二个模板参数推断为B::tostring的返回类型const char *，特化出has_string<B, const char *>的版本，但需要的却是has_string<B, std::string>的版本，于是最后使用通用的has_tostring。
对于“语句4”，也是先看has_tostring的偏特化版本，因为int类型没有tostring函数，推断第二个模板参数的过程中出现错误，转而使用通用的has_tostring。

也可以为has_tostring添加一个默认模板参数：

	template <typename T, typename = std::string>
	struct has_tostring : nstd::false_type {};
语句1和语句2可以变为：

	std::cout << has_tostring<A>::value << std::endl;
	std::cout << has_tostring<B>::value << std::endl;

---
不知道分析的对不对
