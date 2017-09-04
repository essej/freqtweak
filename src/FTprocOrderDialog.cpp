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

#include <wx/wx.h>
#include <wx/listctrl.h>

#include "FTprocOrderDialog.hpp"
#include "FTioSupport.hpp"
#include "FTmainwin.hpp"
#include "FTdspManager.hpp"
#include "FTprocI.hpp"
#include "FTprocessPath.hpp"
#include "FTspectralEngine.hpp"

enum {
	ID_AddButton=8000,
	ID_UpButton,
	ID_DownButton,
	ID_RemoveButton,
	ID_CommitButton,
	ID_CloseButton,

	ID_SourceList,
	ID_TargetList,
	ID_AutoCheck
};


BEGIN_EVENT_TABLE(FTprocOrderDialog, wxFrame)
	EVT_CLOSE(FTprocOrderDialog::onClose)

	EVT_BUTTON(ID_UpButton, FTprocOrderDialog::onTargetButtons)
	EVT_BUTTON(ID_DownButton, FTprocOrderDialog::onTargetButtons)
	EVT_BUTTON(ID_RemoveButton, FTprocOrderDialog::onTargetButtons)

	EVT_BUTTON(ID_CommitButton, FTprocOrderDialog::onCommit)

	EVT_BUTTON(ID_AddButton, FTprocOrderDialog::onAddButton)
	EVT_CHECKBOX(ID_AutoCheck, FTprocOrderDialog::onAutoCheck)
	
	EVT_SIZE (FTprocOrderDialog::onSize)
	EVT_PAINT (FTprocOrderDialog::onPaint)

	
	
END_EVENT_TABLE()


FTprocOrderDialog::FTprocOrderDialog(FTmainwin * parent, wxWindowID id,
				     const wxString & title,
				     const wxPoint& pos,
				     const wxSize& size,
				     long style,
				     const wxString& name )

	: wxFrame(parent, id, title, pos, size, style, name),
	  _mainwin(parent)
{

	init();
}

FTprocOrderDialog::~FTprocOrderDialog()
{
}


void FTprocOrderDialog::onSize(wxSizeEvent &ev)
{

	_justResized = true;
	ev.Skip();
}

void FTprocOrderDialog::onPaint(wxPaintEvent &ev)
{
	if (_justResized) {
		int width,height;

		_justResized = false;
		
		_sourceList->GetClientSize(&width, &height);
		_sourceList->SetColumnWidth(0, width);
		
		_targetList->GetClientSize(&width, &height);
		_targetList->SetColumnWidth(0, width);
	}

	ev.Skip();
}

void FTprocOrderDialog::init()
{
	wxBoxSizer * mainsizer = new wxBoxSizer(wxHORIZONTAL);


	wxBoxSizer * sourceSizer = new wxBoxSizer(wxVERTICAL);

	_sourceList = new wxListCtrl (this, ID_SourceList, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxSUNKEN_BORDER);
	_sourceList->InsertColumn(0, wxT("Available Modules"));
	
	sourceSizer->Add (_sourceList, 1, wxEXPAND|wxALL, 2);

	mainsizer->Add (sourceSizer, 1, wxEXPAND|wxALL, 6);
	

	
	wxBoxSizer *midbuttSizer = new wxBoxSizer(wxVERTICAL);
	wxButton * butt;

	midbuttSizer->Add (-1, 20);
	
	butt = new wxButton(this, ID_AddButton, wxT("Add ->"), wxDefaultPosition, wxSize(-1,-1));
	midbuttSizer->Add(butt, 0, wxEXPAND|wxALL, 2);
	
	butt = new wxButton(this, ID_RemoveButton, wxT("Remove"), wxDefaultPosition, wxSize(-1,-1));
	midbuttSizer->Add(butt, 0, wxEXPAND|wxALL, 2);

	midbuttSizer->Add (-1, 15);
	
	butt = new wxButton(this, ID_UpButton, wxT("Up"), wxDefaultPosition, wxSize(-1,-1));
	midbuttSizer->Add(butt, 0, wxEXPAND|wxALL, 2);
	butt = new wxButton(this, ID_DownButton, wxT("Down"), wxDefaultPosition, wxSize(-1, -1));
	midbuttSizer->Add(butt, 0, wxEXPAND|wxALL, 2);

	midbuttSizer->Add (-1, 5, 1);

	_modifiedText = new wxStaticText (this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	midbuttSizer->Add(_modifiedText, 0, wxALL|wxEXPAND|wxALIGN_CENTRE_VERTICAL|wxALIGN_CENTRE, 3);
	
	//butt =  new wxButton(this, ID_CloseButton, "Close");
	//midbuttSizer->Add(butt, 1, wxALL, 2);

	
	mainsizer->Add (midbuttSizer, 0, wxEXPAND|wxALL, 4);
	
	
	wxBoxSizer * targSizer = new wxBoxSizer(wxVERTICAL);

	_targetList = new wxListCtrl (this, ID_TargetList, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxSUNKEN_BORDER|wxLC_SINGLE_SEL);
	_targetList->InsertColumn(0, wxT("Configured Modules"));

	targSizer->Add (_targetList, 1, wxEXPAND|wxALL, 2);
	

	// button bar
	wxBoxSizer * buttSizer = new wxBoxSizer(wxHORIZONTAL);

	_autoCheck = new wxCheckBox(this, ID_AutoCheck, wxT("Auto"));
	buttSizer->Add( _autoCheck, 0, wxALL, 2);

	butt =  new wxButton(this, ID_CommitButton, wxT("Commit"));
	buttSizer->Add(butt, 1, wxALL, 2);
	
	targSizer->Add(buttSizer, 0, wxALL|wxEXPAND, 0);


	mainsizer->Add(targSizer, 1, wxALL|wxEXPAND, 6);


	refreshState();
	
	SetAutoLayout( TRUE );
	mainsizer->Fit( this );  
	mainsizer->SetSizeHints( this );  
	SetSizer( mainsizer );

	this->SetSizeHints(200,100);

}


void FTprocOrderDialog::refreshState()
{

	_sourceList->DeleteAllItems();
	wxListItem item;
	item.SetColumn(0);
	item.SetMask (wxLIST_MASK_TEXT|wxLIST_MASK_DATA);
	
	
	// get available modules

	FTdspManager::ModuleList mlist;
	FTdspManager::instance()->getAvailableModules (mlist);
	FTdspManager::ModuleList::iterator mod = mlist.begin();

	unsigned int pos = 0;
	
	for (; mod != mlist.end(); ++mod)
	{
		item.SetText (wxString::FromAscii ((*mod)->getName().c_str()));
		item.SetData ((unsigned) (*mod));
		item.SetId(pos++);
		
		_sourceList->InsertItem(item);
	}


	// get configured modules from the first procpath
	_targetList->DeleteAllItems();
	
	FTprocessPath * procpath = FTioSupport::instance()->getProcessPath(0);
	if (procpath) {

		FTspectralEngine *engine = procpath->getSpectralEngine();
		vector<FTprocI *> procmods;
		engine->getProcessorModules (procmods);
		
		for (unsigned int n=0; n < procmods.size(); ++n)
		{
			item.SetText (wxString::FromAscii (procmods[n]->getName().c_str()));
			item.SetData ((unsigned) procmods[n]);
			item.SetId (n);
			
			_targetList->InsertItem(item);
		}
	}

	if (_lastSelected >= 0 && _lastSelected < _targetList->GetItemCount()) {
		_targetList->SetItemState (_lastSelected, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
	
	_actions.clear();
	_modifiedText->SetLabel (wxT(""));
}


void FTprocOrderDialog::onClose(wxCloseEvent & ev)
{

	if (!ev.CanVeto()) {

		Destroy();
	}
	else {
		ev.Veto();

		Show(false);
	}
}


void FTprocOrderDialog::onAutoCheck (wxCommandEvent &ev)
{
	if (_autoCheck->GetValue() == true) {
		// commit immediately
		onCommit(ev);
	}
}

void FTprocOrderDialog::onCommit(wxCommandEvent & ev)
{
	//_mainwin->suspendProcessing();

	FTioSupport * iosup = FTioSupport::instance();
	// do this for every active process path

	
	FTprocessPath * procpath;
	for (int i=0; i < iosup->getActivePathCount(); ++i)
	{
		procpath = iosup->getProcessPath(i);
		if (!procpath) break;
		
		FTspectralEngine *engine = procpath->getSpectralEngine();

		// go through the actions

		for (list<ModAction>::iterator action = _actions.begin(); action != _actions.end(); ++action)
		{
			ModAction & act = *action;

			if (act.remove) {
				engine->removeProcessorModule ((unsigned int) act.from); 
			}
			else if (act.from < 0) {
				// no from, this is an addition
				FTprocI * newproc = act.procmod->clone();
				newproc->initialize();
				
				engine->appendProcessorModule (newproc);
			}
			else {
				// this is a move
				engine->moveProcessorModule ((unsigned int) act.from, (unsigned int) act.to);
			}
		}
	}

	_actions.clear();
	
	// rebuild UI parts
	_mainwin->rebuildDisplay(false);

	// _mainwin->restoreProcessing();

	refreshState();
}


void FTprocOrderDialog::onTargetButtons(wxCommandEvent & ev)
{
	int id = ev.GetId();
	int itemi;
	
	if (id == ID_RemoveButton)
	{
		itemi = _targetList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (itemi != -1) {
			_targetList->DeleteItem(itemi);
			
			_actions.push_back (ModAction ((FTprocI*) _targetList->GetItemData(itemi),
						       itemi , -1, true));

			if (itemi >= _targetList->GetItemCount()) itemi -= 1;
			_targetList->SetItemState (itemi, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

			_lastSelected = itemi;
			
			if (_autoCheck->GetValue()) {
				// commit immediately
				onCommit(ev);
			}
			else {
				_modifiedText->SetLabel (wxT("* modified *"));
			}
		}
	}
	else if (id == ID_UpButton)
	{
		itemi = _targetList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (itemi > 0) {
			// swap with item above
			wxListItem item;
			item.SetId(itemi);
			item.SetMask(wxLIST_MASK_STATE|wxLIST_MASK_TEXT|wxLIST_MASK_DATA);
			_targetList->GetItem (item);

			_targetList->DeleteItem(itemi);

			item.SetId(itemi-1);
			_targetList->InsertItem(item); 
			_targetList->SetItemState (itemi-1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
			
			_actions.push_back (ModAction ((FTprocI*) item.GetData(),
						       itemi , itemi-1, false));

			_lastSelected = itemi-1;

			if (_autoCheck->GetValue()) {
				// commit immediately
				onCommit(ev);
			}
			else {
				_modifiedText->SetLabel (wxT("* modified *"));
			}
		}
	}
	else if (id == ID_DownButton)
	{
		itemi = _targetList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (itemi != -1 && itemi < _targetList->GetItemCount()-1) {
			// swap with item below
			wxListItem item;
			item.SetId(itemi);
			item.SetMask(wxLIST_MASK_STATE|wxLIST_MASK_TEXT|wxLIST_MASK_DATA);
			_targetList->GetItem (item);

			_targetList->DeleteItem(itemi);

			item.SetId(itemi+1);
			_targetList->InsertItem(item);
			_targetList->SetItemState (itemi+1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

			
			_actions.push_back (ModAction ((FTprocI*) item.GetData(),
						       itemi , itemi+1, false));

			_lastSelected = itemi+1;
			
			if (_autoCheck->GetValue()) {
				// commit immediately
				onCommit(ev);
			}
			else {
				_modifiedText->SetLabel (wxT("* modified *"));
			}

		}

	}
	else {

		ev.Skip();
	}
}


void FTprocOrderDialog::onAddButton(wxCommandEvent & ev)
{
	// append selected procmods from source

	wxListItem item;
	item.SetColumn(0);
	item.SetMask (wxLIST_MASK_TEXT|wxLIST_MASK_DATA);
	
	
	long itemi = -1;
	bool didsomething = false;

	for ( ;; )
	{
		itemi = _sourceList->GetNextItem(itemi,
					     wxLIST_NEXT_ALL,
					     wxLIST_STATE_SELECTED);
		if ( itemi == -1 )
			break;

		FTprocI * proc = (FTprocI *) _sourceList->GetItemData(itemi); 

		if (proc) {
			item.SetText (wxString::FromAscii (proc->getName().c_str()));
			item.SetData ((unsigned)proc);
			item.SetId (_targetList->GetItemCount());
			
			_targetList->InsertItem(item);

			_actions.push_back (ModAction (proc, -1 , _targetList->GetItemCount(), false));

			didsomething = true;
		}
	}	

	if (didsomething) {
		if (_autoCheck->GetValue()) {
			onCommit(ev);
		}
		else {
			_modifiedText->SetLabel (wxT("* modified *"));
		}
	}
}

