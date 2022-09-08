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

#ifndef OMNISCIO_OFFSETS_H
#define OMNISCIO_OFFSETS_H

#include "sequitur/oracle.hpp"

namespace omniscio {

class offset_op {
	public:

	typedef enum {  
		FOLLOWING, 
		ABSOLUTE,
		RELATIVE
	} type;    
	
	offset_op() {
		type_ = FOLLOWING;   
	}
	
	offset_op(off_t s, type t = ABSOLUTE) {   
		type_ = t;    
		offset_ = s;
	}

	type get_type() const {   
		return type_;  
	}

	bool operator==(const offset_op& other) const {
		return (type_ == other.type_) && (offset_ == other.offset_);
	}

	bool operator!=(const offset_op& other) const {
		return not (*this == other);
	}

	bool operator<(const offset_op& other) const {
		if(type_ != other.type_) return type_ < other.type_;
		else return offset_ < other.offset_;
	}

	off_t get_offset_after(off_t start, size_t size) const {
		if(type_ == FOLLOWING) return start+size;
		else if(type_ == RELATIVE) return start+size+offset_;
		else return offset_;
	}

	private:
	type type_;
	long offset_;

};


class offset_tracker {

	private:

	class offset_type {

		friend class size_tracker;

		protected:
		typedef enum {
			SIMPLE,
			GRAMMAR,
			FOLLOW
		} type;

		private:
		type type_;

		protected:
		offset_type(type t) : type_(t) {}

		public:
		virtual ~offset_type() {}

		virtual offset_op predict() = 0;

		type get_type() const {
			return type_;
		}
	};

	class simple_offset;
	class gram_offset;
	class follow_offset;

	class simple_offset : public offset_type {

		friend class offset_tracker;
		protected:
		offset_op offset;
		long occurences;

		public:
		simple_offset(offset_op op) 
		: offset_type(SIMPLE), offset(op), occurences(1) {}

		virtual ~simple_offset() {}

		virtual offset_op predict() {
			return offset;
		}

		long get_occurences() const {
			return occurences;
		}
	};

	class gram_offset : public offset_type {

		friend class last_offset;
		friend class offset_tracker;

		protected:
		std::map<offset_op,int> offset_map;
		std::map<offset_op,long> occ_map;
		std::map<int,offset_op> symbols_offset;

		offset_op last_off;
		int last;
		long occurences;
		sequitur::oracle o;
		
		public:
		gram_offset(simple_offset* ss)
		: offset_type(GRAMMAR), last(0) {
			offset_op offset = ss->predict();
			offset_map[offset] = last;
			occ_map[offset] = ss->get_occurences();
			symbols_offset[last] = offset;
			for(int i=0; i < ss->get_occurences(); i++) {
				o.input(last);
			}
			last++;
			last_off = offset;
			occurences = ss->get_occurences();
                }

		virtual ~gram_offset() {}

		long get_occurences() const {
			return occurences;
		}

		virtual offset_op predict() {
			std::set<int> pred = o.predict_next();
			if(pred.size() == 1) {
				int sym = *pred.begin();
				return symbols_offset[sym];
			} else {
				return last_off;
			}
                }

		virtual void input(offset_op op) {
			occurences += 1;
			last_off = op;
			int sym;
			if(offset_map.count(op) > 0) {
				// offset is known
				sym = offset_map[op];
				occ_map[op] += 1;
			} else {
				// offset is unknown
				sym = last;
				last++;
				occ_map[op] = 1;
				offset_map[op] = sym;
				symbols_offset[sym] = op;
			}
			// update grammar
			o.input(sym);
		}
	};

	class follow_offset : public offset_type {

		friend class offset_tracker;

		public:
		follow_offset() 
		: offset_type(FOLLOW) {}

		virtual offset_op predict() {
			return offset_op();
		}

		virtual void input(offset_op) {
		}
        };

	offset_type* st;
	int* instances;
	
	public:

	offset_tracker() {
		st = 0;
		instances = 0;
	}

	offset_tracker(offset_op op) {
		st = new simple_offset(op);
		instances = new int(1);
	}

	offset_tracker(const offset_tracker& other) {
		st = other.st;
		instances = other.instances;
		if(instances) *instances += 1;
	}

	offset_tracker& operator=(const offset_tracker& other) {
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

	~offset_tracker() {
		if(instances) {
			(*instances)--;
			if(*instances == 0) {
				delete st;
				delete instances;
			}
		}
	}

	void input(offset_op op) {
		if(st == 0) {
			*this = offset_tracker(op);
		} else if(st->get_type() == offset_type::SIMPLE) {
			simple_offset* ss = dynamic_cast<simple_offset*>(st);
			if(ss->offset == op) {
				ss->occurences += 1;
			} else {
				simple_offset* ss = dynamic_cast<simple_offset*>(st);
				gram_offset* gs = new gram_offset(ss);
				*instances -= 1;
				if(*instances == 0) {
					delete instances;
					delete st;
				}
				st = gs;
				instances = new int(1);
				gs->input(op);
			}
		} else if(st->get_type() == offset_type::GRAMMAR) {
			gram_offset* gs = dynamic_cast<gram_offset*>(st);
			gs->input(op);
			if(gs->occ_map.size() > 24) {
				follow_offset* as = new follow_offset();
				*instances -= 1;
				if(*instances == 0) {
					delete instances;
					delete st;
				}
				st = as;
				instances = new int(1);
			}
		} else if(st->get_type() == offset_type::FOLLOW) {
			follow_offset* fo = dynamic_cast<follow_offset*>(st);
			fo->input(op);
		}
	}

	offset_op predict() {
		if(st) return st->predict();
		else return offset_op();
	}
};


}

#endif
