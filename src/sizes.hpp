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

#ifndef OMNISCIO_SIZES_H
#define OMNISCIO_SIZES_H

#include "sequitur/oracle.hpp"

namespace omniscio {

class size_tracker {

	private:

	class size_type {

		friend class size_tracker;

		protected:
		typedef enum {
			SIMPLE,
			GRAMMAR,
			AVERAGE
		} type;

		private:
		type type_;

		protected:
		size_type(type t) : type_(t) {}

		public:
		virtual ~size_type() {}

		virtual size_t predict() = 0;

		type get_type() const {
			return type_;
		}
	};

	class simple_size;
	class gram_size;
	class average_size;

	class simple_size : public size_type {

		friend class size_tracker;
		protected:
		size_t size;
		long occurences;

		public:
		simple_size(size_t s) 
		: size_type(SIMPLE), size(s), occurences(1) {}

		virtual ~simple_size() {}

		virtual size_t predict() {
			return size;
		}

		long get_occurences() const {
			return occurences;
		}
	};

	class gram_size : public size_type {

		friend class average_size;
		friend class size_tracker;

		protected:
		std::map<size_t,int> size_map;
		std::map<size_t,long> occ_map;
		std::map<int,size_t> symbols_size;

		double average_size;
		int last;
		long occurences;
		sequitur::oracle o;
		
		public:
		gram_size(simple_size* ss)
		: size_type(GRAMMAR), last(0) {
			size_t size = ss->predict();
			size_map[size] = last;
			occ_map[size] = ss->get_occurences();
			symbols_size[last] = size;
			for(int i=0; i < ss->get_occurences(); i++) {
				o.input(last);
			}
			last++;
			average_size = size;
			occurences = ss->get_occurences();
                }

		virtual ~gram_size() {}

		long get_occurences() const {
			return occurences;
		}

		virtual size_t predict() {
			std::set<int> pred = o.predict_next();
			if(pred.size() == 0) {
				return average_size;
			}
			std::set<int>::iterator it = pred.begin();
			if(pred.size() == 1) {
				int sym = *pred.begin();
				return symbols_size[sym];
			}
			double avg = 0.0;
			int n = 0;
			for(; it != pred.end(); it++) {
				int sym = *it;
				size_t p = symbols_size[sym];
				n += occ_map[p];
				avg += (double)p*occ_map[p];
			}
			avg /= n;
			return (size_t)avg;
                }

		virtual void input(size_t s) {
			double ds = (double)s;
			double f1 = 1.0/(double)(occurences+1);
			double f2 = (double)occurences*f1;
			average_size = f1*ds + f2*average_size;
			occurences += 1;

			int sym;
			if(size_map.count(s) > 0) {
				// size is known
				sym = size_map[s];
				occ_map[s] += 1;
			} else {
				// size is unknown
				sym = last;
				last++;
				occ_map[s] = 1;
				size_map[s] = sym;
				symbols_size[sym] = s;
			}
			// update grammar
			o.input(sym);
		}
	};

	class average_size : public size_type {

		friend class size_tracker;
		protected:
		double average;
		long occurences;

		public:
		average_size(simple_size* s) 
		: size_type(AVERAGE) {
			occurences = s->get_occurences();
			average = s->predict();
		}

		average_size(gram_size* s) 
		: size_type(AVERAGE) {
			std::map<size_t,long>::iterator it = s->occ_map.begin();
			occurences = s->occurences;
			average = 0.0;
			for(;it != s->occ_map.end(); it++) {
				size_t size = it->first;
				long occ = it->second;
				average += (double)(size*occ)/(double)(occurences);
			}
		}

		virtual size_t predict() {
			return (size_t)average;
		}

		virtual void input(size_t s) {
			double f1 = 1.0/(double)(occurences+1);
			double f2 = (double)occurences*f1;
			occurences += 1;
			average = average*f2 + (double)s*f1;
		}
        };

	size_type* st;
	int* instances;
	
	public:

	size_tracker() {
		st = 0;
		instances = 0;
	}

	size_tracker(size_t size) {
		st = new simple_size(size);
		instances = new int(1);
	}

	size_tracker(const size_tracker& other) {
		st = other.st;
		instances = other.instances;
		if(instances) *instances += 1;
	}

	size_tracker& operator=(const size_tracker& other) {
		if(instances) {
			(*instances)--;
			if(*instances == 0) {
				delete st;
				delete instances;
			}
		}
		st = other.st;
		instances = other.instances;
		*instances += 1;
		return *this;
	}

	~size_tracker() {
		if(instances) {
			(*instances)--;
			if(*instances == 0) {
				delete st;
				delete instances;
			}
		}
	}

	void input(size_t s) {
		if(st == 0) {
			*this = size_tracker(s);
		} else if(st->get_type() == size_type::SIMPLE) {
			simple_size* ss = dynamic_cast<simple_size*>(st);
			if(ss->size == s) {
				ss->occurences += 1;
			} else {
				// create a grammar size
				simple_size* ss = dynamic_cast<simple_size*>(st);
				gram_size* gs = new gram_size(ss);
				*instances -= 1;
				if(*instances == 0) {
					delete instances;
					delete st;
				}
				st = gs;
				instances = new int(1);
				gs->input(s);
			}
		} else if(st->get_type() == size_type::GRAMMAR) {
			gram_size* gs = dynamic_cast<gram_size*>(st);
			gs->input(s);
			// create an average size
			if(gs->occ_map.size() > 16) {
				average_size* as = new average_size(gs);
				*instances -= 1;
				if(*instances == 0) {
					delete instances;
					delete st;
				}
				st = as;
				instances = new int(1);
			}
		} else if(st->get_type() == size_type::AVERAGE) {
			average_size* as = dynamic_cast<average_size*>(st);
			as->input(s);
		}
	}

	size_t predict() {
		if(st) return st->predict();
		else return 0;
	}
};


}

#endif
