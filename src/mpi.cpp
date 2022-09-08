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

#include <mpi.h>
#include <iostream>
#include <stdlib.h>
#include "omniscio.h"

extern "C" {

extern int omniscio_tracing_enabled;

int MPI_Init(int* argc, char*** argv) {
	int ret = PMPI_Init(argc,argv);
	omniscio_init(argc,argv);
	return ret;
}

void mpi_init_(MPI_Fint* error) {
	*error = PMPI_Init(0,0);
	omniscio_init(0,0);
}

int MPI_Finalize() {
	omniscio_finalize();
	return PMPI_Finalize();
}

void mpi_finalize_(MPI_Fint* error) {
	omniscio_finalize();
	*error = PMPI_Finalize();
}

int MPI_Send(const void *buf, int count, 
		MPI_Datatype datatype, int dest, int tag,
		MPI_Comm comm)
{
	int old = omniscio_tracing_enabled;
	omniscio_tracing_enabled = 0;
	int err = PMPI_Send(buf,count,datatype,dest,tag,comm);
	omniscio_tracing_enabled = old;
	return err;
}

int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source,
              int tag, MPI_Comm comm, MPI_Request *request)
{
	int old = omniscio_tracing_enabled;
	omniscio_tracing_enabled = 0;
	int err = PMPI_Irecv(buf,count,datatype,source,tag,comm,request);
	omniscio_tracing_enabled = old;
	return err;
}

int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype,
    int root, MPI_Comm comm) 
{
	int old = omniscio_tracing_enabled;
	omniscio_tracing_enabled = 0;
	int err = PMPI_Bcast(buffer,count,datatype,root,comm);
	omniscio_tracing_enabled = old;
	return err;
}

void mpi_bcast_(void *buffer, MPI_Fint* count, MPI_Fint* datatype,
		MPI_Fint* root, MPI_Fint* comm, MPI_Fint* err) 
{
	int old = omniscio_tracing_enabled;
	omniscio_tracing_enabled = 0;
	*err = PMPI_Bcast(buffer,*count,MPI_Type_f2c(*datatype),*root,MPI_Comm_f2c(*comm));
	omniscio_tracing_enabled = old;
}

int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count,
    MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) 
{
	int old = omniscio_tracing_enabled;
	omniscio_tracing_enabled = 0;
	int err = PMPI_Allreduce(sendbuf,recvbuf,count,datatype,op,comm);
	omniscio_tracing_enabled = old;
	return err;
}

int MPI_Waitall(int count, MPI_Request *array_of_requests,
    MPI_Status *array_of_statuses)
{
	int old = omniscio_tracing_enabled;
	omniscio_tracing_enabled = 0;
	int err = PMPI_Waitall(count,array_of_requests,array_of_statuses);
	omniscio_tracing_enabled = old;
	return err;
}

int MPI_Waitany(int count, MPI_Request *array_of_requests,
    int *index, MPI_Status *status)
{
	int old = omniscio_tracing_enabled;
	omniscio_tracing_enabled = 0;
	int err = PMPI_Waitany(count,array_of_requests,index,status);
	omniscio_tracing_enabled = old;
	return err;
}

void mpi_waitany_(MPI_Fint* count, MPI_Fint* array_of_requests,
	MPI_Fint* index, MPI_Fint* status, MPI_Fint* err)
{
	MPI_Status cstatus;
	MPI_Status_f2c(status, &cstatus);
	MPI_Request* c_req = (MPI_Request*)malloc(sizeof(MPI_Request)*(*count));
	int i;
	for(i=0;i<*count;i++) {
		c_req[i] = MPI_Request_f2c(array_of_requests[i]);
	}
	int old = omniscio_tracing_enabled;
        omniscio_tracing_enabled = 0;
	*err = PMPI_Waitany(*count,c_req,index,&cstatus);
	*index += 1;
	MPI_Status_c2f(&cstatus, status);
	for(i=0;i<*count;i++) {
		array_of_requests[i] = MPI_Request_c2f(c_req[i]);
	}
	omniscio_tracing_enabled = old;
	free(c_req);
}

int MPI_Wait(MPI_Request *request, MPI_Status *status)
{
	int old = omniscio_tracing_enabled;
	omniscio_tracing_enabled = 0;
	int err = PMPI_Wait(request,status);
	omniscio_tracing_enabled = old;
	return err;
}

}
