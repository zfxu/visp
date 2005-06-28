/*                                                                -*-c++-*-
#----------------------------------------------------------------------------
#  Copyright (C) 2005  IRISA-INRIA Rennes Vista Project
#  All Rights Reserved.
#
#
#    Contact:
#       Fabien Spindler
#       IRISA-INRIA Rennes
#       Campus Universitaire de Beaulieu
#       35042 Rennes Cedex
#       France
#
#    email: fspindle@irisa.fr
#    www  : http://www.irisa.fr/lagadic
#
#----------------------------------------------------------------------------
*/

/*!
  \file vp1394Grabber.cpp
  \brief member functions for firewire cameras
  \ingroup libdevice
*/
#include <visp/vpConfig.h>


#if ( defined (HAVE_LIBDC1394_CONTROL) & defined(HAVE_LIBRAW1394) )


#include <visp/vp1394Grabber.h>
#include <visp/vpFrameGrabberException.h>
#include <visp/vpImageIo.h>
#define DEBUG_LEVEL1 0

const int vp1394Grabber::DROP_FRAMES = 0; /*!< Number of dropped frames */
const int vp1394Grabber::NUM_BUFFERS = 8; /*!< Number of buffers */
const int vp1394Grabber::MAX_PORTS   = 4; /*!< Maximal number of ports */
const int vp1394Grabber::MAX_CAMERAS = 8; /*!< Maximal number of cameras */


/*!
  Constructor.

  By default the framerate is set to 25 fps.

  \sa setFramerate()
*/
vp1394Grabber::vp1394Grabber( )
{
  sprintf(device_name, "/dev/video1394/0");

  // allocations
  handles      = new raw1394handle_t      [vp1394Grabber::MAX_CAMERAS];
  cameras      = new dc1394_cameracapture [vp1394Grabber::MAX_CAMERAS];
  cam_count    = new int [vp1394Grabber::MAX_CAMERAS];
  format       = new int [vp1394Grabber::MAX_CAMERAS];
  mode         = new int [vp1394Grabber::MAX_CAMERAS];
  framerate    = new int [vp1394Grabber::MAX_CAMERAS];
  width        = new int [vp1394Grabber::MAX_CAMERAS];
  height       = new int [vp1394Grabber::MAX_CAMERAS];
  image_format = new ImageFormatEnum [vp1394Grabber::MAX_CAMERAS];

  // Camera settings
  for (int i=0; i < vp1394Grabber::MAX_CAMERAS; i ++) {
    format[i]    = FORMAT_VGA_NONCOMPRESSED;
    mode[i]      = MODE_640x480_MONO;
    framerate[i] = FRAMERATE_30;
  }
  verbose = false;

  iso_transmission_started = false;
  handle_created           = false;
  camera_found             = false;
  num_cameras              = 0;

  // Image settings
  for (int i=0; i < vp1394Grabber::MAX_CAMERAS; i ++) {
    width[i]        = 0;
    height[i]       = 0;
    image_format[i] = RGB;
  }

  init = false ;
}

/*!
  Constructor.

  \param I  Image data structure (8 bits image)

  By default the framerate is set to 25 fps.

  \sa setFramerate()
*/
vp1394Grabber::vp1394Grabber(vpImage<unsigned char> &I)
{
  sprintf(device_name, "/dev/video1394/0");

  // allocations
  handles      = new raw1394handle_t      [vp1394Grabber::MAX_CAMERAS];
  cameras      = new dc1394_cameracapture [vp1394Grabber::MAX_CAMERAS];
  cam_count    = new int [vp1394Grabber::MAX_CAMERAS];
  format       = new int [vp1394Grabber::MAX_CAMERAS];
  mode         = new int [vp1394Grabber::MAX_CAMERAS];
  framerate    = new int [vp1394Grabber::MAX_CAMERAS];
  width        = new int [vp1394Grabber::MAX_CAMERAS];
  height       = new int [vp1394Grabber::MAX_CAMERAS];
  image_format = new ImageFormatEnum [vp1394Grabber::MAX_CAMERAS];

  // Camera settings
  for (int i=0; i < vp1394Grabber::MAX_CAMERAS; i ++) {
    format[i]    = FORMAT_VGA_NONCOMPRESSED;
    mode[i]      = MODE_640x480_MONO;
    framerate[i] = FRAMERATE_30;
  }
  verbose = false;

  iso_transmission_started = false;
  handle_created           = false;
  camera_found             = false;
  num_cameras              = 0;

  // Image settings
  for (int i=0; i < vp1394Grabber::MAX_CAMERAS; i ++) {
    width[i]        = 0;
    height[i]       = 0;
    image_format[i] = RGB;
  }

  init = false ;

  open ( I ) ;
}

/*!

  Destructor.

  Stops the iso transmission and close the device.

  \sa close()

*/
vp1394Grabber::~vp1394Grabber()
{
  close();
}

/*!

  Set the capture format.

  \param format The camera image format. The current camera format is given by
  GetFormat(). The supported formats are given by GetSupportedFormats().

  \param camera A camera. The value must be comprised between 0 and the
  number of cameras found on the bus and returned by GetNumCameras().

  \warning The requested format is sent to the camera only after a call to
  close(). Depending on the format and the camera mode, image size can differ.

  \exception settingError If the required camera is not present.

  \sa setMode(), setFramerate()

*/
void
vp1394Grabber::setFormat(int format, int camera)
{
  if (camera < 0 || camera >= num_cameras) {
    close();
    throw (vpFrameGrabberException(vpFrameGrabberException::settingError,
				   "The required camera is not present") );
  }

  this->format[camera] = format;
}

/*!

  Set the camera capture mode.

  \param mode The camera capture mode. The current camera mode is given by
  getMode(). The supported modes are given by getSupportedModes().

  \param camera A camera. The value must be comprised between 0 and the
  number of cameras found on the bus and returned by GetNumCameras().

  \warning The requested format is sent to the camera only after a call to
  close(). Depending on the format and the camera mode, image size can differ.

  \exception settingError If the required camera is not present.

  \sa setFormat(), setFramerate()

*/
void
vp1394Grabber::setMode(int mode, int camera)
{
  if (camera < 0 || camera >= num_cameras) {
    close();
    throw (vpFrameGrabberException(vpFrameGrabberException::settingError,
				   "The required camera is not present") );
  }

  this->mode[camera] = mode;

}

/*!

  Set the capture framerate.

  \param framerate The camera framerate. The current framerate of the camera is
  given by GetFramerate(). The supported framerates are given by
  GetSupportedFramerates().

  \param camera A camera. The value must be comprised between 0 and the
  number of cameras found on the bus and returned by GetNumCameras().

  \exception settingError If the required camera is not present.

  \sa setFormat(), setMode()

*/
void
vp1394Grabber::setFramerate(int framerate, int camera)
{
  if (camera < 0 || camera >= num_cameras) {
    close();
    throw (vpFrameGrabberException(vpFrameGrabberException::settingError,
				   "The required camera is not present") );
  }

  this->framerate[camera] = framerate;
}

/*!

  Get the image width. It depends on the camera format SetFormat() and mode
  SetMode(). The width of the images is only available after a call to open().

  \param width The image width, zero if the required camera is not avalaible.
  \param camera A camera. The value must be comprised between 0 and the
  number of cameras found on the bus and returned by GetNumCameras().

  \exception settingError If the required camera is not present.

  \sa getHeight()

*/
void vp1394Grabber::getWidth(int &width, int camera)
{
  if (camera < 0 || camera >= num_cameras) {
    width = 0;
    close();
    throw (vpFrameGrabberException(vpFrameGrabberException::settingError,
				   "The required camera is not present") );
  }

  width = this->width[camera];
}

/*!

  Get the image height. It depends on the camera format setFormat() and mode
  setMode(). The height of the images is only available after a call to
  open().

  \param height The image width.
  \param camera A camera. The value must be comprised between 0 and the
  number of cameras found on the bus and returned by GetNumCameras().

  \exception settingError If the required camera is not present.

  \sa getWidth(), GetImageFormat(), close(), GetNumCameras()

*/
void vp1394Grabber::getHeight(int &height, int camera)
{
  if (camera < 0 || camera >= num_cameras) {
    height = 0;
    close();
    throw (vpFrameGrabberException(vpFrameGrabberException::settingError,
				   "The required camera is not present") );
  }

  height = this->height[camera];
}

/*!
  Initialize grey level image acquisition

  \param I : Image data structure (8 bits image)

*/
void
vp1394Grabber::open(vpImage<unsigned char> &I)
{

  open();

  setup();
  startIsoTransmission();

  getWidth(ncols) ;
  getHeight(nrows) ;

  ERROR_TRACE("%d %d", nrows, ncols ) ;

  I.resize(nrows, ncols) ;

  init = true ;

}

/*!
  Acquire a grey level image.

  \param I : Image data structure (8 bits image)

  \exception initializationError If the device is not openned.

  \sa getField()
*/
void
vp1394Grabber::acquire(vpImage<unsigned char> &I)
{

  if (init==false)
  {
    close();
    throw (vpFrameGrabberException(vpFrameGrabberException::initializationError,
				   "Initialization not done") );
  }

  int  *bitmap ;
  bitmap = dmaCapture();

  if ((I.getCols() != ncols)||(I.getRows() != nrows))
    I.resize(nrows,ncols) ;

  memcpy(I.bitmap, bitmap,
	 I.getRows()*I.getCols()*sizeof(unsigned char));

  dmaDoneWithBuffer();

}


/*!

  Open ohci and asign handle to it and get the camera nodes and
  describe them as we find them.

  \exception initializationError If a raw1394 handle can't be aquired.

  \sa close()
*/
void
vp1394Grabber::open()
{
  //int num_nodes;
  int num_ports = vp1394Grabber::MAX_PORTS;
  struct raw1394_portinfo ports[vp1394Grabber::MAX_PORTS];
  raw1394handle_t raw_handle;

  if (handles == NULL)
    handles      = new raw1394handle_t      [vp1394Grabber::MAX_CAMERAS];
  if (cameras == NULL)
    cameras      = new dc1394_cameracapture [vp1394Grabber::MAX_CAMERAS];
  if (cam_count == NULL)
    cam_count    = new int [vp1394Grabber::MAX_CAMERAS];
  if (format == NULL)
    format       = new int [vp1394Grabber::MAX_CAMERAS];
  if (mode == NULL)
    mode         = new int [vp1394Grabber::MAX_CAMERAS];
  if (framerate == NULL)
    framerate    = new int [vp1394Grabber::MAX_CAMERAS];
  if (width == NULL)
    width        = new int [vp1394Grabber::MAX_CAMERAS];
  if (height == NULL)
    height       = new int [vp1394Grabber::MAX_CAMERAS];
  if (image_format == NULL)
    image_format = new ImageFormatEnum [vp1394Grabber::MAX_CAMERAS];

  raw_handle = raw1394_new_handle();

  if (raw_handle==NULL) {
    close();
    ERROR_TRACE("Unable to aquire a raw1394 handle\n\n"
		"Please check \n"
		"  - if the kernel modules `ieee1394',`raw1394' and `ohci1394' are loaded \n"
		"  - if you have read/write access to /dev/raw1394\n\n");
    throw (vpFrameGrabberException(vpFrameGrabberException::initializationError,
				   "Unable to aquire a raw1394 handle") );
  }

  num_ports = raw1394_get_port_info(raw_handle, ports, num_ports);
  raw1394_destroy_handle(raw_handle);
  if (verbose)
    printf("number of ports detected: %d\n", num_ports);

  if (num_ports < 1) {
    close();
    throw (vpFrameGrabberException(vpFrameGrabberException::initializationError,
				   "no ports found") );
  }

  //num_nodes = raw1394_get_nodecount(raw_handle);
  num_cameras = 0;

  /* get dc1394 handle to each port */
  for (int p = 0; p < num_ports; p++)  {

    /* get the camera nodes and describe them as we find them */
    raw_handle = raw1394_new_handle();
    raw1394_set_port( raw_handle, p );

    camera_nodes = NULL;
    camera_nodes = dc1394_get_camera_nodes(raw_handle, &cam_count[p], 0);
    raw1394_destroy_handle(raw_handle);
    if (verbose)
      fprintf(stdout, "%d camera(s) on port %d\n", cam_count[p], p);

    for (int i = 0; i < cam_count[p]; i++, num_cameras ++) {
      handles[num_cameras] = dc1394_create_handle(p);
      if (handles[num_cameras]==NULL) {
	close();
	ERROR_TRACE("Unable to aquire a raw1394 handle\n");
	ERROR_TRACE("did you load the drivers?\n");
	throw (vpFrameGrabberException(vpFrameGrabberException::initializationError,
				       "Unable to aquire a raw1394 handle.") );
      }
      cameras[num_cameras].node = camera_nodes[i];

    }
    if (cam_count[p])
      dc1394_free_camera_nodes(camera_nodes);
  }

  if (num_cameras < 1) {
    close();
    throw (vpFrameGrabberException(vpFrameGrabberException::initializationError,
				   "no cameras found") );
  }
  ERROR_TRACE("%d cameras detected\n", num_cameras);

  camera_found = true;

  handle_created = true;
}

/*!

  Get the feature and set the dma capture.

  Set the camera to the specified format setFormat(), mode setMode() and
  framerate setFramerate(). Considering the camera format and mode, updates
  the captured image size.

  \exception otherError If unable to get feature set or setup the camera.

  \sa setFormat(), setMode(), setFramerate(), getWidth(), getHeight()

*/
void
vp1394Grabber::setup()
{
  unsigned int channel;
  unsigned int speed;

  if ( handle_created == true && camera_found == true) {

    for (int i = 0; i < num_cameras; i++) {
      if (verbose) {
	dc1394_feature_set features;

	if(dc1394_get_camera_feature_set(handles[i],
					 cameras[i].node,
					 &features) != DC1394_SUCCESS) {
	  close();

	  throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
					 "Unable to get feature set") );
	}
	else {
	  dc1394_print_feature_set(&features);
	}
      }

      // After setting the camera format and mode we update the image size
      getImageCharacteristics(format[i],
			      mode[i],
			      width[i],
			      height[i],
			      image_format[i],
			      i);

      if (dc1394_get_iso_channel_and_speed(handles[i],
					   cameras[i].node,
					   &channel,
					   &speed) != DC1394_SUCCESS) {
	close();
	throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				       "Unable to get the iso channel number") );
      }

      if (format[i] == FORMAT_SCALABLE_IMAGE_SIZE) {
	if( dc1394_dma_setup_format7_capture(handles[i],
					     cameras[i].node,
					     channel,
					     mode[i],
					     speed,
					     USE_MAX_AVAIL, /*max packet size*/
					     0, 0, /* left, top */
					     width[i],
					     height[i],
					     vp1394Grabber::NUM_BUFFERS,
					     vp1394Grabber::DROP_FRAMES,
					     device_name,
					     &cameras[i]) != DC1394_SUCCESS) {
	  close();

	  ERROR_TRACE("Unable to setup camera in format 7 mode 0-\n"
		      "check line %d of %s to"
		      "make sure that the video mode,framerate and format are "
		      "supported by your camera.\n",
		      __LINE__,__FILE__);

	  throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				       "Unable to setup camera in format 7 ") );

	}
	if (verbose) {
	  unsigned int qpp; // packet bytes
	  if (dc1394_query_format7_byte_per_packet(handles[i],
						   cameras[i].node,
						   mode[i],
						   &qpp) != DC1394_SUCCESS) {
	    close();

	    throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
					   "Unable to query format7 byte_per_packet ") );
	  }
	  //cout << "Format 7: byte per packet : " << qpp << endl;
	}

      } else {

	if (dc1394_dma_setup_capture(handles[i],
				     cameras[i].node,
				     channel,
				     format[i],
				     mode[i],
				     speed,
				     framerate[i],
				     vp1394Grabber::NUM_BUFFERS,
				     vp1394Grabber::DROP_FRAMES,
				     device_name,
				     &cameras[i]) != DC1394_SUCCESS) {
	  ERROR_TRACE("Unable to setup camera- check line %d of %s to"
		      "make sure that the video mode,framerate and format are "
		      "supported by your camera.\n",
		      __LINE__,__FILE__);
	  close();

	  throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
					 "Unable to setup camera ") );
	}
      }
    }
  }
}

/*!

  Gets the image size and coding format, depending on the camera image format
  and the camera mode.

  \param _format The camera capture format.
  \param _mode The camera capture mode.
  \param width Width of the image for the given camera capture format and mode.

  \param height Height of the image for the given camera capture format and
  mode.

  \param image_format Coding image format for the given camera capture
  format and mode.

  \param camera The camera for which width, height and image format are
  queried.

  \exception otherError If camera mode (see setMode()) and the image format
  (see setFormat()) are incompatible.

  \sa setFormat(), setMode(), getWidth(), getHeight()
*/
void
vp1394Grabber::getImageCharacteristics(int _format, int _mode,
				       int &_width, int &_height,
				       ImageFormatEnum &_image_format,
				       int camera)
{
  switch(_format)
  {
  case FORMAT_VGA_NONCOMPRESSED:
    switch(_mode)
    {
    case MODE_160x120_YUV444:
      _width = 160; _height = 120;
      _image_format = YUV444;
      break;
    case MODE_320x240_YUV422:
      _width = 320; _height = 240;
      _image_format = YUV422;
      break;
    case MODE_640x480_YUV411:
      _width = 640; _height = 480;
      _image_format = YUV411;
      break;
    case MODE_640x480_YUV422:
      _width = 640; _height = 480;
      _image_format = YUV422;
      break;
    case MODE_640x480_RGB:
      _width = 640; _height = 480;
      _image_format = RGB;
      break;
    case MODE_640x480_MONO:
      _width = 640; _height = 480;
      _image_format = MONO;
      break;
    case MODE_640x480_MONO16:
      _width = 640; _height = 480;
      _image_format = MONO;
      break;
    default:
      close();
      ERROR_TRACE("Error: camera image format and camera mode are uncompatible...\n");
      ERROR_TRACE("format: %d and mode: %d\n", _format, _mode);
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				     "Wrong mode for format 0") );
      break;
    }
    break;
  case FORMAT_SVGA_NONCOMPRESSED_1:
    switch(_mode)
    {
    case MODE_800x600_YUV422:
      _width = 800; _height = 600;
      _image_format = YUV422;
      break;
    case MODE_800x600_RGB:
      _width = 800; _height = 600;
      _image_format = RGB;
      break;
    case MODE_800x600_MONO:
      _width = 800; _height = 600;
      _image_format = MONO;
      break;
    case MODE_800x600_MONO16:
      _width = 800; _height = 600;
      _image_format = MONO16;
      break;
    case MODE_1024x768_YUV422:
      _width = 1024; _height = 768;
      _image_format = YUV422;
      break;
    case MODE_1024x768_RGB:
      _width = 1024; _height = 768;
      _image_format = RGB;
      break;
    case MODE_1024x768_MONO:
      _width = 1024; _height = 768;
      _image_format = MONO;
      break;
    case MODE_1024x768_MONO16:
      _width = 1024; _height = 768;
      _image_format = MONO16;
      break;
    default:
      close();
      ERROR_TRACE("Error: camera image format and camera mode are uncompatible...\n");
      ERROR_TRACE("format: %d and mode: %d\n", _format, _mode);
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				     "Wrong mode for format 1") );
      break;
    }
    break;
  case FORMAT_SVGA_NONCOMPRESSED_2:
    switch(_mode)
    {
    case MODE_1280x960_YUV422:
      _width = 1280; _height = 960;
      _image_format = YUV422;
      break;
    case MODE_1280x960_RGB:
      _width = 1280; _height = 960;
      _image_format = RGB;
      break;
    case MODE_1280x960_MONO:
      _width = 1280; _height = 960;
      _image_format = MONO;
      break;
   case MODE_1280x960_MONO16:
      _width = 1280; _height = 960;
      _image_format = MONO16;
      break;
    case MODE_1600x1200_YUV422:
      _width = 1600; _height = 1200;
      _image_format = YUV422;
      break;
    case MODE_1600x1200_RGB:
      _width = 1600; _height = 1200;
      _image_format = RGB;
      break;
    case MODE_1600x1200_MONO:
      _width = 1600; _height = 1200;
      _image_format = MONO;
      break;
    case MODE_1600x1200_MONO16:
      _width = 1600; _height = 1200;
      _image_format = MONO16;
     break;
    default:
      close();
      ERROR_TRACE("Error: camera image format and camera mode are uncompatible...\n");
      ERROR_TRACE("format: %d and mode: %d\n", _format, _mode);
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				     "Wrong mode for format 2") );
      break;
    }
    break;
  case FORMAT_SCALABLE_IMAGE_SIZE:
#if 1
    switch(_mode)
    {
    case MODE_FORMAT7_0:
      _width = 656; _height = 492;
      _image_format = YUV422;
     break;

    case MODE_FORMAT7_1:
      _width = 328; _height = 492;
      _image_format = MONO;
     break;
    case MODE_FORMAT7_2:
      _width = 656; _height = 244;
      _image_format = MONO;
     break;
    case MODE_FORMAT7_3:
    case MODE_FORMAT7_4:
    case MODE_FORMAT7_5:
    case MODE_FORMAT7_6:
    case MODE_FORMAT7_7:
      _width = 656; _height = 244;
      _image_format = MONO;
     break;
    }


#else
    // Did not work. Even if subsampling is activated in MODE_FORMAT7_1, image
    // max size is the size of the image without considering the subsampling
    switch(_mode)
    {
    case MODE_FORMAT7_0:
      //      _width = 656; _height = 492;
      _image_format = YUV422;
     break;

    case MODE_FORMAT7_1:
      //      _width = 328; _height = 492;
      _image_format = MONO;
     break;
    case MODE_FORMAT7_2:
      //      _width = 656; _height = 244;
      _image_format = MONO;
     break;
    }

    // In format 7 we query set the image size to the maximal image size
    if (dc1394_query_format7_max_image_size(handles[camera],
					    cameras[camera].node,
					    _mode,
					    &_width,
					    &_height) != DC1394_SUCCESS) {
      close();
      ERROR_TRACE("Unable to get maximal image size for format 7\n");
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				     "Unable to get maximal image size for format 7 ") );
    }
    cout << "max width=" << _width << " height: " << _height << endl;
#endif

    break;
  default:
    close();
    ERROR_TRACE("Error: camera image format and camera mode are uncompatible...\n");
    ERROR_TRACE("format: %d and mode: %d\n", _format, _mode);
    throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				   "Wrong format") );
    break;
  }
}


/*!

  Captures a frame from the given camera using DMA (direct memory acces). Two
  policies are available, either this fonction waits for a frame (waiting
  mode), either it returns if no frame is available (polling mode).

  After you have finished with the frame, you must return the buffer to the
  pool by calling DmaDoneWithBuffer().

  \param waiting Capture mode; true if you want to wait for an available image
  (waiting mode), false to activate the polling mode.

  \param camera A camera. The value must be comprised between 0 and the
  number of cameras found on the bus and returned by GetNumCameras().

  \return NULL if no frame is available, the address of the image buffer
  otherwise.

  \exception otherError If no frame is available.

  \sa DmaDoneWithBuffer(), GetNumCameras()
*/

int*
vp1394Grabber::dmaCapture(bool waiting, int camera)
{

  if (camera < 0 || camera >= num_cameras) {
    cout << "The required camera is not present..."
	 << endl;
    return NULL;
  }

  if ( handle_created == true && camera_found == true) {
    if (waiting) {
      if (num_cameras == 1) {
	// Only one camera on the bus
	if (dc1394_dma_single_capture(&cameras[camera]) != DC1394_SUCCESS) {
	  close();
	  throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
					 "No frame is available...") );
	  return NULL;
	}
      }
      else {
	// More than one camera on the bus.
	if (dc1394_dma_multi_capture(cameras, num_cameras) != DC1394_SUCCESS) {
	  close();
	  throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
					 "No frame is available...") );
	  return NULL;
	}
	//	cout << "-";
      }
    }
    else {
      if (num_cameras == 1) {
	// Only one camera on the bus
	if (dc1394_dma_single_capture_poll(&cameras[camera]) != DC1394_SUCCESS) {
	  close();
	  throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
					 "No frame is available...") );
	  return NULL;
	}
      }
      else {
	// More than one camera on the bus.
	if (dc1394_dma_multi_capture_poll(cameras, num_cameras)
	    != DC1394_SUCCESS) {
	  close();
	  throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
					 "No frame is available...") );
	  return NULL;
	}

      }
    }
  }

  return cameras[camera].capture_buffer;
}

/*!

  Return the buffer to the pool. This allows the driver to use the buffer
  previously handed to the user.

  \param camera A camera. The value must be comprised between 0 and the
  number of cameras found on the bus and returned by GetNumCameras().

  \exception settingError If the required camera is not present.
  \exception otherError If can't stop the dma access.

  \sa DmaCapture()

*/
void
vp1394Grabber::dmaDoneWithBuffer(int camera)
{

  if (camera < 0 || camera >= num_cameras) {
    throw (vpFrameGrabberException(vpFrameGrabberException::settingError,
				   "The required camera is not present") );
  }

  if ( handle_created == true && camera_found == true) {
    if (dc1394_dma_done_with_buffer(&cameras[camera]) != DC1394_SUCCESS) {
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				     "Can't done the dma") );
    }
  }
}



/*!

  Close the link between the camera and the acquisition program

*/
void
vp1394Grabber::close()
{
  if (iso_transmission_started  == true) {

    stopIsoTransmission();

    for (int i=0; i < num_cameras; i++)
      dc1394_dma_unlisten( handles[i], &cameras[i] );
    iso_transmission_started = false;
  }
  if (camera_found  == true) {
    for (int i=0; i < num_cameras; i++)
      dc1394_dma_release_camera( handles[i], &cameras[i]);
    camera_found = false;
  }
  if (handle_created == true) {
    for (int i=0; i < num_cameras; i++)
      dc1394_destroy_handle(handles[i]);
    handle_created = false;
  }

  if (handles != NULL)      { delete [] handles;      handles = NULL;      }
  if (cameras != NULL)      { delete [] cameras;      cameras = NULL;      }
  if (cam_count != NULL)    { delete [] cam_count;    cam_count = NULL;    }
  if (format != NULL)       { delete [] format;       format = NULL;       }
  if (mode != NULL)         { delete [] mode;         mode = NULL;         }
  if (framerate != NULL)    { delete [] framerate;    framerate = NULL;    }
  if (width != NULL)        { delete [] width;        width = NULL;        }
  if (height != NULL)       { delete [] height;       height = NULL;       }
  if (image_format != NULL) { delete [] image_format; image_format = NULL; }

}
/*!
  Start the transmission of the images for all the cameras on the bus.

  \exception otherError If Unable to start camera iso transmission.

  \sa stopIsoTransmission()

*/
void vp1394Grabber::startIsoTransmission()
{
  if ( handle_created == true && camera_found == true)  {

    for (int i = 0; i < num_cameras; i ++) {
      if (dc1394_start_iso_transmission(handles[i],
					cameras[i].node) !=DC1394_SUCCESS) {
	close();
	throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				       "Unable to start camera iso transmission") );
      }
    }
    iso_transmission_started = true;
  }
}

/*!

  Stop the Iso transmission for all the cameras on the bus.

  \return true on success, false otherwise.

  \sa StartIsoTransmission()

*/
void vp1394Grabber::stopIsoTransmission()
{

  if (iso_transmission_started == true)  {
    if (handle_created == true && camera_found == true) {
      for (int i = 0; i < num_cameras; i ++) {
	if (dc1394_stop_iso_transmission(handles[i],
					 cameras[i].node) != DC1394_SUCCESS) {
	  close();
	  throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
					 "Can't stop the camera") );
	}

      }
      iso_transmission_started = false;
    }
  }
}
#endif
