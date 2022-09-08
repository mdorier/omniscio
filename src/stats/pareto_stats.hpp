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

#ifndef OMNISCIO_PARETO_STATS_H
#define OMNISCIO_PARETO_STATS_H

#include <iomanip>
#include <iostream>
#include <cmath>
#include "stats/stats.hpp"

namespace omniscio {

template<typename T>
class pareto_stats : public stats<T> {

	private:

	double location;
	double form;

	public:

	pareto_stats() {}

	virtual ~pareto_stats() {}

	virtual pareto_stats& operator+=(const T& x) {
		stats<T>::operator+=(x);
		double m = stats<T>::get_mean();
		double v = stats<T>::get_variance();
		form = 1.0 + std::sqrt(1.0 + m*m/v);
		location = (form-1.0)*m/form;
		return *this;
	}

	double get_location() const {
		return location;
	}
	
	double get_form() const {
		return form;
	}

	double likelihood(const T& x) const {
		if(x < location) return 0.0;
		return form*std::pow(location,form)/std::pow(x,form+1.0);
	}
};

}

#endif
