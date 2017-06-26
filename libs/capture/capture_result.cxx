#include "capture_result.h"
#include <iostream>

namespace capture {


/// convert capture result to string
const std::string& to_string(CaptureResult cr)
{
	static const std::string result_name[] = {
		"FAILURE",                  /// failure without further information
		"OK",                       /// call succeeded
		"NOT_ATTACHED",             /// failure as device was not attached
		"DEVICE_BUSY",              /// failure as device was busy
		"PROPERTY_UNSUPPORTED",     /// attempt to set property that is not supported by device
		"VALUE_OUT_OF_RANGE",       /// attempt to set property value that is out of the range described in the capabilities
		"PIXEL_FORMAT_UNSUPPORTED", /// attempt to set pixel format that is not supported by device
		"IMAGE_SIZE_UNSUPPORTED",   /// attempt to set image size that is not supported by device
		"FRAME_RATE_UNSUPPORTED",   /// attempt to set frame rate to an unsupported value
		"STREAM_UNSUPPORTED"        /// attempt to start stream that is not supported by device
	};
	return result_name[cr];
}

/// return whether result corresponds to success and if not print out error message
bool debug(CaptureResult cr, const std::string& message)
{
	if (cr == CR_OK)
		return true;
	std::cerr << "error " << to_string(cr) << ":" << message << std::endl;
	return false;
}

}
