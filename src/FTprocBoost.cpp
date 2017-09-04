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

#include "FTprocBoost.hpp"
#include "FTutils.hpp"

FTprocBoost::FTprocBoost (nframes_t samprate, unsigned int fftn)
	: FTprocI("EQ Boost", samprate, fftn)
{
	_confname = "Boost";
}

FTprocBoost::FTprocBoost (const FTprocBoost & other)
	: FTprocI (other._name, other._sampleRate, other._fftN)	
{
	_confname = "Boost";
}

void FTprocBoost::initialize()
{
	// create filter

	_eqfilter = new FTspectrumModifier("EQ Boost", "freq_boost", 0, FTspectrumModifier::POS_GAIN_MODIFIER, BOOST_SPECMOD, _fftN/2, 1.0);
	_eqfilter->setRange(1.0, 16.0);
	
	_filterlist.push_back (_eqfilter);

	_inited = true;
}

FTprocBoost::~FTprocBoost()
{
	if (!_inited) return;

        _filterlist.clear();
	delete _eqfilter;
}

void FTprocBoost::process (fft_data *data, unsigned int fftn)
{
	if (!_inited || _eqfilter->getBypassed()) {
		return;
	}

	
	float *filter = _eqfilter->getValues();
	float min = _eqfilter->getMin();
	float max = _eqfilter->getMax();
	float filt;

	int fftN2 = (fftn+1) >> 1;

	filt = FTutils::f_clamp(filter[0], min, max);
	data[0] *=  filt;
	
	for (int i = 1; i < fftN2-1; i++)
	{
		filt = FTutils::f_clamp(filter[i], min, max);
		
		data[i] *=  filt;
		data[fftn-i] *=  filt;
	}
}
