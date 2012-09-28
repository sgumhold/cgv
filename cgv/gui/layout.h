#pragma once

#include <cgv/base/base.h>
#include <cgv/base/group.h>
#include <cgv/gui/resizable.h>
#include <cgv/signal/signal.h>
#include <cgv/signal/rebind.h>
#include <cgv/data/ref_ptr.h>
#include <cgv/type/variant.h>

#include <cgv/gui/layout_spacings.h>

#include "lib_begin.h"

using namespace cgv::base;

namespace cgv {
	namespace gui {

		/**
			Allowed hints:
			s - horizontal shrink
			S - vertical shrink
			x - horizontal expand
			X - vertical expand
			f - horizontal fill
			F - vertical fill
			l - align left
			r - align right
			c - align centered
			t - align top
			b - align bottom
			m - align middle (between top and bottom)
		*/
		enum layout_hint 
		{
			LH_HSHRINK = 1<<0,
			LH_HEXPAND = 1<<1,
			LH_HFILL = 1<<2,
			LH_LEFT = 1<<3,
			LH_CENTER = 1<<4,
			LH_RIGHT = 1<<5,

			LH_VSHRINK = 1<<6,
			LH_VEXPAND = 1<<7,
			LH_VFILL = 1<<8,
			LH_TOP = 1<<9,
			LH_MIDDLE = 1<<10,
			LH_BOTTOM = 1<<11,

			LH_HHINTS = 0,
			LH_VHINTS = 6
		};

		// abstract class to define a layout on containers.
		class CGV_API layout: public cgv::base::base
		{
		public:
			// constructor that sets a container
			layout(cgv::base::group_ptr container = 0);
			// empty destructor
			~layout();

			// set a container to be layouted
			void set_container(cgv::base::group_ptr container);

			// set the spacing parameters
			void set_spacings(const layout_spacings spacings);

			// update children
			virtual void update() {};

			// resize
			void resize(int w, int h);

			virtual std::string get_property_declarations();
			virtual bool set_void(const std::string &property, const std::string &value_type, const void *value_ptr);
			virtual bool get_void(const std::string &property, const std::string &value_type, void *value_ptr);

		protected:
			cgv::base::group_ptr container;
			int w, h, true_w, true_h;
			int min_w, min_h, default_w, default_h;
			layout_spacings spacings;
			std::string spacings_name;

			// get the layout hints for a child
			int get_child_layout_hints(cgv::base::base_ptr child);

			// get the layouted container
			cgv::base::group_ptr get_container();
	
			// get a child from the container.
			base_ptr get_child(unsigned int i);

			// get the size of a child
			void get_child_size(const base_ptr child, int &width, int &height);
			// set the size of a child
			void set_child_size(const base_ptr child, int width, int height);

			// get the default size of a child
			void get_child_default_size(const base_ptr child, int &width, int &height);

			// get the position of a child
			void get_child_position(const base_ptr child, int &x, int &y);
			// set the position of a child
			void set_child_position(const base_ptr child, int x, int y);

			// get the mimimum size of a child
			void get_child_minimum_size(const base_ptr child, int &width, int &height);
		};

	/// ref counted pointer to layout
	typedef cgv::data::ref_ptr<layout> layout_ptr;


	}
}


#include <cgv/config/lib_end.h>