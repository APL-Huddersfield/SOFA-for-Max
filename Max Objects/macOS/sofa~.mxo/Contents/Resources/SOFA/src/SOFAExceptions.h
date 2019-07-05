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
 *   @file       SOFAExceptions.h
 *   @brief      Exception handling
 *   @author     Thibaut Carpentier, UMR STMS 9912 - Ircam-Centre Pompidou / CNRS / UPMC
 *
 *   @date       10/05/2013
 * 
 */
/************************************************************************************/
#ifndef _SOFA_EXCEPTIONS_H__
#define _SOFA_EXCEPTIONS_H__

#include "../src/SOFAPlatform.h"
#include <exception>

namespace sofa
{
    
    /************************************************************************************/
    /*!
     *  @class          Exception 
     *  @brief          Exception handling
     *
     */
    /************************************************************************************/
    class SOFA_API Exception : public std::exception
    {
    public:
        static void LogToCerr(const bool value);
        static bool IsLoggedToCerr();
        
    public:
        Exception(const std::string &text    = "unknown exception",
                  const std::string &file    = "",
                  const unsigned long line_  = 0,
                  const bool exitAfterException = false);
        
        virtual ~Exception() SOFA_NOEXCEPT {};
        virtual const char* what() const SOFA_NOEXCEPT SOFA_OVERRIDE;
        
        const std::string & GetFile() const;
        unsigned long GetLine() const;
        
    private:
        static std::string getFileName(const std::string & fullfilename);
        
        static bool logToCerr;
        
    private:
        const std::string filename;            ///< name of the file where the exception occured
        const std::string description;        ///< description of the exception
        const unsigned long line;            ///< line number where the exception ocurred        
    };
    
    /**
     @brief Handy macro to throw a SOFA exception
     */
    #define SOFA_THROW( message )\
    {\
        throw sofa::Exception( message, __FILE__ , __LINE__ );\
    }
    
}

#endif /* _SOFA_EXCEPTIONS_H__ */

