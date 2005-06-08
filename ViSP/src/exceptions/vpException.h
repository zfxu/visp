
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Copyright Projet Lagadic / IRISA-INRIA Rennes, 2005
 * www  : http://www.irisa.fr/lagadic
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * File:      vpException.h
 * Project:   ViSP2
 * Author:    Eric Marchand
 *
 * Version control
 * ===============
 *
 *  $Id: vpException.h,v 1.1.1.1 2005-06-08 07:08:09 fspindle Exp $
 *
 * Description
 * ============
 *  error handling
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* \file vpException.h
   \brief error that can be emited by the vp class and its derivates
 */

#ifndef __vpException_H
#define __vpException_H


/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */


/* Classes standards. */
using namespace std;
#include <iostream>                /* Classe ostream.    */
#include <string>                  /* Classe string.     */

/* --------------------------------------------------------------------- */
/* --- CLASS ----------------------------------------------------------- */
/* --------------------------------------------------------------------- */


/* \class vpException
   \brief error that can be emited by the vp class and its derivates
 */
class vpException //: public std::exception
{

private:

  //! Contains the error code, see the errorCodeEnum table for details.
  int code;

  //! Contains an error message (can be empty)
  string message;

private:

  //!  forbid the empty constructor (private)
  vpException();

public:

  enum generalExceptionEnum
    {
      memoryAllocationERR,
      memoryFreeERR,
      functionNotImplementedERR,
      ioERR,
      cannotUseConstructorERR,
      notImplementedERR,
      divideByZeroERR
    } ;
  // ~vpException() throw() {;}

  vpException (const int code, const char * msg);
  vpException (const int code, const string & msg);
  vpException (const int code);

  //!  send the object code
  int getCode (void);

  //! send a reference (constant) related the error message (can be empty)
  const string &getStringMessage (void);
  //! send a pointer on the array of  \e char related to the error string.
  //!Cannot be  \e NULL.
  const char *getMessage (void);

  //! print the error structure
  friend ostream & operator << (ostream & os,
				const vpException & art);

};





#endif /* #ifndef __vpException_H */


/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */
