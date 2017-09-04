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

#ifndef __FTPROCORDERDIALOG_HPP__
#define __FTPROCORDERDIALOG_HPP__


#include <list>
using namespace std;

#include <wx/wx.h>

#include "FTtypes.hpp"

class FTprocI;
class FTmainwin;

class wxListCtrl;

class FTprocOrderDialog : public wxFrame
{
  public:
	// ctor(s)
	FTprocOrderDialog(FTmainwin * parent, wxWindowID id, const wxString& title,
			  const wxPoint& pos = wxDefaultPosition,
			  const wxSize& size = wxSize(400,600),
			  long style = wxDEFAULT_FRAME_STYLE,
			  const wxString& name = wxT("ProcOrderDialog"));


	virtual ~FTprocOrderDialog();

	void refreshState();
	
 protected:

	class ModAction
	{
	   public:
		ModAction (FTprocI *pm, int frm , int toi, bool rem)
			: from(frm), to(toi), remove(rem), procmod(pm) {}
		ModAction (const ModAction & o)
			: from(o.from), to(o.to), remove(o.remove), procmod(o.procmod) {}

		int from;
		int to;
		bool remove;
		FTprocI * procmod;
	};

	void init();
	
	void onClose(wxCloseEvent & ev);
	void onCommit(wxCommandEvent & ev);

	void onSize(wxSizeEvent &ev);
	void onPaint(wxPaintEvent &ev);
	
	void onTargetButtons(wxCommandEvent & ev);
	void onAddButton(wxCommandEvent & ev);

	void onAutoCheck (wxCommandEvent &ev);

	wxListCtrl * _sourceList;
	wxListCtrl * _targetList;
	wxCheckBox * _autoCheck;
	wxStaticText * _modifiedText;
	
	FTmainwin * _mainwin;

	list<ModAction> _actions;

	bool _justResized;
	int _lastSelected;
private:
	// any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
	
};


#endif
