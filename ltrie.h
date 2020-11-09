#ifndef LTRIE_H_
#define LTRIE_H_

#include <string>
#include <map>
#include <unordered_map>
#include <queue>
#include <utility>
#include <assert.h>

/**一个AC自动机的实现, 用于多模式字符串的匹配.
顺便支持了trie的删除操作, 但由于删除可能会导致fail指针失效, 所以不应与AC自动机混合使用.
参考自 https://www.cnblogs.com/nullzx/p/7499397.html
*/

struct ltrie
{

struct TrieNode
{
	char value;
	bool isleaf = false;
	std::unordered_map<char, TrieNode*> children;
	TrieNode *parent;//用于回溯和删除
	TrieNode *fail = nullptr;
	TrieNode() : value('^'), parent(nullptr){}
	TrieNode(char v, TrieNode* p) : value(v), parent(p){}
};

static const TrieNode* init_trie()
{
	return new TrieNode;
}

static void add_str(const TrieNode *root, const std::string &str)
{
	TrieNode* node = const_cast<TrieNode*>(root);
	for(char c : str)
	{
		auto ret = node->children.emplace(c, nullptr);
		if(ret.second)
		{
			node = new TrieNode(c, node);
			ret.first->second = node;
		}
		else
		{
			node = ret.first->second;
		}
	}
	node->isleaf = true;
}

static bool search(const TrieNode *root, const std::string &str)
{
	for(auto c : str)
	{
		auto iter = root->children.find(c);
		if(iter == root->children.end())
			return false;
		root = iter->second;
	}
	return root->isleaf;
}

static const TrieNode* find(const TrieNode *root, const std::string &str)
{
	for(auto c : str)
	{
		auto iter = root->children.find(c);
		if(iter == root->children.end())
			return nullptr;
		root = iter->second;
	}
	return root->isleaf ? root : nullptr;
}

//执行过del_str后fail指针需要rebuild
static void del_str(const TrieNode *root, const std::string &str)
{
	if(TrieNode *target = const_cast<TrieNode*>(find(root, str)))
	{
		if(!target->children.empty())
		{
			target->isleaf = false;
			return;
		}
		while(target->parent)
		{
			auto tmp = target->parent;
			if(tmp->children.size() == 1)
			{
				//非root不用clear，因为随后他自己也会被free掉
				if(tmp == root) tmp->children.clear();
				free(target);
				target = tmp;
			}
			else
			{
				tmp->children.erase(target->value);
				free(target);
				return;
			}
		}
		assert(target == root);
	}
}

static std::string backtrack(const TrieNode *node)
{
	std::string str;
	do
	{
		str.push_back(node->value);
		node = node->parent;
	} while(node);
	str.pop_back();
	std::reverse(str.begin(), str.end());
	return std::move(str);
}

static void bfs_trie(const TrieNode *root)
{
	std::queue<decltype(root)> q;
	q.push(root);
	while(!q.empty())
	{
		root = q.front();
		if(root->isleaf)
		{
			cout << "leaf " << backtrack(root) << endl;
		}
		for(auto &child : root->children)
		{
			cout << "bfs " << child.first << ",parent->" << child.second->parent->value
				<< ",failto->" << backtrack(child.second->fail) << endl;
			q.push(child.second);
		}
		q.pop();
	}
}

static void free_trie(const TrieNode *root)
{
	std::queue<decltype(root)> q;
	q.push(root);
	while(!q.empty())
	{
		root = q.front();
		for(auto &child : root->children)
		{
			q.push(child.second);
		}
		free((void*)root);
		q.pop();
	}
}

//Aho–Corasick Algorithm
static void build_fail(const TrieNode *croot)
{
	TrieNode *root = const_cast<TrieNode*>(croot);
	std::queue<TrieNode*> q;
	//将根节点的所有孩子节点的fail指向根节点并插入队列
	for(auto &child : root->children)
	{
		child.second->fail = root;
		q.push(child.second);
	}
	while(!q.empty())
	{
		auto curr = q.front();
		for(auto &child : curr->children)
		{
			//判断每个孩子节点的值是否与其failto节点的某孩子节点的值相等
			auto failto = curr->fail;
			while(true)
			{
				auto iter = failto->children.find(child.first);
				if(iter != failto->children.end())
				{
					//如找到值相等的孩子节点, 更新fail指针
					child.second->fail = iter->second;
					break;
				}
				//否则尝试继续找failto的fail节点, 直到root
				if(!failto->fail)
				{
					assert(failto == root);
					child.second->fail = root;
					break;
				}
				failto = failto->fail;
			}
			q.push(child.second);
		}
		q.pop();
	}
}

static void match_str(const TrieNode *root, const std::string &str)
{
	const TrieNode *curr = root;
	for(auto c : str)
	{
		while(true)
		{
			auto iter = curr->children.find(c);
			if(iter != curr->children.end())
			{
				//当前节点的孩子节点中有所找字符
				curr = iter->second;
				//注: backtrack(curr->fail)是backtrack(curr)的后缀
				//判断孩子节点是不是叶子节点
				if(curr->isleaf)
					cout << backtrack(curr) << endl;
				//判断孩子节点的fail节点是不是叶子节点
				if(curr->fail->isleaf)
					cout << backtrack(curr->fail) << endl;
				break;
			}
			if(curr->fail)
				curr = curr->fail;
			else
			{
				curr = root;
				break;
			}
		}
	}
}

};

#endif
