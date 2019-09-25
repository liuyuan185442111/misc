#include <cstdio>
#include <hash_map>

char table[]=
{
    '0','1','2','3','4','5','6','7','8','9','<','>',
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z'
};
__gnu_cxx::hash_map<unsigned char, unsigned char> retable;

// 一定是4字节的整数倍
int read4chars(FILE *fin, unsigned char *ret)
{
    unsigned char buf[4];
    int t = fgetc(fin);
    if(t == EOF) return -1;
    buf[0] = t;
    buf[1] = fgetc(fin);
    buf[2] = fgetc(fin);
    buf[3] = fgetc(fin);

    unsigned char a = retable[buf[0]];
    unsigned char b = retable[buf[1]];
    unsigned char c = retable[buf[2]];
    unsigned char d = retable[buf[3]];
    ret[0] = a<<2 | b>>4;
    ret[1] = b<<4 | c>>2;
    ret[2] = c<<6 | d;

    return 0;
}

int main()
{
    for(unsigned char i=0; i<64; ++i)
        retable.insert(std::make_pair(table[i], i));

    FILE *fin = fopen("dest.txt", "r");
    if(fin == NULL)
    {
        puts("cann't open dest.txt!");
        getchar();
        return -1;
    }
    FILE *fout = fopen("dest.rar", "wb");
    for(;;)
    {
        unsigned char buf[3];
        int res = read4chars(fin, buf);
        if(res == -1) break;
        fputc(buf[0], fout);
        fputc(buf[1], fout);
        fputc(buf[2], fout);
    }

    fclose(fin);
    fclose(fout);
    return 0;
}
