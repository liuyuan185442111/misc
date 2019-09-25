#include <string>
#include <list>
#include <algorithm>
#include <cstdio>
#include <cstring>
using namespace std;

//ģ���ģ���������Ҳ��ģ��
//����to_string�����أ�����ƫ�ػ���ģ�庯����֧��ƫ�ػ�

template <template<typename,typename> class T,typename Alloc,typename Elem>
string to_string(T<Elem,Alloc> t)
{
    return string("has no specialization");
}

//��Ԫ��Ϊchar�ı�׼���������ϲ�Ϊһ���ַ���
template <template<typename,typename> class T,typename Alloc>
string to_string(T<char,Alloc> t)
{
    //��Ϊstring::operator+=�ж�����ذ汾��mem_fun1_t�������ƶϳ�ģ����������Ա�����ʽʵ����
    //��&s��Ϊmem_fun1_t����ĵ�һ����������t��ÿ��Ԫ����Ϊmem_fun1_t����ĵڶ�������
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
