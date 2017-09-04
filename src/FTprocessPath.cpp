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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "FTprocessPath.hpp"
#include "FTdspManager.hpp"
#include "FTspectralEngine.hpp"
#include "FTprocI.hpp"
#include "RingBuffer.hpp"

FTprocessPath::FTprocessPath()
	: _maxBufsize(16384), _sampleRate(44100), _specEngine(0), _readyToDie(false), _id(0)
{
	// construct lockfree ringbufer
	_inputFifo = new RingBuffer(sizeof(sample_t) * FT_FIFOLENGTH);
	_outputFifo = new RingBuffer(sizeof(sample_t) * FT_FIFOLENGTH);

	initSpectralEngine();
}

FTprocessPath::~FTprocessPath()
{
	printf ("$#$#$#$#$ processpath \n");
	delete _inputFifo;
	delete _outputFifo;
	if (_specEngine) delete _specEngine;
}

void FTprocessPath::initSpectralEngine()
{
	_specEngine = new FTspectralEngine();
	
	// load all dsp modules from dsp manager and put them in
	FTdspManager::ModuleList mlist;
	FTdspManager::instance()->getAvailableModules (mlist);
	FTdspManager::ModuleList::iterator mod = mlist.begin();
	
	for (; mod != mlist.end(); ++mod)
	{
		if (!(*mod)->useAsDefault())
			continue;
	        FTprocI * newmod = (*mod)->clone();

		newmod->initialize();
		_specEngine->appendProcessorModule (newmod);
	}

}


void FTprocessPath::setId (int id)
{
	_id = id;
	if (_specEngine) _specEngine->setId (id);
}

/**
 * This will get called from the jack thread
 */ 

void FTprocessPath::processData (sample_t * inbuf, sample_t *outbuf, nframes_t nframes)
{
	bool good;
	
	if (_specEngine->getBypassed())
	{
		if (_specEngine->getMuted()) {
			memset (outbuf, 0, sizeof(sample_t) * nframes);
		}
		else if (inbuf != outbuf) {
			memcpy (outbuf, inbuf, sizeof(sample_t) * nframes);
		}
	}
	else
	{		
		// copy data from inbuf to the  lock free fifo at write pointer
		if (_inputFifo->write_space() >= (nframes * sizeof(sample_t)))
		{
			_inputFifo->write ((char *) inbuf, sizeof(sample_t) * nframes);	
		}
		else {
			//fprintf(stderr, "BLAH! Can't write into input fifo!\n");
		}
		
		// DO SPECTRAL PROCESSING
		good = _specEngine->processNow (this);
		
		
		// copy data from fifo at read pointer into outbuf
		if (good && _outputFifo->read_space() >= (nframes * sizeof(sample_t)))
		{
			_outputFifo->read ((char *) outbuf, sizeof(sample_t) * nframes);	

			if (_specEngine->getMuted()) {
				memset (outbuf, 0, sizeof(sample_t) * nframes);
			}
		}
		else {
			//fprintf(stderr, "BLAH! Can't read enough data from output fifo!\n");
			if (_specEngine->getMuted()) {
				memset (outbuf, 0, sizeof(sample_t) * nframes);
			}
			else {
				memcpy (outbuf, inbuf, sizeof(sample_t) * nframes);
			}
		}
	}
}

