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

#ifndef __FTUPDATETOKEN_HPP__
#define __FTUPDATETOKEN_HPP__

class FTupdateToken
{
	
public:
	FTupdateToken() : _updated(false), _ignore(false) {};
	virtual ~FTupdateToken(){}

	void setUpdated (bool flag) { _updated = flag; }

	// this is as close of a test-and-set as I need
	bool getUpdated (bool tas=false)
		{
			if (_updated) {
				_updated = tas;
				return true;
			}
			return false;
		}

	void setIgnore (bool flag) { _ignore = flag; }
	bool getIgnore() { return _ignore;}
	
protected:

	volatile bool _updated;
	bool _ignore;
};


#endif
