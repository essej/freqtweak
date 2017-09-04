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

#ifndef __FTTYPES_HPP__
#define __FTTYPES_HPP__

#include <inttypes.h>

// these should match jack's types but we'll avoid the include
// for now

typedef float sample_t;
typedef uint32_t nframes_t;

typedef float fft_data;



#define FT_MAXPATHS 4

#define FT_FIFOLENGTH (1 << 18)

#define FT_MAX_FFT_SIZE 16384
#define FT_MAX_FFT_SIZE_HALF  (FT_MAX_FFT_SIZE / 2)
#define FT_MAX_AVERAGES 128
#define FT_MAX_OVERSAMP  16



enum SpecModType {
	ALL_SPECMOD = 0,
	FREQ_SPECMOD,
	DELAY_SPECMOD,
	FEEDB_SPECMOD,
	SCALE_SPECMOD,
	GATE_SPECMOD,
	SQUELCH_SPECMOD,
	MASH_SPECMOD,
	WARP_SPECMOD,
	EXPAND_SPECMOD,
	COMPRESS_SPECMOD,
	BOOST_SPECMOD,
	RESCUT_SPECMOD,
	RESCUTEQ_SPECMOD
};


enum XScaleType
{
	XSCALE_1X = 0, // 1:1 plot bin to filter bin
	XSCALE_2X, // ~ 2:1 "  with last bin representing the entire upper half
	XSCALE_3X, // ~ 3:1 "  with last 2 bins representing to upper thirds
	XSCALE_4X, // ~ 4:1 "  with last 3 bins representing each upper 1/4 
	XSCALE_LOGA, // pseudo-log scale with lower freqs having bigger bins
	XSCALE_LOGB, // pseudo-log scale with lower freqs having bigger bins
};

enum YScaleType
{
	YSCALE_1X, // 
	YSCALE_2X, // 
	YSCALE_3X, // 
	YSCALE_4X, // 
};


#endif
