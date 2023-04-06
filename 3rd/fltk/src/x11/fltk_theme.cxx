//
// "$Id: fltk_theme.cxx 5456 2006-09-19 02:41:42Z spitzak $"
//
// Copyright 2004 Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

// This is the adjusted default fltk_theme() function for fltk on X. It
// mimics the Windows95 fltk_theme() function by using hardcoded default
// values for colors that resemble the windows colors. The old X
// fltk_theme implementation can be found under fltk_theme.bkp.

#include <fltk/Widget.h>
#include <fltk/Button.h>
#include <fltk/TabGroup.h>
#include <fltk/draw.h>
#include <fltk/Monitor.h>
#include <fltk/events.h>
#include <fltk/string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#if USE_X11
# include <fltk/x.h>
#endif

using namespace fltk;

////////////////////////////////////////////////////////////////

extern "C" bool fltk_theme() {

	// Use same default colors as received on Windows95 systems
	Color background = 4042321920u;
	Color foreground = 56u;
	Color select_background = 7919360u;
	Color select_foreground = 4294967040u;
	Color text_background = 4294967040u;
	Color text_foreground = 56u;
	Color tooltip_background = 4294959360u;
	Color tooltip_foreground = 56u;

	int theme_idx = theme_idx_;
	Style* style;

	fltk::reset_indexed_colors();

	// set colors and styling based on selected theme
	if(theme_idx < 0) {
		// original
		fltk::set_background(background);
		Widget::default_style->labelcolor_ = foreground;
		Widget::default_style->highlight_textcolor_ = foreground;
		Widget::default_style->color_ = text_background;
		Widget::default_style->textcolor_ = text_foreground;
		Widget::default_style->selection_color_ = select_background;
		Widget::default_style->selection_textcolor_ = select_foreground;

		TabGroup::flat_tabs(false);

		if((style = Style::find("Tooltip"))) {
			style->color_ = tooltip_background;
			style->labelcolor_ = tooltip_foreground;
		}

		fltk::set_theme_color(THEME_BACKGROUND_COLOR, GRAY75);
		fltk::set_theme_color(THEME_GROUP_COLOR, GRAY75);
		fltk::set_theme_color(THEME_CONTROL_COLOR, GRAY70);
		fltk::set_theme_color(THEME_BORDER_COLOR, GRAY33);
		fltk::set_theme_color(THEME_TEXT_COLOR, text_foreground);
		fltk::set_theme_color(THEME_TEXT_BACKGROUND_COLOR, text_background);
		fltk::set_theme_color(THEME_SELECTION_COLOR, select_background);
		fltk::set_theme_color(THEME_HIGHLIGHT_COLOR, select_background);
		fltk::set_theme_color(THEME_WARNING_COLOR, RED);
	} else {
		Color highlight_textcolor = foreground;

		// See corporate design 07/2021
		// "Auszeichnungsfarbe 1"
		const Color tud_marking1_blue = color(0, 105, 180);
		// "Auszeichnungsfarbe 2"
		const Color tud_marking2_blue = color(0, 159, 227);
		// "TUD-Web-Interface" - colors for web or other digital media
		// TU-Dresden blue
		const Color tud_blue = color(0, 37, 87);
		// dark red
		const Color tud_dark_red = color(181, 28, 28);
		// light red
		const Color tud_light_red = color(221, 39, 39);

		Color window_color = BLACK;
		// GRAY30 is set to a darker shade of c0 (darkening depending on the brightness: less darkening for brighter c0): used for the shadowing effect on SHADOW_UP_BOX
		Color c0 = WHITE; // set as GRAY33: the window background color (shall be the darkest color in the theme, except black and text colors)
		Color c1 = WHITE; // set as GRAY75: the background color for gui groups
		Color c2 = WHITE; // set as GRAY80: lighter than c1, used for controls
		Color c3 = WHITE; // set as GRAY85: lighter or darker shade than c1, used for border frames
		Color text_symbol_color = WHITE; // set as GRAY95: used for some labels (like slider handle markings)
		Color dial_color = WHITE;
		Color warning_color = RED;

		switch(theme_idx) {
		case 0:
			c0 = color(0.70f);
			c1 = color(0.90f);
			c2 = color(0.98f);
			c3 = color(0.80f);
			text_symbol_color = color(0.19f);

			foreground = color(0.16f);
			text_foreground = text_symbol_color;
			text_background = c2;
			highlight_textcolor = tud_marking1_blue;
			select_background = tud_marking2_blue;
			select_foreground = GRAY99;
			window_color = GRAY33;
			dial_color = c2;
			warning_color = tud_light_red;

			break;
		case 1:
			c0 = color(0.42f);
			c1 = color(0.64f);
			c2 = color(0.78f);
			c3 = color(0.58f);
			text_symbol_color = color(0.04f);

			foreground = text_symbol_color;
			text_foreground = text_symbol_color;
			text_background = c2;
			highlight_textcolor = tud_marking1_blue;
			select_background = tud_marking2_blue;
			select_foreground = GRAY99;
			window_color = GRAY33;
			dial_color = c2;
			warning_color = tud_light_red;

			break;
		case 2:
			c0 = color(0.16f);
			c1 = color(0.22f);
			c2 = color(0.30f);
			c3 = color(0.40f);
			text_symbol_color = color(0.86f);

			foreground = text_symbol_color;
			text_foreground = text_symbol_color;
			text_background = c0;
			highlight_textcolor = tud_marking2_blue;
			select_background = tud_marking1_blue;
			select_foreground = GRAY99;
			window_color = GRAY33;
			dial_color = c3;
			warning_color = tud_dark_red;

			break;
		case 3:
			c0 = color(0.10f);
			c1 = color(0.15f);
			c2 = color(0.20f);
			c3 = color(0.28f);
			text_symbol_color = color(0.74f);

			foreground = text_symbol_color;
			text_foreground = text_symbol_color;
			text_background = c0;
			highlight_textcolor = tud_marking2_blue;
			select_background = tud_marking1_blue;
			select_foreground = GRAY99;
			window_color = GRAY33;
			dial_color = c3;
			warning_color = tud_dark_red;

			break;
		}

		fltk::set_theme_color(THEME_BACKGROUND_COLOR, c0);
		fltk::set_theme_color(THEME_GROUP_COLOR, c1);
		fltk::set_theme_color(THEME_CONTROL_COLOR, c2);
		fltk::set_theme_color(THEME_BORDER_COLOR, c3);
		fltk::set_theme_color(THEME_TEXT_COLOR, text_foreground);
		fltk::set_theme_color(THEME_TEXT_BACKGROUND_COLOR, text_background);
		fltk::set_theme_color(THEME_SELECTION_COLOR, select_background);
		fltk::set_theme_color(THEME_HIGHLIGHT_COLOR, highlight_textcolor);
		fltk::set_theme_color(THEME_WARNING_COLOR, warning_color);

		fltk::set_main_gui_colors(c0, c1, c2, c3, text_symbol_color);
		//fltk::shift_background(background);
		Widget::default_style->labelcolor_ = foreground;
		Widget::default_style->highlight_textcolor_ = highlight_textcolor;
		Widget::default_style->color_ = text_background;
		Widget::default_style->textcolor_ = text_foreground;
		Widget::default_style->selection_color_ = select_background;
		Widget::default_style->selection_textcolor_ = select_foreground;

		Widget::default_style->box_ = BORDER_BOX;
		Widget::default_style->buttonbox_ = FLAT_BOX;


		TabGroup::flat_tabs(true);

		if((style = Style::find("Window"))) {
			style->color_ = window_color;
		}

		if((style = Style::find("MenuBar"))) {
			style->buttonbox_ = MENU_BOX;
		}

		if((style = Style::find("MenuWindow"))) {
			style->box_ = BORDER_BOX;
		}

		// use FLAT_UP_BOX if you dont want the thin shadow around the box
		Box* default_buttonbox = SHADOW_UP_BOX;
		Button::default_style->box_ = default_buttonbox;
		Button::default_style->buttonbox_ = default_buttonbox;
		/*if((style = Style::find("Button"))) {
			style->box_ = SHADOW_UP_BOX;
		}*/

		/*
		FLAT_BOX: no border, only the background color
		BORDER_BOX: background color framed in a 1px thick broder
		*/
		Box* input_frame_box = FLAT_BOX;
		//Box* input_frame_box = BORDER_BOX;
		/*
		NO_BOX: buttons of inputs are not highlighted and have the same background color as the input itself
		FLAT_UP_BOX: the buttons have a different background color than the input itself
		*/
		Box* input_button_box = NO_BOX;
		//Box* input_button_box = FLAT_UP_BOX;

		if((style = Style::find("Choice"))) {
			style->box_ = input_frame_box;
			style->buttonbox_ = input_button_box;
		}

		if((style = Style::find("Dial"))) {
			style->color_ = GRAY80;
			style->textcolor_ = GRAY33;
			style->color_ = dial_color;
		}

		if((style = Style::find("Output"))) {
			style->box_ = input_frame_box;
		}

		if((style = Style::find("ValueOutput"))) {
			style->box_ = input_frame_box;
		}

		if((style = Style::find("Input"))) {
			style->box_ = input_frame_box;
			//style->buttonbox_ = input_button_box;
		}

		if((style = Style::find("ValueInput"))) {
			style->box_ = input_frame_box;
			style->buttonbox_ = input_button_box;
		}

		if((style = Style::find("Check_Button"))) {
			style->box_ = input_frame_box;
		}

		if((style = Style::find("Slider"))) {
			style->box_ = FLAT_BOX; // set to NO_BOX, to make color changes only affect the tickmarks
			style->buttonbox_ = FLAT_BOX;
			style->buttoncolor_ = GRAY80;
			//style->selection_color_ = RED; / can change color of slider handle
		}

		if((style = Style::find("ThumbWheel"))) {
			style->box_ = input_frame_box;
			style->buttonbox_ = NO_BOX;
			style->color_ = GRAY80; // set this if you want a light background
		}

		if((style = Style::find("Scrollbar"))) {
			style->color_ = GRAY33;
			style->buttoncolor_ = GRAY80;
			style->box_ = NO_BOX;
			style->buttonbox_ = FLAT_BOX;
		}

		if((style = Style::find("Tooltip"))) {
			style->box_ = BORDER_BOX;
			style->color_ = text_background;
			style->textcolor_ = text_symbol_color;
		}
	}

	return true;
}

//
// End of "$Id: fltk_theme.cxx 5456 2006-09-19 02:41:42Z spitzak $".
//
