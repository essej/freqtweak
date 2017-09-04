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

#ifndef __FTACTIVEBARGRAPH_HPP__
#define __FTACTIVEBARGRAPH_HPP__

#include <list>
#include <vector>
#include <string>
using namespace std;

#include <wx/wx.h>

#include "FTtypes.hpp"
#include "FTutils.hpp"
#include "FTspectrumModifier.hpp"

class FTmainwin;


class FTactiveBarGraph
	: public wxPanel, public FTspectrumModifier::Listener
{
  public:
	FTactiveBarGraph(FTmainwin *win, wxWindow *parent, wxWindowID id, 
		      const wxPoint& pos = wxDefaultPosition,
		      const wxSize& size = wxDefaultSize,
		      long style = 0,
		      const wxString& name = wxT("ActiveBarGraph"));

	virtual ~FTactiveBarGraph();

	
	void setSpectrumModifier (FTspectrumModifier *sm);
	FTspectrumModifier * getSpectrumModifier() { return _specMod; }

	void setTopSpectrumModifier (FTspectrumModifier *sm);
	FTspectrumModifier * getTopSpectrumModifier() { return _topSpecMod; }
	
	
	void setXscale(XScaleType sc, bool writeextra=true);
	XScaleType getXscale() { return _xScaleType; }

	bool setMinMax(float min, float max);
	void getMinMax(float &min, float &max) { min=_min; max=_max; }

	void setFixMin (bool flag);
	bool getFixMin () { return _fixMin; }

	void setGridLines (bool flag, bool writeextra=true);
	bool getGridLines() { return _gridFlag; }

	void setGridSnap (bool flag, bool writeextra=true);
	bool getGridSnap () { return _gridSnapFlag; }
	
	void OnPaint ( wxPaintEvent &event);
	void OnSize ( wxSizeEvent &event);
	void OnMouseActivity ( wxMouseEvent &event );
	void OnXscaleMenu (wxCommandEvent &event);

	void setBypassed (bool flag) { _bypassed = flag; Refresh(FALSE);}
	bool getBypassed () { return _bypassed; }

	const vector<wxString> & getGridChoiceStrings() { return _gridChoices; }
	void setGridChoice (unsigned int , bool writeextra=true);
	unsigned int getGridChoice () { return _gridChoiceIndex; }

	void setTempo(int bpm);
	int getTempo() { return _tempo; }

	// listener
	void goingAway (FTspectrumModifier * specmod);

	void refreshBounds();
	void recalculate();

	void writeExtra(FTspectrumModifier * sp);
	void readExtra(FTspectrumModifier * sp);
	
  protected:
	void updateSize();

	void xToBinRange(int x, int &frombin, int &tobin);
	void binToXRange(int bin, int &fromx, int &tox);
	int xDeltaToBinDelta(int xdelt);
	void xToFreqRange(int x, float &fromfreq, float &tofreq, int &frombin, int &tobin);
	
	int valToY(float val);
	float yToVal(int y);

	inline float valToDb(float val);
	inline float dbToVal(float db);
	inline float valToSemi(float val);
	inline float semiToVal(float semi);


	float yToDb(int y);
	float yToSemi(int y);

	float valDiffY(float val, int lasty, int newy);

	void updatePositionLabels(int pX, int pY, bool showreal=false, FTspectrumModifier *specmod=0);

	void paintGridlines(wxDC & dc);

	float snapValue(float val);

	void makeGridChoices(bool setdefault=false);

	
	int _width, _height;

	int _plotWidth, _plotHeight;
	int _leftWidth;
	int _topHeight;
	int _rightWidth;
	int _bottomHeight;
	
	FTspectrumModifier * _specMod;
	FTspectrumModifier * _topSpecMod;

	float _min, _max;
	float _absmin, _absmax;
	
	float _xscale;
	
	float _mindb, _maxdb;
	float _absmindb, _absmaxdb;
	float _absposmindb, _absposmaxdb;

	float _minsemi, _maxsemi;
	float _absminsemi, _absmaxsemi;
	
	float *_tmpfilt, *_toptmpfilt;
	
	wxColour _barColor0,_barColor1;
	wxColour _barColor2,_barColor3, _barColorDead, _tipColor;
	wxBrush  _barBrush0, _barBrush1, _barBrush2, _barBrush3, _barBrushDead ,_tipBrush;
	wxBrush _bypassBrush;
	wxBrush _bgBrush;
	wxColour _penColor;
	wxPen  _barPen;

	wxColour  _gridColor;
	wxPen  _gridPen;

	
	wxBitmap * _backingMap;

	XScaleType _xScaleType;

	bool _fixMin;
	YScaleType _yScaleType;
	
	// mouse stuff
	int _lastX, _lastY;
	int _firstX, _firstY;
	float _lastVal;
	bool _dragging;
	bool _firstCtrl, _firstMove;
	bool _zooming;
	int _topzoomY, _bottomzoomY;
	
	wxString _freqstr, _valstr;

	wxMenu * _xscaleMenu;
	
	FTmainwin * _mainwin;

	list<int> _gridPoints;
	bool _gridFlag;
	bool _gridSnapFlag;
	
	bool _mouseCaptured;
	bool _bypassed;

	vector<wxString> _gridChoices;
	vector<float> _gridValues;
	float _gridFactor;
	unsigned int _gridChoiceIndex;

	wxFont _boundsFont;
	wxColour _textColor;
	FTspectrumModifier::ModifierType _mtype;
	wxString _maxstr, _minstr;

	// tempo for time only
	int _tempo;
	unsigned int _beatscutoff;

private:
	// any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()	

};


inline float FTactiveBarGraph::valToDb(float val)
{
	// assumes 0 <= yval <= 1
	
	if (val <= 0.0) {
		// negative infinity really
		return -200.0;
	}
	
	//float nval = (20.0 * FTutils::fast_log10(yval / refmax));
	float nval = (20.0 * FTutils::fast_log10(val));
	
	// printf ("scaled value is %g   mincut=%g\n", nval, _minCutoff);
	return nval;
	
}

inline float FTactiveBarGraph::dbToVal(float db)
{
	
	//float nval = (20.0 * FTutils::fast_log10(yval / refmax));
	float nval = pow ( (float)10.0, db/20);
	
	// printf ("scaled value is %g   mincut=%g\n", nval, _minCutoff);
	return nval;
	
}

inline float FTactiveBarGraph::semiToVal(float semi)
{
	
	float val = pow (2.0, semi/12.0);
	
	// printf ("scaled value is %g   mincut=%g\n", nval, _minCutoff);
	return val;
	
}

inline float FTactiveBarGraph::valToSemi(float val)
{
	// assumes 0 <= yval <= 1
	
	if (val <= 0.0) {
		// invalid
		return 0.0;
	}
	
	float semi = 12.0 * FTutils::fast_log2(val);
		
	return semi;
}



#endif
