#pragma once

#include "lib_begin.h"

#include <string>
#include <vector>

namespace capture {

/// different result codes used in the capture library.
enum CaptureResult {
	CR_FAILURE,                  /// failure without further information
	CR_OK,                       /// call succeeded
	CR_NOT_ATTACHED,             /// failure as device was not attached
	CR_DEVICE_BUSY,              /// failure as device was busy
	CR_PROPERTY_UNSUPPORTED,     /// attempt to set property that is not supported by device
	CR_VALUE_OUT_OF_RANGE,       /// attempt to set property value that is out of the range described in the capabilities
	CR_PIXEL_FORMAT_UNSUPPORTED, /// attempt to set pixel format that is not supported by device
	CR_IMAGE_SIZE_UNSUPPORTED,   /// attempt to set image size that is not supported by device
	CR_FRAME_RATE_UNSUPPORTED,   /// attempt to set frame rate to an unsupported value
	CR_STREAM_UNSUPPORTED        /// attempt to start stream that is not supported by device
};

/// convert capture result to string
extern CGV_API const std::string& to_string(CaptureResult cr);

/// return whether result corresponds to success and if not print out error message
extern CGV_API bool debug(CaptureResult cr, const std::string& message = "");

}

#include <cgv/config/lib_end.h>