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
#include <stdint.h>

#include <algorithm>
using namespace std;

#include "FTspectrumModifier.hpp"
#include "FTtypes.hpp"




FTspectrumModifier::FTspectrumModifier(const string &name, const string &configName, int group,
				       FTspectrumModifier::ModifierType mtype, SpecModType smtype, int length, float initval)
	:  _modType(mtype), _specmodType(smtype), _name(name), _configName(configName), _group(group),
	   _values(0), _length(length), _linkedTo(0), _initval(initval),
	   _id(0), _bypassed(false), _dirty(false), _extra_node(0)

{
	_values = new float[FT_MAX_FFT_SIZE/2];
	_tmpvalues = new float[FT_MAX_FFT_SIZE/2];

	for (int i=0; i < FT_MAX_FFT_SIZE/2; i++)
	{
		_values[i] = initval;
	}

}

FTspectrumModifier::~FTspectrumModifier()
{
	for (list<Listener*>::iterator iter = _listenerList.begin(); iter != _listenerList.end(); ++iter)
	{
		(*iter)->goingAway(this);
	}

	unlink(true);
	
	//printf ("delete specmod\n");
	delete [] _values;
	delete [] _tmpvalues;
	
}


void FTspectrumModifier::registerListener (Listener * listener)
{
	if ( find(_listenerList.begin(), _listenerList.end(), listener) == _listenerList.end())
	{
		_listenerList.push_back (listener);
	}
	
}

void FTspectrumModifier::unregisterListener (Listener *listener)
{
	_listenerList.remove (listener);

}


void FTspectrumModifier::setLength(int length)
{
	int origlen = _length;
	
	if (length < FT_MAX_FFT_SIZE/2) {
		_length = length;

		// now resample existing values into new length
		if (! _linkedTo)
		{
			float scale = origlen / (float) length;
			for (int i=0; i < length; i++) {
				_tmpvalues[i] = _values[(int)(i*scale)];
			}

			memcpy(_values, _tmpvalues, length * sizeof(float));
		}
	}
}


bool FTspectrumModifier::link (FTspectrumModifier *specmod)
{
	unlink(false);
	
	// do a cycle check
	FTspectrumModifier * spec = specmod;
	while (spec)
	{
		if (spec == this) {
			// cycle!!! bad!!
			return false;
		}
		spec = spec->getLink();
	}

	_linkedTo = specmod;

	specmod->addedLinkFrom(this);
	
	return true;
}

void FTspectrumModifier::unlink (bool unlinksources)
{
	if (_linkedTo) {
		_linkedTo->removedLinkFrom ( this );

		// copy their values into our internal
		copy (_linkedTo);
	}
	_linkedTo = 0;

	if (unlinksources) {
		// now unlink all who are linked to me
		for (list<FTspectrumModifier*>::iterator node = _linkedFrom.begin();
		     node != _linkedFrom.end(); )
		{
			FTspectrumModifier *specmod = (*node);
			specmod->unlink(false);

			node = _linkedFrom.begin();
		}
		_linkedFrom.clear();
	}
}

void FTspectrumModifier::addedLinkFrom (FTspectrumModifier * specmod)
{
	// called from link()

	if ( find(_linkedFrom.begin(), _linkedFrom.end(), specmod) == _linkedFrom.end())
	{
		_linkedFrom.push_back (specmod);
	}
		
}

void FTspectrumModifier::removedLinkFrom (FTspectrumModifier * specmod)
{
	// called from unlink()

	_linkedFrom.remove (specmod);
	
}

bool FTspectrumModifier::isLinkedFrom (FTspectrumModifier *specmod)
{
	list<FTspectrumModifier*>::iterator node = find (_linkedFrom.begin(),
							 _linkedFrom.end(),
							 specmod);

	return ( node != _linkedFrom.end());
}


float * FTspectrumModifier::getValues()
{
	if (_linkedTo) {
		return _linkedTo->getValues();
	}

	return _values;
}

void FTspectrumModifier::reset()
{
	float *data;

	if (getModifierType() == FREQ_MODIFIER)
	{
		float incr = (_max - _min) / _length;
		float val = _min;
		
		if (_linkedTo) {
			data = _linkedTo->getValues();
			for (int i=0; i < _length; i++) {
				_values[i] = data[i] = val;
				val += incr;
			}
		}
		else {
			for (int i=0; i < _length; i++) {
				_values[i] = val;
				val += incr;
			}
		}
	}
	else {
		if (_linkedTo) {
			data = _linkedTo->getValues();
			for (int i=0; i < _length; i++) {
				_values[i] = data[i] = _initval;
			}
		}
		else {
			for (int i=0; i < _length; i++) {
				_values[i] = _initval;
			}
		}
	}
}


void FTspectrumModifier::copy (FTspectrumModifier *specmod)
{
	// this always copies into our internal values
	if (!specmod) return;
	
	float * othervals = specmod->getValues();

	memcpy (_values, othervals, _length * sizeof(float));
}

XMLNode * FTspectrumModifier::getExtraNode()
{
	if (!_extra_node) {
		// create one
		_extra_node = new XMLNode("Extra");
	}

	return _extra_node;
}

void FTspectrumModifier::setExtraNode (XMLNode * node)
{
	if (!node) return;

	if (_extra_node) delete _extra_node;

	_extra_node = new XMLNode(*node);
}

