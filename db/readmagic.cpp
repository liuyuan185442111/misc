#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
using namespace std;
#include "performance.h"
#include "page.h"//for PAGESIZE
using lcore::Performance;
using lcore::PAGESIZE;

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		cout << "usage: " << argv[0] << " dbfile\n";
		return 0;
	}

	int fd;
	if((fd = open(argv[1], O_RDONLY)) == -1)
	{
		fprintf(stderr, "open %s failed: %s\n", argv[1], strerror(errno));
		return -1;
	}

	char data[PAGESIZE];
	if(strstr(argv[1], ".okintlog")) lseek(fd, -PAGESIZE, SEEK_END);
	if(read(fd, data, PAGESIZE) != PAGESIZE)
	{
		fprintf(stderr, "read %s failed: %s\n", argv[1], strerror(errno));
		return -2;
	}

	unsigned int *p = (unsigned int *)data;
	cout << "free_page_list: " << *p++ << endl;
	cout << "root_index_idx: " << *p++ << endl;
	cout << "max_page_idx: " << *p++ << endl;
	cout << "last_check: " << *p++ << endl;
	cout << "logger_id: " << *p++ << endl;
	((Performance *)p)->dump();

	return 0;
}
