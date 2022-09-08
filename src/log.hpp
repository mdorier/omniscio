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

#ifndef OMNISCIO_LOGSTREAM_H
#define OMNISCIO_LOGSTREAM_H

#include <fstream>
#include <string>
#include "omniscio.h"

namespace omniscio {

/**
 * The logstream object wraps an output stream of type T
 * and redirect its output to that stream, but prevents Omnisc'IO
 * from tracing I/O events goings through that stream.
 * This class is used in for all log files writen by Omnisc'IO.
 */
template<typename T>
class logstream {
	
	private:

	T stream;

	public:

	/**
	 * Constructor for logstream not yet associated with any file.
	 */
	logstream() {}

	/**
	 * Output operator.
	 * \param[in] object : object to output in the internal stream.
	 */
	template<typename X>
	logstream& operator<<(const X& object) {
		OMNISCIO_UNTRACED_START;
		stream << object;
		OMNISCIO_UNTRACED_END;
		return *this;
	}

	/**
	 * Opens the specified file as internal output stream.
	 */
	void open(const std::string& filename,  
			std::ios_base::openmode mode = std::ios_base::out) {
		OMNISCIO_UNTRACED_START;
		stream.open(filename.c_str(),mode);
		stream << std::fixed << std::setprecision(9);
		OMNISCIO_UNTRACED_END;
	}

	/**
	 * Close the internal stream.
	 */
	void close() {
		OMNISCIO_UNTRACED_START;
		stream.close();
		OMNISCIO_UNTRACED_END;
	}
	
	/**
	 * Flush the internal stream.
	 */
	void flush() {
		OMNISCIO_UNTRACED_START;
		stream.flush();
		OMNISCIO_UNTRACED_END;
	}

	/**
	 * Return the internal stream.
	 */
	T& get_stream() {
		return stream;
	}

	/**
	 * Return the internal stream.
	 */
	const T& get_stream() const {
		return stream;
	}
};

}

#endif
