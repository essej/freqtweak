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

#include "FTportSelectionDialog.hpp"
#include "FTjackSupport.hpp"


#define DESELECTALLID 3241

BEGIN_EVENT_TABLE(FTportSelectionDialog, wxDialog)
	EVT_BUTTON(wxID_OK, FTportSelectionDialog::OnOK)
	EVT_BUTTON(DESELECTALLID, FTportSelectionDialog::OnDeselectAll)
	
END_EVENT_TABLE()


FTportSelectionDialog::FTportSelectionDialog(wxWindow * parent, wxWindowID id, int pathIndex, PortType ptype,
					     const wxString & title,
					     const wxPoint& pos,
					     const wxSize& size,
					     long style,
					     const wxString& name )

	: wxDialog(parent, id, title, pos, size, style, name)
	, _pathIndex(pathIndex), _portType(ptype)
{

	init();
}


void FTportSelectionDialog::init()
{
	// FTjackSupport * iosup = FTjackSupport::instance();
	
	wxBoxSizer * mainsizer = new wxBoxSizer(wxVERTICAL);
	_listBox = new wxListBox(this, wxNewId(), wxDefaultPosition, wxDefaultSize,
					    0, NULL, wxLB_MULTIPLE);


	
	wxButton * deselbutt = new wxButton(this, DESELECTALLID, wxT("Deselect All"));
	mainsizer->Add(deselbutt, 0, wxALL, 3);

	
	mainsizer->Add(_listBox, 1, wxALL|wxEXPAND, 3);

	// button bar
	wxBoxSizer * buttSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *okButton =  new wxButton(this, wxID_OK, wxT("OK"));
	wxButton *cancButton =  new wxButton(this, wxID_CANCEL, wxT("Cancel"));

	buttSizer->Add(okButton, 0, wxRIGHT, 10);
	buttSizer->Add(cancButton, 0, wxLEFT, 10);

	mainsizer->Add(buttSizer, 0, wxALL|wxEXPAND, 4);

	SetAutoLayout( TRUE );
	mainsizer->Fit( this );  
	mainsizer->SetSizeHints( this );  
	SetSizer( mainsizer );
		
}


void FTportSelectionDialog::update()
{
	// get ports from jack
	const char ** availports = 0;
	const char ** connports = 0;
	const char * noportname = 0;
	FTioSupport * iosup = FTioSupport::instance();
	
	if (_portType == INPUT) {
		availports = iosup->getInputConnectablePorts(_pathIndex);
		connports = iosup->getConnectedInputPorts(_pathIndex);
		noportname = iosup->getOutputPortName(_pathIndex);
	}
	else if (_portType == OUTPUT) {
		availports = iosup->getOutputConnectablePorts(_pathIndex);
		connports = iosup->getConnectedOutputPorts(_pathIndex);
		noportname = iosup->getInputPortName(_pathIndex);
	}

	_listBox->Clear();
	
	if (availports) {
		for (int i=0; availports[i]; i++) {
			if (noportname && wxString::FromAscii(availports[i]).Cmp(wxString::FromAscii (noportname)) != 0) {
				_listBox->Append (wxString::FromAscii(availports[i]), (void *) 0);
			}
		}
		free (availports);

		for (int i=0; i < _listBox->GetCount(); i++) {
			_listBox->Deselect(i);
		}
		if (connports) {
			for (int i=0; connports[i]; i++) {
				int n = _listBox->FindString(wxString::FromAscii (connports[i]));
				//printf ("connport = %s  find is %d\n", connports[i], n);
				if (n >= 0)
				   _listBox->SetSelection (n, TRUE);
			}
			free(connports);
		}

		
	}

}

std::vector<wxString> FTportSelectionDialog::getSelectedPorts()
{
	int n = _selectedPorts.GetCount();
	std::vector<wxString> pnames;

	for (int i=0; i < n; i++) {
		pnames.push_back(wxString(_selectedPorts.Item(i)->GetData()));
	}

	return pnames;
}


void FTportSelectionDialog::OnDeselectAll(wxCommandEvent &event)
{
	for (int i=0; i < _listBox->GetCount(); i++) {
		_listBox->Deselect(i);
	}
}


void FTportSelectionDialog::OnOK(wxCommandEvent &event)
{
	// get selection from listbox

	wxArrayInt selarr;

	int n = _listBox->GetSelections(selarr);

	_selectedPorts.Clear();
	
	for (int i=0; i < n; i++) {
		_selectedPorts.Add(_listBox->GetString(selarr[i]));
	}

	if (IsModal()) {
		EndModal(wxID_OK);
	}
	else {
		SetReturnCode(wxID_OK);
		Show(0);
	}	
}
