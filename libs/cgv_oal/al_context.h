#include <list>
#include <map>
#include <string>
#include <optional>

#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>

#include "lib_begin.h"

struct ALCdevice;
struct ALCcontext;
using ALuint = unsigned int;

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
	void load_sample(std::string symbolic_name, const char* data, size_t data_length);
	void load_samples(std::string folder, bool recursive = false);

	std::optional<ALuint> get_buffer_id(std::string sound_name) const;

	static std::list<std::string> enumerate_devices();
	std::string get_error_string();
	bool is_no_error();

	ALCdevice* get_native_device();
	ALCcontext* get_native_context();
  private:
	ALCdevice* oal_device{nullptr};
	ALCcontext* oal_context{nullptr};

	//! Holds all uploaded samples
	std::map<std::string, ALuint> sample_buffers;
};

class CGV_API OALListener final
{
  public:
	OALListener() = delete;
	~OALListener() = delete;

	static cgv::math::fvec<float, 3> get_position();
	static cgv::math::fvec<float, 3> get_velocity();
	static cgv::math::fmat<float, 3, 2> get_orientation();

	static void set_position(cgv::math::fvec<float, 3> pos);
	static void set_velocity(cgv::math::fvec<float, 3> vel);
	static void set_orientation(cgv::math::fmat<float, 3, 2> at_vec_then_up_vec);
};

class CGV_API OALSource
{
  public:
	OALSource() = default;
	OALSource(const OALSource&) = delete;
	OALSource(OALSource&&) = default;
	OALSource& operator=(const OALSource&) = delete;
	OALSource& operator=(OALSource&&) = default;
	~OALSource();

	bool init(OALContext& ctx, std::string sound_name);

	void set_position(cgv::math::fvec<float, 3> pos);
	void set_velocity(cgv::math::fvec<float, 3> vel);
	void set_pitch(float pitch);
	void set_gain(float gain);
	void set_looping(bool should_loop);

	cgv::math::fvec<float, 3> get_position() const;
	cgv::math::fvec<float, 3> get_velocity() const;
	float get_pitch() const;
	float get_gain() const;
	bool is_looping() const;
	bool is_playing() const;

	void play();
	void pause();
	void play_pause(bool);
	void stop();
	void rewind();

  private:
	const OALContext* ctx{nullptr};
	ALuint src_id{0};
};

} // namespace audio
} // namespace cgv

#include <cgv/config/lib_end.h>