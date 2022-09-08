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

#ifndef SEQUITUR_ORACLE_H
#define SEQUITUR_ORACLE_H

#include <iostream>
#include <set>
#include <list>
#include <map>
#include <stack>
#include "rules.hpp"
#include "symbols.hpp"

namespace omniscio {
namespace sequitur {

class oracle {

	private:

	friend class symbols;
	friend class rules;

	std::set<rules*> rules_set;
	std::map<std::pair<ulong,ulong>,symbols*> table;
	rules* start;
	symbols* root;

	std::set<symbols*> predictions;

	rules** R;
	int Ri;
	int64_t version; // number of modifications performed

	void find_new_predictors(symbols* s);

	int get_num_rules() const {
		return rules_set.size();
	}

	symbols* find_digram(symbols* s);

	void delete_digram(symbols* s);

	void set_digram(symbols* s);

	void print_rule(std::ostream& stream, rules* r);

	void add_prediction(symbols* s) {
		predictions.insert(s);
	}

	void remove_prediction(symbols* s) {
		predictions.erase(s);
	}

	std::list<std::stack<symbols*> > 
		build_predictor_stack_from(symbols* s) const;

	public:

	oracle() {
		start = new rules(this);
		root = new symbols(start);
		version = 0;
	}

	~oracle() {
		delete root;
		rules_set.clear();
	}

	void input(int x);

	std::set<int> predict_next() const {
		std::set<int> result;
		std::set<symbols*>::iterator it = predictions.begin();
		for(; it != predictions.end(); it++) {
			if(not (*it)->nt())
				result.insert((*it)->value());
		}
		return result;
	}

	size_t size() const;

	class iterator {
		friend class oracle;
		private:
		std::stack<symbols*> stack;
		const oracle* parent;
		int64_t version;
		iterator(const oracle* p, symbols* start = 0);
	
		class invalid_iterator : public std::exception {
                	public:
				virtual const char* what() const throw()
				{
					return "oracle has changed since creation of this iterator";
				}

		};

		public:
		iterator(const iterator&);
		~iterator();
		iterator& operator=(const iterator&);
		iterator& operator++();
		iterator operator++(int);
		int operator*() const;
		bool operator==(const iterator&);
		bool operator!=(const iterator&);
	};

	iterator begin() const {
		return iterator(this,start->first());
	}

	iterator end() const {
		return iterator(this);
	}

	std::list<iterator> predict_all() const;

	friend std::ostream& operator<<(std::ostream& stream, 
					oracle& o);
	
};

std::ostream& operator<< (std::ostream& stream, oracle& o);

}
}
#endif
