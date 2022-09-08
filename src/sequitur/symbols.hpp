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

#ifndef SEQUITUR_SYMBOLS_H
#define SEQUITUR_SYMBOLS_H

#include <iostream>
#include "rules.hpp"

namespace omniscio {
namespace sequitur {

typedef unsigned long ulong;

class symbols {

	symbols *n, *p;
	ulong s;

	rules* owner; // indicates in which rule this symbol appears
	bool is_predictor;

	std::set<symbols*> predictors;

	public:

	// initializes a new terminal symbol
	// sym = the symbol's value
	// o = rule owning this symbol (0 by default if the symbol is not owned)
	symbols(ulong sym, rules* o = (rules*)0) {
		s = sym * 2 + 1; // an odd number, so that they're a distinct
		// space from the rule pointers, which are 4-byte aligned
		p = n = 0;
		owner = o;
		is_predictor = false;
		next_updated = false;
	}

	// initializes a new non-terminal symbol
	// r = the rule that is instanciated
	// o = rule owning this symbol (0 by default if the symbol is not owned)
	symbols(rules *r, rules* o = (rules*)0) {
		s = (ulong) r;
		p = n = 0;
		rule()->reuse(this);
		owner = o;
		is_predictor = false;
		next_updated = false;
	}

	symbols* find_digram();

	void set_digram();

	bool is_pred() const {
		return is_predictor;
	}

	// links 2 symbols together, removing any old digram from the hash table
	static void join(symbols *left, symbols *right) {
		if (left->n && (not left->is_guard())) {
			left->delete_digram();

		// This is to deal with triples, where we only record the second
		// pair of the overlapping digrams. When we delete the 2nd pair,
		// we insert the first pair into the hash table so that we don't
		// forget about it.  e.g. abbbabcbb

			if (right->p && right->n &&
					right->raw_value() == right->p->raw_value() &&
					right->raw_value() == right->n->raw_value()) {
				right->set_digram();
			}

			if (left->p && left->n &&
					left->raw_value() == left->n->raw_value() &&
					left->raw_value() == left->p->raw_value()) {
				left->p->set_digram();
			}
		}

		// update the new owner of the right symbol 
		right->owner = left->owner;
		left->n = right; right->p = left;
	}

	// cleans up for symbol deletion: removes hash table entry and 
	// decrements rule reference count
	~symbols(); 

	// inserts a symbol after this one.
	void insert_after(symbols *y) {
		join(y, n);
		join(this, y);
	};

	// removes the digram from the hash table
	void delete_digram();

	// true if this is the guard node marking the beginning/end of a rule
	int is_guard() { return nt() && rule()->first()->prev() == this; };

	// nt() returns true if a symbol is non-terminal.
	int nt() { return ((s % 2) == 0) && (s != 0);};

	symbols *next() { return n;};
	symbols *prev() { return p;};
	inline ulong raw_value() {  return s; };
	inline ulong value() { return s / 2;};

	// assuming this is a non-terminal, returns the corresponding rule
	rules *rule() { return (rules *) s;};

	void substitute(rules *r);
	static void match(symbols *s, symbols *m);

	// checks a new digram. If it appears elsewhere, 
	// deals with it by calling match(), otherwise inserts 
	// it into the hash table
	int check() {
		if (is_guard() || n->is_guard()) return 0;
		symbols *x = find_digram();
		if(x == 0) {
			set_digram();
			return 0;
		}
		if(x->next() != this) {
			match(this,x);
		}
		return 1;
	}

	void expand();

	void point_to_self() { join(this, this); };

	// this function is called to make this symbol a predictor.
	// for rules, it means that the first item of the rule becomes
	// a predictor recursively.
	void become_predictor_down_left();

	void become_predictor_down_right();

	// this function is called to notify the users of this symbol
	// that it has been transformed into a predictor.
	// It will be called when infering a context when the prediction
	// context becomes empty, it potentially set many rules as predictors.
	void become_predictor_up(symbols* child);

	private: 
		// the next_* variables are set by compute_next_predictors
		// and correspond to the values of the member variables
		// after a call to update_predictors.	
		std::set<symbols*> next_new_predictor; // next set of new predictors
		std::set<symbols*> next_stay_predictor; // predictors that stay predictors
		bool next_is_predictor; // next value for is_predictore
		bool next_updated; // wether we already called compute_next_predictors
		int  next_return; // return value of the last call to compute_next_predictors

	public:
	// this function recursively goes through the set of predictors
	// of a rule and increment them to point to the next predicted
	// the results are stored in the next_* member variables.
	// update_predictors should be called to make these next values
	// the current ones.
	// It takes a "matching" symbol and will only update the predictors
	// that correctly predicted this symbol. Predictors that did not
	// predict it will be deleted.
	// It returns 0 if no predictor in the recursive call did match with
	// the symbol, 1 if some symbols match and the predictors have been
	// incremented, but this symbol doesn't have to be incremented, 
	// 2 if some symbols match and the parent symbol has to be updated 
	// to its next one, 3 if some symbols match and the parent symbol has to
	// to be updated, but it should also stay a predictor itself.
	int compute_next_predictors(symbols* matching);

	// makes the next_* value the current ones.
	void update_predictors();

	// search for predictors in the rull in which this symbol appears,
	// given the last symbol read (this symol bught be a non-terminal).
	void find_potential_predictors(symbols* matching);
};

}
}

std::ostream& operator<<(std::ostream &out, omniscio::sequitur::symbols& s);

#endif
