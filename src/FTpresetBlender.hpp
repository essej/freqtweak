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

#ifndef __FTPRESETBLENDER_HPP__
#define __FTPRESETBLENDER_HPP__


#include "FTtypes.hpp"

#include <string>
#include <list>
#include <vector>
using namespace std;

class FTprocI;
class FTconfigManager;
class FTspectrumModifier;

class FTpresetBlender
{
   public:
	
	FTpresetBlender(FTconfigManager * confman);
	virtual ~FTpresetBlender();
	

	bool setPreset(const string & name, int index);
	string getPreset(int index);

	bool setBlend (unsigned int specmod_n, unsigned int filt_n, float val);
	float getBlend (unsigned int specmod_n, unsigned int filt_n);
	
   protected:

	void blendFilters (FTspectrumModifier * targFilt, FTspectrumModifier *filt0, FTspectrumModifier *filt1, float val);
	

	vector <vector<vector <FTprocI*> > *> _presetList;

	vector <string> _presetNames;
	
	FTconfigManager * _configMan;

	FTpresetBlender * _presetBlender;
};

#endif
