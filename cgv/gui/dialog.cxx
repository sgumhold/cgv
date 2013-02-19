#include "gui_driver.h"
#include "dialog.h"
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/scan.h>

using namespace cgv::utils;

namespace cgv {
	namespace gui {

/// tell the user something with a message box
void message(const std::string& _message)
{
	gui_driver_ptr d = get_gui_driver();
	if (d.empty())
		return;

	d->question(_message,std::vector<std::string>());
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


	}
}