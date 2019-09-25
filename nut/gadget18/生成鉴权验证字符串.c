/**
��γ���������������:
client���͸�serverһ���ַ���,
serverУ��˴��Ƿ����Ԥ���趨�Ĺ���.

�Լ��޸�, Ҳ��������ʵ������ļ����빦��, �����, ������ûɶ��.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char table[36] =
{
    '0','1','2','3','4','5','6','7','8','9',
    'a','b','c','d','e','f','g','h','i','j',
    'k','l','m','n','o','p','q','r','s','t',
    'u','v','w','x','y','z'
};

// ����һ��ʮλ��֤��
char *generate_code(char *verify)
{
    int i = 0; // ѭ������,��ʱ����
    int sum = 0; // ��λ֮��

    static char term[11];
    if(verify == NULL)
        verify = term;
    memset(verify, 0, 11);

    // �������ǰ6λ
    srand(time(NULL));
    for(i = 0; i < 6; ++i)
        sum += (verify[i] = table[rand() % 36]);

    // ʹ��ͽ���500-700֮��
    while(sum < 500 || sum > 700)
        if(sum < 500) sum *= 2;
        else sum -= 100;

    // ����0ʱ����ǰ������
    time_t curtime;
    time(&curtime);
    struct tm *p = gmtime(&curtime);
    sum += (p->tm_mon * 100 + p->tm_mday);

    // ʹ��ͽ���1000-2000֮��
    while(sum < 1000) sum *= 2;

    // ȷ�����߰�λ
    i = sum / 36;
    if(i >= 36) i -= 36;
    verify[6] = table[i];
    verify[7] = table[sum % 36];

    // ������ɵھ�λ
    verify[8] = table[rand() % 36];

    // ʹ����֮�ͷ���ĳ�ֹ���,ȷ����ʮλ
    for(sum = 0, i = 0; i < 9; ++i)
        sum += verify[i];
    verify[9] = table[35 - sum % 36];

    return verify;
}

// ��֤���Ƿ���ȷ
int verify_code(char *p)
{
    int i = 0; // ѭ������,��ʱ����

    if(strlen(p) != 10) return 0;

    int sum = 0;
    for(; i < 6; ++i)
        sum += p[i];

    while(sum < 500 || sum > 700)
        if(sum < 500) sum *= 2;
        else sum -= 100;

    time_t curtime;
    time(&curtime);
    struct tm * t = gmtime(&curtime);
    sum += (t->tm_mon * 100 + t->tm_mday);

    while(sum < 1000) sum *= 2;

    i = sum / 36;
    if(i >= 36) i -= 36;
    if(p[6] != table[i]) return 0;
    if(p[7] != table[sum % 36]) return 0;

    for(sum = 0, i = 0; i < 9; ++i)
        sum += p[i];

    i = 0;
    while(i < 36) if(p[9] == table[i++]) break;
    sum += i;
    if(sum % 36) return 0;

    return 1;
}

int main()
{
    char *p;
    for(;;)
    {
        p = generate_code(NULL);
        puts(p);
        if(!verify_code(p)) break;
        _sleep(1000);
    }
    return 0;
}
