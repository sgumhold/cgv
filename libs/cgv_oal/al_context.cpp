#include "al_context.h"
#include <AL/alext.h>

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <stdexcept>

#include <AL/al.h>
#include <AL/alc.h>

#include <sndfile.hh>
#include <vector>

namespace cgv {
namespace audio {

OALContext::OALContext() : OALContext(""){};

OALContext::OALContext(const std::string& device)
	: oal_device(alcOpenDevice(device.empty() ? nullptr : device.c_str()))
{
	if (oal_device) {
		oal_context = alcCreateContext(oal_device, nullptr);
		alcMakeContextCurrent(oal_context);
	}
}

OALContext::~OALContext()
{
	// TODO first, delete all sources

	for (const auto& buf : sample_buffers) {
		alDeleteBuffers(1, &buf.second);
	}

	auto* const context = alcGetCurrentContext();
	auto* const device = alcGetContextsDevice(context);
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

void OALContext::load_sample(std::string filepath)
{
	std::filesystem::path path(filepath);
	if (path.empty() || !std::filesystem::exists(path))
		return;

	SndfileHandle soundfile(path.string().c_str());
	if (soundfile.error() != SF_ERR_NO_ERROR) {
		std::cerr << soundfile.strError() << std::endl;
		return;
	}

	assert(oal_context);
	assert(oal_device);

	ALuint buffer;
	alGenBuffers(1, &buffer);
	assert(AL_NO_ERROR == alcGetError(oal_device));

	assert(soundfile.channels() == 1 || soundfile.channels() == 2);
	const size_t num_bytes{soundfile.frames() * soundfile.channels() * sizeof(std::int16_t)};
	const auto buf = std::make_unique<std::int16_t[]>(num_bytes);
	// libsndfile documentation notes that reading from float files into int buffers could require scaling
	soundfile.command(SFC_SET_SCALE_FLOAT_INT_READ, nullptr, SF_TRUE);
	soundfile.readf(buf.get(), soundfile.frames());
	if (soundfile.error() != SF_ERR_NO_ERROR) {
		std::cerr << soundfile.strError() << std::endl;
		return;
	}

	alBufferData(buffer, soundfile.channels() == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, buf.get(), (ALsizei)num_bytes,
				 (ALsizei)soundfile.samplerate());
	assert(AL_NO_ERROR == alcGetError(oal_device));

	assert(path.has_filename());
	sample_buffers.emplace(path.stem().string(), buffer);
}

void OALContext::load_sample(std::string symbolic_name, const char* data, size_t data_length)
{
	if (symbolic_name.empty())
		throw std::invalid_argument("Symbolic file name can not be empty!");
	if (std::end(sample_buffers) != sample_buffers.find(symbolic_name))
		throw std::invalid_argument("Symbolic name \"" + symbolic_name + "\" already present");
	assert(data);

	// The state describing a virtual file descriptor (as anonymous type)
	struct VIO_state
	{
		const char* const data;
		size_t data_length;
		const char* current_byte_ptr;
	} vio_state{data, data_length, data}; // and a concrete variable with list initialization performed

	SF_VIRTUAL_IO sfvio;
	sfvio.get_filelen = [](void* user_data) -> sf_count_t {
		return static_cast<const VIO_state*>(user_data)->data_length;
	};
	sfvio.seek = [](sf_count_t offset, int whence, void* user_data) -> sf_count_t {
		auto* state = static_cast<VIO_state*>(user_data);
		switch (whence) {
		case SEEK_CUR:
			state->current_byte_ptr += offset;
			return state->current_byte_ptr - state->data;
		case SEEK_SET:
			state->current_byte_ptr = state->data + offset;
			return offset;
		case SEEK_END:
			state->current_byte_ptr = (state->data + state->data_length - 1) - offset;
			return state->current_byte_ptr - state->data;
		default:
			return 0;
		}
	};
	sfvio.read = [](void* ptr, sf_count_t count, void* user_data) -> sf_count_t {
		auto* state = static_cast<VIO_state*>(user_data);
		memcpy(ptr, state->current_byte_ptr, count);
		state->current_byte_ptr += count;
		return count;
	};
	sfvio.write = [](const void* ptr, sf_count_t count, void* user_data) -> sf_count_t { return 0; };
	sfvio.tell = [](void* user_data) -> sf_count_t {
		auto* state = static_cast<const VIO_state*>(user_data);
		return state->current_byte_ptr - state->data;
	};

	SndfileHandle soundfile(sfvio, &vio_state);
	if (soundfile.error() != SF_ERR_NO_ERROR) {
		std::cerr << soundfile.strError() << std::endl;
		return;
	}

	assert(oal_context);
	assert(oal_device);

	ALuint buffer;
	alGenBuffers(1, &buffer);
	assert(AL_NO_ERROR == alcGetError(oal_device));

	assert(soundfile.channels() == 1 || soundfile.channels() == 2);
	const size_t num_bytes{soundfile.frames() * soundfile.channels() * sizeof(std::int16_t)};
	const auto buf = std::make_unique<std::int16_t[]>(num_bytes);
	const std::vector<std::int16_t> data_buffer(num_bytes, 0);
	// libsndfile documentation notes that reading from float files into int buffers could require scaling
	soundfile.command(SFC_SET_SCALE_FLOAT_INT_READ, nullptr, SF_TRUE);
	soundfile.readf(buf.get(), soundfile.frames());
	if (soundfile.error() != SF_ERR_NO_ERROR) {
		std::cerr << soundfile.strError() << std::endl;
		return;
	}

	alBufferData(buffer, soundfile.channels() == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, buf.get(), (ALsizei)num_bytes,
				 (ALsizei)soundfile.samplerate());
	assert(AL_NO_ERROR == alcGetError(oal_device));

	sample_buffers.emplace(symbolic_name, buffer);
}

void OALContext::load_samples(std::string folder, bool recursive)
{
	std::filesystem::path f(folder);
	if (!std::filesystem::is_directory(f)) {
		std::cerr << __FUNCTION__ << ": no valid folder '" << folder << "'";
		return;
	}

	// TODO code duplication should be removed via static tag dispatching
	if (recursive) {
		for (auto const& entry : std::filesystem::recursive_directory_iterator(f)) {
			if (!entry.is_regular_file())
				continue;
			load_sample(static_cast<std::filesystem::path>(entry).string());
		}
	}
	else {
		for (auto const& entry : std::filesystem::directory_iterator(f)) {
			if (!entry.is_regular_file())
				continue;
			load_sample(static_cast<std::filesystem::path>(entry).string());
		}
	}
}

ALuint OALContext::get_buffer_id(std::string sound_name) const
{
	auto buffer_id = sample_buffers.find(sound_name);
	return buffer_id != sample_buffers.end() ? buffer_id->second : AL_NONE;
}

std::list<std::string> OALContext::enumerate_devices()
{
	assert(alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT"));
	const ALCchar* dev_strings = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
	assert(dev_strings);

	std::list<std::string> device_names;
	// alcGetString returns cstring array terminated by two successive null bytes
	// (the first terminating the last cstring, the second terminating the array)
	while (strlen(dev_strings) != 0) {
		device_names.emplace_back(dev_strings);
		// +1 for the null byte
		dev_strings += device_names.back().length() + 1;
	}
	return device_names;
}

std::string OALContext::get_error_string() { return {alGetString(alGetError())}; }

bool OALContext::is_no_error() { return AL_NO_ERROR == alGetError(); }

void OALContext::set_HRTF(bool active)
{
	auto* oal_device = get_native_device();
	//assert(alcIsExtensionPresent(oal_device, "SOFT_HRTF"));

	auto reset_function = (LPALCRESETDEVICESOFT)alcGetProcAddress(oal_device, "alcResetDeviceSOFT");

	std::array<ALCint, 2> attribs = {ALC_OUTPUT_MODE_SOFT, active ? ALC_STEREO_HRTF_SOFT : ALC_STEREO_BASIC_SOFT};
	reset_function(oal_device, attribs.data());
}

ALCdevice* OALContext::get_native_device() { return oal_device; }

ALCcontext* OALContext::get_native_context() { return oal_context; }

OALSource::~OALSource() { alDeleteSources(1, &src_id); }

bool OALSource::init(OALContext& ctx, std::string sound_name)
{
	this->ctx = &ctx;

	alcMakeContextCurrent(ctx.get_native_context());
	alGenSources(1, &src_id);

	alSourcei(src_id, AL_BUFFER, ctx.get_buffer_id(sound_name));

	return ctx.is_no_error();
}

void OALSource::set_position(cgv::math::fvec<float, 3> pos) { alSourcefv(src_id, AL_POSITION, pos); }

void OALSource::set_velocity(cgv::math::fvec<float, 3> vel) { alSourcefv(src_id, AL_VELOCITY, vel); }

void OALSource::set_pitch(float pitch) { alSourcef(src_id, AL_PITCH, pitch); }

void OALSource::set_gain(float gain) { alSourcef(src_id, AL_GAIN, gain); }

void OALSource::set_looping(bool should_loop)
{
	alSourcei(src_id, AL_LOOPING, should_loop ? AL_TRUE : AL_FALSE);
}

cgv::math::fvec<float, 3> OALSource::get_position() const
{
	cgv::math::fvec<float, 3> vec;
	alGetSourcefv(src_id, AL_POSITION, vec);
	return vec;
}

cgv::math::fvec<float, 3> OALSource::get_velocity() const
{
	cgv::math::fvec<float, 3> vec;
	alGetSourcefv(src_id, AL_VELOCITY, vec);
	return vec;
}

float OALSource::get_pitch() const
{
	float pitch;
	alGetSourcef(src_id, AL_PITCH, &pitch);
	return pitch;
}

float OALSource::get_gain() const
{
	float gain;
	alGetSourcef(src_id, AL_GAIN, &gain);
	return gain;
}

bool OALSource::is_looping() const
{
	ALint looping;
	alGetSourcei(src_id, AL_LOOPING, &looping);
	return looping == AL_TRUE;
}

bool OALSource::is_playing() const
{
	ALint playing;
	alGetSourcei(src_id, AL_PLAYING, &playing);
	return playing == AL_TRUE;
}

void OALSource::play() { alSourcePlay(src_id); }

void OALSource::pause() { alSourcePause(src_id); }

void OALSource::play_pause(bool should_play)
{
	if (should_play) {
		this->play();
	}
	else {
		this->pause();
	}
}

void OALSource::stop() { alSourceStop(src_id); }

void OALSource::rewind() { alSourceRewind(src_id); }

cgv::math::fvec<float, 3> OALListener::get_position()
{
	cgv::math::fvec<float, 3> pos;
	alGetListenerfv(AL_POSITION, pos);
	return pos;
}

cgv::math::fvec<float, 3> OALListener::get_velocity()
{
	cgv::math::fvec<float, 3> vel;
	alGetListenerfv(AL_VELOCITY, vel);
	return vel;
}

cgv::math::fmat<float, 3, 2> OALListener::get_orientation()
{
	cgv::math::fmat<float, 3, 2> orientation;
	alGetListenerfv(AL_ORIENTATION, orientation);
	return orientation;
}

void OALListener::set_position(cgv::math::fvec<float, 3> pos) { alListenerfv(AL_POSITION, pos); }

void OALListener::set_velocity(cgv::math::fvec<float, 3> vel) { alListenerfv(AL_VELOCITY, vel); }

void OALListener::set_orientation(cgv::math::fvec<float, 3> at, cgv::math::fvec<float, 3> up)
{
	const std::array<float, 6> orientation = {at[0], at[1], at[2], up[0], up[1], up[2]};
	alListenerfv(AL_ORIENTATION, orientation.data());
}

} // namespace cgv
} // namespace audio
