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
    // ���������
    string();
    string(const char *s);
    string(const string& s);
    string(int i,char c);
    string(const char *s,int t);
    string(char *start,char *stop);
    ~string();
    // ���������
    string& operator=(const char * s);
    string& operator=(const string& s);
    char& operator[](int i);
    const char& operator[](int i) const; //��const stringʹ��
    friend ostream& operator<<(ostream& os,const string& s);
    friend string operator+(const string& a,const string& b);
    friend string& operator+=(string& a,const string& b);
    friend string& operator+=(string& a,const char c);
    friend istream& operator>>(istream& is,string& s);
    friend bool operator>(const string& a,const string& b);
    friend bool operator<(const string& a,const string& b);
    friend bool operator==(const string& a,const string& b);
    friend bool operator!=(const string& a,const string& b);
    // ��������
    int size() const;
    int length() const;
    const char *c_str() const; //����������޸�
    // ���ҳɹ�ʱ��������λ��,ʧ�ܷ���string::npos��ֵ
    int find(char c,int pos = 0) const; //��pos��ʼ�����ַ�c�ڵ�ǰ�ַ�����λ��
    int find(const char *s,int pos = 0) const; //��pos��ʼ�����ַ���s�ڵ�ǰ���е�λ��
    int find(const char *s,int pos,int n) const; //��pos��ʼ�����ַ���s��ǰn���ַ��ڵ�ǰ���е�λ��
    int find(const string& s,int pos = 0) const; //��pos��ʼ�����ַ���s�ڵ�ǰ���е�λ��
    // ���Զ���Ĳ���
    void stringup() const; //ȫ��ɴ�д
    void stringlow() const; //ȫ���Сд
    int has(char a) const; //�ַ������ַ�a�ĸ���
};

istream& getline(istream& is, string& s, char c='\n');

#endif // STRING__H
