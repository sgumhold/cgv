#include "gui_driver.h"
#include "dialog.h"
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/scan.h>
#include <cgv/gui/application.h>
#include <cgv/signal/rebind.h>

using namespace cgv::utils;

namespace cgv {
	namespace gui {

/// tell the user something with a message box
void message(const std::string& _message)
{
	gui_driver_ptr d = get_gui_driver();
	if (d.empty())
		return;

	d->message(_message);
}

/// ask the user with \c question to select one of the \c answers, where \c default_answer specifies index of default answer
int question(const std::string& _question, const std::vector<std::string>& answers, int default_answer)
{
	gui_driver_ptr d = get_gui_driver();
	if (d.empty())
		return -1;

	return d->question(_question,answers,default_answer);
}

/// second question interface, where \c answers is a comma seprated list of enum declaration
int question(const std::string& _question, const std::string& answers, int default_answer)
{
	gui_driver_ptr d = get_gui_driver();
	if (d.empty())
		return -1;

	std::vector<std::string> answer_strings;
	std::vector<int> answer_values;

	std::vector<token> tokens;
	tokenizer(answers).set_ws(",").bite_all(tokens);
	int last = -1;
	for (unsigned i=0; i<tokens.size(); ++i) {
		std::vector<token> toks;
		tokenizer(tokens[i]).set_ws("=").bite_all(toks);
		int idx = ++last;
		if (toks.size() > 0) {
			answer_strings.push_back(to_string(toks[0]));
			if (toks.size() > 1)
				is_integer(toks[1].begin, toks[1].end, idx);
		}
		answer_values.push_back(idx);
	}

	return answer_values[d->question(_question,answer_strings,default_answer)];
}

bool query(const std::string& question, std::string& text, bool password)
{
	gui_driver_ptr d = get_gui_driver();
	if (d.empty())
		return false;

	return d->query(question,text,password);
}

void dialog::set_true_and_hide(bool* result, cgv::gui::window* w)
{
	*result = true;
	w->hide();
}
/// create from title and adjust size according to content
dialog::dialog(const std::string& title, const std::string& group_type)
{
	result = false;
	adjust_size = true;
	D = cgv::gui::application::create_window(400,10,title,"generic");
	D->set("group", group_type);
}

/// create from dimensions and title
dialog::dialog(int w, int h, const std::string& title, const std::string& group_type)
{
	result = false;
	adjust_size = h <= 0;
	D = cgv::gui::application::create_window(w,adjust_size ? 10 : h,title,"generic");
	D->set("group", group_type);
}

/// return the gui group to which new elements are to be add
cgv::gui::gui_group_ptr dialog::group()
{
	return D->get_inner_group();
}

/// add buttons for ok and or cancel
void dialog::add_std_buttons(const std::string& ok_label, const std::string& cancel_label)
{
	if (!ok_label.empty())
		cgv::signal::connect_copy(group()->add_button(ok_label, "w=75", "     ")->click, cgv::signal::rebind(set_true_and_hide, cgv::signal::_c(&result), cgv::signal::_c(&(*D))));
	if (!cancel_label.empty())
		cgv::signal::connect_copy(group()->add_button(cancel_label, "w=75", "\n")->click, cgv::signal::rebind(&(*D), &cgv::gui::window::hide));
}

/// execute modal dialog and freeze all other windows
bool dialog::exec()
{
	result = false;
	if (adjust_size) {
		D->set("H", group()->get<int>("H"));
	}
	D->set("hotspot", true);
	D->show(true);
	return result;
}



	}
}