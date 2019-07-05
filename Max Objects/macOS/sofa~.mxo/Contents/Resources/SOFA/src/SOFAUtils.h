/*
Copyright (c) 2013--2017, UMR STMS 9912 - Ircam-Centre Pompidou / CNRS / UPMC
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**

Spatial acoustic data file format - AES69-2015 - Standard for File Exchange - Spatial Acoustic Data File Format
http://www.aes.org

SOFA (Spatially Oriented Format for Acoustics)
http://www.sofaconventions.org

*/


/************************************************************************************/
/*!
 *   @file       SOFAUtils.h
 *   @brief      Standard utility functions
 *   @author     Thibaut Carpentier, UMR STMS 9912 - Ircam-Centre Pompidou / CNRS / UPMC
 *
 *   @date       10/05/2013
 * 
 */
/************************************************************************************/
#ifndef _SOFA_UTILS_H__
#define _SOFA_UTILS_H__

#include "../src/SOFAPlatform.h"
#include <cmath>

namespace sofa
{
    /************************************************************************************/
    /*!
     *  @brief          Absolute value, single precision
     *  @param[in]      x
     *  @return         abs( x )
     *
     */
    /************************************************************************************/
    inline float FAbs(const float x) SOFA_NOEXCEPT
    {
        return fabsf(x);
    }
    
    /************************************************************************************/
    /*!
     *  @brief          Absolute value, double precision
     *  @param[in]      x
     *  @return         abs( x )
     *
     */
    /************************************************************************************/
    inline double FAbs(const double x) SOFA_NOEXCEPT
    {
        return fabs(x);
    }
    
    /************************************************************************************/
    /*!
     *  @brief          template binary minimum function
     *  @param[in]      a
     *  @param[in]      b
     *  @return         min of a and b
     *
     */
    /************************************************************************************/    
    template< typename Type >
    Type smin(const Type &a, const Type &b) SOFA_NOEXCEPT
    {
        return ( a < b ) ? a : b;
    }
    
    /************************************************************************************/
    /*!
     *  @brief          template binary maximum function
     *  @param[in]      a
     *  @param[in]      b
     *  @return         max of a and b
     *
     */
    /************************************************************************************/  
    template< typename Type >
    Type smax(const Type &a, const Type &b) SOFA_NOEXCEPT
    {
        return ( a > b ) ? a : b;
    }
    
}

#endif /* _SOFA_UTILS_H__ */ 

