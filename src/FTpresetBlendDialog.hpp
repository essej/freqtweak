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

#ifndef __FTPRESETBLENDDIALOG_HPP__
#define __FTPRESETBLENDDIALOG_HPP__


#include <wx/wx.h>

#include "FTtypes.hpp"

#include <vector>
#include <string>
using namespace std;


class FTmainwin;
class FTconfigManager;
class FTpresetBlender;
class FTspectrumModifier;

class FTpresetBlendDialog
	: public wxFrame
{

  public:

	FTpresetBlendDialog(FTmainwin * parent, FTconfigManager * confman, wxWindowID id, const wxString & title,
			    const wxPoint& pos = wxDefaultPosition,
			    const wxSize& size = wxSize(400,600),
			    long style = wxDEFAULT_FRAME_STYLE,
			    const wxString& name = wxT("PresetBlend"));

	virtual ~FTpresetBlendDialog();


	void update();

	void refreshState(const wxString & defname = wxT(""), bool usefirst=false,  const wxString & defsec = wxT(""), bool usesec=false);
	
  protected:

	void init();

	void onClose(wxCloseEvent & ev);

	void onSize(wxSizeEvent &ev);
	void onPaint(wxPaintEvent &ev);

	void onSliders(wxScrollEvent &ev);
	void onCombo(wxCommandEvent &ev);
	
	wxBoxSizer * _procSizer;
	wxWindow * _procPanel;
	
	wxComboBox * _priPresetBox;
	wxComboBox * _secPresetBox;

	wxStaticText * _priStatus;
	wxStaticText * _secStatus;
	
	wxSlider *    _masterBlend;
	vector<wxSlider*> _blendSliders;

	typedef pair<unsigned int, unsigned int> ProcPair; 
	
	vector<ProcPair > _blendPairs;
	vector<FTspectrumModifier *> _filtRefs;
	
	FTmainwin * _mainwin;
	
	bool _justResized;
	int _namewidth;

	FTconfigManager * _configMan;

	FTpresetBlender * _presetBlender;
	
  private:
	// any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()

};




#endif
