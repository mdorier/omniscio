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
#include <list>
#include <set>
#include <mpi.h>

#include "matrix.hpp"
#include "vector.hpp"
#include "sizes.hpp"
#include "offsets.hpp"
#include "stats/adaptive_stats.hpp"
#include "oracle.hpp"

#define START_TIMER(name)\
	double __timer_start_ ##name = MPI_Wtime()

#define END_TIMER(name) \
	double name = MPI_Wtime() - __timer_start_ ##name		

#define WINDOW_SIZE 10
#define OUTPUT_OK

using namespace omniscio;
using namespace omniscio::sequitur;

static matrix<adaptive_stats<double> > _trans_stats_;
static vector<size_tracker> _size_stats_;
static matrix<offset_tracker> _offset_stats_;
static std::map<unsigned long long,size_t> _offset_tracker_;
static double _previous_date_ = 0.0;
static int _previous_sym_ = 0;
static long _previous_offset_ = 0;
static size_t _previous_size_ = 0;

int main(int argc, char** argv)
{
	if(argc < 2) {
		std::cerr << "Usage: ./sequitur file" << std::endl;
		exit(0);
	}
	
	MPI_Init(&argc,&argv);

	std::ifstream ifs(argv[1], std::ifstream::in);

	oracle o;
	
	double ts, start, end;
	int sym, op, api, ret;
	long offset;
	long size;
	unsigned long long fd;


#ifdef OUTPUT_OK
	std::cout << "symbol, grammar_size, update_time, "
		  << "symbol_prediction(%%), cumulative_prediction(%%), "
		  << "size_error, offset_error, real_time, predicted_time"
		  << std::endl;
#endif
	std::list<double> _last_pred_prct_; // sliding window of prediction %
	std::set<int> _predictions_; // predictions made at the end of last op

	double _total_prediction_ = 0.0;
	long _num_operations_ = 0;

	while (1) {
		ifs >> ts;
		ifs >> sym;
		ifs >> op;
		ifs >> api;
		ifs >> offset;
		ifs >> size;
		ifs >> start;
		ifs >> end;
		ifs >> fd;
		ifs >> ret;
		if (ifs.eof()) break;

////////////////////////////////////////////////////////////////////////////////
///////////////// ANALYSIS OF PREDICTION PERFORMANCE ///////////////////////////
////////////////////////////////////////////////////////////////////////////////
		// Prediction of time
		std::set<int>::iterator it = _predictions_.begin();
		double predicted_time = 0.0;
		double real_time = start - _previous_date_; 
		if(_previous_sym_ == 0) real_time = 0.0;
		for(;it != _predictions_.end(); it++) {
			int p = *it;
			double t = _trans_stats_(_previous_sym_,p).get_adapted();
			predicted_time += t/(_predictions_.size());
		}

////////////////////////////////////////////////////////////////////////////////
///////////////// UPDATE OF THE MODEL //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
		START_TIMER(update_timer);

		// update the opened/closed files
		if(op == OMNISCIO_OPEN) {
			_offset_tracker_[fd] = 0;
		}

		// inserting symbol and updating statictics
		o.input(sym);
		// updating statistics on transition time
		if(_previous_sym_ != 0) {
			_trans_stats_(_previous_sym_,sym) 
				+= (start - _previous_date_);
		}

		// update the current offset of the file
		_offset_tracker_[fd] = offset + size;

		// erase offset if closing the file
		if(op == OMNISCIO_CLOSE) {
			_offset_tracker_.erase(fd);
		}

		END_TIMER(update_timer);

////////////////////////////////////////////////////////////////////////////////
///////////////// ANALYSIS OF PERFORMANCE //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
		// percentage of correct symbols predicted
		double pred_percent = 0.0;
		if(_predictions_.count(sym) > 0) {
			pred_percent = 1.0/_predictions_.size();
		}
		
		_total_prediction_ = _total_prediction_*_num_operations_ + pred_percent*100;
		_num_operations_ += 1;
		_total_prediction_ /= _num_operations_;

		_last_pred_prct_.push_back(pred_percent);
		if(_last_pred_prct_.size() == WINDOW_SIZE+1) 
			_last_pred_prct_.pop_front();
		pred_percent = 0.0;
		for(std::list<double>::iterator it = _last_pred_prct_.begin();
		    it != _last_pred_prct_.end(); it++) {
			pred_percent += *it;
		}
		pred_percent *= 100.0/WINDOW_SIZE;

		// Prediction of the size
		it = _predictions_.begin();
		double pred_size_avg = 0;
		for(;it != _predictions_.end(); it++) {
			size_t s = _size_stats_(*it).predict();
			pred_size_avg += (double)s/_predictions_.size();
		}
		double size_error = (size != 0) ? 
					std::abs((pred_size_avg-(double)size)/(double)size)
					: std::abs(pred_size_avg-(double)size);


		// Prediction of offset
		std::multiset<long> proposed_offsets;
		it = _predictions_.begin();
		for(;it != _predictions_.end(); it++) {
			offset_op op = _offset_stats_(_previous_sym_,*it).predict();
			long off = op.get_offset_after(_previous_offset_,
							_previous_size_);
			proposed_offsets.insert(off);
		}
		if(proposed_offsets.empty()) {
			proposed_offsets.insert(_previous_offset_+_previous_size_);
		}
		double offset_error = 100.0*proposed_offsets.count(offset)
					/proposed_offsets.size();

		// Hit ratio
		double hit_ratio = 0.0;
		it = _predictions_.begin();
		for(;it != _predictions_.end(); it++) {
			offset_op op = _offset_stats_(_previous_sym_,*it).predict();
			long p_off = op.get_offset_after(_previous_offset_,
							_previous_size_);
			size_t p_size = _size_stats_(*it).predict();
			size_t p_start = p_size != 0 ? 
						std::min(p_off,offset)
						: offset;
			size_t p_end = p_size != 0 ?
						std::max<size_t>(p_off+p_size,offset+size)
						: (offset+size);
			size_t overlap = 0;
			if(p_off <= offset) {
				overlap = (p_off+p_size) <= (unsigned long)offset ? 0
					: ((unsigned long)(offset+size) <= p_off+p_size ? size
					   : (p_off+p_size-offset)
					  );
			} else {
				overlap = offset+size <= p_off ? 0
					: (p_off+p_size <= (unsigned long)(offset+size) ? p_size
					   : (offset+size-p_off)
					  );
			}
			if(size != 0) {
				hit_ratio += 100.0*(double)overlap/((double)(p_end-p_start));
			} else {
				if(p_size == 0) {
					hit_ratio += 100.0;
				}
			}
		}
		if(_predictions_.size() != 0)
			hit_ratio /= _predictions_.size();

////////////////////////////////////////////////////////////////////////////////
///////////////// PRINTING RESULTS /////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#ifdef OUTPUT_OK
		std::cout << sym << ",\t" << o.size() << ",\t"
			  << std::fixed << std::setprecision(6) 
			  << update_timer << ",\t" 
			  << std::setprecision(2) << pred_percent << ",\t" 
			  << std::setprecision(9) << _total_prediction_ << ",\t"
			  << size_error << ",\t"
			  << offset_error << ",\t"
//			  << error_time << ",\t"
			  << real_time << ",\t"
			  << predicted_time << ",\t"
			  << hit_ratio << ",\t"
			  << (real_time-predicted_time) << ",\t"
			  << std::endl;
#endif
		_predictions_ = o.predict_next();
		// update statistics on size
                _size_stats_(sym).input(size);
		// update statistics on offset
		if(_previous_sym_ != 0) {
			offset_op op;
			if((long)(_previous_offset_ + _previous_size_)
			== offset) {
				_offset_stats_(_previous_sym_,sym).input(op);
			} else {
				if(offset == 0) {
					op = offset_op(offset,offset_op::ABSOLUTE);
				} else {
					long relative = offset
						- (_previous_offset_ + _previous_size_);
					op = offset_op(relative,offset_op::RELATIVE);
				}
			}
			_offset_stats_(_previous_sym_,sym).input(op);
		}

		_previous_size_ = size;
		_previous_offset_ = offset;
		_previous_sym_ = sym;
		_previous_date_ = end;
	}

	ifs.close();

	MPI_Finalize();
}

