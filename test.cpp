// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <assert.h>
#include <stdlib.h>
#include <functional>
#include <new>
#include <iterator>

template <typename Key, typename Comparator = std::less<Key> >
class SkipList
{
private:
    struct Node;

public:
    SkipList();
    ~SkipList();

    // 不能重复插入
    void Insert(const Key& key);

    bool Contains(const Key& key) const;
	int debug_get_max_height() const { return max_height_; }

    // Iteration over the contents of a skip list
    class Iterator
    {
    private:
        const SkipList* list_;
        Node* node_;

    public:
		typedef std::forward_iterator_tag iterator_category;
		typedef std::ptrdiff_t difference_type;
        explicit Iterator(const SkipList* list) : list_(list), node_(NULL) { }
        bool Valid() const { return node_ != NULL; }
		int Height()const{return node_->height;}

        // Returns the key at the current position.
        // REQUIRES: Valid()
        const Key& key() const;

        // Advances to the next position.
        // REQUIRES: Valid()
        void Next();

        // Advances to the previous position.
        // REQUIRES: Valid()
        void Prev();

        // Advance to the first entry with a key >= target
        void Seek(const Key& target);

        // Position at the first entry in list.
        // Final state of iterator is Valid() iff list is not empty.
        Iterator& SeekToFirst()
		{
			node_ = list_->head_->Next(0);
			return *this;
		}

        // Position at the last entry in list.
        // Final state of iterator is Valid() iff list is not empty.
        void SeekToLast();

		Iterator& operator++()
		{
			Next();
			return *this;
		}
		Iterator operator++(int)
		{
			Iterator tmp(*this);
			Next();
			return tmp;
		}
		bool operator==(Iterator rhs) { return list_ == rhs.list_ && node_ == rhs.node_; }
		bool operator!=(Iterator rhs) { return !(operator==(rhs)); }
    };

	Iterator begin() { return Iterator(this).SeekToFirst(); }
	Iterator end() { return Iterator(this); }

private:
    enum { kMaxHeight = 12 };

    // Immutable after construction
    Comparator const compare_;

    Node* const head_;

    //当前的最大高度, 不含, 被Insert修改
    int max_height_;

    Node* NewNode(const Key& key, int height);
    int RandomHeight();
    bool Equal(const Key& a, const Key& b) const
    {
		return !compare_(a, b) && !compare_(b, a);
    }

    // Return true if key is greater than the data stored in "n"
    bool KeyIsAfterNode(const Key& key, Node* n) const;

    // Return the earliest node that comes at or after key.
    // Return NULL if there is no such node.
    //
    // If prev is non-NULL, fills prev[level] with pointer to previous
    // node at "level" for every level in [0..max_height_-1].
    Node* FindGreaterOrEqual(const Key& key, Node** prev) const;

    // Return the latest node with a key < key.
    // Return head_ if there is no such node.
    Node* FindLessThan(const Key& key) const;

    // Return the last node in the list.
    // Return head_ if list is empty.
    Node* FindLast() const;

    // No copying allowed
    SkipList(const SkipList&);
    void operator=(const SkipList&);
};


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
inline const Key& SkipList<Key,Comparator>::Iterator::key() const
{
    assert(Valid());
    return node_->key;
}

template <typename Key, class Comparator>
inline void SkipList<Key,Comparator>::Iterator::Next()
{
    assert(Valid());
    node_ = node_->Next(0);
}

template <typename Key, class Comparator>
inline void SkipList<Key,Comparator>::Iterator::Prev()
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
inline void SkipList<Key,Comparator>::Iterator::Seek(const Key& target)
{
    node_ = list_->FindGreaterOrEqual(target, NULL);
}

template <typename Key, class Comparator>
inline void SkipList<Key,Comparator>::Iterator::SeekToLast()
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
    assert(height > 0);
    assert(height <= kMaxHeight);
    return height;
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
      head_(NewNode(0 /* any key will do */, kMaxHeight)),
      max_height_(1)
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
		free(cur);
		cur = next;
	} while(cur);
}

template <typename Key, class Comparator>
void SkipList<Key,Comparator>::Insert(const Key& key)
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
bool SkipList<Key,Comparator>::Contains(const Key& key) const
{
    Node* x = FindGreaterOrEqual(key, NULL);
    if (x != NULL && Equal(key, x->key))
    {
        return true;
    }
    else
    {
        return false;
    }
}

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

int main()
{
	srand(time(NULL));
    SkipList<int> s;
	for(int i=10; i<55; ++i)
	{
		s.Insert(i);
		print_skiplist(s);
	}
    return 0;
}
