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

#include <iostream>
#include "tree.hpp"

using namespace omniscio;

void print_tree(const tree<char>::iterator& t) {
	tree<char>::iterator c = t.begin();
	if(t.begin() == t.end()) {
		std::cout << *t << ' ';
	} else {
		std::cout << '(' << *t << ' ';
		while(c != t.end()) {
			print_tree(c);
			c++;
		}
		std::cout << ')';
	}
}

int main(int argc, char** argv) {

	tree<char> t;

	std::cout << "tree is empty = " << t.is_empty() << std::endl;

	t.add_root('A');
	t.add_root('B');

	tree<char>::iterator it = t.begin();
	while(it != t.end()) {
		std::cout << "root = " << *it << std::endl;
		it++;
	}
	tree<char>::iterator a = t.begin();
	a.append_child('C').append_child('D');
	a.append_child('E');

	print_tree(a);

	return 0;
}
