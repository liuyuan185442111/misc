#ifndef LTRIE_H_
#define LTRIE_H_

struct ltrie
{

struct Node
{
	char value;
	std::unordered_map<char, Node*> children;
	//std::map<char, Node*> children;
	Node *parent;
	//Node *fail = nullptr;
	bool leaf = false;
	Node() : value(0), parent(nullptr){}
	Node(char v, Node* p) : value(v), parent(p){}
};

static Node* init()
{
	return new Node;
}

static void add(Node *root, const std::string &str)
{
	for(char c : str)
	{
		if(root->children.find(c) == root->children.end())
		{
			root->children[c] = new Node(c, root);
		}
		root = root->children[c];
	}
	root->leaf = true;
}

static void del(Node *root, const std::string &str)
{
	if(Node *target = search(root, str))
	{
		if(!target->children.empty())
		{
			target->leaf = false;
			return;
		}
		while(target->parent)
		{
			if(target->parent->children.size() == 1)
			{
				Node *tmp = target->parent;
				free(target);
				//非root不用clear，因为随后他自己也会被free掉
				if(tmp == root)
					tmp->children.clear();
				target = tmp;
			}
			else
			{
				target->parent->children.erase(target->value);
				free(target);
				return;
			}
		}
	}
}

static std::string backtracking(Node *now)
{
	std::string str;
	do
	{
		str.push_back(now->value);
		now = now->parent;
	}
	while(now);
	std::reverse(str.begin(), str.end());
	return str;
}

static void bfs(Node *root)
{
	std::queue<Node*> q;
	q.push(root);
	while(!q.empty())
	{
		root = q.front();
		if(root->leaf)
		{
			cout << "leaf:" << backtracking(root) << endl;
		}
		for(const auto &child : root->children)
		{
			cout << "bfs " << child.first << endl;
			q.push(child.second);
		}
		q.pop();
	}
}

static void release(Node *root)
{
	std::queue<Node*> q;
	q.push(root);
	while(!q.empty())
	{
		root = q.front();
		for(const auto &child : root->children)
		{
			q.push(child.second);
		}
		free(root);
		q.pop();
	}
}

static bool has(Node *root, const std::string &str)
{
	for(char c : str)
	{
		if(root->children.find(c) == root->children.end())
		{
			return false;
		}
		root = root->children[c];
	}
	return root->leaf;
}

static Node* search(Node *root, const std::string &str)
{
	for(char c : str)
	{
		if(root->children.find(c) == root->children.end())
		{
			return nullptr;
		}
		root = root->children[c];
	}
	return root->leaf ? root : nullptr;
}

};

#endif
