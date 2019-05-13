#include "skiplist.h"
#include <string.h>
void print_skiplist(SkipList<int> &s)
{
	int columns = 0;
	SkipList<int>::Iterator it(&s);
	for(SkipList<int>::Iterator it = s.begin(); it != s.end(); ++it)
		++columns;

	int max_height_ = s.debug_get_max_height();
	int **p = new int*[max_height_];
	for(int i=0; i<max_height_; ++i)
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

	for(int i=max_height_-1; i>=0; --i)
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

	for(int i=0; i<max_height_; ++i)
	{
		delete [] p[i];
	}
	delete [] p;
}

#include <unistd.h>
#include <set>
#include <assert.h>
using namespace std;
int test_std()
{
	srand(99999);
	set<int> s;
	for(int i=10; i<555999; ++i)
	{
		s.insert(i);
	}
	for(int i=10; i<555999; ++i)
	{
		assert(s.count(i));
	}
	for(int i=10; i<555999; ++i)
	{
		s.erase(i);
	}
	s.insert(666687);
	return 0;
}

int main()
{
	//return test_std();
	//srand(time(NULL));
	srand(99999);
    SkipList<int> s;
	for(int i=10; i<555999; ++i)
	{
		s.Insert(i);
	}
	for(int i=10; i<555999; ++i)
	{
		assert(s.Contains(i));
	}
	for(int i=10; i<555999; ++i)
	{
		s.erase(i);
	}
	s.Insert(666687);
	print_skiplist(s);
	return 0;
	print_skiplist(s);
	s.erase(14);
	print_skiplist(s);
	s.erase(15);
	print_skiplist(s);
	s.erase(30);
	print_skiplist(s);
	s.erase(28);
	print_skiplist(s);
    return 0;
}
