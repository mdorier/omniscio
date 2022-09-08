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

#ifndef OMNISCIO_DMAP_H
#define OMNISCIO_DMAP_H

#include <cmath>
#include <map>

namespace omniscio {

template<typename T>
class dmap : public std::map<double,T> {

	public:

	dmap() {}

	virtual ~dmap() {}

	double closest_key(double k) {
		if(std::map<double,T>::count(k) 
		|| std::map<double,T>::size() == 0) return k;
		typename std::map<double,T>::iterator it = 
			std::map<double,T>::lower_bound(k);
		if(it == std::map<double,T>::end()) {
			// the last element is the closest one
			return std::map<double,T>::rbegin()->first;
		} else if(it == std::map<double,T>::begin()) {
			// all values are greater then k
			return std::map<double,T>::begin()->first;
		} else {
			double k1 = it->first;
			it--;
			double k2 = it->first;
			if(std::abs(k1 - k) < std::abs(k2 - k)) {
				return k1;
			} else {
				return k2;
			}
		}
	}

	bool change_key(double old_key, double new_key) {
		if(std::map<double,T>::count(old_key)) {
			T v = std::map<double,T>::operator[](old_key);
			std::map<double,T>::erase(old_key);
			std::map<double,T>::operator[](new_key) = v;
			return true;
		}
		return false;
	}
};

}

#endif
