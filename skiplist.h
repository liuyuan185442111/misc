#ifndef _SKIPLIST_H
#define _SKIPLIST_H
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#include <functional>
#include <iterator>

template <typename Key, typename Comparator = std::less<Key> >
class SkipList
{
private:
    struct Node;

public:
    SkipList();
    ~SkipList();

    void Insert(const Key& key); // 不能重复插入
	size_t erase(const Key& key);

    bool Contains(const Key& key) const;
	int debug_get_max_height() const { return max_height_; }

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

        const Key& key() const;

        void Next();

        void Prev();

        Iterator& SeekToFirst()
		{
			node_ = list_->head_->Next(0);
			return *this;
		}

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
	Iterator lower_bound(const Key& key) const
	{
		return Iterator(this, FindGreaterOrEqual(key, NULL));
	}

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

#include "skiplist.cc"
#endif
