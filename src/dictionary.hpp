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

#ifndef OMNISCIO_DICTIONARY_H
#define OMNISCIO_DICTIONARY_H

#include <exception>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "tree.hpp"
#include "omniscio.h"

namespace omniscio {

/**
 * The dictionary class represents a dictionary, that is, a
 * tree structure to associate a sequence of symbols of type T
 * with a unique identifier of type I.
 * 
 * Requirements for type T:
 * - Must have == implemented
 * - Must be copy-constructible
 *
 * Requirements for type I:
 * - Must have == implemented
 * - Must have +(int) implemented
 * - Must be copy-constructible
 *
 * In other parts of Omnisc'IO, the disctionary is used
 * to associate stack traces with integers.
 */
template<typename T, typename I>
class dictionary {

	typedef T letter_type;
	typedef I index_type;
	
	struct node {
		T letter;
		I index;

		bool operator==(const node& other) const {
			return letter == other.letter;
		}
	};

	I last_index;
	I null_index;
	tree<node> content;

	std::ofstream file;
	bool opened;
	I last_written;

	I generate_next_index() {
		I i = last_index;
		last_index += 1;
		return i;
	}

	template<typename ITERATOR>
	I insert_recursive(typename tree<node>::iterator n,
		const ITERATOR& start, const ITERATOR& end)
	{
		node current;
		current.letter = *start;
		current.index  = null_index;

		typename tree<node>::iterator sbegin = n.begin();
		typename tree<node>::iterator send   = n.end();
		typename tree<node>::iterator found  = std::find(sbegin,send,current);

		if(found == send) {
			if(start+1 == end) {
				current.index = generate_next_index();
				n.append_child(current);
				return current.index;
			} else {
				typename tree<node>::iterator c = 
					n.append_child(current);
				return insert_recursive(c,start+1,end);
			}
		} else {
			if(start+1 == end) {
				if(found->index == null_index) {
					found->index = generate_next_index();
				}
				return found->index;
			} else {
				return insert_recursive(found,start+1,end);
			}
		}
	}

	class empty_container : public std::exception {

		public:

		virtual const char* what() const throw()
		{
			return "empty container";
		}

	};
	
	public:

	/**
	 * Creates a dictionary.
 	 */
	dictionary() : opened(false) {
		last_index = null_index + 1;
	}

	/**
	 * Creates a dictionary and open a file to store it.
	 * Erases the file if the file already exists. The file
	 * will be updated every time a new entry is added to the
	 * dictionary.
 	 *
	 * \param[in] filename : name of the file in which to store
	 *			 the dictionary.
	 */
	dictionary(const std::string& filename) : opened(false) {
		open(filename);
		last_index = null_index + 1;
	}

	/**
	 * Inserts a sequence into the dictionary by providing
	 * the beginning and the end iterators of the sequence.
	 * Throws empry_container if start == end.
	 *
	 * WARNING: this version will not write the symbol in the file.
	 *
	 * \param[in] start : start iterator of the sequence.
	 * \param[in] end : end iterator of the sequence.
	 * \return the identifier of the stored sequence.
	 */
	template<typename ITERATOR>
	I insert(const ITERATOR& start,
		 const ITERATOR& end) throw(empty_container) {
		if(start == end) throw empty_container();
		// first time we store something
		if(content.is_empty()) {
			node n;
			n.letter = *start;
			n.index = null_index;
			typename tree<node>::iterator c =
				content.add_root(n);
			if(start+1 == end) {
				c->index = generate_next_index();
				return c->index;
			}
		} 
		return insert_recursive(content.begin(),start,end);
	}

	/**
	 * Inserts a sequence into the dictionary by providing
	 * a container from which to read the sequence.
	 * Throws empry_container if the container is empty.
	 *
	 * \param[in] container : the container from which to
	 * read the sequence. Must provide begin() and end() iterators.
	 * \return the identifier of the stored sequence.
	 */
	template<typename C>
	I insert(const C& container) throw(empty_container) {
		I i = insert(container.begin(),container.end());
		if(i > last_written && opened) {
			OMNISCIO_UNTRACED_START;
			file << "[" << i << "]:";
			file << container;
			file << std::endl;
			OMNISCIO_UNTRACED_END;
			last_written = i;
		}
		return i;
	}

	/**
	 * Opens a file in which to store the entries of
	 * the dictionary.
	 * 
	 * \param[in] filename : name of the file to open.
	 * \return true in case of success, false otherwise.
	 */
	bool open(const std::string& filename) {
		if(not opened) {
			OMNISCIO_UNTRACED_START;
			file.open(filename.c_str());
			opened = true;
			OMNISCIO_UNTRACED_END;
			return true;
		} else {
			return false;
		}
	}

	/**
	 * Close the file associated with this dictionary.
	 *
	 * \return true in case of success, false otherwise.
	 */
	bool close() {
		if(opened) {
			OMNISCIO_UNTRACED_START;
			file.close();
			OMNISCIO_UNTRACED_END;
			opened = false;
			return true;
		} else {
			return false;
		}
	}
};

}

#endif
