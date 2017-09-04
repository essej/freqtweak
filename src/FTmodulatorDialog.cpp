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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <wx/wx.h>

#include <wx/listctrl.h>
#include <iostream>
using namespace std;

#include "FTmodulatorDialog.hpp"
#include "FTmodulatorManager.hpp"
#include "FTioSupport.hpp"
#include "FTmainwin.hpp"
#include "FTdspManager.hpp"
#include "FTprocI.hpp"
#include "FTmodulatorI.hpp"
#include "FTprocessPath.hpp"
#include "FTspectralEngine.hpp"
#include "FTmodulatorGui.hpp"

#include <sigc++/sigc++.h>
using namespace SigC;

enum {
	ID_AddButton=8000,
	ID_PopupMenu,
	ID_EditMenuItem,
	ID_RemoveMenuItem,
	ID_ChannelList,
	ID_ClearButton
};


enum {
	ID_AddModulatorBase = 9000,
	ID_AddModulatorChannelBase = 9100,
	ID_AddModulatorChannelMax = 9110
};

BEGIN_EVENT_TABLE(FTmodulatorDialog, wxFrame)

	EVT_CLOSE(FTmodulatorDialog::onClose)
	EVT_IDLE(FTmodulatorDialog::OnIdle)
	
	EVT_SIZE (FTmodulatorDialog::onSize)
	EVT_PAINT (FTmodulatorDialog::onPaint)

	EVT_BUTTON (ID_ClearButton, FTmodulatorDialog::onClearButton)

	EVT_COMMAND_RANGE(ID_AddModulatorChannelBase, ID_AddModulatorChannelMax, wxEVT_COMMAND_BUTTON_CLICKED, FTmodulatorDialog::onAddButton)
	
END_EVENT_TABLE()


class FTaddmodObject : public wxObject
{
  public:
	FTaddmodObject(int ind) : index(ind) {}
	
	int index;
};

	
FTmodulatorDialog::FTmodulatorDialog(FTmainwin * parent, wxWindowID id,
				     const wxString & title,
				     const wxPoint& pos,
				     const wxSize& size,
				     long style,
				     const wxString& name )

	: wxFrame(parent, id, title, pos, size, style, name),
	 _clickedChannel(-1), _mainwin(parent)
{

	init();
}

FTmodulatorDialog::~FTmodulatorDialog()
{
}


void FTmodulatorDialog::onSize(wxSizeEvent &ev)
{

	_justResized = true;
	ev.Skip();
}

void FTmodulatorDialog::onPaint(wxPaintEvent &ev)
{
	if (_justResized) {
		//int width,height;

		_justResized = false;

// 		for (int i=0; i < _channelCount; ++i)
// 		{
// 			_channelLists[i]->GetClientSize(&width, &height);
// 			_channelLists[i]->SetColumnWidth(0, width);
// 		}
		
	}
	ev.Skip();
}

void FTmodulatorDialog::init()
{
	wxBoxSizer * mainsizer = new wxBoxSizer(wxVERTICAL);
	//wxStaticText * statText;

	_channelSizer = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer * rowsizer = new wxBoxSizer(wxHORIZONTAL);
	
	wxButton *addButt = new wxButton(this, ID_AddModulatorChannelBase, wxT("Add Modulator..."));
	rowsizer->Add (addButt, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 4);

	rowsizer->Add (2,1,1);
	
	wxButton *clearButt = new wxButton(this, ID_ClearButton, wxT("Remove All"));
	rowsizer->Add (clearButt, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 4);

	mainsizer->Add (rowsizer, 0, wxALL|wxEXPAND);

	_channelScroller = new wxScrolledWindow(this, -1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER);
	_channelSizer = new wxBoxSizer(wxVERTICAL);
	
	_channelScroller->SetSizer(_channelSizer);
	_channelScroller->SetAutoLayout(true);

	_channelScroller->SetScrollRate (10, 30);
	_channelScroller->EnableScrolling (true, true);
	

	mainsizer->Add (_channelScroller, 1, wxEXPAND|wxALL, 3);
	
	wxMenuItem * item;
	_popupMenu = new wxMenu();

	int itemid = ID_AddModulatorBase;

	FTmodulatorManager::ModuleList mlist;
	FTmodulatorManager::instance()->getAvailableModules(mlist);

	int modnum = 0;
	for (FTmodulatorManager::ModuleList::iterator moditer = mlist.begin(); moditer != mlist.end(); ++moditer)
	{
		item = new wxMenuItem(_popupMenu, itemid, wxString::FromAscii ((*moditer)->getName().c_str()));
		_popupMenu->Append (item);
		
		Connect( itemid,  wxEVT_COMMAND_MENU_SELECTED,
				     (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
				     &FTmodulatorDialog::onAddModulator,
				     (wxObject *) (new FTaddmodObject(modnum++)));

		itemid++;
	}



	refreshState();
	
	SetAutoLayout( TRUE );
	mainsizer->Fit( this );  
	mainsizer->SetSizeHints( this );  
	SetSizer( mainsizer );

	// this->SetSizeHints(200,100);

}


void FTmodulatorDialog::refreshState()
{
	// first time only
	
	FTioSupport * iosup = FTioSupport::instance();

	FTprocessPath * procpath;
	for (int i=0; i < iosup->getActivePathCount(); ++i)
	{
	
		procpath = iosup->getProcessPath(i);
		if (procpath) {
			
			FTspectralEngine *engine = procpath->getSpectralEngine();

			engine->ModulatorAdded.connect( bind (slot (*this, &FTmodulatorDialog::onModulatorAdded), i));
			
 			vector<FTmodulatorI*> modlist;
			modlist.clear();
 			engine->getModulators (modlist);

 			for (vector<FTmodulatorI*>::iterator iter=modlist.begin(); iter != modlist.end(); ++iter)
 			{
 				FTmodulatorI * mod = (*iter);

				appendModGui (mod, false);
 			}

		}
	}

	_channelScroller->Layout();
	_channelScroller->SetScrollRate(10,30);
	

// 	if (_lastSelected >= 0 && _lastSelected < _targetList->GetItemCount()) {
// 		_targetList->SetItemState (_lastSelected, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
// 	}
	
}


void FTmodulatorDialog::appendModGui(FTmodulatorI * mod, bool layout)
{

	FTmodulatorGui *modgui = new FTmodulatorGui(FTioSupport::instance(), mod, _channelScroller, -1);
	
	modgui->RemovalRequest.connect (bind (slot (*this, &FTmodulatorDialog::onModulatorDeath), mod));
	mod->GoingAway.connect ( slot (*this, &FTmodulatorDialog::onModulatorDeath));
	
	_modulatorGuis[mod] = modgui;
	
	_channelSizer->Add(modgui, 0, wxEXPAND|wxALL, 1);
	
	if (layout) {
		_channelScroller->SetClientSize(_channelScroller->GetClientSize());
		_channelScroller->Layout();
		// _channelScroller->SetScrollbars(1,1,10,30);
	}

	
}


void FTmodulatorDialog::onClose(wxCloseEvent & ev)
{

	if (!ev.CanVeto()) {

		Destroy();
	}
	else {
		ev.Veto();

		Show(false);
	}
}

void FTmodulatorDialog::onClearButton (wxCommandEvent &ev)
{

	// remove all modulators
	for (int i=0; i < FTioSupport::instance()->getActivePathCount(); i++) {
		FTprocessPath * procpath = FTioSupport::instance()->getProcessPath(i);
		if (procpath) {
			FTspectralEngine *engine = procpath->getSpectralEngine();
			
			engine->clearModulators ();
		}
	}
}

void FTmodulatorDialog::onAddButton (wxCommandEvent &ev)
{
	wxWindow * source = (wxWindow *) ev.GetEventObject();

	_clickedChannel = ev.GetId() - ID_AddModulatorChannelBase;
	
	
	wxRect pos = source->GetRect();
	PopupMenu(_popupMenu, pos.x, pos.y + pos.height);
				
}

void FTmodulatorDialog::onAddModulator (wxCommandEvent &ev)
{
	FTaddmodObject * mobj = (FTaddmodObject *)ev.m_callbackUserData;
	if (!mobj) return;
	
	FTmodulatorI * protomod = FTmodulatorManager::instance()->getModuleByIndex(mobj->index);

	//cerr << "add modulator: " << (unsigned) protomod << " clicked chan: " << _clickedChannel <<  "  " << (unsigned) this << endl;
	
	if (protomod)
	{
		FTmodulatorI * mod = protomod->clone();
		mod->initialize();

		appendModGui (mod, true);
		
		// you can change its channel later
		
		FTprocessPath * procpath = FTioSupport::instance()->getProcessPath(0);
		if (procpath) {
			FTspectralEngine *engine = procpath->getSpectralEngine();
			engine->appendModulator (mod);
			// refreshState();
		}
	}
}


void FTmodulatorDialog::OnIdle(wxIdleEvent &ev)
{

	if (_deadGuis.size() > 0) {

		for (list<FTmodulatorGui*>::iterator iter = _deadGuis.begin(); iter != _deadGuis.end(); ++iter) {
			(*iter)->Destroy();
		}
		
		_deadGuis.clear();
	}

	ev.Skip();
}

void FTmodulatorDialog::onModulatorDeath (FTmodulatorI * mod)
{
	//cerr << "mod death: " << mod->getName() << endl;

	if (_modulatorGuis.find (mod) != _modulatorGuis.end())
	{
		//cerr << "deleting modgui" << endl;
		FTmodulatorGui * modgui = _modulatorGuis[mod];
		_channelSizer->Remove (modgui);
		modgui->Show(false);
		_channelScroller->SetClientSize(_channelScroller->GetClientSize());
		_channelScroller->Layout();
		//_channelScroller->SetScrollbars(1,1,10,30);

		_deadGuis.push_back(modgui);
		_modulatorGuis.erase(mod);

		::wxWakeUpIdle();
	}
}

void FTmodulatorDialog::onModulatorAdded (FTmodulatorI * mod, int channel)
{
	if (_modulatorGuis.find(mod) == _modulatorGuis.end())
	{
		FTprocessPath * procpath = FTioSupport::instance()->getProcessPath (channel);
		if (procpath) {
			// FTspectralEngine *engine = procpath->getSpectralEngine();

			appendModGui (mod, true);
			
		}
	}
}
