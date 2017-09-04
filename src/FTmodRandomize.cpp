/*
** Copyright (C) 2004 Jesse Chappell <jesse@essej.net>
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

#include "FTmodRandomize.hpp"
#include <cstdlib>
#include <cstdio>
#include <iostream>

using namespace std;
using namespace PBD;

FTmodRandomize::FTmodRandomize (nframes_t samplerate, unsigned int fftn)
	: FTmodulatorI ("Randomize","Randomize", samplerate, fftn)
{
}

FTmodRandomize::FTmodRandomize (const FTmodRandomize & other)
	: FTmodulatorI ("Randomize", "Randomize", other._sampleRate, other._fftN)
{
}

void FTmodRandomize::initialize()
{
	_lastframe = 0;
	
	_rate = new Control (Control::FloatType, "rate", "Rate", "Hz");
	_rate->_floatLB = 0.0;
	_rate->_floatUB = 20.0;
	_rate->setValue (0.0f);
	_controls.push_back (_rate);

	_minval = new Control (Control::FloatType, "min_val", "Min Val", "%");
	_minval->_floatLB = 0.0;
	_minval->_floatUB = 100.0;
	_minval->setValue (_minval->_floatLB);
	_controls.push_back (_minval);

	_maxval = new Control (Control::FloatType, "max_val", "Max Val", "%");
	_maxval->_floatLB = 0.0;
	_maxval->_floatUB = 100.0;
	_maxval->setValue (_maxval->_floatUB);
	_controls.push_back (_maxval);
	
	_minfreq = new Control (Control::FloatType, "min_freq", "Min Freq", "Hz");
	_minfreq->_floatLB = 0.0;
	_minfreq->_floatUB = _sampleRate / 2;
	_minfreq->setValue (_minfreq->_floatLB);
	_controls.push_back (_minfreq);

	_maxfreq = new Control (Control::FloatType, "max_freq", "Max Freq", "Hz");
	_maxfreq->_floatLB = 0.0;
	_maxfreq->_floatUB = _sampleRate / 2;
	_maxfreq->setValue (_maxfreq->_floatUB);
	_controls.push_back (_maxfreq);

	
	
	srand(0);
	
	_inited = true;
}

FTmodRandomize::~FTmodRandomize()
{
	if (!_inited) return;

	_controls.clear();

	delete _rate;
	delete _minfreq;
	delete _maxfreq;
	delete _minval;
	delete _maxval;
	
}

void FTmodRandomize::modulate (nframes_t current_frame, fft_data * fftdata, unsigned int fftn, sample_t * timedata, nframes_t nframes)
{
	TentativeLockMonitor lm (_specmodLock, __LINE__, __FILE__);

	if (!lm.locked() || !_inited || _bypassed) return;

	float rate = 1.0;
	float ub,lb, tmplb, tmpub;
	float * filter;
	unsigned int len;
	
	_rate->getValue (rate);

	if (rate == 0.0)
		return;
	
	unsigned int minbin, maxbin;
	float minval, maxval;
	float minfreq,maxfreq;
	
	double samps = _sampleRate / rate;

	_minval->getValue (minval);
	_maxval->getValue (maxval);

	if (minval > maxval) {
		minval = maxval;
	}

	_minfreq->getValue (minfreq);
	_maxfreq->getValue (maxfreq);

	if (minfreq >= maxfreq) {
		return;
	}

	double delta = current_frame - _lastframe;
	
	if (delta >= samps) 
	{
		// fprintf (stderr, "randomize at %lu :  samps=%g  s*c=%g  s*e=%g \n", (unsigned long) current_frame, samps, (current_frame/samps), ((current_frame + nframes)/samps) );
		
		for (SpecModList::iterator iter = _specMods.begin(); iter != _specMods.end(); ++iter)
		{
			FTspectrumModifier * sm = (*iter);
			if (sm->getBypassed()) continue;

			filter = sm->getValues();
			sm->getRange(tmplb, tmpub);
			len = sm->getLength();

			lb = tmplb + (tmpub-tmplb) * minval * 0.01;
			ub = tmplb + (tmpub-tmplb) * maxval * 0.01;

// 			cerr << " lb: " << lb << "  ub: " << ub
// 			     << " minval: " << minval << "   maxval: " << maxval
// 			     << " tmlb: " << tmplb << "  tmpub: " << tmpub
// 			     << endl;
			
			minbin = (int) ((minfreq*2/ _sampleRate) * len);
			maxbin = (int) ((maxfreq*2/ _sampleRate) * len);
			
			// crap random
			for (unsigned int i=minbin; i < maxbin; ++i) {
				filter[i] = lb + (float) ((ub-lb) * rand() / (RAND_MAX+1.0));
			}

			sm->setDirty(true);
		}

		_lastframe = current_frame;
	}
}
