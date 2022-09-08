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

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include "oracle.hpp"

namespace omniscio {
namespace sequitur {

void oracle::find_new_predictors(symbols* s) 
{
	std::set<rules*>::iterator it = rules_set.begin();
	for(;it != rules_set.end(); it++) {
		rules* r = *it;
		r->first()->find_potential_predictors(s);
	}
}

void oracle::input(int x) {
	version++;

	symbols* s = new symbols(x,start);
	start->last()->insert_after(s);

	root->compute_next_predictors(s);
	root->update_predictors();
	start->last()->prev()->check();
	
	if(! root->is_pred()) {
		find_new_predictors(start->last());
		root->compute_next_predictors(s);
		root->update_predictors();
	}

}

symbols* oracle::find_digram(symbols* s) {	
	ulong one = s->raw_value();
	ulong two = s->next()->raw_value();
	std::pair<ulong,ulong> p(one,two);

	if(table.count(p)) return table[p];
	else return 0;
}

void oracle::delete_digram(symbols* s) {
	
	ulong one = s->raw_value();
	ulong two = s->next()->raw_value();
	std::pair<ulong,ulong> p(one,two);
	if(table.count(p)) {
		if(table[p] == s)
			table.erase(p);
	}
}

void oracle::set_digram(symbols* s) {
	ulong one = s->raw_value();
	ulong two = s->next()->raw_value();
	std::pair<ulong,ulong> p(one,two);
	
	table[p] = s;
}

size_t oracle::size() const {
	size_t s = 0;
	std::set<rules*>::const_iterator rule = rules_set.begin();
	for(; rule != rules_set.end(); rule++) {
		s += (*rule)->length();
	}
	return s;
}

std::list<std::stack<symbols*> > 
	oracle::build_predictor_stack_from(symbols* s) const
{
	std::list<std::stack<symbols*> > result;

	if(not s->nt()) {
		std::stack<symbols*> st;
		st.push(s);
		result.push_back(st);
		return result;
	}

	s = s->rule()->first();
	while(not s->is_guard()) {
		if(s->is_pred()) {
			std::list<std::stack<symbols*> > stacks =
				build_predictor_stack_from(s);
			std::list<std::stack<symbols*> >::iterator it =
				stacks.begin();
			for(;it != stacks.end(); it++) {
				std::stack<symbols*> st = *it;
				st.push(s);
				result.push_back(st);
			}
		}
		s = s->next();
	}
	return result;
}

std::list<oracle::iterator> oracle::predict_all() const {
	std::list<std::stack<symbols*> > stacks = 
		build_predictor_stack_from(root);
	std::list<oracle::iterator> result;
	std::list<std::stack<symbols*> >::iterator it =
		stacks.begin();
	for(; it != stacks.end(); it++) {
		oracle::iterator i(this);
		// stacks returned by build_predictor_stack_from
		// are reversed (terminal elements are at the bottom)
		while(not (*it).empty()) {
			i.stack.push((*it).top());
			(*it).pop();
		}
		result.push_back(i);
	}
	return result;
}

void oracle::print_rule(std::ostream& stream, rules* r) {
	for (symbols *s = r->first(); !s->is_guard(); s = s->next()) {

		if(((stream == std::cout && isatty(fileno(stdout))) 
		|| (stream == std::cerr && isatty(fileno(stderr))))
		&& s->is_pred()) stream << "\x1b[31m";

		if (s->nt()) {
			int i;

			if (R[s->rule()->index()] == s->rule()) {
				i = s->rule()->index();
			} else {
				i = Ri;
				s->rule()->index(Ri);
				R[Ri ++] = s->rule();
			}
			stream << "[" << i 
			<< "] ";
		} else {
			stream << s->value() << ' ';
		}
		if(((stream == std::cout && isatty(fileno(stdout)))
		|| (stream == std::cerr && isatty(fileno(stderr))))
		&& s->is_pred()) stream << "\x1b[0m" ;
	}
}

std::ostream& operator<< (std::ostream& stream, oracle& o)
{
	o.R = (rules **) malloc(sizeof(rules*) * o.get_num_rules());
	memset(o.R, 0, sizeof(rules *) * o.get_num_rules());
	o.R[0] = o.start;
	o.Ri = 1;
	for (int i = 0; i < o.Ri; i ++) {
		stream << "[" << i 
		<< "] -> ";
		o.print_rule(stream, o.R[i]);
		if(i != o.Ri-1) stream << std::endl;
	}
	free(o.R);
	return stream;
}

oracle::iterator::iterator(const oracle* p, symbols* start) 
: parent(p), version(p->version) {
	if(start != 0) {
		stack.push(start);
		symbols* s = start;
		while(s->nt()) {
			stack.push(s->rule()->first());
			s = s->rule()->first();
		}
	}
}

oracle::iterator::iterator(const oracle::iterator& it) {
	stack = it.stack;
	version = it.version;
	parent = it.parent;
}

oracle::iterator::~iterator() { }

oracle::iterator& oracle::iterator::operator=(const oracle::iterator& it) {
	stack = it.stack;
	version = it.version;
	parent = it.parent;
	return *this;
}

oracle::iterator& oracle::iterator::operator++() {
	if(version != parent->version) throw invalid_iterator();
	if(stack.empty()) return *this;

	symbols* current = stack.top();
	stack.pop();

	// continue reading a rule
	if(not current->next()->is_guard()) {
		symbols* s = current->next();
		stack.push(s);
		// if next is a rule, go down the rule
		while(s->nt()) {
			s = s->rule()->first();
			stack.push(s);
		}
	} else {
	// done reading a rule, s is in the upper rule
		++(*this);
	}

	return *this;
}

oracle::iterator oracle::iterator::operator++(int) {
	if(version != parent->version) throw invalid_iterator();
	iterator it(*this);
	++(*this);
	return it;
}

int oracle::iterator::operator*() const {
	if(version != parent->version) throw invalid_iterator();
	if(stack.empty()) return 0;
	symbols* s = stack.top();
	return s->value();
}

bool oracle::iterator::operator==(const oracle::iterator& it) {
	return (parent == it.parent) && (version == it.version) && (stack == it.stack);
}

bool oracle::iterator::operator!=(const oracle::iterator& it) {
	return not (stack == it.stack);
}

}
}
