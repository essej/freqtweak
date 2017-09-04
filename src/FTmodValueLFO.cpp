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

#include "FTmodValueLFO.hpp"
#include "FTutils.hpp"

#include <cstdlib>
#include <cstdio>
#include <iostream>

using namespace std;
using namespace PBD;

FTmodValueLFO::FTmodValueLFO (nframes_t samplerate, unsigned int fftn)
	: FTmodulatorI ("ValueLFO", "Value LFO", samplerate, fftn)
{
}

FTmodValueLFO::FTmodValueLFO (const FTmodValueLFO & other)
	: FTmodulatorI ("ValueLFO", "Value LFO", other._sampleRate, other._fftN)
{
}

void FTmodValueLFO::initialize()
{
	_lastframe = 0;
	
	_rate = new Control (Control::FloatType, "rate", "Rate", "Hz");
	_rate->_floatLB = 0.0;
	_rate->_floatUB = 20.0;
	_rate->setValue (0.0f);
	_controls.push_back (_rate);

	_depth = new Control (Control::FloatType, "depth", "Depth", "%");
	_depth->_floatLB = 0.0;
	_depth->_floatUB = 100.0;
	_depth->setValue (0.0f);
	_controls.push_back (_depth);

	_lfotype = new Control (Control::EnumType, "lfo_type", "LFO Type", "");
	_lfotype->_enumList.push_back("Sine");
	_lfotype->_enumList.push_back("Triangle");
	_lfotype->_enumList.push_back("Square");
	_lfotype->setValue (string("Sine"));
	_controls.push_back (_lfotype);

	
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

	
	
	
	_tmpfilt = new float[_fftN];
	
	_inited = true;
}

FTmodValueLFO::~FTmodValueLFO()
{
	if (!_inited) return;

	_controls.clear();

	delete _rate;
	delete _depth;
	delete _lfotype;
	delete _minfreq;
	delete _maxfreq;
}


void FTmodValueLFO::addSpecMod (FTspectrumModifier * specmod)
{
	{
		LockMonitor pmlock(_specmodLock, __LINE__, __FILE__);
		_lastshifts[specmod] = 0.0;
	}

	FTmodulatorI::addSpecMod (specmod);
}

void FTmodValueLFO::removeSpecMod (FTspectrumModifier * specmod)
{
	FTmodulatorI::removeSpecMod (specmod);

	LockMonitor pmlock(_specmodLock, __LINE__, __FILE__);
	_lastshifts.erase(specmod);
}

void FTmodValueLFO::clearSpecMods ()
{
	FTmodulatorI::clearSpecMods();

	LockMonitor pmlock(_specmodLock, __LINE__, __FILE__);
	_lastshifts.clear();
}


void FTmodValueLFO::setFFTsize (unsigned int fftn)
{
	_fftN = fftn;

	if (_inited) {
		delete _tmpfilt;
		_tmpfilt = new float[_fftN];
	}
	
}


void FTmodValueLFO::modulate (nframes_t current_frame, fft_data * fftdata, unsigned int fftn, sample_t * timedata, nframes_t nframes)
{
	TentativeLockMonitor lm (_specmodLock, __LINE__, __FILE__);

	if (!lm.locked() || !_inited || _bypassed) return;

	float rate = 1.0;
	double currdev = 0.0;
	float ub,lb, tmplb, tmpub;
	float * filter;
	int len;
	float minfreq, maxfreq;
	float depth = 1.0;
	unsigned int minbin, maxbin;
	float shiftval = 0;
	double current_secs;
	double lastshift;
	string shape;
	
	// in hz
	_rate->getValue (rate);

	_lfotype->getValue (shape);

	// in %
	_depth->getValue (depth);
	
	_minfreq->getValue (minfreq);
	_maxfreq->getValue (maxfreq);

	if (minfreq >= maxfreq) {
		return;
	}

	current_secs = current_frame / (double) _sampleRate;

	
	if (current_frame != _lastframe)
	{
		// fprintf (stderr, "shift at %lu :  samps=%g  s*c=%g  s*e=%g \n", (unsigned long) current_frame, samps, (current_frame/samps), ((current_frame + nframes)/samps) );

		
		for (SpecModList::iterator iter = _specMods.begin(); iter != _specMods.end(); ++iter)
		{
			FTspectrumModifier * sm = (*iter);
			if (sm->getBypassed()) continue;

// 			cerr << "shiftval is: " << shiftval
// 			     << " hz/bin: " << hzperbin
// 			     << " rate:   " << rate << endl;
			
			filter = sm->getValues();
			sm->getRange(tmplb, tmpub);
			len = (int) sm->getLength();
			minbin = (int) ((minfreq*2/ _sampleRate) * len);
			maxbin = (int) ((maxfreq*2/ _sampleRate) * len);

//  			lb = tmplb + (tmpub-tmplb) * minval*0.01;
//  			ub = tmplb + (tmpub-tmplb) * maxval*0.01;
 			lb = tmplb;
 			ub = tmpub;
			
			len = maxbin - minbin;

			if (len <= 0) {
				continue;
			}

			if (shape == "Sine")
			{
				currdev = (double) (FTutils::sine_wave (current_secs, (double) rate) * ( (ub-lb)* (depth * 0.01) * 0.5 ));
				
			} else if (shape == "Square")
			{
				currdev = (double) (FTutils::square_wave (current_secs, (double) rate) * ( (ub-lb)* (depth * 0.01) * 0.5 ));
				
			}
			else if (shape == "Triangle")
			{
				currdev = (double) (FTutils::triangle_wave (current_secs, (double) rate) * ( (ub-lb)* (depth * 0.01) * 0.5 ));
			}
			else {
				continue;
			}
			
			lastshift = _lastshifts[sm];
			shiftval = (float) (currdev - lastshift);		
		
			// fprintf(stderr, "shifting %d  %d:%d  at %lu\n", shiftbins, minbin, maxbin, (unsigned long) current_frame);

			for (unsigned int i=minbin; i < maxbin; ++i) {
				filter[i] += shiftval;
			}
			
			sm->setDirty(true);

			_lastshifts[sm] = currdev;
		}

		_lastframe = current_frame;
	}
}
