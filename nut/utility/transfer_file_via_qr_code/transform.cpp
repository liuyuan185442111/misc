#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
using std::string;
using std::map;
using std::vector;

int main()
{
    FILE *fin = fopen("mid.txt", "r");
    if(fin == NULL)
    {
        puts("cann't open mid.txt!");
        getchar();
        return -1;
    }
    map<int,string> codes;
    char buf[1028];
    while(fgets(buf, sizeof(buf), fin))
    {
        if(buf[0] != '+') continue;
        if(buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = 0;
        char order_buf[4];
        order_buf[0]=buf[1];
        order_buf[1]=buf[2];
        order_buf[2]=buf[3];
        order_buf[3]=0;
        int order = atoi(order_buf);
        codes[order] = buf+4;
    }
    fclose(fin);

    printf("map\'s size is %d\n", codes.size());
    if(codes.empty()) return -1;
    int first = codes.begin()->first;
    int last = codes.rbegin()->first;
    printf("first is %d, last is %d\n", first, last);
    if(last-first != codes.size()-1)
    {
        int *full = new int[last-first+1];
        for(int i=0; i<last-first+1; ++i) full[i]=first+i;
        vector<int> now(codes.size());
        int i(0);
        for(map<int,string>::iterator it(codes.begin()); it!=codes.end(); ++it,++i)
            now[i] = it->first;
        vector<int> result(last-first+1);
        std::set_difference(full,full+(last-first+1),now.begin(),now.end(),result.begin());
        delete[] full;
        for(vector<int>::iterator it(result.begin()); it!=result.end(); ++it)
        {
            if(*it == 0)
            {
                system("pause");
                return 0;
            }
            printf("%d\n", *it);
        }
    }

    FILE *fout = fopen("dest.txt", "w");
    for(map<int,string>::iterator it = codes.begin(); it != codes.end(); ++it)
        fputs(it->second.c_str(), fout);
    fclose(fout);
    system("pause");
    return 0;
}
