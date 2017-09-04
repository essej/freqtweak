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
#include <string>
using namespace std;

#include "FTioSupport.hpp"
#include "FTjackSupport.hpp"

FTioSupport * FTioSupport::_instance = 0;

FTioSupport::IOtype FTioSupport::_iotype = FTioSupport::IO_JACK;
string FTioSupport::_defaultName;
string FTioSupport::_defaultServ;

FTioSupport * FTioSupport::createInstance()
{
	// static method

	if (_iotype == IO_JACK) {
		return new FTjackSupport(_defaultName.c_str(), _defaultServ.c_str());
	}
	else {
		return 0;
	}
}


void FTioSupport::setName (const string & name)
{
	_name = name;
}
