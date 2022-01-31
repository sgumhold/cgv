#include "dispatch_info.h"

namespace cgv {
	namespace nui {

		dispatch_info::dispatch_info(hid_identifier _hid_id, dispatch_mode _mode)
			: hid_id(_hid_id), mode(_mode)
		{
		}
		dispatch_info::~dispatch_info()
		{
		}
		void dispatch_info::copy(const dispatch_info& dis_info)
		{
			*this = dis_info;
		}
		dispatch_info* dispatch_info::clone() const
		{
			return new dispatch_info(*this);
		}
	}
}
