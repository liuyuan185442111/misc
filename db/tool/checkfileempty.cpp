#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>
#include "page.h"

int check_file_empty(const char *db_file)
{
	struct stat statbuf;
	int fd = open(db_file, O_RDONLY);
	if(fd < 0) return -1;
	if(fstat(fd, &statbuf) == -1) return -1;
	void *file = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if(file == MAP_FAILED) return -1;

	using lcore::page_index_t;
	using lcore::PAGESIZE;
	using lcore::PAGEUSED;
	char *ptr = (char *)file;
	page_index_t root_index_idx = *(page_index_t *)(ptr + sizeof(page_index_t));
	page_index_t max_page_idx = *(page_index_t *)(ptr + 2*sizeof(page_index_t));
	printf("file:%s, root_index_idx:%u, max_page_idx:%u\n", db_file,root_index_idx,max_page_idx);

	ptr += PAGEUSED + PAGESIZE;
	page_index_t t = 1;
	for( ; t<root_index_idx; ++t)
	{
		assert(*(page_index_t *)ptr == t);
		if(*((page_index_t *)ptr+1) != lcore::FREE_PAGE) return 1;
		ptr += PAGESIZE;
	}
	assert(*(page_index_t *)ptr == t);
	assert(*((page_index_t *)ptr+1) == lcore::INDEX_PAGE);
	for(++t; t<max_page_idx; ++t)
	{
		ptr += PAGESIZE;
		assert(*(page_index_t *)ptr == t);
		if(*((page_index_t *)ptr+1) != lcore::FREE_PAGE) return 1;
	}
	munmap(file, statbuf.st_size);
	return 0;
}
