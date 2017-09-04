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

#ifndef __FTPROCGATE_HPP__
#define __FTPROCGATE_HPP__

#include "FTprocI.hpp"

class FTprocGate
	: public FTprocI
{
  public:

	FTprocGate(nframes_t samprate, unsigned int fftn);
	FTprocGate (const FTprocGate & other);
	virtual ~FTprocGate();

	FTprocI * clone() { return new FTprocGate(*this); }
	void initialize();
	
	void process (fft_data *data, unsigned int fftn);

	
  protected:

	FTspectrumModifier * _filter;
	FTspectrumModifier * _invfilter;

	float _dbAdjust;
};


#endif
