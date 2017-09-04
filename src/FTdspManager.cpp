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

#include "FTdspManager.hpp"
#include "FTioSupport.hpp"
#include "FTprocI.hpp"

#include "FTprocEQ.hpp"
#include "FTprocGate.hpp"
#include "FTprocDelay.hpp"
#include "FTprocPitch.hpp"
#include "FTprocLimit.hpp"
#include "FTprocWarp.hpp"
#include "FTprocCompressor.hpp"
#include "FTprocBoost.hpp"

FTdspManager * FTdspManager::_instance = 0;

FTdspManager::FTdspManager()
{
	unsigned int fftn = 512;
	nframes_t samprate = FTioSupport::instance()->getSampleRate();

	// TODO: load initial dynamically
	
	FTprocI * procmod = new FTprocEQ (samprate, fftn);
	_prototypes.push_back (procmod);

	procmod = new FTprocBoost (samprate, fftn);
 	_prototypes.push_back (procmod);

	procmod = new FTprocPitch (samprate, fftn);
 	_prototypes.push_back (procmod);

	procmod = new FTprocGate (samprate, fftn);
 	_prototypes.push_back (procmod);

	procmod = new FTprocDelay (samprate, fftn);
 	_prototypes.push_back (procmod);

	procmod = new FTprocLimit (samprate, fftn);
 	_prototypes.push_back (procmod);

	procmod = new FTprocWarp (samprate, fftn);
 	_prototypes.push_back (procmod);

	procmod = new FTprocCompressor (samprate, fftn);
 	_prototypes.push_back (procmod);

}

FTdspManager::~FTdspManager()
{
	// cleanup prototypes
	for (ModuleList::iterator iter = _prototypes.begin(); iter != _prototypes.end(); ++iter) {
		delete (*iter);
	}
	_prototypes.clear();
}


void FTdspManager::getAvailableModules (ModuleList & outlist)
{
	outlist.clear();
	
	for (ModuleList::iterator iter = _prototypes.begin(); iter != _prototypes.end(); ++iter) {
		outlist.push_back(*iter);
	}

}

FTprocI * FTdspManager::getModuleByName (const string & name)
{
	for (ModuleList::iterator iter = _prototypes.begin(); iter != _prototypes.end(); ++iter) {
		if ((*iter)->getName() == name) {
			return (*iter);
		}
	}

	return 0;
}


FTprocI * FTdspManager::getModuleByConfigName (const string & name)
{
	for (ModuleList::iterator iter = _prototypes.begin(); iter != _prototypes.end(); ++iter) {
		if ((*iter)->getConfName() == name) {
			return (*iter);
		}
	}

	return 0;
}


