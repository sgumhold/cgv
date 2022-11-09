#include <list>

#include <cgv/base/base.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/menu_provider.h>

class AudioInteractor : 
	public cgv::base::node,
	public cgv::gui::event_handler,
	public cgv::gui::menu_provider
{
  public:
	AudioInteractor();
	virtual void create_menu() override;
	virtual void destroy_menu() override;

  private:
	std::list<cgv::gui::button_ptr> menu_btn_ptrs;
};