/*
** Copyright (C) 2004 Jesse Chappell <jesse@essej.net>
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

#ifndef __FTMODULATORMANAGER_HPP__
#define __FTMODULATORMANAGER_HPP__


#include "FTtypes.hpp"

#include <list>
#include <string>
using namespace std;

class FTmodulatorI;

class FTmodulatorManager
{
   public:
	typedef list<FTmodulatorI*> ModuleList;
	
	FTmodulatorManager();
	virtual ~FTmodulatorManager();

	static FTmodulatorManager * instance() { if (!_instance) _instance = new FTmodulatorManager(); return _instance; }
	
	void getAvailableModules (ModuleList & outlist);

	FTmodulatorI * getModuleByName (const string & name);
	FTmodulatorI * getModuleByConfigName (const string & name);
	FTmodulatorI * getModuleByIndex (unsigned int index);
	
   protected:

	ModuleList _prototypes;

	static FTmodulatorManager* _instance;
};

#endif
