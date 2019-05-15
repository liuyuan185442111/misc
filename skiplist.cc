#ifdef DEBUG
#include <assert.h>
#else
#define assert(...)
#endif

#include <stdlib.h> // for rand()

template <typename Key, class Comparator>
struct SkipList<Key,Comparator>::Node
{
    explicit Node(const Key& k, int h) : key(k), height(h) { }

    Key const key;
	int height;

    Node* Next(int n)
    {
        assert(n >= 0);
        return next_[n];
    }
    void SetNext(int n, Node* x)
    {
        assert(n >= 0);
        next_[n] = x;
    }

private:
    // Array of length equal to the node height. next_[0] is lowest level link.
    Node *next_[1];
};

template <typename Key, class Comparator>
typename SkipList<Key,Comparator>::Node*
SkipList<Key,Comparator>::NewNode(const Key& key, int height)
{
    void* mem = malloc(sizeof(Node) + sizeof(Node *) * (height - 1));
    return new (mem) Node(key, height);
}

template <typename Key, class Comparator>
inline int SkipList<Key,Comparator>::iterator::height() const
{
	assert(Valid());
	return node_->height;
}

template <typename Key, class Comparator>
inline const Key& SkipList<Key,Comparator>::iterator::operator*() const
{
    assert(Valid());
    return node_->key;
}

template <typename Key, class Comparator>
inline typename SkipList<Key,Comparator>::iterator& SkipList<Key,Comparator>::iterator::operator++()
{
	Next();
	return *this;
}

template <typename Key, class Comparator>
inline typename SkipList<Key,Comparator>::iterator SkipList<Key,Comparator>::iterator::operator++(int)
{
	iterator tmp(*this);
	Next();
	return tmp;
}

template <typename Key, class Comparator>
inline void SkipList<Key,Comparator>::iterator::Next()
{
    assert(Valid());
    node_ = node_->Next(0);
}

template <typename Key, class Comparator>
inline void SkipList<Key,Comparator>::iterator::Prev()
{
    // Instead of using explicit "prev" links, we just search for the
    // last node that falls before key.
    assert(Valid());
    node_ = list_->FindLessThan(node_->key);
    if (node_ == list_->head_)
    {
        node_ = NULL;
    }
}

template <typename Key, class Comparator>
inline typename SkipList<Key,Comparator>::iterator& SkipList<Key,Comparator>::iterator::SeekToFirst()
{
	node_ = list_->head_->Next(0);
	return *this;
}

template <typename Key, class Comparator>
inline void SkipList<Key,Comparator>::iterator::SeekToLast()
{
    node_ = list_->FindLast();
    if (node_ == list_->head_)
    {
        node_ = NULL;
    }
}

template <typename Key, class Comparator>
int SkipList<Key,Comparator>::RandomHeight()
{
    // Increase height with probability 1 in kBranching
    static const unsigned int kBranching = 4;
    int height = 1;
    while (height < kMaxHeight && ((rand() % kBranching) == 0))
    {
        height++;
    }
    return height;
}

template <typename Key, class Comparator>
bool SkipList<Key,Comparator>::Equal(const Key& a, const Key& b) const
{
	return !compare_(a, b) && !compare_(b, a);
}

template <typename Key, class Comparator>
bool SkipList<Key,Comparator>::KeyIsAfterNode(const Key& key, Node* n) const
{
    // NULL n is considered infinite
    return (n != NULL) && compare_(n->key, key);
}

template <typename Key, class Comparator>
typename SkipList<Key,Comparator>::Node* SkipList<Key,Comparator>::FindGreaterOrEqual(const Key& key, Node** prev) const
{
    Node* x = head_;
    int level = max_height_ - 1;
	if (level < 0) return NULL;
    while (true)
    {
        Node* next = x->Next(level);
        if (KeyIsAfterNode(key, next))
        {
            // Keep searching in this list
            x = next;
        }
        else
        {
            if (prev != NULL) prev[level] = x;
            if (level == 0)
            {
                return next;
            }
            else
            {
                // Switch to next list
                level--;
            }
        }
    }
}

template <typename Key, class Comparator>
typename SkipList<Key,Comparator>::Node*
SkipList<Key,Comparator>::FindLessThan(const Key& key) const
{
    Node* x = head_;
    int level = max_height_ - 1;
	if (level < 0) return NULL;
    while (true)
    {
        assert(x == head_ || compare_(x->key, key));
        Node* next = x->Next(level);
        if (next == NULL || !compare_(next->key, key))
        {
            if (level == 0)
            {
                return x;
            }
            else
            {
                // Switch to next list
                level--;
            }
        }
        else
        {
            x = next;
        }
    }
}

template <typename Key, class Comparator>
typename SkipList<Key,Comparator>::Node* SkipList<Key,Comparator>::FindLast() const
{
    Node* x = head_;
    int level = max_height_ - 1;
	if (level < 0) return NULL;
    while (true)
    {
        Node* next = x->Next(level);
        if (next == NULL)
        {
            if (level == 0)
            {
                return x;
            }
            else
            {
                // Switch to next list
                level--;
            }
        }
        else
        {
            x = next;
        }
    }
}

template <typename Key, class Comparator>
SkipList<Key,Comparator>::SkipList()
    : compare_(Comparator()),
      head_(NewNode(Key() /* any key will do */, kMaxHeight)),
      max_height_(0)
{
    for (int i = 0; i < kMaxHeight; i++)
    {
        head_->SetNext(i, NULL);
    }
}

template <typename Key, class Comparator>
SkipList<Key,Comparator>::~SkipList()
{
	Node *cur = head_;
	Node *next;
	do
	{
		next = cur->Next(0);
		cur->~Node();
		free(cur);
		cur = next;
	} while(cur);
}

template <typename Key, class Comparator>
void SkipList<Key,Comparator>::insert(const Key& key)
{
    Node* prev[kMaxHeight];
    Node* x = FindGreaterOrEqual(key, prev);

    // Our data structure does not allow duplicate insertion
    assert(x == NULL || !Equal(key, x->key));

    int height = RandomHeight();
    if (height > max_height_)
    {
        for (int i = max_height_; i < height; i++)
        {
            prev[i] = head_;
        }
        max_height_ = height;
    }

    x = NewNode(key, height);
    for (int i = 0; i < height; i++)
    {
        x->SetNext(i, prev[i]->Next(i));
        prev[i]->SetNext(i, x);
    }
}

template <typename Key, class Comparator>
size_t SkipList<Key,Comparator>::erase(const Key& key)
{
    Node* prev[kMaxHeight];
    Node* x = FindGreaterOrEqual(key, prev);
    if(x == NULL || !Equal(key, x->key)) return 0;

	int i = 0;
	for (; i < x->height; i++)
	{
		if(prev[i] == head_ && x->Next(i) == NULL)
		{
			prev[i]->SetNext(i, NULL);
			max_height_ = i;
			break;
		}
		prev[i]->SetNext(i, x->Next(i));
	}
	for (; i < x->height; i++)
		prev[i]->SetNext(i, NULL);

	x->~Node();
	free(x);
}

template <typename Key, class Comparator>
size_t SkipList<Key,Comparator>::count(const Key& key) const
{
    Node* x = FindGreaterOrEqual(key, NULL);
    if (x != NULL && Equal(key, x->key))
        return 1;
    else
        return 0;
}

#ifndef DEBUG
#undef assert
#endif
