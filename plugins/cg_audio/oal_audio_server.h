#include <cgv/base/base.h>	 
#include <cgv/base/register.h>

#include <cgv_oal/al_context.h>

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
	cgv::audio::OALContext c;
};