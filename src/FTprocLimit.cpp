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

#include "FTprocLimit.hpp"
#include "FTutils.hpp"
#include <cmath>
using namespace std;

FTprocLimit::FTprocLimit (nframes_t samprate, unsigned int fftn)
	: FTprocI("Limit", samprate, fftn)
	  , _dbAdjust(-48.0)
	  
{
	_confname = "Limit";
}

FTprocLimit::FTprocLimit (const FTprocLimit & other)
	: FTprocI (other._name, other._sampleRate, other._fftN)	
	  , _dbAdjust(-48.0)
{
	_confname = "Limit";
}

void FTprocLimit::initialize()
{
	// create filter

	_threshfilter = new FTspectrumModifier("Limit", "limit_thresh", 0, FTspectrumModifier::DB_MODIFIER, MASH_SPECMOD, _fftN/2, 0.0);
	_threshfilter->setRange(-90.0, 0.0);
	
	_filterlist.push_back (_threshfilter);

	_inited = true;
}

FTprocLimit::~FTprocLimit()
{
	if (!_inited) return;

        _filterlist.clear();
	delete _threshfilter;
}

void FTprocLimit::process (fft_data *data, unsigned int fftn)
{
	if (!_inited || _threshfilter->getBypassed()) {
		return;
	}
	
	float *filter = _threshfilter->getValues();
	float min = _threshfilter->getMin();
	float max = _threshfilter->getMax();
	float filt;
	float power;
	float db;
	float scale;

	
	int fftN2 = (fftn+1) >> 1;

	// do for first element
	filt = FTutils::f_clamp (filter[0], min, max);
	power = (data[0] * data[0]);
	db = FTutils::powerLogScale (power, 0.0000000) + _dbAdjust; // total fudge factors
	if (filt < db) {
		// apply limiting
		scale = 1 / (pow (2, (db-filt) / 6.0));
		data[0] *=  scale;
	}
	
	// do for the rest
	for (int i = 1; i < fftN2-1; i++)
	{
		filt = FTutils::f_clamp (filter[i], min, max);
		power = (data[i] * data[i]) + (data[fftn-i] * data[fftn-i]);
		db = FTutils::powerLogScale (power, 0.0000000) + _dbAdjust; // total fudge factors

		if (filt < db) {
			// apply limiting
			scale = 1 / (pow (2, (db-filt) / 6.0));
			
			data[i] *=  scale;
			data[fftn-i] *=  scale;
		}
		
	}
}
