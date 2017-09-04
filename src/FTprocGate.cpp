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

#include "FTprocGate.hpp"
#include "FTutils.hpp"

FTprocGate::FTprocGate (nframes_t samprate, unsigned int fftn)
	: FTprocI("Gate", samprate, fftn)
	  , _dbAdjust(-37.0)
	
{
	_confname = "Gate";
}

FTprocGate::FTprocGate (const FTprocGate & other)
	: FTprocI (other._name, other._sampleRate, other._fftN)
	  , _dbAdjust(other._dbAdjust)
{
	_confname = "Gate";
}

void FTprocGate::initialize()
{
	// create filters

	_filter = new FTspectrumModifier("Gate Bottom", "gate", 0, FTspectrumModifier::DB_MODIFIER, GATE_SPECMOD, _fftN/2, -90.0);
	_filter->setRange(-90.0, 0.0);
	//_gateFilter->reset();
	_filter->setBypassed(true); // by default
	
	_invfilter = new FTspectrumModifier("Gate", "inverse_gate", 0, FTspectrumModifier::DB_MODIFIER, GATE_SPECMOD, _fftN/2, 0.0);
	_invfilter->setRange(-90.0, 0.0);
	_invfilter->setBypassed(true); // by default

	_filterlist.push_back(_invfilter);
	_filterlist.push_back(_filter);

	_inited = true;
}

FTprocGate::~FTprocGate()
{
	if (!_inited) return;

	_filterlist.clear();
	delete _filter;
	delete _invfilter;
}

void FTprocGate::process (fft_data *data, unsigned int fftn)
{
	if (!_inited || _filter->getBypassed()) {
		return;
	}
	
	float *filter = _filter->getValues();
	float *invfilter = _invfilter->getValues();
	
	float power;
	float db;
	int fftn2 = (fftn+1) >> 1;
	
	// only allow data through if power is above threshold

	power = (data[0] * data[0]);
	db = FTutils::powerLogScale (power, 0.0000000) + _dbAdjust; // total fudge factors
	if (db < filter[0] || db > invfilter[0]) {
		data[0] = 0.0;
	}
	
 	for (int i = 1; i < fftn2-1; i++)
 	{
		power = (data[i] * data[i]) + (data[fftn-i] * data[fftn-i]);
		db = FTutils::powerLogScale (power, 0.0000000) + _dbAdjust; // total fudge factors

		if (db < filter[i] || db > invfilter[i])
		{
			//printf ("db %g\n", db);

			data[i] = data[fftn-i] = 0.0;
 		}
 	}
	
}
