#include "list.h"

namespace cgv {
	namespace signal {

		void abst_managed_list::finish_create(size_t index, int as_copy_of) {
			for (auto attachment_ptr : attachments)
				attachment_ptr->create(as_copy_of);
			on_create(index, as_copy_of);
		}
		void abst_managed_list::start_erase(size_t index) {
			on_erase(index);
			for (auto attachment_ptr : attachments)
				attachment_ptr->erase(index);
		}
		abst_managed_list::abst_managed_list(const std::string& _element_name) : element_name(_element_name) {}
		void abst_managed_list::attach_list(abst_managed_list& attachment) { attachments.insert(&attachment); }
		void abst_managed_list::detach_list(abst_managed_list& attachment) { attachments.erase(&attachment); }
		const void* abst_managed_list::get_element_ptr(size_t index) const { return 0; }
		std::string abst_managed_list::get_element_name(size_t index) const { return element_name + " " + cgv::utils::to_string(index); }
		std::string* abst_managed_list::get_element_name_ptr(size_t index) { return 0; }
	}
}
