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
#include <fstream>
#include <cstring>
#include <cstdlib>
#include "sequitur/oracle.hpp"

using namespace omniscio::sequitur;
using namespace std;

int main(int argc, char** argv)
{
	if(argc < 2) {
		std::cerr << "Usage: "<< argv[0] <<" file" << std::endl;
		exit(0);
	}

	std::ifstream ifs(argv[1], std::ifstream::in);

	oracle o;
	
	std::cout << "Press <ENTER> to start reading file...";

	char sym;

	while (1) {
//		if(cin.get() == 'q') break;
		ifs >> sym;
		if (ifs.eof()) break;

		cout << "Inserting: " << (int)sym << endl;
		o.input((int)sym);
		cout << o << endl;
		cout << "Size = " << o.size() << endl;

		cout << "Immediate Prediction: ";
		std::set<int> pred = o.predict_next();
		std::set<int>::iterator it1 = pred.begin();
		for(;it1!=pred.end();it1++) cout << (int)(*it1) << " ";

		cout << endl << "Long-Term Prediction: ";
		std::list<oracle::iterator> all_p = o.predict_all();
		std::list<oracle::iterator>::iterator it2 = all_p.begin();
		for(;it2!=all_p.end();it2++) {
			oracle::iterator seq = *it2;
			cout << endl;
			for(int i=0; (i<5) && (seq != o.end()); i++, seq++) {
				unsigned int c = *seq;
				cout << c << " -> ";
			}
			cout << " ...";
		}

		cout << endl << "-------------------------------------" << endl;
	}

	ifs.close();
}

