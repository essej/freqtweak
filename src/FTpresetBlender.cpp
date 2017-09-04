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

#include "FTpresetBlender.hpp"
#include "FTioSupport.hpp"
#include "FTprocI.hpp"
#include "FTprocessPath.hpp"
#include "FTconfigManager.hpp"
#include "FTspectralEngine.hpp"
#include "FTspectrumModifier.hpp"

FTpresetBlender::FTpresetBlender(FTconfigManager * confman)
	: _configMan(confman)
{
	// add two elements both intially null
	_presetList.push_back(0);
	_presetList.push_back(0);

	_presetNames.push_back("");
	_presetNames.push_back("");
}

FTpresetBlender::~FTpresetBlender()
{
	for (unsigned int n=0; n < _presetList.size(); ++n) {

		if (_presetList[n]) {
			delete _presetList[n];
		}
	}

	_presetList.clear();
	
}
	

bool FTpresetBlender::setPreset(const string & name, int index)
{
	// try to load it up and compare the resulting FTprocs to the
	// currently active ones

	if (index >= (int) _presetList.size()) {
		return false;
	}

	// delete old one no matter what
	if (_presetList[index]) {
		delete _presetList[index];
		_presetList[index] = 0;
	}
	
	vector<vector <FTprocI *> > * procvec = new vector<vector<FTprocI*> > ();
		
	bool succ =  _configMan->loadSettings (name.c_str(), false, true, *procvec, false);

	if (!succ) {
		delete procvec;
		return false;
	}
	
	FTioSupport * iosup = FTioSupport::instance();

	if ((int)procvec->size() != iosup->getActivePathCount()) {
		delete procvec;
		return false;
	}
	
	for ( unsigned int i=0; i < procvec->size(); i++)
	{
		FTprocessPath * procpath = iosup->getProcessPath(i);
		if (!procpath) {
			delete procvec;
			return false; // shouldnt happen

		}
		vector<FTprocI*> & pvec = (*procvec)[i];

		FTspectralEngine * engine = procpath->getSpectralEngine();
		vector<FTprocI *> procmods;
		engine->getProcessorModules (procmods);
		
		for (unsigned int n=0; n < procmods.size(); ++n)
		{
			FTprocI *pm = procmods[n];

			// compare the proctype

			if (pvec.size() <= n ||  pm->getName() != pvec[n]->getName()) {
				fprintf (stderr, "mismatch at %d %d: %s   %u\n", i, n, pm->getName().c_str(), pvec.size());
				delete procvec;
				return false;
			}
		}
	}

	// if we got here we are a match

	_presetList[index]  = procvec;
	_presetNames[index] = name;
	
	return true;
}

string FTpresetBlender::getPreset(int index)
{
	if (index < (int) _presetNames.size()) {
		return _presetNames[index];
	}

	return "";
}

bool FTpresetBlender::setBlend (unsigned int specmod_n, unsigned int filt_n, float val)
{
	// for the given filter given by specmod_n, filt_n, set the ratio to use
	// and adjust the active filters accordingly
	// TODO: support more than 2 presets

	// Assume that both proc lists have the same structure

	if (!_presetList[0] || !_presetList[1]) {
		return false;
	}

	FTioSupport * iosup = FTioSupport::instance();
	
	vector<vector <FTprocI *> > * procvec0 = _presetList[0];
	vector<vector <FTprocI *> > * procvec1 = _presetList[1];

	for (unsigned int chan=0; chan < procvec0->size(); ++chan)
	{
		vector<FTprocI*> & procvec0_p = (*procvec0)[chan];
		vector<FTprocI*> & procvec1_p = (*procvec1)[chan];

		FTprocessPath * procpath = iosup->getProcessPath((int) chan);
		if (!procpath) continue; // shouldnt happen
		
		FTspectralEngine *engine = procpath->getSpectralEngine();
		vector<FTprocI *> procmods;
		engine->getProcessorModules (procmods);

		
		if (specmod_n < procvec0_p.size() && specmod_n < procvec1_p.size()
			&& specmod_n < procmods.size())
		{
			FTspectrumModifier * targFilt = procmods[specmod_n]->getFilter(filt_n);
			FTspectrumModifier * filt0 = procvec0_p[specmod_n]->getFilter(filt_n);
			FTspectrumModifier * filt1 = procvec1_p[specmod_n]->getFilter(filt_n);

			if (targFilt && filt0 && filt1)
			{
				// do the blend
				blendFilters (targFilt, filt0, filt1, val);
			}
		}
	}
	

	
	return true;
}

float FTpresetBlender::getBlend (unsigned int specmod_n, unsigned int filt_n)
{

	return 0.0;
}

void FTpresetBlender::blendFilters (FTspectrumModifier * targFilt, FTspectrumModifier *filt0, FTspectrumModifier *filt1, float val)
{

	float * targvals = targFilt->getValues();
	float * filt0vals = filt0->getValues();
	float * filt1vals = filt1->getValues();


	for (int i=0; i < targFilt->getLength(); ++i)
	{
		targvals[i] = val*filt0vals[i] + (1.0-val)*filt1vals[i];
	}
}

