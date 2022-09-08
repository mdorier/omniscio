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

#ifndef OMNISCIO_MATRIX_H
#define OMNISCIO_MATRIX_H

#include <fstream>
#include <map>
#include "omniscio.h"

namespace omniscio {

/**
 * The matrix class is a wrapper for a map that associates
 * a pair of unsigned int indices with something of type T.
 */
template<typename T>
class matrix {
	
	private:
	std::map<std::pair<unsigned int,unsigned int>,T> internal;
	T trash;
	std::string default_filename;

	public:
	
	matrix() {}

	virtual ~matrix() {}

	const T& operator()(unsigned int i, unsigned int j) const {
		std::pair<unsigned int,unsigned int> p(i,j);
		typename std::map<
			std::pair<unsigned int,unsigned int>,T>::iterator it 
				= internal.find(p);
		if(it != internal.end())
			return internal[p];
		else
			return trash;
	}

	T& operator()(unsigned int i, unsigned int j) {
		std::pair<unsigned int,unsigned int> p(i,j);
		return internal[p];
	}

	bool defined(unsigned i, unsigned j) const {
		return internal.count(
			std::pair<unsigned int, unsigned int>(i,j)) > 0;
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

	void open(const std::string& filename) {
		default_filename = filename;
	}

	template<typename F>
	friend std::ostream& operator<<(std::ostream& stream,
					const matrix<F>& m);
};

template<typename T>
std::ostream& operator<<(std::ostream& stream, const matrix<T>& m) {
	typename std::map<std::pair<unsigned int,unsigned int>,T>::const_iterator it =
			m.internal.begin();
	for(;it != m.internal.end(); it++) {
		std::pair<unsigned int,unsigned int> p = it->first;
		unsigned int i, j;
		i = p.first;
		j = p.second;
		stream << i << ' ' << j << ' ' << it->second << '\n';
	}
	return stream;
}

}

#endif
