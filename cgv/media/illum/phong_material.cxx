#include "phong_material.h"

namespace cgv {
	namespace media { 
		namespace illum { 

		const phong_material& phong_material::get_default() {
			static phong_material mat; return mat;
		}

		} 
	}
}
