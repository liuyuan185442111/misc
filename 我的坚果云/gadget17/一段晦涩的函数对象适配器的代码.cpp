#include <string>
#include <list>
#include <algorithm>
#include <cstdio>
#include <cstring>
using namespace std;

//模板的模板参数可以也是模板
//两个to_string是重载，不是偏特化，模板函数不支持偏特化

template <template<typename,typename> class T,typename Alloc,typename Elem>
string to_string(T<Elem,Alloc> t)
{
    return string("has no specialization");
}

//将元素为char的标准序列容器合并为一个字符串
template <template<typename,typename> class T,typename Alloc>
string to_string(T<char,Alloc> t)
{
    //因为string::operator+=有多个重载版本，mem_fun1_t并不能推断出模板参数，所以必须显式实例化
    //将&s绑定为mem_fun1_t对象的第一个参数，将t的每个元素作为mem_fun1_t对象的第二个参数
    string s;
    for_each(t.begin(),t.end(),bind1st(mem_fun1_t<string&,string,char>(&string::operator+=),&s));
    return s;
}

int main()
{
    list<char> cv;
    cv.push_back('1');
    cv.push_back('2');
    cv.push_back('3');
    cv.push_back('4');
    puts(to_string(cv).c_str());

    list<int> ci;
    ci.push_back(49);
    puts(to_string(ci).c_str());
}
