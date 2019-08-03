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
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**  
*/

#include "FTmodRotate.hpp"
#include <cstdlib>
#include <cstdio>
#include <iostream>

using namespace std;
using namespace PBD;

FTmodRotate::FTmodRotate (nframes_t samplerate, unsigned int fftn)
	: FTmodulatorI ("Rotate", "Rotate", samplerate, fftn)
{
}

FTmodRotate::FTmodRotate (const FTmodRotate & other)
	: FTmodulatorI ("Rotate", "Rotate", other._sampleRate, other._fftN)
{
}

void FTmodRotate::initialize()
{
	_lastframe = 0;
	
	_rate = new Control (Control::FloatType, "rate", "Rate", "Hz/sec");
	_rate->_floatLB = -((float) _sampleRate);
	_rate->_floatUB =  (float) _sampleRate;
	_rate->setValue (0.0f);
	_controls.push_back (_rate);

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

	
	
// 	_dimension = new Control (Control::EnumType, "Target", "");
// 	_dimension->_enumList.push_back("Frequency");
// 	_dimension->_enumList.push_back("Value");
// 	_dimension->setValue ("Frequency");
// 	_controls.push_back (_dimension);
	
	_tmpfilt = new float[_fftN];
	
	_inited = true;
}

FTmodRotate::~FTmodRotate()
{
	if (!_inited) return;

	_controls.clear();

	delete _rate;
	delete _minfreq;
	delete _maxfreq;
}

void FTmodRotate::setFFTsize (unsigned int fftn)
{
	_fftN = fftn;

	if (_inited) {
		delete _tmpfilt;
		_tmpfilt = new float[_fftN];
	}
	
}


void FTmodRotate::modulate (nframes_t current_frame, fft_data * fftdata, unsigned int fftn, sample_t * timedata, nframes_t nframes)
{
	TentativeLockMonitor lm (_specmodLock, __LINE__, __FILE__);

	if (!lm.locked() || !_inited || _bypassed) return;

	float rate = 1.0;
	float ub,lb;
	float * filter;
	int len;
	int i,j;
	float minfreq, maxfreq;
	int minbin, maxbin;
	double hzperbin;
	
	// in hz/sec
	_rate->getValue (rate);
	
	_minfreq->getValue (minfreq);
	_maxfreq->getValue (maxfreq);

	if (minfreq >= maxfreq) {
		return;
	}

	hzperbin = _sampleRate / (double) fftn;

	// shiftval (bins) = deltasamples / (samp/sec) * (hz/sec) / (hz/bins)

	// bins = sec * hz/sec 
	
	int shiftval = (int) (((current_frame - _lastframe) / (double) _sampleRate) * rate / hzperbin);

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
	}
}
