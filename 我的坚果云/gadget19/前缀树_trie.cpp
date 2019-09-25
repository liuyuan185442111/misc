#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//前缀树的简单实现, 包括insert,search,travel,和子串查找
//统计单词出现的次数

//假设全部是小写字母组成的单词
#define ALPHABET_SIZE 26

typedef struct _trie_node
{
	int count;
	struct _trie_node *children[ALPHABET_SIZE];
} *trie_node;

trie_node create_trie_node()
{
	trie_node pNode = (trie_node)malloc(sizeof(struct _trie_node));
	memset(pNode, 0, sizeof(struct _trie_node));
	return pNode;
}

void trie_insert(trie_node root, const char* key)
{
	while(*key)
	{
		if(root->children[*key-'a'] == NULL)
		{
			root->children[*key-'a'] = create_trie_node();
		}
		root = root->children[*key-'a'];
		++key;
	}
	root->count += 1;
}

int trie_search(trie_node root, const char* key)
{
	while(*key && root!=NULL)
	{
		root = root->children[*key-'a'];
		++key;
	}
	return root ? root->count : 0;
}

//与key同前缀的子串是否在root表示的trie中
//比如key是password, 如果trie中有pass, 则会返回大于0的整数
int trie_hassub(trie_node root, const char* key)
{
	while(*key && root!=NULL)
	{
		if(root->count > 0) return root->count;
		root = root->children[*key-'a'];
		++key;
	}
	return root ? root->count : 0;
}

//以上c环境下即可编译, 以下需要c++的stack和string
#include <stack>
#include <string>
void do_travel(trie_node root)
{
	std::stack<std::pair<trie_node, std::string>> stk;
	stk.push(std::make_pair(root, ""));
	while(!stk.empty())
	{
		auto now = std::move(stk.top());
		stk.pop();
		for(auto e = now.first->children+ALPHABET_SIZE-1, b = now.first->children, c = e; c >= b; --c)
		{
			if(*c)
			{
				if((*c)->count > 0)
					printf("%s -- %d\n", (now.second+char(c-b+'a')).c_str(),  (*c)->count);
				stk.push(std::make_pair(*c, now.second+char(c-b+'a')));
			}
		}
	}
}

int main()
{
	trie_node root = create_trie_node();
	const char *keys[] = {"the", "a", "there", "answer", "any", "by", "bye", "their", "the"};

	size_t n = sizeof(keys)/sizeof(char*);
	while(n != 0) trie_insert(root, keys[--n]);

	puts("travel:");
	do_travel(root);

	puts("\nsearch:");
	printf("%s --- %d\n", "the", trie_hassub(root, "thethread"));
	printf("%s --- %d\n", "the", trie_search(root, "the"));
	printf("%s --- %d\n", "these", trie_search(root, "these"));
	printf("%s --- %d\n", "their", trie_search(root, "their"));
	printf("%s --- %d\n", "thaw", trie_search(root, "thaw"));

	return 0;
}
