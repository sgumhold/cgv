#pragma once

#include "capture_format.h"

#include "lib_begin.h"

namespace capture {

/// struct that represents one accelerometer measurement
struct accelerometer_measurement
{
	/// x component of gravity vector [in units of g, i.e. 9.81 m/s² corresponds to 1.0]
	float x;
	/// y component of gravity vector [in units of g, i.e. 9.81 m/s² corresponds to 1.0]
	float y;
	/// z component of gravity vector [in units of g, i.e. 9.81 m/s² corresponds to 1.0]
	float z;
	/// time stamp in milliseconds. Only present if supported by accelerometer
	long long time_stamp;
};

/// struct that represents a captured image
struct captured_image : public image_format
{
	/// pointer to RAW data in the user-specified format
	void* data_ptr;
	/// 
	long long time_stamp;
};

/// struct that represents a frame from an image stream
struct captured_frame : public captured_image
{
	///
	int frame_index;
	///
	int nr_dropped_frames;
};

/** interface used to provide read only access to captured frames. 
	The frame data is completely managed by the capture device such that there is no need for deallocation.
	The pointers returned by the frame provider are only valid during the call of the process_frame method of the capture_processor interface.
	Neither the capture device not the frame provide cache previous frames. If a frame queue is necessary
	this has to be added on top of this interface. */
class CGV_API frame_provider
{
	/// return a pointer to a color image. If instead a color frame is available just return a pointer to the contained image. If no color image or frame is available return null pointer.
	virtual const captured_image* get_color_image() const;
	/// return a pointer to an infrared image. If instead a infrared frame is available just return a pointer to the contained image. If no infrared image or frame is available return null pointer.
	virtual const captured_image* get_infrared_image() const;
	/// return a pointer to a depth image. If a instead depth frame is available just return a pointer to the contained image. If no depth image or frame is available return null pointer.
	virtual const captured_image* get_depth_image() const;
	/// return a pointer to a color frame. If no color frame is available return null pointer even if there is a color image available.
	virtual const captured_frame* get_color_frame() const;
	/// return a pointer to an infrared frame. If no infrared frame is available return null pointer even if there is an infrared image available.
	virtual const captured_frame* get_infrared_frame() const;
	/// return a pointer to a depth frame. If no depth frame is available return null pointer even if there is a depth image available.
	virtual const captured_frame* get_depth_frame() const;
};

/// interface for a callback handler that 
class CGV_API capture_processor
{
	//! Called by capture device as soon as frame data is available. 
	/*! Available frames can be queried from the frame_provider. 
		Pointers to the frame_provider and to the available frames are only valid during this call. 
		For later use one needs to copy the frame data to a different storage location.
		Return whether frames have been processed. */
	virtual bool process_frame(const frame_provider* fp) = 0;
};

}

#include <cgv/config/lib_end.h>