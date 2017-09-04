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

#include <string>
using namespace std;

#include <wx/wx.h>
#include <wx/listctrl.h>

#include "FTpresetBlendDialog.hpp"
#include "FTioSupport.hpp"
#include "FTmainwin.hpp"
#include "FTdspManager.hpp"
#include "FTprocI.hpp"
#include "FTprocessPath.hpp"
#include "FTspectralEngine.hpp"
#include "FTpresetBlender.hpp"

enum {
	ID_PriPresetCombo=2000,
	ID_SecPresetCombo,
	ID_MasterSlider,
	ID_FilterBlendSlider
};


BEGIN_EVENT_TABLE(FTpresetBlendDialog, wxFrame)
	EVT_CLOSE(FTpresetBlendDialog::onClose)

	
	EVT_SIZE (FTpresetBlendDialog::onSize)
	EVT_PAINT (FTpresetBlendDialog::onPaint)

	EVT_COMMAND_SCROLL (ID_MasterSlider, FTpresetBlendDialog::onSliders)
	EVT_COMMAND_SCROLL (ID_FilterBlendSlider, FTpresetBlendDialog::onSliders)

	EVT_COMBOBOX (ID_PriPresetCombo, FTpresetBlendDialog::onCombo)
	EVT_COMBOBOX (ID_SecPresetCombo, FTpresetBlendDialog::onCombo)
	
	
END_EVENT_TABLE()


FTpresetBlendDialog::FTpresetBlendDialog(FTmainwin * parent, FTconfigManager *confman,
					 wxWindowID id,
					 const wxString & title,
					 const wxPoint& pos,
					 const wxSize& size,
					 long style,
					 const wxString& name )

	: wxFrame(parent, id, title, pos, size, style, name),
	  _mainwin(parent), _justResized(false), _namewidth(85),
	  _configMan(confman)
{
	_presetBlender = new FTpresetBlender(_configMan);
	
	init();
}

FTpresetBlendDialog::~FTpresetBlendDialog()
{
	delete _presetBlender;
}


void FTpresetBlendDialog::onSize(wxSizeEvent &ev)
{

	_justResized = true;
	ev.Skip();
}

void FTpresetBlendDialog::onPaint(wxPaintEvent &ev)
{
	if (_justResized) {

		_justResized = false;

	}

	ev.Skip();
}

void FTpresetBlendDialog::init()
{
	wxBoxSizer * mainsizer = new wxBoxSizer(wxVERTICAL);

	
	wxBoxSizer *tmpsizer, *tmpsizer2;
	wxStaticText * stattext;
	wxBoxSizer * comboSizer = new wxBoxSizer(wxHORIZONTAL);

	tmpsizer = new wxBoxSizer(wxVERTICAL);
	tmpsizer2 = new wxBoxSizer(wxHORIZONTAL);
	
	stattext = new wxStaticText(this, -1, wxT("Preset 1: "), wxDefaultPosition, wxSize(-1, -1));
	//stattext->SetFont(titleFont);
	tmpsizer2->Add(stattext, 0, wxALL|wxEXPAND, 1);

	_priStatus = new wxStaticText(this, -1, wxT("not set"), wxDefaultPosition, wxSize(-1, -1));
	tmpsizer2->Add(_priStatus, 0, wxALL, 1);
	tmpsizer->Add (tmpsizer2, 0, wxALL|wxEXPAND, 1);
	
	_priPresetBox = new wxComboBox (this, ID_PriPresetCombo, wxT(""),  wxDefaultPosition, wxSize(175,-1), 0, 0, wxCB_READONLY|wxCB_SORT);
	tmpsizer->Add( _priPresetBox, 0, wxALL|wxEXPAND|wxALIGN_LEFT, 1);
	
	comboSizer->Add( tmpsizer, 0, wxALL|wxALIGN_LEFT, 1);

	comboSizer->Add(1,-1,1);

	tmpsizer = new wxBoxSizer(wxVERTICAL);
	tmpsizer2 = new wxBoxSizer(wxHORIZONTAL);
	
	stattext = new wxStaticText(this, -1, wxT("Preset 2: "), wxDefaultPosition, wxSize(-1, -1));
	//stattext->SetFont(titleFont);
	tmpsizer2->Add(stattext, 0, wxALL|wxEXPAND, 1);

	_secStatus = new wxStaticText(this, -1, wxT("not set"), wxDefaultPosition, wxSize(-1, -1));
	tmpsizer2->Add(_secStatus, 0, wxALL|wxEXPAND, 1);
	tmpsizer->Add (tmpsizer2, 0, wxALL|wxEXPAND, 1);
	
	_secPresetBox = new wxComboBox (this, ID_SecPresetCombo, wxT(""),  wxDefaultPosition, wxSize(175,-1),  0, 0, wxCB_READONLY|wxCB_SORT);
	tmpsizer->Add( _secPresetBox, 0, wxALL|wxEXPAND|wxALIGN_LEFT, 1);

	comboSizer->Add( tmpsizer, 0, wxALL|wxALIGN_LEFT, 1);

	
	mainsizer->Add (comboSizer, 0, wxEXPAND|wxALL, 2);

	tmpsizer = new wxBoxSizer(wxHORIZONTAL);
	stattext = new wxStaticText(this, -1, wxT("Master"), wxDefaultPosition, wxSize(_namewidth, -1));
	//stattext->SetFont(titleFont);
	stattext->SetSize(_namewidth, -1);
	
	tmpsizer->Add (stattext, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 1);
	_masterBlend = new wxSlider(this, ID_MasterSlider, 0, 0, 1000);
	tmpsizer->Add (_masterBlend, 1, wxALL|wxALIGN_CENTRE_VERTICAL, 3);
	
	mainsizer->Add (tmpsizer, 0, wxEXPAND|wxALL, 5);
	mainsizer->Add (2,5);

	//wxScrolledWindow * scrolled = new wxScrolledWindow(this, -1);
	
	_procSizer = new wxBoxSizer(wxVERTICAL);
	wxScrolledWindow *scrolled = new wxScrolledWindow(this, -1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxSUNKEN_BORDER);
	scrolled->SetScrollRate (0,20);
	
	_procPanel = scrolled;

	refreshState();

	_procPanel->SetAutoLayout( TRUE );
	_procSizer->Fit( _procPanel );  
	_procSizer->SetSizeHints( this );  
	_procPanel->SetSizer( _procSizer );
	
	
	
	mainsizer->Add (_procPanel, 1, wxEXPAND|wxALL, 2);
	
	SetAutoLayout( TRUE );
	mainsizer->Fit( this );  
	mainsizer->SetSizeHints( this );  
	SetSizer( mainsizer );

	this->SetSizeHints(400,100);

}


void FTpresetBlendDialog::refreshState(const wxString & defname, bool usefirst, const wxString & defsec, bool usesec)
{
	//wxFont titleFont(12, wxDEFAULT, wxNORMAL, wxBOLD);

	// not compatible with old wx
	_procSizer->Clear (true);
	_procSizer->Layout();

	_blendSliders.clear();
	_blendPairs.clear();
	_filtRefs.clear();
	
	// reload presets
	list<string> presetlist = _mainwin->getConfigManager().getSettingsNames();

	wxString origfirst = _priPresetBox->GetValue();
	wxString origsec = _secPresetBox->GetValue();

	_priPresetBox->Clear();
	_secPresetBox->Clear();

	for (list<string>::iterator name=presetlist.begin(); name != presetlist.end(); ++name)
	{
		_priPresetBox->Append(wxString::FromAscii ((*name).c_str()));
		_secPresetBox->Append(wxString::FromAscii ((*name).c_str()));
	}

	_priPresetBox->SetValue(defname.c_str());
	_secPresetBox->SetValue(defsec.c_str());

	
	
	// get configured modules from the first procpath

	
	FTprocessPath * procpath = FTioSupport::instance()->getProcessPath(0);
	if (procpath) {

		FTspectralEngine *engine = procpath->getSpectralEngine();
		vector<FTprocI *> procmods;
		engine->getProcessorModules (procmods);
		
		for (unsigned int n=0; n < procmods.size(); ++n)
		{
// 			item.SetText (procmods[n]->getName().c_str());
// 			item.SetData ((unsigned) procmods[n]);
// 			item.SetId (n);
			
// 			_targetList->InsertItem(item);
			FTprocI *pm = procmods[n];
			vector<FTspectrumModifier *> filts;
			pm->getFilters (filts);
			
			for (unsigned int m=0; m < filts.size(); ++m)
			{
			
				wxBoxSizer * tmpsizer = new wxBoxSizer(wxHORIZONTAL);
				
				wxStaticText * stattext = new wxStaticText(_procPanel,
									   -1,
									   wxString::FromAscii (filts[m]->getName().c_str()),
									   wxDefaultPosition,
									   wxSize(_namewidth, -1));
				//stattext->SetFont(titleFont);
				stattext->SetSize(_namewidth, -1);
				tmpsizer->Add (stattext, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 1);
				
				wxSlider * slid = new wxSlider(_procPanel, ID_FilterBlendSlider, 0, 0, 1000);
				tmpsizer->Add (slid, 1, wxALL|wxALIGN_CENTRE_VERTICAL, 1);
				
				_blendSliders.push_back (slid);
				
				_blendPairs.push_back (ProcPair(n, m));
				_filtRefs.push_back (filts[m]);
				
				_procSizer->Add (tmpsizer, 0, wxEXPAND|wxALL, 4);
			}
		}
	}

	_procSizer->Layout();
	
	// try to load them up
	if (usefirst) {
		if (_presetBlender->setPreset (static_cast<const char *> (defname.mb_str()), 0)) {
			_priPresetBox->SetValue (wxString(defname.c_str()));
			_priStatus->SetLabel (wxT("ready"));
		}
		else {
			_priStatus->SetLabel (wxT("not set or invalid"));
		}
	}
	else {
		if (_presetBlender->setPreset (static_cast<const char *> (origfirst.mb_str()), 0)) {
			_priPresetBox->SetValue (origfirst);
			_priStatus->SetLabel (wxT("ready"));
		}
		else {
			_priStatus->SetLabel (wxT("not set or invalid"));
		}
	}
	
	if (usesec) {
		if (_presetBlender->setPreset (static_cast<const char *> (defsec.mb_str()), 1)) {
			_secPresetBox->SetValue (wxString(defsec.c_str()));
			_secStatus->SetLabel (wxT("ready"));
		}
		else {
			_secStatus->SetLabel (wxT("not set or invalid"));
		}
		
	}
	else {
		if (_presetBlender->setPreset (static_cast<const char *> (origsec.mb_str()), 1)) {
			_secPresetBox->SetValue (origsec);
			_secStatus->SetLabel (wxT("ready"));
		}
		else {
			_secStatus->SetLabel (wxT("not set or invalid"));
		}
	}
	
}


void FTpresetBlendDialog::onClose(wxCloseEvent & ev)
{

	if (!ev.CanVeto()) {

		Destroy();
	}
	else {
		ev.Veto();

		Show(false);
	}
}


void FTpresetBlendDialog::onCombo(wxCommandEvent &ev)
{
	if (ev.GetId() == ID_PriPresetCombo) {

		wxString name = _priPresetBox->GetStringSelection();

		if (!name.empty()) {

			if (!_presetBlender->setPreset(static_cast<const char *> (name.mb_str()), 0)) {
				// display error message
				printf ("error could not load preset %s\n", static_cast<const char *> (name.mb_str()));
				_priPresetBox->SetSelection(-1);
				_priPresetBox->SetValue(wxT(""));
				_priStatus->SetLabel (wxT("not set or invalid"));

			}
			else {
				_priStatus->SetLabel (wxT("ready"));
			}
		}
	}
	else if (ev.GetId() == ID_SecPresetCombo) {

		wxString name = _secPresetBox->GetStringSelection();

		if (!name.empty()) {

			if (!_presetBlender->setPreset(static_cast<const char *> (name.mb_str()), 1)) {
				// display error message
				printf ("error could not load preset %s\n", static_cast<const char *> (name.mb_str()));
				_secPresetBox->SetSelection(-1);
				_secPresetBox->SetValue(wxT(""));
				_secStatus->SetLabel (wxT("not set or invalid"));
			}
			else {
				_secStatus->SetLabel (wxT("ready"));
			}
		}
	}

}


void FTpresetBlendDialog::onSliders(wxScrollEvent &ev)
{

	wxSlider * source = (wxSlider *) ev.GetEventObject();

	bool updateall = false;
	static bool ignoreevent = false;

	if (ignoreevent) return;
	
	for (unsigned int i=0; i < _blendSliders.size(); ++i)
	{
		if (_masterBlend == source) {
			ProcPair & ppair = _blendPairs[i];

			float max = (float) _masterBlend->GetMax();
			float min = (float) _masterBlend->GetMin(); 
			float newval = (((float)_masterBlend->GetValue() - min) / (max-min));
				
			_presetBlender->setBlend (ppair.first, ppair.second, 1.0 - newval);

			ignoreevent = true;
			_blendSliders[i]->SetValue(_masterBlend->GetValue());
			
			updateall = true;
		}
		else if (_blendSliders[i] == source) {

			ProcPair & ppair = _blendPairs[i];

			float max = (float) _blendSliders[i]->GetMax();
			float min = (float) _blendSliders[i]->GetMin(); 
			float newval = ((_blendSliders[i]->GetValue() - min) / (max-min));
				
			_presetBlender->setBlend (ppair.first, ppair.second, 1.0 - newval);

			_mainwin->updateGraphs(0, _filtRefs[i]->getSpecModifierType());
				
			break;
		}
	}

	if (updateall)
	{
		_mainwin->updateGraphs(0, ALL_SPECMOD);
		if (ignoreevent) {
			ignoreevent = false;
		}
	}
}

