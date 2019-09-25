// string的一个部分实现
#include <iostream>
#include <cstring>
#include "string.h"
#include <cstddef>

using std::strcpy;
using std::strcmp;
using std::strcat;
using std::strlen;

string::string()
{
    len = 0;
    str = new char[len+1];
    str[0] = '\0';
}

string::string(const char *s)
{
    len = strlen(s);
    str = new char[len+1];
    strcpy(str,s);
}

string::string(const string& s)
{
    len = s.len;
    str = new char[len+1];
    strcpy(str,s.str);
}

string::string(int t,char c)
{
    len = t;
    str = new char[len+1];
    for(int i=0; i<t; ++i)
        str[i] = c;
    str[len] = '\0';
}

string::string(const char *s,int t)
{
    len = t;
    str = new char[len+1];
    for(int i=0; i<t; ++i)
        str[i] = s[i];
    str[len] = '\0';
}

string::string(char *start,char *stop)
{
    len = stop - start;
    str = new char[len+1];
    for(int i=0; i<len; ++i)
        str[i] = start[i];
    str[len] = '\0';
}

string::~string()
{
    delete[] str;
}

string& string::operator=(const char *s)
{
    delete[] str;
    len = strlen(s);
    str = new char[len+1];
    strcpy(str,s);
    return *this;
}

string& string::operator=(const string& s)
{
    if(this == &s) return *this;
    delete[] str;
    len = s.len;
    str = new char[len+1];
    strcpy(str,s.str);
    return *this;
}

char& string::operator[](int i)
{
    return str[i];
}

const char& string::operator[](int i) const
{
    return str[i];
}

ostream& operator<<(ostream& os,const string& s)
{
    os << s.str;
    return os;
}

string operator+(const string& a,const string& b)
{
    string t;
    t.len = a.len + b.len;
    t.str = new char[t.len+1];
    strcpy(t.str,a.str);
    strcat(t.str,b.str);
    return t;
}

string& operator+=(string& a,const string& b)
{
    char *p = a.str;
    a.len += b.len;
    a.str = new char[a.len+1];
    strcpy(a.str,p);
    strcat(a.str,b.str);
    delete[] p;
    return a;
}

string& operator+=(string& a,const char c)
{
    char *p = a.str;
    a.len += 1;
    a.str = new char[a.len+1];
    strcpy(a.str,p);
    a.str[a.len-1] = c;
    a.str[a.len] = '\0';
    delete[] p;
    return a;
}

istream& operator>>(istream& is, string& s)
{
    const int CINLIM = 11; //字符串分段大小,实际上是CINLIM-1,最后一个存'\0'
    bool overflag = false; //用来消除开头的空白
    char p[CINLIM] = "";
    string strtemp;
    while(1)
    {
        for(int i=0; i<CINLIM-1; i++)
        {
            char t = is.get();
            switch(t)
            {
            case -1: //到达文件结尾
                if(!overflag) return is; //上来就遇到了文件结束符
                is.clear();
            case ' ':
            case '\t':
            case '\n':
                if(overflag)
                {
                    p[i] = '\0'; //很重要!保证下句的正确执行
                    s = strtemp + p;
                    return is;
                }
                else i = -1; //忽略掉空白,进行下一次循环
                break;
            default:
                overflag = true;
                p[i] = t;
            }
        }
        //如果执行到此处,说明输入字符串比较长
        strtemp += p;
    }
    return is;
}

bool operator>(const string& a,const string& b)
{
    return strcmp(a.str,b.str) > 0;
}

bool operator<(const string& a,const string& b)
{
    return strcmp(a.str,b.str) < 0;
}

bool operator==(const string& a, const string& b)
{
    return !strcmp(a.str,b.str);
}

bool operator!=(const string& a, const string& b)
{
    return strcmp(a.str,b.str);
}

int string::size() const
{
    return len;
}

int string::length() const
{
    return len;
}

const char *string::c_str() const
{
    return str;
}

int string::find(char c, int pos) const
{
    for(int i=pos; i<len; ++i)
        if(str[i] == c) return i;
    return npos;
}

int string::find(const char *s,int pos) const
{
    char *p = std::strstr(str+pos,s);
    return (p == NULL) ? npos : p-str;
}

int string::find(const char *s,int pos,int n) const
{
    char *temp = new char[n+1];
    std::strncpy(temp,s,n);
    temp[n] = '\0';
    int t = find(temp,pos);
    delete[] temp;
    return t;
}

int string::find(const string& s,int pos) const
{
    return find(s.str,pos);
}

void string::stringup() const
{
    char *p = str;
    while(*p)
    {
        *p = std::toupper(*p);
        ++p;
    }
}

void string::stringlow() const
{
    char *p = str;
    while(*p)
    {
        *p = std::tolower(*p);
        ++p;
    }
}

int string::has(char a) const
{
    int i = 0;
    char *p = str;
    while(*p)
    {
        if(*p == a) ++i;
        ++p;
    }
    return i;
}

istream& getline(istream& is,string& s,char c)
{
    const int CINLIM = 11; //字符串分段大小,实际上是CINLIM-1,最后一个存'\0'
    char p[CINLIM] = "";
    string strtemp;
    while(1)
    {
        for(int i=0; i<CINLIM-1; ++i)
        {
            char t = is.get();
            if(t == -1) //到达文件结尾
            {
                p[i] = '\0';
                s = strtemp + p;
                is.clear();
                return is;
            }
            if(t == c) //默认'\n'
            {
                p[i] = '\0'; //很重要!保证下句的正确执行
                s = strtemp + p;
                return is;
            }
            else
                p[i] = t;
        }
        //如果执行到此处,说明输入字符串很长
        strtemp += p;
    }
    return is;
}

int main()
{
    string s("hello");
    std::cout << s;
}
