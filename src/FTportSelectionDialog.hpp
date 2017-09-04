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

#ifndef __FTPOERTSELECTIONDIALOG_HPP__
#define __FTPOERTSELECTIONDIALOG_HPP__


#include <wx/wx.h>
#include <vector>

class FTportSelectionDialog
   : public wxDialog
{

  public:

    enum PortType
    {
       OUTPUT = 0,
       INPUT
    };
    
    FTportSelectionDialog(wxWindow * parent, wxWindowID id, int pathIndex, PortType ptype, const wxString & title,
			  const wxPoint& pos = wxDefaultPosition,
			  const wxSize& size = wxSize(400,600),
			  long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
			  const wxString& name = wxT("PortSelectionDialog"));


    std::vector<wxString> getSelectedPorts();

    void update();
    
  protected:

    void init();
    void OnOK(wxCommandEvent &event);
    void OnDeselectAll(wxCommandEvent &event);
    
    int _pathIndex;
    PortType _portType;

    wxListBox * _listBox;
    
    wxStringList _selectedPorts;

  private:
	// any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()

};




#endif
