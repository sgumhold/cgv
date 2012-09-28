#pragma once

#include <vector>
#include <fltk/Group.h>

class DockableGroup : public fltk::Group 
{
  int spacing_;
  int margin_left_;
  int margin_right_;
  int margin_top_;
  int margin_bottom_;
  int last_x, last_y;
  fltk::Widget* drag_widget;
public:
  std::vector<fltk::Widget*> dock_order;
  int handle(int event);
  void layout();
  DockableGroup(int x, int y, int w, int h, const char *l = 0, bool begin=false);
  ~DockableGroup();
  void draw();
  void dock(fltk::Widget* w, int side, bool resizable = false);
  void undock(fltk::Widget* w);

  int spacing() const {return spacing_;}
  void spacing(int i) {spacing_ = i;}

  int margin_left() const {return margin_left_;}
  void margin_left(int m) {margin_left_ = m;}
  int margin_right() const {return margin_right_;}
  void margin_right(int m) {margin_right_ = m;}
  int margin_top() const {return margin_top_;}
  void margin_top(int m) {margin_top_ = m;}
  int margin_bottom() const {return margin_bottom_;}
  void margin_bottom(int m) {margin_bottom_ = m;}

  void margin(int m) {margin_left_ = margin_right_ = margin_top_ = margin_bottom_ = m;}
};