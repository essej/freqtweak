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

#include "FTprocCompressor.hpp"
#include "FTutils.hpp"
#include <cmath>
using namespace std;

#include <stdlib.h>


/**
 * the following taken from SWH plugins utility functions
 * we'll wrap it up later
 */

#define A_TBL 256


static inline float rms_env_process(rms_env *r, float x);

inline static float rms_env_process(rms_env *r, const float x)
{
	r->sum -= r->buffer[r->pos];
	r->sum += x;
	r->buffer[r->pos] = x;
	r->pos = (r->pos + 1) & (RMS_BUF_SIZE - 1);

	return FTutils::fast_square_root(r->sum / (float)RMS_BUF_SIZE);
}



static void rms_env_reset(rms_env *r)
{
	unsigned int i;

	for (i=0; i<RMS_BUF_SIZE; i++) {
		r->buffer[i] = 0.0f;
	}
	r->pos = 0;
	r->sum = 0.0f;
}

static rms_env *rms_env_new()
{
	rms_env *nenv = new rms_env;
	rms_env_reset(nenv);
	
	return nenv;
}

static void rms_env_free(rms_env *r)
{
	delete r;
}

static inline int f_round(float f) {
        f += (3<<22);
        return *((int*)&f) - 0x4b400000;
}



FTprocCompressor::FTprocCompressor (nframes_t samprate, unsigned int fftn)
	: FTprocI("Compressor", samprate, fftn)
	  , _dbAdjust(12.0)
	  
{
	_confname = "Compressor";
}

FTprocCompressor::FTprocCompressor (const FTprocCompressor & other)
	: FTprocI (other._name, other._sampleRate, other._fftN)	
	  , _dbAdjust(12.0)
{
	_confname = "Compressor";
}

void FTprocCompressor::initialize()
{
	// create filter

	_thresh_filter = new FTspectrumModifier("Comp Thresh", "compressor_thresh", 0, FTspectrumModifier::DB_MODIFIER, COMPRESS_SPECMOD, _fftN/2, 0.0);
	_thresh_filter->setRange(-60.0, 0.0);
	_filterlist.push_back (_thresh_filter);

	_ratio_filter = new FTspectrumModifier("Comp Ratio", "compressor_ratio", 1, FTspectrumModifier::RATIO_MODIFIER, COMPRESS_SPECMOD, _fftN/2, 1.0);
	_ratio_filter->setRange(1.0, 20.0);
	_filterlist.push_back (_ratio_filter);

	_release_filter = new FTspectrumModifier("Comp A/R", "compressor_release", 2, FTspectrumModifier::TIME_MODIFIER, COMPRESS_SPECMOD, _fftN/2, 0.2);
	_release_filter->setRange(0.0, 1.0);
	_filterlist.push_back (_release_filter);

	_attack_filter = new FTspectrumModifier("Comp A/R", "compressor_attack", 2, FTspectrumModifier::TIME_MODIFIER, COMPRESS_SPECMOD, _fftN/2, 0.1);
	_attack_filter->setRange(0.0, 1.0);
	_filterlist.push_back (_attack_filter);


	_makeup_filter = new FTspectrumModifier("Comp Makeup", "compressor_makeup", 3, FTspectrumModifier::DB_MODIFIER, COMPRESS_SPECMOD, _fftN/2, 0.0);
	_makeup_filter->setRange(0.0, 32.0);
	_filterlist.push_back (_makeup_filter);

	// state
	unsigned int nbins = _fftN >> 1;

	_rms = new rms_env*[nbins];
	_sum = new float[nbins];
	_amp = new float[nbins];
	_gain = new float[nbins];
	_gain_t = new float[nbins];
	_env = new float[nbins];
	_count = new unsigned int[nbins];

	for (unsigned int n=0; n < nbins; ++n)
	{
		_rms[n] = rms_env_new();
	}

	memset(_sum, 0, nbins * sizeof(float));
	memset(_amp, 0, nbins * sizeof(float));
	memset(_gain, 0, nbins * sizeof(float));
	memset(_gain_t, 0, nbins * sizeof(float));
	memset(_env, 0, nbins * sizeof(float));
	memset(_count, 0, nbins * sizeof(unsigned int));
	
	_as = new float[A_TBL];
	_as[0] = 1.0f;
	for (unsigned int i=1; i<A_TBL; i++) {
		_as[i] = expf(-1.0f / ((_sampleRate/(_oversamp*(float)_fftN)) * (float)i / (float)A_TBL));
	}
	
	_inited = true;
}

void FTprocCompressor::setOversamp (int osamp)
{
	FTprocI::setOversamp(osamp);
	
	for (unsigned int i=1; i<A_TBL; i++) {
		_as[i] = expf(-1.0f / ((_sampleRate/(_oversamp*(float)_fftN)) * (float)i / (float)A_TBL));
	}
}


void FTprocCompressor::setFFTsize (unsigned int fftn)
{
	// This must be called when we are not active, don't worry
	unsigned int orignbins = _fftN >> 1;
	
	FTprocI::setFFTsize(fftn);

	// reallocate all state arrays

	for (unsigned int n=0; n < orignbins; ++n)
	{
		rms_env_free(_rms[n]);
	}

	delete [] _rms;
	delete [] _sum;
	delete [] _amp;	
	delete [] _gain;
	delete [] _gain_t;
	delete [] _env;
	delete [] _count;

	
	unsigned int nbins = _fftN >> 1;

	_rms = new rms_env*[nbins];
	_sum = new float[nbins];
	_amp = new float[nbins];
	_gain = new float[nbins];
	_gain_t = new float[nbins];
	_env = new float[nbins];
	_count = new unsigned int [nbins];

	for (unsigned int n=0; n < nbins; ++n)
	{
		_rms[n] = rms_env_new();
	}

	memset(_sum, 0, nbins * sizeof(float));
	memset(_amp, 0, nbins * sizeof(float));
	memset(_gain, 0, nbins * sizeof(float));
	memset(_gain_t, 0, nbins * sizeof(float));
	memset(_env, 0, nbins * sizeof(float));
	memset(_count, 0, nbins * sizeof(unsigned int));

	_as[0] = 1.0f;
	for (unsigned int i=1; i<A_TBL; i++) {
		_as[i] = expf(-1.0f / ((_sampleRate/((float)4*_fftN)) * (float)i / (float)A_TBL));
	}

}

FTprocCompressor::~FTprocCompressor()
{
	if (!_inited) return;

        _filterlist.clear();
	delete _thresh_filter;
	delete _ratio_filter;
	delete _attack_filter;
	delete _release_filter;
	delete _makeup_filter;
}

void FTprocCompressor::process (fft_data *data, unsigned int fftn)
{
	if (!_inited || _thresh_filter->getBypassed()) {
		return;
	}
	
	float *threshold = _thresh_filter->getValues();
	float *ratio = _ratio_filter->getValues();
	float *attack = _attack_filter->getValues();
	float *release = _release_filter->getValues();
	float *makeup = _makeup_filter->getValues();
	
	const float knee = 5.0;
	float ga;
	float gr;
	float rs;
	float mug;
	float knee_min;
	float knee_max;
	float ef_a;
	float ef_ai;

	float thresh;
	float rat;
	float att, rel;
	
	int fftN2 = (fftn+1) >> 1;
	
	for (int i = 0; i < fftN2-1; i++)
	{
// 		if (filter[i] > max) filt = max;
// 		else if (filter[i] < min) filt = min;
// 		else filt = filter[i];

// 		power = (data[i] * data[i]) + (data[fftn-i] * data[fftn-i]);
// 		db = FTutils::powerLogScale (power, 0.0000000) + _dbAdjust; // total fudge factors

		thresh = LIMIT(threshold[i], -60.0f, 0.0f) + _dbAdjust;
		rat = LIMIT(ratio[i], 1.0f, 80.0f);
		att = LIMIT(attack[i], 0.002f, 1.0f); 
		rel = LIMIT(release[i], att, 1.0f);
		
		ga = _as[f_round(att  * (float)(A_TBL-1))];
		gr = _as[f_round(rel * (float)(A_TBL-1))];
		rs = (rat - 1.0f) / rat;
		mug = db2lin(LIMIT(makeup[i], 0.0f, 32.0f));
		knee_min = db2lin(thresh - knee);
		knee_max = db2lin(thresh + knee);
		ef_a = ga * 0.25f;
		ef_ai = 1.0f - ef_a;
		
		//_sum[i] += FTutils::fast_square_root((data[i] * data[i]) + (data[fftn-i] * data[fftn-i]));
		if (i == 0) {
			_sum[i] += (data[i] * data[i]);
		}
		else {
			_sum[i] += (data[i] * data[i]) + (data[fftn-i] * data[fftn-i]);
		}

// 		_sum[i] = FLUSH_TO_ZERO(_sum[i]);
// 		_env[i] = FLUSH_TO_ZERO(_env[i]);
// 		_amp[i] = FLUSH_TO_ZERO(_amp[i]);
// 		_rms[i]->sum = FLUSH_TO_ZERO(_rms[i]->sum);
//		_gain[i] = FLUSH_TO_ZERO(_gain[i]);
		
		if (_amp[i] > _env[i]) {
			_env[i] = _env[i] * ga + _amp[i] * (1.0f - ga);
		}
		else {
			_env[i] = _env[i] * gr + _amp[i] * (1.0f - gr);
		}
		if (_count[i]++ % 4 == 3)
		{
			_amp[i] = rms_env_process(_rms[i], _sum[i] * 0.25f);
			_sum[i] = 0.0f;
			if (_env[i] <= knee_min) {
				_gain_t[i] = 1.0f;
			} else if (_env[i] < knee_max) {
				const float x = -(thresh - knee - lin2db(_env[i])) / knee;
				_gain_t[i] = db2lin(-knee * rs * x * x * 0.25f);
			} else {
				_gain_t[i] = db2lin((thresh - lin2db(_env[i])) * rs);
			}
		}

		_gain[i] = _gain[i] * ef_a + _gain_t[i] * ef_ai;
		
		data[i] *=  _gain[i] * mug;
		if (i > 0) {
			data[fftn-i] *=  _gain[i] * mug;
		}

	}
}
