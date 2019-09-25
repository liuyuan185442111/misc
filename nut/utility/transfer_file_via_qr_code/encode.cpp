#include <stdio.h>
#include <string.h>

unsigned char table[]=
{
    '0','1','2','3','4','5','6','7','8','9','<','>',
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z'
};

// 当到达文件尾不足3个字符时, 末尾补0, 最多会有2个空字符, 对rar文件无影响
int read3chars(FILE *fin, unsigned char *re)
{
    int t;
    unsigned char buf[3];
    memset(buf, 0, 3*sizeof(unsigned char));

    t = fgetc(fin);
    if(t == EOF) return 1;
    buf[0] = t;
    t = fgetc(fin);
    if(t == EOF) goto last;
    buf[1] = t;
    t = fgetc(fin);
    if(t == EOF) goto last;
    buf[2] = t;

last:
    unsigned char a = (buf[0]&0xFC)>>2;
    unsigned char b = (buf[0]&0x03)<<4 | (buf[1]&0xF0)>>4;
    unsigned char c = (buf[1]&0x0F)<<2 | buf[2]>>6;
    unsigned char d = buf[2]&0x3F;
    re[0] = table[a];
    re[1] = table[b];
    re[2] = table[c];
    re[3] = table[d];

    return 0;
}

int main()
{
    FILE *fin = fopen("src.rar", "rb");
    if(fin == NULL)
    {
        puts("cann't open src.rar!");
        getchar();
        return -1;
    }
    int order = 101;
    char dest[] = "100.txt";
    sprintf(dest, "%d.txt", order);
    FILE *fout = fopen(dest, "w");
    fputc('+', fout);
    fputc(dest[0], fout);
    fputc(dest[1], fout);
    fputc(dest[2], fout);
    for(int i=1;;++i)
    {
        unsigned char buf[4];
        int ret = read3chars(fin, buf);
        if(ret == 1) break;
        fputc(buf[0], fout);
        fputc(buf[1], fout);
        fputc(buf[2], fout);
        fputc(buf[3], fout);
        if(i == 255)
        {
            i = 0;
            fclose(fout);
            sprintf(dest, "%d.txt", ++order);
            fout = fopen(dest, "w");
            fputc('+', fout);
            fputc(dest[0], fout);
            fputc(dest[1], fout);
            fputc(dest[2], fout);
        }
    }
    fclose(fin);
    fclose(fout);
    return 0;
}
