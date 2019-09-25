#include <unistd.h>
#include <stdio.h>

//15.3 popen和pclose函数
//函数popen先执行fork, 然后调用exec以执行cmd, 并且返回一个文件指针.
int main()
{
	char line[1024];
	FILE *f = popen("ls -l", "r");
	if(f == NULL) return -1;
	while(fgets(line, 1024, f) != NULL)
		printf("%s", line);
	return pclose(f)>>8;
}
