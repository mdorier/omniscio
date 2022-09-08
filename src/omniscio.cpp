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

#include <execinfo.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <mpi.h>

#include "trace.hpp"
#include "dictionary.hpp"
#include "matrix.hpp"
#include "vector.hpp"
#include "model.hpp"
#include "sizes.hpp"
#include "offsets.hpp"
#include "stats/adaptive_stats.hpp"
#include "log.hpp"
#include "omniscio.h"

extern "C" {
	int omniscio_tracing_enabled = 0;
}

typedef int omniscio_symbol;

namespace omniscio {

static dictionary<omniscio_addr,omniscio_symbol> _dictionary_;
static model<omniscio_symbol> 			_model_;

static matrix<adaptive_stats<double> > 		_time_table_;
static vector<size_tracker> 			_size_table_;
static matrix<offset_tracker> 			_offset_table_;
static vector<omniscio_op_type>			_type_table_;

static logstream<std::ofstream> 		_predictions_;
static logstream<std::ofstream> 		_operations_;

static bool 					_enabled_ = false;
static bool 					_started_ = false;

static omniscio_date				_current_date_ = 0.0;
static omniscio_date				_previous_date_ = 0.0;
static omniscio_symbol 				_previous_sym_ = 0;
static omniscio_size 				_previous_size_ = 0;
static omniscio_offset 				_previous_offset_ = 0;

static const char* _api_name_[3] = {"POSIX","MPIIO","LIBC"};

int init(int* /*argc*/, char*** /*argv*/)
{
	_enabled_ = true;
	omniscio_tracing_enabled = 1;

	if(std::getenv("OMNISCIO_DISABLE") != NULL) {
		_enabled_ = false;
		return OMNISCIO_OK;
	}

	std::string wdir(".");
	char* w = std::getenv("OMNISCIO_DIRECTORY");
	if(w != NULL) wdir = std::string(w);

	time_t t;
	time(&t);

	MPI_Bcast(&t, sizeof(time_t), MPI_BYTE, 0, MPI_COMM_WORLD);
	int rank, size;
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	std::stringstream ss;
	ss << wdir << "/omniscio." << t << "."
	   << std::setw(log10((double)size)+1) 
	   << std::setfill('0') << rank << ".";

	_dictionary_.open(ss.str()+"dict");
	_model_.open(ss.str()+"model");
//	_time_table_.open(ss.str()+"time");
//	_size_table_.open(ss.str()+"size");
//	_offset_table_.open(ss.str()+"offset");
//	_type_table_.open(ss.str()+"type");
	_predictions_.open(ss.str()+"pred");
	_operations_.open(ss.str()+"log");

	return OMNISCIO_OK;
}

void update_offset(omniscio_offset current, omniscio_symbol sym) 
{
	if(_previous_sym_ == 0) return;

	offset_op op;
	if((omniscio_offset)(_previous_offset_ + _previous_size_) == current) {
		_offset_table_(_previous_sym_,sym).input(op);
	} else {
		if(current == 0) {
			op = offset_op(current,offset_op::ABSOLUTE);
		} else {
			long relative = 
				current - (_previous_offset_ + _previous_size_);
			op = offset_op(relative,offset_op::RELATIVE);
		}
		_offset_table_(_previous_sym_,sym).input(op);
	}
}

int open_start(const char* filename, omniscio_api_type api)
{
	if(not _enabled_) return OMNISCIO_OK;
	if(_started_) return OMNISCIO_ERROR;
	_started_ = true;

	_current_date_ = MPI_Wtime();

	// create or read symbol from trace
	trace t(256);
	if(t.size() == 0) return OMNISCIO_ERROR;
	omniscio_symbol sym = _dictionary_.insert(t);

	// logging the current operation
	omniscio_date now = MPI_Wtime();
	_operations_ << now << ' ' << sym
		<< " OPEN " << _api_name_[api]
		<< " _ " << filename;

	// inserting symbol
	_model_ << sym;

	// update type
	_type_table_(sym) = OMNISCIO_OPEN;

	// updating statistics on transition time
	if(_previous_sym_ != 0) {
		_time_table_(_previous_sym_,sym)
			+= (_current_date_ - _previous_date_);
	}
	
	// updating statistics on size
	_size_table_(sym).input(0);

	// updating statistics on offset
	update_offset(0,sym);

	// update global variables
	_previous_size_ 	= 0;
	_previous_offset_ 	= 0;
	_previous_sym_ 		= sym;

	return OMNISCIO_OK;
}

int open_end(int success, omniscio_file fh)
{
	if(not _enabled_) return OMNISCIO_OK;
	if(not _started_) return OMNISCIO_ERROR;
	_started_ = false;

	// logging the end of the current operation
	omniscio_date now = MPI_Wtime();
	_operations_ << ' ' << fh.handle.posix 
		<< ' ' << success << ' ' << now << '\n';
	_operations_.flush();

	_previous_date_ = MPI_Wtime();

	return OMNISCIO_OK;
}

int close_start(omniscio_file fh)
{
	if(not _enabled_) return OMNISCIO_OK;
	if(_started_) return OMNISCIO_ERROR;
	_started_ = true;
	
	_current_date_ = MPI_Wtime();

	// create or read symbol from trace
	trace t(256);
	if(t.size() == 0) return OMNISCIO_ERROR;
	omniscio_symbol sym = _dictionary_.insert(t);

	// logging the current operation
	omniscio_date now = MPI_Wtime();
	_operations_ << now << ' ' << sym
		<< " CLOSE " << _api_name_[fh.type]
		<< " _ _ " << fh.handle.posix;
	
	// inserting symbol
	_model_ << sym;

	// update type
	_type_table_(sym) = OMNISCIO_CLOSE;

	// updating statistics on transition time
	if(_previous_sym_ != 0) {
		_time_table_(_previous_sym_,sym)
			+= (_current_date_ - _previous_date_);
	}

	// update statistics on size
	_size_table_(sym).input(0);

	// updating statistics on offset
	update_offset(0,sym);

	// update global variables
	_previous_size_         = 0;
	_previous_offset_       = 0;
	_previous_sym_          = sym;

	return OMNISCIO_OK;
}

int close_end(int success)
{
	if(not _enabled_) return OMNISCIO_OK;
	if(not _started_) return OMNISCIO_ERROR;
	_started_ = false;
	
	// logging the end of the current operation
	omniscio_date now = MPI_Wtime();
	_operations_ << ' ' << success << ' ' << now << '\n';

	_previous_date_ = MPI_Wtime();

	return OMNISCIO_OK;
}

int write_start(omniscio_file fh, omniscio_offset offset, omniscio_size size)
{
	if(not _enabled_) return OMNISCIO_OK;
	if(_started_) return OMNISCIO_ERROR;
	_started_ = true;

	_current_date_ = MPI_Wtime();

	// create or read symbol from trace
	trace t(256);
	if(t.size() == 0) return OMNISCIO_ERROR;
	omniscio_symbol sym = _dictionary_.insert(t);

	// logging the current operation
	omniscio_date now = MPI_Wtime();
	_operations_ << now << ' ' << sym
		<< " WRITE " << _api_name_[fh.type]
		<< ' ' << offset << ' ' << size << ' ' << fh.handle.posix;

	// inserting symbol
	_model_ << sym;

	// update type
	_type_table_(sym) = OMNISCIO_WRITE;

	// updating statistics on transition time
	if(_previous_sym_ != 0) {
		_time_table_(_previous_sym_,sym)
			+= (_current_date_ - _previous_date_);
	}

	// updating statistics on size
	_size_table_(sym).input(size);

	// updating statistics on offset
	update_offset(offset,sym);

	// update global variables
	_previous_size_         = size;
	_previous_offset_       = offset;
	_previous_sym_          = sym;

	return OMNISCIO_OK;
}

int write_end(int success)
{
	if(not _enabled_) return OMNISCIO_OK;
	if(not _started_) return OMNISCIO_ERROR;
	_started_ = false;

	// logging the end of the current operation
	omniscio_date now = MPI_Wtime();
	_operations_ << ' ' << success << ' ' << now << '\n';

	_previous_date_ = MPI_Wtime();

	return OMNISCIO_OK;
}

int read_start(omniscio_file fh, omniscio_offset offset, omniscio_size size)
{
	if(not _enabled_) return OMNISCIO_OK;
	if(_started_) return OMNISCIO_ERROR;
	_started_ = true;

	_current_date_ = MPI_Wtime();

	// create or read symbol from trace
	trace t(256);
	if(t.size() == 0) return OMNISCIO_ERROR;
	omniscio_symbol sym = _dictionary_.insert(t);

	// logging the current operation
	omniscio_date now = MPI_Wtime();
	_operations_ << now << ' ' << sym
		<< " READ " << _api_name_[fh.type]
		<< ' ' << offset << ' ' << size << ' ' << fh.handle.posix;

	// inserting symbol
	_model_ << sym;

	// update type
	_type_table_(sym) = OMNISCIO_READ;

	// updating statistics on transition time
	if(_previous_sym_ != 0) {
		_time_table_(_previous_sym_,sym)
			+= (_current_date_ - _previous_date_);
	}

	// updating statistics on size
	_size_table_(sym).input(size);

	// updating statistics on offset
	update_offset(offset,sym);

	// update global variables
	_previous_size_         = size;
	_previous_offset_       = offset;
	_previous_sym_          = sym;
	
	return OMNISCIO_OK;
}

int read_end(int success)
{
	if(not _enabled_) return OMNISCIO_OK;
	if(not _started_) return OMNISCIO_ERROR;
	_started_ = false;

	// logging the end of the current operation
	omniscio_date now = MPI_Wtime();
	_operations_ << ' ' << success << ' ' << now << '\n';

	_previous_date_ = MPI_Wtime();

	return OMNISCIO_OK;
}

int finalize(void)
{
	if(not _enabled_) return OMNISCIO_OK;

	_dictionary_.close();
	_model_.close();
	//_time_table_.close();
	//_size_table_.close();
	//_offset_table_.close();
	//_type_table_.close();
	_predictions_.close();
	_operations_.close();
	_enabled_ = false;
	_started_ = false;

	omniscio_tracing_enabled = 0;

	return OMNISCIO_OK;
}

int predict_next(omniscio_req** prediction, int* n)
{
	if(_started_) {
		*n = 0;
		return OMNISCIO_ERROR;
	}

	std::vector<std::pair<omniscio_symbol,double> > pred_sym;
	_model_.predict(pred_sym);
	*n = pred_sym.size();
	*prediction = (omniscio_req*)malloc(sizeof(omniscio_req)*(*n));

	std::vector<std::pair<omniscio_symbol,double> >::iterator it = 
		pred_sym.begin();
	for(int i=0; it != pred_sym.end(); it++, i++) {
		omniscio_symbol next = it->first;
		double proba = it->second;
		// predict the size
		size_t size = _size_table_(next).predict();
		(*prediction)[i].size = size;
		// predict the offset		
		offset_op op = _offset_table_(_previous_sym_,next).predict();
		omniscio_offset offset = op.get_offset_after(_previous_offset_,
							     _previous_size_);
		(*prediction)[i].offset = offset;
		// predict the date
		omniscio_date date = 
			_time_table_(_previous_sym_,next).get_adapted();
		(*prediction)[i].date = date;
		// predict the type
		omniscio_op_type type = _type_table_(next);
		(*prediction)[i].type = type;
		// set probability
		(*prediction)[i].proba = proba;
	}

	return OMNISCIO_OK;
}

}

extern "C" {

int omniscio_finalize(void)
{
	return omniscio::finalize();
}

int omniscio_init(int* argc, char*** argv)
{	
	return omniscio::init(argc,argv);
}

int omniscio_open_start(const char* filename, omniscio_api_type t)
{
	return omniscio::open_start(filename,t);
}

int omniscio_open_end(int success, omniscio_file fh) 
{
	return omniscio::open_end(success,fh);
}

int omniscio_close_start(omniscio_file fh)
{
	return omniscio::close_start(fh);
}

int omniscio_close_end(int success)
{
	return omniscio::close_end(success);
}

int omniscio_write_start(omniscio_file fh,
	omniscio_offset offset, omniscio_size size)
{
	return omniscio::write_start(fh,offset,size);
}

int omniscio_write_end(int success)
{
	return omniscio::write_end(success);
}

int omniscio_read_start(omniscio_file fh,
	omniscio_offset offset, omniscio_size size)
{
	return omniscio::read_start(fh,offset,size);
}

int omniscio_read_end(int success)
{
	return omniscio::read_end(success);
}

int omniscio_file_from_posix(omniscio_file* result, int fd)
{
	result->handle.posix = fd;
	result->type = OMNISCIO_POSIX;
	return OMNISCIO_OK;
}

int omniscio_file_from_libc(omniscio_file* result, FILE* fh)
{
	result->handle.libc = fh;
	result->type = OMNISCIO_LIBC;
	return OMNISCIO_OK;
}

int omniscio_file_from_mpiio(omniscio_file* result, MPI_File fh)
{
	result->handle.mpiio = fh;
	result->type = OMNISCIO_MPIIO;
	return OMNISCIO_OK;
}

int omniscio_next(omniscio_req** prediction, int* n)
{
	return omniscio::predict_next(prediction,n);
}

int omniscio_free(omniscio_req* prediction)
{
	free(prediction);
	return OMNISCIO_OK;
}

int omniscio_predict_from(int /*index*/, omniscio_req** /*predicted*/, int /*n*/)
{
	// TODO
	return OMNISCIO_ERROR;
}

}
