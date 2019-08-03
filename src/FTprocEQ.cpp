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
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**  
*/

#include "FTprocEQ.hpp"
#include "FTutils.hpp"


FTprocEQ::FTprocEQ (nframes_t samprate, unsigned int fftn)
	: FTprocI("EQ Cut", samprate, fftn)
{
	_confname = "EQ";
}

FTprocEQ::FTprocEQ (const FTprocEQ & other)
	: FTprocI (other._name, other._sampleRate, other._fftN)	
{
	_confname = "EQ";
}

void FTprocEQ::initialize()
{
	// create filter

	_eqfilter = new FTspectrumModifier("EQ Cut", "freq", 0, FTspectrumModifier::GAIN_MODIFIER, FREQ_SPECMOD, _fftN/2, 1.0);
	_eqfilter->setRange(0.0, 1.0);
	
	_filterlist.push_back (_eqfilter);

	_inited = true;
}

FTprocEQ::~FTprocEQ()
{
	if (!_inited) return;

        _filterlist.clear();
	delete _eqfilter;
}

void FTprocEQ::process (fft_data *data, unsigned int fftn)
{
	if (!_inited || _eqfilter->getBypassed()) {
		return;
	}
	
	float *filter = _eqfilter->getValues();
	float min = _eqfilter->getMin();
	float max = _eqfilter->getMax();
	float filt;

	int fftN2 = fftn/2;

	filt = FTutils::f_clamp (filter[0], min, max);
	data[0] *= filt;
	
	for (int i = 1; i < fftN2-1; i++)
	{
		filt = FTutils::f_clamp (filter[i], min, max);
		
		data[i] *=  filt;
		data[fftn-i] *=  filt;
	}
}
