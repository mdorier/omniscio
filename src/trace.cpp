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

#include <cstdlib>
#include <cstring>
#include <string>
#include "trace.hpp"
#ifdef HAVE_CXXABI
#include <cxxabi.h>
#endif
#undef HAVE_CXXABI
namespace omniscio {

std::ostream& operator<<(std::ostream& os, const trace& t)
{
	size_t s = t.size();
	char** strings = backtrace_symbols(&(t[0]),s);
	for(size_t i = 0; i < s; i++) {

		char* a = std::strstr(strings[i],"(");
		char* a2 = std::strstr(strings[i],")");
		char* b = NULL;
		char* c = NULL;

		std::string filename;
		std::string function;

		if(a == NULL) {
			os << strings[i];
			continue;
		}

		if(a+1 != a2 && a2 != NULL) {
			filename = std::string(strings[i],
					(size_t)(a - strings[i]));
			a += 1;
			b = std::strstr(a,"+");
			if(b != NULL) {
				function = std::string(a,(size_t)(b - a));
			}
			b += 1;
		}
		c = std::strstr(strings[i],")");
		std::string offset;
		if(b != NULL) {
			offset = std::string(b,(size_t)(c-b));
		}
		std::string address(c+1);

#ifdef HAVE_CXXABI
		int status;
		char* res = abi::__cxa_demangle(function.c_str(),0,0,&status);
		if(status == 0) {
			os << filename << " (" << res << "+" 
			<< offset << ") " << address;
			free(res);
		} else {
			os << filename << " (" << function << "+"
			<< offset << ") " << address;
		}
#else
		os << filename << " (" << function << "+" 
			<< offset << ") " << address;
#endif
		if(i != s-1) os << ";";
	}
	free(strings);
	return os;
}

}
