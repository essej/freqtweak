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

#ifndef __FTPROCDELAY_HPP__
#define __FTPROCDELAY_HPP__


#include "FTprocI.hpp"

class RingBuffer;

class FTprocDelay
	: public FTprocI
{
  public:

	FTprocDelay (nframes_t samprate, unsigned int fftn);
	FTprocDelay (const FTprocDelay & other);

	virtual ~FTprocDelay();

	FTprocI * clone() { return new FTprocDelay(*this); }
	void initialize();

	
	void process (fft_data *data, unsigned int fftn);

	void reset();

	
	void setMaxDelay(float secs);
	
  protected:

	FTspectrumModifier * _delayFilter;
	FTspectrumModifier * _feedbackFilter;


	// this is a very large ringbuffer
	// used to store the fft results over time
	// each frame is stored sequentially.
	// the length is determined by the maximum delay time
	RingBuffer *_frameFifo;

	unsigned long _maxDelaySamples;
	float _maxDelay;

};

#endif
