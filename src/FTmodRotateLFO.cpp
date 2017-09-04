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

#include "FTmodRotateLFO.hpp"
#include "FTutils.hpp"

#include <cstdlib>
#include <cstdio>
#include <iostream>

using namespace std;
using namespace PBD;

FTmodRotateLFO::FTmodRotateLFO (nframes_t samplerate, unsigned int fftn)
	: FTmodulatorI ("RotateLFO", "Rotate LFO", samplerate, fftn)
{
}

FTmodRotateLFO::FTmodRotateLFO (const FTmodRotateLFO & other)
	: FTmodulatorI ("RotateLFO", "Rotate LFO", other._sampleRate, other._fftN)
{
}

void FTmodRotateLFO::initialize()
{
	_lastframe = 0;
	_lastshift = 0;
	
	_rate = new Control (Control::FloatType, "rate", "Rate", "Hz");
	_rate->_floatLB = 0.0;
	_rate->_floatUB = 20.0;
	_rate->setValue (0.0f);
	_controls.push_back (_rate);

	_depth = new Control (Control::FloatType, "depth", "Depth", "Hz");
	_depth->_floatLB = 0;
	_depth->_floatUB = (float) _sampleRate/2;
	_depth->setValue (_depth->_floatUB);
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

FTmodRotateLFO::~FTmodRotateLFO()
{
	if (!_inited) return;

	_controls.clear();

	delete _rate;
	delete _depth;
	delete _lfotype;
	delete _minfreq;
	delete _maxfreq;
}

void FTmodRotateLFO::setFFTsize (unsigned int fftn)
{
	_fftN = fftn;

	if (_inited) {
		delete _tmpfilt;
		_tmpfilt = new float[_fftN];
	}
	
}


void FTmodRotateLFO::modulate (nframes_t current_frame, fft_data * fftdata, unsigned int fftn, sample_t * timedata, nframes_t nframes)
{
	TentativeLockMonitor lm (_specmodLock, __LINE__, __FILE__);

	if (!lm.locked() || !_inited || _bypassed) return;

	float rate = 1.0;
	double currdev = 0.0;
	float ub,lb;
	float * filter;
	int len;
	int i,j;
	float minfreq, maxfreq;
	float depth = 1.0;
	int minbin, maxbin;
	double hzperbin;
	double current_secs;
	string shape;
	
	// in hz
	_rate->getValue (rate);
	_lfotype->getValue (shape);

	// in hz
	_depth->getValue (depth);
	
	_minfreq->getValue (minfreq);
	_maxfreq->getValue (maxfreq);

	if (minfreq >= maxfreq) {
		return;
	}

	hzperbin = _sampleRate / (double) fftn;

	// last_secs = _lastframe / (double) _sampleRate;
	current_secs = current_frame / (double) _sampleRate;

	int shiftval = 0;
	
	if (shape == "Sine")
	{
		currdev = (double) (FTutils::sine_wave (current_secs, (double) rate) * (depth * 0.5 / hzperbin));

	} else if (shape == "Square")
	{
		currdev = (double) (FTutils::square_wave (current_secs, (double) rate) * (depth * 0.5 / hzperbin));

	}
	else if (shape == "Triangle")
	{
		currdev = (double) (FTutils::triangle_wave (current_secs, (double) rate) * (depth * 0.5 / hzperbin));

	}
	else {
		return;
	}


	shiftval = (int) (currdev - _lastshift);		
		
	//cerr << "currdev: " << currdev << "  depth: " << depth << "  hzper: " << hzperbin << "  shift: " << shiftval << endl;

	
	if (current_frame != _lastframe && shiftval != 0)
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
			sm->getRange(lb, ub);
			len = (int) sm->getLength();
			minbin = (int) ((minfreq*2/ _sampleRate) * len);
			maxbin = (int) ((maxfreq*2/ _sampleRate) * len);

			len = maxbin - minbin;

			if (len <= 0) {
				continue;
			}
			
			int shiftbins = (abs(shiftval) % len) * (shiftval > 0 ? 1 : -1);


			
			// fprintf(stderr, "shifting %d  %d:%d  at %lu\n", shiftbins, minbin, maxbin, (unsigned long) current_frame);
			
			if (shiftbins > 0) {
				// shiftbins is POSITIVE, shift right
				// store last shiftbins
				for (i=maxbin-shiftbins; i < maxbin; i++) {
					_tmpfilt[i] = filter[i];
				}
				
				
				for ( i=maxbin-1; i >= minbin + shiftbins; i--) {
					filter[i] = filter[i-shiftbins];
				}
				
				for (j=maxbin-shiftbins, i=minbin; i < minbin + shiftbins; i++, j++) {
					filter[i] = _tmpfilt[j];
				}
			}
			else if (shiftbins < 0) {
				// shiftbins is NEGATIVE, shift left
				// store last shiftbins

				// store first shiftbins
				for (i=minbin; i < minbin-shiftbins; i++) {
					_tmpfilt[i] = filter[i];
				}
			
				for (i=minbin; i < maxbin + shiftbins; i++) {
					filter[i] = filter[i-shiftbins];
				}

				for (j=minbin, i=maxbin+shiftbins; i < maxbin; i++, j++) {
					filter[i] = _tmpfilt[j];
				}
			}
			
			
			sm->setDirty(true);
		}

		_lastframe = current_frame;
		_lastshift = (int) currdev;
	}
}
