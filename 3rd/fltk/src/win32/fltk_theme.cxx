//
// "$Id: fltk_theme.cxx 6025 2008-02-04 01:26:46Z dejan $"
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

// This is the default fltk_theme() function on Windows. It reads
// colors and font sizes from standard Windows95 interfaces.

#include <fltk/Widget.h>
#include <fltk/Button.h>
#include <fltk/TabGroup.h>
#include <fltk/draw.h>
#include <fltk/Monitor.h>
#include <fltk/Font.h>
#include <fltk/events.h>
#include <stdio.h>
#include <fltk/string.h>
#include <fltk/utf.h>
#include <fltk/x.h>
#include <limits.h>
#include <wchar.h>

using namespace fltk;

extern int has_unicode();

////////////////////////////////////////////////////////////////

#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif

#ifndef WHEEL_PAGESCROLL
#define WHEEL_PAGESCROLL	(UINT_MAX) /* Scroll one page */
#endif

static Color win_color(int wincol) {
  int R = wincol&0xff;
  int G = (wincol >> 8)&0xff;
  int B = (wincol >> 16)&0xff;
  Color col = color(R, G, B);
  if (col) return col;
  return BLACK;
}

static int win_fontsize(int winsize) {
  if (winsize < 0) return -winsize; // -charsize: which is what FLTK uses
  if (winsize == 0) return 12; // pick any good size.  12 is good!
  return winsize*3/4; // cellsize: convert to charsize
}

extern "C" bool fltk_theme() {

  Color background = win_color(GetSysColor(COLOR_BTNFACE));
  Color foreground = win_color(GetSysColor(COLOR_BTNTEXT));
  Color select_background = win_color(GetSysColor(COLOR_HIGHLIGHT));
  Color select_foreground = win_color(GetSysColor(COLOR_HIGHLIGHTTEXT));
  Color text_background = win_color(GetSysColor(COLOR_WINDOW));
  Color text_foreground = win_color(GetSysColor(COLOR_WINDOWTEXT));
// These colors are incorrect on XP with it's default scheme:
//   Color menuitem_background = win_color(GetSysColor(COLOR_MENU));
//   Color menuitem_foreground = win_color(GetSysColor(COLOR_MENUTEXT));
  Color tooltip_background = win_color(GetSysColor(COLOR_INFOBK));
  Color tooltip_foreground = win_color(GetSysColor(COLOR_INFOTEXT));
// Windows doesn't seem to honor this one
// Color slider_background = win_color(GetSysColor(COLOR_SCROLLBAR));

  int theme_idx = theme_idx_;
  Style* style;

  fltk::reset_indexed_colors();

  // TODO: split flat style from windows theme?

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
	  fltk::set_theme_color(THEME_SHADOW_COLOR, GRAY30);
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

	  fltk::set_main_gui_colors(c0, c1, c2, c3, text_symbol_color);

	  fltk::set_theme_color(THEME_BACKGROUND_COLOR, c0);
	  fltk::set_theme_color(THEME_GROUP_COLOR, c1);
	  fltk::set_theme_color(THEME_CONTROL_COLOR, c2);
	  fltk::set_theme_color(THEME_BORDER_COLOR, c3);
	  fltk::set_theme_color(THEME_TEXT_COLOR, text_foreground);
	  fltk::set_theme_color(THEME_TEXT_BACKGROUND_COLOR, text_background);
	  fltk::set_theme_color(THEME_SELECTION_COLOR, select_background);
	  fltk::set_theme_color(THEME_HIGHLIGHT_COLOR, highlight_textcolor);
	  fltk::set_theme_color(THEME_WARNING_COLOR, warning_color);
	  fltk::set_theme_color(THEME_SHADOW_COLOR, GRAY30);

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

//   if (menuitem_background != background || menuitem_foreground != foreground) {
//     if ((style = Style::find("Menu"))) {
//       style->color_ = menuitem_background;
//       style->textcolor_ = menuitem_foreground;
// //    style->selection_color_ = select_background;
// //    style->selection_textcolor_ = select_foreground;
//     }
//   }

/* This is the same as the defaults:
  if ((style = Style::find("menu title"))) {
    style->highlight_color_ = GRAY75;
    style->highlight_textcolor_ = foreground;
    style->selection_color_ = GRAY75;
    style->selection_textcolor_ = foreground;
  }
*/

  /*
     Windows font stuff

     It looks Windows has just three separate fonts that it actually
     uses for stuff replaced by FLTK.  But the "Display Properties"
     dialog has a lot more fonts that you can set?  Wrong, look again.
     Some of the fonts are duplicates and another doesn't do anything!
     It has fonts for the titlebar and icons which we don't have to worry
     about, a menu font which is used for menubars and menu items, a
     status font which is for status bars and tooltips, and a message
     box font which is used for everything else.  Except that it's not
     used by everything else;  almost all non-menu widgets in every
     application I tested did not respond to font changes.  The fonts
     are apparently hard coded by the applications which seems to me to
     bad programming considering that Windows has an adequate system for
     allowing the user to specify font preferences.  This is especially
     true of Microsoft applications and Windows itself!  We will allow
     FLTK applications to automatically use the fonts specified by the
     user.

     CET
  */
#ifndef _WIN32_WCE

  if (has_unicode()) {
    NONCLIENTMETRICSW ncm;
    int sncm = sizeof(ncm);
    ncm.cbSize = sncm;
    if (false == SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sncm, &ncm, SPIF_SENDCHANGE))
      return false;
    Font* font; float size;
    wchar_t *name;
    const int BUFLEN = 1024;
    char buffer[BUFLEN];

    // get font info for regular widgets from LOGFONT structure
    name = ncm.lfMessageFont.lfFaceName;
    utf8fromwc(buffer, BUFLEN, name, unsigned(wcslen(name)));
    font = fltk::font(buffer,
		     (ncm.lfMessageFont.lfWeight >= 600 ? BOLD : 0) +
		     (ncm.lfMessageFont.lfItalic ? ITALIC : 0));
    size = float(win_fontsize(ncm.lfMessageFont.lfHeight));

    Widget::default_style->labelfont_ = font;
    Widget::default_style->textfont_ = font;
    Widget::default_style->labelsize_ = size;
    Widget::default_style->textsize_ = size;

    // get font info for menu items from LOGFONT structure
    name = ncm.lfMenuFont.lfFaceName;
    utf8fromwc(buffer, BUFLEN, name, unsigned(wcslen(name)));
    font = fltk::font(buffer,
		     (ncm.lfMenuFont.lfWeight >= 600 ? BOLD : 0) +
		     (ncm.lfMenuFont.lfItalic ? ITALIC : 0));
    size = float(win_fontsize(ncm.lfMenuFont.lfHeight));
    if ((style = Style::find("Menu"))) {
      style->textfont_ = font;
      style->textsize_ = size;
    }

    if ((style = Style::find("Tooltip"))) {
      name = ncm.lfStatusFont.lfFaceName;
      utf8fromwc(buffer, BUFLEN, name, unsigned(wcslen(name)));
      // get font info for tooltips from LOGFONT structure
      font = fltk::font(buffer,
		       (ncm.lfStatusFont.lfWeight >= 600 ? BOLD : 0) +
		       (ncm.lfStatusFont.lfItalic ? ITALIC : 0));
      size = float(win_fontsize(ncm.lfStatusFont.lfHeight));

      style->labelfont_ = font;
      style->labelsize_ = size;
    }

  } else {

    NONCLIENTMETRICSA ncm;
    int sncm = sizeof(ncm);
    ncm.cbSize = sncm;
    if (false == SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sncm, &ncm, SPIF_SENDCHANGE))
      return false;

    Font* font; float size;
    char *name;

    // get font info for regular widgets from LOGFONT structure
    name = ncm.lfMessageFont.lfFaceName;
    font = fltk::font(name,
		     (ncm.lfMessageFont.lfWeight >= 600 ? BOLD : 0) +
		     (ncm.lfMessageFont.lfItalic ? ITALIC : 0));
    size = float(win_fontsize(ncm.lfMessageFont.lfHeight));

    Widget::default_style->labelfont_ = font;
    Widget::default_style->textfont_ = font;
    Widget::default_style->labelsize_ = size;
    Widget::default_style->textsize_ = size;

    // get font info for menu items from LOGFONT structure
    name = ncm.lfMenuFont.lfFaceName;
    font = fltk::font(name,
		     (ncm.lfMenuFont.lfWeight >= 600 ? BOLD : 0) +
		     (ncm.lfMenuFont.lfItalic ? ITALIC : 0));
    size = float(win_fontsize(ncm.lfMenuFont.lfHeight));
    if ((style = Style::find("Menu"))) {
      style->textfont_ = font;
      style->textsize_ = size;
    }

    if ((style = Style::find("Tooltip"))) {
      name = ncm.lfStatusFont.lfFaceName;
      // get font info for tooltips from LOGFONT structure
      font = fltk::font(name,
		       (ncm.lfStatusFont.lfWeight >= 600 ? BOLD : 0) +
		       (ncm.lfStatusFont.lfItalic ? ITALIC : 0));
      size = float(win_fontsize(ncm.lfStatusFont.lfHeight));

      style->labelfont_ = font;
      style->labelsize_ = size;
    }
  }
#endif
  // grab mousewheel stuff from Windows
  UINT delta;
  SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, (PVOID)&delta, 0);
  if (delta == WHEEL_PAGESCROLL) Style::wheel_scroll_lines_ = 10000;
  else Style::wheel_scroll_lines_ = (int)delta;

  // CET - FIXME - do encoding stuff
  return true;
}

//
// End of "$Id: fltk_theme.cxx 6025 2008-02-04 01:26:46Z dejan $".
//
