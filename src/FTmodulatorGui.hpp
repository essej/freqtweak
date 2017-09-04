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

#ifndef __FTMODULATORGUI_HPP__
#define __FTMODULATORGUI_HPP__

#include <wx/wx.h>

#include <map>

#include <sigc++/sigc++.h>

#include "FTtypes.hpp"
//#include "FTmodulatorI.hpp"

#include "LockMonitor.hpp"
#include "FTspectrumModifier.hpp"

class FTioSupport;
class FTmodulatorI;
class FTspectralEngine;

class FTmodulatorGui
	: public wxPanel, public SigC::Object
{
   public:
	FTmodulatorGui(FTioSupport * iosup, FTmodulatorI * mod,
		       wxWindow *parent, wxWindowID id, 
		       const wxPoint& pos = wxDefaultPosition,
		       const wxSize& size = wxDefaultSize,
		       long style = wxRAISED_BORDER,
		       const wxString& name = wxT("ModulatorGui"));

	virtual ~FTmodulatorGui();


	SigC::Signal0<void> RemovalRequest;
	
   protected:

	void init();

	void onCheckboxChanged(wxCommandEvent &ev);
	void onSliderChanged(wxScrollEvent &ev);
	void onChoiceChanged(wxCommandEvent &ev);
	void onRemoveButton (wxCommandEvent & ev);
	void onAttachButton (wxCommandEvent & ev);
	void onChannelButton (wxCommandEvent & ev);
	void onTextEnter (wxCommandEvent &ev);
	void onBypassButton (wxCommandEvent &ev);
	

	void onModulatorDeath (FTmodulatorI * mod);

	
	void onAttachMenu (wxCommandEvent &ev);
	void onChannelMenu (wxCommandEvent &ev);
	
	void refreshMenu();
	void refreshChannelMenu();
	
	FTmodulatorI * _modulator;
	FTioSupport * _iosup;
	
	wxTextCtrl *   _nameText;

	wxMenu *       _popupMenu;

	wxMenu *       _channelPopupMenu;
	
	// std::map<wxWindow *, FTmodulatorI::Control *> _controlMap;

  private:
	// any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
	
};


#endif
