#include <cgv/gui/layout_inline.h>

namespace cgv {
	namespace gui {


		layout_inline::layout_inline() 
		{

		}


		layout_inline::layout_inline(cgv::base::group_ptr container):
		layout(container)
		{

		}


		layout_inline::~layout_inline()
		{

		}


		void layout_inline::update()
		{
			// nothing to to if no container is attached
			if (container.empty())
				return;

			// std::cout<<"info: "<<get_child(0)->get

			std::cout<<"size: "<<w<<", "<<h<<std::endl;

		}
	}
}