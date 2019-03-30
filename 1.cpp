类的非static成员变量可以在定义的时候直接初始化。

sizeof运算符可以在类的数据成员上使用，无需明确对象。但该数据成员应是public的。
class A {
public:
	int m = 5;
};
int main() {
	A a;
	cout << sizeof(A::m) << sizeof(a.m);
	return 0;
}

增强的定义别名的能力
using pf = void(*)();
template <typename First, typename Seconde, typename Third>
class Some
{
	//...
};
template <typename Second>
using Spec = Some<int, Second, int>;

auto和decltype组合起来用还能实现函数返回类型后置:
template <typename T, typename U>
auto add(T t, U u) -> decltype(t + u) {
	return t + u;
}
