#ifndef LTRIE_H_
#define LTRIE_H_

#include <string>
#include <map>
#include <unordered_map>
#include <queue>
#include <utility>

struct ltrie
{

struct TrieNode
{
	char value;
	bool isleaf = false;
	//std::unordered_map<char, TrieNode*> children;
	std::map<char, TrieNode*> children;
	TrieNode *parent;//用于回溯和删除
	TrieNode *fail = nullptr;
	TrieNode() : value(0), parent(nullptr){}
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
			cout << "bfs " << child.first << endl;
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

};

#endif
