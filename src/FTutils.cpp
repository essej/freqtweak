/*
** Copyright (C) 2002 Jesse Chappell <jesse@essej.net>
**                and Jay Gibble
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

#include "FTutils.hpp"

#include <stdint.h>

#include <locale.h>
#include <cstring>
#include <cstdlib>


#include <cmath>
using namespace std;

/*
 * Please excuse all the macros.
 * They are used for explicit inlining of
 * the "vector" versions of these functions.
 */


/* Logarithm base-10.      */
/* absolute error < 6.4e-4 */
/* input must be > 0       */
float FTutils::fast_log10 (float x)
{
    /******************************************************************/
    /* strategy: factor x into                                        */
    /* 2^k * xprime                                                   */
    /*   (where xprime is in the interval [1, 2))                     */
    /* and compute log2 of both parts.                                */
    /*                                                                */
    /* (Then convert to log10.)                                       */
    /*                                                                */
    /* a) log2 of 2^k is k (exact)                                    */
    /* b) log2 of xprime is approximated with                         */
    /*    a minimax polynomial.                                       */
    /*                                                                */
    /* The answer is the sum of the answers (a) and (b).              */
    /*                                                                */
    /* The approximation used for (b) yields max error of             */
    /* 6.4e-4 over the interval [1, 2)                                */
    /******************************************************************/


    /* minimax approximation courtesy of MAPLE                    */
    /* invocation:                                                */
    /* > with('numapprox');                                       */
    /* > y := minimax(log(x)/log(2), 1..2, [3,0], 1, 'maxerror'); */
    /* > maxerror;                                                */


#define FAST_LOG10_PREFACE                                                     \
                                                                               \
    /* coefficient to convert from log2 to log10 */                            \
    static const float log10_2 = 0.3010299957;                                 \
                                                                               \
    /* coefficients for polynomial approximation */                            \
    static const float c0 = -2.153620718;                                      \
    static const float c1 = 3.047884161;                                       \
    static const float c2 = -1.051875031;                                      \
    static const float c3 = 0.1582487046;                                      \
                                                                               \
    float ans = 0;                                                             \
                                                                               \
                                                                               \
    /* "bits" representation of the argument "x" */                            \
    uint32_t* xbits = (uint32_t*)&x;                                           \
                                                                               \
    /* exponent of IEEE float */                                               \
    int expo;                                                                  \


    FAST_LOG10_PREFACE


#define FAST_LOG10_BODY                                                        \
    /* get the IEEE float exponent and debias */                               \
    expo = (int) (*xbits >> 23) - 127;                                         \
                                                                               \
    /* force the exponent of x to zero */                                      \
    /* (range reduction to [1, 2)):    */                                      \
    *xbits &= 0x007fffff;                                                      \
    *xbits += 0x3f800000;                                                      \
                                                                               \
    /* do the polynomial approximation */                                      \
    ans = c0 + (c1 + (c2 + c3 * x) * x) * x;                                   \
                                                                               \
    /* add the log2(2^k) term */                                               \
    ans += expo;                                                               \
                                                                               \
    /******************************************/                               \
    /* convert to log10                       */                               \
    /******************************************/                               \
    ans *= log10_2;                                                            \

    FAST_LOG10_BODY

    return ans;
}

/* Logarithm base-10.      */
/* absolute error < 6.4e-4 */
/* input must be > 0       */
float FTutils::fast_log2 (float x)
{
    /******************************************************************/
    /* strategy: factor x into                                        */
    /* 2^k * xprime                                                   */
    /*   (where xprime is in the interval [1, 2))                     */
    /* and compute log2 of both parts.                                */
    /*                                                                */
    /* a) log2 of 2^k is k (exact)                                    */
    /* b) log2 of xprime is approximated with                         */
    /*    a minimax polynomial.                                       */
    /*                                                                */
    /* The answer is the sum of the answers (a) and (b).              */
    /*                                                                */
    /* The approximation used for (b) yields max error of             */
    /* 6.4e-4 over the interval [1, 2)                                */
    /******************************************************************/


    /* minimax approximation courtesy of MAPLE                    */
    /* invocation:                                                */
    /* > with('numapprox');                                       */
    /* > y := minimax(log(x)/log(2), 1..2, [3,0], 1, 'maxerror'); */
    /* > maxerror;                                                */


#define FAST_LOG2_PREFACE                                                     \
                                                                               \
    /* coefficients for polynomial approximation */                            \
    static const float c0 = -2.153620718;                                      \
    static const float c1 = 3.047884161;                                       \
    static const float c2 = -1.051875031;                                      \
    static const float c3 = 0.1582487046;                                      \
                                                                               \
    float ans = 0;                                                             \
                                                                               \
                                                                               \
    /* "bits" representation of the argument "x" */                            \
    uint32_t* xbits = (uint32_t*)&x;                                           \
                                                                               \
    /* exponent of IEEE float */                                               \
    int expo;                                                                  \


    FAST_LOG2_PREFACE


#define FAST_LOG2_BODY                                                        \
    /* get the IEEE float exponent and debias */                               \
    expo = (int) (*xbits >> 23) - 127;                                         \
                                                                               \
    /* force the exponent of x to zero */                                      \
    /* (range reduction to [1, 2)):    */                                      \
    *xbits &= 0x007fffff;                                                      \
    *xbits += 0x3f800000;                                                      \
                                                                               \
    /* do the polynomial approximation */                                      \
    ans = c0 + (c1 + (c2 + c3 * x) * x) * x;                                   \
                                                                               \
    /* add the log2(2^k) term */                                               \
    ans += expo;                                                               \

    FAST_LOG2_BODY

    return ans;
}


/* Fourth root.           */
/* relative error < 0.06% */
/* input must be >= 0     */
float FTutils::fast_fourth_root (float x)
{
    /******************************************************************/
    /* strategy: factor x into                                        */
    /* 2^(4k) * 2^({0,1,2,3}) * xprime                                */
    /*   (where xprime is in the interval [1/2, 1))                   */
    /* and compute the fourth root                                    */
    /* on each of the three parts.                                    */
    /*                                                                */
    /* a) The fourth root of 2^(4k) is 2^k                            */
    /* b) The fourth root of 2^({0,1,2,3}) is saved in a lookup table */
    /* c) The fourth root of xprime is approximated with              */
    /*    a minimax polynomial.                                       */
    /*                                                                */
    /* The answer is the product of the answers from (a),(b) and (c)  */
    /*                                                                */
    /* The approximation used for (c) yields max error of             */
    /* 5.2e-4 over the interval [1/2, 1)                              */
    /*                                                                */
    /* Relative error is always < 0.06%                               */
    /******************************************************************/

    /* minimax approximations courtesy of MAPLE                   */
    /* invocation:                                                */
    /* > with('numapprox');                                       */
    /* > y := minimax(x->(x^0.25), 0.5..1, [2,0], 1, 'maxerror'); */
    /* > maxerror;                                                */


#define FAST_FOURTH_ROOT_PREFACE                                               \
                                                                               \
    /* table of fourth roots of small powers of 2 */                           \
    static const float fourth_root_pow_2[] =                                   \
    {                                                                          \
	1.0,           /* 1^(1/4) */                                           \
	1.189207115,   /* 2^(1/4) */                                           \
	1.414213562,   /* 4^(1/4) */                                           \
	1.681792831    /* 8^(1/4) */                                           \
    };                                                                         \
                                                                               \
    /* 2nd degree poly:                            */                          \
    /* max error = 5.2e-4                          */                          \
    /* y := x -> 0.6011250669 +                    */                          \
    /* (0.5627811782 - 0.1644203886 x) x           */                          \
    static const float c0 = 0.6011250669;                                      \
    static const float c1 = 0.5627811782;                                      \
    static const float c2 = -0.1644203886;                                     \
                                                                               \
    /* 3rd degree poly:                            */                          \
    /* max error = 6.1e-5                          */                          \
    /* y := x -> 0.5511182600 + (0.7764109321 +    */                          \
    /*      (-0.4594631650 + 0.1319948485 x) x) x  */                          \
                                                                               \
    /* 4th degree poly:                            */                          \
    /* max error =  7.8e-6                         */                          \
    /* y := x -> 0.5167374448 +                    */                          \
    /* (0.9719985167 + (-0.8683104381 +            */                          \
    /* (0.5043560567 - 0.1247894259 x) x) x) x     */                          \
                                                                               \
                                                                               \
    /* "bits" representation of argument x */                                  \
    uint32_t* xbits = (uint32_t*) &x;                                          \
                                                                               \
    /* factor of 2^(4k) */                                                     \
    float two_to_4k = 1.0;                                                     \
    /* bits representation */                                                  \
    int* two_to_4k_bits = (int *) &two_to_4k;                                  \
                                                                               \
    /* exponent of IEEE float */                                               \
    int expo;                                                                  \
    /* remainder of "expo" after dividing out factor of 2^(4k) */              \
    int expo_rem;                                                              \
                                                                               \
    /* result */                                                               \
    float ans;                                                                 \


    FAST_FOURTH_ROOT_PREFACE


#define FAST_FOURTH_ROOT_BODY                                                  \
    /* get the IEEE float exponent and debias */                               \
    /* (assuming non-negative sign bit)       */                               \
    /* NOTE: we are debiasing to a reference point of 2^(-1) */                \
    expo = (int) (*xbits >> 23) - 126;                                         \
    /* get the remainder after division of exponent by 4 */                    \
    expo_rem = expo & 0x03;                                                    \
    /* do the division by 4 */                                                 \
    expo >>= 2;                                                                \
    /* rebias and shift back up to make an IEEE float */                       \
    *two_to_4k_bits = (expo + 127) << 23;                                      \
                                                                               \
    /* force the exponent of x to -1   */                                      \
    /* (range reduction to [1/2, 1):   */                                      \
                                                                               \
    /* mask out any exponent bits      */                                      \
    *xbits &= 0x007FFFFF;                                                      \
    /* "0x3F000000" is the exponent for 2^(-1) (=126) shifted left by 23 */    \
    *xbits += 0x3F000000;                                                      \
                                                                               \
    /* do the polynomial approximation */                                      \
    ans = c0 + (c1 + c2 * x) * x;                                              \
                                                                               \
    /* include the other factors */                                            \
    ans *= fourth_root_pow_2 [expo_rem];                                       \
    ans *= two_to_4k;                                                          \

	
    FAST_FOURTH_ROOT_BODY

    return ans;
}


/* Square root.           */
/* relative error < 0.08% */
/* input must be >= 0     */
float FTutils::fast_square_root (float x)
{
    /******************************************************************/
    /* strategy: factor x into                                        */
    /* 2^(2k) * 2^({0,1}) * xprime                                    */
    /*   (where xprime is in the interval [1/2, 1))                   */
    /* and compute the square root                                    */
    /* on each of the three parts.                                    */
    /*                                                                */
    /* a) The square root of 2^(2k) is 2^k                            */
    /* b) The square root of 2^({0,1}) is saved in a lookup table     */
    /* c) The square root of xprime is approximated with              */
    /*    a minimax polynomial.                                       */
    /*                                                                */
    /* The answer is the product of the answers from (a),(b) and (c)  */
    /*                                                                */
    /* The approximation used for (c) yields max error of             */
    /* 5.4e-4 over the interval [1/2, 1)                              */
    /*                                                                */
    /* Relative error is always < 0.08%                               */
    /******************************************************************/

    /* minimax approximations courtesy of MAPLE                   */
    /* invocation:                                                */
    /* > with('numapprox');                                       */
    /* > y := minimax(x->(x^0.5), 0.5..1, [2,0], 1, 'maxerror'); */
    /* > maxerror;                                                */

#define FAST_SQUARE_ROOT_PREFACE                                               \
                                                                               \
    /* table of square roots of small powers of 2 */                           \
    static const float square_root_pow_2[] =                                   \
    {                                                                          \
	1.0,           /* 1^(1/2) */                                           \
	1.414213562    /* 2^(1/2) */                                           \
    };                                                                         \
                                                                               \
    /* 2nd degree poly:                            */                          \
    /* max error = 5.4e-4                          */                          \
    /* y := x -> 0.3151417738 +                    */                          \
    /*  (0.8856989002 - 0.2013800934 x) x          */                          \
    static const float c0 = 0.3151417738;                                      \
    static const float c1 = 0.8856989002;                                      \
    static const float c2 = -0.2013800934;                                     \
                                                                               \
                                                                               \
    /* "bits" representation of argument x */                                  \
    uint32_t* xbits = (uint32_t*) &x;                                          \
                                                                               \
    /* factor of 2^(2k) */                                                     \
    float two_to_2k = 1.0;                                                     \
    /* bits representation */                                                  \
    int* two_to_2k_bits = (int *) &two_to_2k;                                  \
                                                                               \
    /* exponent of IEEE float */                                               \
    int expo;                                                                  \
    /* remainder of "expo" after dividing out factor of 2^(4k) */              \
    int expo_rem;                                                              \
                                                                               \
    /* result */                                                               \
    float ans;                                                                 \

    FAST_SQUARE_ROOT_PREFACE
    
#define FAST_SQUARE_ROOT_BODY                                                  \
                                                                               \
    /* get the IEEE float exponent and debias */                               \
    /* (assuming non-negative sign bit)       */                               \
    /* NOTE: we are debiasing to a reference point of 2^(-1) */                \
    expo = (int) (*xbits >> 23) - 126;                                         \
    /* get the remainder after division of exponent by 2 */                    \
    expo_rem = expo & 0x01;                                                    \
    /* do the division by 2 */                                                 \
    expo >>= 1;                                                                \
    /* rebias and shift back up to make an IEEE float */                       \
    *two_to_2k_bits = (expo + 127) << 23;                                      \
                                                                               \
    /* force the exponent of x to -1   */                                      \
    /* (range reduction to [1/2, 1):   */                                      \
                                                                               \
    /* mask out any exponent bits      */                                      \
    *xbits &= 0x007FFFFF;                                                      \
    /* "0x3F000000" is the exponent for 2^(-1) (=126) shifted left by 23 */    \
    *xbits += 0x3F000000;                                                      \
                                                                               \
    /* do the polynomial approximation */                                      \
    ans = c0 + (c1 + c2 * x) * x;                                              \
                                                                               \
    /* include the other factors */                                            \
    ans *= square_root_pow_2 [expo_rem];                                       \
    ans *= two_to_2k;                                                          \

    FAST_SQUARE_ROOT_BODY

    return ans;
}





/******************************************************************************/
/* here are some explicitly inlined vector versions                           */
/******************************************************************************/


void FTutils::vector_fast_log10 (const float* x_input, float* y_output, int N)
{
    int i;
    float x;
    FAST_LOG10_PREFACE;
    
    for (i=0; i<N; ++i)
    {
	x = x_input[i];
	FAST_LOG10_BODY
	y_output[i] = ans;
    }
}

void FTutils::vector_fast_log2 (const float* x_input, float* y_output, int N)
{
    int i;
    float x;
    FAST_LOG2_PREFACE;
    
    for (i=0; i<N; ++i)
    {
	x = x_input[i];
	FAST_LOG2_BODY
	y_output[i] = ans;
    }
}



void FTutils::vector_fast_square_root (const float* x_input, float* y_output, int N)
{
    int i;
    float x;
    FAST_SQUARE_ROOT_PREFACE;
    
    for (i=0; i<N; ++i)
    {
	x = x_input[i];
	FAST_SQUARE_ROOT_BODY
	y_output[i] = ans;
    }
}

void FTutils::vector_fast_fourth_root (const float* x_input, float* y_output, int N)
{
    int i;
    float x;
    FAST_FOURTH_ROOT_PREFACE;
    
    for (i=0; i<N; ++i)
    {
	x = x_input[i];
	FAST_FOURTH_ROOT_BODY
	y_output[i] = ans;
    }
}





double FTutils::sine_wave (double time, double freq_Hz)
{
    return sin (2.0 * M_PI * freq_Hz * time);
}

double FTutils::square_wave (double time, double freq_Hz)
{
    double sq = 1.0;

    /* get fractional time in the current period */
    /* of the waveform                           */
    double norm_time = time * freq_Hz;
    double frac = norm_time - floor(norm_time);
    /* check which half of period we're in */
    if (frac < 0.5)
    {
        sq = -1.0;
    }

    return sq;
}

double FTutils::triangle_wave (double time, double freq_Hz)
{
    double tr;

    /* get fractional time in the current period */
    /* of the waveform                           */
    double norm_time = time * freq_Hz;
    double frac = norm_time - floor(norm_time);
    
    /* check which half of period we're in */
    if (frac < 0.5)
    {
        /* ascending slope        */
        /* frac in range [0, 1/2) */
        tr = -1.0 + 4.0*frac;
    }
    else
    {
        /* descending slope */
        /* frac in range [1/2, 1) */
        tr = 3.0 - 4.0*frac;
    }

    return tr;
}



LocaleGuard::LocaleGuard (const char* str)
{
	old = strdup (setlocale (LC_NUMERIC, NULL));
	if (strcmp (old, str)) {
		setlocale (LC_NUMERIC, str);
	} 
}

LocaleGuard::~LocaleGuard ()
{
	setlocale (LC_NUMERIC, old);
	free ((char*)old);
}

