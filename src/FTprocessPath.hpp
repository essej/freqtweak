/*
** Copyright (C) 2002 Jesse Chappell <jesse@essej.net>
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

#ifndef __FTPROCESSPATH_HPP__
#define __FTPROCESSPATH_HPP__


#include "FTtypes.hpp"

class RingBuffer;
class FTspectralEngine;

class FTprocessPath
{
  public:
	FTprocessPath();
	virtual ~FTprocessPath();

	void setId (int id);
	int getId () { return _id; }
	
	void setMaxBufsize (nframes_t bsize) { _maxBufsize = bsize; }
	void setSampleRate (nframes_t srate) { _sampleRate = srate; }

	//void setSpectralEngine (FTspectralEngine * sengine) { _specEngine = sengine; }
	FTspectralEngine * getSpectralEngine () { return _specEngine; }
	
	void processData (sample_t *inbuf, sample_t *outbuf, nframes_t nframes);

	RingBuffer * getInputFifo() { return _inputFifo; }
	RingBuffer * getOutputFifo() { return _outputFifo; }

	bool getReadyToDie() { return _readyToDie; }
	void setReadyToDie(bool flag) { _readyToDie = flag; } 
	
 protected:

	void initSpectralEngine();
	
	
	nframes_t _maxBufsize;
	nframes_t _sampleRate;

	RingBuffer * _inputFifo;
	RingBuffer * _outputFifo;

	FTspectralEngine * _specEngine;

	bool _readyToDie;
	int _id;
};

#endif

