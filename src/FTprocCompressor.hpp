/*
** Copyright (C) 2003 Jesse Chappell <jesse@essej.net>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**  
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**  
*/

#ifndef __FTPROCCOMPRESSOR_HPP__
#define __FTPROCCOMPRESSOR_HPP__

#include "FTprocI.hpp"
#include "FTutils.hpp"
#include <cmath>

#define RMS_BUF_SIZE 64

typedef struct {
	float        buffer[RMS_BUF_SIZE];
	unsigned int pos;
	float        sum;
} rms_env;


class FTprocCompressor
	: public FTprocI
{
  public:

	FTprocCompressor(nframes_t samprate, unsigned int fftn);
	FTprocCompressor (const FTprocCompressor & other);

	virtual ~FTprocCompressor();

	FTprocI * clone() { return new FTprocCompressor(*this); }
	void initialize();
	
	void process (fft_data *data,  unsigned int fftn);

	virtual void setFFTsize (unsigned int fftn);
	virtual void setOversamp (int osamp);

	virtual bool useAsDefault() { return false; }

	inline float db2lin(float db);
	inline float lin2db(float pow);
	
	
  protected:

	FTspectrumModifier * _thresh_filter;
	FTspectrumModifier * _ratio_filter;
	FTspectrumModifier * _attack_filter;
	FTspectrumModifier * _release_filter;
	FTspectrumModifier * _makeup_filter;


	// state
	
	float * _sum;
	float * _amp;
	float * _gain;
	float * _gain_t;
	float * _env;
	unsigned int * _count;
	float * _as;
	rms_env ** _rms;
	
	float _dbAdjust;
};


inline float FTprocCompressor::db2lin(float db)
{
	
	//float nval = (20.0 * FTutils::fast_log10(yval / refmax));
	float nval = ::pow ( (float)10.0, db/20);
	
	// printf ("scaled value is %g   mincut=%g\n", nval, _minCutoff);
	return nval;
}

inline float FTprocCompressor::lin2db(float power)
{
	//float db = FTutils::powerLogScale (power, 0.0000000) + _dbAdjust; // total fudge factors
	float db = FTutils::powerLogScale (power, 0.0000000);

	return db;
}
#endif
