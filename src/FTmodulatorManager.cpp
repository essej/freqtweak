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

#include "FTmodulatorManager.hpp"
#include "FTioSupport.hpp"
#include "FTmodulatorI.hpp"

#include "FTmodRandomize.hpp"
#include "FTmodRotate.hpp"
#include "FTmodRotateLFO.hpp"
#include "FTmodValueLFO.hpp"

FTmodulatorManager * FTmodulatorManager::_instance = 0;

FTmodulatorManager::FTmodulatorManager()
{
	unsigned int fftn = 512;
	nframes_t samprate = FTioSupport::instance()->getSampleRate();

	// TODO: load initial dynamically
	

	FTmodulatorI * procmod = new FTmodRotate (samprate, fftn);
	_prototypes.push_back (procmod);

	procmod = new FTmodRotateLFO (samprate, fftn);
	_prototypes.push_back (procmod);

	procmod = new FTmodValueLFO (samprate, fftn);
	_prototypes.push_back (procmod);
	
	 procmod = new FTmodRandomize (samprate, fftn);
	_prototypes.push_back (procmod);

	
}

FTmodulatorManager::~FTmodulatorManager()
{
	// cleanup prototypes
	for (ModuleList::iterator iter = _prototypes.begin(); iter != _prototypes.end(); ++iter) {
		delete (*iter);
	}
	_prototypes.clear();
}


void FTmodulatorManager::getAvailableModules (ModuleList & outlist)
{
	outlist.clear();
	
	for (ModuleList::iterator iter = _prototypes.begin(); iter != _prototypes.end(); ++iter) {
		outlist.push_back(*iter);
	}

}

FTmodulatorI * FTmodulatorManager::getModuleByName (const string & name)
{
	for (ModuleList::iterator iter = _prototypes.begin(); iter != _prototypes.end(); ++iter) {
		if ((*iter)->getName() == name) {
			return (*iter);
		}
	}

	return 0;
}


FTmodulatorI * FTmodulatorManager::getModuleByConfigName (const string & name)
{
	for (ModuleList::iterator iter = _prototypes.begin(); iter != _prototypes.end(); ++iter) {
		if ((*iter)->getConfName() == name) {
			return (*iter);
		}
	}

	return 0;
}

FTmodulatorI * FTmodulatorManager::getModuleByIndex (unsigned int index)
{
	if (index < _prototypes.size()) {

		unsigned int n=0;
		for (ModuleList::iterator iter = _prototypes.begin(); iter != _prototypes.end(); ++iter) {
			if (n == index) {
				return (*iter);
			}
			n++;
		}
		
	}

	return 0;
}

