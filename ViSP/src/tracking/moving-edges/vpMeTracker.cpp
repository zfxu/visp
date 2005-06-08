/*!
  \file vpMeTracker.cpp
  \name Distance
*/

// ===================================================================
/*!
 *\class vpMeTracker
 *\brief 2D state = list of points, 3D state = feature
 *\n		 Contains abstract elements for a Distance to Feature type feature.
 *\author Andrew Comport
 *\date 4/7/03
 */
// ===================================================================


#include <visp/vpMeTracker.h>
#include <visp/vpDisplay.h>
#include <visp/vpColor.h>

#include <visp/vpTrackingException.h>
#include <visp/vpDebug.h>

#define DEBUG_LEVEL1 0
#define DEBUG_LEVEL2 0
#define DEBUG_LEVEL3 0




void
vpMeTracker::init() 
{
  if (DEBUG_LEVEL1)
    cout << "begin vpMeTracker::init() " <<  endl ;

  vpTracker::init()  ;
  p.resize(2) ;
  selectDisplay = NONE ;

  if (DEBUG_LEVEL1)
    cout << "end vpMeTracker::init() " <<  endl ;
}

vpMeTracker::vpMeTracker() 
{
  if (DEBUG_LEVEL1)
    cout << "begin vpMeTracker::vpMeTracker() " <<  endl ;

  init();
  me = NULL ;
  display_point = false ;
  nGoodElement = 0;

  if (DEBUG_LEVEL1)
    cout << "end vpMeTracker::vpMeTracker() " << endl ;
}

vpMeTracker::~vpMeTracker()
{
  if (DEBUG_LEVEL1) cout << "begin vpMeTracker::~vpMeTracker() " << endl ;

  if(!(list.empty()))
    list.kill();

  if (DEBUG_LEVEL1) cout << "end vpMeTracker::~vpMeTracker() " << endl ;
}

vpMeTracker&
vpMeTracker::operator = (vpMeTracker& p)
{
  if (DEBUG_LEVEL1) cout << "begin vpMeTracker::operator=" << endl ;

  list = p.list;
  me = p.me;
  selectDisplay = p.selectDisplay ;

  if (DEBUG_LEVEL1) cout << "end vpMeTracker::operator=" << endl ;
  return *this;
}

int
vpMeTracker::numberOfSignal()
{
  int number_signal=0;

  // Loop through all the points tracked from the contour
  list.front();
  while(!list.outside())
  {
    vpMeSite P = list.value();
    if(P.suppress == 0) number_signal++;
    list.next();
  }

  return number_signal;
}

int
vpMeTracker::totalNumberOfSignal()
{
  return list.nbElement();

}

int
vpMeTracker::outOfImage(int i, int j, int half, int rows, int cols)
{
  return (! ((i> half+2) &&
	     (i< rows -(half+2)) &&
	     (j>half+2) &&
	     (j<cols-(half+2)))
	  ) ;
}


//! Virtual function that is called by lower classes vpMeTrackerLine/Circle/Cylinder
void
vpMeTracker::initTracking(vpImage<unsigned char>& I)
{
  if (DEBUG_LEVEL1)
    cout << "begin vpMeTracker::initTracking() " << endl ;


  // Must set range to 0
  int range_tmp = me->range;
  me->range=1;

  nGoodElement=0;

  int d = 0;
  // Loop through list of sites to track
  list.front();
  while(!list.outside())
  {
    vpMeSite refp = list.value() ;//current reference pixel

    d++ ;
    // If element hasn't been suppressed
    if(refp.suppress==0)
    {
      try {
	refp.track(I,me,false);
      }
      catch(...)
      {
	// EM verifier quel signal est de sortie !!!
	ERROR_TRACE(" ") ;
	throw ;
      }
      if(refp.suppress==0) nGoodElement++;
    }


    if(DEBUG_LEVEL2)
    {
      double a,b ;
      a = refp.i_1 - refp.i ;
      b = refp.j_1 - refp.j ;
      if(refp.suppress==0)
	vpDisplay::displayArrow(I,
				refp.i,refp.j,
				refp.i+(int)(a),refp.j+(int)(b),
				vpColor::green) ;
    }

    list.modify(refp) ;
    list.next() ;
  }

  /*
  if (res != OK)
  {
    cout<< "In vpMeTracker::initTracking(): " ;
    switch (res)
    {
    case  ERR_TRACKING:
      cout << "vpMeTracker::initTracking:Track return ERR_TRACKING " << endl ;
      break ;
    case FATAL_ERROR:
      cout << "vpMeTracker::initTracking:Track return FATAL_ERROR" << endl ;
      break ;
    default:
      cout << "vpMeTracker::initTracking:Track return error " << res << endl ;
    }
    return res ;
  }
  */

  me->range=range_tmp;


  if (DEBUG_LEVEL1)
  cout << "end vpMeTracker::initTracking() " << endl ;

}

// ===================================================================
/*!
 * \note Track
 * \pre -CList<CPixel> liste : the list of pixel to track
 * \n  	-CImage& im : image t+1
 * \n	  -vpMe *me : me tracking process parameters
 * \n	  -int test_contrast : 1 if you want to test the contrast (using mu1 and mu2)
 *				also to test temporal component of the hypothesis. i.e. log liklihood ratio
 * \post modification of the liste of pixel
 * 				Return value : OK : all is ok, FATAL_ERROR : ususaly a division by 0
 * \author Andrew Comport
 * \date 2002
 */
// ===================================================================
void
vpMeTracker::track(vpImage<unsigned char>& I)
{
  if (DEBUG_LEVEL1)
    cout << "begin  vpMeTracker::Track():" << endl ;

  if (list.nbElement()==0)
  {

    ERROR_TRACE("Error Tracking: only %d "
		 "pixels when entered the function ",list.nbElement()) ;
    throw(vpTrackingException(vpTrackingException::NOT_ENOUGH_POINT_ERR,
			      "too few pixel to track")) ;

  }

  nGoodElement=0;
  //  int d =0;
  // Loop through list of sites to track
  list.front();
  while(!list.outside())
  {
    vpMeSite s = list.value() ;//current reference pixel

    //    d++ ;
    // If element hasn't been suppressed
    if(s.suppress==0)
    {

      try{
	//	ERROR_TRACE("%d",d ) ;
	//	ERROR_TRACE("range %d",me->range) ;
	 s.track(I,me,true);
      }
      catch(vpTrackingException)
      {
	ERROR_TRACE("catch exception ") ;
	s.suppress=2 ;
      }

      if(s.suppress != 2)
      {
	//	ERROR_TRACE("je passe la ") ;
	nGoodElement++;

	if(DEBUG_LEVEL2)
	{
	  double a,b ;
	  a = s.i_1 - s.i ;
	  b = s.j_1 - s.j ;
	  if(s.suppress==0)
	    vpDisplay::displayArrow(I,
				    s.i,s.j,
				    s.i+(int)(a*5),s.j+(int)(b*5),
				    vpColor::red) ;
	}

      }
      list.modify(s) ;
    }
    list.next() ;

  }

  if (DEBUG_LEVEL1)
    cout << "end  vpMeTracker::Track()" <<nGoodElement << endl ;

}


void
vpMeTracker::display(vpImage<unsigned char>& I)
{
  if (DEBUG_LEVEL1)
  {
    cout <<"begin vpMeTracker::displayList() " << endl ;
    cout<<" There are "<<list.nbElement()<< " sites in the list " << endl ;
  }

  list.front();

  while (!list.outside())
  {
    vpMeSite p = list.value() ;

    if(p.suppress == 1)
      vpDisplay::displayCross(I,p.i, p.j, 2, vpColor::white) ; // Contrast
    else if(p.suppress == 2)
      vpDisplay::displayCross(I,p.i, p.j, 2,vpColor::blue) ; // Threshold
    else if(p.suppress == 3)
      vpDisplay::displayCross(I,p.i, p.j, 3, vpColor::green) ; // M-estimator
    else if(p.suppress == 0)
      vpDisplay::displayCross(I,p.i, p.j, 2, vpColor::red) ; // OK

    list.next() ;
  }

  list.front() ;

  if (DEBUG_LEVEL1)
  {
    cout <<"end vpMeTracker::displayList() " << endl ;
  }
}


void
vpMeTracker::display(vpImage<unsigned char>& I,vpColVector &w,int &index_w)
{

  list.front();
  while(!list.outside())
  {
    vpMeSite P = list.value();

    if(P.suppress == 0)
    {
      P.weight = w[index_w];
      index_w++;
    }

    list.modify(P) ;
    list.next();

  }
  display(I);
}

#undef DEBUG_LEVEL1
#undef DEBUG_LEVEL2
#undef DEBUG_LEVEL3

