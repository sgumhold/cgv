#include <cgv/gui/layout.h>
#include <cgv/gui/resizable.h>

namespace cgv {
	namespace gui {

		// constructor that sets a container
		layout::layout(cgv::base::group_ptr container)
		{
			w = 0;
			h = 0;
			min_w = 0;
			min_h = 0;
			default_w = 0;
			default_h = 0;
			spacings_name = "compact";
			spacings = get_layout_spacings(spacings_name);
			set_container(container);
		}


		// empty destructor
		layout::~layout() 
		{
		}


		// set a container to be layouted
		void layout::set_container(cgv::base::group_ptr container)
		{
			if (!container) {
				std::cerr<<"Error: A container was NULL (maybe an unsuccessfull dynamic_cast?)!"<<std::endl;
				return;
			}

			this->container = container;	
		}


		// get the layouted container
		cgv::base::group_ptr layout::get_container()
		{
			return container;
		}


		// get a child from the container. make sure that the child can be resized
		base_ptr layout::get_child(unsigned int i)
		{
			if (!container) {
				std::cerr<<"Layout is not connected to any container"<<std::endl;
				return 0;
			}

			return container->get_child(i);
		}


		int layout::get_child_layout_hints(cgv::base::base_ptr child)
		{
			// nothing to do if the pointer is unset
			if (child.empty())
				return 0;

			// get information via the aligmnent string
			std::string align("");
			align = child->get<std::string>("alignment");

			int info = 0;

			for (unsigned int i=0; i<align.length(); i++)
				switch(align[i]) {
					case 'x': 
						info |= LH_HEXPAND;
						break;
					case 's':
						info |= LH_HSHRINK;
						break;
					case 'f':
						info |= LH_HFILL;
						break;
					case 'X':
						info |= LH_VEXPAND;
						break;
					case 'S':
						info |= LH_VSHRINK;
						break;
					case 'F':
						info |= LH_VFILL;
						break;
					case 'l':
						info |= LH_LEFT;
						break;
					case 'r':
						info |= LH_RIGHT;
						break;
					case 'c':
						info |= LH_CENTER;
						break;
					case 't':
						info |= LH_TOP;
						break;
					case 'b':
						info |= LH_BOTTOM;
						break;
					case 'm':
						info |= LH_MIDDLE;
						break;
			}

			return info;
		}


		// get the size of a child
		void layout::get_child_size(const base_ptr child, int &width, int &height)
		{
			width = height = 0;

			if (!child)
				return;

			width = child->get<int>("w");
			height = child->get<int>("h");
		}


		// set the size of a child
		void layout::set_child_size(const base_ptr child, int width, int height)
		{
			if (!child)
				return;

			child->set<int>("w", width);
			child->set<int>("h", height);
			child->set<int>("dolayout", 0);
		}


		// get the default size of a child
		void layout::get_child_default_size(const base_ptr child, int &width, int &height)
		{
			width = height = 0;
			if (!child)
				return;

			width = height = 0;
			// try to get the default width and height
			width = child->get<int>("dw");
			height = child->get<int>("dh");
		}


		void layout::get_child_minimum_size(const base_ptr child, int &width, int &height)
		{
			width = height = 0;
			if (!child)
				return;

			// try to get the mimimum size of a child
			width = child->get<int>("mw");
			height = child->get<int>("mh");
		}




		// get the position of a child
		void layout::get_child_position(const base_ptr child, int &x, int &y)
		{
			x = y = 0;

			if (!child)
				return;

			x = child->get<int>("x");
			y = child->get<int>("y");
		}


		// set the position of a child
		void layout::set_child_position(const base_ptr child, int x, int y)
		{
			if (!child)
				return;

			child->set<int>("x", x);
			child->set<int>("y", y);
		}




		void layout::resize(int w, int h)
		{
			this->w = w;
			this->h = h;

			update();
		}


		std::string layout::get_property_declarations()
		{
			return "spacings:string";
		}


		bool layout::set_void(const std::string &property, const std::string &value_type, const void *value_ptr)
		{
			if (property == "spacings") {
				cgv::type::get_variant(spacings_name, value_type, value_ptr);
				spacings = get_layout_spacings(spacings_name);
				update();
			} else
			if (property == "mw") 
				cgv::type::get_variant(min_w, value_type, value_ptr);
			else
			if (property == "mh")
				cgv::type::get_variant(min_h, value_type, value_ptr);
			else
			if (property == "dw")
				cgv::type::get_variant(default_w, value_type, value_ptr);
			else
			if (property == "dh")
				cgv::type::get_variant(default_h, value_type, value_ptr);
			else
				return false;

			return true;
		}


		bool layout::get_void(const std::string &property, const std::string &value_type, void *value_ptr)
		{
			if (property == "spacings")
				cgv::type::set_variant(spacings_name, value_type, value_ptr);
			else
			if (property == "mw")
				cgv::type::set_variant(min_w, value_type, value_ptr);
			else
			if (property == "mh")
				cgv::type::set_variant(min_h, value_type, value_ptr);
			else
			if (property == "dw")
				cgv::type::set_variant(default_w, value_type, value_ptr);
			else
			if (property == "dh")
				cgv::type::set_variant(default_h, value_type, value_ptr);
			else
				return false;

			return true;
		}
	}
}