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

#include <iostream>
#include "symbols.hpp"
#include "oracle.hpp"

std::ostream& operator<<(std::ostream &out, omniscio::sequitur::symbols& s) {
        if(s.nt()) {
                out << (int)s.rule()->index();
        } else {
                out << (char)s.value();
        }
        return out;
}

namespace omniscio {
namespace sequitur {

symbols* symbols::find_digram() {
	return owner->get_oracle()->find_digram(this);
}

// removes the digram from the hash table
void symbols::delete_digram() {
	if (is_guard() || n->is_guard()) return;
	if (owner == (rules*)0) {
		return;
	}
	owner->get_oracle()->delete_digram(this);
}

void symbols::set_digram() {
	if (is_guard() || n->is_guard()) return;
	if (owner == (rules*)0) {
		return;
	}
	owner->get_oracle()->set_digram(this);
}

// This symbol is the last reference to its rule. It is deleted, and the
// contents of the rule substituted in its place.
// example: 	A -> ...B...
//		B -> a...b
// becomes:	A -> ...a...b...
// In terms of predictors, if B contains a predictor, since B only appears
// in rule A, then A is a predictor in some rule S.
void symbols::expand() {
	symbols *left = prev();
	symbols *right = next();
	symbols *f = rule()->first();
	symbols *l = rule()->last();

	// if this symbol is a predictor, copy its nested predictor symbols
	// into the users that have this symbol as a predictor (usr = only A)
	if(is_pred()) {
		std::set<symbols*>::iterator user = owner->get_users().begin();
		for(; user != owner->get_users().end(); user++) {
			if((*user)->predictors.count(this) > 0) {
				(*user)->predictors.erase(this);
				(*user)->predictors.insert(predictors.begin(), 
							   predictors.end());
			}
		}
	}

	// before deleting the rule, change the owner of all sub-symbols
	symbols *ns = rule()->first();
	while(ns != rule()->last()) {
		ns->owner = owner;
		ns = ns->next();
	}
	ns->owner = owner;

	delete_digram();
	delete rule();
	s = 0; // if we don't do this, deleting the symbol will deuse the rule!
	delete this;

	join(left, f);
	join(l, right);

	l->set_digram();
}

// Replace a digram with a non-terminal
// Example: A -> ...X1Y1...
//	    B -> X2Y2
// Becomes: A -> ...B...
// Note that the function is called on the X1 in rule A.
void symbols::substitute(rules *r)
{
	symbols *q = p; // q = previous

	// create the new symbol ("B" in rule A in the example)
	symbols* B  = new symbols(r,owner);
	symbols* X1 = this;
	symbols* X2 = r->first();
	symbols* Y1 = X1->next();
	symbols* Y2 = X2->next();

	// is X1 is a predictor, then it should be erased from
	// all parents that used it as a predictor, and replaced
	// by the new symbol "B" in these parents.
	if(X1->is_pred()) {
		B->is_predictor = true;
		for(std::set<symbols*>::iterator user 
			= owner->get_users().begin();
			user != owner->get_users().end(); user++) {
			if((*user)->predictors.count(X1) != 0) {
				(*user)->predictors.erase(X1);
				(*user)->predictors.insert(B);
			}
		}
		// additionaly, all predictors in X1 should be copied
		// into the set of predictors of X2.
		X2->predictors.insert(X1->predictors.begin(),
				      X1->predictors.end());
		X2->is_predictor = true;
		if(not X2->nt()) owner->get_oracle()->add_prediction(X2);
		B->predictors.insert(X2);
	}

	// If the next 
	if(Y1->is_pred()) {
		B->is_predictor = true;
		for(std::set<symbols*>::iterator user 
			= owner->get_users().begin();
			user != owner->get_users().end(); user++) {
			if((*user)->predictors.count(Y1) != 0) {
				(*user)->predictors.erase(Y1);
				(*user)->predictors.insert(B);
			}
		}
		// additionaly, all predictors in Y1 should be copied
		// into the set of predictors of Y2.
		Y2->predictors.insert(Y1->predictors.begin(),
				      Y1->predictors.end());
		Y2->is_predictor = true;
		if(not Y2->nt()) owner->get_oracle()->add_prediction(Y2);
		B->predictors.insert(Y2);
	}

	delete X1;
	delete Y1;
	
	q->insert_after(B);

	if (!q->check()) q->n->check();
}

// Deal with a matching digram
void symbols::match(symbols *ss, symbols *m) 
{
	rules *r;
	// reuse an existing rule

	if (m->prev()->is_guard() 
	&& m->next()->next()->is_guard()) {
		// this is the case where the matching digram ss = ...ab
		// and the matching symbol is actualy withing a rule T -> ab
		// so we need to replace ss by T
		r = m->prev()->rule();
		ss->substitute(r); 
	}
	else {
		// This is for the case where ss = ...ab and we match with
		// another "ab" which is somewhere else but does not yet
		// correspond to a rule.
		// create a new rule for "ab"
		r = new rules(ss->owner->get_oracle());

		if (ss->nt())
			r->last()->insert_after(new symbols(ss->rule(),r));
		else
			r->last()->insert_after(new symbols(ss->value(),r));

		if (ss->next()->nt())
			r->last()->insert_after(
				new symbols(ss->next()->rule(),r));
		else
			r->last()->insert_after(
				new symbols(ss->next()->value(),r));

		m->substitute(r);
		ss->substitute(r);

		r->first()->set_digram();
	}

	// check for an underused rule

	if (r->first()->nt() && r->first()->rule()->freq() == 1) 
		r->first()->expand();
}

// When called on a rule, the first item of the rule
// becomes a predictor, and so on recursively
void symbols::become_predictor_down_left() {
	is_predictor = true;
	if(nt()) {
		predictors.insert(rule()->first());
		rule()->first()->become_predictor_down_left();
	} else {
		owner->get_oracle()->add_prediction(this);
	}
}

void symbols::become_predictor_down_right() {
	is_predictor = true;
	if(nt()) {
		predictors.insert(rule()->last());
		rule()->last()->become_predictor_down_right();
	} else {
		owner->get_oracle()->add_prediction(this);
	}
}

// Considering that "child" is a predictor,
// make thus symbol a predictor itself and make all users
// of this symbol a predictor.
void symbols::become_predictor_up(symbols* child) {
	if(predictors.count(child) != 0) {
		return;
	}
	predictors.insert(child);
	is_predictor = true;
	if(owner == (rules*)0) {
		return;
	}
	std::set<symbols*>::iterator user = owner->get_users().begin();
	for(;user != owner->get_users().end(); user++) {
		(*user)->become_predictor_up(this);
	}
}


int symbols::compute_next_predictors(symbols* matching) {
	if(next_updated) return next_return;
	if(not is_pred()) return 0;
	next_updated = true;
	next_return = 0;
	next_is_predictor = true;

	if(matching->raw_value() == this->raw_value()) {
		next_return = 2;
		next_is_predictor = false;
		owner->get_oracle()->remove_prediction(this);
		return next_return;
	}

	if(nt()) {

		std::set<symbols*>::iterator it 
			= predictors.begin();
		
		for(; it != predictors.end(); it++) {
			symbols* s = *it;
			int r = s->compute_next_predictors(matching);
			switch(r) {

			case 0: // child says I'm not a predictor!
				// ...do nothing
				break;
			case 1: // child says I'm a predictor, and I
				// keep being one.
				next_stay_predictor.insert(s);
				next_return |= 1;
				break;
			case 2: // child says I'm a predictor and completed
				// the prediction, use s->next() if exist, 
				// otherwise return 2 (or 3 yourself).
				if(s->next()->is_guard()) {
				// if there is no next one, ask the parent to
				// find a next one. And change the next_return
				// to either 3 or 2 depending on wether we 
				// should stay a predictor (1) or not (0).
					next_return |= 2;
				} else {
				// if the next one is not a guard, we can add it
				// as predictor, and we stay a predictor (1).
					next_new_predictor.insert(s->next());
					next_return |= 1;
				}
				break;
			case 3: // child says I'm a predictor, I stay one and
				// my next() should also be a predictor.
				next_return |= 1;
				next_stay_predictor.insert(s);
				if(! s->next()->is_guard()) {
					next_new_predictor.insert(s->next());
				} else {
					next_return |= 2;
				}
				break;
			}
		}
		if(next_stay_predictor.empty() && next_new_predictor.empty()) {
			next_is_predictor = false;
		}
		return next_return;
	} else {
		next_is_predictor = false;
		owner->get_oracle()->remove_prediction(this);
		if(matching->raw_value() == this->raw_value()) {
			return 2;
		} else {
			return 0;
		}
	}
}

void symbols::update_predictors() {
	if(not is_pred()) return;
	if(not next_updated) return;

	next_updated = false;
	std::set<symbols*>::iterator it = predictors.begin();

	for(; it != predictors.end(); it++) {
		(*it)->update_predictors();
	}

	it = next_new_predictor.begin();

	for(; it != next_new_predictor.end(); it++) {
		(*it)->become_predictor_down_left();
	}
	
	is_predictor = next_is_predictor;
	if(is_predictor) {
		predictors = next_new_predictor;
		predictors.insert(next_stay_predictor.begin(),next_stay_predictor.end());
	} else {
		predictors.clear();
		if(owner != (rules*)0)
			owner->get_oracle()->remove_prediction(this);
	}

	next_stay_predictor.clear();
	next_new_predictor.clear();
}

void symbols::find_potential_predictors(symbols* matching) {
	if(this == matching) return; // prevents from using the last symbol
	// of the rule, which will disappear anyway
	if(this->raw_value() == matching->raw_value()) {
		if(owner != (rules*)0) {
			become_predictor_down_right();
			std::set<symbols*>::iterator user
				= owner->get_users().begin();
			for(; user != owner->get_users().end(); user++) {
				(*user)->become_predictor_up(this);
			}
		}
	}
	if(not next()->is_guard()) {
		next()->find_potential_predictors(matching);
	}
}

symbols::~symbols() {
	if(p == NULL && n == NULL) return;
	join(p, n);
	if (!is_guard()) {
		delete_digram();
		if (nt()) rule()->deuse(this);
	}
	if(is_pred() && (not nt()) && (owner != (rules*)0)) {
		owner->get_oracle()->remove_prediction(this);
	}
}

}
}
