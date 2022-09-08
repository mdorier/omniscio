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

/******************************************************************************
 This file contains portions of code originating from C. Nevill-Manning's
 Sequitur (http://www.sequitur.info/sequitur_simple.cc) under the terms and
 conditions of the Apache 2.0 License, which can be found at
 https://www.apache.org/licenses/LICENSE-2.0
*******************************************************************************/

#ifndef SEQUITUR_RULES_H
#define SEQUITUR_RULES_H

#include <cstdlib> 
#include <set>

namespace omniscio {
namespace sequitur {

class symbols;
class oracle;

class rules {

	friend class symbols;
	// the guard node in the linked list of symbols that make up the rule
	// It points forward to the first symbol in the rule, and backwards
	// to the last symbol in the rule. Its own value points to the rule data
	// structure, so that symbols can find out which rule they're in

	symbols *guard;     

	// count keeps track of the number of times the rule is used 
	// in the grammar
	int count;

	std::set<symbols*> users; // keeps track of instanciations of the rule
	// this is just for numbering the rules nicely for printing; it's
	// not essential for the algorithm

	public:
	int number;

	oracle* oracle_;

	public:


	rules(oracle* o);
	~rules();

	void reuse(symbols* user) { 
		count++;
		users.insert(user);
	}

	void deuse(symbols* user) { 
		count--;
		users.erase(user);
	}

	symbols *first() const;
	symbols *last() const;

	int freq() const { return count; };
	int index() const { return number; };
	void index(int i) { number = i; };

	oracle* get_oracle() const { return oracle_; }

	std::set<symbols*>& get_users() {
		return users;
	}

	size_t length() const;
};

}
}

#endif
