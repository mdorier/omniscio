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

#ifndef OMNISCIO_VECTOR_H
#define OMNISCIO_VECTOR_H

#include <fstream>
#include <vector>
#include "omniscio.h"

namespace omniscio {

template<typename T>
class vector {
	
	private:

	std::map<unsigned int,T> internal;
	T trash;
	std::string default_filename;

	public:
	
	vector() {}

	virtual ~vector() {}

	const T& operator()(unsigned int i) const {
		if(internal.size() >= i) {
			return trash;
		}
		return internal[i];
	}

	T& operator()(unsigned int i) {
		return internal[i];
	}

	void close() const {
		std::ofstream ofs;
		OMNISCIO_UNTRACED_START;
		ofs.open(default_filename.c_str(), 
			std::ofstream::out | std::ofstream::trunc);
		if(ofs.is_open()) {
			ofs << *this;
			ofs.close();
		}
		OMNISCIO_UNTRACED_END;
	}

	bool defined(unsigned int i) const {
		return internal.count(i) > 0;
	}

	void open(const std::string& filename) {
		default_filename = filename;
	}

	template<typename F>
	friend std::ostream& operator<<(std::ostream& stream,
					const vector<F>& v);
};

template<typename T>
std::ostream& operator<<(std::ostream& stream, const vector<T>& v) {
	typename std::map<unsigned int, T>::const_iterator it =
			v.internal.begin();
	for(; it != v.internal.end(); it++) {
		stream << it->first << ' '  << it->second << '\n';
	}
	return stream;
}

}

#endif
