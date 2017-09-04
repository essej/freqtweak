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

#ifndef __FTPROCI_HPP__
#define __FTPROCI_HPP__


#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "FTtypes.hpp"

#include "xml++.hpp"

#include <map>
#include <list>
#include <vector>
#include <string>
using namespace std;

#include "FTtypes.hpp"
#include "FTspectrumModifier.hpp"

// Limit a value to be l<=v<=u
#define LIMIT(v,l,u) ((v)<(l)?(l):((v)>(u)?(u):(v)))

// to handle denormals
#define FLUSH_TO_ZERO(fv) (((*(unsigned int*)&(fv))&0x7f800000)==0)?0.0f:(fv)


class FTprocI
{
  public:
	//typedef map<string, FTspectrumModifier *> FilterMap;
	typedef vector<FTspectrumModifier *> FilterList;

	virtual ~FTprocI();

	virtual FTprocI * clone() = 0;
	
	virtual void initialize() = 0;
	
	virtual void process (fft_data *data, unsigned int fftn) = 0;

	virtual void setBypassed (bool flag);

	virtual void setId (int id);

	//virtual bool getBypassed () { return _bypassed; }
	
	virtual FTspectrumModifier * getFilter(unsigned int n) {
		if (n < _filterlist.size()) {
			return _filterlist[n];
		}
		return 0;
	}

	virtual void getFilters (vector<FTspectrumModifier *> & filtlist) {
		filtlist.clear();
		
		for (FilterList::iterator filt = _filterlist.begin();
		     filt != _filterlist.end(); ++filt)
		{
			filtlist.push_back ((*filt));
		}
	}

	
	virtual void setFFTsize (unsigned int fftn) {
		_fftN = fftn;
		for (FilterList::iterator filt = _filterlist.begin();
		     filt != _filterlist.end(); ++filt)
		{
			(*filt)->setLength (fftn/2);
		}
	}
		
	virtual void setSampleRate (nframes_t rate) { _sampleRate = rate; }
	virtual nframes_t getSampleRate() { return _sampleRate; }

	virtual void setOversamp (int osamp) { _oversamp = osamp; }
	virtual int getOversamp() { return _oversamp; }

	virtual void setName (const string & name) { _name = name; }
	virtual const string & getName() { return _name; }

	virtual const string & getConfName() { return _confname; }

	virtual void setMaxDelay(float secs) {}
	
	virtual void reset() {}

	virtual bool useAsDefault() { return true; }
 protected:

	FTprocI (const string & name, nframes_t samprate, unsigned int fftn);
	
	
	bool _bypassed;
	nframes_t _sampleRate;
	unsigned int _fftN;
	int _oversamp;
	bool _inited;

        FilterList _filterlist;

	int _id;
	string _name;
	string _confname;
};


#endif
