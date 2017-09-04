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

#include <wx/wx.h>

#include "FTmodulatorGui.hpp"
#include "FTmodulatorI.hpp"
#include "FTspectralEngine.hpp"
#include "FTioSupport.hpp"
#include "FTprocessPath.hpp"
#include "FTprocI.hpp"

using namespace SigC;

enum
{
	ID_RemoveButton = 8000,
	ID_AttachButton,
	ID_ChannelButton,
	ID_DetachAll,
	ID_AttachAll,
	ID_ModUserName,
	ID_BypassCheck
};


enum {
	ID_ControlBase = 10000,
	ID_SpecModBase = 11000,
	ID_ChannelBase = 12000
};

class FTmodControlObject : public wxObject
{
  public:
	FTmodControlObject(FTmodulatorI::Control *ctrl)
		: control(ctrl)
		  , slider(0), textctrl(0), choice(0), checkbox(0)
		{}
	
	FTmodulatorI::Control * control;

	wxSlider * slider;
	wxTextCtrl * textctrl;
	wxChoice * choice;
	wxCheckBox * checkbox;
};

class FTspecmodObject : public wxObject
{
  public:
	FTspecmodObject(int chan, int mi, int fi) : channel(chan), modIndex(mi), filtIndex(fi) {}

	int channel;
	int modIndex;
	int filtIndex;
};

class FTchannelObject : public wxObject
{
  public:
	FTchannelObject(int chan) : channel(chan) {}

	int channel;
};



BEGIN_EVENT_TABLE(FTmodulatorGui, wxPanel)

	EVT_BUTTON(ID_RemoveButton, FTmodulatorGui::onRemoveButton)
	EVT_BUTTON(ID_AttachButton, FTmodulatorGui::onAttachButton)
	EVT_BUTTON(ID_ChannelButton, FTmodulatorGui::onChannelButton)
	EVT_CHECKBOX(ID_BypassCheck, FTmodulatorGui::onBypassButton)
	
	EVT_TEXT_ENTER (ID_ModUserName, FTmodulatorGui::onTextEnter)
	
	EVT_MENU (ID_AttachAll, FTmodulatorGui::onAttachMenu)
	EVT_MENU (ID_DetachAll, FTmodulatorGui::onAttachMenu)
	
END_EVENT_TABLE()

FTmodulatorGui::FTmodulatorGui (FTioSupport * iosup, FTmodulatorI *mod, wxWindow *parent, wxWindowID id,
				const wxPoint& pos,
				const wxSize& size,
				long style ,
				const wxString& name)

	: wxPanel(parent, id, pos, size, style, name),
	  _modulator (mod), _iosup(iosup), _popupMenu(0), _channelPopupMenu(0)
{

	init();

}

FTmodulatorGui::~FTmodulatorGui()
{
	// cerr << "MODGUI destructor" << endl;
}


void FTmodulatorGui::init()
{

	wxBoxSizer * mainSizer = new wxBoxSizer(wxVERTICAL);


	wxBoxSizer * topSizer = new wxBoxSizer(wxHORIZONTAL);

//	wxBoxSizer *tmpsizer, *tmpsizer2;
	wxStaticText * stattext;

	stattext = new wxStaticText(this, -1, wxString::FromAscii(_modulator->getName().c_str()),
				    wxDefaultPosition, wxSize(-1, -1));
	stattext->SetFont(wxFont(stattext->GetFont().GetPointSize(), wxDEFAULT, wxNORMAL, wxBOLD));
	topSizer->Add (stattext, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 2);


	_nameText = new wxTextCtrl (this, ID_ModUserName, wxString::FromAscii(_modulator->getUserName().c_str()), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	topSizer->Add (_nameText, 1, wxALL|wxALIGN_CENTRE_VERTICAL, 2);

	wxCheckBox * bypassCheck = new wxCheckBox(this, ID_BypassCheck, wxT("Bypass"));
	bypassCheck->SetValue(_modulator->getBypassed());
	topSizer->Add (bypassCheck, 0, wxTOP|wxBOTTOM|wxALIGN_CENTRE_VERTICAL, 2);
	
// 	wxButton * chanButton = new wxButton(this, ID_ChannelButton, wxT("Source..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
// 	topSizer->Add (chanButton, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 2);

	
	wxButton * attachButton = new wxButton(this, ID_AttachButton, wxT("Attach..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	topSizer->Add (attachButton, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 2);

	wxButton * removeButton = new wxButton(this, ID_RemoveButton, wxT("X"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	topSizer->Add (removeButton, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 2);

	
	
	mainSizer->Add (topSizer, 0, wxEXPAND|wxALL, 2);

	
	wxBoxSizer * controlSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer * rowsizer;

	
	int ctrlid = ID_ControlBase;
	int textwidth = 70;
	
	// get controls
	FTmodulatorI::ControlList controls;
	_modulator->getControls (controls);

	for (FTmodulatorI::ControlList::iterator ctrliter = controls.begin(); ctrliter != controls.end(); ++ctrliter)
	{
		FTmodulatorI::Control * ctrl = (FTmodulatorI::Control *) *ctrliter;

		wxString unitstr = wxString::Format(wxT("%s"), ctrl->getName().c_str());
		if (!ctrl->getUnits().empty()) {
			unitstr +=  wxString::Format(wxT(" [%s]"), ctrl->getUnits().c_str());
		}
		
		if (ctrl->getType() == FTmodulatorI::Control::BooleanType) {
			// make a checkbox
			
			wxCheckBox * checkb = new wxCheckBox(this, ctrlid, unitstr);
							     

			FTmodControlObject * ctrlobj = new FTmodControlObject(ctrl);
			ctrlobj->checkbox = checkb;
			
			Connect( ctrlid,  wxEVT_COMMAND_CHECKBOX_CLICKED,
				 (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
				 &FTmodulatorGui::onCheckboxChanged,
				 ctrlobj);

			controlSizer->Add (checkb, 0, wxEXPAND|wxALL, 2);
		}
		else if (ctrl->getType() == FTmodulatorI::Control::IntegerType) {
			// make a slider and spinbox for now

			rowsizer = new wxBoxSizer(wxHORIZONTAL);
			stattext = new wxStaticText(this, -1, unitstr);

			rowsizer->Add (stattext, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 2);

			int currval = 0;
			int minval=0,maxval=1;
			ctrl->getValue(currval);
			ctrl->getBounds(minval, maxval);
			
			wxSlider * slider = new wxSlider(this, ctrlid, currval, minval, maxval);

			rowsizer->Add (slider, 1, wxALL|wxALIGN_CENTRE_VERTICAL, 2);

			ctrlid++;
			
			wxTextCtrl * textctrl = new wxTextCtrl(this, ctrlid, wxString::Format(wxT("%d"), currval), wxDefaultPosition, wxSize(textwidth, -1),
							       wxTE_PROCESS_ENTER|wxTE_RIGHT);
			rowsizer->Add (textctrl, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 2);
			
			FTmodControlObject * ctrlobj = new FTmodControlObject(ctrl);
			ctrlobj->slider = slider;
			ctrlobj->textctrl = textctrl;
			
			Connect( slider->GetId(),  wxEVT_SCROLL_THUMBTRACK,
				 (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxScrollEventFunction)
				 &FTmodulatorGui::onSliderChanged,
				 ctrlobj);

			ctrlobj= new FTmodControlObject(ctrl);
			ctrlobj->slider = slider;
			ctrlobj->textctrl = textctrl;
			
			Connect( textctrl->GetId(),  wxEVT_COMMAND_TEXT_ENTER,
				 (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
				 &FTmodulatorGui::onTextEnter,
				 ctrlobj);
			
			
			controlSizer->Add (rowsizer, 0, wxEXPAND|wxALL, 2);
			
		}
		else if (ctrl->getType() == FTmodulatorI::Control::FloatType) {
			// make a slider and spinbox for now
			rowsizer = new wxBoxSizer(wxHORIZONTAL);
			stattext = new wxStaticText(this, -1, unitstr);

			rowsizer->Add (stattext, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 2);

			float currval = 0;
			float minval=0,maxval=1;
			int calcval = 0;
			
			ctrl->getValue(currval);
			ctrl->getBounds(minval, maxval);

			// we'll always have slider values between 0 and 1000 for now

			calcval = (int) (((currval-minval) / (maxval - minval)) * 1000);
			
			wxSlider * slider = new wxSlider(this, ctrlid, (int) calcval, 0, 1000);

			rowsizer->Add (slider, 1, wxALL|wxALIGN_CENTRE_VERTICAL, 2);

			ctrlid++;
			
			wxTextCtrl * textctrl = new wxTextCtrl(this, ctrlid, wxString::Format(wxT("%.6g"), currval), wxDefaultPosition, wxSize(textwidth, -1),
							       wxTE_PROCESS_ENTER|wxTE_RIGHT);
			rowsizer->Add (textctrl, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 2);
			
			FTmodControlObject * ctrlobj = new FTmodControlObject(ctrl);
			ctrlobj->slider = slider;
			ctrlobj->textctrl = textctrl;
			
			Connect( slider->GetId(),  wxEVT_SCROLL_THUMBTRACK,
				 (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxScrollEventFunction)
				 &FTmodulatorGui::onSliderChanged,
				 ctrlobj);

			ctrlobj= new FTmodControlObject(ctrl);
			ctrlobj->slider = slider;
			ctrlobj->textctrl = textctrl;
						
			Connect( textctrl->GetId(),  wxEVT_COMMAND_TEXT_ENTER,
				 (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
				 &FTmodulatorGui::onTextEnter,
				 ctrlobj);

			
			
			controlSizer->Add (rowsizer, 0, wxEXPAND|wxALL, 2);
			
		}
		else if (ctrl->getType() == FTmodulatorI::Control::EnumType) {
			// use a wxChoice

			rowsizer = new wxBoxSizer(wxHORIZONTAL);
			stattext = new wxStaticText(this, -1, unitstr);

			rowsizer->Add (stattext, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 2);

		        string currval;
			list<string> vallist;

			ctrl->getValue(currval);
			ctrl->getEnumStrings (vallist);
			
			wxChoice * choice = new wxChoice(this, ctrlid, wxDefaultPosition, wxDefaultSize, 0, 0);

			
			for (list<string>::iterator citer = vallist.begin(); citer != vallist.end(); ++citer) {
				choice->Append (wxString::FromAscii((*citer).c_str()));
			}
			choice->SetStringSelection (wxString::FromAscii(currval.c_str()));

			
			rowsizer->Add (choice, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 2);

			FTmodControlObject * ctrlobj = new FTmodControlObject(ctrl);
			ctrlobj->choice = choice;
			
			Connect( ctrlid,  wxEVT_COMMAND_CHOICE_SELECTED,
				 (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
				 &FTmodulatorGui::onChoiceChanged,
				 ctrlobj);

			
			controlSizer->Add (rowsizer, 0, wxEXPAND|wxALL, 2);
			
		}
		
		ctrlid++;
	}

	//controlSizer->Add(new wxButton(this, -1, wxT("BIG TEST"), wxDefaultPosition, wxSize(120, 90)),
	//		  1, wxEXPAND|wxALL, 2);


	_modulator->GoingAway.connect ( slot (*this, &FTmodulatorGui::onModulatorDeath));
	
	mainSizer->Add (controlSizer, 1, wxEXPAND|wxALL, 2);
	
	SetAutoLayout( TRUE );
	mainSizer->Fit( this );  
	mainSizer->SetSizeHints( this );  
	SetSizer( mainSizer );
}


void FTmodulatorGui::refreshMenu()
{
	if (_popupMenu) {
		delete _popupMenu;
	}
	
	_popupMenu = new wxMenu();

	int itemid = ID_SpecModBase;
	
	_popupMenu->Append (ID_DetachAll, wxT("Detach All"));
	_popupMenu->Append (ID_AttachAll, wxT("Attach All"));

	FTprocessPath * procpath;
	for (int i=0; i < _iosup->getActivePathCount(); ++i)
	{
		procpath = _iosup->getProcessPath(i);
		if (procpath) {

			FTspectralEngine *engine = procpath->getSpectralEngine();

			_popupMenu->AppendSeparator();
			_popupMenu->Append (itemid, wxString::Format(wxT("Channel %d"), i+1));
			_popupMenu->Enable (itemid, false);
			itemid++;
						   
			
			// go through all the spectrum modifiers in the engine
			vector<FTprocI *> procmods;
			engine->getProcessorModules (procmods);
			
			for (unsigned int n=0; n < procmods.size(); ++n)
			{
				FTprocI *pm = procmods[n];
				vector<FTspectrumModifier *> filts;
				pm->getFilters (filts);
				
				for (unsigned int m=0; m < filts.size(); ++m)
				{
					
					_popupMenu->AppendCheckItem (itemid, wxString::FromAscii (filts[m]->getName().c_str()));
					
					if (_modulator->hasSpecMod (filts[m])) {
						_popupMenu->Check (itemid, true);
					}
					
					Connect( itemid,  wxEVT_COMMAND_MENU_SELECTED,
						 (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
						 &FTmodulatorGui::onAttachMenu,
						 (wxObject *) new FTspecmodObject(i, n, m));
					
					itemid++;
				}
			}
		}
	}

}

void FTmodulatorGui::onAttachMenu (wxCommandEvent & ev)
{
	int id = ev.GetId();

	if (id == ID_AttachAll) {
		// go through every one and add it
		FTprocessPath * procpath;
		for (int i=0; i < _iosup->getActivePathCount(); ++i)
		{
			procpath = _iosup->getProcessPath(i);
			if (procpath) {
				
				FTspectralEngine *engine = procpath->getSpectralEngine();
				
				vector<FTprocI *> procmods;
				engine->getProcessorModules (procmods);
				
				for (unsigned int n=0; n < procmods.size(); ++n)
				{
					FTprocI *pm = procmods[n];
					vector<FTspectrumModifier *> filts;
					pm->getFilters (filts);
					
					for (unsigned int m=0; m < filts.size(); ++m)
					{
						_modulator->addSpecMod (filts[m]);
					}
				}
			}
		}
	}
	else if (id == ID_DetachAll)
	{
		_modulator->clearSpecMods();
	}
	else {
		// a filter menu item
		FTspecmodObject * smo = (FTspecmodObject *) ev.m_callbackUserData;
		FTprocI * procmod;
		FTspectrumModifier * specmod;
		FTprocessPath *  procpath;
		FTspectralEngine * engine;
		
		if (smo
		    && (procpath = _iosup->getProcessPath(smo->channel))
		    && (engine   = procpath->getSpectralEngine())
		    && (procmod = engine->getProcessorModule(smo->modIndex))
		    && (specmod = procmod->getFilter(smo->filtIndex)))
		{
			if (ev.IsChecked()) {
				_modulator->addSpecMod (specmod);
			}
			else {
				_modulator->removeSpecMod (specmod);
			}
		}
	}

}

void FTmodulatorGui::onRemoveButton (wxCommandEvent & ev)
{
	// remove our own dear mod
	//cerr << "on remove" << endl;

	RemovalRequest (); // emit signal
	
	// remove from old engine 
	for (int i=0; i < _iosup->getActivePathCount(); ++i)
	{
		FTprocessPath * ppath = _iosup->getProcessPath(i);
		if (ppath) {
			ppath->getSpectralEngine()->removeModulator (_modulator);
		}
	}

	//cerr << "post remove" << endl;

	_modulator = 0;
}

void FTmodulatorGui::onModulatorDeath (FTmodulatorI * mod)
{
	// cerr << "modguiuui: mod death" << endl;
	_modulator = 0;
}


void FTmodulatorGui::onAttachButton (wxCommandEvent & ev)
{

	wxWindow * source = (wxWindow *) ev.GetEventObject();
	
	wxRect pos = source->GetRect();

	refreshMenu();
	
	PopupMenu(_popupMenu, pos.x, pos.y + pos.height);
		
}



void FTmodulatorGui::onSliderChanged(wxScrollEvent &ev)
{
	wxSlider * slider = (wxSlider *) ev.GetEventObject();

	FTmodControlObject * obj = (FTmodControlObject *) ev.m_callbackUserData;
	FTmodulatorI::Control * ctrl;

	if (obj && (ctrl = obj->control)) {
	
		
		if (ctrl->getType() == FTmodulatorI::Control::IntegerType) {
			int currval = slider->GetValue();

			ctrl->setValue(currval);
			//cerr << "slider int changed for " << ctrl->getName() <<  ": new val = " << currval << endl;

			if (obj->textctrl) {
				obj->textctrl->SetValue(wxString::Format(wxT("%d"), currval));
			}
			
		}
		else if (ctrl->getType() == FTmodulatorI::Control::FloatType) {
			float minval,maxval;
			ctrl->getBounds(minval, maxval);
			float currval = (slider->GetValue() / 1000.0) * (maxval - minval)  + minval;

			ctrl->setValue (currval);
			//cerr << "slider float changed for " << ctrl->getName() <<  ": new val = " << currval << endl;

			if (obj->textctrl) {
				obj->textctrl->SetValue(wxString::Format(wxT("%.6g"), currval));
			}
		}
	}
	
}

void FTmodulatorGui::onChoiceChanged(wxCommandEvent &ev)
{
	wxChoice * choice = (wxChoice *) ev.GetEventObject();

	FTmodControlObject * obj = (FTmodControlObject *) ev.m_callbackUserData;
	FTmodulatorI::Control * ctrl;

	if (obj && (ctrl = obj->control)) {

		
		ctrl->setValue (string(static_cast<const char *> (choice->GetStringSelection().mb_str())));

		//cerr << " choice changed for " << ctrl->getName() <<  ": new val = " << choice->GetStringSelection().c_str() << endl;
		
	}

}

void FTmodulatorGui::onCheckboxChanged(wxCommandEvent &ev)
{
	FTmodControlObject * obj = (FTmodControlObject *) ev.m_callbackUserData;
	FTmodulatorI::Control * ctrl;

	if (obj && (ctrl = obj->control) && obj->checkbox) {
		
		ctrl->setValue ((bool) obj->checkbox->GetValue());

		// cerr << " checkbox changed for " << ctrl->getName() <<  ": new val = " << obj->checkbox->GetValue() << endl;
		
	}
}

void FTmodulatorGui::refreshChannelMenu ()
{

	if (_channelPopupMenu) {
		delete _channelPopupMenu;
	}
	
	_channelPopupMenu = new wxMenu();

	int itemid = ID_ChannelBase;
	
	FTprocessPath * procpath;
	for (int i=0; i < _iosup->getActivePathCount(); ++i)
	{
		procpath = _iosup->getProcessPath(i);
		if (procpath) {
			
			_channelPopupMenu->AppendCheckItem (itemid, wxString::Format(wxT("Channel %d"), i+1));

			if (procpath->getSpectralEngine()->hasModulator(_modulator)) {
				_channelPopupMenu->Check (itemid, true);
			}
			
			Connect( itemid,  wxEVT_COMMAND_MENU_SELECTED,
				 (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
				 &FTmodulatorGui::onChannelMenu,
				 (wxObject *) new FTchannelObject(i));
			
			itemid++;
		}
	}

}

void FTmodulatorGui::onChannelMenu (wxCommandEvent &ev)
{
	FTchannelObject * smo = (FTchannelObject *) ev.m_callbackUserData;
	FTprocessPath *  procpath;
	
	if (smo
	    && (procpath = _iosup->getProcessPath(smo->channel))
	    && (!procpath->getSpectralEngine()->hasModulator(_modulator)))
	{
		// only if engine doesn't already contain it
		// cerr << "on channel menu" << endl;
		
		// remove from old engine without destroying
		for (int i=0; i < _iosup->getActivePathCount(); ++i)
		{
			FTprocessPath * ppath = _iosup->getProcessPath(i);
			if (ppath) {
				ppath->getSpectralEngine()->removeModulator (_modulator, false);
			}
		}

		// add to new one
		procpath->getSpectralEngine()->appendModulator(_modulator);
		
	}
	    

}

void FTmodulatorGui::onChannelButton (wxCommandEvent &ev)
{
	wxWindow * source = (wxWindow *) ev.GetEventObject();
	
	wxRect pos = source->GetRect();

	refreshChannelMenu();
	
	PopupMenu(_channelPopupMenu, pos.x, pos.y + pos.height);
}


void FTmodulatorGui::onTextEnter (wxCommandEvent &ev)
{
	
	if (ev.GetId() == ID_ModUserName)
	{

		string name = static_cast<const char *> (_nameText->GetValue().fn_str());
		
		_modulator->setUserName (name);
		// cerr << "name changed to :" << name << endl;
	}
	else {

		FTmodControlObject * cobj = (FTmodControlObject *) ev.m_callbackUserData;
		wxString tmpstr;
		long tmplong;
		double tmpfloat;
		
		if (cobj && cobj->textctrl) {
			tmpstr = cobj->textctrl->GetValue();

			if (cobj->control->getType() == FTmodulatorI::Control::IntegerType) {
				int lb,ub,currval;
				cobj->control->getBounds(lb, ub);
				cobj->control->getValue(currval);
				
				
				if (tmpstr.ToLong (&tmplong)
				    && cobj->control->setValue ((int) tmplong)
				    && (float)tmpfloat >= lb && (float)tmpfloat <= ub)
				{
					// change slider too
					cobj->slider->SetValue (tmplong);
				}
				else {
					cobj->textctrl->SetValue(wxString::Format(wxT("%d"), currval));
				}
			}
			else if (cobj->control->getType() == FTmodulatorI::Control::FloatType) {
				float lb,ub,currval;
				cobj->control->getBounds(lb, ub);
				cobj->control->getValue(currval);
				
				if (tmpstr.ToDouble (&tmpfloat)
				    && cobj->control->setValue ((float) tmpfloat)
				    && (float)tmpfloat >= lb && (float)tmpfloat <= ub)
				{
					// change slider too
					int slidval = (int) ((tmpfloat - lb) / (ub-lb) * 1000.0);
					cobj->slider->SetValue (slidval);
				}
				else {
					cobj->textctrl->SetValue(wxString::Format(wxT("%.6g"), currval));
				}
			}
		}

	}
}


void FTmodulatorGui::onBypassButton (wxCommandEvent &ev)
{

	if (_modulator->getBypassed() != ev.IsChecked()) {
	
		_modulator->setBypassed (ev.IsChecked());
	}
	
}
