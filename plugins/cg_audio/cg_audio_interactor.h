#include <cgv/base/base.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>

class AudioInteractor : 
	public cgv::base::node,
	public cgv::gui::event_handler,
	public cgv::gui::provider
{
  public:
	AudioInteractor();
	virtual void create_gui() override;

  private:
};