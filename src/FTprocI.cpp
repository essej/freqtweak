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

#if HAVE_CONFIG_H
#include <config.h>
#endif


#include "FTprocI.hpp"


FTprocI::FTprocI (const string & name, nframes_t samprate, unsigned int fftn)
	: _sampleRate(samprate), _fftN(fftn), _oversamp(4), _inited(false), _name(name), _confname(name)
{
}


FTprocI::~FTprocI()
{
}

void FTprocI::setBypassed (bool flag)
{
	_bypassed = flag; 
	for (FilterList::iterator filt = _filterlist.begin();
	     filt != _filterlist.end(); ++filt)
	{
		(*filt)->setBypassed (flag);
	}
}

void FTprocI::setId (int id)
{
	_id = id;
	for (FilterList::iterator filt = _filterlist.begin();
	     filt != _filterlist.end(); ++filt)
	{
		(*filt)->setId(id);
	}
}
	


