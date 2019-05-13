#include "skiplist.h"
#include <string.h>

void print_skiplist(SkipList<int> &s)
{
	int columns = 0;
	for(SkipList<int>::Iterator it = s.begin(); it != s.end(); ++it)
		++columns;

	int max_height = s.max_height();
	int **p = new int*[max_height];
	for(int i=0; i<max_height; ++i)
	{
		p[i] = new int[columns];
		memset(p[i], 0, sizeof(int)*columns);
	}

	{
		int j = 0;
		for(SkipList<int>::Iterator it = s.begin(); it != s.end(); ++it, ++j)
		{
			for(int i=0; i<it.Height(); ++i)
			{
				p[i][j] = it.key();
			}
		}
	}

	for(int i=max_height-1; i>=0; --i)
	{
		printf("HEAD->");
		for(int j=0; j<columns; ++j)
		{
			if(p[i][j] == 0)
				printf("->");
			else
				printf("%2d", p[i][j]);
			printf("->");
		}
		printf("NULL\n");
	}
	printf("\n");

	for(int i=0; i<max_height; ++i)
	{
		delete [] p[i];
	}
	delete [] p;
}

#include <assert.h>

int main()
{
	srand(time(NULL));
    SkipList<int> s;
	for(int i=80; i>60; --i) s.insert(i);
	print_skiplist(s);
	for(int i=80; i>60; --i) s.erase(i);
	print_skiplist(s);
	for(int i=10; i<555999; ++i)
	{
		s.insert(i);
	}
	for(int i=10; i<555999; ++i)
	{
		assert(s.Contains(i));
	}
	for(int i=10; i<555999; ++i)
	{
		s.erase(i);
	}
	s.insert(666687);
	print_skiplist(s);
    return 0;
}
