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

#ifndef __FTMODROTATE_HPP__
#define __FTMODROTATE_HPP__

#include "FTmodulatorI.hpp"

class FTmodRotate
	: public FTmodulatorI
{
  public:

	FTmodRotate(nframes_t samplerate, unsigned int fftn);
	FTmodRotate (const FTmodRotate & other);

	virtual ~FTmodRotate();

	FTmodulatorI * clone() { return new FTmodRotate(*this); }
	void initialize();
	
	void modulate (nframes_t current_frame, fft_data * fftdata, unsigned int fftn, sample_t * timedata, nframes_t nframes);

	void setFFTsize (unsigned int fftn);
	
	
  protected:

	Control * _rate;
	Control * _minfreq;
	Control * _maxfreq;
	
	nframes_t _lastframe;
	float * _tmpfilt;
};

#endif
