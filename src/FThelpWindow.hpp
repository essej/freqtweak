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

#ifndef __FTHELPWIN_HPP__
#define __FTHELPWIN_HPP__

#include <wx/wx.h>
#include <wx/html/htmlwin.h>

#include "FTtypes.hpp"

#include <vector>
#include <string>
using namespace std;


class FThelpWindow
	: public wxFrame
{
  public:

	FThelpWindow(wxWindow * parent, wxWindowID id, const wxString & title,
		     const wxPoint& pos = wxDefaultPosition,
		     const wxSize& size = wxSize(400,600),
		     long style = wxDEFAULT_FRAME_STYLE,
		     const wxString& name = wxT("HelpWin"));

	virtual ~FThelpWindow();
	

  protected:

	void init();

	wxHtmlWindow * _htmlWin;
};

#endif
