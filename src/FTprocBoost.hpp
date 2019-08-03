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
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**  
*/

#ifndef __FTPROCBOOST_HPP__
#define __FTPROCBOOST_HPP__

#include "FTprocI.hpp"

class FTprocBoost
	: public FTprocI
{
  public:

	FTprocBoost(nframes_t samprate, unsigned int fftn);
	FTprocBoost (const FTprocBoost & other);

	virtual ~FTprocBoost();

	FTprocI * clone() { return new FTprocBoost(*this); }
	void initialize();
	
	void process (fft_data *data,  unsigned int fftn);

	virtual bool useAsDefault() { return false; }
	
  protected:

	FTspectrumModifier * _eqfilter;

};

#endif
