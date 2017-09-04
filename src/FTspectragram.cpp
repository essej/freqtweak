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
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <math.h>


#include "FTspectragram.hpp"

#include "FTmainwin.hpp"
#include "FTioSupport.hpp"

#include "FTtypes.hpp"

int FTspectragram::_colorCount = 128;
int FTspectragram::_maxColorCount = 256;
int FTspectragram::_maxDiscreteColorCount = 16;

int FTspectragram::_discreteColorCount = 10;

unsigned char ** FTspectragram::_colorMap = 0;
unsigned char ** FTspectragram::_discreteColors = 0;


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
BEGIN_EVENT_TABLE(FTspectragram, wxPanel)
	EVT_PAINT(FTspectragram::OnPaint)
	EVT_SIZE(FTspectragram::OnSize)
	EVT_MOTION(FTspectragram::OnMouseActivity)
	EVT_ENTER_WINDOW(FTspectragram::OnMouseActivity)
	EVT_LEAVE_WINDOW(FTspectragram::OnMouseActivity)
	EVT_MIDDLE_DOWN(FTspectragram::OnMouseActivity)
	EVT_MIDDLE_UP(FTspectragram::OnMouseActivity)
	EVT_RIGHT_DOWN(FTspectragram::OnMouseActivity)
	EVT_RIGHT_UP(FTspectragram::OnMouseActivity)
	
	
	EVT_MENU (FT_1Xscale,FTspectragram::OnXscaleMenu)
	EVT_MENU (FT_2Xscale,FTspectragram::OnXscaleMenu)
	EVT_MENU (FT_LogaXscale,FTspectragram::OnXscaleMenu)
	EVT_MENU (FT_LogbXscale,FTspectragram::OnXscaleMenu)
	
END_EVENT_TABLE()


FTspectragram::FTspectragram(FTmainwin * mwin, wxWindow *parent, wxWindowID id,
			     const wxPoint& pos,
			     const wxSize& size,
			     long style ,
			     const wxString& name,
			     PlotType pt)

	: wxPanel(parent, id, pos, size, style, name)
	, _mwin(mwin), _ptype(pt)
	, _width(0), _height(0)
	, _minCutoff(0.0), _dbAbsMin(-200.0), _dbAdjust(-38.0)
	, _yMin(-90.0), _yMax(0.0), _imageBuf(0)
	, _rasterImage(0), _rasterData(0)
	, _colorTableType(COLOR_BVRYW), _justresized(false)
	, _fillColor(20,120,120), _maxval(0.0), _xScaleType(XSCALE_1X)
	, _dataLength(1024)
{
	initColorTable();

	// max size
	_rasterData = new unsigned char[1600 * 3];
	_rasterImage = new wxImage(1600, 1 , _rasterData, true);
	
	
	_points = new wxPoint[FT_MAX_FFT_SIZE/2 + 3];

	_fillBrush.SetColour(_fillColor);
	_fillBrush.SetStyle(wxSOLID);
	
	_xscaleMenu = new wxMenu(wxT(""));
	_xscaleMenu->Append ( new wxMenuItem(_xscaleMenu, FT_1Xscale, wxT("1x Scale")));
	_xscaleMenu->Append ( new wxMenuItem(_xscaleMenu, FT_2Xscale, wxT("2x Scale")));
	_xscaleMenu->Append ( new wxMenuItem(_xscaleMenu, FT_LogaXscale, wxT("logA Scale")));
	_xscaleMenu->Append ( new wxMenuItem(_xscaleMenu, FT_LogbXscale, wxT("logB Scale")));

	_linePen.SetColour(255,255,255);

	updateSize();
}

FTspectragram::~FTspectragram()
{
	delete [] _rasterData;
	if (_imageBuf) delete _imageBuf;
	if (_rasterImage) delete _rasterImage;

	delete _xscaleMenu;
	delete [] _points;

	
}

void FTspectragram::setDataLength(unsigned int len)
{
	// this is the new max length our raster
	// data needs to handle
	if (_rasterData)
		delete [] _rasterData;
	
	if ((int) len <  _width) {
		_rasterData = new unsigned char[_width * 3];
		_dataLength = _width;
	}
	else {
		_rasterData = new unsigned char[len * 3];
		_dataLength = len;
	}

	if (_rasterImage)
		delete _rasterImage;
	
	//_rasterImage = new wxImage(_width, 1 , _rasterData, true);
	_rasterImage = new wxImage(_dataLength, 1 , _rasterData, true);

}


void FTspectragram::setPlotType (PlotType pt)
{
	if (_ptype != pt) {
		_ptype = pt;		
		_justresized = true;

		
		Refresh(TRUE);
	}
}

void FTspectragram::OnPaint(wxPaintEvent & event)
{
	//printf ("Spectragram::OnPaint\n");

	wxPaintDC dc(this);
	wxMemoryDC sdc;

	
	if (_justresized)
	{
		if (_imageBuf) delete _imageBuf;
		_imageBuf = new wxBitmap(_width, _height);

		sdc.SelectObject(*_imageBuf);

		sdc.SetBackground(*wxBLACK_BRUSH);
		sdc.Clear();
		_justresized = false;
	}

	if (_imageBuf) {
		sdc.SelectObject(*_imageBuf);
		dc.Blit(0,0, _width, _height, &sdc, 0,0);
	}

	
	//event.Skip();
}


void FTspectragram::updateSize()
{
	int w,h;
	GetClientSize(&w, &h);

	//printf ("Spectragram::OnSize  w=%d  h=%d\n", _width, _height);
		
	if (w != _width || h != _height) {
		_justresized = true;
	}

	_width = w;
	_height = h;


	if (_width > (int) _dataLength) {
		setDataLength(_width);
	}
	else {
		if (_rasterImage) 
			delete _rasterImage;
		//_rasterImage = new wxImage(_width, 1 , _rasterData, true);
		_rasterImage = new wxImage(_dataLength, 1 , _rasterData, true);
	}
}

void FTspectragram::OnSize(wxSizeEvent & event)
{
	updateSize();
	event.Skip();
}


void FTspectragram::plotNextData (const float *data, int length)
{

	if (_ptype == SPECTRAGRAM) {
		plotNextDataSpectragram (data, length);
	}
	else if (_ptype == AMPFREQ_LINES || _ptype == AMPFREQ_SOLID) {
		plotNextDataAmpFreq (data, length);
	}

	//printf ("maxval is %g\n", _maxval);
}


void FTspectragram::plotNextDataSpectragram (const float *data, int length)
{
	wxClientDC dc(this);
	
	//wxMemoryDC rdc;
	//rdc.SelectObject(*_raster);
	
	wxMemoryDC sdc;
	if (_imageBuf)
		sdc.SelectObject(*_imageBuf);
	else return;

	//sdc.SetOptimization(true);
	//dc.SetOptimization(true);
	//rdc.SetOptimization(true);

	
	float dbval, sum;
	int coli, i, k, j;

	double xSkipD = (double)length / (double)_width;
	int    xSkipI = (int)(xSkipD + 0.5);
	if (xSkipI < 1) xSkipI = 1;

	//wxPen pen = rdc.GetPen();

	_xscale = (double)_width/(double)length;
	_length = length;

	int x1,x2;
	
//	if (_width >= length)
	if (_width == length)
	{
		/*
		if (_width > length) {
			//printf ("undersampled: w=%d  l=%d  sxale=%g\n", _width, length, xscale);
			rdc.SetUserScale(_xscale, 1.0);
		}
		*/
		
		// draw new raster
		for (int i=0; i < length; i++)
		{
			dbval = powerLogScale(data[i]);
			coli = (int) (((dbval - _yMin) / ( _yMax - _yMin)) * _colorCount);
			if (coli >= _colorCount) {
				coli = _colorCount-1;
			}
			else if (coli < 0) {
				coli = 0;
			}

			//pen.SetColour((int)_colorMap[coli][0], (int)_colorMap[coli][1], (int)_colorMap[coli][2] );
			//rdc.SetPen(pen);
			//rdc.DrawLine(i,0,i+1,0);

			binToXRange(i, x1, x2, _length, _length);
			for (int n=x1; n <= x2; n++) { 
				_rasterData[3*n] = _colorMap[coli][0];
				_rasterData[3*n + 1] = _colorMap[coli][1];
				_rasterData[3*n + 2] = _colorMap[coli][2];
			}
			
		}

		
	}
	else if (_width > length)
	{
		double xscale = (double)_width / ((double)length-1);
      
		int xadj = (int) (xscale * 0.5);

		double xposf = -xadj;
		int lasti = 0;
		coli = 0;
      
		for(j = 0; j < length; j++, xposf+=xscale)
		{
			i = (int) lrint(xposf);
	      
			while (lasti < i) {
				// fill blanks with last

				binToXRange(lasti, x1, x2, _length, _length);
				for (int n=x1; n <= x2; ++n) { 
					_rasterData[3*n] = _colorMap[coli][0];
					_rasterData[3*n+1] = _colorMap[coli][1];
					_rasterData[3*n+2] = _colorMap[coli][2];
				}		      
				++lasti;
			}
	      
			dbval = powerLogScale(data[j]);
	      
			// normalize it from _dataRefMin/Max to 0-numcolors
			coli = (int) (((dbval - _yMin) / ( _yMax - _yMin)) * _colorCount);
	      
			if (coli >= (int)_colorCount) {
				coli = (int) (_colorCount-1);
			}
			else if (coli < 0) {
				coli = 0;
			}
	      
		}
		
		while (lasti < _width) {
			// fill blanks with last
			binToXRange(lasti, x1, x2, _length, _length);
			for (int n=x1; n <= x2; ++n) { 
				_rasterData[3*n] = _colorMap[coli][0];
				_rasterData[3*n+1] =  _colorMap[coli][1];
				_rasterData[3*n+2] =  _colorMap[coli][2];
			}
			++lasti;
		}
	      
	}
	else {
		j = 0;
		int nextj;
		
		for(i = 0; i < _width; i++)
		{
			nextj = (int)((i+1)*xSkipD + 0.5);
			sum = -100000;

			for (; j < nextj; ++j)
			{
				if ((data[j] ) > sum)
				{
					sum = (data[j] );
				}
			}

			dbval = powerLogScale(sum);
	    
			// normalize it from _dataRefMin/Max to 0-numcolors
			//coli = (int) (((sum - _dataRefMin) / refDiff) * _colorCount);

			coli = (int) (((dbval - _yMin) / ( _yMax - _yMin)) * _colorCount);

			if (coli >= (int) _colorCount) {
				coli = (int)_colorCount-1;
			}
			else if (coli < 0) {
				coli = 0;
			}
	 
			binToXRange(i, x1, x2, _length, _length);
			for (int n=x1; n <= x2; ++n) { 
				_rasterData[3*n] =  _colorMap[coli][0];
				_rasterData[3*n+1] =  _colorMap[coli][1];
				_rasterData[3*n+2] =  _colorMap[coli][2];
			}
	 
		}


	}
/*
	else {
		// Assume they want average
		// (_scaleType == DATA_MAX)
		//_dataElements = _plotWidth;
		
		for(i = 0; i < _width; i++)
		{

			sum = 0;
			for (k = 0, j = (int)(i*xSkipD); k < xSkipI; j++, k++)
			{
				sum += (data[j]);
				//if (sum < data[j]) {
				//	sum = data[j];
				//}
			}
			sum /= xSkipI;

			
			dbval = powerLogScale(sum);
			
			coli = (int) (((dbval - _yMin) / ( _yMax - _yMin)) * _colorCount);
			if (coli >= _colorCount) {
				coli = _colorCount-1;
			}
			else if (coli < 0) {
				coli = 0;
			}

			//printf ("dbval= %g   coli=%d\n", dbval, coli);

			//pen.SetColour((int)_colorMap[coli][0], (int)_colorMap[coli][1], (int)_colorMap[coli][2] );
			//rdc.SetPen(pen);
			//rdc.DrawLine(i,0, i+1, 0);

			binToXRange(i, x1, x2, _length, _length);
			for (int n=x1; n <= x2; n++) { 
				_rasterData[3*n] = _colorMap[coli][0];
				_rasterData[3*n + 1] = _colorMap[coli][1];
				_rasterData[3*n + 2] = _colorMap[coli][2];
				}

			
			}
	}
*/
	
	// this is double buffered
	// move the specbuf down
	sdc.Blit(0,1,_width,_height-1, &sdc, 0, 0); 

	// blit the new raster
	//sdc.Blit(0,0,_width, 1, &rdc, 0,0);

	
	//if (_width > length) {
		//printf ("undersampled: w=%d  l=%d  sxale=%g\n", _width, length, _xscale);
	//	sdc.SetUserScale(_xscale, 1.0);
		//}

	sdc.DrawBitmap (wxBitmap (*_rasterImage), 0, 0, false);
	
	
	// blit to screen
	dc.Blit(0,0, _width, _height, &sdc, 0,0);

	
}

void FTspectragram::plotNextDataAmpFreq (const float *data, int length)
{
	
	// printf ("Plot new data\n");
	wxClientDC dc(this);
	
	
	wxMemoryDC sdc;
	if (_imageBuf)
		sdc.SelectObject(*_imageBuf);
	else
		return;

	//sdc.SetOptimization(true);
	//dc.SetOptimization(true);
	
	float dbval, sum;
	int yval, i, k, j;

	double xSkipD = (double)length / (double)_width;
	int    xSkipI = (int)(xSkipD + 0.5);
	if (xSkipI < 1) xSkipI = 1;

	_xscale = (double)_width/(double)length;
	_length = length;

	sdc.SetBackground(*wxBLACK_BRUSH);
	sdc.Clear();

	sdc.SetPen(_linePen);
	sdc.SetBrush(_fillBrush);

	_points[0].x = 0;
	_points[0].y = _height + 2;

	int x1,x2, lastx=0;
	
	if (_width >= length)
	{
		
		for (int i=0; i < length; i++)
		{
			dbval = powerLogScale(data[i]);
			yval = _height - (int) (((dbval - _yMin) / ( _yMax - _yMin)) * _height);
			if (yval >= _height) {
				yval = _height;
			}
			else if (yval < 0) {
				yval = 0;
			}

			switch (_ptype)
			{
			case AMPFREQ_LINES:
				if (_xScaleType != XSCALE_1X) {
					binToXRange(i, x1, x2, _length, _length);
					_points[i+2].x = (int) ( (x1+ ((x2-x1)>>1)) * _xscale);
				}
				else {
					_points[i+2].x = (int) (i * _xscale);
				}
				
				_points[i+2].y = yval;
			
				//printf ("avg  dbval=%g  yval=%d  data=%g\n", dbval, yval, sum);
				break;
			case AMPFREQ_SOLID:
				binToXRange(i, x1, x2, _width, _length);

				
				sdc.DrawRectangle ( (int)(lastx), yval,
						    (int) (x2-lastx) , _height-yval);
				
				lastx = x2;
								
				break;
			default: break;
			}

			
			//printf ("dbval=%g  yval=%d   data=%g\n", dbval, yval, data[i]);
			
		}


		if (_ptype == AMPFREQ_LINES) {
			_points[1].x = 0;
			_points[1].y = _points[2].y;
		
			_points[length].x = _width + 1;
			_points[length].y = _height + 2;
			_points[length+1].x = _points[0].x;
			_points[length+1].y = _points[0].y;

//			sdc.DrawLines(length+2, _points);
 			sdc.DrawPolygon(length+2, _points);
		}
		
	}
	else {
		// Assume they want average
		// (_scaleType == DATA_MAX)
		//_dataElements = _plotWidth;
		
		for(i = 0; i < _width; i++)
		{

			sum = 0;
			for (k = 0, j = (int)(i*xSkipD); k < xSkipI; j++, k++)
			{ 
				sum += (data[j]);
			}
			sum /= xSkipI;

			dbval = powerLogScale(sum);
			
			yval = _height - (int) (((dbval - _yMin) / ( _yMax - _yMin)) * _height);

			if (yval >= _height) {
				yval = _height;
			}
			else if (yval < 0) {
				yval = 0;
			}

			switch (_ptype)
			{
			case AMPFREQ_LINES:
				if (_xScaleType != XSCALE_1X) {
					binToXRange(i, x1, x2, _width, _width);
					_points[i+1].x = (int) ( x1+ ((x2-x1)>>1));
				}
				else {
					_points[i+1].x = (int) (i);
				}
				
				_points[i+2].y = yval;
			
				//printf ("avg  dbval=%g  yval=%d  data=%g\n", dbval, yval, sum);
				break;
			case AMPFREQ_SOLID:
				// treat even points as x,y  odd as width,height
				binToXRange(i, x1, x2, _width, _width);
				
				sdc.DrawRectangle ( lastx, yval, (x2-lastx), _height-yval);
				
				lastx = x2;
								
				break;
			default: break;
			}
			
		}

		//printf ("undersampled: w=%d  l=%d  sxale=%g\n", _width, length, xscale);
		//sdc.SetUserScale(_xscale, 1.0);

 		if (_ptype == AMPFREQ_LINES) {
			_points[1].x = 0;
			_points[1].y = _points[2].y;
			
			_points[_width].x = _width+1;
			_points[_width].y = _height + 2;
			_points[_width+1].x = _points[0].x;
			_points[_width+1].y = _points[0].y;
			
//			sdc.DrawLines(_width+2, _points);
 			sdc.DrawPolygon(_width+2, _points);
		}

		
	}
	
	
	// this is double buffered
	//sdc.SetTextForeground(*wxWHITE);
	//sdc.DrawText(wxString::Format("%g", _maxval), 0, 0);
	
	// blit to screen
	dc.Blit(0,0, _width, _height, &sdc, 0,0);
	
}

void FTspectragram::setXscale(XScaleType sc)
{
	_xScaleType = sc; 
	Refresh(FALSE);
}


void FTspectragram::initColorTable()
{
   if (!_colorMap) {
      // this is only allocated once (and it is static)
      _colorMap = new unsigned char * [_maxColorCount];
      for (int i=0; i < _maxColorCount; i++) {
	 _colorMap[i] = new unsigned char[3];
      }
   }

   if (!_discreteColors) {
      _discreteColors = new unsigned char * [_maxDiscreteColorCount];
      for (int i=0; i < _maxDiscreteColorCount; i++) {
	 _discreteColors[i] = new unsigned char[3];
      }

      // for now we are just using
      if (_colorTableType == COLOR_GRAYSCALE) {
	 _discreteColorCount = 2;
	 // black
	 _discreteColors[0][0] = 0;
	 _discreteColors[0][1] = 0;
	 _discreteColors[0][2] = 0;
	 // white
	 _discreteColors[1][0] = 0xff;
	 _discreteColors[1][1] = 0xff;
	 _discreteColors[1][2] = 0xff;
      }
      if (_colorTableType == COLOR_GREENSCALE) {
	 _discreteColorCount = 2;
	 // black
	 _discreteColors[0][0] = 0;
	 _discreteColors[0][1] = 0;
	 _discreteColors[0][2] = 0;
	 // green
	 _discreteColors[1][0] = 0;
	 _discreteColors[1][1] = 200;
	 _discreteColors[1][2] = 0;
      }
      else if (_colorTableType == COLOR_BVRYW)
      {
	      _discreteColorCount = 6;
	      setDiscreteColor(0, 0, 0, 0); // black
	      setDiscreteColor(1, 0, 0, 149); // blue
	      setDiscreteColor(2, 57, 122, 138); // blue-green
	      setDiscreteColor(3, 92, 165, 79); // green
	      setDiscreteColor(4, 229, 171, 0); // orange
	      setDiscreteColor(5, 255, 0, 18); // red


	      /*  
	      _discreteColorCount = 7;
	 // black
	 _discreteColors[0][0] = 0;
	 _discreteColors[0][1] = 0;
	 _discreteColors[0][2] = 0;
	 // blue
	 _discreteColors[1][0] = 0;
	 _discreteColors[1][1] = 0;
	 _discreteColors[1][2] = 200;
	 // violet
	 _discreteColors[2][0] = 200;
	 _discreteColors[2][1] = 0;
	 _discreteColors[2][2] = 200;
	 // red
	 _discreteColors[3][0] = 200;
	 _discreteColors[3][1] = 0;
	 _discreteColors[3][2] = 0;
	 // orangish
	 _discreteColors[4][0] = 238;
	 _discreteColors[4][1] = 118;
	 _discreteColors[4][2] = 33;
	 // yellow
	 _discreteColors[5][0] = 200;
	 _discreteColors[5][1] = 200;
	 _discreteColors[5][2] = 0;
	 // pale green
	 _discreteColors[6][0] = 100;
	 _discreteColors[6][1] = 160;
	 _discreteColors[6][2] = 100;
	      */
	 

	 
      }
	
   }
    
   float seglen = _colorCount * 1.0/(_discreteColorCount-1);  
   float ratio;
   int pos = 0;

   // printf ("seglen is %f\n", seglen);
    
   for (int dcol=0; dcol < _discreteColorCount-1; dcol++)
   {
      // fade this color into the next one
      for (int i=0; i < (int)seglen; i++)
      {
	 ratio = i / seglen;
	 _colorMap[i + pos][0] = (unsigned char) (_discreteColors[dcol][0]*(1-ratio) + _discreteColors[dcol+1][0]*ratio) & 0xff ;
	 _colorMap[i + pos][1] = (unsigned char) (_discreteColors[dcol][1]*(1-ratio) + _discreteColors[dcol+1][1]*ratio) & 0xff ;
	 _colorMap[i + pos][2] = (unsigned char) (_discreteColors[dcol][2]*(1-ratio) + _discreteColors[dcol+1][2]*ratio) & 0xff ;
      }

      pos += (int) seglen;
   }

   // finish off
   for (; pos < _colorCount; pos++) {
      _colorMap[pos][0] = (unsigned char) (_discreteColors[_discreteColorCount-1][0]);
      _colorMap[pos][1] = (unsigned char) (_discreteColors[_discreteColorCount-1][1]);
      _colorMap[pos][2] = (unsigned char) (_discreteColors[_discreteColorCount-1][2]);
   }


}



float FTspectragram::powerLogScale(float yval)
{
	
	if (yval <= _minCutoff) {
		return _dbAbsMin;
	}

//   	if (yval > _maxval) {
//   		_maxval = yval;
//   	}
	
	//float nval = (10.0 * FTutils::fast_log10(yval / _dataRefMax));
	float nval = (10.0 * FTutils::fast_log10(yval)) + _dbAdjust;
	// printf ("scaled value is %g   mincut=%g\n", nval, _minCutoff);
	return nval;
	
}


void FTspectragram::updatePositionLabels(int pX, int pY, bool showreal)
{
	// calculate freq range and val for status bar
	float sfreq, efreq;
	int frombin, tobin;
	xToFreqRange(pX, sfreq, efreq, frombin, tobin);
	_freqstr.Printf (wxT("%5d - %5d Hz"), (int) sfreq, (int) efreq);

	_mwin->updatePosition ( _freqstr, wxT("") );
	
}


void FTspectragram::xToFreqRange(int x, float &fromfreq, float &tofreq, int &frombin, int &tobin)
{
	float freqPerBin =  FTioSupport::instance()->getSampleRate()/(2.0 * (double)_length);

	//printf ("specmod length = %d  freqperbin=%g\n", _specMod->getLength(), freqPerBin);
	xToBinRange(x, frombin, tobin);

	fromfreq = freqPerBin * frombin;
	tofreq = freqPerBin * tobin + freqPerBin;
}

/*
void FTspectragram::xToBinRange(int x, int &frombin, int &tobin)
{
	
	// converts x coord into filter bin
	// according to scaling

	int bin, lbin, rbin;
	int totbins = _length;
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
	else {
		frombin = 0;
		tobin = 0;
	}

	//printf ("x=%d  frombin=%d tobin=%d\n", x, frombin, tobin);
}
*/

void FTspectragram::xToBinRange(int x, int &frombin, int &tobin)
{
	// converts x coord into filter bin
	// according to scaling

	int bin, lbin, rbin;
	int totbins = _length;
	//int totbins = _specMod->getLength();
	//float xscale = _width / (float)totbins;


	
	//if (x < 0) x = 0;
	//else if (x >= _width) x = _width-1;
	
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
			lbin = rbin = (int) pow ( totbins>>1, xscale) - 1;
			
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
			lbin = rbin = (int) pow ( (float)(totbins/3.0), xscale) - 1;
			
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


void FTspectragram::binToXRange(int bin, int &fromx, int &tox, int length, int bins)
{

	// converts bin into x coord range
	// according to scaling

	int x, lx, rx;
	int totbins = bins;
	float xscale = length / (float)totbins;

	// assume width whatever passed in
	int width = length;

	//if (bin < 0) bin = 0;
	//else if (bin >= totbins) bin = totbins-1;
	
	if (_xScaleType == XSCALE_1X) {
		//bin = rbin = lbin = (int)(x / xscale);
		x = lx = rx = (int) (bin * xscale);
		//printf (" %d  %g  %d\n", x, bin*xscale, (int)(bin * xscale));
		
		// find lowest x with same bin
		while ( ((int)((lx-1)/xscale) == bin) && (lx > 0)) {
			lx--;
		}
		// find highest with same x
		while ( ((int)((rx+1)/xscale) == bin) && (rx < width-1)) {
			rx++;
		}

		fromx = lx;
		tox = rx;
	}
	else if (_xScaleType == XSCALE_2X) {
		float hxscale = xscale * 2;

		if (bin >= totbins>>1) {
			rx = (int) (totbins * xscale);
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
			while ( ((int)((rx+1)/hxscale) == bin) && (rx < width-1)) {
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
			lx = rx = (int) (totbins * xscale);
			
		} else if (bin == 0) {
			lx = 0;
			rx = (int) ((FTutils::fast_log10(bin+2) / FTutils::fast_log10(totbins*0.5)) * width);
		} else {
			lx = (int) ((FTutils::fast_log10(bin+1) / FTutils::fast_log10(totbins*0.5)) * width);
			rx = (int) ((FTutils::fast_log10(bin+2) / FTutils::fast_log10(totbins*0.5)) * width);
		}


		fromx = lx;
		tox = rx;

		// printf ("bin %d  fromx=%d  tox=%d\n", bin, fromx, tox);
	}
	else if (_xScaleType == XSCALE_LOGB)
	{
		// use log scale for freq
		if (bin > (int)(totbins*0.3333)) {
			lx = rx = (int) (totbins * xscale);
			
		} else if (bin == 0) {
			lx = 0;
			rx = (int) ((FTutils::fast_log10(bin+2) / FTutils::fast_log10(totbins*0.3333)) * width);
		} else {
			lx = (int) ((FTutils::fast_log10(bin+1) / FTutils::fast_log10(totbins*0.3333)) * width);
			rx = (int) ((FTutils::fast_log10(bin+2) / FTutils::fast_log10(totbins*0.3333)) * width);
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


void FTspectragram::OnMouseActivity( wxMouseEvent &event)
{
	int pX = event.GetX();
	int pY = event.GetY();

	if (event.Moving() || event.Entering())
	{
		updatePositionLabels(pX, pY, true);
	}
	else if (event.Leaving()) {
		_mwin->updatePosition(wxT(""), wxT(""));
	}
	else if (event.MiddleUp() || event.RightUp()) {
		// popup scale menu
		this->PopupMenu ( _xscaleMenu, pX, pY);

	}
	else {
		event.Skip();
	}
	
}



void FTspectragram::OnXscaleMenu (wxCommandEvent &event)
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
