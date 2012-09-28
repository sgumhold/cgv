#pragma once

#ifdef WIN32
#pragma warning(disable:4311)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <fltk/run.h>
#include <fltk/events.h>
#include <fltk/Group.h>
#include <fltk/Window.h>
#include <fltk/ask.h>
#include <fltk/file_chooser.h>
#include <fltk/Input.h>
#include <fltk/Button.h>
#include <fltk/ReturnButton.h>
#include <fltk/TextBuffer.h>
#include <fltk/TextEditor.h>
#include <fltk/MenuBuild.h>

#include "fltk_base.h"
#include <cgv/gui/text_editor.h>

class fltk_text_editor : public CW< ::fltk::Window>, public cgv::gui::text_editor, public fltk_base
{
	bool changed;
	std::string filename;
	std::string title;
	std::string base_path;
	std::string filter;
	::fltk::TextBuffer *textbuf;
	::fltk::TextBuffer *stylebuf;
	::fltk::TextDisplay::StyleTableEntry* style_table;
	int nr_styles;


	// Editor window functions and class...

	int check_save(void);

	int loading;
	int num_windows;

	::fltk::Window          *replace_dlg;
	::fltk::Input           *replace_find;
    ::fltk::Input           *replace_with;
    ::fltk::Button          *replace_all;
    ::fltk::ReturnButton    *replace_next;
    ::fltk::Button          *replace_cancel;

    ::fltk::TextEditor     *editor;
    char               search[256];

	static void changed_cb(int, int nInserted, int nDeleted,int, const char*, void* v);

	static void save_cb(::fltk::Widget*, void*);
	static void saveas_cb(::fltk::Widget*, void*);
	static void view_cb(::fltk::Widget*, void* v);

	static void replall_cb(::fltk::Widget*, void*);
	static void replace2_cb(::fltk::Widget*, void*);
	static void replcan_cb(::fltk::Widget*, void*);
	static void copy_cb(::fltk::Widget*, void* v);
	static void cut_cb(::fltk::Widget*, void* v);
	static void delete_cb(::fltk::Widget*, void* v);
	static void find_cb(::fltk::Widget*, void* v);
	static void find2_cb(::fltk::Widget*, void* v);
	static void new_cb(::fltk::Widget*, void* v);
	static void open_cb(::fltk::Widget*, void* v);
	static void update_cb(::fltk::Widget*, void* v);
	static void insert_cb(::fltk::Widget*, void* v);
	static void paste_cb(::fltk::Widget*, void* v);
	static void close_cb(::fltk::Widget*, void* v);
	static void quit_cb(::fltk::Widget*, void* v);
	static void replace_cb(::fltk::Widget*, void* v);
	static void on_close_callback(::fltk::Widget* w);
	void build_menus(::fltk::MenuBar * menu);
public:
	fltk_text_editor(const std::string& name, int w, int h, const char* t);
   ~fltk_text_editor();
	/// overload to return the type name of this object
	std::string get_type_name() const;
	/** return a semicolon separated list of property declarations of the form 
	    "name:type", by default an empty list is returned. The types should by
		 consistent with the names returned by cgv::type::info::type_name::get_name. */
	std::string get_property_declarations();
	/// abstract interface for the setter of a dynamic property, by default it simply returns false
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter of a dynamic property, by default it simply returns false
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);

	void load_file(const char *newfile, int ipos);
	void save_file(const char *newfile);
	void set_title(::fltk::Window* w);
	void build_menus(::fltk::MenuBar * menu, ::fltk::Widget *w);

	/// set a different base path used for the file open dialog
	void set_base_path(const std::string& path);
	/// set a file filter for the file open and save dialog, default is "*"
	void set_filter(const std::string& filter);
	/// read a new text file
	bool read(const std::string& file_name);
	/// check if the current file is modified
	bool is_modified() const;
	/// save the current file 
	bool save();
	/// show editor
	void show();
	/// hide editor
	void hide();
	/// return whether editor is visible
	bool is_visible() const;
	/// return pointer to text buffer
	const char* get_text() const;
	/// replace the complete text
	void set_text(const std::string& new_text);
	/// return pointer to style buffer
	const char* get_style() const;
	/// set the style of a given text portion
	void set_style(int text_pos, int length, const char* style);
	/// return text length
	unsigned int get_length() const;
	/// set text styles from a table, where the first style is labeled with the character A, the snd by B, etc.
	void set_text_styles(const cgv::gui::text_style* table, int nr_styles);
};

