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


#if USING_FFTW3

#include <fftw3.h>

#else

#ifdef HAVE_SFFTW_H
#include <sfftw.h>
#else
#include <fftw.h>
#endif

#ifdef HAVE_SRFFTW_H
#include <srfftw.h>
#else
#include <rfftw.h>
#endif

#endif

#include <cstdio>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <algorithm>

#include "FTtypes.hpp"
#include "FTspectralEngine.hpp"
#include "FTprocessPath.hpp"
#include "RingBuffer.hpp"
#include "FTspectrumModifier.hpp"
#include "FTioSupport.hpp"
#include "FTupdateToken.hpp"
#include "FTprocI.hpp"
#include "FTmodulatorI.hpp"

using namespace PBD;
using namespace std;

const int FTspectralEngine::_windowStringCount = 4;
const char * FTspectralEngine::_windowStrings[] = {
	"Hanning", "Hamming", "Blackman", "Rectangle"
	};

const int FTspectralEngine::_fftSizeCount = 9;
const int FTspectralEngine::_fftSizes[] = {
	32, 64, 128, 256, 512, 1024, 2048, 4096, 8192
};


// in samples  (about 3 seconds at 44100)

#define FT_MAX_DELAYSAMPLES (1 << 19)


FTspectralEngine::FTspectralEngine()
	: _fftN (512), _windowing(FTspectralEngine::WINDOW_HANNING)
	, _oversamp(4), _averages(8), _fftnChanged(false)
	, _inputGain(1.0), _mixRatio(1.0), _bypassFlag(false), _mutedFlag(false), _updateSpeed(SPEED_MED)
	  , _id(0), _updateToken(0), _maxDelay(2.5)
	, _currInAvgIndex(0), _currOutAvgIndex(0), _avgReady(false)
{
	// one time allocations, why?  because mysterious crash occurs when
	// when reallocating them
	
	_inputPowerSpectra = new fft_data [FT_MAX_FFT_SIZE_HALF];
        _outputPowerSpectra = new fft_data [FT_MAX_FFT_SIZE_HALF];
	memset((char *) _inputPowerSpectra, 0, FT_MAX_FFT_SIZE_HALF*sizeof(fft_data));
	memset((char *) _outputPowerSpectra, 0, FT_MAX_FFT_SIZE_HALF*sizeof(fft_data));

	_runningInputPower = new fft_data [FT_MAX_FFT_SIZE_HALF];
	_runningOutputPower = new fft_data [FT_MAX_FFT_SIZE_HALF];
	memset((char *) _runningOutputPower, 0, FT_MAX_FFT_SIZE_HALF*sizeof(fft_data));
	memset((char *) _runningInputPower, 0, FT_MAX_FFT_SIZE_HALF*sizeof(fft_data));


	initState();
}

void FTspectralEngine::initState()
{
	_inwork = new fft_data [_fftN];
	_accum = new fft_data [2 * _fftN];

	memset((char *) _accum, 0, 2*_fftN*sizeof(fft_data));
	memset((char *) _inwork, 0, _fftN*sizeof(fft_data));

		
#if USING_FFTW3
	_outwork = (fft_data *) fftwf_malloc(sizeof(fft_data) * _fftN);
 	_winwork = (fft_data *) fftwf_malloc(sizeof(fft_data) * _fftN);

	_fftPlan  = fftwf_plan_r2r_1d(_fftN, _winwork, _outwork, FFTW_R2HC, FFTW_ESTIMATE);
	_ifftPlan = fftwf_plan_r2r_1d(_fftN, _outwork, _winwork, FFTW_HC2R, FFTW_ESTIMATE);
#else
	_outwork = new fft_data [_fftN];
	_winwork = new fft_data [_fftN];

	_fftPlan = rfftw_create_plan(_fftN, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);		
	_ifftPlan = rfftw_create_plan(_fftN, FFTW_COMPLEX_TO_REAL, FFTW_ESTIMATE);		
#endif
	_sampleRate = FTioSupport::instance()->getSampleRate();

	// window init
	createWindowVectors();


	_averages = (int) (_oversamp * _updateSpeed * 512/(float)_fftN); // magic?
	if (_averages == 0) _averages = 1;

	// reset averages
	_currInAvgIndex = 0;
	_currOutAvgIndex = 0;
	_avgReady = false;
	
	for (vector<FTprocI*>::iterator iter = _procModules.begin();
	     iter != _procModules.end(); ++iter)
	{
		(*iter)->reset();
	}
}

void FTspectralEngine::destroyState()
{

 	delete [] _inwork;
 	delete [] _accum;
	

	// destroy window vectors
	for(int i = 0; i < NUM_WINDOWS; i++)
	{
		delete [] _mWindows[i];
	}
	
#if USING_FFTW3
	
	fftwf_destroy_plan (_fftPlan);
	fftwf_destroy_plan (_ifftPlan);

	fftwf_free (_winwork);
	fftwf_free (_outwork);

#else

	rfftw_destroy_plan (_fftPlan);
	rfftw_destroy_plan (_ifftPlan);
	delete [] _outwork;
	delete [] _winwork;

#endif
}

FTspectralEngine::~FTspectralEngine()
{
	destroyState();

	delete [] _inputPowerSpectra;
        delete [] _outputPowerSpectra;
	delete [] _runningInputPower;
	delete [] _runningOutputPower;

	
	for (vector<FTprocI*>::iterator iter = _procModules.begin();
	     iter != _procModules.end(); ++iter)
	{
		delete (*iter);
	}
}


void FTspectralEngine::getProcessorModules (vector<FTprocI *> & modules)
{
	modules.clear();

	LockMonitor pmlock(_procmodLock, __LINE__, __FILE__);
	modules.insert (modules.begin(), _procModules.begin(), _procModules.end());
}

FTprocI * FTspectralEngine::getProcessorModule ( unsigned int num)
{
	LockMonitor (_procmodLock, __LINE__, __FILE__);
	
	if (num < _procModules.size()) {
		return _procModules[num];
	}

	return 0;
}

void FTspectralEngine::insertProcessorModule (FTprocI * procmod, unsigned int index)
{
	if (!procmod) return;

	LockMonitor pmlock(_procmodLock, __LINE__, __FILE__);
	
	vector<FTprocI*>::iterator iter = _procModules.begin();

	for (unsigned int n=0; n < index && iter!=_procModules.end(); ++n) {
		++iter;
	}

	procmod->setOversamp (_oversamp);
	procmod->setFFTsize (_fftN);
	procmod->setSampleRate (_sampleRate);
	
	_procModules.insert (iter, procmod);
}

void FTspectralEngine::appendProcessorModule (FTprocI * procmod)
{
	if (!procmod) return;

	LockMonitor (_procmodLock, __LINE__, __FILE__);
	
	procmod->setOversamp (_oversamp);
	procmod->setFFTsize (_fftN);
	procmod->setSampleRate (_sampleRate);
	
	_procModules.push_back (procmod);
}

void FTspectralEngine::moveProcessorModule (unsigned int from, unsigned int to)
{
	// both indexes refer to current positions within the list
	LockMonitor pmlock(_procmodLock, __LINE__, __FILE__);
	
	vector<FTprocI*>::iterator iter = _procModules.begin();

	for (unsigned int n=0; n < from && iter!=_procModules.end(); ++n) {
		++iter;
	}

	if (iter == _procModules.end())
		return;

	// remove from
	FTprocI * fproc = (*iter);
	_procModules.erase (iter);

	iter = _procModules.begin();
	
	if (to >= from) {
		// need to go one less
		for (unsigned int n=0; n < to && iter!=_procModules.end(); ++n) {
			++iter;
		}
	}
	else {
		for (unsigned int n=0; n < to && iter!=_procModules.end(); ++n) {
			++iter;
		}
	}

	_procModules.insert (iter, fproc);
	
}

void FTspectralEngine::removeProcessorModule (unsigned int index, bool destroy)
{
	LockMonitor pmlock(_procmodLock, __LINE__, __FILE__);
	
	if (index >= _procModules.size()) return;

	vector<FTprocI*>::iterator iter = _procModules.begin();

	for (unsigned int n=0; n < index && iter!=_procModules.end(); ++n) {
		++iter;
	}
	if (iter == _procModules.end())
		return;

	if (destroy) {
		FTprocI * proc = (*iter);
		delete proc;
	}
	
	_procModules.erase(iter);
}

void FTspectralEngine::clearProcessorModules (bool destroy)
{
	LockMonitor pmlock(_procmodLock, __LINE__, __FILE__);
	
	vector<FTprocI*>::iterator iter = _procModules.begin();

	if (destroy) {
		for (; iter != _procModules.end(); ++iter) {
			delete (*iter);
		}
	}

	_procModules.clear();
}

void FTspectralEngine::getModulators (vector<FTmodulatorI *> & modules)
{
	modules.clear();

	LockMonitor pmlock(_modulatorLock, __LINE__, __FILE__);
	modules.insert (modules.begin(), _modulators.begin(), _modulators.end());
}

FTmodulatorI * FTspectralEngine::getModulator ( unsigned int num)
{
	LockMonitor (_modulatorLock, __LINE__, __FILE__);
	
	if (num < _modulators.size()) {
		return _modulators[num];
	}

	return 0;
}

void FTspectralEngine::insertModulator (FTmodulatorI * procmod, unsigned int index)
{
	if (!procmod) return;

	{
		LockMonitor pmlock(_modulatorLock, __LINE__, __FILE__);
		
		vector<FTmodulatorI*>::iterator iter = _modulators.begin();
		
		for (unsigned int n=0; n < index && iter!=_modulators.end(); ++n) {
			++iter;
		}
		
		procmod->setFFTsize (_fftN);
		procmod->setSampleRate (_sampleRate);
		
		_modulators.insert (iter, procmod);
	}
	
	ModulatorAdded (procmod); // emit
}

void FTspectralEngine::appendModulator (FTmodulatorI * procmod)
{
	if (!procmod) return;

	{
		LockMonitor (_modulatorLock, __LINE__, __FILE__);
		
		procmod->setFFTsize (_fftN);
		procmod->setSampleRate (_sampleRate);
		
		_modulators.push_back (procmod);
	}
	
	ModulatorAdded (procmod); // emit

}

void FTspectralEngine::moveModulator (unsigned int from, unsigned int to)
{
	// both indexes refer to current positions within the list
	LockMonitor pmlock(_modulatorLock, __LINE__, __FILE__);
	
	vector<FTmodulatorI*>::iterator iter = _modulators.begin();

	for (unsigned int n=0; n < from && iter!=_modulators.end(); ++n) {
		++iter;
	}

	if (iter == _modulators.end())
		return;

	// remove from
	FTmodulatorI * fproc = (*iter);
	_modulators.erase (iter);

	iter = _modulators.begin();
	
	if (to >= from) {
		// need to go one less
		for (unsigned int n=0; n < to && iter!=_modulators.end(); ++n) {
			++iter;
		}
	}
	else {
		for (unsigned int n=0; n < to && iter!=_modulators.end(); ++n) {
			++iter;
		}
	}

	_modulators.insert (iter, fproc);
	
}

void FTspectralEngine::removeModulator (unsigned int index, bool destroy)
{
	LockMonitor pmlock(_modulatorLock, __LINE__, __FILE__);
	
	if (index >= _modulators.size()) return;

	vector<FTmodulatorI*>::iterator iter = _modulators.begin();

	for (unsigned int n=0; n < index && iter!=_modulators.end(); ++n) {
		++iter;
	}
	if (iter == _modulators.end())
		return;

	if (destroy) {
		FTmodulatorI * proc = (*iter);

		delete proc;
	}
	
	_modulators.erase(iter);
}

void FTspectralEngine::removeModulator (FTmodulatorI * procmod, bool destroy)
{
	bool candel = false;

	{
		LockMonitor pmlock(_modulatorLock, __LINE__, __FILE__);
		
		for (vector<FTmodulatorI*>::iterator iter = _modulators.begin();
		     iter != _modulators.end(); ++iter)
		{
			if (procmod == *iter) {
				
				_modulators.erase(iter);
				candel = true;
				break;
			}
		}
	}
	
	if (destroy && candel) {
		delete procmod;
	}
}


bool FTspectralEngine::hasModulator (FTmodulatorI * procmod)
{
	LockMonitor pmlock(_modulatorLock, __LINE__, __FILE__);

	return (find(_modulators.begin(), _modulators.end(), procmod) != _modulators.end());
}

void FTspectralEngine::clearModulators (bool destroy)
{
	LockMonitor pmlock(_modulatorLock, __LINE__, __FILE__);
	
	vector<FTmodulatorI*>::iterator iter = _modulators.begin();

	if (destroy) {
		for (; iter != _modulators.end(); ++iter) {
			delete (*iter);
		}
	}

	_modulators.clear();
}


void FTspectralEngine::setId (int id)
{
	LockMonitor pmlock(_procmodLock, __LINE__, __FILE__);
	_id = id;
	// set the id of all our filters

	for (vector<FTprocI*>::iterator iter = _procModules.begin();
	     iter != _procModules.end(); ++iter)
	{
		(*iter)->setId (id);
	}
}

void FTspectralEngine::setFFTsize (FTspectralEngine::FFT_Size sz)
{
	// THIS MUST NOT BE CALLED WHILE WE ARE ACTIVATED!
	//LockMonitor pmlock(_fftLock, __LINE__, __FILE__);
	
	if ((int) sz != _fftN)
	{
		_fftN = sz;

		// change these first
		for (vector<FTprocI*>::iterator iter = _procModules.begin();
		     iter != _procModules.end(); ++iter)
		{
			(*iter)->setFFTsize (_fftN);
		}

		destroyState();
		initState();
	}
}

void FTspectralEngine::setOversamp (int osamp)
{
	_oversamp = osamp;

	_averages = (int) (_oversamp * (float) _updateSpeed * 512/(float)_fftN); // magic?
	if (_averages == 0) _averages = 1;

	// reset averages
	memset(_runningOutputPower, 0, _fftN * sizeof(float));
	memset(_runningInputPower, 0, _fftN * sizeof(float));
	_currInAvgIndex = 0;
	_currOutAvgIndex = 0;
	_avgReady = false;

	LockMonitor pmlock(_procmodLock, __LINE__, __FILE__);
	
	// set it in all the modules
	for (vector<FTprocI*>::iterator iter = _procModules.begin();
	     iter != _procModules.end(); ++iter)
	{
		(*iter)->setOversamp (_oversamp);
	}
	
}

void FTspectralEngine::setUpdateSpeed (UpdateSpeed speed)
{
	_updateSpeed = speed;
	_averages = (int) (_oversamp * (float) _updateSpeed * 512/(float)_fftN); // magic?
	if (_averages == 0) _averages = 1;

	// reset averages
	memset(_runningOutputPower, 0, _fftN * sizeof(float));
	memset(_runningInputPower, 0, _fftN * sizeof(float));
	_currInAvgIndex = 0;
	_currOutAvgIndex = 0;
	_avgReady = false;

}

void FTspectralEngine::setMaxDelay(float secs)
{
	// THIS MUST NOT BE CALLED WHILE WE ARE ACTIVATED!
	
	if (secs <= 0.0) return;

	LockMonitor pmlock(_procmodLock, __LINE__, __FILE__);
	
	_maxDelay = secs;
	
	for (vector<FTprocI*>::iterator iter = _procModules.begin();
	     iter != _procModules.end(); ++iter)
	{
		(*iter)->setMaxDelay (secs);
	}
	
}


nframes_t FTspectralEngine::getLatency()
{
    	int step_size = _fftN / _oversamp;
        int latency = _fftN - step_size;

	return latency;
}

/**
 * Main FFT processing done here
 *  this is called from the i/o thread
 */
bool FTspectralEngine::processNow (FTprocessPath *procpath)
{
	int i;
	int osamp = _oversamp;
	int step_size = _fftN / osamp;
        int latency = _fftN - step_size;
	float * win = _mWindows[_windowing];

	
	nframes_t current_frame = FTioSupport::instance()->getTransportFrame();

	
	// do we have enough data for next frame (oversampled)?
	while (procpath->getInputFifo()->read_space() >= (step_size * sizeof(sample_t)))
	{
		//printf ("processing spectral sizeof sample = %d  fft_data = %d\n", sizeof(sample_t), sizeof(fftw_real));
		
		// copy data into fft work buf
		procpath->getInputFifo()->read ( (char *) (&_inwork[latency]), step_size * sizeof(sample_t) );

		//printf ("real data 1 = %g\n", _inwork[1]);
		
		// window data into winwork
		for(i = 0; i < _fftN; i++)
		{
			_winwork[i] = _inwork[i] * win[i] * _inputGain; 
		}

#if USING_FFTW3
		// do forward real FFT
		fftwf_execute(_fftPlan);
#else
		// do forward real FFT
		rfftw_one(_fftPlan, _winwork, _outwork);
#endif
		// compute running mag^2 buffer for input
		computeAverageInputPower (_outwork);


		// do modulation in order with each modulator
		{
			TentativeLockMonitor modlock(_modulatorLock, __LINE__, __FILE__);
			if (modlock.locked()) {
			
				for (vector<FTmodulatorI*>::iterator iter = _modulators.begin();
				     iter != _modulators.end(); ++iter)
				{
					(*iter)->modulate (current_frame, _outwork, _fftN, _inwork, _fftN);
				}
			}
		}
		
		// do processing in order with each processing module
		{
			TentativeLockMonitor pmlock(_procmodLock, __LINE__, __FILE__);
			if (pmlock.locked()) {
			
				for (vector<FTprocI*>::iterator iter = _procModules.begin();
				     iter != _procModules.end(); ++iter)
				{
					// do it in place
					(*iter)->process (_outwork,  _fftN);
				}
			}
		}
		
		// compute running mag^2 buffer for output
		computeAverageOutputPower (_outwork);

#if USING_FFTW3
		// do reverse FFT
		fftwf_execute(_ifftPlan);
#else
		// do reverse FFT
		rfftw_one(_ifftPlan, _outwork, _winwork);
#endif

		// the output is scaled by fftN, we need to normalize it and window it
		for ( i=0; i < _fftN; i++)
		{
			_accum[i] += _mixRatio * 4.0f * win[i] * _winwork[i] / ((float)_fftN * osamp);
		}

		// mix in dry only if necessary
		if (_mixRatio < 1.0) {
			float dry = 1.0 - _mixRatio;
			for (i=0; i < step_size; i++) {
				_accum[i] += dry * _inwork[i] ;
			}
		}
		
		// put step_size worth of the real data into the processPath out buffer
		procpath->getOutputFifo()->write( (char *)_accum, sizeof(sample_t) * step_size);		

		
		// shift output accumulator data
		memmove(_accum, _accum + step_size, _fftN*sizeof(sample_t));

		// shift input fifo (inwork)
		memmove(_inwork, _inwork + step_size, latency*sizeof(sample_t));
		
		
		

		// update events for those who listen
 		if (_avgReady && _updateToken) {
			_updateToken->setUpdated(true);
 			_avgReady = false;
 		}

		current_frame += step_size;
	}

	return true;
}




void FTspectralEngine::computeAverageInputPower (fft_data *fftbuf)
{
	int fftn2 = _fftN / 2;

	if (_averages > 1) {

		if (_currInAvgIndex > 0) {
			_inputPowerSpectra[0] += fftbuf[0] * fftbuf[0];
			for (int i=1; i < fftn2-1; i++)
			{
				_inputPowerSpectra[i] += (fftbuf[i] * fftbuf[i]) + (fftbuf[_fftN-i] * fftbuf[_fftN-i]);
			}	
		}
		else {
			_inputPowerSpectra[0] = fftbuf[0] * fftbuf[0];
		
			for (int i=1; i < fftn2-1 ; i++)
			{
				_inputPowerSpectra[i] = (fftbuf[i] * fftbuf[i]) + (fftbuf[_fftN-i] * fftbuf[_fftN-i]);
			}	
		}
		
		_currInAvgIndex = (_currInAvgIndex+1) % _averages;
		
		if (_currInAvgIndex == 0) {
			_runningInputPower[0] = _inputPowerSpectra[0] / _averages;
			for (int i=1; i < fftn2-1 ; i++)
			{
				_runningInputPower[i] = _inputPowerSpectra[i] / _averages;
			}
			_avgReady = true;
		}
	}
	else {
		// 1 average, minimize looping
		_runningInputPower[0] = (fftbuf[0] * fftbuf[0]);
		
		for (int i=1; i < fftn2-1 ; i++)
		{
			_runningInputPower[i] = (fftbuf[i] * fftbuf[i]) + (fftbuf[_fftN-i] * fftbuf[_fftN-i]);
		}		
		_avgReady = true;

	}
	
}

void FTspectralEngine::computeAverageOutputPower (fft_data *fftbuf)
{
	int fftn2 = (_fftN+1) / 2;

	if (_averages > 1) {

		if (_currOutAvgIndex > 0) {
		
			_outputPowerSpectra[0] += (fftbuf[0] * fftbuf[0]);
			for (int i=1; i < fftn2-1; i++)
			{
				_outputPowerSpectra[i] += (fftbuf[i] * fftbuf[i]) + (fftbuf[_fftN-i] * fftbuf[_fftN-i]);
			}
		}
		else {
			_outputPowerSpectra[0] = (fftbuf[0] * fftbuf[0]);
			for (int i=1; i < fftn2-1 ; i++)
			{
				_outputPowerSpectra[i] = (fftbuf[i] * fftbuf[i]) + (fftbuf[_fftN-i] * fftbuf[_fftN-i]);
			}
		}
		
		_currOutAvgIndex = (_currOutAvgIndex+1) % _averages;
		
		if (_currOutAvgIndex == 0) {
			_runningOutputPower[0] = _outputPowerSpectra[0] / _averages;
			for (int i=1; i < fftn2-1 ; i++)
			{
				_runningOutputPower[i] = _outputPowerSpectra[i] / _averages;
			}
			_avgReady = true;
		}
	}
	else {
		// 1 average, minimize looping
		_runningOutputPower[0] = (fftbuf[0] * fftbuf[0]);
		for (int i=1; i < fftn2-1 ; i++)
		{
			_runningOutputPower[i] = (fftbuf[i] * fftbuf[i]) + (fftbuf[_fftN-i] * fftbuf[_fftN-i]);
		}		
		_avgReady = true;
	}

	
}




void FTspectralEngine::createWindowVectors (bool noalloc)
{
    ///////////////////////////////////////////////////////////////////////////
    int i;
    ///////////////////////////////////////////////////////////////////////////

    if (!noalloc)
    {
	    ///////////////////////////////////////////////////////////////////////////
	    // create window array
	    _mWindows = new float*[NUM_WINDOWS];
	    
	    ///////////////////////////////////////////////////////////////////////////
	    // allocate vectors
	    for(i = 0; i < NUM_WINDOWS; i++)
	    {
		    _mWindows[i] = new float[_fftN];
	    }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // create windows
    createRectangleWindow ();
    createHanningWindow ();
    createHammingWindow ();
    createBlackmanWindow ();
}

void FTspectralEngine::createRectangleWindow ()
{
    ///////////////////////////////////////////////////////////////////////////
    int i;
    ///////////////////////////////////////////////////////////////////////////
    
    for(i = 0; i < _fftN; i++)
    {
	_mWindows[WINDOW_RECTANGLE][i] = 0.5;
    }
}


void FTspectralEngine::createHanningWindow ()
{
   ///////////////////////////////////////////////////////////////////////////
   int i;
   ///////////////////////////////////////////////////////////////////////////
   
   for(i = 0; i < _fftN; i++)
   {
	   _mWindows[WINDOW_HANNING][i] = 0.81 * ( // fudge factor
		   0.5 - 
		   (0.5 * 
		    //(float) cos(2.0 * M_PI * i / (_fftN - 1.0)));
		    (float) cos(2.0 * M_PI * i / (_fftN))));
    }    
}

void FTspectralEngine::createHammingWindow ()
{
   ///////////////////////////////////////////////////////////////////////////
   int i;
   ///////////////////////////////////////////////////////////////////////////
   
   for(i = 0; i < _fftN; i++)
    {
	    _mWindows[WINDOW_HAMMING][i] = 0.82 * ( // fudge factor
		    0.54 - 
		    (0.46 * 
		     (float) cos(2.0 * M_PI * i / (_fftN - 1.0))));
    }   
}


void FTspectralEngine::createBlackmanWindow ()
{
    ///////////////////////////////////////////////////////////////////////////
    int i;
    ///////////////////////////////////////////////////////////////////////////
    
    for(i = 0; i < _fftN; i++)
    {
	    _mWindows[WINDOW_BLACKMAN][i] = 0.9 * ( // fudge factor
		    0.42 - 
		    (0.50 * (float) cos(
			    2.0 * M_PI * i /(_fftN - 1.0))) + 
		    (0.08 * (float) cos(
			    4.0 * M_PI * i /(_fftN - 1.0))));
    }
}





