#include "base64.h"
#include <iostream>
#include <cstring>
using namespace std;
int main()
{
    char in[] = "1234567890!@#$%^&*()_+";
    char out[256];
    memset(out,0,256);
    base64_encode(out,in,strlen(in));
    cout << out << endl;
    base64_decode(in, out, strlen(out));
    cout << in << endl;
    return 0;
}
