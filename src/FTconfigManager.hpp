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

#ifndef __FTCONFIGMANAGER_HPP__
#define __FTCONFIGMANAGER_HPP__


#include <wx/wx.h>
#include <wx/textfile.h>
#include "FTtypes.hpp"
#include <string>
#include <list>
#include <vector>
using namespace std;

class FTspectralEngine;
class FTspectrumModifier;
class XMLNode;
class FTprocI;

class FTconfigManager
{
   public:
	
	FTconfigManager(const std::string &basedir = "");
	virtual ~FTconfigManager();


	bool storeSettings (const std::string &name, bool uselast=false);

	bool loadSettings (const std::string &name, bool restore_ports=false, bool uselast=false);
	bool loadSettings (const std::string &name, bool restore_ports, bool ignore_iosup, vector<vector <FTprocI *> > & procvec, bool uselast);

	list<std::string> getSettingsNames();

   protected:

	void writeFilter (FTspectrumModifier *specmod, wxTextFile & tf);

	void loadFilter (FTspectrumModifier *specmod, wxTextFile & tf);

	bool lookupFilterLocation (FTspectrumModifier * specmod, int & chan, int & modpos, int & filtpos);

	FTspectrumModifier * lookupFilter (int  chan, int  modpos, int  filtpos);

        void loadModulators (const XMLNode * modulatorsNode);
	
	XMLNode* find_named_node (const XMLNode * node, string name);
	
	std::string _basedir;

	class LinkCache {
	public:
		LinkCache (unsigned int src, unsigned int dest, unsigned int modn, unsigned int filtn)
			: source_chan(src), dest_chan(dest), mod_n(modn), filt_n(filtn) {}
		
		unsigned int source_chan;
		unsigned int dest_chan;
		unsigned int mod_n;
		unsigned int filt_n;

	};

	list<LinkCache> _linkCache;
};


#endif
