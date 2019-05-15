// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#ifndef _SKIPLIST_H
#define _SKIPLIST_H
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

    size_t count(const Key& key) const;
    void insert(const Key& key); // 不能重复插入
	size_t erase(const Key& key);
	int max_height() const { return max_height_; }

    class iterator
    {
    private:
        const SkipList* list_;
        Node* node_;

    public:
		typedef std::forward_iterator_tag iterator_category;
		typedef std::ptrdiff_t difference_type;

        iterator(const SkipList* list = NULL) : list_(list), node_(NULL) { }
		int height() const;
		const Key& operator*() const; // 如果Key将来变成pair<key,value>形式, 将返回值的const去掉
		iterator& operator++();
		iterator operator++(int);
		bool operator==(iterator rhs) { return list_ == rhs.list_ && node_ == rhs.node_; }
		bool operator!=(iterator rhs) { return !(operator==(rhs)); }

	private:
        bool Valid() const { return list_ && node_; }
        void Next();
        void Prev();
        iterator& SeekToFirst();
        void SeekToLast();
		friend SkipList;
    };

	iterator begin() { return iterator(this).SeekToFirst(); }
	iterator end() { return iterator(this); }
	iterator lower_bound(const Key& key) const
	{
		return iterator(this, FindGreaterOrEqual(key, NULL));
	}
	iterator upper_bound(const Key& key) const;

private:
    enum { kMaxHeight = 12 };
    Comparator const compare_;
    Node* const head_;
    int max_height_; // 当前的最大高度

	// alloc a new Node
    Node* NewNode(const Key& key, int height);
    int RandomHeight();
    bool Equal(const Key& a, const Key& b) const;

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

#include "skiplist.cc"
#endif
