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

#include "FTprocDelay.hpp"
#include "RingBuffer.hpp"

FTprocDelay::FTprocDelay (nframes_t samprate, unsigned int fftn)
	: FTprocI("Delay", samprate, fftn),
	  _delayFilter(0), _feedbackFilter(0), _frameFifo(0), _maxDelay(2.5)
{
	_confname = "Delay";
}

FTprocDelay::FTprocDelay (const FTprocDelay & other)
	: FTprocI (other._name, other._sampleRate, other._fftN),	
	_delayFilter(0), _feedbackFilter(0), _frameFifo(0), _maxDelay(2.5)

{
	_confname = "Delay";
}

void FTprocDelay::initialize()
{
	// create filters
	
	_delayFilter = new FTspectrumModifier("Delay", "delay", 0, FTspectrumModifier::TIME_MODIFIER, DELAY_SPECMOD, _fftN/2, 0.0);
	_delayFilter->setRange(0.0, _maxDelay);
	
	_feedbackFilter = new FTspectrumModifier("D Feedback", "feedback", 1, FTspectrumModifier::UNIFORM_MODIFIER, FEEDB_SPECMOD, _fftN/2, 0.0);
	_feedbackFilter->setRange(0.0, 1.0);

	setMaxDelay(_maxDelay);

	_filterlist.push_back(_delayFilter);
	_filterlist.push_back(_feedbackFilter);

	_inited = true;
}

FTprocDelay::~FTprocDelay()
{
	if (!_inited) return;
	
	delete _frameFifo;
	
        _filterlist.clear();
	delete _delayFilter;
	delete _feedbackFilter;
}

void FTprocDelay::reset()
{
	// flush FIFOs
	_frameFifo->reset();
	_frameFifo->mem_set(0);
}

void FTprocDelay::setMaxDelay(float secs)
{
	// THIS MUST NOT BE CALLED WHILE WE ARE ACTIVATED!
	if (secs <= 0.0) return;
	
	unsigned long maxsamples = 1;

	_maxDelay = secs;
	_maxDelaySamples = (unsigned long) (_maxDelay * _sampleRate) * sizeof(sample_t);

	// we need to force this to the next bigger power of 2 for the memory allocation
	while (maxsamples < _maxDelaySamples) {
		maxsamples <<= 1;
	}
	       
	if (_frameFifo)
		delete _frameFifo;

	//printf ("using %lu for maxsamples\n", maxsamples);

	// this is a big boy containing the frequency data frames over time
	_frameFifo = new RingBuffer( maxsamples * sizeof(fft_data) );

	// adjust time filter
	if (_delayFilter) {
		_delayFilter->setRange (0.0, _maxDelay);
		_delayFilter->reset();
	}
}


void FTprocDelay::process (fft_data *data, unsigned int fftn)
{
	if (!_inited) return;
	
	if (_delayFilter->getBypassed())
	{
		_frameFifo->write ( (char *) data, sizeof(fft_data) * fftn);

		_frameFifo->read( (char *) data, sizeof(fft_data) * fftn);
		
		// RETURNS HERE
		return;
	}

	float *delay = _delayFilter->getValues();
	float *feedb = _feedbackFilter->getValues();
	float feedback = 0.0;
	bool bypassfeed = _feedbackFilter->getBypassed();
	
	float mindelay = _delayFilter->getMin();
	float maxdelay = _delayFilter->getMax();
	float thisdelay;
	
	float *rdest = 0, *idest = 0;
	float *rcurr = 0, *icurr = 0;
	nframes_t bshift, fshift;
	int fftn2 = (fftn+1) >> 1;

	
	//RingBuffer::rw_vector readvec[2];
	RingBuffer::rw_vector wrvec[2];

	_frameFifo->get_write_vector(wrvec);

	
	for (int i = 0; i < fftn2-1; i++)
	{
		if (bypassfeed) {
			feedback = 0.0;
		}
		else {
			feedback = feedb[i] < 0.0 ? 0.0 : feedb[i];
		}
		
		if (delay[i] > maxdelay) {
			thisdelay = maxdelay;
		}
		else if (delay[i] <= mindelay) {
			// force feedback to 0 if no delay
			feedback = 0.0;
			thisdelay = mindelay;
		}
		else {
			thisdelay = delay[i];
		}
		
		// frames to shift
		fshift = ((nframes_t)(_sampleRate * thisdelay * sizeof(sample_t))) / fftn;
		//nframes_t bshift = fshift * fftn * sizeof(sample_t);
		//printf ("bshift %Zd  wrvec[0]=%d\n", bshift, wrvec[0].len);
		// byte offset to start of frame
		//bshift = fshift * fftn * sizeof(sample_t);
		bshift = fshift * fftn * sizeof(sample_t);


		// we know the next frame is in the first segment of the FIFO
		// because our FIFO size is always shifted one frame at a time
		rcurr = (float * ) (wrvec[0].buf + i*sizeof(sample_t));
		icurr = (float * ) (wrvec[0].buf + (fftn-i)*sizeof(sample_t));
		
		if (wrvec[0].len > bshift) {
			rdest = (float *) (wrvec[0].buf + bshift + i*sizeof(sample_t));
			idest = (float *) (wrvec[0].buf + bshift + (fftn-i)*sizeof(sample_t));
		}
		else if (wrvec[1].len) {
			bshift -= wrvec[0].len;
			rdest = (float *) (wrvec[1].buf + bshift + i*sizeof(sample_t));
			idest = (float *) (wrvec[1].buf + bshift + (fftn-i)*sizeof(sample_t));
		}
		else {
			printf ("BLAHHALALAHLA\n");
			continue;
		}
		
		
		*rdest = data[i] + (*rcurr)*feedback;
		if (i > 0) {
			*idest = data[fftn-i] + (*icurr)*feedback;
		}
	}
	
	// advance it
	_frameFifo->write_advance(fftn * sizeof(fft_data));
	
	//_frameFifo->write ( (char *) data, sizeof(fft_data) * fftn);

	// read data into output
	_frameFifo->read( (char *) data, sizeof(fft_data) * fftn);

}
