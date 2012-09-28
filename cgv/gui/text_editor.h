#pragma once

#include <string>
#include <cgv/base/named.h>
#include <cgv/data/ref_ptr.h>
#include <cgv/media/font/font.h>
#include "lib_begin.h"

namespace cgv {
	namespace gui {

/** implement this interface to interact with the text editor*/
struct CGV_API text_editor_callback_handler
{
	/// called when the editor window is closed
	virtual void on_close_editor();
	/// called when new file has been read
	virtual void after_read();
	/// called when text has been saved
	virtual void after_save();
	/// called when nr_inserted characters have been inserted at text_pos
	virtual void on_text_insertion(int text_pos, int nr_inserted);
	/// called when the nr_deleted characters in deleted_text have been deleted at text position text_pos
	virtual void on_text_deletion(int text_pos, int nr_deleted, const char* deleted_text);
		/// called when text has been saved
	virtual void on_update_callback();
};

/** description of a text style*/
struct text_style
{
	/// rgb color packed into one integer, i.e. 0xff00ff corresponds to magenta
	int color;
	/// font face string with font name first, followed by +bold and or +italic
	cgv::media::font::font_face_ptr font_face;
	/// font size in pixels
	float font_size;
};

/// abstract base class for text editors
class CGV_API text_editor : public base::named
{
protected:
	/// store a callback handler
	text_editor_callback_handler* handler;
public:
	/// construct from callback handler
	text_editor(const std::string& name, text_editor_callback_handler* _handler = 0);
	/// set a new callback handler
	void set_callback_handler(text_editor_callback_handler* _handler);
	/// return current callback handler
	const text_editor_callback_handler* get_callback_handler() const;
	/// virtual destructor
	virtual ~text_editor();
	/// set a different base path used for the file open and save dialog
	virtual void set_base_path(const std::string& path) = 0;
	/// set a file filter for the file open and save dialog, default is "*"
	virtual void set_filter(const std::string& filter) = 0;
	/// read a new text file
	virtual bool read(const std::string& file_name) = 0;
	/// check if the current file is modified
	virtual bool is_modified() const = 0;
	/// save the current file 
	virtual bool save() = 0;
	/// show editor
	virtual void show() = 0;
	/// hide editor
	virtual void hide() = 0;
	/// return whether editor is visible
	virtual bool is_visible() const = 0;
	/// return pointer to text buffer
	virtual const char* get_text() const = 0;
	/// replace the complete text
	virtual void set_text(const std::string& new_text) = 0;
	/// return pointer to style buffer
	virtual const char* get_style() const = 0;
	/// set the style of a given text portion
	virtual void set_style(int text_pos, int length, const char* style) = 0;
	/// return length of text and style buffer
	virtual unsigned int get_length() const = 0;
	/// set text styles from a table, where the first style is labeled with the character A, the snd by B, etc.
	virtual void set_text_styles(const text_style* table, int nr_styles) = 0;
};

/// ref counted pointer to abst control
typedef data::ref_ptr<text_editor> text_editor_ptr;

#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API data::ref_ptr<text_editor_ptr>;
#endif

/// use the currently registered gui driver to construct a new text editor in a separate window of given size, title and position
extern CGV_API text_editor_ptr create_text_editor(unsigned int w, unsigned int h, const std::string& title, int x=-1, int y=-1);

	}
}

#include <cgv/config/lib_end.h>
