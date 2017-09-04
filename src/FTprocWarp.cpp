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

#include "FTprocWarp.hpp"
#include "FTutils.hpp"

FTprocWarp::FTprocWarp (nframes_t samprate, unsigned int fftn)
	: FTprocI("Warp", samprate, fftn)
{
	_confname = "Warp";
}

FTprocWarp::FTprocWarp (const FTprocWarp & other)
	: FTprocI (other._name, other._sampleRate, other._fftN)	
{
	_confname = "Warp";
}

void FTprocWarp::initialize()
{
	// create filter

	_filter = new FTspectrumModifier("Warp", "warp", 0, FTspectrumModifier::FREQ_MODIFIER, WARP_SPECMOD, _fftN/2, 0.0);
	_filter->setRange(0.0, _fftN/2.0);
	_filter->reset();
	
	_filterlist.push_back (_filter);

	_tmpdata = new fft_data[FT_MAX_FFT_SIZE];
	
	_inited = true;
}

FTprocWarp::~FTprocWarp()
{
	if (!_inited) return;

        _filterlist.clear();
	delete _filter;

	delete [] _tmpdata;
}

void FTprocWarp::process (fft_data *data, unsigned int fftn)
{
	if (!_inited || _filter->getBypassed()) {
		return;
	}
	
	float *filter = _filter->getValues();
	float min = _filter->getMin();
	float max = _filter->getMax();
	float filt;

	int fftN2 = (fftn+1) >> 1;

	memset(_tmpdata, 0, fftn * sizeof(fft_data));

	for (int i = 1; i < fftN2-1; i++)
	{
		filt = FTutils::f_clamp(filter[i], min, max);
		
// 		_tmpdata[i] +=  data[(int)filt];
// 		_tmpdata[fftn-i] +=  data[fftn - ((int)filt)];
 		_tmpdata[(int)filt] +=  data[i];
		if (i > 0 && filt > 0) {
			_tmpdata[fftn-((int)filt)] +=  data[fftn - i];
		}
		
	}

	memcpy (data, _tmpdata, fftn * sizeof(fft_data));
}


void FTprocWarp::setFFTsize (unsigned int fftn)
{
	FTprocI::setFFTsize (fftn);

	// reset our filters max
	_filter->setRange (0.0, (float) (fftn >> 1));
	_filter->reset();
}
