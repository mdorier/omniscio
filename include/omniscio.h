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

#ifndef OMNISCIO_H
#define OMNISCIO_H

#include <sys/types.h>
#include <mpi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long 	omniscio_offset;
typedef unsigned long 	omniscio_size;
typedef double 		omniscio_date;

typedef enum {
	OMNISCIO_OPEN,
	OMNISCIO_CLOSE,
	OMNISCIO_READ,
	OMNISCIO_WRITE
} omniscio_op_type;

typedef enum {
	OMNISCIO_POSIX, // open, write...
	OMNISCIO_MPIIO, // MPI_File_open, MPI_File_write...
	OMNISCIO_LIBC   // fopen, fwrite...
} omniscio_api_type;

typedef union {
	MPI_File 	mpiio;
	unsigned long 	posix;
	FILE* 		libc;
} omniscio_file_ptr;

typedef struct {
	omniscio_api_type type;
	omniscio_file_ptr handle;
} omniscio_file;

typedef struct {
	omniscio_op_type 	type;
	omniscio_file 		fh;	// not yet supported
	omniscio_offset 	offset;
	omniscio_size 		size;
	omniscio_date 		date;
	const char* 		name;	// not yet supported
	double			proba;
} omniscio_req;

enum {
	OMNISCIO_OK 	= 0,
	OMNISCIO_ERROR 	= -1
};

/**
 * Initializes Omnisc'IO. Must be called after MPI_Init.
 * Returns OMNISCIO_OK in case of success, OMNISCIO_ERROR otherwise.
 */
int omniscio_init(int* argc, char*** argv);

/**
 * Starts tracing an open operation.
 */
int omniscio_open_start(const char* filename, omniscio_api_type t);

/**
 * Stops tracing an open operation.
 */
int omniscio_open_end(int success, omniscio_file fh);

/**
 * Starts tracing a close operation.
 */
int omniscio_close_start(omniscio_file fh);

/**
 * Stops tracing a close operation.
 */
int omniscio_close_end(int success);

/**
 * Starts tracing a write operation.
 */
int omniscio_write_start(omniscio_file fh, 
		omniscio_offset offset, omniscio_size size);

/**
 * Stops tracing a write operation.
 */
int omniscio_write_end(int success);

/**
 * Starts tracing a read operation.
 */
int omniscio_read_start(omniscio_file fh,
		omniscio_offset offset, omniscio_size size);

/**
 * Stops tracing a read operation.
 */
int omniscio_read_end(int success);

/**
 * Creates an omniscio_file handle from a POSIX file descriptor.
 */
int omniscio_file_from_posix(omniscio_file* result, int fd);

/**
 * Creates an omniscio_file handle from a LIBC file pointer.
 */
int omniscio_file_from_libc(omniscio_file* result, FILE* fh);

/**
 * Creates an omniscio_file handle from an MPI_File handle.
 */
int omniscio_file_from_mpiio(omniscio_file* result, MPI_File fh);

/**
 * Gets the list of predicted immediate next operations. The prediction
 * array is allocated and should be freed using omniscio_predict_free.
 * The size of the array is given by n after the call to the function.
 * n can be equal to 0 if Omnisc'IO has not been able to make a prediction.
 */
int omniscio_next(omniscio_req** prediction, int* n);

/**
 * Frees the array allocated by omniscio_predict_next.
 */
int omniscio_free(omniscio_req* prediction);

/**
 * After calling omniscio_next, one can call omniscio_predict_from 
 * to get up to n predicted consecutive operations from the immediate
 * next operations characterized by the index (which corresponds to the
 * index of in the "prediction" array returned by omniscio_next).
 * This function returns OMNISCIO_ERROR in case of error (e.g. if the
 * index or the value of n are invalid), otherwise it returns the number
 * of prediction effectively made, in which case the "predicted" array
 * has been allocated, of size n, and should be freed with omniscio_free.
 */
int omniscio_predict_from(int index, omniscio_req** predicted, int n);

/**
 * Finalizes Omnisc'IO. Should be called before calling MPI_Finalize.
 */
int omniscio_finalize(void);

extern int omniscio_tracing_enabled;

#define OMNISCIO_UNTRACED_START \
	int __ote = omniscio_tracing_enabled; \
	omniscio_tracing_enabled = 0;

#define OMNISCIO_UNTRACED_END \
	omniscio_tracing_enabled = __ote;

#ifdef __cplusplus
}
#endif

#endif
