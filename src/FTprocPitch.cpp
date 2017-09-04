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

#include <math.h>

#include "FTprocPitch.hpp"
#include "FTutils.hpp"

FTprocPitch::FTprocPitch (nframes_t samprate, unsigned int fftn)
	: FTprocI("Pitch", samprate, fftn)
{
	_confname = "Pitch";
}

FTprocPitch::FTprocPitch (const FTprocPitch & other)
	: FTprocI (other._name, other._sampleRate, other._fftN)
{
	_confname = "Pitch";
}

void FTprocPitch::initialize()
{
	// create filter
	_filter = new FTspectrumModifier("Pitch", "scale", 0, FTspectrumModifier::SEMITONE_MODIFIER, SCALE_SPECMOD, _fftN/2, 1.0);
 	_filter->setRange(0.5, 2.0);
	_filter->setBypassed(true); // by default

	_filterlist.push_back (_filter);


	gLastPhase = new float [FT_MAX_FFT_SIZE];
	gSumPhase = new float [FT_MAX_FFT_SIZE];
	gAnaFreq = new float [FT_MAX_FFT_SIZE];
	gSynFreq = new float [FT_MAX_FFT_SIZE];
	gAnaMagn = new float [FT_MAX_FFT_SIZE];
	gSynMagn = new float [FT_MAX_FFT_SIZE];

	memset(gLastPhase, 0, FT_MAX_FFT_SIZE*sizeof(float));
	memset(gSumPhase, 0, FT_MAX_FFT_SIZE*sizeof(float));
	memset(gAnaFreq, 0, FT_MAX_FFT_SIZE*sizeof(float));
	memset(gSynFreq, 0, FT_MAX_FFT_SIZE*sizeof(float));
	memset(gAnaMagn, 0, FT_MAX_FFT_SIZE*sizeof(float));
	memset(gSynMagn, 0, FT_MAX_FFT_SIZE*sizeof(float));
	
	_inited = true;
}

FTprocPitch::~FTprocPitch()
{
	if (!_inited) return;

	delete [] gLastPhase;
	delete [] gSumPhase;
	delete [] gAnaFreq;
	delete [] gSynFreq;
	delete [] gAnaMagn;
	delete [] gSynMagn;

        _filterlist.clear();
	delete _filter;
}

void FTprocPitch::process (fft_data *data, unsigned int fftn)
{
 	if (!_inited || _filter->getBypassed()) {
 		return;
 	}

	float *filter = _filter->getValues();

	double magn, phase, tmp, real, imag;
	double freqPerBin, expct;
	long k, qpd, index, stepSize;
	int fftFrameSize2 = fftn / 2;
	int fftFrameLength = fftn;

	float min = _filter->getMin();
	float max = _filter->getMax();
	float filt;

	int osamp = _oversamp;
	
	stepSize = fftFrameLength/osamp;
	freqPerBin = _sampleRate*2.0/(double)fftFrameLength;
	expct = 2.0*M_PI*(double)stepSize/(double)fftFrameLength;

	/* this is the analysis step */
	for (k = 1; k < fftFrameSize2-1; k++) {
		
		real = data[k];
		imag = data[fftFrameLength - k];
		
		/* compute magnitude and phase */
		magn = sqrt(real*real + imag*imag);
		phase = atan2(imag,real);
		
		/* compute phase difference */
		tmp = phase - gLastPhase[k];
		gLastPhase[k] = phase;
		
		/* subtract expected phase difference */
		tmp -= (double)k*expct;
		
		/* map delta phase into +/- Pi interval */
		qpd = (long) (tmp/M_PI);
		if (qpd >= 0) qpd += qpd&1;
		else qpd -= qpd&1;
		tmp -= M_PI*(double)qpd;
		
		/* get deviation from bin frequency from the +/- Pi interval */
		tmp = osamp*tmp/(2.0f*M_PI);
		
		/* compute the k-th partials' true frequency */
		tmp = (double)k*freqPerBin + tmp*freqPerBin;
		
		/* store magnitude and true frequency in analysis arrays */
		gAnaMagn[k] = magn;
		gAnaFreq[k] = tmp;
		
	}
	
	/* ***************** PROCESSING ******************* */
	/* this does the actual pitch scaling */
	memset(gSynMagn, 0, fftFrameLength*sizeof(float));
	memset(gSynFreq, 0, fftFrameLength*sizeof(float));
	for (k = 0; k <= fftFrameSize2; k++)
	{
		filt = FTutils::f_clamp (filter[k], min, max);

		index = (long) (k/filt);
		if (index <= fftFrameSize2) {
			/* new bin overrides existing if magnitude is higher */ 

			if (gAnaMagn[index] > gSynMagn[k]) {
				gSynMagn[k] = gAnaMagn[index];
				gSynFreq[k] = gAnaFreq[index] * filt;
			}
			
			/* fill empty bins with nearest neighbour */
			
			if ((gSynFreq[k] == 0.) && (k > 0)) {
				gSynFreq[k] = gSynFreq[k-1];
				gSynMagn[k] = gSynMagn[k-1];
			}
		}
	}
	
	
	/* ***************** SYNTHESIS ******************* */
	/* this is the synthesis step */
	for (k = 1; k < fftFrameSize2-1; k++) {
		
		/* get magnitude and true frequency from synthesis arrays */
		magn = gSynMagn[k];
		tmp = gSynFreq[k];
		
		/* subtract bin mid frequency */
		tmp -= (double)k*freqPerBin;

		/* get bin deviation from freq deviation */
		tmp /= freqPerBin;
		
		/* take osamp into account */
		tmp = 2.*M_PI*tmp/osamp;
		
		/* add the overlap phase advance back in */
		tmp += (double)k*expct;
		
		/* accumulate delta phase to get bin phase */
		gSumPhase[k] += tmp;
		phase = gSumPhase[k];
		
		data[k] = magn*cos(phase);
		data[fftFrameLength - k] = magn*sin(phase);
	} 
	
}
