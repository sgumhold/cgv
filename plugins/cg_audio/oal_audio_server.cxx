#include "oal_audio_server.h"

#include <array>
#include <algorithm>
#include <limits>
#include <numeric>

#include <AL/al.h>

void OALAudioServer::on_register()
{
	//auto devs = c.enumerate_devices();
	//c.load_samples("C:/Windows/Media");

	////// nullptr returns the default device
	////oal_device = alcOpenDevice(nullptr);
	////if (nullptr != oal_device) {
	////	oal_context = alcCreateContext(oal_device, nullptr);
	////	alcMakeContextCurrent(oal_context);
	////}

	////ALuint buffer;
	////alGenBuffers(1, &buffer);

	////auto error = alcGetError(oal_device);
	////assert(AL_NO_ERROR == error);

	////constexpr int sampling_rate = 48000;
	////constexpr float sine_freq = 1000.;
	////constexpr float sine_samples = sampling_rate / sine_freq;
	////// PCM samples need to have the sine zero crossings at offset 127
	////constexpr ALCubyte half_point = std::numeric_limits<ALCubyte>::max() / static_cast<ALCubyte>(2);

	////std::array<ALCubyte, sampling_rate> sine;
	////std::generate(std::begin(sine), std::end(sine), [&, i = 0]() mutable -> ALCubyte {
	////	const auto sin = std::sin(2 * 3.14159 * ((i++) / sine_samples));
	////	return half_point * (1. + sin);
	////});

	////alBufferData(buffer, AL_FORMAT_MONO8, sine.data(), sine.size() * sizeof(ALCubyte), static_cast<ALsizei>(sampling_rate));
	////error = alcGetError(oal_device);
	////assert(AL_NO_ERROR == error);

	//ALuint source;
	//alGenSources(1, &source);
	//auto error = alcGetError(c.get_native_device());
	//assert(AL_NO_ERROR == error);

	//assert(c.get_buffer_id("tada").has_value());

	//alSourcei(source, AL_BUFFER, c.get_buffer_id("tada").value());
	//alSource3f(source, AL_POSITION, 1., 1., 1.);
	//alSource3f(source, AL_VELOCITY, 0., 0., 0.);
	//alSourcef(source, AL_PITCH, 1.);
	//alSourcef(source, AL_GAIN, 1.);
	//alSourcei(source, AL_LOOPING, AL_TRUE);
	//error = alcGetError(c.get_native_device());
	//assert(AL_NO_ERROR == error);

	//std::array<ALfloat, 3> listenerPos = {0.0, 0.0, 0.0};
	//std::array<ALfloat, 3> listenerVel = {0.0, 0.0, 0.0};
	//std::array<ALfloat, 6> listenerOri = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};

	//alListenerfv(AL_POSITION, listenerPos.data());
	//alListenerfv(AL_VELOCITY, listenerVel.data());
	//alListenerfv(AL_ORIENTATION, listenerOri.data());
	//error = alcGetError(c.get_native_device());
	//assert(AL_NO_ERROR == error);

	//alSourcePlay(source);
	//error = alcGetError(c.get_native_device());
	//assert(AL_NO_ERROR == error);
}

bool OALAudioServer::on_exit_request()
{
	/*auto* const context = alcGetCurrentContext();
	auto* const device = alcGetContextsDevice(context);
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	alcCloseDevice(device);*/
	return true;
}

cgv::base::object_registration<OALAudioServer> oal_audio_server_registration("");