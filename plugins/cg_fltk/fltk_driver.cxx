#include "fltk_driver.h"

#include "fltk_button.h"
#include "fltk_viewer_window.h"
#include "fltk_generic_window.h"
#include "fltk_align_group.h"
#include "fltk_tab_group.h"
#include "fltk_tree_group.h"
#include "fltk_dockable_group.h"
#include "fltk_layout_group.h"
#include "fltk_driver_registry.h"
#include "fltk_text_editor.h"

#include <fltk/Monitor.h>
#include <fltk/events.h>
#include <fltk/run.h>

#include <cgv/gui/base_provider_generator.h>
#include <cgv/gui/menu_provider.h>
#include <cgv_gl/gl/wgl.h>

#ifdef _WIN32
#include <windows.h>
#include <fltk/../../OpenGL/GlChoice.h>
#define USE_WIN32
#else
#include <fltk/file_chooser.h>
#define USE_FLTK
#endif

#ifdef USE_WIN32
#include <Windows.h>
#include <commdlg.h>
#include <cgv/utils/convert.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/file.h>
#include <cgv/utils/dir.h>
#include <Tchar.h>
#include <Shlobj.h> // BrowseFolder

typedef std::basic_string<_TCHAR> tstring;

void prepare_ofn_struct(OPENFILENAME& ofn, _TCHAR *szFile, int file_size,
	const std::string& title, tstring& wtitle,
	const std::string& filter, tstring& wfilter,
	const std::string& path, tstring& wpath)
{
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetForegroundWindow();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = file_size;
	std::string f = filter;
	cgv::utils::replace(f, ':', '\0');
	cgv::utils::replace(f, '|', '\0');
	f += '\0';
	std::string p = path;
	cgv::utils::replace(p, '/', '\\');
#ifdef _UNICODE
	wtitle = cgv::utils::str2wstr(title);
	wfilter = cgv::utils::str2wstr(f);
	wpath = cgv::utils::str2wstr(p);
#else
	wtitle = title;
	wfilter = f;
	wpath = p;
#endif
	ofn.lpstrFilter = wfilter.c_str();
	ofn.nFilterIndex = 0;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = wtitle.c_str();
	if (!cgv::utils::dir::exists(p)) {
		unsigned i;
		for (i = 0; i<wpath.size(); ++i) {
			szFile[i] = wpath[i];
			if (i + 2 == file_size)
				break;
		}
		szFile[i] = '\0';
		ofn.lpstrInitialDir = NULL;
	}
	else {
		szFile[0] = '\0';
		ofn.lpstrInitialDir = wpath.c_str();
	}
}

// CALLBACK message procedure for the browse folder dialog
// this callback procedure sets the initial folder of the browse folder dialog
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg,
	LPARAM lParam, LPARAM lpData)
{
	TCHAR initialPath[MAX_PATH];

	switch (uMsg)
	{

	case BFFM_INITIALIZED:
		// check whether initial folder is give
		if (lpData)
		{
			// so set the initial folder
			_tcscpy(initialPath, (TCHAR*)lpData);
		}
		else
		{
			// otherwise use current folder als initial folder
			GetCurrentDirectory(sizeof(initialPath) / sizeof(TCHAR), initialPath);
		}
		// set the initial folder in the folder dialog by a message
		SendMessage(hwnd, BFFM_SETSELECTION, true, (LPARAM)initialPath);
		break;
	}

	return 0;
}

void prepare_bi_struct(BROWSEINFO& bi, _TCHAR *wszPath,
	const std::string& title, tstring& wtitle,
	const std::string& path, tstring& wpath
	)
{
	ZeroMemory(&bi, sizeof(bi));
	bi.hwndOwner = GetForegroundWindow();
	bi.ulFlags = BIF_USENEWUI;
	bi.lParam = 0;
	std::string windows_path(path);
	cgv::utils::replace(windows_path, "/", "\\");
#ifdef _UNICODE
	wtitle = cgv::utils::str2wstr(title);
	wpath = cgv::utils::str2wstr(windows_path);
#else
	wtitle = title;
	wpath = windows_path;
#endif    

	bi.pidlRoot = NULL;
	bi.lpszTitle = wtitle.c_str();
	bi.pszDisplayName = wszPath; // in this variable the choosen folder will be saved
	bi.lpfn = BrowseCallbackProc; // set the callback procedure (used for initial folder setting)
	bi.lParam = (LPARAM)wpath.c_str(); // remember the initial folder for the callback function (lpfn)    			
}



#endif

std::string directory_open_dialog(const std::string& title, const std::string& path)
{
#ifdef USE_FLTK
	const char* fn = fltk::dir_chooser(title.c_str(), path.c_str());
	if (!fn)
		return "";
	return fn;
#endif

#ifdef USE_WIN32
	_TCHAR szPath[MAX_PATH];
	tstring wtitle, wpath;
	BROWSEINFO bi;

	HRESULT hr = CoInitialize(NULL);

	if (SUCCEEDED(hr))
	{
		prepare_bi_struct(bi, szPath, title, wtitle, path, wpath);
		LPITEMIDLIST item = SHBrowseForFolder(&bi);

		if (item != NULL)
		{
			SHGetPathFromIDList(item, szPath);
			CoTaskMemFree(item);
			CoUninitialize();
#ifdef _UNICODE
			std::string result = cgv::utils::wstr2str(szPath);
#else
			std::string result = szPath;
#endif
			cgv::utils::replace(result, "\\", "/");
			return result;
		}
		else
		{
			CoUninitialize();
			return "";
		}
	}

#endif
	std::cerr << "no implementation" << std::endl;
	return "";
}

std::string directory_save_dialog(const std::string& title, const std::string& path)
{
	return directory_open_dialog(title, path);
}

std::string fltk_driver::file_open_dialog(const std::string& title, const std::string& filter, const std::string& path)
{
	if (filter.empty())
		return directory_open_dialog(title, path);
#ifdef USE_FLTK
	ensure_lock();

	fltk::Widget* f = fltk::focus();
	const char* fn = fltk::file_chooser(title.c_str(), filter.c_str(), path.empty() ? NULL : path.c_str(), 0);
	if (f != NULL)
		f->window()->show();
	if (!fn)
		return std::string();
	return std::string(fn);
#endif

#ifdef USE_WIN32
	OPENFILENAME ofn;
	_TCHAR szFile[500];
	tstring wfilter, wtitle, wpath;
	prepare_ofn_struct(ofn, szFile, 500, title, wtitle, filter, wfilter, path, wpath);
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (GetOpenFileName(&ofn) == TRUE)
#ifdef _UNICODE
		return cgv::utils::wstr2str(szFile);
#else
		return szFile;
#endif
	return "";
#endif
	std::cerr << "no implementation" << std::endl;
	return "";
}

/// ask user for a open dialog that can select multiple files
std::string fltk_driver::files_open_dialog(std::vector<std::string>& file_names, const std::string& title, const std::string& filter, const std::string& path)
{
#ifdef USE_WIN32
	OPENFILENAME ofn;
	_TCHAR szFile[10000];
	tstring wfilter, wtitle, wpath;
	prepare_ofn_struct(ofn, szFile, 10000, title, wtitle, filter, wfilter, path, wpath);
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	if (GetOpenFileName(&ofn) == TRUE) {
		unsigned i = 0;
		do {
			while (i < 10000 && szFile[i] != 0)
				++i;
			if (i >= 9999)
				break;
			++i;
			if (szFile[i] == 0)
				break;
#ifdef _UNICODE
			file_names.push_back(cgv::utils::wstr2str(szFile + i));
#else
			file_names.push_back(szFile + i);
#endif
		} while (true);
#ifdef _UNICODE
		return cgv::utils::wstr2str(szFile);
#else
		return szFile;
#endif
	}
	return "";
#endif
	std::cerr << "no implementation" << std::endl;
	return "";
}

std::string fltk_driver::file_save_dialog(const std::string& title, const std::string& filter, const std::string& path)
{
	if (filter.empty())
		return directory_save_dialog(title, path);
#ifdef USE_WIN32
	OPENFILENAME ofn;
	_TCHAR szFile[500];
	tstring wfilter, wtitle, wpath;
	prepare_ofn_struct(ofn, szFile, 500, title, wtitle, filter, wfilter, path, wpath);
	ofn.Flags = OFN_OVERWRITEPROMPT;
	if (GetSaveFileName(&ofn) == TRUE)
#ifdef _UNICODE
		return cgv::utils::wstr2str(szFile);
#else
		return szFile;
#endif
	return "";
#endif

#ifdef USE_FLTK
	return file_open_dialog(title, filter, path);
#endif

	std::cerr << "no implementation" << std::endl;
	return "";
}

//TODO: Remove?
/*#ifdef _WIN32
#undef TA_LEFT
#undef TA_TOP
#undef TA_RIGHT
#undef TA_BOTTOM
#endif
#include "fltk_driver.h"
#include "fltk_button.h"
#include "fltk_viewer_window.h"
#include "fltk_generic_window.h"
#include "fltk_align_group.h"
#include "fltk_tab_group.h"
#include "fltk_tree_group.h"
#include "fltk_dockable_group.h"
#include "fltk_layout_group.h"
#include "fltk_driver_registry.h"
#include "fltk_text_editor.h"
#include <fltk/file_chooser.h>
#include <fltk/events.h>
#include <fltk/run.h>
#include <cgv/base/named.h>
#include <cgv/render/context.h>
#include <cgv/gui/menu_provider.h>
#include <cgv/gui/base_provider_generator.h>*/

using namespace cgv::base;


void ensure_lock()
{
	static bool lock_set = false;
	if (!lock_set) {
		fltk::lock();
		lock_set = true;
	}
}

void fltk_driver::on_register()
{
	register_gui_driver(gui_driver_ptr(this));
}

/// remove a window that has been destroyed
void fltk_driver::remove_window(window_ptr w)
{
	for (unsigned i=0; i<windows.size(); ++i) {
		if (windows[i] == w) {
			windows.erase(windows.begin()+i);
			--i;
		}
	}
}

std::string fltk_driver::get_type_name() const
{
	return "fltk_driver";
}

/// fill list of monitor descriptions
bool fltk_driver::enumerate_monitors(std::vector<monitor_description>& monitor_descriptions)
{
	const fltk::Monitor* mons;
	int n = fltk::Monitor::list(&mons);
	for (int i = 0; i < n; ++i) {
		monitor_description md;
		md.x = mons[i].x();
		md.y = mons[i].y();
		md.w = mons[i].w();
		md.h = mons[i].h();
		md.dpi_x = mons[i].dpi_x();
		md.dpi_y = mons[i].dpi_y();
		monitor_descriptions.push_back(md);
	}
	return true;
}

/// create a window of the given type. Currently only the types "viewer with gui", "viewer" and "gui" are supported
window_ptr fltk_driver::create_window(int w, int h, const std::string& title, const std::string& window_type)
{
	//TODO: make platform-independent
#ifdef _WIN32
	static std::vector<int> context_creation_attrib_list;

	cgv::render::render_config_ptr rcp = cgv::render::get_render_config();
	if (rcp) {
		context_creation_attrib_list.clear();
		if (rcp->forward_compatible || rcp->debug) {
			context_creation_attrib_list.push_back(WGL_CONTEXT_FLAGS_ARB);
			context_creation_attrib_list.push_back((rcp->forward_compatible ? WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB : 0) + (rcp->debug ? WGL_CONTEXT_DEBUG_BIT_ARB : 0));
		}
		if (rcp->version_major > 0) {
			context_creation_attrib_list.push_back(WGL_CONTEXT_MAJOR_VERSION_ARB);
			context_creation_attrib_list.push_back(rcp->version_major);
		}
		if (rcp->version_minor > 0) {
			context_creation_attrib_list.push_back(WGL_CONTEXT_MINOR_VERSION_ARB);
			context_creation_attrib_list.push_back(rcp->version_minor);
		}
		context_creation_attrib_list.push_back(WGL_CONTEXT_PROFILE_MASK_ARB);
		context_creation_attrib_list.push_back(rcp->core_profile ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB : WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB);
		context_creation_attrib_list.push_back(0);
		fltk::GlChoice::ref_attrib_list() = &context_creation_attrib_list.front();
	}
#endif
	ensure_lock();
	window_ptr wp;
	if (window_type == "viewer")
		wp = window_ptr(new fltk_viewer_window(w, h, title));
	else
	if (window_type == "generic")
		wp = window_ptr(new fltk_generic_window(0, 0, w, h, title));
	if (wp.empty())
		return wp;
	windows.push_back(wp);
	return wp;
}

/// set the input focus to the given window
bool fltk_driver::set_focus(const_window_ptr )
{
	std::cerr << "set focus of fltk_driver not implemented" << std::endl;
	return false;
}
/// return the number of created windows
unsigned int fltk_driver::get_nr_windows()
{
	return (unsigned int) windows.size();
}

/// return the i-th created window
window_ptr fltk_driver::get_window(unsigned int i)
{
	if (i >= windows.size())
		return window_ptr();
	return windows[i];
}
/// run the main loop of the window system
bool fltk_driver::run()
{
	ensure_lock();
	return fltk::run() != 0;
}
/// quit the application by closing all windows
void fltk_driver::quit(int exit_code)
{
	for (unsigned int i=0; i<windows.size(); ++i) {
		static_cast<fltk::Window*>(static_cast<fltk::Widget*>(windows[i]->get_user_data()))->hide();
	}
#ifdef _WIN32
	TerminateProcess(GetCurrentProcess(), exit_code);
#else
	exit(exit_code);
#endif
}

/// copy text to the clipboard
void fltk_driver::copy_to_clipboard(const std::string& s)
{
	fltk::copy(s.c_str(), (int) s.length(), true);
}

struct PasteWidget : public fltk::Widget
{
	std::string clipboard;
	PasteWidget() : fltk::Widget(0,0,0,0) {}
	int handle(int event)
	{
		if (event == fltk::PASTE) {
			clipboard = fltk::event_text();
			return 1;
		}
		return 0;
	}
};

/// retreive text from clipboard
std::string fltk_driver::paste_from_clipboard()
{
	PasteWidget pw;
	fltk::paste(pw, true);
	return pw.clipboard;
}

/// create a text editor
text_editor_ptr fltk_driver::create_text_editor(unsigned int w, unsigned int h, const std::string& title, int x, int y)
{
	ensure_lock();
	fltk_text_editor* e = new fltk_text_editor(title,w,h,title.c_str());
	e->position(x,y);
	return text_editor_ptr(e);
}

/// ask the user with \c _question to select one of the \c answers, where \c default_answer specifies index of default answer
int fltk_driver::question(const std::string& _question, const std::vector<std::string>& answers, int default_answer)
{
	std::vector<const char*> answer_ptrs;
	answer_ptrs.resize(answers.size());
	for (unsigned i=0; i<answers.size(); ++i)
		answer_ptrs[i] = answers[i].c_str();
	std::string default_answer_string;
	if (default_answer > -1 && default_answer < (int)answers.size()) {
		default_answer_string = std::string("*")+answers[default_answer];
		answer_ptrs[default_answer] = default_answer_string.c_str();
	}
	switch (answers.size()) {
	case 0 :
	case 1 : fltk::message(_question.c_str()); return 0;
	case 2 : return fltk::ask(_question.c_str());
	case 3 : return fltk::choice(_question.c_str(), answer_ptrs[0], answer_ptrs[1], answer_ptrs[2]);
	case 4 : return fltk::choice(_question.c_str(), answer_ptrs[0], answer_ptrs[1], answer_ptrs[2], answer_ptrs[3]);
	case 5 : return fltk::choice(_question.c_str(), answer_ptrs[0], answer_ptrs[1], answer_ptrs[2], answer_ptrs[3], answer_ptrs[4]);
	case 6 : return fltk::choice(_question.c_str(), answer_ptrs[0], answer_ptrs[1], answer_ptrs[2], answer_ptrs[3], answer_ptrs[4], answer_ptrs[5]);
	}
	return -1;
}

bool fltk_driver::query(const std::string& question, std::string& text, bool password)
{
	const char* answer = password ? 
		fltk::password(question.c_str(), text.c_str()) :
		fltk::input(question.c_str(), text.c_str());
	if (answer == 0)
		return false;
	text = answer;
	return true;
}


void fltk_driver::lock()
{
	fltk::lock();
}
/// unlock the main thread
void fltk_driver::unlock()
{
	fltk::unlock();
}
/// wake the main thread to ensure that it is not going to sleep any longer
void fltk_driver::wake(const std::string& message)
{
	wakeup_message = message;
	if (wakeup_message.empty())
		fltk::awake();
	else
		fltk::awake(const_cast<char*>(wakeup_message.c_str()));
}

std::string fltk_driver::get_wakeup_message()
{
	const char* message = (const char*) fltk::thread_message();
	if (message)
		return std::string(message);
	return "";
}

/// let the main thread sleep for the given number of seconds
void fltk_driver::sleep(float time_in_seconds)
{
	fltk::wait(time_in_seconds);
}


/// add a new gui group to the given parent group
gui_group_ptr fltk_driver::add_group(gui_group_ptr parent, const std::string& label, const std::string& group_type, const std::string& options, const std::string& align)
{
	ensure_lock();
	int x,y,w,h;
	fltk_gui_group* fggp = parent->get_interface<fltk_gui_group>();
	fggp->prepare_new_element(parent,x,y,w,h);

		gui_group_ptr gg;
		if (group_type.empty() || group_type == "align_group")
			gg = gui_group_ptr(new fltk_align_group(x,y,w,h,label));
		else if (group_type == "tab_group")
			gg = gui_group_ptr(new fltk_tab_group(x,y,w,h,label));
		else if (group_type == "dockable_group")
			gg = gui_group_ptr(new fltk_dockable_group(x,y,w,h,label));
		else if (group_type == "layout_group")
			gg = gui_group_ptr(new fltk_layout_group(x,y,w,h,label));
		else if (group_type == "tree_group")
			gg = gui_group_ptr(new fltk_tree_group(x,y,w,h,label));
		// add further group types here

	if (!gg.empty()) {
		if (!options.empty())
			gg->multi_set(options);

		fggp->finalize_new_element(parent, align, gg);
	}
	return gg;
}

/// add a newly created decorator to the parent group
base_ptr fltk_driver::add_decorator(gui_group_ptr parent, const std::string& label, 
												const std::string& decorator_type, const std::string& options, 
												const std::string& align)
{
	ensure_lock();
	int x,y,w,h;
	fltk_gui_group* fggp = parent->get_interface<fltk_gui_group>();
	fggp->prepare_new_element(parent,x,y,w,h);

		base_ptr d = create_decorator(label,decorator_type,x,y,w,h);

	if (!d.empty()) {
		if (!options.empty())
			d->multi_set(options);

		fggp->finalize_new_element(parent,align, d);
	}
	return d;
}

button_ptr fltk_driver::add_button(gui_group_ptr parent, const std::string& label, const std::string& options, const std::string& align)
{
	ensure_lock();
	int x,y,w,h;
	fltk_gui_group* fggp = parent->get_interface<fltk_gui_group>();
	fggp->prepare_new_element(parent,x,y,w,h);

	button_ptr b(new fltk_button(label,x,y,w,h));

	if (!b.empty()) {
		if (!options.empty())
			b->multi_set(options);

		fggp->finalize_new_element(parent,align, b);
	}
	return b;
}

view_ptr fltk_driver::add_view(gui_group_ptr parent, const std::string& label, const void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align)
{
	ensure_lock();
	int x,y,w,h;
	fltk_gui_group* fggp = parent->get_interface<fltk_gui_group>();
	fggp->prepare_new_element(parent,x,y,w,h);

	view_ptr v = create_view(label, value_ptr, value_type, gui_type, x, y, w, h);

	if (!v.empty()) {
		if (!options.empty())
			v->multi_set(options);
		
		fggp->finalize_new_element(parent,align, v);
	}
	return v;
}

view_ptr fltk_driver::find_view(gui_group_ptr parent, const void* value_ptr, int* idx_ptr)
{
	ensure_lock();
	int n = parent->get_nr_children();
	int i=0;
	if (idx_ptr)
		i = *idx_ptr;
	for (; i<n; ++i) {
		abst_view* v = parent->get_child(i)->get_interface<abst_view>();
		if (v) {
			if (v->shows(value_ptr)) {
				if (idx_ptr)
					*idx_ptr = i;
				return view_ptr(v);
			}
		}
	}
	return view_ptr();
}


control_ptr fltk_driver::add_control(gui_group_ptr parent, const std::string& label, 
												 void* value_ptr, abst_control_provider* acp,
												 const std::string& value_type, 
												 const std::string& gui_type, 
												 const std::string& options, 
												 const std::string& align)
{
	ensure_lock();
	int x,y,w,h;
	fltk_gui_group* fggp = parent->get_interface<fltk_gui_group>();
	fggp->prepare_new_element(parent,x,y,w,h);

	control_ptr c = create_control(label, value_ptr, acp, value_type, gui_type, options, x, y, w, h);

	if (!c.empty()) {
		if (!options.empty())
			c->multi_set(options);
		
		fggp->finalize_new_element(parent,align, c);
	}
	return c;
}


control_ptr fltk_driver::find_control(gui_group_ptr parent, void* value_ptr, int* idx_ptr)
{
	ensure_lock();
	int n = parent->get_nr_children();
	int i=0;
	if (idx_ptr)
		i = *idx_ptr;
	for (; i<n; ++i) {
		abst_control* c = parent->get_child(i)->get_interface<abst_control>();
		if (c && c->controls(value_ptr)) {
			if (idx_ptr)
				*idx_ptr = i;
			return control_ptr(c);
		}
	}
	return control_ptr();
}

/// analyze a menu path description, search for the menu that will contain the path and split path in path and item name
fltk::Menu* fltk_driver::resolve_menu_path(const std::string& menu_path, std::string& path, std::string& name, bool ensure_created) const
{
	if (windows.empty())
		return 0;
	size_t pos = menu_path.find_first_of(':');
	std::string window_name;
	if (pos == std::string::npos)
		path = menu_path;
	else {
		path = menu_path.substr(pos+1);
		window_name = menu_path.substr(0,pos);
	}
	pos = path.find_last_of('/');
	if (pos == std::string::npos)
		name = path;
	else
		name = path.substr(pos+1);
	fltk::Menu* m = 0;
	for (unsigned int i=0; i<windows.size(); ++i) {
		if (window_name.empty() || window_name == windows[i]->get_name() || 
			window_name == windows[i]->get_type_name()) {
				fltk_viewer_window* fvw = windows[i]->get_interface<fltk_viewer_window>();
				if (fvw) {
					m = fvw->get_menu();
					if (m)
						break;
				}
		}

	}
	if (m) {
		fltk::Widget* w = m->find(menu_path.c_str());
		if (!w)
			return m;
		if (w->is_group())
			return (fltk::Menu*)w;
		return (fltk::Menu*)w->parent();
	}
	return 0;
}

struct fltk_separator : public named
{
	fltk::Widget* w;
	void* get_user_data() 
	{
		return w; 
	}
	fltk_separator(const std::string& name) : named(name)
	{
		set_ref_count(1);
	}
	void self_unref()
	{
		set_ref_count(get_ref_count()-1);
	}
};

struct fltk_menu_button : public button, public fltk_base
{
	fltk::Widget* w;
	void* get_user_data() 
	{
		return w; 
	}
	static void menu_cb(fltk::Widget* w, void* p)
	{
		fltk_menu_button* b = static_cast<fltk_menu_button*>(static_cast<base*>(p));
		b->click(*b);
	}
	fltk_menu_button(const std::string& name) : button(name)
	{
		set_ref_count(1);
	}
	/// only uses the implementation of fltk_base
	std::string get_property_declarations()
	{
		return fltk_base::get_property_declarations();
	}
	/// abstract interface for the setter
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
	{
		return fltk_base::set_void(w, this, property, value_type, value_ptr);
	}
	/// abstract interface for the getter
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr)
	{
		return fltk_base::get_void(w, this, property, value_type, value_ptr);
	}
	void self_unref()
	{
		set_ref_count(get_ref_count()-1);
	}
};


/// add a newly created decorator to the menu
base_ptr fltk_driver::add_menu_separator(const std::string& menu_path)
{
	std::string path, name;
	fltk::Menu* mp = resolve_menu_path(menu_path, path, name,true);
	if (mp) {
		mp->begin();
		new fltk::Divider();
		mp->end();
		return base_ptr();
		/*
		fltk_separator* sep = new fltk_separator(path); 
		fltk::Widget* w = mp->add(sep->get_name().c_str(), 0, 0, sep->get_interface<base>());
		sep->w = w;
		return base_ptr(sep);*/
	}
	return 0;
}

/// use the current gui driver to append a new button in the menu, where menu path is a '/' separated path
button_ptr fltk_driver::add_menu_button(const std::string& menu_path, const std::string& options)
{
	std::string path, name;
	fltk::Menu* mp = resolve_menu_path(menu_path, path, name,true);
	if (mp) {
		fltk_menu_button* b = new fltk_menu_button(path); 
		fltk::Widget* w = mp->add(b->get_name().c_str(), 0, fltk_menu_button::menu_cb, b->get_interface<base>());
		b->w = w;
		b->multi_set(options);
		return button_ptr(b);
	}
	return button_ptr();
}
/// use this to add a new control to the gui with a given value type, gui type and init options
cgv::data::ref_ptr<control<bool> > fltk_driver::add_menu_bool_control(const std::string& menu_path, bool& value, const std::string& options)
{
	return cgv::data::ref_ptr<control<bool> >();
}

/// return the element of the given menu path
base_ptr fltk_driver::find_menu_element(const std::string& menu_path) const
{
	std::string path, name;
	fltk::Menu* mp = resolve_menu_path(menu_path, path, name, false);
	if (mp) {
		fltk::Widget* w = mp->find(path.c_str());
		if (w)
			return base_ptr((base*)w->user_data());
	}
	return base_ptr();
}

/// remove a single element from the gui
void fltk_driver::remove_menu_element(base_ptr bp)
{
	if (bp->get_interface<fltk_menu_button>()) {
		fltk_menu_button* fmb = bp->get_interface<fltk_menu_button>();
		fmb->w->parent()->remove(*fmb->w);
		fmb->self_unref();
	}
	else if (bp->get_interface<fltk_separator>()) {
		fltk_separator* fs = bp->get_interface<fltk_separator>();
		fs->w->parent()->remove(*fs->w);
		fs->self_unref();
	}
}

base_ptr get_base_provider_generator(bool unregister = false)
{
	static base_ptr bpg_ptr;
	if (unregister) {
		if (bpg_ptr)
			unregister_object(bpg_ptr);
		bpg_ptr.clear();
	}
	else {
		if (!bpg_ptr) {
			bpg_ptr = base_ptr(new cgv::gui::base_provider_generator());
			register_object(base_ptr(bpg_ptr));
		}
	}
	return bpg_ptr;
}

/// process the gui declarations in the given gui file
bool fltk_driver::process_gui_file(const std::string& file_name)
{
	cgv::gui::base_provider_generator* bpg = get_base_provider_generator()->get_interface<cgv::gui::base_provider_generator>();
	return bpg->parse_gui_file(file_name);
}

struct menu_listener : public cgv::base::base, public registration_listener
{
	std::string get_type_name() const { return "menu_listener"; }
	void register_object(base_ptr object, const std::string& options)
	{
		menu_provider* mp = object->get_interface<menu_provider>();
		if (!mp) 
			return;
		mp->create_menu();
	}
	void unregister_object(base_ptr object, const std::string& options)
	{
		menu_provider* mp = object->get_interface<menu_provider>();
		if (!mp) 
			return;
		mp->destroy_menu();
	}
};

object_registration<fltk_driver> fltk_driver_registration("fltk driver");
object_registration<menu_listener> fml_reg("fltk menu driver");

//object_registration<fltk_driver> fltk_driver_registration;
//object_registration<menu_listener> fml_reg;
