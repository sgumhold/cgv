#include "text_editor.h"
#include "gui_driver.h"

namespace cgv {
	namespace gui {


/// construct a new text editor in a separate window of given size, title and position
text_editor_ptr create_text_editor(unsigned int w, unsigned int h, const std::string& title, int x, int y)
{
	gui_driver_ptr d = get_gui_driver();
	if (d.empty())
		return text_editor_ptr();
	return d->create_text_editor(w,h,title,x,y);
}

/// called when the editor window is closed
void text_editor_callback_handler::on_close_editor()
{
}

	/// called when text has been saved
void text_editor_callback_handler::on_update_callback()
{
}

/// called when new file has been read
void text_editor_callback_handler::after_read()
{
}

/// called when text has been saved
void text_editor_callback_handler::after_save()
{
}


/// called when nr_inserted characters have been inserted at text_pos
void text_editor_callback_handler::on_text_insertion(int /*text_pos*/, int /*nr_inserted*/)
{
}

/// called when the nr_deleted characters in deleted_text have been deleted at text position text_pos
void text_editor_callback_handler::on_text_deletion(int , int , const char* )
{
}

/// construct from callback handler
text_editor::text_editor(const std::string& name, text_editor_callback_handler* _handler) : 
	named(name), handler(_handler)
{
}
/// set a new callback handler
void text_editor::set_callback_handler(text_editor_callback_handler* _handler)
{
	handler = _handler;
}
/// return current callback handler
const text_editor_callback_handler* text_editor::get_callback_handler() const
{
	return handler;
}

/// virtual destructor
text_editor::~text_editor()
{
}

	}
}