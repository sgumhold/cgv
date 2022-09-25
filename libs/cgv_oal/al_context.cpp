#include "al_context.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <stdexcept>

#include <AL/al.h>
#include <AL/alc.h>

#include <sndfile.hh>

cgv::audio::OALContext::OALContext() : OALContext(""){};

cgv::audio::OALContext::OALContext(std::string device) {
	// nullptr returns the default device
	oal_device = alcOpenDevice(device.empty() ? nullptr : device.c_str());
	if (oal_device) {
		oal_context = alcCreateContext(oal_device, nullptr);
		alcMakeContextCurrent(oal_context);
	}
}

cgv::audio::OALContext::~OALContext() {
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

void cgv::audio::OALContext::load_sample(std::string filepath)
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

	alBufferData(buffer, soundfile.channels() == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, buf.get(), num_bytes,
				 soundfile.samplerate());
	assert(AL_NO_ERROR == alcGetError(oal_device));

	assert(path.has_filename());
	sample_buffers.emplace(path.stem().string(), buffer);
}

void cgv::audio::OALContext::load_sample(std::string symbolic_name, const char* data, size_t data_length) {
	if (symbolic_name.empty())
		throw std::invalid_argument(__FUNCTION__ ": Symbolic file name can not be empty!");
	if (std::end(sample_buffers) != sample_buffers.find(symbolic_name))
		throw std::invalid_argument(__FUNCTION__ ": Symbolic name \"" + symbolic_name + "\" already present");
	assert(data);

	// The state describing a virtual file descriptor (as anonymous type)
	struct VIO_state
	{
		const char* const data;
		size_t data_length;
		const char* current_byte_ptr;
	} vio_state{data, data_length, data}; // and a concrete variable with list initialization performed

	SF_VIRTUAL_IO sfvio;
	sfvio.get_filelen = [](void* user_data) -> sf_count_t { return static_cast<const VIO_state*>(user_data)->data_length; };
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
	// libsndfile documentation notes that reading from float files into int buffers could require scaling
	soundfile.command(SFC_SET_SCALE_FLOAT_INT_READ, nullptr, SF_TRUE);
	soundfile.readf(buf.get(), soundfile.frames());
	if (soundfile.error() != SF_ERR_NO_ERROR) {
		std::cerr << soundfile.strError() << std::endl;
		return;
	}

	alBufferData(buffer, soundfile.channels() == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, buf.get(), num_bytes,
				 soundfile.samplerate());
	assert(AL_NO_ERROR == alcGetError(oal_device));

	sample_buffers.emplace(symbolic_name, buffer);
}

void cgv::audio::OALContext::load_samples(std::string folder, bool recursive)
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

std::optional<ALuint> cgv::audio::OALContext::get_buffer_id(std::string sound_name) const
{
	auto buffer_id = sample_buffers.find(sound_name);
	return buffer_id != sample_buffers.end() ? std::make_optional(buffer_id->second) : std::optional<ALuint>();
}

std::list<std::string> cgv::audio::OALContext::enumerate_devices()
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

std::string cgv::audio::OALContext::get_error_string()
{
	return {alGetString(alGetError())};
}

bool cgv::audio::OALContext::is_no_error()
{
	return AL_NO_ERROR == alGetError();
}

ALCdevice* cgv::audio::OALContext::get_native_device()
{
	return oal_device;
}

ALCcontext* cgv::audio::OALContext::get_native_context()
{
	return oal_context;
}

cgv::audio::OALSource::~OALSource() {
	alDeleteSources(1, &src_id);
}

bool cgv::audio::OALSource::init(OALContext& ctx, std::string sound_name)
{
	this->ctx = &ctx;

	alcMakeContextCurrent(ctx.get_native_context());
	alGenSources(1, &src_id);

	auto buf_id = ctx.get_buffer_id(sound_name);
	alSourcei(src_id, AL_BUFFER, buf_id ? buf_id.value() : AL_NONE);

	return ctx.is_no_error();
}

void cgv::audio::OALSource::set_position(cgv::math::fvec<float, 3> pos)
{
	alSourcefv(src_id, AL_POSITION, pos);
}

void cgv::audio::OALSource::set_velocity(cgv::math::fvec<float, 3> vel)
{
	alSourcefv(src_id, AL_VELOCITY, vel);
}

void cgv::audio::OALSource::set_pitch(float pitch) {
	alSourcef(src_id, AL_PITCH, pitch);
}

void cgv::audio::OALSource::set_gain(float gain) {
	alSourcef(src_id, AL_GAIN, gain);
}

void cgv::audio::OALSource::set_looping(bool should_loop) {
	alSourcei(src_id, AL_LOOPING, should_loop ? AL_TRUE : AL_FALSE);
}

cgv::math::fvec<float, 3> cgv::audio::OALSource::get_position() const
{
	cgv::math::fvec<float, 3> vec;
	alGetSourcefv(src_id, AL_POSITION, vec);
	return vec;
}

cgv::math::fvec<float, 3> cgv::audio::OALSource::get_velocity() const
{
	cgv::math::fvec<float, 3> vec;
	alGetSourcefv(src_id, AL_VELOCITY, vec);
	return vec;
}

float cgv::audio::OALSource::get_pitch() const
{
	float pitch;
	alGetSourcef(src_id, AL_PITCH, &pitch);
	return pitch;
}

float cgv::audio::OALSource::get_gain() const
{
	float gain;
	alGetSourcef(src_id, AL_GAIN, &gain);
	return gain;
}

bool cgv::audio::OALSource::is_looping() const
{
	ALint looping;
	alGetSourcei(src_id, AL_LOOPING, &looping);
	return looping == AL_TRUE;
}

bool cgv::audio::OALSource::is_playing() const
{
	ALint playing;
	alGetSourcei(src_id, AL_PLAYING, &playing);
	return playing == AL_TRUE;
}

void cgv::audio::OALSource::play() {
	alSourcePlay(src_id);
}

void cgv::audio::OALSource::pause() {
	alSourcePause(src_id);
}

void cgv::audio::OALSource::play_pause(bool play) {
	if (play) {
		this->play();
	}
	else {
		this->pause();
	}
}

void cgv::audio::OALSource::stop() {
	alSourceStop(src_id);
}

void cgv::audio::OALSource::rewind() {
	alSourceRewind(src_id);
}

cgv::math::fvec<float, 3> cgv::audio::OALListener::get_position()
{
	cgv::math::fvec<float, 3> pos;
	alGetListenerfv(AL_POSITION, pos);
	return pos;
}

cgv::math::fvec<float, 3> cgv::audio::OALListener::get_velocity()
{
	cgv::math::fvec<float, 3> vel;
	alGetListenerfv(AL_VELOCITY, vel);
	return vel;
}

cgv::math::fmat<float, 3, 2> cgv::audio::OALListener::get_orientation()
{
	cgv::math::fmat<float, 3, 2> orientation;
	alGetListenerfv(AL_ORIENTATION, orientation);
	return orientation;
}

void cgv::audio::OALListener::set_position(cgv::math::fvec<float, 3> pos) {
	alListenerfv(AL_POSITION, pos);
}

void cgv::audio::OALListener::set_velocity(cgv::math::fvec<float, 3> vel) {
	alListenerfv(AL_VELOCITY, vel);
}

void cgv::audio::OALListener::set_orientation(cgv::math::fmat<float, 3, 2> at_vec_then_up_vec) {
	alListenerfv(AL_ORIENTATION, at_vec_then_up_vec);
}
