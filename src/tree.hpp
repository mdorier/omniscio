/******************************************************************************
 Copyright (c) 2014 ENS Rennes, Inria Rennes Bretagne Atlantique
 All rights reserved.
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 * Neither the name of the University of California, Berkeley nor the
   names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#ifndef OMNISCIO_TREE_H
#define OMNISCIO_TREE_H

#include <iterator>

namespace omniscio {

template<typename T>
class tree {

public:

	class iterator;

private:

	class node {

		friend class iterator;
		friend class tree;

	private:
		T val;
		node* parent;
		node* first_child;
		node* last_child;
		node* prev_sibling;
		node* next_sibling;
		
	public:

		node(const T& t) : val(t), parent(0) {}

		node(const T& t, node* p) : val(t), parent(p) {}

		const T& value() const {
			return val;
		}

		T& value() {
			return val;
		}

		~node() {
			node* n = first_child;
			while(n != NULL) {
				node* m = n->next_sibling;
				delete n;
				n = m;
			}
		}
		
	};

public:

	class iterator {

	friend class tree;

	private:
		
		node* n;

		iterator(node *n_) : n(n_) {}

	public:

		typedef std::forward_iterator_tag iterator_category;
		typedef T value_type;
		typedef int difference_type;
		typedef T* pointer;
		typedef T& reference;

		iterator() : n(NULL) {}

		iterator(const iterator& it) : n(it.n) {}

		iterator& operator++() {
			n = n->next_sibling;
			return *this;
		}

		iterator operator++(int) {
			iterator tmp(*this);
			operator++();
			return tmp;
		}

		bool operator==(const iterator& rhs) const { 
			return (n == rhs.n);
		}

		bool operator!=(const iterator& rhs) const {
			return (n != rhs.n);
		}

		T& operator*() {
			return n->value();
		}

		const T& operator*() const {
			return n->value();
		}

		T* operator->() {
			return &(n->value());
		}

		const T* operator->() const {
			return &(n->value());
		}

		iterator begin() const {
			return iterator(n->first_child);
		}

		iterator end() const {
			return iterator(NULL);
		}

		iterator append_child(const T& t) const {
			node* c = new node(t,n);
			c->parent = n;
			if(n->last_child == NULL) {
				n->last_child = c;
				n->first_child = c;
			} else {
				node* old_last_child = n->last_child;
				old_last_child->next_sibling = c;
				c->prev_sibling = old_last_child;
				n->last_child = c;
			}
			return iterator(c);
		}

	};

private:

	node* first_root;
	node* last_root;

public:

	tree() : first_root(NULL), last_root(NULL) {}

	iterator add_root(const T& t) {
		node* n = NULL;
		if(first_root == NULL) {
			first_root = new node(t,NULL);
			last_root = first_root;
			n = first_root;
		} else {
			n = new node(t,NULL);
			n->prev_sibling = last_root;
			last_root->next_sibling = n;
			last_root = n;
		}
		return iterator(n);
	}

	iterator begin() {
		return iterator(first_root);
	}

	iterator end() {
		return iterator(NULL);
	}

	bool is_empty() const {
		return first_root == NULL;
	}

	~tree() {
		node* n = first_root;
		while(n != NULL) {
			node* m = n->next_sibling;
			delete n;
			n = m;
		}
	}

};

}

#endif
