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

#ifndef OMNISCIO_MODEL_H
#define OMNISCIO_MODEL_H

#include <fstream>
#include "sequitur/oracle.hpp"


namespace omniscio {

template<typename T>
class model {

	private:

	std::ofstream file;
	bool opened;

	T last_seen;
	int num_seen;

	sequitur::oracle oracle_;

	public:

	model() : opened(false) { }

	model(const std::string& filename) : opened(false) {
		open(filename);
	}

	bool open(const std::string& filename) {
		if(not opened) {
			OMNISCIO_UNTRACED_START;
			file.open(filename.c_str());
			OMNISCIO_UNTRACED_END;
			opened = true;
			return true;
		} else {
			return false;
		}
	}

	bool close() {
		if(not opened) return false;
		OMNISCIO_UNTRACED_START;
		file << oracle_;
		file.close();
		OMNISCIO_UNTRACED_END;
		return true;
	}
	
	model& operator<<(const T& observation) {
		oracle_.input(observation);
		return *this;
	}

	void predict(std::vector<std::pair<T,double> >& prediction) {
		std::set<int> pred = oracle_.predict_next();
		std::set<int>::iterator it = pred.begin();
		int n = pred.size();
		prediction.resize(n);
		for(int i = 0; it != pred.end(); it++, i++) {
			prediction[i] = std::pair<T,double>(*it,1.0/n);
		}
	}

	~model() {
		close();
	}

};

}

#endif
