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

#ifndef __FTUTILS_HPP__
#define __FTUTILS_HPP__

#include <iostream>
#include <sstream>
#include <cmath>

#include <wx/string.h>

#if 0
#  define DEBUGOUT(x) (std::cout << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << ": " << x)
#else
#  define DEBUGOUT(x) ;
#endif

class FTutils
{
public:

/* Rough but quick versions of some basic math functions */
/* There is no input error checking.                     */
	
	
/* Logarithm base-10.      */
/* absolute error < 6.4e-4 */
/* input must be > 0       */
	static float fast_log10 (float x);

/* Logarithm base-2.      */
/* absolute error < 6.4e-4 */
/* input must be > 0       */
	static float fast_log2 (float x);
	

/* Square root.           */
/* relative error < 0.08% */
/* input must be >= 0     */
	static float fast_square_root (float x);


/* Fourth root.           */
/* relative error < 0.06% */
/* input must be >= 0     */
	static float fast_fourth_root (float x);


/* vector versions of the above.    */
/* "in" and "out" pointer can refer */
/* to the same array for in-place   */
/* computation                      */

	static void vector_fast_log2 (const float* x_input, float* y_output, int N);
	static void vector_fast_log10 (const float* x_input, float* y_output, int N);
	static void vector_fast_square_root (const float* x_input, float* y_output, int N);
	static void vector_fast_fourth_root (const float* x_input, float* y_output, int N);
	


	static inline float powerLogScale(float yval, float min);


	static inline float f_clamp(float x, float a, float b)
		{
			const float x1 = std::fabs(x - a);
			const float x2 = std::fabs(x - b);
			
			x = x1 + a + b;
			x -= x2;
			x *= 0.5;
			
			return x;
		}

	/* these are not too cheap */
	static double sine_wave     (double time, double freq_Hz);
	static double square_wave   (double time, double freq_Hz);
	static double triangle_wave (double time, double freq_Hz);

	
};



inline float FTutils::powerLogScale(float yval, float min)
{
			
	if (yval <= min) {
		return -200.0;
	}
	
//   	if (yval > _maxval) {
//   		_maxval = yval;
//   	}
	
	//float nval = (10.0 * FTutils::fast_log10(yval / max));
	float nval = (10.0 * FTutils::fast_log10 (yval));
	// printf ("scaled value is %g   mincut=%g\n", nval, _minCutoff);
	return nval;
}



/* 32 bit "pointer cast" union */
typedef union {
        float f;
        int32_t i;
} ls_pcast32;
	
static inline float flush_to_zero(float f)
{
	ls_pcast32 v;

	v.f = f;

	// original: return (v.i & 0x7f800000) == 0 ? 0.0f : f;
	// version from Tim Blechmann
	return (v.i & 0x7f800000) < 0x08000000 ? 0.0f : f;
}



/* A set of branchless clipping operations from Laurent de Soras */

static inline float f_max(float x, float a)
{
	x -= a;
	x += fabsf(x);
	x *= 0.5;
	x += a;

	return x;
}

static inline float f_min(float x, float b)
{
	x = b - x;
	x += fabsf(x);
	x *= 0.5;
	x = b - x;

	return x;
}

static inline float f_clamp(float x, float a, float b)
{
	const float x1 = fabsf(x - a);
	const float x2 = fabsf(x - b);

	x = x1 + a + b;
	x -= x2;
	x *= 0.5;

	return x;
}


struct LocaleGuard {
	LocaleGuard (const char*);
	~LocaleGuard ();
	const char* old;
};


#define DB_CO(g) ((g) > -90.0f ? pow(10.0, (g) * 0.05) : 0.0)
#define CO_DB(v) (20.0 * log10(v))


#endif
