/**
这段程序可用于这种情况:
client发送给server一个字符串,
server校验此串是否符合预先设定的规则.

稍加修改, 也可以用来实现软件的激活码功能, 或许吧, 可能是没啥用.
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

// 生成一个十位验证码
char *generate_code(char *verify)
{
    int i = 0; // 循环变量,临时变量
    int sum = 0; // 各位之和

    static char term[11];
    if(verify == NULL)
        verify = term;
    memset(verify, 0, 11);

    // 随机生成前6位
    srand(time(NULL));
    for(i = 0; i < 6; ++i)
        sum += (verify[i] = table[rand() % 36]);

    // 使其和介于500-700之间
    while(sum < 500 || sum > 700)
        if(sum < 500) sum *= 2;
        else sum -= 100;

    // 加上0时区当前的日期
    time_t curtime;
    time(&curtime);
    struct tm *p = gmtime(&curtime);
    sum += (p->tm_mon * 100 + p->tm_mday);

    // 使其和介于1000-2000之间
    while(sum < 1000) sum *= 2;

    // 确定第七八位
    i = sum / 36;
    if(i >= 36) i -= 36;
    verify[6] = table[i];
    verify[7] = table[sum % 36];

    // 随机生成第九位
    verify[8] = table[rand() % 36];

    // 使所有之和符合某种规律,确定第十位
    for(sum = 0, i = 0; i < 9; ++i)
        sum += verify[i];
    verify[9] = table[35 - sum % 36];

    return verify;
}

// 验证码是否正确
int verify_code(char *p)
{
    int i = 0; // 循环变量,临时变量

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
