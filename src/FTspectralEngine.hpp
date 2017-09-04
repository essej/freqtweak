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

#ifndef __FTSPECTRALENGINE_HPP__
#define __FTSPECTRALENGINE_HPP__

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if USING_FFTW3

#include <fftw3.h>

#else

#ifdef HAVE_SRFFTW_H
#include <srfftw.h>
#else
#include <rfftw.h>
#endif

#endif

#include "FTutils.hpp"
#include "FTtypes.hpp"
#include "LockMonitor.hpp"

#include <sigc++/sigc++.h>


#include <vector>
using namespace std;


class FTprocessPath;
class RingBuffer;
class FTspectrumModifier;
class FTupdateToken;
class FTmodulatorI;
class FTprocI;

class FTspectralEngine

{
  public: 
	FTspectralEngine();
	virtual ~FTspectralEngine();
	
	bool processNow (FTprocessPath *procpath);

	enum FFT_Size
	{
		FFT_32 = 32,
		FFT_64 = 64,
		FFT_128 = 128,
		FFT_256 = 256,
		FFT_512 = 512,
		FFT_1024 = 1024,
		FFT_2048 = 2048,
		FFT_4096 = 4096,
		FFT_8192 = 8192

	};

	enum Windowing
	{
		WINDOW_HANNING = 0,
		WINDOW_HAMMING,
		WINDOW_BLACKMAN,
		WINDOW_RECTANGLE
	};

	enum UpdateSpeed
	{
		SPEED_FAST = 1,
		SPEED_MED = 2,
		SPEED_SLOW = 4,
		SPEED_TURTLE = 8,
	};
	
	static const int  NUM_WINDOWS  = 5;
	

	void setId (int id);
	int getId () { return _id; }

	void setUpdateToken (FTupdateToken *tok) { _updateToken = tok; }
	FTupdateToken * getUpdateToken() { return _updateToken; }
	
	void setFFTsize (FFT_Size sz); // ONLY call when not processing
	FFT_Size getFFTsize() { return (FFT_Size) _fftN; }

	void setSampleRate (nframes_t rate) { _sampleRate = rate; }
	nframes_t getSampleRate() { return _sampleRate; }
	
	void setWindowing (Windowing w) { _windowing = w; }
	Windowing getWindowing() { return _windowing; }

	void setAverages (int avg) { if (avg > _maxAverages) _averages = _maxAverages; _averages = avg; }
	int getAverages () { return _averages; }

	void setOversamp (int osamp);
	int getOversamp () { return _oversamp; }

	void setUpdateSpeed (UpdateSpeed speed);
	UpdateSpeed getUpdateSpeed () { return _updateSpeed; }

	void setInputGain (float gain) { _inputGain = gain; }
	float getInputGain () { return _inputGain; }

	// 1.0 is fully wet
	void setMixRatio (float ratio) { _mixRatio = ratio; }
	float getMixRatio () { return _mixRatio;}

	void setBypassed (bool flag) { _bypassFlag = flag; }
	bool getBypassed () { return _bypassFlag; }

	void setMuted (bool flag) { _mutedFlag = flag; }
	bool getMuted () { return _mutedFlag; }
	
	
	float getMaxDelay () { return _maxDelay; }
	void setMaxDelay (float secs); // ONLY call when not processing
	
	void setTempo (int tempo) { _tempo = tempo; }
        int getTempo() { return _tempo; }
	
	const float * getRunningInputPower() { return _runningInputPower; }
	const float * getRunningOutputPower() { return _runningOutputPower; }

	nframes_t getLatency();


	// processor module handling

	void insertProcessorModule (FTprocI * procmod, unsigned int index);
	void appendProcessorModule (FTprocI * procmod);
	void moveProcessorModule (unsigned int from, unsigned int to);
	void removeProcessorModule (unsigned int index, bool destroy=true);
	void clearProcessorModules (bool destroy=true);
	void getProcessorModules (vector<FTprocI *> & modules);
	FTprocI * getProcessorModule ( unsigned int num);

	// modulator handling
	void insertModulator (FTmodulatorI * procmod, unsigned int index);
	void appendModulator (FTmodulatorI * procmod);
	void moveModulator (unsigned int from, unsigned int to);
	void removeModulator (unsigned int index, bool destroy=true);
	void removeModulator (FTmodulatorI * procmod, bool destroy=true);
	void clearModulators (bool destroy=true);
	void getModulators (vector<FTmodulatorI *> & modules);
	FTmodulatorI * getModulator ( unsigned int num);
	bool hasModulator (FTmodulatorI * procmod);

	SigC::Signal1<void, FTmodulatorI *> ModulatorAdded;


	
	static const char ** getWindowStrings() { return (const char **) _windowStrings; }
	static const int getWindowStringsCount() { return _windowStringCount; }
	static const int * getFFTSizes() { return (const int *) _fftSizes; }
	static const int getFFTSizeCount() { return _fftSizeCount; }


	
protected:

	
	void computeAverageInputPower (fft_data *fftbuf);
	void computeAverageOutputPower (fft_data *fftbuf);
	
	void createWindowVectors(bool noalloc=false);   
	void createRaisedCosineWindow();
	void createRectangleWindow();    
	void createHanningWindow();
	void createHammingWindow();
	void createBlackmanWindow();

	void initState();
	void destroyState();
	
	static const int _windowStringCount;
	static const char * _windowStrings[];
	static const int _fftSizeCount;
	static const int  _fftSizes[];

	// the processing modules
	vector<FTprocI *> _procModules;
	PBD::NonBlockingLock _procmodLock;

	// the modulators
	vector<FTmodulatorI *> _modulators;
	PBD::NonBlockingLock _modulatorLock;

	
	// fft size (thus frame length)
        int _fftN;
	Windowing _windowing;
	int _oversamp;
	int _maxAverages;
	int _averages;

#ifdef USING_FFTW3
	fftwf_plan _fftPlan;
	fftwf_plan _ifftPlan;
#else
	rfftw_plan _fftPlan; // forward fft
	rfftw_plan _ifftPlan; // inverse fft
#endif

	int _newfftN;
	bool _fftnChanged;

	PBD::NonBlockingLock _fftLock;
	
	// space for average input power buffer
	// elements = _fftN/2 * MAX_AVERAGES * MAX_OVERSAMP 
	fft_data * _inputPowerSpectra;
	fft_data * _outputPowerSpectra;

	// the current running avg power
	// elements = _fftN/2
	fft_data * _runningInputPower;
	fft_data * _runningOutputPower;

	
	nframes_t _sampleRate;
	
	float _inputGain;
	float _mixRatio;
	bool _bypassFlag;
	bool _mutedFlag;

	UpdateSpeed _updateSpeed;
	int _id;
	FTupdateToken * _updateToken;

	int _tempo;
	float _maxDelay;
private:
	
	fft_data *_inwork, *_outwork;
	fft_data *_winwork;
	fft_data *_accum;
	fft_data *_scaletemp;
	
	// for windowing
	float ** _mWindows;
	

	// for averaging

	int _currInAvgIndex;
	int _currOutAvgIndex;

	bool _avgReady;

	
};





#endif
