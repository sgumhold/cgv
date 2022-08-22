#include <list>
#include <map>
#include <string>
#include <optional>

#include <AL/al.h>
#include <AL/alc.h>

#include "lib_begin.h"

namespace cgv {
namespace audio {

class CGV_API OALContext
{
  public:
	OALContext();
	explicit OALContext(std::string device);
	OALContext(const OALContext& other) = delete;
	OALContext& operator=(const OALContext& other) = delete;
	~OALContext();

	void load_sample(std::string filepath);
	void load_samples(std::string folder, bool recursive = false);

	std::optional<ALuint> get_buffer_id(std::string sound_name) const;

	static std::list<std::string> enumerate_devices();

	ALCdevice* get_native_device() const;
	ALCcontext* get_native_context() const;
  private:
	ALCdevice* oal_device{nullptr};
	ALCcontext* oal_context{nullptr};

	//! Holds all uploaded samples
	std::map<std::string, ALuint> sample_buffers;
};

} // namespace audio
} // namespace cgv

#include <cgv/config/lib_end.h>