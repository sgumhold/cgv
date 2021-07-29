#pragma once

#include <string>
#include <vector>
#include <cgv/gui/window.h>

#include "lib_begin.h"

namespace cgv {
	namespace gui {
		/// tell the user something with a message box
		extern CGV_API void message(const std::string& _message);
		/// ask the user with \c question to select one of the \c answers, where \c default_answer specifies index of default answer
		extern CGV_API int question(const std::string& question, const std::vector<std::string>& answers = {}, int default_answer = -1);
		/// second question interface, where \c answers is a comma seprated list of enum declaration
		extern CGV_API int question(const std::string& question, const std::string& answers, int default_answer = -1);
		//! query the user for a text, where the second parameter is the default \c text as well as the returned text. 
		/*! If \c password is true, the text is hidden. The function returns false if the user canceled the input of if no gui driver is available. */
		extern CGV_API bool query(const std::string& question, std::string& text, bool password = false);
		/** use this class to construct and execute a modal dialog. */
		class CGV_API dialog 
		{
			bool adjust_size;
			bool result;
		protected:
			cgv::gui::window_ptr D;
			static void set_true_and_hide(bool* result, cgv::gui::window* w);
		public:
			/// create from title and adjust size according to content
			dialog(const std::string& title, const std::string& group_type = "align_group");
			/// create from dimensions and title
			dialog(int w, int h, const std::string& title, const std::string& group_type = "align_group");
			/// return the gui group to which new elements are to be add
			cgv::gui::gui_group_ptr group();
			/// add buttons for ok and or cancel
			void add_std_buttons(const std::string& ok_label = "ok", const std::string& cancel_label = "");
			/// execute modal dialog and freeze all other windows
			bool exec();
		};

	}
}

#include <cgv/config/lib_end.h>