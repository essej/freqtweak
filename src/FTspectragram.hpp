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

#ifndef __FTSPECTRAGRAM_HPP__
#define __FTSPECTRAGRAM_HPP__

#include <wx/wx.h>
#include <wx/image.h>

#include "FTutils.hpp"
#include "FTtypes.hpp"

class FTmainwin;

class FTspectragram
	: public wxPanel
{
	//DECLARE_DYNAMIC_CLASS(FTspectragram)
	
public:
	enum PlotType
	{
		SPECTRAGRAM = 0,
		AMPFREQ_SOLID,
		AMPFREQ_LINES,
	};


	FTspectragram(FTmainwin *mwin, wxWindow *parent, wxWindowID id, 
		      const wxPoint& pos = wxDefaultPosition,
		      const wxSize& size = wxDefaultSize,
		      long style = 0,
		      const wxString& name = wxT("Spectragram"),
		      PlotType pt = SPECTRAGRAM);
//		      PlotType pt = AMPFREQ_SOLID);
//		      PlotType pt = AMPFREQ_LINES);

	virtual ~FTspectragram();

	enum  ColorTableType
	{
		COLOR_GRAYSCALE = 0,
		COLOR_BVRYW,
		COLOR_GREENSCALE
	};

	
	void plotNextData (const float *data, int length);

	void setPlotType (PlotType pt);
	PlotType getPlotType() { return _ptype; }

	void setXscale(XScaleType sc);
	XScaleType getXscale() { return _xScaleType; }
	
	void setDataLength(unsigned int len);
	
	void OnPaint ( wxPaintEvent &event);
	void OnSize ( wxSizeEvent &event);
	void OnMouseActivity ( wxMouseEvent &event );

	void OnXscaleMenu (wxCommandEvent &event);
	
	float powerLogScale(float yval);
	
  protected:
	void updateSize();

	void initColorTable();
	void plotNextDataSpectragram (const float *data, int length);
	void plotNextDataAmpFreq (const float *data, int length);

	void updatePositionLabels (int pX, int pY, bool showreal);

	void xToFreqRange(int x, float &fromfreq, float &tofreq, int &frombin, int &tobin);
	void xToBinRange(int x, int &frombin, int &tobin);
	void binToXRange(int bin, int &fromx, int &tox, int width, int bins);

	bool setDiscreteColor(int index, int r, int g, int b) {
		
		if (index < _discreteColorCount)
		{
			_discreteColors[index][0] = r;
			_discreteColors[index][1] = g;
			_discreteColors[index][2] = b;
			
			return true;
		}
		return false;
	}

	
	FTmainwin * _mwin;
	PlotType _ptype;
	
	int _width, _height;
	
	float _minCutoff;
	float _dbAbsMin;
	//float _dataRefMax;
	float _dbAdjust;

	float _yMin, _yMax;
	
	// bitmaps
	wxBitmap * _raster;
	wxBitmap * _imageBuf;

	wxImage * _rasterImage;
	unsigned char * _rasterData;
	
	wxPoint *_points;
	
	// color table stuff
	ColorTableType _colorTableType;
	
	static int _maxColorCount;
	static int _maxDiscreteColorCount;
	static int _colorCount;
	static int _discreteColorCount;
	static unsigned char **_colorMap;
	static unsigned char **_discreteColors;

	
	bool _justresized;

	wxBrush _fillBrush;
	wxColour _fillColor;

	wxPen _linePen;
	
	float _maxval;

	float _xscale;
	int _length;

	XScaleType _xScaleType;
	
	wxString _freqstr;

	wxMenu * _xscaleMenu;

	unsigned int _dataLength;
  private:
	// any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()	

};




#endif
