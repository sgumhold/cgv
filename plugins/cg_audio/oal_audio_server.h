#include <AL/alc.h>

#include <cgv/base/base.h>	 
#include <cgv/base/register.h>

class OALAudioServer : 
	public cgv::base::base,
	public cgv::base::server
{
  public:
	OALAudioServer() = default;
	~OALAudioServer() = default;

	virtual void on_register() override;
	virtual bool on_exit_request() override;

  private:
	ALCdevice* oal_device{nullptr};
	ALCcontext* oal_context{nullptr};
};