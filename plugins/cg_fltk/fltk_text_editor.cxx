#include "fltk_text_editor.h"
#include <fltk/Font.h>
#include "fltk_font_server.h"
#include <cgv/type/variant.h>

using namespace cgv::type;

/// overload to return the type name of this object
std::string fltk_text_editor::get_type_name() const
{
	return "text_editor";
}

/** return a semicolon separated list of property declarations of the form 
    "name:type", by default an empty list is returned. The types should by
	 consistent with the names returned by cgv::type::info::type_name::get_name. */
std::string fltk_text_editor::get_property_declarations()
{
	return fltk_base::get_property_declarations()+";base_path:string";
}

/// abstract interface for the setter of a dynamic property, by default it simply returns false
bool fltk_text_editor::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (property == "base_path") {
		base_path = variant<std::string>::get(value_type,value_ptr);
		return true;
	}
	return fltk_base::set_void(this, this, property, value_type, value_ptr);
}

/// abstract interface for the getter of a dynamic property, by default it simply returns false
bool fltk_text_editor::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	if (property == "base_path") {
		set_variant(base_path,value_type,value_ptr);
		return true;
	}
	return fltk_base::get_void(this, this, property, value_type, value_ptr);
}
			
/// set a different base path used for the file open dialog
void fltk_text_editor::set_base_path(const std::string& path)
{
	base_path = path;
}
/// set a file filter for the file open and save dialog, default is "*"
void fltk_text_editor::set_filter(const std::string& _filter)
{
	filter = _filter;
}
/// read a new text file
bool fltk_text_editor::read(const std::string& file_name)
{
	load_file(file_name.c_str(), -1);
	return true;
}
/// check if the current file is modified
bool fltk_text_editor::is_modified() const
{
  return changed != 0;
}
/// save the current file 
bool fltk_text_editor::save()
{
	if (!is_modified())
		return false;
    save_cb(0,this); // Save the file...
	 return true;
}
/// show editor
void fltk_text_editor::show()
{
	fltk::Window::show();
}
/// hide editor
void fltk_text_editor::hide()
{
	fltk::Window::hide();
}
/// return whether editor is visible
bool fltk_text_editor::is_visible() const
{
	return visible();
}

struct TextBufferAccess : public fltk::TextBuffer
{
	void call_modify_callbacks(int pos, int nDeleted, int nInserted,
								 int nRestyled, const char* deletedText)
	{
		fltk::TextBuffer::call_modify_callbacks(pos, nDeleted, nInserted, nRestyled, deletedText);
	}
	const char* get_text()
	{
		if (gapstart_ < gapend_) {
			memmove(&buf_[gapstart_], &buf_[gapend_], length_-gapstart_);
			gapstart_ = gapend_ = length_;
		}
		buf_[length_] = 0; // add null terminator, assumme length < buffer size!
		return buf_;
  }
};

void fltk_text_editor::changed_cb(int pos, int nInserted, int nDeleted,int, const char* del_text, void* v) 
{
	fltk_text_editor *e = (fltk_text_editor*) v;
	if ((nInserted || nDeleted) && !e->loading) 
		e->changed = true;
	e->set_title(e);
	if (e->loading)
		e->editor->show_insert_position();

	char  *style;

	// If this is just a selection change, just unselect the style buffer...
	if (nInserted == 0 && nDeleted == 0) {
		e->stylebuf->unselect();
		return;
	}

	// Track changes in the text buffer...
	if (nInserted > 0) {
		// Insert characters into the style buffer...
		style = new char[nInserted + 1];
		memset(style, 'A', nInserted);
		style[nInserted] = '\0';
		e->stylebuf->replace(pos, pos + nDeleted, style);
		delete[] style;
	} 
	else {
		// Just delete characters in the style buffer...
		e->stylebuf->remove(pos, pos + nDeleted);
	}
	if ((nInserted || nDeleted) && e->handler) {
		if (nInserted > 0)
			e->handler->on_text_insertion(pos, nInserted);
		else if (nDeleted > 0)
			e->handler->on_text_deletion(pos,nDeleted,del_text);
	}
}

int fltk_text_editor::check_save(void) {
  if (!changed) return 1;

  int r = fltk::choice("The current file has not been saved.\n"
                    "Would you like to save it now?",
                    "Cancel", "Save", "Don't Save");

  if (r == 1) {
    save_cb(0,this); // Save the file...
    return !changed;
  }

  return (r == 2) ? 1 : 0;
}

void fltk_text_editor::load_file(const char *newfile, int ipos) 
{
  loading = 1;
  int insert = (ipos != -1);
  changed = insert != 0;
  if (!insert)
	  filename.clear();
  int r;
  if (!insert) 
	  r = textbuf->loadfile(newfile);
  else 
	  r = textbuf->insertfile(newfile, ipos);
  if (r) {
    if (fltk::ask("File '%s' does not exist. Do you want to create one?", newfile))
      filename = newfile;
    else
      filename.clear();
  } // if
  else
    if (!insert) 
		filename = newfile;
  loading = 0;
  textbuf->call_modify_callbacks();
  handler->after_read();
}

void fltk_text_editor::save_file(const char *newfile) {
  if (textbuf->savefile(newfile))
    fltk::alert("Error writing to file \'%s\':\n%s.", newfile, strerror(errno));
  else
    filename = newfile;
  changed = 0;
  textbuf->call_modify_callbacks();
  handler->after_save();
}

void fltk_text_editor::copy_cb(fltk::Widget*, void* v) {
  fltk_text_editor* e = (fltk_text_editor*)v;
  fltk::TextEditor::kf_copy(0, e->editor);
}

void fltk_text_editor::cut_cb(fltk::Widget*, void* v) {
  fltk_text_editor* e = (fltk_text_editor*)v;
  fltk::TextEditor::kf_cut(0, e->editor);
}

void fltk_text_editor::delete_cb(fltk::Widget*, void* v) {
  fltk_text_editor* e = (fltk_text_editor*)v;
  e->textbuf->remove_selection();
}

void fltk_text_editor::find_cb(fltk::Widget* w, void* v) {
  fltk_text_editor* e = (fltk_text_editor*)v;
  const char *val;

  val = fltk::input("Search String:", e->search);
  if (val != NULL) {
    // User entered a string - go find it!
    strcpy(e->search, val);
    find2_cb(w, v);
  }
}

void fltk_text_editor::find2_cb(fltk::Widget* w, void* v) {
  fltk_text_editor* e = (fltk_text_editor*)v;
  if (e->search[0] == '\0') {
    // Search string is blank; get a new one...
    find_cb(w, v);
    return;
  }

  int pos = e->editor->insert_position();
  int found = e->textbuf->search_forward(pos, e->search, &pos);
  if (found) {
    // Found a match; select and update the position...
    e->textbuf->select(pos, pos+(int)strlen(e->search));
    e->editor->insert_position(pos+(int)strlen(e->search));
    e->editor->show_insert_position();
  }
  else fltk::alert("No occurrences of \'%s\' found!", e->search);
}

void fltk_text_editor::set_title(fltk::Window* w) 
{
	if (filename.empty()) 
		title = "Untitled.txt";
	else {
		size_t spos = filename.find_last_of('/');
		size_t tpos = filename.find_last_of('\\');
		if (tpos != std::string::npos && tpos > spos)
			spos = tpos;
		if (spos != std::string::npos)
			title = filename.substr(spos+1);
		else
			title = filename;
	}
	if (changed) 
		title += " (modified)";

	w->label(title.c_str());
}

void fltk_text_editor::new_cb(fltk::Widget*, void* v) 
{
  fltk_text_editor* e = (fltk_text_editor*)v;
  if (!e->check_save()) return;

  e->filename.clear();
  e->textbuf->select(0, e->textbuf->length());
  e->textbuf->remove_selection();
  e->changed = 0;
  e->textbuf->call_modify_callbacks();
}

void fltk_text_editor::open_cb(fltk::Widget*, void* v) 
{
  fltk_text_editor* e = (fltk_text_editor*)v;
  if (!e->check_save()) 
	  return;

  const char *newfile = fltk::file_chooser("Open File?", e->filter.c_str(), e->filename.empty()?e->base_path.c_str():e->filename.c_str());
  if (newfile != NULL) e->load_file(newfile, -1);
}

void fltk_text_editor::update_cb(fltk::Widget*, void* v) 
{
  fltk_text_editor* e = (fltk_text_editor*)v;
  e->handler->on_update_callback();
}

void fltk_text_editor::insert_cb(fltk::Widget*, void *v) 
{
  fltk_text_editor *e = (fltk_text_editor *)v;
  const char *newfile = fltk::file_chooser("Insert File?", e->filter.c_str(), e->filename.empty()?e->base_path.c_str():e->filename.c_str());
  if (newfile != NULL) e->load_file(newfile, e->editor->insert_position());
}

void fltk_text_editor::paste_cb(fltk::Widget*, void* v) 
{
  fltk_text_editor* e = (fltk_text_editor*)v;
  fltk::TextEditor::kf_paste(0, e->editor);
}

void fltk_text_editor::close_cb(fltk::Widget*, void* v) 
{
  fltk_text_editor* e = (fltk_text_editor*)v;
  if (e->num_windows == 1 && !e->check_save()) {
    return;
  }

  e->hide();
  e->textbuf->remove_modify_callback(fltk_text_editor::changed_cb, e);
//  delete e;
//  e->num_windows--;
//  if (!num_windows) exit(0);
}

void fltk_text_editor::quit_cb(fltk::Widget*, void* v) 
{
  fltk_text_editor* e = (fltk_text_editor*)v;
  if (e->changed && !e->check_save())
    return;
  exit(0);
}

void fltk_text_editor::replace_cb(fltk::Widget*, void* v) 
{
  fltk_text_editor* e = (fltk_text_editor*)v;
  e->replace_dlg->show();
}

void fltk_text_editor::replace2_cb(fltk::Widget*, void* v) 
{
  fltk_text_editor* e = (fltk_text_editor*)v;
  const char *find = e->replace_find->text();
  const char *replace = e->replace_with->text();

  if (find[0] == '\0') {
    // Search string is blank; get a new one...
    e->replace_dlg->show();
    return;
  }

  e->replace_dlg->hide();

  int pos = e->editor->insert_position();
  int found = e->textbuf->search_forward(pos, find, &pos);

  if (found) {
    // Found a match; update the position and replace text...
    e->textbuf->select(pos, pos+(int)strlen(find));
    e->textbuf->remove_selection();
    e->textbuf->insert(pos, replace);
    e->textbuf->select(pos, pos+(int)strlen(replace));
    e->editor->insert_position(pos+(int)strlen(replace));
    e->editor->show_insert_position();
  }
  else fltk::alert("No occurrences of \'%s\' found!", find);
}

void fltk_text_editor::replall_cb(fltk::Widget*, void* v) {
  fltk_text_editor* e = (fltk_text_editor*)v;
  const char *find = e->replace_find->text();
  const char *replace = e->replace_with->text();

  find = e->replace_find->text();
  if (find[0] == '\0') {
    // Search string is blank; get a new one...
    e->replace_dlg->show();
    return;
  }

  e->replace_dlg->hide();

  e->editor->insert_position(0);
  int times = 0;

  // Loop through the whole string
  for (int found = 1; found;) {
    int pos = e->editor->insert_position();
    found = e->textbuf->search_forward(pos, find, &pos);

    if (found) {
      // Found a match; update the position and replace text...
      e->textbuf->select(pos, pos+(int)strlen(find));
      e->textbuf->remove_selection();
      e->textbuf->insert(pos, replace);
      e->editor->insert_position(pos+(int)strlen(replace));
      e->editor->show_insert_position();
      times++;
    }
  }

  if (times) fltk::message("Replaced %d occurrences.", times);
  else fltk::alert("No occurrences of \'%s\' found!", find);
}

void fltk_text_editor::replcan_cb(fltk::Widget*, void* v) {
  fltk_text_editor* e = (fltk_text_editor*)v;
  e->replace_dlg->hide();
}

void fltk_text_editor::save_cb(fltk::Widget*, void* v) {
  fltk_text_editor* e = (fltk_text_editor*)v;
  if (e->filename.empty()) {
    // No filename - get one!
    saveas_cb(0,v);
    return;
  }
  else e->save_file(e->filename.c_str());
}

void fltk_text_editor::view_cb(fltk::Widget*, void*) {
}

void fltk_text_editor::saveas_cb(fltk::Widget*, void* v) {
  fltk_text_editor* e = (fltk_text_editor*)v;
  const char *newfile = fltk::file_chooser("Save File As?", e->filter.c_str(), e->filename.empty()?e->base_path.c_str():e->filename.c_str());
  if (newfile != NULL) e->save_file(newfile);
}

void fltk_text_editor::build_menus(fltk::MenuBar * menu) {
    fltk::ItemGroup * g;
    menu->user_data(this);
    menu->begin();
      g = new fltk::ItemGroup( "&File" );
      g->begin();
    new fltk::Item( "&New File",        0, (fltk::Callback *)new_cb );
    new fltk::Item( "&Open File...",    fltk::COMMAND + 'o', (fltk::Callback *)open_cb );
    new fltk::Item( "&Insert File...",  fltk::COMMAND + 'i', (fltk::Callback *)insert_cb);
    new fltk::Divider();
    new fltk::Item( "&Save File",       fltk::COMMAND + 's', (fltk::Callback *)save_cb );
    new fltk::Item( "Save File &As...", fltk::COMMAND + fltk::SHIFT + 's', (fltk::Callback *)saveas_cb);
    new fltk::Divider();
    new fltk::Item( "New &View", fltk::ACCELERATOR + 'v', (fltk::Callback *)view_cb, 0 );
    new fltk::Item( "&Close View", fltk::COMMAND + 'w', (fltk::Callback *)close_cb);
    new fltk::Item( "&Update 3D View", fltk::COMMAND + 'u', (fltk::Callback *)update_cb);
    new fltk::Divider();
    new fltk::Item( "E&xit", fltk::COMMAND + 'q', (fltk::Callback *)quit_cb, 0 );
      g->end();
      g = new fltk::ItemGroup( "&Edit" );
      g->begin();
    new fltk::Item( "Cu&t",        fltk::COMMAND + 'x', (fltk::Callback *)cut_cb );
    new fltk::Item( "&Copy",       fltk::COMMAND + 'c', (fltk::Callback *)copy_cb );
    new fltk::Item( "&Paste",      fltk::COMMAND + 'v', (fltk::Callback *)paste_cb );
    new fltk::Item( "&Delete",     0, (fltk::Callback *)delete_cb );
      g->end();
      g = new fltk::ItemGroup( "&Search" );
      g->begin();
    new fltk::Item( "&Find...",       fltk::COMMAND + 'f', (fltk::Callback *)find_cb );
    new fltk::Item( "F&ind Again",    fltk::COMMAND + 'g', find2_cb );
    new fltk::Item( "&Replace...",    fltk::COMMAND + 'r', replace_cb );
    new fltk::Item( "Re&place Again", fltk::COMMAND + 't', replace2_cb );
      g->end();
    menu->end();
}

void style_unfinished_cb(int, void*) {}

fltk_text_editor::fltk_text_editor(const std::string& name, int w, int h, const char* t) : CW<fltk::Window>(w, h, t), text_editor(name)
{
	static fltk::TextDisplay::StyleTableEntry styles[] = {
		{ fltk::BLACK,           fltk::COURIER,        12, 0 }
	};
	filter = "*";
	replace_dlg = new fltk::Window(300, 105, "Replace");
	replace_dlg->begin();
	replace_find = new fltk::Input(80, 10, 210, 25, "Find:");
	replace_find->align(fltk::ALIGN_LEFT);

	replace_with = new fltk::Input(80, 40, 210, 25, "Replace:");
	replace_with->align(fltk::ALIGN_LEFT);

	replace_all = new fltk::Button(10, 70, 90, 25, "Replace All");
	replace_all->callback((fltk::Callback *)replall_cb, this);

	replace_next = new fltk::ReturnButton(105, 70, 120, 25, "Replace Next");
	replace_next->callback((fltk::Callback *)replace2_cb, this);

	replace_cancel = new fltk::Button(230, 70, 60, 25, "Cancel");
	replace_cancel->callback((fltk::Callback *)replcan_cb, this);
	replace_dlg->end();
	replace_dlg->set_non_modal();
	editor = 0;
	*search = (char)0;
	changed = 0;
	textbuf = new fltk::TextBuffer(0);
	stylebuf = new fltk::TextBuffer(0);
	style_table = 0;
	nr_styles = 0;
	loading = 0;
	num_windows = 0;
	begin();
		fltk::MenuBar* m = new fltk::MenuBar(0, 0, w, 21);
		build_menus(m);
		editor = new fltk::TextEditor(0, 21, w, h-21);
		editor->buffer(textbuf);
		editor->highlight_data(stylebuf, styles, 1, 'A', style_unfinished_cb, 0);
		editor->textfont(fltk::COURIER);
	end();
	resizable(editor);
	callback((fltk::Callback *)close_cb, this);

	editor->linenumber_width(60);
	editor->wrap_mode(true, 0);
	editor->cursor_style(fltk::TextDisplay::BLOCK_CURSOR);

	textbuf->add_modify_callback(changed_cb, this);
	textbuf->call_modify_callbacks();
	num_windows++;
}

fltk_text_editor::~fltk_text_editor() 
{
	if (handler) {
		handler->on_close_editor();
	}
	delete [] style_table;
	delete replace_dlg;
}

/// return pointer to text buffer
const char* fltk_text_editor::get_text() const
{
	return static_cast<TextBufferAccess*>(textbuf)->get_text();
}
/// replace the complete text
void fltk_text_editor::set_text(const std::string& new_text)
{
	textbuf->remove(0,textbuf->length());
	textbuf->insert(0,new_text.c_str());
}

/// return text length
unsigned int fltk_text_editor::get_length() const
{
	return textbuf->length();
}

/// return pointer to style buffer
const char* fltk_text_editor::get_style() const
{
	return stylebuf->text();
}

/// set the style of a given text portion
void fltk_text_editor::set_style(int text_pos, int length, const char* style)
{
	stylebuf->replace(text_pos, text_pos+length, style);
	editor->redisplay_range(text_pos, text_pos+length);
}


/// set text styles from a table, where the first style is labeled with the character A, the snd by B, etc.
void fltk_text_editor::set_text_styles(const cgv::gui::text_style* table, int _nr_styles)
{
	nr_styles = _nr_styles;
	if (style_table)
		delete [] style_table;
	style_table = new fltk::TextDisplay::StyleTableEntry[nr_styles];
	for (int i=0; i<nr_styles; ++i) {
		style_table[i].color = fltk::color((table[i].color/65536)&255,(table[i].color/256)&255,table[i].color&255);
		style_table[i].font  = table[i].font_face->
			get_const_interface<fltk_font_face>()->get_fltk_font();
		style_table[i].size  = table[i].font_size;
		style_table[i].attr  = 0;
	}
    editor->highlight_data(stylebuf, style_table, nr_styles, 'A', style_unfinished_cb, 0);
}

