
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
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <stdio.h>
#include <math.h>
#include "FTactiveBarGraph.hpp"
#include "FTspectrumModifier.hpp"
#include "FTutils.hpp"
#include "FTmainwin.hpp"
#include "FTjackSupport.hpp"


enum
{
	FT_1Xscale=1000,
	FT_2Xscale,
	FT_LogaXscale,
	FT_LogbXscale,	
};



// the event tables connect the wxWindows events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(FTactiveBarGraph, wxPanel)
	EVT_PAINT(FTactiveBarGraph::OnPaint)
	EVT_SIZE(FTactiveBarGraph::OnSize)
	EVT_LEFT_DOWN(FTactiveBarGraph::OnMouseActivity)
	EVT_LEFT_UP(FTactiveBarGraph::OnMouseActivity)
	EVT_RIGHT_DOWN(FTactiveBarGraph::OnMouseActivity)
	EVT_RIGHT_UP(FTactiveBarGraph::OnMouseActivity)
	EVT_MIDDLE_DOWN(FTactiveBarGraph::OnMouseActivity)
	EVT_MIDDLE_UP(FTactiveBarGraph::OnMouseActivity)
	EVT_MOTION(FTactiveBarGraph::OnMouseActivity)
	EVT_ENTER_WINDOW(FTactiveBarGraph::OnMouseActivity)
	EVT_LEAVE_WINDOW(FTactiveBarGraph::OnMouseActivity)
	EVT_MOTION(FTactiveBarGraph::OnMouseActivity)

	EVT_MENU (FT_1Xscale,FTactiveBarGraph::OnXscaleMenu)
	EVT_MENU (FT_2Xscale,FTactiveBarGraph::OnXscaleMenu)
	EVT_MENU (FT_LogaXscale,FTactiveBarGraph::OnXscaleMenu)
	EVT_MENU (FT_LogbXscale,FTactiveBarGraph::OnXscaleMenu)
	
END_EVENT_TABLE()


FTactiveBarGraph::FTactiveBarGraph(FTmainwin *win, wxWindow *parent, wxWindowID id,
			     const wxPoint& pos,
			     const wxSize& size,
			     long style ,
			     const wxString& name)

	: wxPanel(parent, id, pos, size, style, name)
	//, _topHeight(4), _bottomHeight(4), _leftWidth(4, _rightWidth(4)
	  , _width(1), _height(1)
	  , _specMod(0), _topSpecMod(0), _min(0.0), _max(1.0), _absmin(0.0), _absmax(1.0)
	  ,_mindb(-50.0), _maxdb(0.0), _absmindb(-60), _absmaxdb(0.0), _absposmindb(0.0f), _absposmaxdb(24.0)
	,_minsemi(-12.0), _maxsemi(12), _absminsemi(-12), _absmaxsemi(12)
	, _tmpfilt(0), _toptmpfilt(0)
	, _barColor0(135, 207, 235), _barColor1(69, 130, 181)
	,  _barColor2(46, 140, 87), _barColor3(143, 189, 143)
	,_barColorDead(77,77,77)
	,_tipColor(200,200,0)
	, _penColor(0,0,255), _gridColor(64,64,64)
	,_backingMap(0)
	, _xScaleType(XSCALE_1X), _lastX(0)
	, _dragging(false), _zooming(false)
	, _mainwin(win)
	, _gridFlag(false), _gridSnapFlag(false)
	, _mouseCaptured(false), _bypassed(false)
	//, _boundsFont(8, wxDEFAULT, wxNORMAL, wxNORMAL, false, "Helvetica")
	  , _boundsFont(8, wxDEFAULT, wxNORMAL, wxNORMAL)	
	, _textColor(255,255,255)
	, _tempo(120)
{
	SetBackgroundColour(*wxBLACK);
	
	_mainwin->normalizeFontSize(_boundsFont, 11, wxT("999"));

	
	_barBrush0.SetColour(_barColor0);
	_barBrush0.SetStyle(wxSOLID);
	_barBrush1.SetColour(_barColor1);
	_barBrush1.SetStyle(wxSOLID);

	_barBrush2.SetColour(_barColor2);
	_barBrush2.SetStyle(wxSOLID);
	_barBrush3.SetColour(_barColor3);
	_barBrush3.SetStyle(wxSOLID);
	_barBrushDead.SetColour(_barColorDead);
	_barBrushDead.SetStyle(wxSOLID);

	_bypassBrush.SetColour(wxColour(77,77,77));
	_bypassBrush.SetStyle(wxSOLID);

	
	_tipBrush.SetColour(_tipColor);
	_tipBrush.SetStyle(wxSOLID);
	
	_bgBrush.SetColour(wxColour(30,50,30));
	//_bgBrush.SetStyle(wxCROSS_HATCH);
	
	_barPen.SetColour(_penColor);
	_barPen.SetStyle(wxTRANSPARENT);

	_gridPen.SetColour(_gridColor);
	_gridPen.SetStyle(wxSOLID);

	
	_xscaleMenu = new wxMenu(wxT(""));
	_xscaleMenu->Append ( new wxMenuItem(_xscaleMenu, FT_1Xscale, wxT("1x Scale")));
	_xscaleMenu->Append ( new wxMenuItem(_xscaleMenu, FT_2Xscale, wxT("2x Scale")));
	_xscaleMenu->Append ( new wxMenuItem(_xscaleMenu, FT_LogaXscale, wxT("logA Scale")));
	_xscaleMenu->Append ( new wxMenuItem(_xscaleMenu, FT_LogbXscale, wxT("logB Scale")));

	// grid choices can't be determined until we get a specmod

	updateSize();
}

FTactiveBarGraph::~FTactiveBarGraph()
{
	if (_tmpfilt) delete [] _tmpfilt;
	if (_toptmpfilt) delete [] _toptmpfilt;

	if (_backingMap) delete _backingMap;
	delete _xscaleMenu;

	if (_specMod) _specMod->unregisterListener(this);
	if (_topSpecMod) _topSpecMod->unregisterListener(this);
}


void FTactiveBarGraph::refreshBounds()
{
	// mainly to handle when certain units change
	if (!_specMod) return;
	
	_xscale = _width/(float)_specMod->getLength();

	_min = _absmin = _specMod->getMin();
	_max = _absmax = _specMod->getMax();

 	if (_specMod->getModifierType() == FTspectrumModifier::GAIN_MODIFIER)
	{
		_mindb = _absmindb;
		_maxdb = _absmaxdb;
		_absmin = valToDb(_absmin); // special case
		_absmax = valToDb(_absmax); // special case
		_min = dbToVal(_mindb);
		_max = dbToVal(_maxdb);
	}
 	else if (_specMod->getModifierType() == FTspectrumModifier::POS_GAIN_MODIFIER)
	{
		_mindb = _absposmindb;
		_maxdb = _absposmaxdb;
		_absmin = valToDb(_absmin); // special case
		_absmax = valToDb(_absmax); // special case
		_min = dbToVal(_mindb);
		_max = dbToVal(_maxdb);
	}
 	else if (_specMod->getModifierType() == FTspectrumModifier::SEMITONE_MODIFIER)
	{
		_minsemi = valToSemi(_min);
		_maxsemi = valToSemi(_max);
	}

	_mtype = _specMod->getModifierType();

	
	makeGridChoices(true);

	recalculate();
	
}

void FTactiveBarGraph::setSpectrumModifier (FTspectrumModifier *sm)
{
	int origlength=-1;

	if (_specMod) {
		origlength = _specMod->getLength();
		// unregister from previous
		_specMod->unregisterListener (this);
	}
	
	_specMod = sm;
	if (sm == 0) {
		return;
	}

	_specMod->registerListener (this);
	
	if (_specMod->getLength() != origlength) {
		if (_tmpfilt) delete [] _tmpfilt;
		_tmpfilt = new float[_specMod->getLength()];
	}

	refreshBounds();
}

void FTactiveBarGraph::goingAway (FTspectrumModifier * sm)
{
	if (sm == _specMod) {
		// clean it up
		_specMod = 0;
	}
	else if (sm == _topSpecMod) {
		// clean it up
		_topSpecMod = 0;
	}
}


void FTactiveBarGraph::setTopSpectrumModifier (FTspectrumModifier *sm)
{
	if (_topSpecMod) {
		// unregister from previous
		_topSpecMod->unregisterListener (this);
	}

	_topSpecMod = sm;

	if (sm == 0) {
	   return;
	}

	_topSpecMod->registerListener (this);
	
	// should be same as other one
	_xscale = _width/(float)_topSpecMod->getLength();
	_min = _absmin = _topSpecMod->getMin();
	_max =  _absmax = _topSpecMod->getMax();

 	if (_topSpecMod->getModifierType() == FTspectrumModifier::GAIN_MODIFIER)
	{
		_mindb = _absmindb;
		_maxdb = _absmaxdb;
		_absmin = valToDb(_absmin); // special case
		_absmax = valToDb(_absmax); // special case
		_min = dbToVal(_mindb);
		_max = dbToVal(_maxdb);
	}
 	else if (_topSpecMod->getModifierType() == FTspectrumModifier::POS_GAIN_MODIFIER)
	{
		_mindb = _absposmindb;
		_maxdb = _absposmaxdb;
		_absmin = valToDb(_absmin); // special case
		_absmax = valToDb(_absmax); // special case
		_min = dbToVal(_mindb);
		_max = dbToVal(_maxdb);
	}
 	else if (_topSpecMod->getModifierType() == FTspectrumModifier::SEMITONE_MODIFIER)
	{
		_minsemi = valToSemi(_min);
		_maxsemi = valToSemi(_max);
	}

	
	if (_toptmpfilt) delete [] _toptmpfilt;
	_toptmpfilt = new float[_topSpecMod->getLength()];

	_mtype = _specMod->getModifierType();
	
	makeGridChoices(true);
	recalculate();
}


void FTactiveBarGraph::makeGridChoices (bool setdefault)
{
	_gridChoices.clear();
	_gridValues.clear();

	float tscale;
	
	switch(_mtype)
	{
	case FTspectrumModifier::GAIN_MODIFIER:
	case FTspectrumModifier::POS_GAIN_MODIFIER:
		_gridChoices.push_back (wxT("1 dB"));
		_gridValues.push_back (1.0);

		_gridChoices.push_back (wxT("3 dB"));
		_gridValues.push_back (3.0);
		
		_gridChoices.push_back (wxT("6 dB"));
		_gridValues.push_back (6.0);

		_gridChoices.push_back (wxT("12 dB"));
		_gridValues.push_back (12.0);

		_gridChoices.push_back (wxT("18 dB"));
		_gridValues.push_back (18.0);

		_gridChoices.push_back (wxT("24 dB"));
		_gridValues.push_back (24.0);

		if (setdefault) {
			// default
			_gridFactor = 6.0;
			_gridChoiceIndex = 2;
		}
		break;
	case FTspectrumModifier::SEMITONE_MODIFIER:
		_gridChoices.push_back (wxT("1/2 semi"));
		_gridValues.push_back (0.5);

		_gridChoices.push_back (wxT("1 semi"));
		_gridValues.push_back (1.0);
		
		_gridChoices.push_back (wxT("2 semi"));
		_gridValues.push_back (2.0);

		_gridChoices.push_back (wxT("3 semi"));
		_gridValues.push_back (3.0);

		_gridChoices.push_back (wxT("4 semi"));
		_gridValues.push_back (4.0);

		_gridChoices.push_back (wxT("6 semi"));
		_gridValues.push_back (6.0);

		if (setdefault) {
			// default
			_gridFactor = 1.0;
			_gridChoiceIndex = 1;
		}
		break;
	case FTspectrumModifier::TIME_MODIFIER:
		_gridChoices.push_back (wxT("1000 msec"));
		_gridValues.push_back (1.0);

		_gridChoices.push_back (wxT("500 msec"));
		_gridValues.push_back (2.0);

		_gridChoices.push_back (wxT("250 msec"));
		_gridValues.push_back (4.0);

		_gridChoices.push_back (wxT("200 msec"));
		_gridValues.push_back (5.0);

		_gridChoices.push_back (wxT("100 msec"));
		_gridValues.push_back (10.0);

		_gridChoices.push_back (wxT("50 msec"));
		_gridValues.push_back (20.0);

		_gridChoices.push_back (wxT("25 msec"));
		_gridValues.push_back (40.0);
		
		_gridChoices.push_back (wxT("10 msec"));
		_gridValues.push_back (100.0);

		_gridChoices.push_back (wxT("5 msec"));
		_gridValues.push_back (200.0);

		_beatscutoff = _gridValues.size();
		
		_gridChoices.push_back (wxT(""));
		_gridValues.push_back (0.0);
		
		
		//  meter time
		tscale = 60.0 / _tempo;
		_gridChoices.push_back (wxT("1/16 beat"));
		_gridValues.push_back (tscale * 64.0);

		_gridChoices.push_back (wxT("1/8 beat"));
		_gridValues.push_back (tscale * 32.0);

		_gridChoices.push_back (wxT("1/4 beat"));
		_gridValues.push_back (tscale * 16.0);

		_gridChoices.push_back (wxT("1/3 beat"));
		_gridValues.push_back (tscale * 12.0);
		
		_gridChoices.push_back (wxT("1/2 beat"));
		_gridValues.push_back (tscale * 8.0);

		_gridChoices.push_back (wxT("1 beat"));
		_gridValues.push_back (tscale * 4.0);

		_gridChoices.push_back (wxT("2 beats"));
		_gridValues.push_back (tscale * 2.0);
		
		_gridChoices.push_back (wxT("4 beats"));
		_gridValues.push_back (tscale * 1.0);

		
		if (setdefault) {
			// default
			_gridFactor = 4.0;
			_gridChoiceIndex = 2;
		}
		break;
	case FTspectrumModifier::UNIFORM_MODIFIER:
	case FTspectrumModifier::RATIO_MODIFIER:
		_gridChoices.push_back (wxT("50 %"));
		_gridValues.push_back (2.0);

		_gridChoices.push_back (wxT("25 %"));
		_gridValues.push_back (4.0);

		_gridChoices.push_back (wxT("20 %"));
		_gridValues.push_back (5.0);

		_gridChoices.push_back (wxT("10 %"));
		_gridValues.push_back (10.0);

		_gridChoices.push_back (wxT("5 %"));
		_gridValues.push_back (20.0);

		if (setdefault) {
			// default
			_gridFactor = 10.0;
			_gridChoiceIndex = 3;
		}
		break;
	case FTspectrumModifier::FREQ_MODIFIER:
		
		break;
	case FTspectrumModifier::DB_MODIFIER:
		_gridChoices.push_back (wxT("1 dB"));
		_gridValues.push_back (1.0);

		_gridChoices.push_back (wxT("3 dB"));
		_gridValues.push_back (3.0);
		
		_gridChoices.push_back (wxT("6 dB"));
		_gridValues.push_back (6.0);

		_gridChoices.push_back (wxT("12 dB"));
		_gridValues.push_back (12.0);

		_gridChoices.push_back (wxT("18 dB"));
		_gridValues.push_back (18.0);

		_gridChoices.push_back (wxT("24 dB"));
		_gridValues.push_back (24.0);

		if (setdefault) {
			// default
			_gridFactor = 6.0;
			_gridChoiceIndex = 2;
		}
		break;
	default:
		break;
	}

}

void FTactiveBarGraph::setGridChoice (unsigned int index, bool writeextra)
{
	if ( index < _gridValues.size()) {
		_gridFactor = _gridValues[index];
		_gridChoiceIndex = index;

		if (writeextra) {
			writeExtra (_specMod != 0 ? _specMod : _topSpecMod);
		}
		
		recalculate();
	}
}

void FTactiveBarGraph::setGridLines (bool flag, bool writeextra)
{
	_gridFlag = flag;

	if (writeextra) {
		writeExtra (_specMod != 0 ? _specMod : _topSpecMod);
	}

	recalculate();
}

void FTactiveBarGraph::setXscale(XScaleType sc, bool writeextra)
{
	_xScaleType = sc;

	if (writeextra) {
		writeExtra (_specMod != 0 ? _specMod : _topSpecMod);
	}
	
	Refresh(FALSE);
}

void FTactiveBarGraph::setTempo(int bpm)
{
	if (_tempo <= 0) return; // this is important!!
	
	_tempo = bpm;
	if (_mtype == FTspectrumModifier::TIME_MODIFIER) {
		makeGridChoices();
		_gridFactor = _gridValues[_gridChoiceIndex];
		recalculate();
	}
}

bool FTactiveBarGraph::setMinMax(float min, float max)
{
	FTspectrumModifier *specmod;
	if (_specMod) specmod = _specMod;
	else if (_topSpecMod) specmod = _topSpecMod;
	else return false;
	
	if (min >= specmod->getMin() && max <= specmod->getMax()) {
		_min = min;
		_max = max;

		recalculate();
		return true;
	}
	else {
		Refresh(FALSE);
	}
	
	return false;
}

void FTactiveBarGraph::setGridSnap (bool flag, bool writeextra)
{
	_gridSnapFlag = flag;

	if (writeextra) {
		writeExtra (_specMod != 0 ? _specMod : _topSpecMod);
	}
}

void FTactiveBarGraph::xToFreqRange(int x, float &fromfreq, float &tofreq, int &frombin, int &tobin)
{
	float freqPerBin =  FTjackSupport::instance()->getSampleRate()/(2.0 * (double)_specMod->getLength());

	//printf ("specmod length = %d  freqperbin=%g\n", _specMod->getLength(), freqPerBin);
	xToBinRange(x, frombin, tobin);

	fromfreq = freqPerBin * frombin;
	tofreq = freqPerBin * tobin + freqPerBin;
}

void FTactiveBarGraph::xToBinRange(int x, int &frombin, int &tobin)
{
	if (!_specMod) {
		frombin = tobin = 0;
		return;
	}
	
	// converts x coord into filter bin
	// according to scaling

	int bin, lbin, rbin;
	int totbins = _specMod->getLength();
	//float xscale = _width / (float)totbins;


	
	if (x < 0) x = 0;
	else if (x >= _width) x = _width-1;
	
	if (_xScaleType == XSCALE_1X) {
		bin = rbin = lbin = (int)(x / _xscale);
		//printf (" %d  %g  %d\n", x, bin*xscale, (int)(bin * xscale));
		
		// find lowest with same x
		while ( ((int)((lbin-1)*_xscale) == x) && (lbin > 0)) {
			lbin--;
		}
		// find highest with same x
		while ( ((int)((rbin+1)*_xscale) == x) && (rbin < totbins-1)) {
			rbin++;
		}

		frombin = lbin;
		tobin = rbin;
	}
	else if (_xScaleType == XSCALE_2X) {
		float hxscale = _xscale * 2;

		if (x >= _width-2) {
			lbin = totbins/2;
			rbin = totbins-1;
		}
		else {
		
			bin = rbin = lbin = (int)(x / hxscale);
			//printf (" %d  %g  %d\n", x, bin*xscale, (int)(bin * xscale));
			
			// find lowest with same x
			while ( ((int)((lbin-1)*hxscale) == x) && (lbin > 0)) {
				lbin--;
			}
			// find highest with same x
			while ( ((int)((rbin+1)*hxscale) == x) && (rbin < totbins>>1)) {
				rbin++;
			}
		}
		
		frombin = lbin;
		tobin = rbin;
	}
	else if (_xScaleType == XSCALE_LOGA)
	{
		if (x >= _width-1) {
			lbin = totbins/2;
			rbin = totbins-1;
		}
		else {
			float xscale = x / (float)_width;
			
			// use log scale for freq
			lbin = rbin = (int) ::pow ( (double) (totbins>>1), (double) xscale) - 1;
			
			// find lowest with same x
			while ( (lbin > 0) && ((int)(((FTutils::fast_log10(lbin) / FTutils::fast_log10(totbins/2)) * _width)) == x)) {
				lbin--;
			}
			// find highest with same x
			while ( (rbin < totbins>>1) && ((int)(((FTutils::fast_log10(rbin) / FTutils::fast_log10(totbins/2)) )) == x)) {
				rbin++;
			}
			
		}

		frombin = lbin;
		tobin = rbin;

		// printf ("bin %d  fromx=%d  tox=%d\n", bin, fromx, tox);
	}
	else if (_xScaleType == XSCALE_LOGB)
	{
		if (x >= _width-1) {
			lbin = (int) (totbins/3.0);
			rbin = totbins-1;
		}
		else {
			float xscale = x / (float)_width;
			
			// use log scale for freq
			lbin = rbin = (int) ::pow ( (double)(totbins/3.0), xscale) - 1;
			
			// find lowest with same x
			while ( (lbin > 0) && ((int)(((FTutils::fast_log10(lbin) / FTutils::fast_log10(totbins/3.0)) * _width)) == x)) {
				lbin--;
			}
			// find highest with same x
			while ( (rbin < (int)(totbins/3.0)) && ((int)(((FTutils::fast_log10(rbin) / FTutils::fast_log10(totbins/3.0)) )) == x)) {
				rbin++;
			}
			
		}

		frombin = lbin;
		tobin = rbin;

		// printf ("bin %d  fromx=%d  tox=%d\n", bin, fromx, tox);
	}
	else {
		frombin = 0;
		tobin = 0;
	}

	//printf ("x=%d  frombin=%d tobin=%d\n", x, frombin, tobin);
}

int FTactiveBarGraph::xDeltaToBinDelta(int xdelt)
{

	return (int) (xdelt / _xscale);

}

float FTactiveBarGraph::valDiffY(float val, int lasty, int newy)
{
	float retval;
	
	if (_specMod->getModifierType() == FTspectrumModifier::GAIN_MODIFIER
	    || _specMod->getModifierType() == FTspectrumModifier::POS_GAIN_MODIFIER)
	{
		// db scaled
		float vdb = valToDb (val);
		float ddb = ((lasty-newy)/(float)_height) * (_maxdb - _mindb) ;
		retval =  dbToVal (vdb + ddb);
	}
	else if (_specMod->getModifierType() == FTspectrumModifier::SEMITONE_MODIFIER) {
		// semitone scaled
		float vsemi = valToSemi (val);
		float dsemi = ((lasty-newy)/(float)_height) * (_maxsemi - _minsemi) ;
		retval = semiToVal (vsemi + dsemi);
	}
	else {
		float shiftval = yToVal(newy) - yToVal(lasty);
		retval = ( val + shiftval);
	}

	if (_gridSnapFlag) {
		retval = snapValue(retval);
	}
	
	return retval;
}

void FTactiveBarGraph::binToXRange(int bin, int &fromx, int &tox)
{
	if (!_specMod) {
		fromx = tox = 0;
		return;
	}

	// converts bin into x coord range
	// according to scaling

	int x, lx, rx;
	int totbins = _specMod->getLength();
	//float xscale = _width / (float)totbins;

	
	if (bin < 0) bin = 0;
	else if (bin >= totbins) bin = totbins-1;
	
	if (_xScaleType == XSCALE_1X) {
		//bin = rbin = lbin = (int)(x / xscale);
		x = lx = rx = (int) (bin * _xscale);
		//printf (" %d  %g  %d\n", x, bin*xscale, (int)(bin * xscale));
		
		// find lowest x with same bin
		while ( ((int)((lx-1)/_xscale) == bin) && (lx > 0)) {
			lx--;
		}
		// find highest with same x
		while ( ((int)((rx+1)/_xscale) == bin) && (rx < _width-1)) {
			rx++;
		}

		fromx = lx;
		tox = rx;
	}
	else if (_xScaleType == XSCALE_2X) {
		float hxscale = _xscale * 2;

		if (bin >= totbins>>1) {
			rx = (int) (totbins * _xscale);
			lx = rx - 2;
		}
		else {
			//bin = rbin = lbin = (int)(x / xscale);
			x = lx = rx = (int) (bin * hxscale );
			//printf (" %d  %g  %d\n", x, bin*xscale, (int)(bin * xscale));

			// find lowest x with same bin
			while ( ((int)((lx-1)/(hxscale)) == bin) && (lx > 0)) {
				lx--;
			}
			// find highest with same x
			while ( ((int)((rx+1)/hxscale) == bin) && (rx < _width-1)) {
				rx++;
			}
		}

		

		fromx = lx;
		tox = rx;
	}
	else if (_xScaleType == XSCALE_LOGA)
	{
		// use log scale for freq
		if (bin > totbins/2) {
			lx = rx = (int) (totbins * _xscale);
			
		} else if (bin == 0) {
			lx = 0;
			rx = (int) ((FTutils::fast_log10(bin+2) / FTutils::fast_log10(totbins*0.5)) * _width);
		} else {
			lx = (int) ((FTutils::fast_log10(bin+1) / FTutils::fast_log10(totbins*0.5)) * _width);
			rx = (int) ((FTutils::fast_log10(bin+2) / FTutils::fast_log10(totbins*0.5)) * _width);
		}


		fromx = lx;
		tox = rx;

		// printf ("bin %d  fromx=%d  tox=%d\n", bin, fromx, tox);
	}
	else if (_xScaleType == XSCALE_LOGB)
	{
		// use log scale for freq
		if (bin > (int)(totbins*0.3333)) {
			lx = rx = (int) (totbins * _xscale);
			
		} else if (bin == 0) {
			lx = 0;
			rx = (int) ((FTutils::fast_log10(bin+2) / FTutils::fast_log10(totbins*0.3333)) * _width);
		} else {
			lx = (int) ((FTutils::fast_log10(bin+1) / FTutils::fast_log10(totbins*0.3333)) * _width);
			rx = (int) ((FTutils::fast_log10(bin+2) / FTutils::fast_log10(totbins*0.3333)) * _width);
		}


		fromx = lx;
		tox = rx;

		// printf ("bin %d  fromx=%d  tox=%d\n", bin, fromx, tox);
	}
	else {
		fromx = 0;
		tox = 0;
	}

	//printf ("bin=%d  fromx=%d tox=%d\n", bin, fromx, tox);
}



int FTactiveBarGraph::valToY(float val)
{
	int y = 0;
	
	if (_specMod->getModifierType() == FTspectrumModifier::GAIN_MODIFIER
	    ||_specMod->getModifierType() == FTspectrumModifier::POS_GAIN_MODIFIER)
	{
		// db scale it
		float db = valToDb(val);
		y = (int) (( (db - _mindb) / (_maxdb - _mindb)) * _height);
		//printf ("val=%g  db=%g  y=%d\n", val, db, y);

	}
	else if (_specMod->getModifierType() == FTspectrumModifier::SEMITONE_MODIFIER) {
		// go from octave to semitone
		float semi = valToSemi (val);
		y = (int) (( (semi - _minsemi) / (_maxsemi - _minsemi)) * _height);
	}
	else if (_specMod->getModifierType() == FTspectrumModifier::FREQ_MODIFIER) {

		if (_xScaleType == XSCALE_1X || _xScaleType == XSCALE_2X)
		{
			y = (int) ( (val - _min)/(_max-_min) * _height);
		}
		else if (_xScaleType == XSCALE_LOGA)
		{
			// use log scale for freq
			y = (int) ((FTutils::fast_log10(val+1) / FTutils::fast_log10(_absmax*0.5)) * _height);

			// printf ("bin %d  fromx=%d  tox=%d\n", bin, fromx, tox);
		}
		else if (_xScaleType == XSCALE_LOGB)
		{
			y = (int) ((FTutils::fast_log10(val+1) / FTutils::fast_log10(_absmax*0.3333)) * _height);
		}
		
		//  scale it however the X scale is

	}
	else {
		y = (int) ( (val - _min)/(_max-_min) * _height);
	}	

	//printf ("val to y: %g to %d\n", val, y);
	return y;
}


float FTactiveBarGraph::yToDb(int y)
{
	return (((_height - y)/(float)_height) * (_maxdb-_mindb)) + _mindb ;
}

float FTactiveBarGraph::yToSemi(int y)
{
	return (((_height - y)/(float)_height) * (_maxsemi-_minsemi)) + _minsemi ;
}


float FTactiveBarGraph::yToVal(int y)
{
	float val = 0.0;
	
	if (_specMod->getModifierType() == FTspectrumModifier::GAIN_MODIFIER
	    ||_specMod->getModifierType() == FTspectrumModifier::POS_GAIN_MODIFIER)
	{
		// go from db to linear
		
		float db = yToDb (y);

		val = dbToVal(db);
		//printf ("y=%d  val=%g  db=%g\n",  y, val, db);

	}
	else if (_specMod->getModifierType() == FTspectrumModifier::SEMITONE_MODIFIER) {
		// go from semitone to octave
		
		float semi = (((_height - y)/(float)_height) * (_maxsemi-_minsemi)) + _minsemi ;
		// Up a semitone is * 1.0593 or 12   2 (12th root of 2)
		val = semiToVal(semi);
		//printf ("y=%d  val=%g  db=%g\n",  y, val, db);

	}
	else if (_specMod->getModifierType() == FTspectrumModifier::FREQ_MODIFIER) {

		if (_xScaleType == XSCALE_1X || _xScaleType == XSCALE_2X)
		{
			val = (((_height - y) / (float)_height)) * (_max-_min) + _min;
			//y = (int) ( (val - _min)/(_max-_min) * _height);
		}
		else if (_xScaleType == XSCALE_LOGA)
		{
			// use log scale for freq
			//y = (int) ((FTutils::fast_log10(val) / FTutils::fast_log10(_absmax*0.5)) * _height);

			val = (int) ::pow ( (double) (_absmax*0.5), (double) (_height-y)/_height ) - 1;

			// printf ("bin %d  fromx=%d  tox=%d\n", bin, fromx, tox);
		}
		else if (_xScaleType == XSCALE_LOGB)
		{
			//y = (int) ((FTutils::fast_log10(val) / FTutils::fast_log10(_absmax*0.3333)) * _height);
			val = (int) ::pow ( (double) (_absmax*0.3333), (double) (_height-y)/_height ) - 1;
		}
	}
	else {
		val = (((_height - y) / (float)_height)) * (_max-_min) + _min;
	}	
	
	return val;
}

float FTactiveBarGraph::snapValue(float val)
{
	// takes a given value (as passed to the specmanip)
	// and snaps it to the nearest gridline

	float snapval = val;

 	if (_specMod->getModifierType() == FTspectrumModifier::GAIN_MODIFIER)
	{
		// every 6 db
		float dbval = valToDb(val) / _gridFactor;
		float numDivs = ((_absmaxdb - _absmindb) / _gridFactor);
		float divdb = (_absmaxdb - _absmindb) / numDivs;

		snapval = divdb * rint(dbval);

		//printf ("gain snap: %g \n", snapval);
		snapval = dbToVal (snapval);
	}
 	else if (_specMod->getModifierType() == FTspectrumModifier::POS_GAIN_MODIFIER)
	{
		// every 6 db
		float dbval = valToDb(val) / _gridFactor;
		float numDivs = ((_absposmaxdb - _absposmindb) / _gridFactor);
		float divdb = (_absposmaxdb - _absposmindb) / numDivs;

		snapval = divdb * rint(dbval);

		//printf ("gain snap: %g \n", snapval);
		snapval = dbToVal (snapval);
	}
 	else if (_specMod->getModifierType() == FTspectrumModifier::SEMITONE_MODIFIER)
	{
		float semival = valToSemi(val) / _gridFactor;

		snapval = rint(semival) * _gridFactor;
		
		//printf ("semi snap: %g \n", snapval);
		snapval = semiToVal (snapval);
		
	}
	else if (_specMod->getModifierType() == FTspectrumModifier::TIME_MODIFIER) {
		// every 500 msec
		float updiv = val * _gridFactor;
		snapval = rint(updiv) / _gridFactor;
	}
	else if (_specMod->getModifierType() == FTspectrumModifier::UNIFORM_MODIFIER) {
		// every 0.2
		float updiv = val * _gridFactor;
		snapval = rint(updiv) / _gridFactor;
	}
	else if (_specMod->getModifierType() == FTspectrumModifier::DB_MODIFIER) {
		// every 12 db
		float dbval = val / _gridFactor;
		float numDivs = ((_absmax - _absmin) / _gridFactor);
		float divdb = (_absmax - _absmin) / numDivs;

		snapval = divdb * rint(dbval);
		//printf ("gain snap: %g \n", snapval);
	}
	
	return snapval;
}



void FTactiveBarGraph::OnPaint(wxPaintEvent & event)
{
	if (!_backingMap || !_specMod) {
		event.Skip();
		return;
	}

	//printf ("ActiveBarGraph::OnPaint  xscale=%g\n", _xscale);
	
	//Clear();
	wxPaintDC dc(this);
	
	wxMemoryDC backdc;
	backdc.SelectObject(*_backingMap);

	
	int bincnt = _specMod->getLength();
	//float barwidthF = _width / (float) barcnt;
	//int currx = 0;

	float *bvalues = 0, *tvalues=0;
	FTspectrumModifier * specmod = 0;
	
	if (_specMod) {
		bvalues = _specMod->getValues();
		specmod = _specMod;
	}

	if (_topSpecMod) {
		tvalues = _topSpecMod->getValues();
		specmod = _topSpecMod;
	}

	bool both = false;
	if (bvalues && tvalues) {
		both = true;
	}
	else if (!specmod) {
		return;
	}

	//backdc.BeginDrawing();

	backdc.SetBackground(_bgBrush);
	backdc.Clear();
	
	// draw bars
	//backdc.SetBrush(_barBrush);
	backdc.SetPen(_barPen);

	/*
	if (_width == barcnt)
	{
		for (int i=0; i < barcnt; i++ )
		{
			int val = (int) ( (values[i] - _min)/(_max-_min) * _height);
			backdc.DrawRectangle( i, _height - val, 1,  val);
		}
	}
	*/

	
	for (int i=0; i < bincnt; i++ )
	{
		int yu=0 , yl = _height;
		if (bvalues) {
			yu =  _height - valToY (bvalues[i]);
		}
		if (tvalues) {
			yl =  _height - valToY (tvalues[i]);
		}
		
		int leftx, rightx;
		binToXRange(i, leftx, rightx);

		//printf ("%08x:  %d  %d\n", (unsigned) this, leftx, rightx);

		// main bar
		if (_bypassed) {
			backdc.SetBrush(_bypassBrush);
			backdc.DrawRectangle( leftx,  yu , rightx - leftx + 1,  yl - yu);
		}
		else if (yu < yl) {
			if (both)			{
				if (i%2==0) {
					backdc.SetBrush(_barBrush2);
				}
				else  {
					backdc.SetBrush(_barBrush3);
				}
			}
			else {
				if (i%2==0) {
					backdc.SetBrush(_barBrush1);
				}
				else  {
					backdc.SetBrush(_barBrush0);
				}
			}

			backdc.DrawRectangle( leftx,  yu , rightx - leftx + 1,  yl - yu);

			// top line
			backdc.SetBrush(_tipBrush);
			backdc.DrawRectangle( leftx, yu, rightx - leftx + 1,  2);
			backdc.DrawRectangle( leftx, yl, rightx - leftx + 1,  2);
		}
		else {
			backdc.SetBrush(_barBrushDead);
			backdc.DrawRectangle( leftx,  yu , rightx - leftx + 1,  yl - yu);
		}
	}


	// draw gridlines
	if (_gridFlag) {
		paintGridlines (backdc);
	}
	

	// draw min and max text
	backdc.SetFont(_boundsFont);
	
	wxCoord mtw, mth, xtw, xth;
	backdc.GetTextExtent(_maxstr, &xtw, &xth);
	backdc.GetTextExtent(_minstr, &mtw, &mth);

 	backdc.SetTextForeground(*wxBLACK);
 	backdc.DrawText(_maxstr, (_width - xtw) - 1, 1);
 	backdc.DrawText(_minstr, (_width - mtw) - 1, (_height - mth) + 1);

	backdc.SetTextForeground(_textColor);
	backdc.DrawText(_maxstr, (_width - xtw) - 2, 0);
	backdc.DrawText(_minstr, (_width - mtw) - 2, (_height - mth));


	if (_zooming) {
		// draw xor'd box
		backdc.SetLogicalFunction (wxINVERT);

		backdc.DrawRectangle ( 0, _topzoomY, _width, _bottomzoomY - _topzoomY);

	}	
	
	//backdc.EndDrawing();

	// blit to screen
	dc.Blit(0,0, _width, _height, &backdc, 0,0);
	
	// event.Skip();
}


void FTactiveBarGraph::paintGridlines(wxDC & dc)
{
	int origfunc = dc.GetLogicalFunction();
	wxPen origPen = dc.GetPen();
	
	dc.SetPen(_gridPen);
	dc.SetLogicalFunction(wxOR);
	
	// draw grid lines
	list<int>::iterator yend = _gridPoints.end();
	for (list<int>::iterator y = _gridPoints.begin() ; y != yend; ++y)
	{
		dc.DrawLine (0,  *y, _width, *y);
	}
  	
	dc.SetPen(origPen);
	dc.SetLogicalFunction(origfunc);
}


void FTactiveBarGraph::recalculate()
{
	if (!_specMod) return;
	
	int totbins = _specMod->getLength();
	_xscale = _width / (float)totbins;

	_gridPoints.clear();
	wxString maxstr, minstr;
	if (_mtype == FTspectrumModifier::GAIN_MODIFIER
	    || _mtype == FTspectrumModifier::POS_GAIN_MODIFIER) {
	}
	else if (_mtype == FTspectrumModifier::SEMITONE_MODIFIER) {
		maxstr = wxString::Format(wxT("%.3g"), _maxsemi);
		minstr = wxString::Format(wxT("%.3g"), _minsemi);
	}
	else {
		maxstr = wxString::Format(wxT("%.3g"), _max);
		minstr = wxString::Format(wxT("%.3g"), _min);
	}


 	if (_specMod->getModifierType() == FTspectrumModifier::GAIN_MODIFIER
	    || _specMod->getModifierType() == FTspectrumModifier::POS_GAIN_MODIFIER)
	{
		// every _gridFactor db
		_mindb = valToDb(_min);
		_maxdb = valToDb(_max);
		float numDivs = ((_absmax - _absmin) / _gridFactor);
		float divdb = _gridFactor;
		int y;
		float cdb;
		for (int i=1; i < (int)numDivs; ++i) {
			cdb = _absmax - (divdb * i);
			if (cdb <= _maxdb && cdb >= _mindb) {
				// add line
				y = (int) (( (cdb - _mindb) / (_maxdb - _mindb)) * _height);
				_gridPoints.push_back (_height - y);
				//printf ("gain grid %d:  %g  y=%d\n", i, cdb, y);
			}
		}

		_maxstr = wxString::Format(wxT("%.1f"), _maxdb);
		_minstr = wxString::Format(wxT("%.1f"), _mindb);
		
	}
 	else if (_specMod->getModifierType() == FTspectrumModifier::SEMITONE_MODIFIER)
	{
		// every semitone
		_minsemi = valToSemi(_min);
		_maxsemi = valToSemi(_max);

		float numDivs = (_absmaxsemi - _absminsemi) / _gridFactor;
		float updiv = _gridFactor;
		int y;
		float cunit;
		for (int i=1; i < (int)numDivs; ++i) {
			cunit = _absmaxsemi - (updiv * i);
			if (cunit <= _maxsemi && cunit >= _minsemi) {
				// add line
				y = valToY(semiToVal(cunit));
				_gridPoints.push_back (_height - y);
				//printf ("pitch grid %d:  %g  y=%d\n", i, cunit, y);
			}
		}

		_maxstr = wxString::Format(wxT("%+.1f"), _maxsemi);
		_minstr = wxString::Format(wxT("%+.1f"), _minsemi);

	}
	else if (_specMod->getModifierType() == FTspectrumModifier::TIME_MODIFIER) {
		// every 500 msec
		float updiv = 1 / _gridFactor;
		int y;
		for (float i=updiv; i < _absmax; i+=updiv) {
			if (i <= _max && i >= _min) {
				// add line
				y = valToY(i);
				_gridPoints.push_back (_height - y);
				//printf ("time grid: %g  y=%d\n", i, y);
			}
		}

		if (_gridChoiceIndex >= _beatscutoff) {
			_maxstr = wxString::Format(wxT("%.2f"), _max * _tempo/60.0);
			_minstr = wxString::Format(wxT("%.2f"), _min * _tempo/60.0);
		} else {
			_maxstr = wxString::Format(wxT("%.0f"), _max * 1000);
			_minstr = wxString::Format(wxT("%.0f"), _min * 1000);
		}
		
	}
	else if (_specMod->getModifierType() == FTspectrumModifier::UNIFORM_MODIFIER) {
		// every 0.2
		float updiv = 1 / _gridFactor;
		int y;
		for (float i=updiv; i < _absmax; i+=updiv) {
			if (i <= _max && i >= _min) {
				// add line
				y = valToY(i);
				_gridPoints.push_back (_height - y);
				//printf ("uniform grid: %g  y=%d\n", i, y);
			}
		}
		_maxstr = wxString::Format(wxT("%.1f"), _max * 100);
		_minstr = wxString::Format(wxT("%.1f"), _min * 100);

	}
	else if (_specMod->getModifierType() == FTspectrumModifier::DB_MODIFIER) {
		// every 12 db
		float numDivs = ((_absmax - _absmin) / _gridFactor);
		float divdb = _gridFactor;
		int y;
		float cdb;
		for (int i=1; i < (int)numDivs; ++i) {
			cdb = _absmax - (divdb * i);
			if (cdb <= _max && cdb >= _min) {
				// add line
				y = (int) (( (cdb - _min) / (_max - _min)) * _height);
				_gridPoints.push_back (_height - y);
				//printf ("gain grid %d:  %g  y=%d\n", i, cdb, y);
			}
		}

		_maxstr = wxString::Format(wxT("%.1f"), _max);
		_minstr = wxString::Format(wxT("%.1f"), _min);
	}
	else if (_specMod->getModifierType() == FTspectrumModifier::RATIO_MODIFIER) {

		_maxstr = wxString::Format(wxT("%.0f"), _max);
		_minstr = wxString::Format(wxT("%.0f"), _min);
		
	}

	
	
	Refresh(FALSE);
}


void FTactiveBarGraph::updateSize()
{
	GetClientSize(&_width, &_height);
	//printf ("ActiveBarGraph::OnSize:  width=%d\n", _width);

	if (_width > 0 && _height > 0 ) 
	{
		if (_backingMap) delete _backingMap;
		
		_backingMap = new wxBitmap(_width, _height);
		
		SetBackgroundColour(*wxBLACK);
		//Clear();
		
		recalculate();
	}
}

void FTactiveBarGraph::OnSize(wxSizeEvent & event)
{
	updateSize();
	event.Skip();
}

void FTactiveBarGraph::writeExtra (FTspectrumModifier * specmod)
{
	XMLNode * extra = specmod->getExtraNode();

	extra->add_property ("xscale", static_cast<const char *> (wxString::Format(wxT("%d"), _xScaleType).mb_str()));

	extra->add_property ("gridsnap", static_cast<const char *> (wxString::Format(wxT("%d"), (int) _gridSnapFlag).mb_str()));

	extra->add_property ("grid", static_cast<const char *> (wxString::Format(wxT("%d"), (int) _gridChoiceIndex).mb_str()));

	extra->add_property ("gridlines", static_cast<const char *> (wxString::Format(wxT("%d"), (int) _gridFlag).mb_str()));
	
}

void FTactiveBarGraph::readExtra (FTspectrumModifier * specmod)
{
	XMLNode * extra = specmod->getExtraNode();
	XMLProperty * prop;
	long val;
	wxString strval;
	
	if ((prop = extra->property ("xscale"))) {
		strval = wxString::FromAscii (prop->value().c_str());
		if (strval.ToLong (&val)) {
			setXscale ((XScaleType) val, false);
		}
	}	

	if ((prop = extra->property ("grid"))) {
		strval = wxString::FromAscii (prop->value().c_str());
		if (strval.ToLong (&val)) {
			setGridChoice ((unsigned int) val, false);
		}
	}	
	
	if ((prop = extra->property ("gridsnap"))) {
		strval = wxString::FromAscii (prop->value().c_str());
		if (strval.ToLong (&val)) {
			setGridSnap ( val != 0 ? true : false, false);
		}
	}	

	if ((prop = extra->property ("gridlines"))) {
	        strval = wxString::FromAscii (prop->value().c_str());
		if (strval.ToLong (&val)) {
			setGridLines ( val != 0 ? true : false, false);
		}
	}	
	
}


void FTactiveBarGraph::OnMouseActivity( wxMouseEvent &event)
{
	if (!_specMod) {
		event.Skip();
		return;
	}
	
	int pX = event.GetX();
	int pY = event.GetY();
	int i,j;
	
	//int length = _specMod->getLength();
	//float xscale = (float) _width / (float)length;

	FTspectrumModifier *specm;
	
	if (_specMod) {
		specm = _specMod;
	}
	else if (_topSpecMod) {
		specm = _topSpecMod;
	}
	else {
		// nothing to do
		return;
	}
	
	
	if (event.LeftDown()) {
		if (event.RightIsDown()) {
			SetCursor(wxCURSOR_HAND);
		}
		else if (event.ControlDown() && event.ShiftDown() && event.AltDown() && !_dragging)
		{
			// ZOOMING
			SetCursor(wxCURSOR_MAGNIFIER);
		}
		
		if (!_mouseCaptured) {
			CaptureMouse();
			_mouseCaptured = true;
		}
	}
	else if (event.RightDown()) {
		SetCursor(wxCURSOR_HAND);
		if (!_mouseCaptured) {
			CaptureMouse();
			_mouseCaptured = true;
		}
	}
			


	if (event.Entering()) {
		SetCursor(wxCURSOR_PENCIL);
		updatePositionLabels(pX, pY, true);
	}
	else if (event.Leaving()) {
		SetCursor(*wxSTANDARD_CURSOR);
		_mainwin->updatePosition(wxT(""), wxT(""));
	}
	else if (event.MiddleUp()) {
		// popup scale menu
		this->PopupMenu ( _xscaleMenu, pX, pY);

	}
	else if (!_dragging && event.LeftIsDown() && (_zooming || (event.ControlDown() && event.ShiftDown() && event.AltDown())))
	{
		// zooming related
		if (event.LeftDown()) {
			_zooming = true;

			if (_gridSnapFlag) {
				_firstY = _topzoomY = _bottomzoomY = valToY(snapValue(yToVal(_height - pY))) + 1;
			} else {
				_firstY = _topzoomY = _bottomzoomY = pY;
			}
			Refresh(FALSE);
		}
		else if (event.LeftIsDown()) {
			if (_gridSnapFlag) {
				if (pY < _firstY) {
					_bottomzoomY = _firstY;
					_topzoomY =  valToY(snapValue(yToVal(_height - pY))) + 1;
					pY = _topzoomY + 1;
				}
				else {
					pY = valToY(snapValue(yToVal(_height - pY)));
					_bottomzoomY = valToY(snapValue(yToVal(_height - pY))) + 1;
					_topzoomY = _firstY;
					pY = _bottomzoomY;
				}
			}
			else {
				if (pY < _firstY) {
					_bottomzoomY = _firstY;
					_topzoomY = pY;
				}
				else {
					_bottomzoomY = pY;
					_topzoomY = _firstY;
				}
			}

			if (_topzoomY < 0) _topzoomY = 0;
			if (_bottomzoomY > _height) _bottomzoomY = _height;

			Refresh(FALSE);
			updatePositionLabels(pX, pY, true);
		}
		
	}
	else if ((event.LeftDown() || (_dragging && event.Dragging() && event.LeftIsDown())) && !event.RightIsDown() && !_zooming)
	{
		// Pencil drawing
	   
		if (event.LeftDown()) {
			_firstX = _lastX = pX;
			_firstY = _lastY = pY;
			_dragging = true;
		}
		
		// modify spectrumModifier for bins
		float *values ;
		FTspectrumModifier *specmod=0;
		
		if (_topSpecMod) {
			if (event.ShiftDown() && _specMod) {
				// shift does regular specmod if there is a topspecmod
				values = _specMod->getValues();
				specmod =_specMod;
			}
			else {
				values = _topSpecMod->getValues();
				specmod = _topSpecMod;
			}
		}
		else if (_specMod) {
			values = _specMod->getValues();
			specmod = _specMod;
		}
		else {
			event.Skip();
			return;
		}
		
		int leftx, rightx;
		int sign, linesign = 0;

		if (_lastX <= pX) {
			leftx = _lastX;
			rightx = pX;
			sign  = 1;
		}
		else {
			leftx = pX;
			rightx = _lastX;
			sign = -1;
		}
		
		
		// compute values to modify
		int frombin, tobin, fromi, toi;

		xToBinRange(leftx, fromi, toi);
		frombin = fromi;
		xToBinRange(rightx, fromi, toi);
		tobin = toi;
		
		//int fromi = (int) (_lastX / xscale);
		//int toi = (int) (pX / xscale);

		int useY = pY;
		
		if (event.ControlDown() && !event.AltDown()) {
			if (_firstCtrl) {
				_firstY = pY;
				_firstX = pX;
				_firstCtrl = false;
			}
			useY = _firstY;

			if (useY < 0) useY = 0;
			else if (useY > _height) useY = _height;

			float val = yToVal (useY);

			if (_gridSnapFlag) {
				val = snapValue(val);
			}
			
			for ( i=frombin; i<=tobin; i++)
			{
				values[i] = val;
				//values[i] = (((_height - useY) / (float)_height)) * (_max-_min) + _min;
				//printf ("i=%d  values %g\n", i, values[i]);
			}

		}
		else {
			_firstCtrl = true;
			
			if (useY < 0) useY = 0;
			else if (useY > _height) useY = _height;

			if (event.ControlDown() && event.AltDown())
			{
				if (_firstX <= pX) {
					leftx = _firstX;
					rightx = pX;
					linesign = 1;
				}
				else {
					leftx = pX;
					rightx = _firstX;
					linesign = -1;
				}
			}

			int leftY, rightY;

			if (sign != linesign)
			{
				if (sign > 0) {
					leftY = _lastY;
					rightY = useY;
				}
				else {
					leftY = useY;
					rightY = _lastY;
				}


//				if (specmod->getModifierType() != FTspectrumModifier::GAIN_MODIFIER)
				{
					float rightval = yToVal(rightY);
					float leftval = yToVal(leftY);
					if (_gridSnapFlag) {
						leftval = snapValue(leftval);
						rightval = snapValue(rightval);
					}

					float slope = (rightval - leftval) / (float)(1+tobin-frombin);				
					int n = 0;
					
					if (_gridSnapFlag) {
						for ( i=frombin; i<=tobin; i++, n++)
						{
							values[i] = snapValue(leftval + slope * n);
						}
					}
					else {
						for ( i=frombin; i<=tobin; i++, n++)
						{
							values[i] = leftval + slope * n;
						}
					}
				}
// 				else {
// 					// do linear interpolation between firsty and usey
// 					float slope = (rightY - leftY) / (float)(1+tobin-frombin);				
// 					//printf ("adjust is %g   useY=%d  lasty=%d\n", adj, useY, _lastY);

// 					int n=0;
// 					for ( i=frombin; i<=tobin; i++, n++)
// 					{
// 						values[i] = yToVal((int) (leftY + slope*n));
// 					}

// 				}

			}
			
			if (event.ControlDown() && event.AltDown())
			{
				
				xToBinRange(leftx, fromi, toi);
				frombin = fromi;
				xToBinRange(rightx, fromi, toi);
				tobin = toi;

				
				if (linesign > 0) {
					leftY = _firstY;
					rightY = useY;
				}
				else {
					leftY = useY;
					rightY = _firstY;
				}
				

				//if (specmod->getModifierType() != FTspectrumModifier::GAIN_MODIFIER)
				{
					float rightval = yToVal(rightY);
					float leftval = yToVal(leftY);
					if (_gridSnapFlag) {
						leftval = snapValue(leftval);
						rightval = snapValue(rightval);
					}

					float slope = (rightval - leftval) / (float)(1+tobin-frombin);				
					
					int n = 0;
					if (_gridSnapFlag) {
						for ( i=frombin; i<=tobin; i++, n++)
						{
							values[i] = snapValue(leftval + slope * n);
						}
					}
					else {
						for ( i=frombin; i<=tobin; i++, n++)
						{
							values[i] = leftval + slope * n;
						}
					}

				}
// 				else {
// 					// do linear interpolation between firsty and usey
// 					float slope = (rightY - leftY) / (float)(1+tobin-frombin);				
// 					//printf ("adjust is %g   useY=%d  lasty=%d\n", adj, useY, _lastY);

// 					int n=0;
// 					for ( i=frombin; i<=tobin; i++, n++)
// 					{
// 						values[i] = yToVal((int) (leftY + slope*n));
// 					}

// 				}
			}			
			
		}
		
		
		Refresh(FALSE);
		_mainwin->updateGraphs(this, specm->getSpecModifierType());
		
		_lastX = pX;
		_lastY = useY;

		updatePositionLabels(pX, useY, true, specmod);

	}
	else if ((event.RightDown() || (_dragging && event.Dragging() && event.RightIsDown())) && !_zooming)
	{
		// shift entire contour around
		if (event.RightDown()) {
			_firstX = _lastX = pX;
			_firstY = _lastY = pY;
			SetCursor(wxCURSOR_HAND);
			_dragging = true;
			_firstMove = true;
		}

		float * valueslist[2];
		FTspectrumModifier *specmod = _specMod;
		bool edgehold = event.ControlDown();

		float *values;
		int totbins;

		if (_topSpecMod) {
			if (event.ShiftDown() && _specMod) {
				valueslist[0] = _specMod->getValues();
				specmod = _specMod;
				totbins = _specMod->getLength();
				valueslist[1] = 0;

				// do both
				if (event.LeftIsDown()) {
					valueslist[1] = _topSpecMod->getValues();
				}
			}
			else {
				valueslist[0] = _topSpecMod->getValues();
				specmod = _topSpecMod;
				totbins = _topSpecMod->getLength();
				valueslist[1] = 0;

				// do both
				if (event.LeftIsDown()) {
					valueslist[1] = _specMod->getValues();
				}
			}
		}
		else if (_specMod) {
			valueslist[0] = _specMod->getValues();
			specmod = _specMod;
			totbins = _specMod->getLength();
			valueslist[1] = 0;

			if (event.LeftIsDown() && _topSpecMod) {
				valueslist[1] = _topSpecMod->getValues();
			}
		}
		else {
			event.Skip();
			return;
		}

		
		if (_gridSnapFlag)
		{
			int fromi, toi;
			xToBinRange(_firstX, fromi, toi);
			
			if (_firstMove) {
				_lastVal = snapValue(valueslist[0][fromi]);
				_firstMove = false;
			}
			else {
				if (_lastY != pY) {
					float tval = valDiffY (_lastVal, _lastY, pY);
					if (tval == _lastVal) {
						//
						//printf ("not good enough returning: %g\n", tval);
						pY = _lastY;
					}
					else {
						_lastVal = tval;
						//printf ("good enough: new lastval=%g\n", _lastVal);					
					}
				}
			}
		}
		
		// compute difference in x and y from last
		int diffX, diffY;
		diffX = pX - _lastX;
		diffY = pY - _lastY;
		
		int shiftbins = xDeltaToBinDelta (diffX);
		//float shiftval = yToVal(pY) - yToVal(_lastY);

		//printf ("shiftbins %d   diffx %d   diffy %d\n", shiftbins, diffX, diffY);

		for (int n = 0; n < 2 && valueslist[n]; n++)
		{
			values = valueslist[n];
			
			if (shiftbins < 0) {
				// shiftbins is NEGATIVE shift left

				// store first shiftbins
				for (i=0; i < -shiftbins; i++) {
					_tmpfilt[i] = valDiffY (values[i],  _lastY, pY);
				}
			
				for (i=0; i < totbins + shiftbins; i++) {
					values[i] = valDiffY (values[i-shiftbins], _lastY, pY);
				}

				// finish off with end
				if (edgehold) {
					for (i=totbins+shiftbins; i < totbins; i++) {
						values[i] = values[i-1];
					}
				}
				else {
					for (j=0, i=totbins+shiftbins; i < totbins; i++, j++) {
						values[i] = _tmpfilt[j];
					}
				}
			}
			else if (shiftbins > 0) {

				// shiftbins is POSITIVE, shift right
				// store last shiftbins
				for (i=totbins-shiftbins; i < totbins; i++) {
					_tmpfilt[i] = valDiffY (values[i], _lastY, pY);
				}
			
			
				for ( i=totbins-1; i >= shiftbins; i--) {
					values[i] = valDiffY (values[i-shiftbins],  _lastY, pY);
				}

				// start off with first values (either wrapped or edge held)
				if (edgehold) {
					for ( i=shiftbins-1; i >= 0; i--) {
						values[i] = values[i+1];
					}
				}
				else {
					for (j=totbins-shiftbins, i=0; i < shiftbins; i++, j++) {
						values[i] = _tmpfilt[j];
					}
				}
			}
			else {
				// no bin shift just values
				for ( i=0; i < totbins; i++) {
					values[i] = valDiffY (values[i], _lastY, pY);
				}
			}

		}

		
		Refresh(FALSE);
		_mainwin->updateGraphs(this, specm->getSpecModifierType());

		
		if (shiftbins != 0) {
			_lastX = pX;
		}
		
		_lastY = pY;

		updatePositionLabels(pX, pY, true, specmod);

	}
	else if (event.ButtonUp()  && !event.LeftIsDown() && !event.RightIsDown()) {
		if (_mouseCaptured) {
			ReleaseMouse();
			_mouseCaptured = false;
		}
		
		SetCursor(wxCURSOR_PENCIL);
		_dragging = false;

		if (event.RightUp() && event.ControlDown() && event.AltDown())
		{
			if (event.ShiftDown())
			{
				// reset zoom
				_zooming = false;
				if (_specMod->getModifierType() == FTspectrumModifier::GAIN_MODIFIER)
				{
					_mindb = _absmindb;
					_maxdb = _absmaxdb;
					_min = dbToVal(_mindb);
					_max = dbToVal(_maxdb);
					recalculate();
				}
				else if (_specMod->getModifierType() == FTspectrumModifier::POS_GAIN_MODIFIER)
				{
					_mindb = _absposmindb;
					_maxdb = _absposmaxdb;
					_min = dbToVal(_mindb);
					_max = dbToVal(_maxdb);
					recalculate();
				}
				else {
					setMinMax (_absmin, _absmax);
				}
			}
			else {
				// reset filter
				if (_specMod) {
					_specMod->reset();
				}
				if (_topSpecMod) {
					_topSpecMod->reset();
				}
				
				Refresh(FALSE);
				
				_mainwin->updateGraphs(this, specm->getSpecModifierType());
				updatePositionLabels(pX, pY, true);
			}
		}
		else if (_zooming)
		{
			// commit zoom
			_zooming = false;
			if (_gridSnapFlag) {
				setMinMax ( snapValue(yToVal (_bottomzoomY)), snapValue(yToVal (_topzoomY)) );
			} else {
				setMinMax ( yToVal (_bottomzoomY), yToVal (_topzoomY) );
			}
		}

		event.Skip();
	}
	else if (event.Moving())
	{
		if (_topSpecMod && !event.ShiftDown()) {
			updatePositionLabels(pX, pY, true, _topSpecMod);
		}
		else if (_specMod) {
			updatePositionLabels(pX, pY, true, _specMod);
		}
	}
	else {
		event.Skip();
	}

}

void FTactiveBarGraph::updatePositionLabels(int pX, int pY, bool showreal, FTspectrumModifier *specmod)
{
	// calculate freq range and val for status bar
	float sfreq, efreq;
	int frombin, tobin;
	xToFreqRange(pX, sfreq, efreq, frombin, tobin);
	_freqstr.Printf (wxT("%5d - %5d Hz"), (int) sfreq, (int) efreq);

	float val, realval;	

	if (!specmod) {
		if (_specMod) specmod = _specMod;
		else if (_topSpecMod) specmod = _topSpecMod;
		else return;
	}

	float *data = specmod->getValues();

	if (specmod->getModifierType() == FTspectrumModifier::GAIN_MODIFIER
	    ||specmod->getModifierType() == FTspectrumModifier::POS_GAIN_MODIFIER)
	{
		val = yToDb (pY);
		if (showreal) {
			realval = valToDb (data[frombin]);
			_valstr.Printf (wxT("C: %7.1f dB  @: %7.1f dB"), val, realval);
		}
		else {
			_valstr.Printf (wxT("C: %7.1f dB"), val);
		}
	}
	else if (specmod->getModifierType() == FTspectrumModifier::SEMITONE_MODIFIER) {
		val = yToSemi (pY);
		if (showreal) {
			realval = valToSemi (data[frombin]);
			_valstr.Printf (wxT("C: %7.1f semi  @: %7.1f semi"), val, realval);
		}
		else {
			_valstr.Printf (wxT("C: %7.1f dB"), val);
		}
	}
	else if (specmod->getModifierType() == FTspectrumModifier::TIME_MODIFIER) {
		
		val = yToVal (pY);
		if (showreal) {
			realval = data[frombin];
			if (_gridChoiceIndex >= _beatscutoff) {
				_valstr.Printf (wxT("C: %7.3f beats  @: %7.3f beats"), val * _tempo/60.0, realval * _tempo/60.0);
			} else {
				_valstr.Printf (wxT("C: %7.0f ms  @: %7.0f ms"), val * 1000.0, realval * 1000.0);
			}
		}
		else {
			if (_gridChoiceIndex >= _beatscutoff) {
				_valstr.Printf (wxT("C: %7.3f beats"), val * _tempo/60.0);
			} else {
				_valstr.Printf (wxT("C: %7.0f ms"), val * 1000.0);
			}
		}
	}
	else if (specmod->getModifierType() == FTspectrumModifier::UNIFORM_MODIFIER) {
		
		val = yToVal (pY);
		if (showreal) {
			realval = data[frombin];
			_valstr.Printf (wxT("C: %7.1f %%  @: %7.1f %%"), val * 100.0, realval * 100.0);
		}
		else {
			_valstr.Printf (wxT("C: %7.1f %%"), val * 100.0);
		}
	}
	else if (specmod->getModifierType() == FTspectrumModifier::DB_MODIFIER) {
		val = yToVal (pY);
		if (showreal) {
			realval = data[frombin];
			_valstr.Printf (wxT("C: %7.2f dB  @: %7.2f dB"), val, realval);
		}
		else {
			_valstr.Printf (wxT("C: %8.2f dB"), val);
		}
		
	}
	else if (specmod->getModifierType() == FTspectrumModifier::FREQ_MODIFIER) {
		FTioSupport *iosup = FTioSupport::instance();
		float samprate = (float) iosup->getSampleRate();
		
		val = yToVal (pY);

		val = ((val - _absmin) / (_absmax-_absmin)) * samprate * 0.5;
		
		if (showreal) {
			realval = data[frombin];
			realval = ((realval - _absmin) / (_absmax-_absmin)) * samprate * 0.5;
			
			_valstr.Printf (wxT("C: %5d Hz  @: %5d Hz"), (int) val, (int) realval);
		}
		else {
			_valstr.Printf (wxT("C: %5d Hz"), (int)val);
		}
	}
	else {
		val = yToVal (pY);
		if (showreal) {
			realval = data[frombin];
			_valstr.Printf (wxT("C: %7.3f  @: %7.3f "), val, realval);
		}
		else {
			_valstr.Printf (wxT("C: %8.3f"), val);
		}
		
	}
	
	_mainwin->updatePosition ( _freqstr, _valstr );
}


void FTactiveBarGraph::OnXscaleMenu (wxCommandEvent &event)
{
	//wxMenuItem *item = (wxMenuItem *) event.GetEventObject();

	
	if (event.GetId() == FT_1Xscale) {
		_xScaleType = XSCALE_1X;
	}
	else if (event.GetId() == FT_2Xscale) {
		_xScaleType = XSCALE_2X;
	}
	else if (event.GetId() == FT_LogaXscale) {
		_xScaleType = XSCALE_LOGA;
	}
	else if (event.GetId() == FT_LogbXscale) {
		_xScaleType = XSCALE_LOGB;
	}
	else {
		event.Skip();
	}

	Refresh(FALSE);
}
