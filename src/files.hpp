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

#ifndef OMNISCIO_FILE_H
#define OMNISCIO_FILE_H

#include <string>
#include <mpi.h>

namespace omniscio {

/**
 * This file_h type represents a generic file handle
 * from the point of view of Omnisc'IO.
 */
typedef unsigned long file_h;

/**
 * Create a file handle from a file name and a posix file descriptor.
 * \param[in] filename : name of the opened file.
 * \param[in] fd : posix file descriptor.
 * \return Omnisc'IO file handle.
 */
file_h open_posix_file(const char* filename, int fd);

/**
 * Create a file handle from a file name and an MPI-I/O file handle.
 * \param[in] filename : name of the opened file.
 * \param[in] fh : MPI-I/O file handle.
 * \return Omnisc'IO file handle.
 */
file_h open_mpiio_file(const char* filename, MPI_File fh);

/**
 * Create a file handle from a file name and a LibC file pointer.
 * \param[in] filename : name of the opened file.
 * \param[in] fh : file pointer.
 * \return Omnisc'IO file handle.
 */
file_h open_libc_file(const char* filename, FILE* fh);

/**
 * Notifies Omnisc'IO that a POSIX file descriptor has been closed.
 * \param[in] fd : posix file descriptor.
 */
void close_posix_file(int fd);

/**
 * Notifies Omnisc'IO that an MPI-I/O file handle has been closed.
 * \param[in] fh : MPI-I/O file handle.
 */
void close_mpiio_file(MPI_File fh);

/**
 * Notifies Omnisc'IO that a LibC file pointer has been closed.
 * \param[in] fh : LibC file handle.
 */
void close_libc_file(FILE* fh);

/**
 * Gets the name of a file from its file descriptor.
 * \param[in] fd : posix file descriptor.
 * \return the name of the file.
 */
const std::string& filename_from_posix_file(int fd);

/**
 * Gets the name of a file from its file descriptor.
 * \param[in] fh : MPI-I/O file handle.
 * \return the name of the file.
 */
const std::string& filename_from_mpiio_file(MPI_File fh);

/**
 * Gets the name of a file from its file descriptor.
 * \param[in] fh : LibC file handle.
 * \return the name of the file.
 */
const std::string& filename_from_libc_file(FILE* fh);

}

#endif
