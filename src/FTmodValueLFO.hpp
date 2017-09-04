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

#ifndef __FTMODVALUELFO_HPP__
#define __FTMODVALUELFO_HPP__

#include "FTmodulatorI.hpp"

#include <map>

class FTmodValueLFO
	: public FTmodulatorI
{
  public:

	FTmodValueLFO(nframes_t samplerate, unsigned int fftn);
	FTmodValueLFO (const FTmodValueLFO & other);

	virtual ~FTmodValueLFO();

	FTmodulatorI * clone() { return new FTmodValueLFO(*this); }
	void initialize();
	
	void modulate (nframes_t current_frame, fft_data * fftdata, unsigned int fftn, sample_t * timedata, nframes_t nframes);

	void setFFTsize (unsigned int fftn);
	
	void addSpecMod (FTspectrumModifier * specmod);
	void removeSpecMod (FTspectrumModifier * specmod);
	void clearSpecMods ();
	
  protected:

	Control * _rate;
	Control * _depth;
	Control * _lfotype;
	Control * _minfreq;
	Control * _maxfreq;
	
	nframes_t _lastframe;

	std::map<FTspectrumModifier *, double> _lastshifts;
	
	float * _tmpfilt;
};

#endif
