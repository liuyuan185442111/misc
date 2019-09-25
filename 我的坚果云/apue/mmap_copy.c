#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
int main(int argc, char *argv[])
{
	int fdin,fdout;
	void *src,*dst;
	struct stat statbuf;
	if(argc != 3)
	{
		printf("copy usage: %s <fromfile> <tofile>\n", argv[0]);
		return 0;
	}
	if((fdin = open(argv[1], O_RDONLY)) < 0)
		return -1;
	if(fstat(fdin, &statbuf) < 0)
		return -2;
	if((fdout = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, statbuf.st_mode)) < 0)
		return -3;
	ftruncate(fdout, statbuf.st_size);

	if((src = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0)) == MAP_FAILED)
		return -4;
	if((dst = mmap(0, statbuf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fdout, 0)) == MAP_FAILED)
		return -5;

	close(fdin);
	close(fdout);
	memcpy(dst, src, statbuf.st_size);
	msync(dst, statbuf.st_size, MS_SYNC);
	puts("copy successfully");
	return 0;
}
