#ifndef STRING__H
#define STRING__H

#include <iostream>
using std::ostream;
using std::istream;

class string
{
private:
    char *str;
    int len;
public:
    enum{npos=32767};
public:
    // 构造和析构
    string();
    string(const char *s);
    string(const string& s);
    string(int i,char c);
    string(const char *s,int t);
    string(char *start,char *stop);
    ~string();
    // 运算符重载
    string& operator=(const char * s);
    string& operator=(const string& s);
    char& operator[](int i);
    const char& operator[](int i) const; //供const string使用
    friend ostream& operator<<(ostream& os,const string& s);
    friend string operator+(const string& a,const string& b);
    friend string& operator+=(string& a,const string& b);
    friend string& operator+=(string& a,const char c);
    friend istream& operator>>(istream& is,string& s);
    friend bool operator>(const string& a,const string& b);
    friend bool operator<(const string& a,const string& b);
    friend bool operator==(const string& a,const string& b);
    friend bool operator!=(const string& a,const string& b);
    // 其他操作
    int size() const;
    int length() const;
    const char *c_str() const; //不允许外界修改
    // 查找成功时返回所在位置,失败返回string::npos的值
    int find(char c,int pos = 0) const; //从pos开始查找字符c在当前字符串的位置
    int find(const char *s,int pos = 0) const; //从pos开始查找字符串s在当前串中的位置
    int find(const char *s,int pos,int n) const; //从pos开始查找字符串s中前n个字符在当前串中的位置
    int find(const string& s,int pos = 0) const; //从pos开始查找字符串s在当前串中的位置
    // 我自定义的操作
    void stringup() const; //全变成大写
    void stringlow() const; //全变成小写
    int has(char a) const; //字符串中字符a的个数
};

istream& getline(istream& is, string& s, char c='\n');

#endif // STRING__H
