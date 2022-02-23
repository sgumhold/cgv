//
// "$Id: Box.h 5865 2007-06-01 13:04:19Z sanel.z $"
//
// Define your own values for box() on a widget by making one of these.
//
// Copyright 2002 by Bill Spitzak and others.
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

#ifndef fltk_Box_h
#define fltk_Box_h

#include "Color.h"
#include "Flags.h"
#include "Symbol.h"

namespace fltk {

typedef Symbol Box;

class FL_API FrameBox : public Box {
protected:
  const char* data_;
  const Box* down_;
public:
  const char* data() const {return data_;}
  void data(const char* d) {data_ = d;}
  void _draw(const Rectangle&) const;
  void inset(Rectangle&) const;
  bool fills_rectangle() const;
  bool is_frame() const;
  FrameBox(const char* name, int dx,int dy,int dw,int dh, const char* pattern, const Box* down=0)
    : Box(name),data_(pattern),down_(down) {set_inset(dx,dy,-dw,-dh);}
};

class FL_API StateBox : public Box {
protected:
	const Box* up_;
	const Box* down_;
public:
	void _draw(const Rectangle&) const;
	bool fills_rectangle() const;
	bool is_frame() const;
	StateBox(const char* name, const Box* up = 0, const Box* down = 0)
		: Box(name), up_(up), down_(down) {}
};

class FL_API StateBox2 : public FrameBox {
protected:
	const char bg_color_;
	const char hover_color_;
public:
	void _draw(const Rectangle&) const;
	StateBox2(const char* name, int dx, int dy, int dw, int dh, const char* pattern, const char bg, const char hover, const Box* down = 0)
		: FrameBox(name, dx, dy, dw, dh, pattern, down), bg_color_(bg), hover_color_(hover) {}
};

class FL_API FlatBox : public Box {
public:
  void _draw(const Rectangle&) const;
  bool fills_rectangle() const;
  bool is_frame() const;
  FlatBox(const char* n);
};

class FL_API HoverBox : public FlatBox {
protected:
	const char bg_color_;
	const char hover_color_;
public:
	void _draw(const Rectangle&) const;
	bool fills_rectangle() const;
	bool is_frame() const;
	HoverBox(const char* name, const char bg, const char hover_bg)
		: FlatBox(name), bg_color_(bg), hover_color_(hover_bg) {}
};

class FL_API HighlightBox : public FlatBox {
  const Box* down_;
public:
  void _draw(const Rectangle&) const;
  bool fills_rectangle() const;
  bool is_frame() const;
  HighlightBox(const char* n, const Box* d);
};

}

#endif

//
// End of "$Id: Box.h 5865 2007-06-01 13:04:19Z sanel.z $".
//
