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


#include "FThelpWindow.hpp"

FThelpWindow::FThelpWindow(wxWindow * parent, wxWindowID id, const wxString & title,
			   const wxPoint& pos,
			   const wxSize& size,
			   long style,
			   const wxString& name)

	: wxFrame(parent, id, title, pos, size, style, name)
{

	init();
}

FThelpWindow::~FThelpWindow()
{

}
	

void FThelpWindow::init()
{
	wxBoxSizer * mainsizer = new wxBoxSizer(wxVERTICAL);
	
	_htmlWin = new wxHtmlWindow(this, -1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxHW_SCROLLBAR_AUTO);

	mainsizer->Add (_htmlWin, 1, wxALL|wxEXPAND, 4);

	SetAutoLayout( TRUE );
	mainsizer->Fit( this );  
	mainsizer->SetSizeHints( this );  
	SetSizer( mainsizer );

	const int sizes[] = {7, 8, 10, 12, 16, 22, 30};
	
	_htmlWin->SetFonts(wxT(""), wxT(""), sizes);
#ifndef __WXMAC__
	wxString helppath = wxString(wxT(HELP_HTML_PATH)) + wxFileName::GetPathSeparator() + wxString(wxT("usagehelp.html")); 
#else
	wxString helppath(wxT(""));
#endif	
	if (wxFile::Access(helppath, wxFile::read))
	{
		_htmlWin->LoadPage(helppath);
	}
	else {
		_htmlWin->SetPage(wxString(wxT("Help information could not be found at ")) + helppath +
				  wxString(wxT(" . If you can't get it, please see http://freqtweak.sf.net")));
				  
	}
		
	this->SetSizeHints(200,100);
	this->SetSize(600,400);
}
