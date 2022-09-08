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

#ifndef OMNISCIO_STATS_H
#define OMNISCIO_STATS_H

#include <iomanip>
#include <iostream>

namespace omniscio {

template<typename T>
class stats {

	private:

	int n;
	double mean;
	double var;
	T min;
	T max;

	public:

	stats() : n(0) {}

	virtual ~stats() {}

	virtual stats& operator+=(const T& x) {
		if(n == 0) {
			n = 1;
			mean = x;
			var = 0;
			min = max = x;
		} else {
			var += mean*mean;
			mean = (n*mean + x)/(n+1);
			var  = (n*var + x*x)/(n+1) - mean*mean;
			n += 1;
			min = ((min < x) ? min : x);
			max = ((max > x) ? max : x);
		}
		return *this;
	}

	double get_mean() const {
		return mean;
	}

	double get_variance() const {
		return var;
	}

	T get_min() const {
		return min;
	}

	T get_max() const {
		return max;
	}

	int get_number() const {
		return n;
	}

	template<typename D>
	friend std::ostream& operator<<(std::ostream& stream,
				const stats<D>& s);
};

template<typename D>
std::ostream& operator<<(std::ostream& stream, const stats<D>& s) {
	stream  << std::fixed << std::setprecision(9) 
		<<'{' << s.n << ",[" << s.min << ':' << s.max
		<< "]," << s.mean << ',' << s.var << '}';
	return stream;
}

}

#endif
