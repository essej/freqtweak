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

#ifndef __FTMAINWIN_HPP__
#define __FTMAINWIN_HPP__


#include <vector>
using namespace std;

#include <sigc++/sigc++.h>


#include <wx/wx.h>

//#include <wx/sashwin.h>
#include <wx/laywin.h>
#include <wx/spinctrl.h>

#include "FTtypes.hpp"
#include "FTspectragram.hpp"
#include "FTconfigManager.hpp"
#include "FTspectrumModifier.hpp"



class FTprocessPath;
class FTactiveBarGraph;
class FTspectralEngine;
class FTspectrumModifier;

class FTupdateToken;
class FTupdateTimer;
class FTrefreshTimer;
class FTlinkMenu;
class FTprocOrderDialog;
class FTpresetBlendDialog;
class FTmodulatorDialog;

namespace JLCui {
	class PixButton;
}

BEGIN_DECLARE_EVENT_TYPES()
   DECLARE_EVENT_TYPE( FT_EVT_TITLEMENU_COMMAND, 9000)
END_DECLARE_EVENT_TYPES()

   
#define EVT_TITLEMENU_COMMAND(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        FT_EVT_TITLEMENU_COMMAND, id, -1, \
        (wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction)&fn, \
        (wxObject *) NULL \
    ),

   
class FTtitleMenuEvent
   : public wxCommandEvent
{
public:
	enum CmdType
	{
		ExpandEvt = 1,
		MinimizeEvt,
		RemoveEvt
	};
	
	FTtitleMenuEvent(int id=0, CmdType ctype=ExpandEvt,  wxWindow * targ=0)
		: wxCommandEvent(FT_EVT_TITLEMENU_COMMAND, id), 
		  cmdType(ctype), target(targ), ready(false)
		{
		}
	
	FTtitleMenuEvent(const FTtitleMenuEvent & ev)
		: wxCommandEvent(ev),
		  cmdType (ev.cmdType),
		  target (ev.target), ready(ev.ready)
		{
		}
	
	virtual ~FTtitleMenuEvent() {}
	
	wxEvent *Clone(void) const { return new FTtitleMenuEvent(*this); }
	
	
	CmdType cmdType;
	wxWindow * target;

	bool ready;
	
  private:
    DECLARE_DYNAMIC_CLASS(FTtitleMenuEvent)
};


/**
 *  FTmainWin
 *
 */

class FTmainwin : public wxFrame, public SigC::Object
{
  public:
	// ctor(s)
	FTmainwin(int startpaths, const wxString& title, const wxString &rcdir , const wxPoint& pos, const wxSize& size);
	
	// event handlers (these functions should _not_ be virtual)
	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnIdle(wxIdleEvent & event);
	void OnClose(wxCloseEvent &event);
	
	//void OnSize(wxSizeEvent &event);

	

	void updateDisplay();
	void updateGraphs(FTactiveBarGraph *exclude, SpecModType smtype, bool refreshonly=false);

	void updatePosition(const wxString &freqstr, const wxString &valstr); 

	void loadPreset (const wxString & name, bool uselast=false);

	void cleanup ();

	void handleGridButtonMouse (wxMouseEvent &event);
	void handleTitleButtonMouse (wxMouseEvent &event, bool minimized);
	void doMinimizeExpand (wxWindow * source);
	void doRemoveRow (wxWindow * source);
	
	void suspendProcessing();
	void restoreProcessing();

	void rebuildDisplay(bool dolink=true);

	FTconfigManager & getConfigManager() { return _configManager; }

	void normalizeFontSize(wxFont & fnt, int height, wxString fitstr);
	
  protected:

	
	void buildGui();
	void updatePlot(int plotnum);
	void checkEvents();
	void checkRefreshes();

	
	// per path handlers
	
	void handleInputButton (wxCommandEvent &event);
	void handleOutputButton (wxCommandEvent &event);

	void handleBypassButtons (wxCommandEvent &event);
	void handleLinkButtons (wxCommandEvent &event);
	void handleLabelButtons (wxCommandEvent &event);

	void bypass_clicked_events (int button, JLCui::PixButton *);
	void link_clicked_events (int button, JLCui::PixButton *);
	void plot_clicked_events (int button, JLCui::PixButton *);
	void grid_clicked_events (int button, JLCui::PixButton *);
	void grid_pressed_events (int button, JLCui::PixButton *);
	
	void handlePlotTypeButtons (wxCommandEvent &event);
	void handleGridButtons (wxCommandEvent &event);
	
	void handleChoices (wxCommandEvent &event);
	void handleSashDragged (wxSashEvent &event);
	void handleSpins (wxSpinEvent &event);

	void handleMixSlider (wxScrollEvent &event);
	void handleGain (wxSpinEvent &event);
	void handlePathCount (wxCommandEvent &event);
	void changePathCount (int newcnt, bool rebuild=false, bool ignorelink=false);
	

	void handleStoreButton (wxCommandEvent &event);
	void handleLoadButton (wxCommandEvent &event);

	void handleIOButtons (wxCommandEvent &event);

	void OnProcMod (wxCommandEvent &event);
	void OnPresetBlend (wxCommandEvent &event);
	void OnModulatorDialog (wxCommandEvent &event);

	void handleTitleMenuCmd (FTtitleMenuEvent & ev);
	
	void rowpanelScrollSize();

	void removePathStuff(int i, bool deactivate=true);
	void createPathStuff(int i);
	void rebuildPresetCombo();

	void pushProcRow(FTspectrumModifier *specmod);
	void popProcRow();

	void updateAllExtra();
	void minimizeRow (wxWindow * shown, wxWindow * hidden, int rownum, bool layout=true);
	
	FTprocessPath * _processPath[FT_MAXPATHS];

	int _startpaths;
	
	// overall controls
	wxChoice * _freqBinsChoice;
	wxChoice * _overlapChoice;
	wxChoice * _windowingChoice;
	wxChoice * _freqScaleChoice;
	wxSlider * _timescaleSlider;
	wxChoice * _pathCountChoice;
	wxTextCtrl * _ioNameText;
	
	// array of spectragrams
	FTspectragram * _inputSpectragram[FT_MAXPATHS];
	FTspectragram * _outputSpectragram[FT_MAXPATHS];


	vector<FTactiveBarGraph **> _barGraphs;
	
// 	FTactiveBarGraph * _scaleGraph[FT_MAXPATHS];
// 	FTactiveBarGraph * _freqGraph[FT_MAXPATHS];
// 	FTactiveBarGraph * _delayGraph[FT_MAXPATHS];
// 	FTactiveBarGraph * _feedbackGraph[FT_MAXPATHS];
// 	FTactiveBarGraph * _mashGraph[FT_MAXPATHS];
// 	FTactiveBarGraph * _gateGraph[FT_MAXPATHS];


	//wxButton * _bypassAllButton;
	//wxButton * _muteAllButton;
	wxCheckBox * _bypassAllCheck;
	wxCheckBox * _muteAllCheck;
	
	vector<JLCui::PixButton *> _bypassAllButtons;
	
// 	wxButton * _scaleBypassAllButton;
// 	wxButton * _mashBypassAllButton;
// 	wxButton * _gateBypassAllButton;
// 	wxButton * _freqBypassAllButton;
// 	wxButton * _delayBypassAllButton;
// 	wxButton * _feedbBypassAllButton;

	vector<JLCui::PixButton *> _linkAllButtons;
	
// 	wxButton * _scaleLinkAllButton;
// 	wxButton * _mashLinkAllButton;
// 	wxButton * _gateLinkAllButton;
// 	wxButton * _freqLinkAllButton;
// 	wxButton * _delayLinkAllButton;
// 	wxButton * _feedbLinkAllButton;

	vector<JLCui::PixButton *> _gridButtons;
	
// 	wxButton * _scaleGridButton;
// 	wxButton * _gateGridButton;
// 	wxButton * _freqGridButton;
// 	wxButton * _delayGridButton;
// 	wxButton * _feedbGridButton;

	vector<JLCui::PixButton *> _gridSnapButtons;
	
// 	wxButton * _scaleGridSnapButton;
// 	wxButton * _gateGridSnapButton;
// 	wxButton * _freqGridSnapButton;
// 	wxButton * _delayGridSnapButton;
// 	wxButton * _feedbGridSnapButton;
	
	
	wxButton * _inspecLabelButton;
	wxButton * _outspecLabelButton;

	vector<wxButton *> _labelButtons;
	
// 	wxButton * _scaleLabelButton;
// 	wxButton * _mashLabelButton;
// 	wxButton * _gateLabelButton;
// 	wxButton * _freqLabelButton;
// 	wxButton * _delayLabelButton;
// 	wxButton * _feedbLabelButton;

	wxButton * _inspecLabelButtonAlt;
	wxButton * _outspecLabelButtonAlt;
	
	vector<wxButton *> _altLabelButtons;

// 	wxButton * _scaleLabelButtonAlt;
// 	wxButton * _mashLabelButtonAlt;
// 	wxButton * _gateLabelButtonAlt;
// 	wxButton * _freqLabelButtonAlt;
// 	wxButton * _delayLabelButtonAlt;
// 	wxButton * _feedbLabelButtonAlt;



	JLCui::PixButton * _inspecSpecTypeAllButton;
	JLCui::PixButton * _inspecPlotSolidTypeAllButton;
	JLCui::PixButton * _inspecPlotLineTypeAllButton;
	JLCui::PixButton * _outspecSpecTypeAllButton;
	JLCui::PixButton * _outspecPlotSolidTypeAllButton;
	JLCui::PixButton * _outspecPlotLineTypeAllButton;


	wxCheckBox * _linkMixCheck;
	//wxButton * _linkMixButton;

	
	
	// per path panels
	wxPanel * _upperPanels[FT_MAXPATHS];
	wxPanel * _inspecPanels[FT_MAXPATHS];

	vector<wxPanel **> _subrowPanels;

	
// 	wxPanel * _freqPanels[FT_MAXPATHS];
// 	wxPanel * _scalePanels[FT_MAXPATHS];
// 	wxPanel * _mashPanels[FT_MAXPATHS];
// 	wxPanel * _gatePanels[FT_MAXPATHS];
// 	wxPanel * _delayPanels[FT_MAXPATHS];
// 	wxPanel * _feedbPanels[FT_MAXPATHS];

	wxPanel * _outspecPanels[FT_MAXPATHS];
	wxPanel * _lowerPanels[FT_MAXPATHS];
	
	// per path buttons
	
	wxButton * _inputButton[FT_MAXPATHS];
	wxButton * _outputButton[FT_MAXPATHS];

	wxSpinCtrl *_gainSpinCtrl[FT_MAXPATHS];
	wxSlider * _mixSlider[FT_MAXPATHS];
	
	//wxButton * _bypassButton[FT_MAXPATHS];
	//wxButton * _muteButton[FT_MAXPATHS];
	wxCheckBox * _bypassCheck[FT_MAXPATHS];
	wxCheckBox * _muteCheck[FT_MAXPATHS];

    
	JLCui::PixButton * _inspecSpecTypeButton[FT_MAXPATHS];
	JLCui::PixButton * _inspecPlotSolidTypeButton[FT_MAXPATHS];
	JLCui::PixButton * _inspecPlotLineTypeButton[FT_MAXPATHS];
	JLCui::PixButton * _outspecSpecTypeButton[FT_MAXPATHS];
	JLCui::PixButton * _outspecPlotSolidTypeButton[FT_MAXPATHS];
	JLCui::PixButton * _outspecPlotLineTypeButton[FT_MAXPATHS];

	vector<JLCui::PixButton **> _bypassButtons;
	
// 	wxButton * _scaleBypassButton[FT_MAXPATHS];
// 	wxButton * _mashBypassButton[FT_MAXPATHS];
// 	wxButton * _gateBypassButton[FT_MAXPATHS];
// 	wxButton * _freqBypassButton[FT_MAXPATHS];
// 	wxButton * _delayBypassButton[FT_MAXPATHS];
// 	wxButton * _feedbBypassButton[FT_MAXPATHS];

	vector<JLCui::PixButton **> _linkButtons;

// 	wxButton * _scaleLinkButton[FT_MAXPATHS];
// 	wxButton * _mashLinkButton[FT_MAXPATHS];
// 	wxButton * _gateLinkButton[FT_MAXPATHS];
// 	wxButton * _freqLinkButton[FT_MAXPATHS];
// 	wxButton * _delayLinkButton[FT_MAXPATHS];
// 	wxButton * _feedbLinkButton[FT_MAXPATHS];

	// sizers
	wxBoxSizer *_inspecsizer, *_outspecsizer;
	wxBoxSizer *_inspecbuttsizer,  *_outspecbuttsizer;

	vector<wxBoxSizer *> _rowSizers;
	vector<wxBoxSizer *> _rowButtSizers;
	
	wxBoxSizer *_lowersizer, *_uppersizer;

	wxScrolledWindow *_rowPanel;

	vector<wxPanel *> _rowPanels;
	
	wxPanel *_inspecPanel,  *_outspecPanel;

	vector<wxSashLayoutWindow *> _rowSashes;
	wxSashLayoutWindow *_inspecSash, *_outspecSash;


	// shown flags
	bool _inspecShown;

	vector<bool> _shownFlags;
// 	bool _freqShown;
// 	bool _scaleShown;
// 	bool _mashShown;
// 	bool _gateShown;
// 	bool _delayShown;
// 	bool _feedbShown;

	bool _outspecShown;

	bool _linkedMix;
	
	// bitmap data
	wxBitmap * _bypassBitmap;
	wxBitmap * _bypassActiveBitmap;
	wxBitmap * _linkBitmap;
	wxBitmap  *_linkActiveBitmap;

	wxColour _defaultBg;
	wxColour _activeBg;
	
	
	FTupdateTimer *_eventTimer;
	int _updateMS;
	bool _superSmooth;

	FTrefreshTimer *_refreshTimer;
	int _refreshMS;

	vector<wxWindow *> _rowItems;
	//wxWindow ** _rowItems;
	int _pathCount;
	int _rowCount;

	FTconfigManager _configManager;

	wxComboBox * _presetCombo;
	wxChoice * _plotSpeedChoice;
	wxCheckBox *_superSmoothCheck;
	wxCheckBox *_restorePortsCheck;

	wxChoice * _maxDelayChoice;
	vector<float> _delayList;

	wxSpinCtrl * _tempoSpinCtrl;

	
	FTupdateToken * _updateTokens[FT_MAXPATHS];

        bool _bypassArray[FT_MAXPATHS];

	FTprocOrderDialog * _procmodDialog;
	FTpresetBlendDialog * _blendDialog;
	FTmodulatorDialog *   _modulatorDialog;
	
	int _bwidth;
	int _labwidth;
	int _bheight;
	int _rowh;

	wxFont _titleFont;
	wxFont _titleAltFont;
	wxFont _buttFont;
	
	vector<FTtitleMenuEvent *> _pendingTitleEvents;
	
	friend class FTupdateTimer;
	friend class FTrefreshTimer;
	friend class FTlinkMenu;
	friend class FTgridMenu;
	
  private:
	// any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};


class FTupdateTimer
	: public wxTimer
{
  public:
	FTupdateTimer(FTmainwin *win) : mwin(win) {}

	void Notify() { mwin->checkEvents(); }

	FTmainwin *mwin;
};

class FTrefreshTimer
	: public wxTimer
{
  public:
	FTrefreshTimer(FTmainwin *win) : mwin(win) {}

	void Notify() { mwin->checkRefreshes(); }

	FTmainwin *mwin;
};



class FTlinkMenu
	: public wxMenu
{
  public:
	FTlinkMenu (wxWindow *parent, FTmainwin *win, FTspectralEngine *specmod, SpecModType stype,
		    unsigned int procmodnum, unsigned int filtnum);

	void OnLinkItem(wxCommandEvent &event);
	void OnUnlinkItem(wxCommandEvent &event);

	FTmainwin *_mwin;
	FTspectralEngine *_specengine;
	SpecModType _stype;
	unsigned int _procmodnum;
	unsigned int _filtnum;
	
	class SpecModObject : public wxObject
	{
	   public:
		SpecModObject(FTspectralEngine *sm) : specm(sm) {;}

		FTspectralEngine *specm;
	};
	
  private:
	// any class wishing to process wxWindows events must use this macro
//	DECLARE_EVENT_TABLE()
};

class FTgridMenu
	: public wxMenu
{
  public:
	FTgridMenu (wxWindow *parent, FTmainwin *win, vector<FTactiveBarGraph*> & graph, FTspectrumModifier::ModifierType mtype);

	void OnSelectItem(wxCommandEvent &event);

	FTmainwin *_mwin;
	vector<FTactiveBarGraph *> _graphlist;
	FTspectrumModifier::ModifierType _mtype;
		
  private:
	// any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};

class FTgridButton
	: public wxButton
{
   public:
	FTgridButton(FTmainwin *mwin, wxWindow * parent, wxWindowID id,
		     const wxString& label,
		     const wxPoint& pos,
		     const wxSize& size = wxDefaultSize,
		     long style = 0, const wxValidator& validator=wxDefaultValidator, const wxString& name = wxButtonNameStr);

	void handleMouse (wxMouseEvent &event);

	FTmainwin * _mainwin;
	
  private:
	// any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()

};


class FTtitleMenu
	: public wxMenu
{
  public:
	FTtitleMenu (wxWindow *parent, FTmainwin *win, bool minimized);

	void OnSelectItem(wxCommandEvent &event);

	FTmainwin *_mwin;
	wxWindow * _parent;
		
  private:
	// any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};

class FTtitleButton
	: public wxButton
{
   public:
	FTtitleButton(FTmainwin *mwin, bool minimized, wxWindow * parent, wxWindowID id,
		     const wxString& label,
		     const wxPoint& pos,
		     const wxSize& size = wxDefaultSize,
		     long style = 0, const wxValidator& validator=wxDefaultValidator, const wxString& name = wxButtonNameStr);

	void handleMouse (wxMouseEvent &event);

	FTmainwin * _mainwin;
	bool _minimized;
  private:
	// any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()

};




#endif
