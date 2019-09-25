#include <time.h>
#include <sys/utsname.h>
//like "uname -a"
int main()
{
    char line[1024];
    time_t caltime = time(NULL);
    struct tm *tm=localtime(&caltime);
    strftime(line, 1024,"%a %b %d %X %Z %Y",tm);
    puts(line);

    struct utsname utsname;
    uname(&utsname);
    puts(utsname.sysname);
    puts(utsname.nodename);
    puts(utsname.release);
    puts(utsname.version);
    puts(utsname.machine);
    return 0;
}
