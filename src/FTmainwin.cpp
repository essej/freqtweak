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


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWindows headers
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/sashwin.h>
#include <wx/spinctrl.h>
#include <wx/notebook.h>
#include <wx/string.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string>
using namespace std;

#include "FTmainwin.hpp"
#include "FTspectragram.hpp"
#include "FTioSupport.hpp"
#include "FTprocessPath.hpp"
#include "FTprocI.hpp"
#include "FTspectralEngine.hpp"
#include "FTactiveBarGraph.hpp"
#include "FTportSelectionDialog.hpp"
#include "FTspectrumModifier.hpp"
#include "FTconfigManager.hpp"
#include "FTupdateToken.hpp"
#include "FTprocOrderDialog.hpp"
#include "FTpresetBlendDialog.hpp"
#include "FTmodulatorDialog.hpp"
#include "FThelpWindow.hpp"

#include "pix_button.hpp"

using namespace JLCui;

#include "version.h"

#include "ftlogo.xpm"

//#include "images/bypass.xpm"
//#include "images/bypass_active.xpm"
//#include "images/link.xpm"
//#include "images/link_active.xpm"

#include "pixmap_includes.hpp"


// ----------------------------------------------------------------------------
// event tables and other macros for wxWindows
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum WindowIds
{
	// menu items
	FT_QuitMenu = 1,
	FT_AboutMenu,
	FT_ProcModMenu,
	FT_PresetBlendMenu,
	FT_ModulatorMenu,
	FT_HelpTipsMenu,
	FT_InputButtonId,
	FT_OutputButtonId,
	FT_InSpecTypeId,
	FT_OutSpecTypeId,

	FT_BypassBase = 100,


	FT_InSpecLabelId = 130,
	FT_OutSpecLabelId,

	FT_LabelBase = 150,

	FT_LinkBase = 180,

	FT_FreqBinsChoiceId = 210,
	FT_OverlapChoiceId,
	FT_WindowingChoiceId,
	FT_TimescaleSliderId,
	FT_FreqScaleChoiceId,

	FT_RowPanelId,
	FT_RowPanel,
	FT_BypassId,
	FT_MuteId,

	FT_GainSlider,
	FT_GainSpin,
	FT_MixSlider,
	FT_PathCountChoice,

	FT_StoreButton,
	FT_LoadButton,
	FT_PresetCombo,
	FT_PlotSpeedChoiceId,
	FT_PlotSuperSmoothId,
	
	FT_MixLinkedButton,
	FT_IOreconnectButton,
	FT_IOdisconnectButton,
	FT_IOnameText,

	FT_GridBase = 500,

	FT_GridSnapBase = 530,

	FT_MaxDelayChoiceId = 560,
	FT_TempoSpinId,
	FT_RestorePortCheckId
};


DEFINE_LOCAL_EVENT_TYPE (FT_EVT_TITLEMENU_COMMAND);

IMPLEMENT_DYNAMIC_CLASS(FTtitleMenuEvent, wxCommandEvent);


// the event tables connect the wxWindows events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(FTmainwin, wxFrame)
//	EVT_SIZE(FTmainwin::OnSize)
	EVT_MENU(FT_QuitMenu,  FTmainwin::OnQuit)
	EVT_MENU(FT_AboutMenu, FTmainwin::OnAbout)
	EVT_MENU(FT_HelpTipsMenu, FTmainwin::OnAbout)
	EVT_MENU(FT_ProcModMenu, FTmainwin::OnProcMod)
	EVT_MENU(FT_PresetBlendMenu, FTmainwin::OnPresetBlend)
	EVT_MENU(FT_ModulatorMenu, FTmainwin::OnModulatorDialog)

	
	EVT_IDLE(FTmainwin::OnIdle)

	EVT_CLOSE(FTmainwin::OnClose)
	
	EVT_BUTTON(FT_InputButtonId, FTmainwin::handleInputButton)
	EVT_BUTTON(FT_OutputButtonId, FTmainwin::handleOutputButton)

	EVT_BUTTON(FT_LinkBase, FTmainwin::handleLinkButtons)
	EVT_BUTTON(FT_BypassBase, FTmainwin::handleBypassButtons)

	EVT_CHOICE(FT_FreqBinsChoiceId, FTmainwin::handleChoices)
	EVT_CHOICE(FT_OverlapChoiceId, FTmainwin::handleChoices)
	EVT_CHOICE(FT_WindowingChoiceId, FTmainwin::handleChoices)
	EVT_CHOICE(FT_PlotSpeedChoiceId, FTmainwin::handleChoices)
	EVT_CHECKBOX(FT_PlotSuperSmoothId, FTmainwin::handleChoices)
	EVT_CHOICE(FT_MaxDelayChoiceId, FTmainwin::handleChoices)
	
	
	EVT_CHOICE(FT_PathCountChoice, FTmainwin::handlePathCount)

	
	EVT_COMMAND_SCROLL(FT_MixSlider, FTmainwin::handleMixSlider)
	EVT_SPINCTRL(FT_GainSpin, FTmainwin::handleGain)
	
	EVT_SASH_DRAGGED(FT_RowPanelId, FTmainwin::handleSashDragged)

	EVT_BUTTON(FT_InSpecLabelId, FTmainwin::handleLabelButtons)
	EVT_BUTTON(FT_LabelBase, FTmainwin::handleLabelButtons)
	EVT_BUTTON(FT_OutSpecLabelId, FTmainwin::handleLabelButtons)

	EVT_BUTTON(FT_InSpecTypeId, FTmainwin::handlePlotTypeButtons)
	EVT_BUTTON(FT_OutSpecTypeId, FTmainwin::handlePlotTypeButtons)

	EVT_CHECKBOX(FT_BypassId, FTmainwin::handleBypassButtons)
	EVT_CHECKBOX(FT_MuteId, FTmainwin::handleBypassButtons)

	EVT_BUTTON(FT_StoreButton, FTmainwin::handleStoreButton)
	EVT_BUTTON(FT_LoadButton, FTmainwin::handleLoadButton)

	EVT_CHECKBOX(FT_MixLinkedButton, FTmainwin::handleLinkButtons)
	EVT_BUTTON(FT_IOreconnectButton, FTmainwin::handleIOButtons)
	EVT_BUTTON(FT_IOdisconnectButton, FTmainwin::handleIOButtons)

	EVT_BUTTON(FT_GridBase, FTmainwin::handleGridButtons)

	EVT_BUTTON(FT_GridSnapBase, FTmainwin::handleGridButtons)

	EVT_SPINCTRL(FT_TempoSpinId, FTmainwin::handleSpins)

	EVT_TITLEMENU_COMMAND (0, FTmainwin::handleTitleMenuCmd)
END_EVENT_TABLE()


	

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

FTmainwin::FTmainwin(int startpath, const wxString& title, const wxString& rcdir, const wxPoint& pos, const wxSize& size)
	: wxFrame((wxFrame *)NULL, -1, title, pos, size), _startpaths(startpath),
	  _inspecShown(true), _outspecShown(true), _linkedMix(true),

	  _updateMS(10), _superSmooth(false), _refreshMS(200),
	  _pathCount(startpath),
	  _configManager(static_cast<const char *> (rcdir.fn_str())),
	  _procmodDialog(0), _blendDialog(0), _modulatorDialog(0),
	  _titleFont(10, wxDEFAULT, wxNORMAL, wxBOLD),
	  _titleAltFont(10, wxDEFAULT, wxSLANT, wxBOLD),
	  _buttFont(10, wxDEFAULT, wxNORMAL, wxNORMAL)
	  
{
	_eventTimer = new FTupdateTimer(this);
	_refreshTimer = new FTrefreshTimer(this);

	
	for (int i=0; i < FT_MAXPATHS; i++) {
		_processPath[i] = 0;
		_updateTokens[i] = new FTupdateToken();
	}
	
	buildGui();

}

void FTmainwin::normalizeFontSize(wxFont & fnt, int height, wxString fitstr)
{
	int fontw, fonth, desch, lead, lasth=0;
	
	GetTextExtent(fitstr, &fontw, &fonth, &desch, &lead, &fnt);
	//printf ("Text extent for %s: %d %d %d %d   sz: %d\n", fitstr.c_str(), fontw, desch, lead, fonth, fnt.GetPointSize());
	while (fonth > height ) {
		fnt.SetPointSize(fnt.GetPointSize() - 1);
		GetTextExtent(fitstr, &fontw, &fonth, &desch, &lead, &fnt);
		//printf ("Text extent for buttfont: %d %d %d %d sz: %d\n", fontw, fonth, desch, lead, fnt.GetPointSize());

		if (lasth == fonth) break; // safety
		lasth = fonth;
	}

}


void FTmainwin::buildGui()
{
	_bwidth = 22;
	_labwidth = 74;
	_bheight = 24;
	_rowh = 68;


	normalizeFontSize(_buttFont, _bheight-8, wxT("BA"));
	normalizeFontSize(_titleFont, _bheight-8, wxT("EQ"));
	normalizeFontSize(_titleAltFont, _bheight-8, wxT("EQ"));
	
	// set the frame icon
	//SetIcon(wxICON(mondrian));
	
	// create a menu bar
	wxMenu *menuFile = new wxMenu(wxT(""), wxMENU_TEAROFF);

	menuFile->Append(FT_ProcModMenu, wxT("&DSP Modules...\tCtrl-P"), wxT("Configure DSP modules"));
	menuFile->Append(FT_ModulatorMenu, wxT("&Modulators...\tCtrl-M"), wxT("Configure Modulations"));
	menuFile->Append(FT_PresetBlendMenu, wxT("Preset &Blend...\tCtrl-B"), wxT("Blend multiple presets"));

	menuFile->AppendSeparator();	
	menuFile->Append(FT_QuitMenu, wxT("&Quit\tCtrl-Q"), wxT("Quit this program"));

	// now append the freshly created menu to the menu bar...
	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(menuFile, wxT("&Control"));

	wxMenu *menuHelp = new wxMenu(wxT(""), wxMENU_TEAROFF);
	menuHelp->Append(FT_HelpTipsMenu, wxT("&Usage Tips...\tCtrl-H"), wxT("Show Usage Tips window"));
	menuHelp->Append(FT_AboutMenu, wxT("&About...\tCtrl-A"), wxT("Show about dialog"));
	menuBar->Append(menuHelp, wxT("&Help"));
	
	// ... and attach this menu bar to the frame
	SetMenuBar(menuBar);
	
#if wxUSE_STATUSBAR
	// create a status bar just for fun (with 3 panes)
	CreateStatusBar(3);
	SetStatusText(wxT("FreqTweak"), 0);
#endif // wxUSE_STATUSBAR

	// terrible!
	_defaultBg = GetBackgroundColour();
	//_activeBg.Set (254,185,55);
	_activeBg.Set (246,229,149);
	
	// get processPaths from jacksupport
	FTioSupport * iosup = FTioSupport::instance();

	for (int i=0; i < _startpaths; i++)
	{
		// add to io support instance
		_processPath[i] = iosup->setProcessPathActive (i, true);
	}	

	wxBoxSizer *mainsizer = new wxBoxSizer(wxVERTICAL);
	
	// overall controls
	wxNotebook * ctrlbook = new wxNotebook(this,-1, wxDefaultPosition, wxSize(-1, 70));

	// preset panel
	wxPanel * configpanel = new wxPanel (ctrlbook, -1);
	wxBoxSizer *configSizer = new wxBoxSizer(wxHORIZONTAL);

	_presetCombo = new wxComboBox (configpanel, FT_PresetCombo, wxT(""),  wxDefaultPosition, wxSize(200,-1), 0, 0,  wxCB_SORT);
	configSizer->Add( _presetCombo, 0, wxALL|wxALIGN_CENTER, 2);

	
	wxButton *storeButt = new wxButton(configpanel, FT_StoreButton, wxT("Store"));
	configSizer->Add( storeButt, 0, wxALL|wxALIGN_CENTER, 2);

	wxButton *loadButt = new wxButton(configpanel, FT_LoadButton, wxT("Load"));
	configSizer->Add( loadButt, 0, wxALL|wxALIGN_CENTER, 2);

	_restorePortsCheck = new wxCheckBox(configpanel, FT_RestorePortCheckId, wxT("Restore Ports"));
	configSizer->Add( _restorePortsCheck, 0, wxALL|wxALIGN_CENTER, 2);

	
	configpanel->SetAutoLayout(TRUE);
	configSizer->Fit( configpanel );  
	configpanel->SetSizer(configSizer);

	ctrlbook->AddPage ( (wxNotebookPage *) configpanel, wxT("Presets"), true);
	
	//mainsizer->Add (configSizer, 0, wxALL|wxEXPAND, 1);

	
	wxPanel * fftpanel = new wxPanel(ctrlbook, -1);

	wxStaticText *stattext;
	//wxBoxSizer *specCtrlSizer = new wxStaticBoxSizer(new wxStaticBox(fftpanel, -1, "FFT Params"), wxHORIZONTAL);
	wxBoxSizer *specCtrlSizer = new wxBoxSizer(wxHORIZONTAL);

	stattext =  new wxStaticText(fftpanel, -1, wxT("Freq Bands"));
	specCtrlSizer->Add (stattext, 0, wxALIGN_CENTER|wxLEFT, 4);
	_freqBinsChoice = new wxChoice(fftpanel, FT_FreqBinsChoiceId);
	const int * fftbins = FTspectralEngine::getFFTSizes();
	for (int i=0; i < FTspectralEngine::getFFTSizeCount(); i++) {
		_freqBinsChoice->Append(wxString::Format(wxT("%d"), fftbins[i] / 2), (void *) ((intptr_t)fftbins[i]));
	}

	specCtrlSizer->Add(_freqBinsChoice, 0, wxALL|wxALIGN_CENTER, 2);

	stattext =  new wxStaticText(fftpanel, -1, wxT("Overlap"));
	specCtrlSizer->Add ( stattext, 0, wxALIGN_CENTER|wxLEFT, 4);
	_overlapChoice = new wxChoice(fftpanel, FT_OverlapChoiceId);
	_overlapChoice->Append(wxT("0 %"), (void *) 1);
	_overlapChoice->Append(wxT("50 %"), (void *) 2);
	_overlapChoice->Append(wxT("75 %"), (void *) 4);
	_overlapChoice->Append(wxT("87.5 %"), (void *) 8);
	_overlapChoice->Append(wxT("93.75 %"), (void *) 16);
	_overlapChoice->SetSelection(2); // hack
	specCtrlSizer->Add(_overlapChoice, 0, wxALL|wxALIGN_CENTER, 2);

	stattext =  new wxStaticText(fftpanel, -1, wxT("Window"));
	specCtrlSizer->Add ( stattext, 0, wxALIGN_CENTER|wxLEFT, 4);
	_windowingChoice = new wxChoice(fftpanel, FT_WindowingChoiceId, wxDefaultPosition, wxSize(100,-1));
	const char ** winstrs = FTspectralEngine::getWindowStrings();
	for (int i=0; i < FTspectralEngine::getWindowStringsCount(); i++) {
#if wxUSE_WCHAR_T
	        wchar_t wstr[1024]; // FIXME: no idea how good this size is
                wxMB2WC (wstr, winstrs[i], 1024);
		_windowingChoice->Append(wstr);
#else
		_windowingChoice->Append(winstrs[i]);
#endif
	}
	_windowingChoice->SetSelection(0);
	

	specCtrlSizer->Add(_windowingChoice, 0, wxALL|wxALIGN_CENTER, 2);

	fftpanel->SetAutoLayout(TRUE);
	specCtrlSizer->Fit( fftpanel );  
	fftpanel->SetSizer(specCtrlSizer);

	ctrlbook->AddPage ( (wxNotebookPage *) fftpanel, wxT("FFT"));


	// plot page
	wxPanel * plotpanel = new wxPanel (ctrlbook, -1);
	wxBoxSizer *plotSizer = new wxBoxSizer(wxHORIZONTAL);

	stattext =  new wxStaticText(plotpanel, -1, wxT("Speed"));
	plotSizer->Add ( stattext, 0, wxALIGN_CENTER|wxLEFT, 4);
	_plotSpeedChoice = new wxChoice(plotpanel, FT_PlotSpeedChoiceId, wxDefaultPosition, wxSize(100,-1));
	_plotSpeedChoice->Append(wxT("Turtle"), (void *) FTspectralEngine::SPEED_TURTLE);
	_plotSpeedChoice->Append(wxT("Slow"), (void *) FTspectralEngine::SPEED_SLOW);
	_plotSpeedChoice->Append(wxT("Medium"), (void *) FTspectralEngine::SPEED_MED);
	_plotSpeedChoice->Append(wxT("Fast"), (void *) FTspectralEngine::SPEED_FAST);
	//_plotSpeedChoice->SetSelection(2); // hack
	plotSizer->Add(_plotSpeedChoice, 0, wxALL|wxALIGN_CENTER, 2);

	_superSmoothCheck = new wxCheckBox (plotpanel, FT_PlotSuperSmoothId, wxT("Extra smooth but expensive")); 
	_superSmoothCheck->SetValue(false);
	plotSizer->Add (_superSmoothCheck, 0, wxALL|wxALIGN_CENTER, 2);
	
	plotpanel->SetAutoLayout(TRUE);
	plotSizer->Fit( plotpanel );  
	plotpanel->SetSizer(plotSizer);

	ctrlbook->AddPage ( (wxNotebookPage *) plotpanel, wxT("Plots"), false);
	

	// IO page
	wxPanel * iopanel = new wxPanel (ctrlbook, -1);
	wxBoxSizer *ioSizer = new wxBoxSizer(wxHORIZONTAL);

	stattext =  new wxStaticText(iopanel, -1, wxT("JACK-Name"));
	ioSizer->Add ( stattext, 0, wxALIGN_CENTER|wxLEFT, 4);
	_ioNameText = new wxTextCtrl (iopanel, FT_IOnameText, wxT(""), wxDefaultPosition, wxSize(150,-1));
	ioSizer->Add (_ioNameText, 0, wxALL|wxALIGN_CENTER, 2);
	
	wxButton * reconnButton = new wxButton(iopanel, FT_IOreconnectButton, wxT("Reconnect As"));
	ioSizer->Add (reconnButton, 0, wxALL|wxALIGN_CENTER, 2);

	wxButton * disconnButton = new wxButton(iopanel, FT_IOdisconnectButton, wxT("Disconnect"));
	ioSizer->Add (disconnButton, 0, wxALL|wxALIGN_CENTER, 2);

	iopanel->SetAutoLayout(TRUE);
	ioSizer->Fit( iopanel );  
	iopanel->SetSizer(ioSizer);
	
	ctrlbook->AddPage ( (wxNotebookPage *) iopanel, wxT("I/O"), false);
	

	// time/memory page
	wxPanel * timepanel = new wxPanel (ctrlbook, -1);
	wxBoxSizer *timeSizer = new wxBoxSizer(wxHORIZONTAL);

	stattext =  new wxStaticText(timepanel, -1, wxT("Max Delay Time"));
	timeSizer->Add ( stattext, 0, wxALIGN_CENTER|wxLEFT, 4);
	_maxDelayChoice = new wxChoice(timepanel, FT_MaxDelayChoiceId, wxDefaultPosition, wxSize(100,-1));
	_maxDelayChoice->Append(wxT("0.5 sec"));
	_delayList.push_back (0.5);
	_maxDelayChoice->Append(wxT("1 sec"));
	_delayList.push_back (1.0);
	_maxDelayChoice->Append(wxT("2.5 sec"));
	_delayList.push_back (2.5);
	_maxDelayChoice->Append(wxT("5 sec"));
	_delayList.push_back (5.0);
	_maxDelayChoice->Append(wxT("10 sec"));
	_delayList.push_back (10.0);
	_maxDelayChoice->Append(wxT("20 sec"));
	_delayList.push_back (20.0);
	timeSizer->Add(_maxDelayChoice, 0, wxALL|wxALIGN_CENTER, 2);

	stattext =  new wxStaticText(timepanel, -1, wxT("Tempo"));
	timeSizer->Add ( stattext, 0, wxALIGN_CENTER|wxLEFT, 4);
	
	_tempoSpinCtrl = new wxSpinCtrl(timepanel, FT_TempoSpinId, wxT("120"), wxDefaultPosition, wxSize(65,-1),
						  wxSP_ARROW_KEYS, 1, 300, 120);
	timeSizer->Add(_tempoSpinCtrl, 0, wxALL|wxALIGN_CENTER, 2);
	
	
	timepanel->SetAutoLayout(TRUE);
	timeSizer->Fit( timepanel );  
	timepanel->SetSizer(timeSizer);

	ctrlbook->AddPage ( (wxNotebookPage *) timepanel, wxT("Time") , false);

	wxBoxSizer *booksizer = new wxBoxSizer (wxHORIZONTAL);
	
	booksizer->Add (ctrlbook, 1, wxALL, 0);

	wxStaticBitmap * logobit = new wxStaticBitmap(this, -1, wxBitmap(ftlogo_xpm));
	//wxStaticBitmap * logobit = new wxStaticBitmap(this, -1, wxBitmap(ftlogo1_xpm));
	//wxStaticBitmap * logobit = new wxStaticBitmap(this, -1, wxBitmap(ftlogo2_xpm));

	booksizer->Add (logobit, 0, wxALIGN_BOTTOM);
	
	
	mainsizer->Add ( booksizer, 0, wxALL|wxEXPAND, 2 );




	//_rowPanel = new wxPanel(this, -1);
	_rowPanel = new wxScrolledWindow(this, FT_RowPanel, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL|wxSUNKEN_BORDER);
	_rowPanel->SetBackgroundColour(wxColour(20,20,20));
	_rowPanel->SetThemeEnabled(false);
	
	
	// ASSUME we have at least one spectral engine
	FTspectralEngine * sengine = _processPath[0]->getSpectralEngine();
	vector<FTprocI *> procmods;
	sengine->getProcessorModules (procmods);
	
	wxBoxSizer *mainvsizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer  *tmpsizer, *tmpsizer2;

	_uppersizer = new wxBoxSizer(wxHORIZONTAL);
	_inspecsizer = new wxBoxSizer(wxHORIZONTAL);

	_inspecSash = new wxSashLayoutWindow(_rowPanel, FT_RowPanelId, wxDefaultPosition, wxSize(-1,_rowh));
	_inspecSash->SetBackgroundColour(*wxBLACK);
	_inspecSash->SetThemeEnabled(false);
	_inspecPanel = new wxPanel(_inspecSash, -1);
	_inspecPanel->SetBackgroundColour(*wxBLACK);
	_inspecPanel->SetThemeEnabled(false);
	_inspecSash->SetSashBorder(wxSASH_BOTTOM, true);
	_inspecSash->SetSashVisible(wxSASH_BOTTOM, true);
	_rowItems.push_back( _inspecSash);


	// create master link and bypass buttons
	_inspecLabelButton = new wxButton(_inspecPanel, FT_InSpecLabelId, wxT("In Spectra"),
 					  wxDefaultPosition, wxSize(_labwidth,_bheight));
	_inspecLabelButton->SetBackgroundColour(*wxBLACK);
	_inspecLabelButton->SetForegroundColour(*wxWHITE);
	_inspecLabelButton->SetThemeEnabled(false);
	_inspecLabelButton->SetFont(_titleFont);
	_inspecLabelButton->SetToolTip(wxT("Hide In Spectra"));

	wxLayoutConstraints * constr;
	
	// create alts
	_inspecLabelButtonAlt = new wxButton(_rowPanel, FT_InSpecLabelId, wxT("In Spectra"),
 					  wxDefaultPosition, wxSize(_labwidth,_bheight));
	_inspecLabelButtonAlt->SetBackgroundColour(*wxBLACK);
	_inspecLabelButtonAlt->SetForegroundColour(*wxWHITE);
	_inspecLabelButtonAlt->SetThemeEnabled(false);
	_inspecLabelButtonAlt->SetFont(_titleAltFont);
	_inspecLabelButtonAlt->SetToolTip(wxT("Show In Spectra"));
	_inspecLabelButtonAlt->Show(false);
	constr = new wxLayoutConstraints;
	constr->left.SameAs (_rowPanel, wxLeft, 2);
	constr->width.Absolute (_labwidth);
	constr->top.SameAs (_rowPanel, wxTop);
	constr->height.Absolute (_bheight);
	_inspecLabelButtonAlt->SetConstraints(constr);


	_bypassAllCheck  = new  wxCheckBox(this, FT_BypassId, wxT("Bypass All"),
					     wxDefaultPosition, wxSize(_labwidth,_bheight+3));
	_bypassAllCheck->SetFont(_buttFont);

	_muteAllCheck  = new  wxCheckBox(this, FT_MuteId, wxT("Mute All"),
					     wxDefaultPosition, wxSize(_labwidth,_bheight+3));
	_muteAllCheck->SetFont(_buttFont);


	_linkMixCheck  = new  wxCheckBox(this, FT_MixLinkedButton, wxT("Link Mix"),
					     wxDefaultPosition, wxSize(_labwidth,_bheight+3));
	_linkMixCheck->SetFont(_buttFont);

	

	_pathCountChoice = new wxChoice(this, FT_PathCountChoice, wxDefaultPosition, wxSize(_labwidth,-1));
	for (int i=0; i < FT_MAXPATHS; i++) {
		_pathCountChoice->Append(wxString::Format(wxT("%d chan"), i+1), (void *) ((intptr_t)(i+1)));
	}
	_pathCountChoice->SetStringSelection(wxString::Format(wxT("%d chan"), _startpaths));
	
		

	// spec types buttons
	PixButton * pixbutt;
	pixbutt = _inspecSpecTypeAllButton = new PixButton(_inspecPanel, FT_InSpecTypeId, false, wxDefaultPosition, wxSize(_bwidth, _bheight));
	_inspecSpecTypeAllButton->SetFont(_buttFont);
	_inspecSpecTypeAllButton->SetToolTip (wxT("Spectrogram Plot"));
	pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::plot_clicked_events), pixbutt));
	pixbutt->set_normal_bitmap (wxBitmap(specplot_normal_xpm));
	pixbutt->set_selected_bitmap (wxBitmap(specplot_selected_xpm));
	pixbutt->set_focus_bitmap (wxBitmap(specplot_focus_xpm));
	pixbutt->set_disabled_bitmap (wxBitmap(specplot_disabled_xpm));
	pixbutt->set_active_bitmap (wxBitmap(specplot_active_xpm));


	pixbutt = _inspecPlotSolidTypeAllButton = new PixButton(_inspecPanel, FT_InSpecTypeId, false, wxDefaultPosition, wxSize(_bwidth, _bheight));
	_inspecPlotSolidTypeAllButton->SetFont(_buttFont);
	_inspecPlotSolidTypeAllButton->SetToolTip (wxT("Filled Plot"));
	pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::plot_clicked_events), pixbutt));
	pixbutt->set_normal_bitmap (wxBitmap(barplot_normal_xpm));
	pixbutt->set_selected_bitmap (wxBitmap(barplot_selected_xpm));
	pixbutt->set_focus_bitmap (wxBitmap(barplot_focus_xpm));
	pixbutt->set_disabled_bitmap (wxBitmap(barplot_disabled_xpm));
	pixbutt->set_active_bitmap (wxBitmap(barplot_active_xpm));

	pixbutt = _inspecPlotLineTypeAllButton = new PixButton(_inspecPanel, FT_InSpecTypeId, false, wxDefaultPosition, wxSize(_bwidth, _bheight));
	_inspecPlotLineTypeAllButton->SetFont(_buttFont);
	_inspecPlotLineTypeAllButton->SetToolTip (wxT("Line Plot"));
	pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::plot_clicked_events), pixbutt));
	pixbutt->set_normal_bitmap (wxBitmap(lineplot_normal_xpm));
	pixbutt->set_selected_bitmap (wxBitmap(lineplot_selected_xpm));
	pixbutt->set_focus_bitmap (wxBitmap(lineplot_focus_xpm));
	pixbutt->set_disabled_bitmap (wxBitmap(lineplot_disabled_xpm));
	pixbutt->set_active_bitmap (wxBitmap(lineplot_active_xpm));

	

	// uppersizer
	tmpsizer = new wxBoxSizer(wxVERTICAL);
	tmpsizer->Add (_pathCountChoice, 0, wxALL, 1);
	tmpsizer->Add (1,4, 0);
	tmpsizer->Add (_bypassAllCheck, 0, wxALL, 1 );
	_uppersizer->Add(tmpsizer, 0, wxEXPAND);

	mainvsizer->Add(_uppersizer, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 2);

	
	// input spectragram
	tmpsizer = new wxBoxSizer(wxVERTICAL);
	tmpsizer->Add (_inspecLabelButton, 0, wxALL , 1);
	tmpsizer2 = new wxBoxSizer(wxHORIZONTAL);
	tmpsizer2->Add (_inspecSpecTypeAllButton, 0, wxALL, 1);
	tmpsizer2->Add (_inspecPlotLineTypeAllButton, 0, wxALL, 1);
	tmpsizer2->Add (_inspecPlotSolidTypeAllButton, 0, wxALL, 1);
	tmpsizer->Add (tmpsizer2, 0, wxALL|wxEXPAND, 0);

	_inspecsizer->Add (tmpsizer, 0, wxALL|wxEXPAND, 0);

	_inspecPanel->SetAutoLayout(TRUE);
	_inspecsizer->Fit( _inspecPanel );  
	_inspecPanel->SetSizer(_inspecsizer);
	wxLayoutConstraints *inspecConst = new wxLayoutConstraints;
	inspecConst->top.SameAs (_rowPanel, wxTop, 2);
	inspecConst->left.SameAs (_rowPanel, wxLeft, 2);
	inspecConst->right.SameAs (_rowPanel, wxRight, 2);
	inspecConst->height.AsIs();
	_inspecSash->SetConstraints(inspecConst);
	//_inspecSash->Show(false);

	// NOTE: need to create and push this onto rowItems
	// before calling pushProcRow
	_outspecsizer = new wxBoxSizer(wxHORIZONTAL);
	_lowersizer = new wxBoxSizer(wxHORIZONTAL);

	_outspecSash = new wxSashLayoutWindow(_rowPanel, FT_RowPanelId, wxDefaultPosition, wxSize(-1,_rowh));
	_outspecPanel = new wxPanel(_outspecSash, -1);
	_outspecPanel->SetBackgroundColour(*wxBLACK);
	_outspecPanel->SetThemeEnabled(false);
	_outspecSash->SetSashBorder(wxSASH_BOTTOM, true);
	_outspecSash->SetSashVisible(wxSASH_BOTTOM, true);
	_rowItems.push_back (_outspecSash);


	
	wxWindow *lastsash = _inspecSash;
	
	int rowcnt=0;
	
	for (unsigned int n=0; n < procmods.size(); ++n)
	{
		FTprocI *pm = procmods[n];
		vector<FTspectrumModifier *> filts;
		pm->getFilters (filts);
		int lastgroup = -1;
		
		for (unsigned int m=0; m < filts.size(); ++m)
		{
			// if the group is different from the last
			// make a new row for it
			if (filts[m]->getGroup() == lastgroup) {
				continue;
			}
			lastgroup = filts[m]->getGroup();

			pushProcRow (filts[m]);

			lastsash = _rowSashes[rowcnt];
			
			rowcnt++;
		}
	}
	
	//_mashsizer = new wxBoxSizer(wxHORIZONTAL);
// 	_gatesizer = new wxBoxSizer(wxHORIZONTAL);
// 	_freqsizer = new wxBoxSizer(wxHORIZONTAL);
// 	_delaysizer = new wxBoxSizer(wxHORIZONTAL);
// 	_feedbsizer = new wxBoxSizer(wxHORIZONTAL);

	//printf ("subrowpanel cnt = %d\n", _subrowPanels.size());
	
	

	_outspecLabelButton = new wxButton(_outspecPanel, FT_OutSpecLabelId, wxT("Out Spectra"),
						  wxDefaultPosition, wxSize(_labwidth,_bheight));
	_outspecLabelButton->SetBackgroundColour(*wxBLACK);
	_outspecLabelButton->SetForegroundColour(*wxWHITE);
	_outspecLabelButton->SetThemeEnabled(false);
	_outspecLabelButton->SetFont(_titleFont);
	_outspecLabelButton->SetToolTip(wxT("Hide Out Spectra"));


	
	_outspecLabelButtonAlt = new wxButton(_rowPanel, FT_OutSpecLabelId, wxT("Out Spectra"),
						  wxDefaultPosition, wxSize(_labwidth,_bheight));
	_outspecLabelButtonAlt->SetFont(_titleAltFont);
	_outspecLabelButtonAlt->SetToolTip(wxT("Show Out Spectra"));
	_outspecLabelButtonAlt->SetBackgroundColour(*wxBLACK);
	_outspecLabelButtonAlt->SetForegroundColour(*wxWHITE);
	//_outspecLabelButtonAlt->SetThemeEnabled(false);
	_outspecLabelButtonAlt->Show(false);
	constr = new wxLayoutConstraints;
	constr->left.SameAs (_rowPanel, wxLeft, 2);
	constr->width.Absolute (_labwidth);
	constr->height.Absolute (_bheight);
	constr->top.SameAs (_outspecSash, wxTop);
	_outspecLabelButtonAlt->SetConstraints(constr);



	// @@@@@@@
	
	pixbutt = _outspecSpecTypeAllButton = new PixButton(_outspecPanel, FT_OutSpecTypeId, false, wxDefaultPosition, wxSize(_bwidth, _bheight));
	_outspecSpecTypeAllButton->SetFont(_buttFont);
	_outspecSpecTypeAllButton->SetToolTip (wxT("Spectrogram Plot"));
	pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::plot_clicked_events), pixbutt));
	pixbutt->set_normal_bitmap (wxBitmap(specplot_normal_xpm));
	pixbutt->set_selected_bitmap (wxBitmap(specplot_selected_xpm));
	pixbutt->set_focus_bitmap (wxBitmap(specplot_focus_xpm));
	pixbutt->set_disabled_bitmap (wxBitmap(specplot_disabled_xpm));
	pixbutt->set_active_bitmap (wxBitmap(specplot_active_xpm));

	pixbutt = _outspecPlotSolidTypeAllButton = new PixButton(_outspecPanel, FT_OutSpecTypeId, false, wxDefaultPosition, wxSize(_bwidth, _bheight));
	_outspecPlotSolidTypeAllButton->SetFont(_buttFont);
	_outspecPlotSolidTypeAllButton->SetToolTip (wxT("Filled Plot"));
	pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::plot_clicked_events), pixbutt));
	pixbutt->set_normal_bitmap (wxBitmap(barplot_normal_xpm));
	pixbutt->set_selected_bitmap (wxBitmap(barplot_selected_xpm));
	pixbutt->set_focus_bitmap (wxBitmap(barplot_focus_xpm));
	pixbutt->set_disabled_bitmap (wxBitmap(barplot_disabled_xpm));
	pixbutt->set_active_bitmap (wxBitmap(barplot_active_xpm));

	pixbutt = _outspecPlotLineTypeAllButton = new PixButton(_outspecPanel, FT_OutSpecTypeId, false, wxDefaultPosition, wxSize(_bwidth, _bheight));
	_outspecPlotLineTypeAllButton->SetFont(_buttFont);
	_outspecPlotLineTypeAllButton->SetToolTip (wxT("Line Plot"));
	pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::plot_clicked_events), pixbutt));
	pixbutt->set_normal_bitmap (wxBitmap(lineplot_normal_xpm));
	pixbutt->set_selected_bitmap (wxBitmap(lineplot_selected_xpm));
	pixbutt->set_focus_bitmap (wxBitmap(lineplot_focus_xpm));
	pixbutt->set_disabled_bitmap (wxBitmap(lineplot_disabled_xpm));
	pixbutt->set_active_bitmap (wxBitmap(lineplot_active_xpm));


		
	// output spectragram
	tmpsizer = new wxBoxSizer(wxVERTICAL);
	tmpsizer->Add (_outspecLabelButton, 0, wxALL , 1);
	tmpsizer2 = new wxBoxSizer(wxHORIZONTAL);
	tmpsizer2->Add (_outspecSpecTypeAllButton, 1, wxALL, 1);
	tmpsizer2->Add (_outspecPlotLineTypeAllButton, 1, wxALL, 1);
	tmpsizer2->Add (_outspecPlotSolidTypeAllButton, 1, wxALL, 1);
	tmpsizer->Add (tmpsizer2, 0, wxALL|wxEXPAND, 0);
	_outspecsizer->Add (tmpsizer, 0, wxALL|wxEXPAND, 0);

	_outspecPanel->SetAutoLayout(TRUE);
	_outspecsizer->Fit( _outspecPanel );  
	_outspecPanel->SetSizer(_outspecsizer);
	wxLayoutConstraints *outspecConst = new wxLayoutConstraints;
	outspecConst->left.SameAs (_rowPanel, wxLeft, 2);
	outspecConst->right.SameAs (_rowPanel, wxRight, 2);
	outspecConst->top.SameAs (lastsash, wxBottom, 2);
	outspecConst->height.AsIs();
	_outspecSash->SetConstraints(outspecConst);
	//_outspecSash->Show(false);


	_rowPanel->SetAutoLayout(TRUE);

	
	mainvsizer->Add (_rowPanel, 1,  wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 2);
	

	// lowersizer
	tmpsizer = new wxBoxSizer(wxVERTICAL);
	tmpsizer->Add (1,10,0);
	tmpsizer->Add (_muteAllCheck, 0, wxALL, 1);
	tmpsizer->Add (_linkMixCheck, 0, wxALL, 1);
	_lowersizer->Add(tmpsizer, 0, wxEXPAND);

	mainvsizer->Add(_lowersizer, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 2);


	for (int i=0; i < _startpaths; i++)
	{
		// add to io support instance
		//_processPath[i] = iosup->setProcessPathActive (i, true);

		if (!_processPath[i]) continue;

		FTspectralEngine * engine = _processPath[i]->getSpectralEngine();

		// set default freqbin
		_freqBinsChoice->SetStringSelection(wxString::Format(wxT("%d"), engine->getFFTsize()/2));

		engine->setUpdateToken (_updateTokens[i]);
		
		createPathStuff (i);

		engine->setTempo (_tempoSpinCtrl->GetValue()); // purely for saving
		
		if (i > 0) {
			// link everything to first
			vector<FTprocI *> newprocmods;
			engine->getProcessorModules (newprocmods);
			
			for (unsigned int n=0; n < newprocmods.size(); ++n)
			{
				FTprocI *pm = newprocmods[n];
				vector<FTspectrumModifier *> filts;
				pm->getFilters (filts);
				
				for (unsigned int m=0; m < filts.size(); ++m)
				{
					filts[m]->link (procmods[n]->getFilter(m));
				}

			}
		}
	}

	
	rebuildPresetCombo();

	
	mainsizer->Add(mainvsizer, 1, wxALL|wxEXPAND, 0);
	
	SetAutoLayout( TRUE );
	// set frame to minimum size
	mainsizer->Fit( this );  
	// don't allow frame to get smaller than what the sizers tell ye
	mainsizer->SetSizeHints( this );  
	SetSizer( mainsizer );

	rowpanelScrollSize();

	//wxToolTip::Enable(true);
	
	// force a nice minimum size
	this->SetSizeHints(453,281);
	
	// set timer
	
	
	_eventTimer->Start(_updateMS, FALSE);

	_refreshTimer->Start(_refreshMS, FALSE);

}


void FTmainwin::pushProcRow(FTspectrumModifier *specmod)
{
	wxSashLayoutWindow * lastsash;

	if (_rowSashes.size() == 0) {
		lastsash = _inspecSash;
	}
	else {
		lastsash = _rowSashes.back();
	}

	wxBoxSizer *rowsizer = new wxBoxSizer(wxHORIZONTAL);
	_rowSizers.push_back (rowsizer);

	wxSashLayoutWindow * sash = new wxSashLayoutWindow(_rowPanel, FT_RowPanelId, wxDefaultPosition, wxSize(-1,_rowh));
	sash->SetBackgroundColour(wxColour(20,20,20));
	sash->SetThemeEnabled(false);
	_rowSashes.push_back (sash);
			
	wxPanel * rpanel = new wxPanel(sash, -1);
	rpanel->SetBackgroundColour(*wxBLACK);
	rpanel->SetThemeEnabled(false);
	_rowPanels.push_back (rpanel);
			
	sash->SetSashBorder(wxSASH_BOTTOM, true);
	sash->SetSashVisible(wxSASH_BOTTOM, true);
	// need to insert this as second to last
	wxWindow * lastrow = _rowItems.back();
	_rowItems.pop_back();
	_rowItems.push_back (sash);
	_rowItems.push_back (lastrow);
	
	string name = specmod->getName();

	// main label button
	wxButton *labbutt = new FTtitleButton(this, false, rpanel, FT_LabelBase, wxString::FromAscii (name.c_str()),
					 wxDefaultPosition, wxSize(_labwidth,_bheight));
	labbutt->SetFont(_titleFont);
	labbutt->SetToolTip(wxString(wxT("Hide ")) + 
			    wxString::FromAscii (name.c_str()) +
			    wxT("\nRight-click for menu"));
	_labelButtons.push_back (labbutt);
			
	// alt label button
	wxButton * altlab  = new FTtitleButton(this, true, _rowPanel, FT_LabelBase, wxString::FromAscii (name.c_str()),
					  wxDefaultPosition, wxSize(_labwidth,_bheight));
	altlab->SetFont(_titleAltFont);
	altlab->SetToolTip(wxString(wxT("Show ")) +
			   wxString::FromAscii (name.c_str()) +
			   wxT("\nRight-click for menu"));
	altlab->Show(false);
	wxLayoutConstraints * constr = new wxLayoutConstraints;
	constr->left.SameAs (_rowPanel, wxLeft, 2);
	constr->width.Absolute (_labwidth);
	constr->height.Absolute (_bheight);
	constr->top.SameAs (sash, wxTop);
	altlab->SetConstraints(constr);
	_altLabelButtons.push_back (altlab);
			
	// link all button
	PixButton * pixbutt;
	PixButton * linkallbutt = pixbutt = new PixButton(rpanel, FT_LinkBase, false, wxDefaultPosition, wxSize(_bwidth,_bheight));
	linkallbutt->SetFont(_buttFont);
	linkallbutt->SetToolTip (wxT("Link All"));
	pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::link_clicked_events), pixbutt));
	pixbutt->set_normal_bitmap (wxBitmap(link_normal_xpm));
	pixbutt->set_selected_bitmap (wxBitmap(link_selected_xpm));
	pixbutt->set_focus_bitmap (wxBitmap(link_focus_xpm));
	pixbutt->set_disabled_bitmap (wxBitmap(link_disabled_xpm));
	pixbutt->set_active_bitmap (wxBitmap(link_active_xpm));
	_linkAllButtons.push_back (linkallbutt);

	// bypass all button
	PixButton * bypallbutt = pixbutt = new PixButton(rpanel, FT_BypassBase, false, wxDefaultPosition, wxSize(_bwidth,_bheight));
	bypallbutt->SetFont(_buttFont);
	bypallbutt->SetToolTip (wxT("Bypass All"));
	pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::bypass_clicked_events), pixbutt));
	pixbutt->set_normal_bitmap (wxBitmap(bypass_normal_xpm));
	pixbutt->set_selected_bitmap (wxBitmap(bypass_selected_xpm));
	pixbutt->set_focus_bitmap (wxBitmap(bypass_focus_xpm));
	pixbutt->set_disabled_bitmap (wxBitmap(bypass_disabled_xpm));
	pixbutt->set_active_bitmap (wxBitmap(bypass_active_xpm));
	_bypassAllButtons.push_back (bypallbutt);

	// Grid buttons
			
	PixButton * gridbutt = pixbutt = new PixButton (rpanel, FT_GridBase, false, wxDefaultPosition,  wxSize(_bwidth,_bheight));
	gridbutt->SetFont(_buttFont);
	gridbutt->SetToolTip (wxT("Toggle Grid\nRight-click to Adjust"));
	pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::grid_clicked_events), pixbutt));
	pixbutt->pressed.connect (bind (slot (*this, &FTmainwin::grid_pressed_events), pixbutt));
	pixbutt->set_normal_bitmap (wxBitmap(grid_normal_xpm));
	pixbutt->set_selected_bitmap (wxBitmap(grid_selected_xpm));
	pixbutt->set_focus_bitmap (wxBitmap(grid_focus_xpm));
	pixbutt->set_disabled_bitmap (wxBitmap(grid_disabled_xpm));
	pixbutt->set_active_bitmap (wxBitmap(grid_active_xpm));
	_gridButtons.push_back (gridbutt);

	// GridSnap buttons
			
	PixButton * gridsnbutt = pixbutt = new PixButton (rpanel, FT_GridSnapBase, false, wxDefaultPosition,  wxSize(_bwidth,_bheight));
	gridsnbutt->SetFont(_buttFont);
	gridsnbutt->SetToolTip (wxT("Toggle Grid Snap"));
	pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::grid_clicked_events), pixbutt));
	pixbutt->set_normal_bitmap (wxBitmap(gridsnap_normal_xpm));
	pixbutt->set_selected_bitmap (wxBitmap(gridsnap_selected_xpm));
	pixbutt->set_focus_bitmap (wxBitmap(gridsnap_focus_xpm));
	pixbutt->set_disabled_bitmap (wxBitmap(gridsnap_disabled_xpm));
	pixbutt->set_active_bitmap (wxBitmap(gridsnap_active_xpm));
	_gridSnapButtons.push_back (gridsnbutt);

			
	// main row stuff
	wxBoxSizer * rowbuttsizer = new wxBoxSizer(wxVERTICAL);
	rowbuttsizer->Add (labbutt, 0, wxALL , 1);
	wxBoxSizer * tmpsizer2 = new wxBoxSizer(wxHORIZONTAL);
	tmpsizer2->Add (bypallbutt, 0, wxALL, 1);
	tmpsizer2->Add (linkallbutt, 0, wxALL, 1);
	rowbuttsizer->Add (tmpsizer2, 0, wxALL|wxEXPAND, 0);
	tmpsizer2 = new wxBoxSizer(wxHORIZONTAL);
	tmpsizer2->Add (gridbutt, 0, wxALL, 1);
	tmpsizer2->Add (gridsnbutt, 0, wxALL, 1);
	rowbuttsizer->Add (tmpsizer2, 0, wxALL|wxEXPAND, 0);
	_rowButtSizers.push_back ( rowbuttsizer);
			
	rowsizer->Add (rowbuttsizer, 0, wxALL|wxEXPAND, 0);
			
	rpanel->SetAutoLayout(TRUE);
	rowsizer->Fit( rpanel );  
	rpanel->SetSizer(rowsizer);
	wxLayoutConstraints *rowconst = new wxLayoutConstraints;
	rowconst->left.SameAs (_rowPanel, wxLeft, 2);
	rowconst->right.SameAs (_rowPanel, wxRight, 2);
	rowconst->top.SameAs (lastsash, wxBottom,  2);
	rowconst->height.AsIs();
	sash->SetConstraints(rowconst);


	// construct initial arrays
	FTactiveBarGraph ** bgraphs = new FTactiveBarGraph*[FT_MAXPATHS];
	for (int n=0; n < FT_MAXPATHS; ++n) bgraphs[n] = 0;
	_barGraphs.push_back (bgraphs);
			
	wxPanel ** srpanels = new wxPanel*[FT_MAXPATHS];
	for (int n=0; n < FT_MAXPATHS; ++n) srpanels[n] = 0;
	_subrowPanels.push_back (srpanels);

	PixButton ** bybuttons = new PixButton*[FT_MAXPATHS];
	for (int n=0; n < FT_MAXPATHS; ++n) bybuttons[n] = 0;
	_bypassButtons.push_back (bybuttons);
			
	PixButton ** linkbuttons = new PixButton*[FT_MAXPATHS];
	for (int n=0; n < FT_MAXPATHS; ++n) linkbuttons[n] = 0;
	_linkButtons.push_back (linkbuttons);

	

}


void FTmainwin::popProcRow()
{
	// remove the last row of stuff
	wxWindow * top = _rowSashes.back();
	//wxSizer * topsizer = _rowSizers.back();
	//wxPanel * rpanel = _rowPanels.back();

	// pop all the stuff we pushed before
	_rowSizers.pop_back();
	_rowSashes.pop_back();
	_rowPanels.pop_back();

	// need to pop the second to last off
	_rowItems.pop_back();
	_rowItems.pop_back();
	_rowItems.push_back(_outspecSash);
	
	_labelButtons.pop_back();
	_bypassAllButtons.pop_back();
	_linkAllButtons.pop_back();
	_rowButtSizers.pop_back();
	_gridButtons.pop_back();
	_gridSnapButtons.pop_back();
	
	// owned by _rowPanel
	wxButton * altbutt = _altLabelButtons.back();
	altbutt->SetConstraints(NULL);
	altbutt->Destroy();
	_altLabelButtons.pop_back();
	
	FTactiveBarGraph **bgraphs = _barGraphs.back();
	delete [] bgraphs;
	_barGraphs.pop_back();

	wxPanel **srpanels = _subrowPanels.back();
	delete [] srpanels;
	_subrowPanels.pop_back();

	PixButton **bybuttons = _bypassButtons.back();
	delete [] bybuttons;
	_bypassButtons.pop_back();
	
	PixButton **linkbuttons = _linkButtons.back();
	delete [] linkbuttons;
	_linkButtons.pop_back();


	wxLayoutConstraints * outspecConst = _outspecSash->GetConstraints();
	if (_rowSashes.size() > 0) {
		outspecConst->top.SameAs (_rowSashes.back(), wxBottom, 2);
		_outspecLabelButtonAlt->GetConstraints()->top.Below(_rowSashes.back(), 2);

	}
	else {
		outspecConst->top.SameAs (_inspecSash, wxBottom, 2);
		_outspecLabelButtonAlt->GetConstraints()->top.Below(_inspecSash, 2);
	}
	
	// this should destroy all the wx windows and stuff
	top->SetConstraints(NULL);
	top->Destroy();

	
	_rowPanel->Layout();
	rowpanelScrollSize();
	
}

	
// void FTmainwin::OnSize(wxSizeEvent &event)
// {
// 	// rowpanelScrollSize();

// 	event.Skip();
// }

void FTmainwin::removePathStuff(int i, bool deactivate)
{
	FTspectralEngine * engine = _processPath[i]->getSpectralEngine();

	if (deactivate) {
		// deactivate it by the io thread
		FTioSupport::instance()->setProcessPathActive(i, false);
		_processPath[i] = 0;
	}

	vector<FTprocI *> procmods;
	engine->getProcessorModules (procmods);
	
	_uppersizer->Remove(i+1);
	_inspecsizer->Remove(i+1);


	for (unsigned int n=0; n < _rowSizers.size(); ++n)
	{
		_rowSizers[n]->Remove (i+1);
		_subrowPanels[n][i]->Destroy();
		_rowSizers[n]->Layout();
	}
	
// 	unsigned int rowcnt = 0;
	
// 	for (unsigned int n=0; n < procmods.size(); ++n)
// 	{
// 		FTprocI *pm = procmods[n];
// 		vector<FTspectrumModifier *> filts;
// 		pm->getFilters (filts);
// 		int lastgroup=-1;
		
// 		for (unsigned int m=0; m < filts.size(); ++m)
// 		{
// 			if (filts[m]->getGroup() == lastgroup) {
// 				continue;
// 			}
// 			lastgroup = filts[m]->getGroup();

// 			_rowSizers[rowcnt]->Remove (i+1);

// 			_subrowPanels[rowcnt][i]->Destroy();

// 			_rowSizers[rowcnt]->Layout();
						
// 			rowcnt++;
// 		}

// 	}
	

	_outspecsizer->Remove(i+1);
	_lowersizer->Remove(i+1);
	

	_upperPanels[i]->Destroy();
	_inspecPanels[i]->Destroy();


	_outspecPanels[i]->Destroy();
	_lowerPanels[i]->Destroy();

	
	_uppersizer->Layout();
	_inspecsizer->Layout();
	_outspecsizer->Layout();
	_lowersizer->Layout();

}

void FTmainwin::createPathStuff(int i)
{
	wxBoxSizer * buttsizer, *tmpsizer, *tmpsizer2;
	wxStaticText *stattext;


	
	FTspectralEngine * engine = _processPath[i]->getSpectralEngine();
	vector<FTprocI *> procmods;
	engine->getProcessorModules (procmods);

	_upperPanels[i] = new wxPanel(this, -1);
	_lowerPanels[i] = new wxPanel(this, -1);

	_inspecPanels[i] = new wxPanel(_inspecPanel, -1);
	_inspecPanels[i]->SetBackgroundColour(*wxBLACK);
	_inspecPanels[i]->SetThemeEnabled(false);
	
	int rowcnt=-1; // preincremented below
	
	for (unsigned int n=0; n < procmods.size(); ++n)
	{
		FTprocI *pm = procmods[n];
		vector<FTspectrumModifier *> filts;
		pm->getFilters (filts);
		int lastgroup = -1;
			
		for (unsigned int m=0; m < filts.size(); ++m)
		{
			wxPanel ** rowpanels = 0;
			FTactiveBarGraph **bargraphs = 0;

			if (filts[m]->getGroup() != lastgroup)
			{				
				rowcnt++;

				rowpanels = _subrowPanels[rowcnt];
				bargraphs = _barGraphs[rowcnt];
				
				rowpanels[i] = new wxPanel (_rowPanels[rowcnt], -1);
				rowpanels[i]->SetBackgroundColour(*wxBLACK);
				rowpanels[i]->SetThemeEnabled(false);
				
				bargraphs[i] = new FTactiveBarGraph(this, rowpanels[i], -1);
				bargraphs[i]->setSpectrumModifier (filts[m]);
				bargraphs[i]->setBypassed (filts[m]->getBypassed());
				bargraphs[i]->setTempo(_tempoSpinCtrl->GetValue());

				lastgroup = filts[m]->getGroup();
			}
			else 
			{
				// assign nth filter to already created plot
				// TODO: support more than 2
				rowpanels = _subrowPanels[rowcnt];
				bargraphs = _barGraphs[rowcnt];
				bargraphs[i]->setTopSpectrumModifier (filts[m]);

				continue;
			}
			
					
			buttsizer = new wxBoxSizer(wxVERTICAL);
			PixButton * pixbutt;
			pixbutt = new PixButton(rowpanels[i], FT_BypassBase, false, wxDefaultPosition, wxSize(_bwidth, -1));
			
			pixbutt->set_normal_bitmap (wxBitmap(bypass_normal_xpm));
			pixbutt->set_selected_bitmap (wxBitmap(bypass_selected_xpm));
			pixbutt->set_focus_bitmap (wxBitmap(bypass_focus_xpm));
			pixbutt->set_disabled_bitmap (wxBitmap(bypass_disabled_xpm));
			pixbutt->set_active_bitmap (wxBitmap(bypass_active_xpm));
			
			_bypassButtons[rowcnt][i] = pixbutt;
			//buttsizer->Add ( _bypassButtons[rowcnt][i] = new wxButton(rowpanels[i], FT_BypassBase, wxT("B"),
			//							  wxDefaultPosition, wxSize(_bwidth,-1)), 1, 0,0);
			pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::bypass_clicked_events), pixbutt));
			
			buttsizer->Add (pixbutt, 0, 0, 0);

			//buttsizer->Add ( _bypassButtons[rowcnt][i] = new wxButton(rowpanels[i], FT_BypassBase, wxT("B"),
			//							  wxDefaultPosition, wxSize(_bwidth,-1)), 1, 0,0);
			_bypassButtons[rowcnt][i]->SetFont(_buttFont);
			_bypassButtons[rowcnt][i]->SetToolTip(wxT("Toggle Bypass"));		

			pixbutt = new PixButton(rowpanels[i], FT_LinkBase, false, wxDefaultPosition, wxSize(_bwidth, -1));
			pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::link_clicked_events), pixbutt));
			
			pixbutt->set_normal_bitmap (wxBitmap(link_normal_xpm));
			pixbutt->set_selected_bitmap (wxBitmap(link_selected_xpm));
			pixbutt->set_focus_bitmap (wxBitmap(link_focus_xpm));
			pixbutt->set_disabled_bitmap (wxBitmap(link_disabled_xpm));
			pixbutt->set_active_bitmap (wxBitmap(link_active_xpm));
			
			_linkButtons[rowcnt][i] = pixbutt;
			//buttsizer->Add ( _linkButtons[rowcnt][i] = new wxButton(rowpanels[i], FT_LinkBase, wxT("L"),
					//							wxDefaultPosition, wxSize(_bwidth,-1)), 0, wxTOP,3);
			buttsizer->Add (_linkButtons[rowcnt][i], 0, wxTOP, 3);


			_linkButtons[rowcnt][i]->SetFont(_buttFont);
			_linkButtons[rowcnt][i]->SetToolTip(wxT("Link"));		

			tmpsizer = new wxBoxSizer(wxHORIZONTAL);
			tmpsizer->Add ( buttsizer, 0, wxALL|wxEXPAND, 1);
			tmpsizer->Add ( bargraphs[i],  1, wxALL|wxEXPAND, 1);
				
				
			rowpanels[i]->SetAutoLayout(TRUE);
			tmpsizer->Fit( rowpanels[i] );  
			rowpanels[i]->SetSizer(tmpsizer);

			_rowSizers[rowcnt]->Insert (i+1, rowpanels[i],  1, wxLEFT|wxEXPAND, 4);

		}

	}




	_outspecPanels[i] = new wxPanel(_outspecPanel, -1);
	_outspecPanels[i]->SetBackgroundColour(*wxBLACK);
	_outspecPanels[i]->SetThemeEnabled(false);


	wxStaticBox *inbox = new wxStaticBox(_upperPanels[i], -1, wxString::Format(wxT("Input %d"), i+1));
	wxStaticBox *outbox = new wxStaticBox(_lowerPanels[i], -1, wxString::Format(wxT("Output %d"), i+1));
	
	// plots and active graphs
	_inputSpectragram[i] = new FTspectragram(this, _inspecPanels[i], -1);
	_inputSpectragram[i]->setDataLength((unsigned int)engine->getFFTsize() >> 1);
	_outputSpectragram[i] = new FTspectragram(this, _outspecPanels[i], -1);
	_outputSpectragram[i]->setDataLength((unsigned int)engine->getFFTsize() >> 1);
	
	
	
	// I/O buttons
	_inputButton[i] = new wxButton(_upperPanels[i], (int) FT_InputButtonId, wxT("No Input"), wxDefaultPosition, wxSize(-1,-1));
	
	_outputButton[i] = new wxButton(_lowerPanels[i], (int) FT_OutputButtonId, wxT("No Output"), wxDefaultPosition, wxSize(-1,-1));
	
	_bypassCheck[i] = new wxCheckBox(_upperPanels[i], (int) FT_BypassId, wxT("Bypass"), wxDefaultPosition, wxSize(-1,-1));
	_muteCheck[i] = new wxCheckBox(_lowerPanels[i], (int) FT_MuteId, wxT("Mute"), wxDefaultPosition, wxSize(-1,-1));
	
	// input area
	{
		tmpsizer = new wxStaticBoxSizer (inbox, wxVERTICAL);
		tmpsizer->Add (_inputButton[i], 0, wxBOTTOM|wxEXPAND, 1);
		
		tmpsizer2 = new wxBoxSizer (wxHORIZONTAL);
		tmpsizer2->Add (_bypassCheck[i], 1, wxRIGHT|wxALIGN_CENTRE_VERTICAL, 3);
		
		stattext = new wxStaticText(_upperPanels[i], -1, wxT("Gain (dB)"), wxDefaultPosition, wxDefaultSize);
		tmpsizer2->Add (stattext, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 0);
		//_gainSlider[i] = new wxSlider(this, FT_GainSlider, 0, -70, 10);
		//tmpsizer2->Add (_gainSlider[i], 1, wxALL|wxALIGN_CENTRE_VERTICAL, 1);
		_gainSpinCtrl[i] = new wxSpinCtrl(_upperPanels[i], FT_GainSpin, wxT("0"), wxDefaultPosition, wxSize(65,-1),
						  wxSP_ARROW_KEYS, -70, 10, 0);
		tmpsizer2->Add (_gainSpinCtrl[i], 0, wxLEFT, 4);
		
		tmpsizer->Add (tmpsizer2, 0, wxTOP|wxEXPAND, 1);

		_upperPanels[i]->SetAutoLayout(TRUE);
		tmpsizer->Fit( _upperPanels[i] );  
		_upperPanels[i]->SetSizer(tmpsizer);
		
		_uppersizer->Insert (i+1, _upperPanels[i], 1, wxLEFT, 3);
	}

	// input spec
	{		
		buttsizer = new wxBoxSizer(wxVERTICAL);
		PixButton * pixbutt;
		pixbutt = _inspecSpecTypeButton[i] = new PixButton(_inspecPanels[i], FT_InSpecTypeId, false, wxDefaultPosition, wxSize(_bwidth, _bheight));
		_inspecSpecTypeButton[i]->SetFont(_buttFont);
		_inspecSpecTypeButton[i]->SetToolTip(wxT("Spectrogram Plot"));
		pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::plot_clicked_events), pixbutt));
		pixbutt->set_normal_bitmap (wxBitmap(specplot_normal_xpm));
		pixbutt->set_selected_bitmap (wxBitmap(specplot_selected_xpm));
		pixbutt->set_focus_bitmap (wxBitmap(specplot_focus_xpm));
		pixbutt->set_disabled_bitmap (wxBitmap(specplot_disabled_xpm));
		pixbutt->set_active_bitmap (wxBitmap(specplot_active_xpm));
		buttsizer->Add ( _inspecSpecTypeButton[i], 0, wxALL , 1);

		pixbutt = _inspecPlotLineTypeButton[i] = new PixButton(_inspecPanels[i], FT_InSpecTypeId, false, wxDefaultPosition, wxSize(_bwidth, _bheight));
		_inspecPlotLineTypeButton[i]->SetFont(_buttFont);
		_inspecPlotLineTypeButton[i]->SetToolTip(wxT("Line Plot"));
		pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::plot_clicked_events), pixbutt));
		pixbutt->set_normal_bitmap (wxBitmap(lineplot_normal_xpm));
		pixbutt->set_selected_bitmap (wxBitmap(lineplot_selected_xpm));
		pixbutt->set_focus_bitmap (wxBitmap(lineplot_focus_xpm));
		pixbutt->set_disabled_bitmap (wxBitmap(lineplot_disabled_xpm));
		pixbutt->set_active_bitmap (wxBitmap(lineplot_active_xpm));
		buttsizer->Add ( _inspecPlotLineTypeButton[i], 0, wxALL , 1);

		pixbutt = _inspecPlotSolidTypeButton[i] = new PixButton(_inspecPanels[i], FT_InSpecTypeId, false, wxDefaultPosition, wxSize(_bwidth, _bheight));
		_inspecPlotSolidTypeButton[i]->SetFont(_buttFont);
		_inspecPlotSolidTypeButton[i]->SetToolTip(wxT("Filled Plot"));
		pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::plot_clicked_events), pixbutt));
		pixbutt->set_normal_bitmap (wxBitmap(barplot_normal_xpm));
		pixbutt->set_selected_bitmap (wxBitmap(barplot_selected_xpm));
		pixbutt->set_focus_bitmap (wxBitmap(barplot_focus_xpm));
		pixbutt->set_disabled_bitmap (wxBitmap(barplot_disabled_xpm));
		pixbutt->set_active_bitmap (wxBitmap(barplot_active_xpm));
		buttsizer->Add ( _inspecPlotSolidTypeButton[i], 0, wxALL ,1);

		tmpsizer = new wxBoxSizer(wxHORIZONTAL);
		tmpsizer->Add ( buttsizer, 0, wxALL|wxEXPAND, 0);
		tmpsizer->Add ( _inputSpectragram[i],  1, wxALL|wxEXPAND, 1);


		_inspecPanels[i]->SetAutoLayout(TRUE);
		tmpsizer->Fit( _inspecPanels[i] );  
		_inspecPanels[i]->SetSizer(tmpsizer);
		
		_inspecsizer->Insert (i+1, _inspecPanels[i],  1, wxLEFT|wxEXPAND, 4);
	}


	// output spec
	{				
		buttsizer = new wxBoxSizer(wxVERTICAL);
		PixButton * pixbutt;
		
		pixbutt = _outspecSpecTypeButton[i] = new PixButton(_outspecPanels[i], FT_OutSpecTypeId, false, wxDefaultPosition, wxSize(_bwidth, _bheight));
		_outspecSpecTypeButton[i]->SetFont(_buttFont);
		_outspecSpecTypeButton[i]->SetToolTip(wxT("Spectrogram Plot"));
		pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::plot_clicked_events), pixbutt));
		pixbutt->set_normal_bitmap (wxBitmap(specplot_normal_xpm));
		pixbutt->set_selected_bitmap (wxBitmap(specplot_selected_xpm));
		pixbutt->set_focus_bitmap (wxBitmap(specplot_focus_xpm));
		pixbutt->set_disabled_bitmap (wxBitmap(specplot_disabled_xpm));
		pixbutt->set_active_bitmap (wxBitmap(specplot_active_xpm));
		buttsizer->Add ( _outspecSpecTypeButton[i], 0, wxALL,1);

		pixbutt = _outspecPlotLineTypeButton[i] = new PixButton(_outspecPanels[i], FT_OutSpecTypeId, false, wxDefaultPosition, wxSize(_bwidth, _bheight));
		_outspecPlotLineTypeButton[i]->SetFont(_buttFont);
		_outspecPlotLineTypeButton[i]->SetToolTip(wxT("Line Plot"));
		pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::plot_clicked_events), pixbutt));
		pixbutt->set_normal_bitmap (wxBitmap(lineplot_normal_xpm));
		pixbutt->set_selected_bitmap (wxBitmap(lineplot_selected_xpm));
		pixbutt->set_focus_bitmap (wxBitmap(lineplot_focus_xpm));
		pixbutt->set_disabled_bitmap (wxBitmap(lineplot_disabled_xpm));
		pixbutt->set_active_bitmap (wxBitmap(lineplot_active_xpm));
		buttsizer->Add ( _outspecPlotLineTypeButton[i], 0, wxALL, 1);

		pixbutt = _outspecPlotSolidTypeButton[i] = new PixButton(_outspecPanels[i], FT_OutSpecTypeId, false, wxDefaultPosition, wxSize(_bwidth, _bheight));
		_outspecPlotSolidTypeButton[i]->SetFont(_buttFont);
		_outspecPlotSolidTypeButton[i]->SetToolTip(wxT("Filled Plot"));
		pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::plot_clicked_events), pixbutt));
		pixbutt->set_normal_bitmap (wxBitmap(barplot_normal_xpm));
		pixbutt->set_selected_bitmap (wxBitmap(barplot_selected_xpm));
		pixbutt->set_focus_bitmap (wxBitmap(barplot_focus_xpm));
		pixbutt->set_disabled_bitmap (wxBitmap(barplot_disabled_xpm));
		pixbutt->set_active_bitmap (wxBitmap(barplot_active_xpm));
		buttsizer->Add ( _outspecPlotSolidTypeButton[i], 0, wxALL, 1);

		tmpsizer = new wxBoxSizer(wxHORIZONTAL);
		tmpsizer->Add ( buttsizer, 0, wxALL|wxEXPAND, 0);
		tmpsizer->Add ( _outputSpectragram[i],  1, wxALL|wxEXPAND, 1);

		_outspecPanels[i]->SetAutoLayout(TRUE);
		tmpsizer->Fit( _outspecPanels[i] );  
		_outspecPanels[i]->SetSizer(tmpsizer);

		_outspecsizer->Insert (i+1, _outspecPanels[i],  1, wxLEFT|wxEXPAND, 4);
	}


	// output stuff
	{		
		tmpsizer = new wxStaticBoxSizer (outbox, wxVERTICAL);

		tmpsizer2 = new wxBoxSizer (wxHORIZONTAL);
		tmpsizer2->Add (_muteCheck[i], 1, wxRIGHT, 5);
		stattext = new wxStaticText(_lowerPanels[i], -1, wxT("Dry"), wxDefaultPosition, wxDefaultSize);
		tmpsizer2->Add (stattext, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 1);
		_mixSlider[i] = new wxSlider(_lowerPanels[i], FT_MixSlider, 1000, 0, 1000);
		tmpsizer2->Add (_mixSlider[i], 2, wxALL|wxALIGN_CENTRE_VERTICAL, 1);
		stattext = new wxStaticText(_lowerPanels[i], -1, wxT("Wet"), wxDefaultPosition, wxDefaultSize);
		tmpsizer2->Add (stattext, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 1);

		tmpsizer->Add (tmpsizer2, 0, wxBOTTOM|wxEXPAND, 1);

		tmpsizer->Add(_outputButton[i], 0, wxEXPAND|wxTOP, 1);

		_lowerPanels[i]->SetAutoLayout(TRUE);
		tmpsizer->Fit( _lowerPanels[i] );  
		_lowerPanels[i]->SetSizer(tmpsizer);
		
		_lowersizer->Insert (i+1, _lowerPanels[i], 1, wxLEFT, 3);
		
	}
	
	
	engine->setUpdateToken (_updateTokens[i]);
		
	
}



void FTmainwin::rowpanelScrollSize()
{
	//int mw, mh;
	//int w, h;
	//int vw, vh;

	//GetClientSize(&mw, &mh);	
	//_rowPanel->GetClientSize(&w, &h);
	//_rowPanel->GetVirtualSize(&vw, &vh);

	// calculate virtual full height of rowpanel
	int realh = 0;

	for (unsigned int i=0; i < _rowItems.size(); i++) {
		// printf ("rowpanelsize: %d  %d\n", i, _rowItems.size());
		realh += _rowItems[i]->GetSize().GetHeight() + 2;
	}
	
	//printf ("rowpanel size %d %d   %d %d  realh %d\n", w, h, vw, vh, realh);

	_rowPanel->SetScrollbars(1, 1, 0, realh);
	

}

void FTmainwin::updateDisplay()
{
	FTioSupport *iosup = FTioSupport::instance();
	const char ** portnames = 0;

	bool inspec=true, inplotline=true, inplotsolid=true;
	bool outspec=true, outplotline=true, outplotsolid=true;
	bool bypassed=true, muted=true, linked, active;

	vector<bool> bypvec(_rowSizers.size(), true);
	vector<bool> linkvec(_rowSizers.size(), true);
	vector<bool> gridvec(_rowSizers.size(), true);
	vector<bool> gridsnapvec(_rowSizers.size(), true);

			
	
	for (int i=0; i<_pathCount; i++)
	{		
		if (!_processPath[i]) continue;
			
		// update port button labels
		portnames = iosup->getConnectedInputPorts(i);
		if (portnames) {
			_inputButton[i]->SetLabel (wxString::FromAscii (portnames[0]));
			free(portnames);
		}
		else {
			_inputButton[i]->SetLabel (wxT("No Input"));
		}
		
		portnames = iosup->getConnectedOutputPorts(i);
		if (portnames) {
			_outputButton[i]->SetLabel (wxString::FromAscii (portnames[0]));
			free(portnames);
		}
		else {
			_outputButton[i]->SetLabel (wxT("No Output"));
		}

		// update bypass and link buttons
		FTspectralEngine *engine =  _processPath[i]->getSpectralEngine();
		vector<FTprocI *> procmods;
		engine->getProcessorModules (procmods);
		unsigned int rowcnt=0;
		
		for (unsigned int n=0; n < procmods.size(); ++n)
		{
			FTprocI *pm = procmods[n];
			vector<FTspectrumModifier *> filts;
			pm->getFilters (filts);
			int lastgroup = -1;
			
			for (unsigned int m=0; m < filts.size(); ++m)
			{
				
				if (filts[m]->getGroup() == lastgroup) {
					// the first of any group will do
					continue;
				}
				lastgroup = filts[m]->getGroup();
				
				//_bypassButtons[rowcnt][i]->SetBackgroundColour ((filts[m]->getBypassed() ? (_activeBg) : (_defaultBg)));
				_bypassButtons[rowcnt][i]->set_active (filts[m]->getBypassed());				
				
				if (bypvec[rowcnt] && !filts[m]->getBypassed()) bypvec[rowcnt] = false;


				linked = ( filts[m]->getLink()!=0 || filts[m]->getLinkedFrom().size() > 0);
				//_linkButtons[rowcnt][i]->SetBackgroundColour ((linked ? (_activeBg) : (_defaultBg)));
				_linkButtons[rowcnt][i]->set_active (linked);				

				if (linkvec[rowcnt] && i!=0 && !linked) linkvec[rowcnt] = false;
	
			
				if (gridvec[rowcnt] && !_barGraphs[rowcnt][i]->getGridLines()) gridvec[rowcnt] = false;

				if (gridsnapvec[rowcnt] && !_barGraphs[rowcnt][i]->getGridSnap()) gridsnapvec[rowcnt] = false;

				if (_barGraphs[rowcnt][i]->getTempo() != engine->getTempo()) {
					_barGraphs[rowcnt][i]->setTempo(engine->getTempo());
				}
				
				
				rowcnt++;
			}

		}

		

		_bypassCheck[i]->SetValue(engine->getBypassed());
		if (!engine->getBypassed()) bypassed = false;
		
		_muteCheck[i]->SetValue(engine->getMuted());
		if (!engine->getMuted()) muted = false;


		// sliders and stuff
		_gainSpinCtrl[i]->SetValue ((int) (20 * FTutils::fast_log10 ( engine->getInputGain() )));
		_mixSlider[i]->SetValue ((int) ( engine->getMixRatio() * 1000 ));

		// plots
		active = _inputSpectragram[i]->getPlotType()==FTspectragram::SPECTRAGRAM;
		_inspecSpecTypeButton[i]->set_active (active);
		if (inspec && !active) inspec = false;

		active = _inputSpectragram[i]->getPlotType()==FTspectragram::AMPFREQ_SOLID;
		_inspecPlotSolidTypeButton[i]->set_active (active);
		if (inplotsolid && !active) inplotsolid = false;

		active = _inputSpectragram[i]->getPlotType()==FTspectragram::AMPFREQ_LINES;
		_inspecPlotLineTypeButton[i]->set_active (active);
		if (inplotline && !active) inplotline = false;

		active = _outputSpectragram[i]->getPlotType()==FTspectragram::SPECTRAGRAM;
		_outspecSpecTypeButton[i]->set_active(active);
		if (outspec && !active) outspec = false;

		active = _outputSpectragram[i]->getPlotType()==FTspectragram::AMPFREQ_SOLID;
		_outspecPlotSolidTypeButton[i]->set_active(active);
		if (outplotsolid && !active) outplotsolid = false;

		active = _outputSpectragram[i]->getPlotType()==FTspectragram::AMPFREQ_LINES;
		_outspecPlotLineTypeButton[i]->set_active(active);
		if (outplotline && !active) outplotline = false;

	}



	for (unsigned int n=0; n < _bypassAllButtons.size(); ++n)
	{
		_bypassAllButtons[n]->set_active (bypvec[n]);
		_linkAllButtons[n]->set_active (linkvec[n]);

		_gridButtons[n]->set_active (gridvec[n]);
		_gridSnapButtons[n]->set_active (gridsnapvec[n]);
	}

	
	_bypassAllCheck->SetValue (bypassed);
	_muteAllCheck->SetValue (muted);
	_linkMixCheck->SetValue (_linkedMix);

	_inspecSpecTypeAllButton->set_active (inspec);
	_inspecPlotLineTypeAllButton->set_active (inplotline);
	_inspecPlotSolidTypeAllButton->set_active (inplotsolid);
	_outspecSpecTypeAllButton->set_active (outspec);
	_outspecPlotLineTypeAllButton->set_active (outplotline);
	_outspecPlotSolidTypeAllButton->set_active (outplotsolid);

	
	_pathCountChoice->SetSelection(_pathCount - 1);

	if (_processPath[0]) {

		FTspectralEngine * engine = _processPath[0]->getSpectralEngine();
		
		if (engine->getUpdateSpeed() == FTspectralEngine::SPEED_TURTLE)
			_plotSpeedChoice->SetSelection(0);
		else if (engine->getUpdateSpeed() == FTspectralEngine::SPEED_SLOW)
			_plotSpeedChoice->SetSelection(1);
		else if (engine->getUpdateSpeed() == FTspectralEngine::SPEED_MED)
			_plotSpeedChoice->SetSelection(2);
		else if (engine->getUpdateSpeed() == FTspectralEngine::SPEED_FAST)
			_plotSpeedChoice->SetSelection(3);

		_windowingChoice->SetSelection(engine->getWindowing());
		
		const int * fftbins = FTspectralEngine::getFFTSizes();
		for (int i=0; i < FTspectralEngine::getFFTSizeCount(); i++) {
			if (fftbins[i] == engine->getFFTsize()) {
				_freqBinsChoice->SetSelection(i);
				break;
			}
		}
		
		// hack
		for (int i=0; i < 5; i++) {
			if (engine->getOversamp() == 1<<i) {
				_overlapChoice->SetSelection(i);
				break;
			}
		}

 		for (unsigned int i=0; i < _delayList.size(); ++i) {
  			if (engine->getMaxDelay() == _delayList[i]) {
 				_maxDelayChoice->SetSelection(i);
 				break;
 			}
 		}

		_tempoSpinCtrl->SetValue(engine->getTempo());
		
	}
	
	_ioNameText->SetValue (wxString::FromAscii (iosup->getName()));
	
	
	// reset timer just in case
	_updateMS = 10; // start small
	_eventTimer->Stop();
	_eventTimer->Start(_updateMS, FALSE);
	
}


// event handlers


void FTmainwin::handleInputButton(wxCommandEvent &event)
{
   wxObject *source = event.GetEventObject();

   wxString label(wxT("No Input"));
   
   for (int i=0; i < _pathCount; i++) {
	   if (!_processPath[i]) continue;
	   
	   if (source == _inputButton[i]) {
		   FTportSelectionDialog *dial = new FTportSelectionDialog(this, wxNewId(), i,
									   FTportSelectionDialog::INPUT, wxT("Input Port Selection"));
		   dial->update();
		   dial->SetSize(wxSize(190,190));
		   dial->CentreOnParent();
		   if (dial->ShowModal() == wxID_OK) {
				std::vector<wxString> pnames = dial->getSelectedPorts();
				FTioSupport::instance()->disconnectPathInput(i, NULL); // disconnect all
				for (unsigned j=0; j<pnames.size(); j++) {
					FTioSupport::instance()->connectPathInput(i, pnames[j].mb_str());
				}
		   }
		   dial->Close();
	   }
   }

   updateDisplay();
   
}

void FTmainwin::handleOutputButton(wxCommandEvent &event)
{
   wxObject *source = event.GetEventObject();

   wxString label(wxT("No Output"));
   
   for (int i=0; i < _pathCount; i++) {
	   if (!_processPath[i]) continue;

	   if (source == _outputButton[i]) {
		   FTportSelectionDialog *dial = new FTportSelectionDialog(this, wxNewId(), i,
									   FTportSelectionDialog::OUTPUT, wxT("Output Port Selection"));
		   dial->update();
		   dial->SetSize(wxSize(190,190));
		   dial->CentreOnParent();
		   if (dial->ShowModal() == wxID_OK) {
				std::vector<wxString> pnames = dial->getSelectedPorts();
				FTioSupport::instance()->disconnectPathOutput(i, NULL); // disconnect all
				for (unsigned j=0; j<pnames.size(); j++) {
					FTioSupport::instance()->connectPathOutput(i, pnames[j].mb_str());
				}
		   }

		   dial->Close();
	   }
   }

   updateDisplay();

}



void FTmainwin::bypass_clicked_events (int button, PixButton * source)
{
   bool alldone = false;
   
   for (int i=0; i < _pathCount; i++)
   {
	   if (!_processPath[i]) continue;
	   
	   bool done = false;
	   
	   FTspectralEngine *engine =  _processPath[i]->getSpectralEngine();
	   vector<FTprocI *> procmods;
	   engine->getProcessorModules (procmods);
	   int rowcnt=-1; // preincremented below
	   
	   for (unsigned int n=0; n < procmods.size(); ++n)
	   {
		   FTprocI *pm = procmods[n];
		   vector<FTspectrumModifier *> filts;
		   pm->getFilters (filts);
		   int lastgroup = -1;
		   
		   for (unsigned int m=0; m < filts.size(); ++m)
		   {
			   if (filts[m]->getGroup() != lastgroup) {
				   rowcnt++;
				   lastgroup = filts[m]->getGroup();
			   }

			   
			   if (source == _bypassButtons[rowcnt][i])
			   {
				   filts[m]->setBypassed ( ! filts[m]->getBypassed() );
				   _barGraphs[rowcnt][i]->setBypassed (filts[m]->getBypassed());
				   _bypassButtons[rowcnt][i]->set_active(filts[m]->getBypassed());
				   alldone = done = true;
			   }
 			   else if (source == _bypassAllButtons[rowcnt]) {
 				   filts[m]->setBypassed ( !_bypassAllButtons[rowcnt]->get_active());
 				   _barGraphs[rowcnt][i]->setBypassed (filts[m]->getBypassed());
 				   done = true;
 			   }
		   }

		   if (done) break;
	   }

	   if (alldone) break;
   }

   updateDisplay();

}

void FTmainwin::handleBypassButtons (wxCommandEvent &event)
{
   wxObject *source = event.GetEventObject();

   bool alldone = false;

   for (int i=0; i < _pathCount; i++)
   {
	   if (!_processPath[i]) continue;
	   
	   bool done = false;
	   
	   FTspectralEngine *engine =  _processPath[i]->getSpectralEngine();
	   vector<FTprocI *> procmods;
	   engine->getProcessorModules (procmods);
	   int rowcnt=-1; // preincremented below
	   
	   for (unsigned int n=0; n < procmods.size(); ++n)
	   {
		   FTprocI *pm = procmods[n];
		   vector<FTspectrumModifier *> filts;
		   pm->getFilters (filts);
		   int lastgroup = -1;
		   
		   for (unsigned int m=0; m < filts.size(); ++m)
		   {
			   if (filts[m]->getGroup() != lastgroup) {
				   rowcnt++;
				   lastgroup = filts[m]->getGroup();
			   }

			   
// 			   if (source == _bypassButtons[rowcnt][i])
// 			   {
// 				   filts[m]->setBypassed ( ! filts[m]->getBypassed() );
// 				   _barGraphs[rowcnt][i]->setBypassed (filts[m]->getBypassed());
// 				   alldone = done = true;
// 			   }
// 			   if (source == _bypassAllButtons[rowcnt]) {
// 				   filts[m]->setBypassed ( _bypassAllButtons[rowcnt]->GetBackgroundColour() != _activeBg);
// 				   _barGraphs[rowcnt][i]->setBypassed (filts[m]->getBypassed());
// 				   done = true;
// 			   }
			   if (source == _bypassAllCheck) {
				   engine->setBypassed( _bypassAllCheck->GetValue());
				   if (engine->getBypassed()) {
					   _updateTokens[i]->setIgnore(true);
				   }
				   else {
					   _updateTokens[i]->setIgnore(false);
				   }

				   done = true;
				   break;
			   }
			   else if (source == _bypassCheck[i]) {
				   engine->setBypassed( _bypassCheck[i]->GetValue());
				   if (engine->getBypassed()) {
					   _updateTokens[i]->setIgnore(true);
				   }
				   else {
					   _updateTokens[i]->setIgnore(false);
				   }

				   alldone = done = true;
				   break;
			   }
			   else if (source == _muteAllCheck) {
				   engine->setMuted( _muteAllCheck->GetValue());
				   done = true;
				   break;
			   }
			   else if (source == _muteCheck[i]) {
				   engine->setMuted( _muteCheck[i]->GetValue());
				   done = alldone = true;
				   break;
			   }

		   }

		   if (done) break;
	   }

	   if (alldone) break;
	   

   }

   updateDisplay();
}

void FTmainwin::handlePlotTypeButtons (wxCommandEvent &event)
{
   wxObject *source = event.GetEventObject();

   for (int i=0; i < _pathCount; i++) {
	   if (!_processPath[i]) continue;

	   if (source == _inspecSpecTypeButton[i]) {
		   _inputSpectragram[i]->setPlotType (FTspectragram::SPECTRAGRAM);
		   break;
	   }
	   else if (source == _inspecPlotSolidTypeButton[i]) {
		   _inputSpectragram[i]->setPlotType (FTspectragram::AMPFREQ_SOLID);
		   break;
	   }
	   else if (source == _inspecPlotLineTypeButton[i]) {
		   _inputSpectragram[i]->setPlotType (FTspectragram::AMPFREQ_LINES);
		   break;
	   }
	   else if (source == _outspecSpecTypeButton[i]) {
		   _outputSpectragram[i]->setPlotType (FTspectragram::SPECTRAGRAM);
		   break;
	   }
	   else if (source == _outspecPlotSolidTypeButton[i]) {
		   _outputSpectragram[i]->setPlotType (FTspectragram::AMPFREQ_SOLID);
		   break;
	   }
	   else if (source == _outspecPlotLineTypeButton[i]) {
		   _outputSpectragram[i]->setPlotType (FTspectragram::AMPFREQ_LINES);
		   break;
	   }

	   // all, don't break
 	   else if (source == _inspecSpecTypeAllButton) {
		   _inputSpectragram[i]->setPlotType (FTspectragram::SPECTRAGRAM);
 	   }
 	   else if (source == _inspecPlotSolidTypeAllButton) {
		   _inputSpectragram[i]->setPlotType (FTspectragram::AMPFREQ_SOLID);
 	   }
 	   else if (source == _inspecPlotLineTypeAllButton) {
		   _inputSpectragram[i]->setPlotType (FTspectragram::AMPFREQ_LINES);
 	   }
 	   else if (source == _outspecSpecTypeAllButton) {
		   _outputSpectragram[i]->setPlotType (FTspectragram::SPECTRAGRAM);
 	   }
 	   else if (source == _outspecPlotSolidTypeAllButton) {
		   _outputSpectragram[i]->setPlotType (FTspectragram::AMPFREQ_SOLID);
 	   }
 	   else if (source == _outspecPlotLineTypeAllButton) {
		   _outputSpectragram[i]->setPlotType (FTspectragram::AMPFREQ_LINES);
 	   }

   }

   updateDisplay();
}

void FTmainwin::plot_clicked_events (int button, PixButton * source)
{

   for (int i=0; i < _pathCount; i++) {
	   if (!_processPath[i]) continue;

	   if (source == _inspecSpecTypeButton[i]) {
		   _inputSpectragram[i]->setPlotType (FTspectragram::SPECTRAGRAM);
		   break;
	   }
	   else if (source == _inspecPlotSolidTypeButton[i]) {
		   _inputSpectragram[i]->setPlotType (FTspectragram::AMPFREQ_SOLID);
		   break;
	   }
	   else if (source == _inspecPlotLineTypeButton[i]) {
		   _inputSpectragram[i]->setPlotType (FTspectragram::AMPFREQ_LINES);
		   break;
	   }
	   else if (source == _outspecSpecTypeButton[i]) {
		   _outputSpectragram[i]->setPlotType (FTspectragram::SPECTRAGRAM);
		   break;
	   }
	   else if (source == _outspecPlotSolidTypeButton[i]) {
		   _outputSpectragram[i]->setPlotType (FTspectragram::AMPFREQ_SOLID);
		   break;
	   }
	   else if (source == _outspecPlotLineTypeButton[i]) {
		   _outputSpectragram[i]->setPlotType (FTspectragram::AMPFREQ_LINES);
		   break;
	   }

	   // all, don't break
 	   else if (source == _inspecSpecTypeAllButton) {
		   _inputSpectragram[i]->setPlotType (FTspectragram::SPECTRAGRAM);
 	   }
 	   else if (source == _inspecPlotSolidTypeAllButton) {
		   _inputSpectragram[i]->setPlotType (FTspectragram::AMPFREQ_SOLID);
 	   }
 	   else if (source == _inspecPlotLineTypeAllButton) {
		   _inputSpectragram[i]->setPlotType (FTspectragram::AMPFREQ_LINES);
 	   }
 	   else if (source == _outspecSpecTypeAllButton) {
		   _outputSpectragram[i]->setPlotType (FTspectragram::SPECTRAGRAM);
 	   }
 	   else if (source == _outspecPlotSolidTypeAllButton) {
		   _outputSpectragram[i]->setPlotType (FTspectragram::AMPFREQ_SOLID);
 	   }
 	   else if (source == _outspecPlotLineTypeAllButton) {
		   _outputSpectragram[i]->setPlotType (FTspectragram::AMPFREQ_LINES);
 	   }

   }

   updateDisplay();
}



void FTmainwin::link_clicked_events (int button, PixButton * source)
{
   vector<FTprocI *> firstprocmods;
   
   bool alldone = false;
   for (int i=0; i < _pathCount; i++) {
	   if (!_processPath[i]) continue;
	   
	   FTspectralEngine *engine =  _processPath[i]->getSpectralEngine();

	   bool done = false;
	   
	   vector<FTprocI *> procmods;
	   engine->getProcessorModules (procmods);
	   if (i==0) {
		   engine->getProcessorModules (firstprocmods);
	   }
	   
	   int rowcnt=-1; // preincremented below
	   
	   for (unsigned int n=0; n < procmods.size(); ++n)
	   {
		   FTprocI *pm = procmods[n];
		   vector<FTspectrumModifier *> filts;
		   pm->getFilters (filts);
		   int lastgroup = -1;
		   
		   for (unsigned int m=0; m < filts.size(); ++m)
		   {
			   if (filts[m]->getGroup() != lastgroup) {
				   rowcnt++;
				   lastgroup = filts[m]->getGroup();
			   }

			   if (source == _linkButtons[rowcnt][i])
			   {
				   FTlinkMenu *menu = new FTlinkMenu (_linkButtons[rowcnt][i], this, engine,
								      filts[m]->getSpecModifierType(), n, m);
				   _linkButtons[rowcnt][i]->PopupMenu (menu, 0, 0);

				   alldone = done = true;
				   break;
			   }
			   else if (source == _linkAllButtons[rowcnt])
			   {
				   if (_linkAllButtons[rowcnt]->get_active()) {
					   // unlink
					   filts[m]->unlink();
				   }
				   else if (i != 0) {
					   // link to first path (unless we are the first)
					   filts[m]->link (firstprocmods[n]->getFilter(m));
				   }
				   else {
					   // unlink we are the master
					   filts[m]->unlink();
				   }
				   updateGraphs(0, filts[m]->getSpecModifierType());

				   done = true;
			   }

		   }

		   if (done) break;

	   }

	   if (alldone) break;

// 	   if (source == _linkMixButton) {
// 		   // toggle it
// 		   _linkedMix = (_linkMixButton->GetBackgroundColour() != _activeBg);
// 	   }
	   
   }

   updateDisplay();
   
}


void FTmainwin::handleLinkButtons (wxCommandEvent &event)
{
   wxObject *source = event.GetEventObject();

   vector<FTprocI *> firstprocmods;
   
   bool alldone = false;
   
   for (int i=0; i < _pathCount; i++) {
	   if (!_processPath[i]) continue;
	   
	   FTspectralEngine *engine =  _processPath[i]->getSpectralEngine();

	   bool done = false;
	   
	   vector<FTprocI *> procmods;
	   engine->getProcessorModules (procmods);
	   if (i==0) {
		   engine->getProcessorModules (firstprocmods);
	   }
	   
	   int rowcnt=-1; // preincremented below
	   
	   for (unsigned int n=0; n < procmods.size(); ++n)
	   {
		   FTprocI *pm = procmods[n];
		   vector<FTspectrumModifier *> filts;
		   pm->getFilters (filts);
		   int lastgroup = -1;
		   
		   for (unsigned int m=0; m < filts.size(); ++m)
		   {
			   if (filts[m]->getGroup() != lastgroup) {
				   rowcnt++;
				   lastgroup = filts[m]->getGroup();
			   }

			   if (source == _linkAllButtons[rowcnt])
			   {
				   if (_linkAllButtons[rowcnt]->GetBackgroundColour() == _activeBg) {
					   // unlink
					   filts[m]->unlink();
				   }
				   else if (i != 0) {
					   // link to first path (unless we are the first)
					   filts[m]->link (firstprocmods[n]->getFilter(m));
				   }
				   else {
					   // unlink we are the master
					   filts[m]->unlink();
				   }
				   updateGraphs(0, filts[m]->getSpecModifierType());

				   done = true;
			   }

		   }

		   if (done) break;

	   }

	   if (alldone) break;

	   if (source == _linkMixCheck) {
		   // toggle it
		   _linkedMix = (_linkMixCheck->GetValue());
	   }
	   
   }

   updateDisplay();
   
}

void FTmainwin::handleTitleMenuCmd (FTtitleMenuEvent & ev)
{

	if (ev.ready)
	{
		// recieved from title menu selections
		
		if (ev.cmdType == FTtitleMenuEvent::RemoveEvt)
		{
			doRemoveRow(ev.target);
		}
		else if (ev.cmdType == FTtitleMenuEvent::ExpandEvt
			 || ev.cmdType == FTtitleMenuEvent::MinimizeEvt)
		{
			
			doMinimizeExpand(ev.target);
		}
	}
	else {

		_pendingTitleEvents.push_back ((FTtitleMenuEvent *) ev.Clone());
	}
	
}


void FTmainwin::doRemoveRow (wxWindow * source)
{
	if (source == _inspecLabelButton || source == _inspecLabelButtonAlt
	    || source == _outspecLabelButton || source == _outspecLabelButtonAlt)
	{
		
		return;
	}


	FTspectralEngine *engine =  _processPath[0]->getSpectralEngine();
	vector<FTprocI *> procmods;
	engine->getProcessorModules (procmods);
	
	
	unsigned int rowcnt=0;
	int itemi = -1;
	bool done = false;
	for (unsigned int n=0; n < procmods.size(); ++n)
	{
		FTprocI *pm = procmods[n];
		vector<FTspectrumModifier *> filts;
		pm->getFilters (filts);
		int lastgroup = -1;
		
		for (unsigned int m=0; m < filts.size(); ++m)
		{
			if (filts[m]->getGroup() == lastgroup) {
				continue; // first fill do
			}
			lastgroup = filts[m]->getGroup();
			   
			if (source == _labelButtons[rowcnt] || source == _altLabelButtons[rowcnt]) {
				itemi = (int) n;
				done = true;
			}
			   
			rowcnt++;
		}
		   
		if (done) break;
	}
	   
	if (itemi >= 0)
	{
		//suspendProcessing();
		
		FTioSupport * iosup = FTioSupport::instance();
		// do this for every active process path
		
		FTprocessPath * procpath;
		for (int i=0; i < iosup->getActivePathCount(); ++i)
		{
			procpath = iosup->getProcessPath(i);
			if (!procpath) break;
			
			engine = procpath->getSpectralEngine();

			engine->removeProcessorModule ((unsigned int) itemi); 
		}

		rebuildDisplay(false);
		//restoreProcessing();

		if (_procmodDialog && _procmodDialog->IsShown()) {
			_procmodDialog->refreshState();
		}
		
	}
}

void FTmainwin::handleLabelButtons (wxCommandEvent &event)
{
	wxWindow *source = (wxWindow *) event.GetEventObject();

	doMinimizeExpand (source);
}

void FTmainwin::doMinimizeExpand (wxWindow * source)
{
	// remove sash from rowsizer and replace with button
	int itemi=-1;
	wxWindow * hidewin=0, *showwin=0;

	if (source == _inspecLabelButton || source == _inspecLabelButtonAlt) {
		if (_inspecSash->IsShown()) {
			hidewin = _inspecSash;
			showwin = _inspecLabelButtonAlt;
			itemi = 0;
		}
		else {
			hidewin = _inspecLabelButtonAlt;
			showwin = _inspecSash;
			itemi = 0;
		}
	}
	else if (source == _outspecLabelButton || source == _outspecLabelButtonAlt) {
		if (_outspecSash->IsShown()) {
			hidewin = _outspecSash;
			showwin = _outspecLabelButtonAlt;
			itemi = _rowItems.size()-1;
		}
		else {
			hidewin = _outspecLabelButtonAlt;
			showwin = _outspecSash;
			itemi = _rowItems.size()-1;
		}
	}		
	else
	{
		for (unsigned int n=0; n < _labelButtons.size(); ++n) {
			
			if (source == _labelButtons[n] || source == _altLabelButtons[n]) {
				
				if (_rowSashes[n]->IsShown()) {
					hidewin = _rowSashes[n];
					showwin = _altLabelButtons[n];
					itemi = n+1;
				}
				else {
					hidewin = _altLabelButtons[n];
					showwin = _rowSashes[n];
					itemi = n+1;
				}
				break;
			}
		}

	}

	minimizeRow (showwin, hidewin, itemi, true);

	updateAllExtra();
}

void FTmainwin::minimizeRow (wxWindow * showwin, wxWindow * hidewin, int itemi, bool layout)
{
	
	if (hidewin && showwin) {
		hidewin->Show(false);

		_rowItems[itemi] = showwin;

		if (itemi > 0) {
			_rowItems[itemi]->GetConstraints()->top.Below(_rowItems[itemi-1], 2);
		}

		if (itemi < (int) _rowItems.size()-1) {
			_rowItems[itemi+1]->GetConstraints()->top.Below(_rowItems[itemi], 2);
		}
		
		showwin->Show(true);

		if (layout) {
			_rowPanel->Layout();
			rowpanelScrollSize();
		}
	}

}


void FTmainwin::handleSpins (wxSpinEvent &event)
{
	wxObject *source = event.GetEventObject();

	if (source == _tempoSpinCtrl)
	{
		int tval = _tempoSpinCtrl->GetValue();
		for (int i=0; i < _pathCount; i++) {
			if (!_processPath[i]) continue;

			// set tempo of all, just in case
			for (unsigned int n=0; n < _barGraphs.size(); ++n)
			{
				_barGraphs[n][i]->setTempo (tval);
			}

			_processPath[i]->getSpectralEngine()->setTempo (tval); // purely for saving
		}

	}

}

void FTmainwin::handleChoices (wxCommandEvent &event)
{
	wxObject *source = event.GetEventObject();

	if (source == _freqBinsChoice) {
		int sel = _freqBinsChoice->GetSelection();

		// MUST bypass and wait until not working
		suspendProcessing();
		
		for (int i=0; i < _pathCount; i++) {
			if (!_processPath[i]) continue;

			FTspectralEngine *engine =  _processPath[i]->getSpectralEngine();

			// unset all specmods for safety
			for (unsigned int n=0; n < _barGraphs.size(); ++n)
			{
				_barGraphs[n][i]->setSpectrumModifier(0);
				_barGraphs[n][i]->setTopSpectrumModifier(0);
			}
			
			engine->setFFTsize ((FTspectralEngine::FFT_Size) (intptr_t) _freqBinsChoice->GetClientData(sel) );

			vector<FTprocI *> procmods;
			engine->getProcessorModules (procmods);
			
			// reset all the activeplots
			int rowcnt=-1;  // preincremented
			for (unsigned int n=0; n < procmods.size(); ++n)
			{
				FTprocI *pm = procmods[n];
				vector<FTspectrumModifier *> filts;
				pm->getFilters (filts);
				int lastgroup = -1;
				
				for (unsigned int m=0; m < filts.size(); ++m)
				{
					if (filts[m]->getGroup() != lastgroup) {
						rowcnt++;
						lastgroup = filts[m]->getGroup();
						_barGraphs[rowcnt][i]->setSpectrumModifier(filts[m]);
					}
					else {
						_barGraphs[rowcnt][i]->setTopSpectrumModifier(filts[m]);
					}

				}
			}

			_inputSpectragram[i]->setDataLength((unsigned int)_processPath[i]->getSpectralEngine()->getFFTsize() >> 1);
			_outputSpectragram[i]->setDataLength((unsigned int)_processPath[i]->getSpectralEngine()->getFFTsize() >> 1);
		}

		restoreProcessing();
		
		updateGraphs(0, ALL_SPECMOD);
	}
	else if (source == _overlapChoice) {
		int sel = _overlapChoice->GetSelection();
		for (int i=0; i < _pathCount; i++) {
			if (!_processPath[i]) continue;

			_processPath[i]->getSpectralEngine()->setOversamp( (intptr_t) _overlapChoice->GetClientData(sel) );
		}
		
	}
	else if (source == _windowingChoice) {
		int sel = _windowingChoice->GetSelection();
		for (int i=0; i < _pathCount; i++) {
			if (!_processPath[i]) continue;

			_processPath[i]->getSpectralEngine()->setWindowing( (FTspectralEngine::Windowing) sel );
		}
		
	}
	else if (source == _plotSpeedChoice) {
		int sel = _plotSpeedChoice->GetSelection();
		for (int i=0; i < _pathCount; i++) {
			if (!_processPath[i]) continue;

			_processPath[i]->getSpectralEngine()->setUpdateSpeed( (FTspectralEngine::UpdateSpeed)
									     (intptr_t) _plotSpeedChoice->GetClientData(sel) );
			if (i==0) {
				_updateMS = 10; // start small
				_eventTimer->Stop();
				_eventTimer->Start(_updateMS, FALSE);
			}
		}
		
	}
	else if (source == _superSmoothCheck) {
		_superSmooth = _superSmoothCheck->GetValue();
		if (_superSmooth) {
			_updateMS = 10; // start small
			_eventTimer->Stop();
			_eventTimer->Start(_updateMS, FALSE);
		}
	}
	else if (source == _maxDelayChoice) {
	        int sel = _maxDelayChoice->GetSelection();

		if (sel >= (int) _delayList.size()) return;

		float maxdelay = _delayList[sel];

		// MUST bypass and wait until not working
		suspendProcessing();
		
		// set the max delay
		for (int i=0; i < _pathCount; i++) {
			if (!_processPath[i]) continue;

			_processPath[i]->getSpectralEngine()->setMaxDelay (maxdelay);

			// TODO: restore delay graph
			for (unsigned int n=0; n < _barGraphs.size(); ++n) {
				_barGraphs[n][i]->refreshBounds();
			}

		}

		restoreProcessing();

		updateGraphs(0, DELAY_SPECMOD);
		
	}
	
}

void FTmainwin::grid_clicked_events (int button, PixButton * source)
{

   for (int i=0; i < _pathCount; i++) {
	   if (!_processPath[i]) continue;

	   FTspectralEngine *engine =  _processPath[i]->getSpectralEngine();
	   vector<FTprocI *> procmods;
	   engine->getProcessorModules (procmods);
	   
	   
	   unsigned int rowcnt=0;
	   bool done = false;
	   for (unsigned int n=0; n < procmods.size(); ++n)
	   {
		   FTprocI *pm = procmods[n];
		   vector<FTspectrumModifier *> filts;
		   pm->getFilters (filts);
		   int lastgroup = -1;
		   
		   for (unsigned int m=0; m < filts.size(); ++m)
		   {
			   // master gridlines buttons
			   if (filts[m]->getGroup() == lastgroup) {
				   continue; // first fill do
			   }
			   lastgroup = filts[m]->getGroup();
			   
			   if (source == _gridButtons[rowcnt]) {
				   _barGraphs[rowcnt][i]->setGridLines (!_gridButtons[rowcnt]->get_active());
				   done = true;
			   }
			   else if (source == _gridSnapButtons[rowcnt]) {
				   _barGraphs[rowcnt][i]->setGridSnap (!_gridSnapButtons[rowcnt]->get_active());
				   done = true;
			   }
			   
			   rowcnt++;
		   }

		   if (done) break;
	   }
	   
   }

   updateDisplay();
}


void FTmainwin::handleGridButtons (wxCommandEvent &event)
{
   wxObject *source = event.GetEventObject();

   for (int i=0; i < _pathCount; i++) {
	   if (!_processPath[i]) continue;

	   FTspectralEngine *engine =  _processPath[i]->getSpectralEngine();
	   vector<FTprocI *> procmods;
	   engine->getProcessorModules (procmods);
	   
	   
	   unsigned int rowcnt=0;
	   bool done = false;
	   for (unsigned int n=0; n < procmods.size(); ++n)
	   {
		   FTprocI *pm = procmods[n];
		   vector<FTspectrumModifier *> filts;
		   pm->getFilters (filts);
		   int lastgroup = -1;
		   
		   for (unsigned int m=0; m < filts.size(); ++m)
		   {
			   // master gridlines buttons
			   if (filts[m]->getGroup() == lastgroup) {
				   continue; // first fill do
			   }
			   lastgroup = filts[m]->getGroup();
			   
			   if (source == _gridButtons[rowcnt]) {
				   _barGraphs[rowcnt][i]->setGridLines (_gridButtons[rowcnt]->GetBackgroundColour() != _activeBg);
				   done = true;
			   }
			   else if (source == _gridSnapButtons[rowcnt]) {
				   _barGraphs[rowcnt][i]->setGridSnap (_gridSnapButtons[rowcnt]->GetBackgroundColour() != _activeBg);
				   done = true;
			   }
			   
			   rowcnt++;
		   }

		   if (done) break;
	   }
	   
   }

   updateDisplay();
}

void FTmainwin::handleGridButtonMouse (wxMouseEvent &event)
{

}

void FTmainwin::grid_pressed_events (int button, PixButton * source)
{
	if (button != PixButton::RightButton) return;

	vector<FTactiveBarGraph*> graphs;
	FTspectrumModifier::ModifierType mtype = FTspectrumModifier::NULL_MODIFIER;
	
	for (int i=0; i < _pathCount; i++) {
		if (!_processPath[i]) continue;

		FTspectralEngine * engine = _processPath[i]->getSpectralEngine();
		vector<FTprocI *> procmods;
		engine->getProcessorModules (procmods);
		
		
		unsigned int rowcnt=0;
		bool done = false;
		for (unsigned int n=0; n < procmods.size(); ++n)
		{
			FTprocI *pm = procmods[n];
			vector<FTspectrumModifier *> filts;
			pm->getFilters (filts);
			int lastgroup = -1;
			
			for (unsigned int m=0; m < filts.size(); ++m)
			{
				if (filts[m]->getGroup() == lastgroup) {
					continue; // first will do
				}
				lastgroup = filts[m]->getGroup();
				
				if (source == _gridButtons[rowcnt]) {
					mtype = filts[m]->getModifierType();
					graphs.push_back (_barGraphs[rowcnt][i]);

					done = true;
					break;
				}
				rowcnt++;
			}
			
			if (done) break;
		}
		
	}
	
	if (graphs.size() > 0) {
		FTgridMenu *menu = new FTgridMenu (source, this, graphs, mtype);
		source->PopupMenu (menu, 0, source->GetSize().GetHeight());
		delete menu;
	}
}

void FTmainwin::handleTitleButtonMouse (wxMouseEvent &event, bool minimized)
{

	wxWindow *source = (wxWindow *) event.GetEventObject();

	
	FTtitleMenu *menu = new FTtitleMenu (source, this, minimized);
	source->PopupMenu (menu, 0, source->GetSize().GetHeight());
	delete menu;

	// now repost pending title events
	// XXX: we do this because one of the actions
	// actually deletes the source of this event,
	// and the first pending event was posted in the popupmenu's idle
	// context.  what a pain.
	for (vector<FTtitleMenuEvent *>::iterator evt = _pendingTitleEvents.begin();
	     evt != _pendingTitleEvents.end() ; ++evt)
	{
		(*evt)->ready = true;
		AddPendingEvent (*(*evt));
		delete (*evt);
	}
	_pendingTitleEvents.clear();
	
	event.Skip();
}

void FTmainwin::handleIOButtons (wxCommandEvent &event)
{
	FTioSupport *iosup = FTioSupport::instance();

	if (event.GetId() == FT_IOreconnectButton)
	{
		if (! iosup->isInited() || (_ioNameText->GetValue() != wxString::FromAscii (iosup->getName())))
		{
			fprintf(stderr, "Reconnecting as %s...\n", static_cast<const char *>(_ioNameText->GetValue().mb_str()));
			
			iosup->stopProcessing();
			wxThread::Sleep(200);
			iosup->close();
			
			iosup->setName (static_cast<const char *> (_ioNameText->GetValue().mb_str()));
			if (iosup->init()) {
				if (iosup->startProcessing()) {
					iosup->reinit();
				}
			}

			updateDisplay();
		}
	}
	else if (event.GetId() == FT_IOdisconnectButton)
	{
		if (iosup->isInited()) {
			iosup->stopProcessing();
			wxThread::Sleep(200);
			iosup->close();
		}
	}

}



void FTmainwin::rebuildDisplay(bool dolink)
{

	wxWindow *lastsash = _inspecSash;
	
	int rowcnt=0;

	FTspectralEngine * engine = _processPath[0]->getSpectralEngine();
	vector<FTprocI *> pmods;
	engine->getProcessorModules (pmods);

	
	int initrows = _rowSizers.size();
	
	for (unsigned int n=0; n < pmods.size(); ++n)
	{
		FTprocI *pm = pmods[n];
		vector<FTspectrumModifier *> filts;
		pm->getFilters (filts);
		int lastgroup = -1;
		
		for (unsigned int m=0; m < filts.size(); ++m)
		{
			// if the group is different from the last
			// make a new row for it
			if (filts[m]->getGroup() == lastgroup) {
				continue;
			}
			lastgroup = filts[m]->getGroup();

			if (rowcnt >= initrows)
			{
				// we need to make new stuff
				pushProcRow(filts[m]);

				lastsash = _rowSashes[rowcnt];
			}
			else {

						
				wxSashLayoutWindow * sash = _rowSashes[rowcnt];
				
				// main label button
				string name = filts[m]->getName();
				_labelButtons[rowcnt]->SetLabel (wxString::FromAscii (name.c_str()));
				_labelButtons[rowcnt]->SetToolTip(wxString(wxT("Hide ")) + wxString::FromAscii (name.c_str()));

				// alt label button
				_altLabelButtons[rowcnt]->SetLabel (wxString::FromAscii (name.c_str()));
				_altLabelButtons[rowcnt]->SetToolTip(wxString(wxT("Hide ")) + wxString::FromAscii (name.c_str()));


				lastsash = sash;
			}
			
			rowcnt++;
		}
	}


	if (rowcnt < (int) _rowSizers.size()) {
		// we need to pop some rows off
		int diff = _rowSizers.size() - rowcnt;
		
		for (int i=0; i < diff; i++) {

			//printf ("pop a row off: %d\n", diff);
			popProcRow();
		}
	}
	else {
	
		//_outspecSash->GetConstraints()->top.SameAs (lastsash, wxBottom, 2);
		_outspecSash->GetConstraints()->top.Below (lastsash, 2);
		_outspecLabelButtonAlt->GetConstraints()->top.Below(lastsash, 2);
		
		_rowPanel->Layout();
		rowpanelScrollSize();
	}
	

	for (int i=0; i < _pathCount; i++)
	{
		if (!_processPath[i]) continue; // shouldnt happen
		
		engine = _processPath[i]->getSpectralEngine();
		vector<FTprocI *> procmods;
		engine->getProcessorModules (procmods);
		
		// change the plot stuff
		_inputSpectragram[i]->setDataLength((unsigned int)engine->getFFTsize() >> 1);
		_outputSpectragram[i]->setDataLength((unsigned int)engine->getFFTsize() >> 1);

		
		int rowcnt=-1; // preincremented below
	
		for (unsigned int n=0; n < procmods.size(); ++n)
		{
			FTprocI *pm = procmods[n];
			vector<FTspectrumModifier *> filts;
			pm->getFilters (filts);
			int lastgroup = -1;
			
			for (unsigned int m=0; m < filts.size(); ++m)
			{
				wxPanel ** rowpanels = 0;
				FTactiveBarGraph **bargraphs = 0;
				bool newrow = false;

				if (filts[m]->getGroup() != lastgroup)
				{				
					rowcnt++;

					rowpanels = _subrowPanels[rowcnt];
					bargraphs = _barGraphs[rowcnt];

					if (!bargraphs[i]) {
						// only if brand new row
						rowpanels[i] = new wxPanel (_rowPanels[rowcnt], -1);
						rowpanels[i]->SetBackgroundColour(*wxBLACK);
						rowpanels[i]->SetThemeEnabled(false);
						bargraphs[i] = new FTactiveBarGraph(this, rowpanels[i], -1);
						newrow = true;
					}

					bargraphs[i]->setSpectrumModifier (filts[m]);
					bargraphs[i]->setTopSpectrumModifier (0);
					bargraphs[i]->setBypassed (filts[m]->getBypassed());
					bargraphs[i]->setTempo(_tempoSpinCtrl->GetValue());
					bargraphs[i]->readExtra(filts[m]);
					bargraphs[i]->refreshBounds();

					
					XMLNode * extra = filts[m]->getExtraNode();
					XMLProperty * prop;

					// get stuff from extra node
					if ((prop = extra->property ("height"))) {
						wxString val(wxString::FromAscii (prop->value().c_str()));
						unsigned long newh;
						if (val.ToULong (&newh)) {
							wxLayoutConstraints *cnst = _rowSashes[rowcnt]->GetConstraints();
							if (newh < 36) newh = 36;
							
							cnst->height.Absolute (newh);
						}
					}

					if ((prop = extra->property ("minimized"))) {
						wxString val(wxString::FromAscii (prop->value().c_str()));
						unsigned long newh;
						if (val.ToULong (&newh)) {
							if (newh) {
								// minimize
								minimizeRow (_altLabelButtons[rowcnt], _rowSashes[rowcnt], rowcnt+1, false);
							}
							else {
								// un-minimize
								minimizeRow (_rowSashes[rowcnt], _altLabelButtons[rowcnt], rowcnt+1, false);
							}
						}
					}

					
					if (dolink) {
						// link it to first
						filts[m]->link (pmods[n]->getFilter(m));
					}
					
					lastgroup = filts[m]->getGroup();
				}
				else 
				{
					// assign nth filter to already created plot
					// TODO: support more than 2
					rowpanels = _subrowPanels[rowcnt];
					bargraphs = _barGraphs[rowcnt];
					bargraphs[i]->setTopSpectrumModifier (filts[m]);
					bargraphs[i]->readExtra(filts[m]);
					bargraphs[i]->refreshBounds();

					if (dolink) {
						// link it to first
						filts[m]->link (pmods[n]->getFilter(m));
					}
					
					continue;
				}


				if (newrow) {

					wxBoxSizer * buttsizer = new wxBoxSizer(wxVERTICAL);
					PixButton * pixbutt;
					pixbutt = new PixButton(rowpanels[i], FT_BypassBase, false, wxDefaultPosition, wxSize(_bwidth, -1));

					pixbutt->set_normal_bitmap (wxBitmap(bypass_normal_xpm));
					pixbutt->set_selected_bitmap (wxBitmap(bypass_selected_xpm));
					pixbutt->set_focus_bitmap (wxBitmap(bypass_focus_xpm));
					pixbutt->set_disabled_bitmap (wxBitmap(bypass_disabled_xpm));
					pixbutt->set_active_bitmap (wxBitmap(bypass_active_xpm));
					
					_bypassButtons[rowcnt][i] = pixbutt;
					//buttsizer->Add ( _bypassButtons[rowcnt][i] = new wxButton(rowpanels[i], FT_BypassBase, wxT("B"),
					//							  wxDefaultPosition, wxSize(_bwidth,-1)), 1, 0,0);
					pixbutt->clicked.connect (bind (slot (*this, &FTmainwin::bypass_clicked_events), pixbutt));
					
					buttsizer->Add (pixbutt, 0, 0, 0);
					_bypassButtons[rowcnt][i]->SetFont(_buttFont);
					_bypassButtons[rowcnt][i]->SetToolTip(wxT("Toggle Bypass"));

					
					pixbutt = new PixButton(rowpanels[i], FT_LinkBase, false, wxDefaultPosition, wxSize(_bwidth, -1));

					pixbutt->set_normal_bitmap (wxBitmap(link_normal_xpm));
					pixbutt->set_selected_bitmap (wxBitmap(link_selected_xpm));
					pixbutt->set_focus_bitmap (wxBitmap(link_focus_xpm));
					pixbutt->set_disabled_bitmap (wxBitmap(link_disabled_xpm));
					pixbutt->set_active_bitmap (wxBitmap(link_active_xpm));
					
					_linkButtons[rowcnt][i] = pixbutt;
					//buttsizer->Add ( _linkButtons[rowcnt][i] = new wxButton(rowpanels[i], FT_LinkBase, wxT("L"),
					//							wxDefaultPosition, wxSize(_bwidth,-1)), 0, wxTOP,3);
					buttsizer->Add (_linkButtons[rowcnt][i], 0, wxTOP, 3);
					
					_linkButtons[rowcnt][i]->SetFont(_buttFont);
					_linkButtons[rowcnt][i]->SetToolTip(wxT("Link"));		
					
					wxBoxSizer * tmpsizer = new wxBoxSizer(wxHORIZONTAL);
					tmpsizer->Add ( buttsizer, 0, wxALL|wxEXPAND, 1);
					tmpsizer->Add ( bargraphs[i],  1, wxALL|wxEXPAND, 1);
					
					
					rowpanels[i]->SetAutoLayout(TRUE);
					tmpsizer->Fit( rowpanels[i] );  
					rowpanels[i]->SetSizer(tmpsizer);
					
					_rowSizers[rowcnt]->Insert (i+1, rowpanels[i],  1, wxLEFT|wxEXPAND, 4);
					_rowSizers[rowcnt]->Layout();
				}

			}

		}

		
	}

	//_rowPanel->Layout();
	//rowpanelScrollSize();
	
	updateGraphs(0, ALL_SPECMOD);

	updateDisplay();

	if (_blendDialog) {
		_blendDialog->refreshState();
	}
}

void FTmainwin::handlePathCount (wxCommandEvent &event)
{
	int newcnt = _pathCountChoice->GetSelection() + 1;

	changePathCount (newcnt, true);
}

void FTmainwin::changePathCount (int newcnt, bool rebuild, bool ignorelink)
{
	FTioSupport *iosup = FTioSupport::instance();

	// change path count
	if (newcnt < _pathCount)
	{
		// get rid of the some
		for (int i=_pathCount-1; i >= newcnt; i--)
		{
			if (!_processPath[i]) continue; // shouldnt happen

			// remove all stuff
			// processpath deactivateed in here too
			removePathStuff(i);

		}
	}
	else if (newcnt > _pathCount)
	{
		if (rebuild) {
			// rebuild first with smaller number
			rebuildDisplay(!ignorelink);
		}
		
		// add some
		for (int i=_pathCount; i < newcnt; i++)
		{
			// add to io support instance
			_processPath[i] = iosup->setProcessPathActive (i, true);

			createPathStuff(i);
		}
	}

	_pathCount = iosup->getActivePathCount();
	//printf ("new path cnt = %d\n", _pathCount);

	if (rebuild) {
		rebuildDisplay(!ignorelink);
	} else {
		updateDisplay();
	}
}

void FTmainwin::updateAllExtra ()
{
	FTspectralEngine * engine;

	// go through each specfilt and get its sash constraint height
	for (int i=0; i < _pathCount; i++)
	{
		if (!_processPath[i]) continue; // shouldnt happen

		engine = _processPath[i]->getSpectralEngine();
		vector<FTprocI *> procmods;
		engine->getProcessorModules (procmods);

		int rowcnt=-1; // preincremented below
	
		for (unsigned int n=0; n < procmods.size(); ++n)
		{
			FTprocI *pm = procmods[n];
			vector<FTspectrumModifier *> filts;
			pm->getFilters (filts);
			int lastgroup = -1;
		
			for (unsigned int m=0; m < filts.size(); ++m)
			{
				wxPanel ** rowpanels = 0;
				FTactiveBarGraph **bargraphs = 0;
			
				if (filts[m]->getGroup() != lastgroup)
				{				
					rowcnt++;

					rowpanels = _subrowPanels[rowcnt];
					bargraphs = _barGraphs[rowcnt];
					FTactiveBarGraph * graph = bargraphs[i];				
					wxSashLayoutWindow * sash = _rowSashes[rowcnt];

					wxLayoutConstraints *cnst = sash->GetConstraints();

					XMLNode * extra = filts[m]->getExtraNode();
				
					extra->add_property ("height", std::string (wxString::Format(wxT("%d"), cnst->height.GetValue()).mb_str()));
					extra->add_property ("minimized", std::string (wxString::Format(wxT("%d"), (int) !sash->IsShown()).mb_str()));

					//printf ("updating extra for %s : shown %d\n", filts[m]->getName().c_str(), (int) sash->IsShown());
					
					
					graph->writeExtra (filts[m]);

					lastgroup = filts[m]->getGroup();
					
				}
				else {
					continue;
				}
			}
		}
	}
}

void FTmainwin::handleSashDragged (wxSashEvent &event)
{
	wxObject *source = event.GetEventObject();

	wxSashLayoutWindow *sash = (wxSashLayoutWindow *) source;
	
	if (event.GetDragStatus() == wxSASH_STATUS_OK)
	{
		wxLayoutConstraints *cnst = sash->GetConstraints();
		wxRect bounds = event.GetDragRect();

		int newh = bounds.height;
		if (newh < 36) newh = 36;
		
		cnst->height.Absolute (newh);
		_rowPanel->Layout();
		rowpanelScrollSize();

		// update all extra settings
		updateAllExtra();
	}

}


void FTmainwin::handleGain (wxSpinEvent &event)
{
	wxObject *source = event.GetEventObject();

	for (int i=0; i < _pathCount; i++)
	{
		if (!_processPath[i]) continue;
		FTspectralEngine *engine =  _processPath[i]->getSpectralEngine();

		if (source == _gainSpinCtrl[i]) {
			// db scale
			float gain = pow(10, _gainSpinCtrl[i]->GetValue() / 20.0);
			// printf ("new gain is %g\n", gain);
			engine->setInputGain(gain);
		}
	}
}

void FTmainwin::handleMixSlider (wxScrollEvent &event)
{
	wxObject *source = event.GetEventObject();

	float newval = 1.0;
	
	for (int i=0; i < _pathCount; i++)
	{
		if (_mixSlider[i] == source) {
			newval = _mixSlider[i]->GetValue() / 1000.0;
			break;
		}
	}
		
	for (int i=0; i < _pathCount; i++)
	{
		if (!_processPath[i]) continue;
		FTspectralEngine *engine =  _processPath[i]->getSpectralEngine();

		if (_linkedMix || source == _mixSlider[i]) {
			// 0 -> 1000
			engine->setMixRatio( newval);
			_mixSlider[i]->SetValue( (int) (newval * 1000.0));
		}
	}
}


void FTmainwin::OnProcMod (wxCommandEvent &event)
{
	// popup our procmod dsp dialog

	if (!_procmodDialog) {
		_procmodDialog = new FTprocOrderDialog(this, -1, wxT("DSP Modules"));
		_procmodDialog->SetSize(372,228);
	}

	_procmodDialog->refreshState();

	_procmodDialog->Show(true);
	
}

void FTmainwin::OnPresetBlend (wxCommandEvent &event)
{
	// popup our preset blend dialog

	if (!_blendDialog) {
		_blendDialog = new FTpresetBlendDialog(this, &_configManager, -1, wxT("Preset Blend"));
		_blendDialog->SetSize(372,228);
	}

	_blendDialog->refreshState();

	_blendDialog->Show(true);
	
}


void FTmainwin::OnModulatorDialog (wxCommandEvent &event)
{
	// popup our modulator dialog
	if (!_modulatorDialog) {
		_modulatorDialog = new FTmodulatorDialog(this, -1, wxT("Modulations"));
		_modulatorDialog->SetSize(450,350);
	}

	//_modulatorDialog->refreshState();

	_modulatorDialog->Show(true);
}


void FTmainwin::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	// TRUE is to force the frame to close
	Close(TRUE);
}

void FTmainwin::OnAbout(wxCommandEvent& event)
{
	if (event.GetId() == FT_AboutMenu)
	{
		wxString msg(wxString(wxT("FreqTweak ")) +
			     wxString::FromAscii (freqtweak_version) +
			     wxT(" brought to you by Jesse Chappell"));
		
		wxMessageBox(msg, wxT("About FreqTweak"), wxOK | wxICON_INFORMATION, this);
	}
	else if (event.GetId() == FT_HelpTipsMenu)
	{
		FThelpWindow *helpwin = new FThelpWindow(this, -1, wxT("Usage Tips"));
		helpwin->Show(true);
	}
}


void FTmainwin::OnIdle(wxIdleEvent &event)
{
	//printf("OnIdle\n");
	if (_superSmooth) {
		::wxWakeUpIdle();
	}
}

void FTmainwin::OnClose(wxCloseEvent &event)
{
	cleanup();
	event.Skip();
}


void FTmainwin::cleanup ()
{
	// save default preset
	_configManager.storeSettings ("", true);
	
	//printf ("cleaning up\n");
	FTioSupport::instance()->close();
}

void FTmainwin::checkEvents()
{
	for (int i=0; i < _pathCount; i++)
	{
		if ( ! _updateTokens[i]->getIgnore()) {
			if (_updateTokens[i]->getUpdated(false)) {
				updatePlot (i);
			}
			else {
				// auto calibrate
				_eventTimer->Stop();
				_eventTimer->Start(++_updateMS, FALSE);
				// printf("%d\n", _updateMS);
			}
		}
	}
}

void FTmainwin::checkRefreshes()
{
	// TODO smartness
	updateGraphs(0, ALL_SPECMOD, true);
}

void FTmainwin::updatePlot (int plotid)
{
	FTspectralEngine *engine = _processPath[plotid]->getSpectralEngine();
	
	if (_inspecSash->IsShown()) {
		const float *inpower = engine->getRunningInputPower();
		_inputSpectragram[plotid]->plotNextData(inpower, engine->getFFTsize() >> 1);
	}

	if (_outspecSash->IsShown()) {
		const float *outpower = engine->getRunningOutputPower();
		_outputSpectragram[plotid]->plotNextData(outpower, engine->getFFTsize() >> 1);
	}
	
}


void FTmainwin::updateGraphs(FTactiveBarGraph *exclude, SpecModType smtype, bool  refreshonly)
{
	for (int i=0; i < _pathCount; i++)
	{
		if (!_processPath[i]) continue; 

		FTspectralEngine * engine = _processPath[i]->getSpectralEngine();
		vector<FTprocI *> procmods;
		engine->getProcessorModules (procmods);
		
		
		unsigned int rowcnt=0;
		bool done = false;
		for (unsigned int n=0; n < procmods.size(); ++n)
		{
			FTprocI *pm = procmods[n];
			vector<FTspectrumModifier *> filts;
			pm->getFilters (filts);
			int lastgroup = -1;
			
			for (unsigned int m=0; m < filts.size(); ++m)
			{

				if (rowcnt >= _rowSashes.size()) {
					done = true;
					break;
				}

				if (filts[m]->getGroup() == lastgroup) {
					continue; // first will do
				}
				lastgroup = filts[m]->getGroup();

				// prevent too many updates
				if ((!_rowSashes[rowcnt]->IsShown()) ||
				    (refreshonly && !filts[m]->getDirty())) {
					//cerr << "skipping " << filts[m]->getName() << " dirty: " << filts[m]->getDirty() << " refre: " << refreshonly<<  endl;
					rowcnt++;
					continue;
				}
				
				if (smtype == ALL_SPECMOD)
				{
					//printf ("refreshing %08x\n", (unsigned) _barGraphs[rowcnt][i]);
					if (refreshonly) {
						_barGraphs[rowcnt][i]->Refresh(FALSE);
					}
					else {
						_barGraphs[rowcnt][i]->recalculate();
					}
				}
				else if (filts[m]->getSpecModifierType() == smtype
				    && _barGraphs[rowcnt][i] != exclude)
				{
					_barGraphs[rowcnt][i]->Refresh(FALSE);
					// may not be done
					//done = true;
					//break;
				}

				rowcnt++;
				
			}
			if (done) break;
		}
		
	}

}


void FTmainwin::updatePosition(const wxString &freqstr, const wxString &valstr)
{
	wxStatusBar * statBar = GetStatusBar();

	if (statBar) {
		statBar->SetStatusText(freqstr, 1);
		statBar->SetStatusText(valstr, 2);
	}
}


void FTmainwin::handleStoreButton (wxCommandEvent &event)
{
	updateAllExtra();

	DEBUGOUT ("store button pressed" << std::endl);
	
	_configManager.storeSettings (std::string (_presetCombo->GetValue().mb_str()));

	if (_blendDialog && _blendDialog->IsShown()) {
		_blendDialog->refreshState();
	}
		
	
	rebuildPresetCombo();
}

void FTmainwin::handleLoadButton (wxCommandEvent &event)
{

        DEBUGOUT ("load button pressed" << std::endl);
	loadPreset (_presetCombo->GetValue().c_str());
}


void FTmainwin::loadPreset (const wxString &name, bool uselast)
{
	suspendProcessing();
	
	bool success = _configManager.loadSettings (std::string (name.mb_str()), _restorePortsCheck->GetValue(), uselast);

	if (success) {
		_presetCombo->SetValue(name);
		rebuildPresetCombo();


		// this rebuilds
		changePathCount ( FTioSupport::instance()->getActivePathCount() , true, true);

		if (_procmodDialog && _procmodDialog->IsShown()) {
			_procmodDialog->refreshState();
		}

		if (_modulatorDialog && _modulatorDialog->IsShown()) {
			// _modulatorDialog->refreshState();
		}
		
		if (_blendDialog && _blendDialog->IsShown()) {
			_blendDialog->refreshState(name, true, wxT(""), true);
		}
		
	}
	
	restoreProcessing();
	
}


void FTmainwin::rebuildPresetCombo()
{
	list<string> namelist = _configManager.getSettingsNames();
	
	wxString selected = _presetCombo->GetValue();
	
	_presetCombo->Clear();
	_presetCombo->Append(wxT(""));

	for (list<string>::iterator name=namelist.begin(); name != namelist.end(); ++name)
	{
		_presetCombo->Append(wxString::FromAscii (name->c_str()));
	}

	if ( _presetCombo->FindString(selected) >= 0) {
		_presetCombo->SetValue(selected);
	}
}

void FTmainwin::suspendProcessing()
{

// 	for (int i=0; i < _pathCount; i++) {
// 		if (!_processPath[i]) continue;
		
// 		_bypassArray[i] = _processPath[i]->getSpectralEngine()->getBypassed();
// 		_processPath[i]->getSpectralEngine()->setBypassed(true);
// 	}
	FTioSupport::instance()->setProcessingBypassed (true);
	
	//printf ("suspended before sleep\n");
	
	wxThread::Sleep(250); // sleep to let the process callback get around to the beginning
	//printf ("suspended after sleep\n");

}

void FTmainwin::restoreProcessing()
{
	
	// restore bypass state
// 	for (int i=0; i < _pathCount; i++) {
// 		if (!_processPath[i]) continue;
		
// 		_processPath[i]->getSpectralEngine()->setBypassed(_bypassArray[i]);
// 	}

	FTioSupport::instance()->setProcessingBypassed (false);
	
	//printf ("restored\n");
	
}



// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

FTlinkMenu::FTlinkMenu (wxWindow * parent, FTmainwin *win, FTspectralEngine *specengine, SpecModType stype,
			unsigned int procmodnum, unsigned int filtnum)
	: wxMenu(), _mwin(win), _specengine(specengine), _stype(stype), _procmodnum(procmodnum), _filtnum(filtnum)
{

	FTspectrumModifier *linkedto = 0;
	int itemid = 1000;
	FTspectrumModifier *tempfilt, *thisfilt;
	FTspectralEngine *tempengine;
	
	thisfilt = _specengine->getProcessorModule(procmodnum)->getFilter(filtnum);
	linkedto = thisfilt->getLink();
	
	
	for (int i=0; i < FT_MAXPATHS; i++) {
		if (!_mwin->_processPath[i] || _mwin->_processPath[i]->getSpectralEngine()==_specengine)
			continue;

		wxMenuItem * item = 0;

		tempengine = _mwin->_processPath[i]->getSpectralEngine();

		
		tempfilt =  tempengine->getProcessorModule(procmodnum)->getFilter(filtnum);
		if (linkedto == tempfilt || thisfilt->isLinkedFrom(tempfilt)) {
			item = new wxMenuItem(this, itemid, wxString::Format(wxT("* Path %d *"), i+1)); 
		} else {
			item = new wxMenuItem(this, itemid, wxString::Format(wxT("Path %d"), i+1)); 
		}
		
		this->Connect( itemid,  wxEVT_COMMAND_MENU_SELECTED,
			       (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
			       &FTlinkMenu::OnLinkItem,
			       new SpecModObject(tempengine));
		
		if (item) {
			this->Append(item);
			itemid++;
		}
	}

	// add unlink item
	wxMenuItem * item = new wxMenuItem(this, itemid, wxT("Unlink")); 
	this->Connect( itemid, wxEVT_COMMAND_MENU_SELECTED,
		       (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
		       &FTlinkMenu::OnUnlinkItem);

	this->AppendSeparator ();
	this->Append (item);
}


void FTlinkMenu::OnLinkItem(wxCommandEvent &event)
{
	SpecModObject * smobj = (SpecModObject *) event.m_callbackUserData;

	// this is disaster waiting to happen :)
	// we need to link all the filters in the same group to their corresponding ones

	//int group = _specengine->getProcessorModule(_procmodnum)->getFilter(_filtnum)->getGroup();
	vector<FTspectrumModifier*> sfilts, dfilts;

	_specengine->getProcessorModule(_procmodnum)->getFilters (sfilts);
	smobj->specm->getProcessorModule(_procmodnum)->getFilters (dfilts);
	
	for (unsigned int m=0; m < dfilts.size(); ++m)
	{
		//if (dfilts[m]->getGroup() == group) {
		sfilts[m]->link (dfilts[m]);
		//}
	}

	wxMenuItem *item = (wxMenuItem *) event.GetEventObject();
	if (item) {
		this->Disconnect( item->GetId(),  wxEVT_COMMAND_MENU_SELECTED,
				  (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
				  &FTlinkMenu::OnLinkItem,
				  smobj);
		
	}

	_mwin->updateGraphs(0, _stype);
	
	delete smobj;
}
		    
void FTlinkMenu::OnUnlinkItem(wxCommandEvent &event)
{
	_specengine->getProcessorModule(_procmodnum)->getFilter(_filtnum)->unlink();


	wxMenuItem *item = (wxMenuItem *) event.GetEventObject();
	if (item) {
		this->Disconnect( item->GetId(),  wxEVT_COMMAND_MENU_SELECTED,
				  (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
				  &FTlinkMenu::OnLinkItem);
		
	}

	_mwin->updateGraphs(0, _stype);
	
}

BEGIN_EVENT_TABLE(FTgridMenu, wxMenu)
	EVT_MENU_RANGE (0, 20, FTgridMenu::OnSelectItem)
END_EVENT_TABLE()

FTgridMenu::FTgridMenu (wxWindow * parent, FTmainwin *win, vector<FTactiveBarGraph *> & graphlist, FTspectrumModifier::ModifierType mtype)
	: wxMenu(), _mwin(win), _graphlist(graphlist), _mtype(mtype)
{
	wxMenuItem * item = 0;

	vector<wxString> gridunits = graphlist[0]->getGridChoiceStrings();
	unsigned int gindex = graphlist[0]->getGridChoice();
	
	int itemid = 0;
	for (vector<wxString>::iterator gridi = gridunits.begin(); gridi != gridunits.end(); ++gridi) {
		if ( (*gridi).empty()) {
			// add separator
			AppendSeparator();
			itemid++;
		}
		else if ((int)gindex == itemid) {
			item = new wxMenuItem(this, itemid++, (*gridi) + wxT(" *"));
			Append (item);
		}
		else {
			item = new wxMenuItem(this, itemid++, *gridi);
			Append (item);
		}
	}

}

void FTgridMenu::OnSelectItem(wxCommandEvent &event)
{
	wxMenuItem *item = (wxMenuItem *) event.GetEventObject();
	if (item) {
		for (vector<FTactiveBarGraph*>::iterator gr = _graphlist.begin(); gr != _graphlist.end(); ++gr) {
			(*gr)->setGridChoice (event.GetId());
		}
	}
}


BEGIN_EVENT_TABLE(FTgridButton, wxButton)
	EVT_RIGHT_DOWN (FTgridButton::handleMouse)
END_EVENT_TABLE()

FTgridButton::FTgridButton(FTmainwin * mwin, wxWindow *parent, wxWindowID id,
			   const wxString& label,
			   const wxPoint& pos,
			   const wxSize& size,
			   long style, const wxValidator& validator, const wxString& name)

	: wxButton(parent, id, label, pos, size, style, validator, name), _mainwin(mwin)
{
}

void FTgridButton::handleMouse(wxMouseEvent &event)
{
	if (event.RightDown()) {
		_mainwin->handleGridButtonMouse(event);
	}

	event.Skip();
}



BEGIN_EVENT_TABLE(FTtitleMenu, wxMenu)
	EVT_MENU_RANGE (0, 20, FTtitleMenu::OnSelectItem)
END_EVENT_TABLE()

enum {
	ID_TitleExpand = 0,
	ID_TitleMinimize,
	ID_TitleRemove
};
	
FTtitleMenu::FTtitleMenu (wxWindow * parent, FTmainwin *win, bool minimized)
	: wxMenu(),  _mwin(win), _parent(parent)
{
	wxMenuItem * item = 0;

	if (minimized) {
		item = new wxMenuItem(this, ID_TitleExpand, wxT("Expand"));
	}
	else {
		item = new wxMenuItem(this, ID_TitleMinimize, wxT("Minimize"));
	}
	Append (item);

	Append (new wxMenuItem(this, ID_TitleRemove, wxT("Remove")));
	
}

void FTtitleMenu::OnSelectItem(wxCommandEvent &event)
{
	int id = event.GetId();
	
	if (id == ID_TitleRemove)
	{
		FTtitleMenuEvent tev (0, FTtitleMenuEvent::RemoveEvt, _parent);
		_mwin->AddPendingEvent (tev);
		//_mwin->doRemoveRow(_parent);
	}
	else if (id == ID_TitleMinimize)
	{
		FTtitleMenuEvent tev (0, FTtitleMenuEvent::MinimizeEvt, _parent);
		_mwin->AddPendingEvent (tev);

		//_mwin->doMinimizeExpand(_parent);		
	}
	else if (id == ID_TitleExpand)
	{
		FTtitleMenuEvent tev (0, FTtitleMenuEvent::ExpandEvt, _parent);
		_mwin->AddPendingEvent (tev);
		
		//_mwin->doMinimizeExpand(_parent);
	}
}




BEGIN_EVENT_TABLE(FTtitleButton, wxButton)
	EVT_RIGHT_DOWN (FTtitleButton::handleMouse)
END_EVENT_TABLE()

FTtitleButton::FTtitleButton(FTmainwin * mwin, bool minimized, wxWindow *parent, wxWindowID id,
			   const wxString& label,
			   const wxPoint& pos,
			   const wxSize& size,
			   long style, const wxValidator& validator, const wxString& name)

	: wxButton(parent, id, label, pos, size, style, validator, name), _mainwin(mwin), _minimized(minimized)
{
	SetForegroundColour(*wxWHITE);
	SetBackgroundColour(wxColour(50,50,50));
	SetThemeEnabled(false);
}

void FTtitleButton::handleMouse(wxMouseEvent &event)
{
	if (event.RightDown()) {
		_mainwin->handleTitleButtonMouse(event, _minimized);
	}

	event.Skip();
}
