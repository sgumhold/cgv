#pragma once

#include <iostream>
#include <set>
#include <cgv/utils/convert_string.h>
#include "signal.h"
#include "lib_begin.h"

namespace cgv {
	namespace signal {

		class CGV_API abst_managed_list
		{
		public:
			std::string element_name;
		protected:
			std::set<abst_managed_list*> attachments;
			void finish_create(size_t index, int as_copy_of);
			void start_erase(size_t index);
		public:
			abst_managed_list(const std::string& _element_name);
			void attach_list(abst_managed_list& attachment);
			void detach_list(abst_managed_list& attachment);
			virtual size_t size() const = 0;
			virtual const void* get_element_ptr(size_t index) const;
			virtual std::string get_element_name(size_t index) const;
			virtual std::string* get_element_name_ptr(size_t index);
			virtual size_t create(int as_copy_of = -1) = 0;
			virtual void erase(size_t index) = 0;
			cgv::signal::signal<size_t, int> on_create;
			cgv::signal::signal<size_t> on_erase;
		};

		template <typename T>
		class managed_list : public abst_managed_list
		{
		protected:
			std::vector<T*> elements;
			std::string T::* name_member;
		public:
			managed_list(const std::string& _element_name, std::string T::* _name_member = 0) : abst_managed_list(_element_name), name_member(_name_member) {}
			size_t size() const { return elements.size(); }
			const void* get_element_ptr(size_t index) const { return &elements[index]; }
			std::string* get_element_name_ptr(size_t index) {
				if (name_member)
					return &(elements[index]->*name_member);
				return 0;
			}
			std::string get_element_name(size_t index) const {
				if (name_member == 0)
					return abst_managed_list::get_element_name(index);
				if (index >= size()) {
					std::cerr << "WARNING: attempt to get_element_name " << element_name << " of index " << index << " out of range[0, " << size() << "[. Returning base implementation." << std::endl;
					return abst_managed_list::get_element_name(index);
				}
				T* element_ptr = elements[index];
				if ((element_ptr->*name_member).empty())
					return abst_managed_list::get_element_name(index);
				return element_ptr->*name_member;
			}
			size_t create(int as_copy_of = -1) {
				T* ptr;
				if (as_copy_of == -1)
					ptr = new T();
				else {
					if (as_copy_of >= (int)size()) {
						std::cerr
							<< "WARNING: attempt to create " << element_name << " as copy of " << element_name << "  with index " << as_copy_of
							<< " out of range[0, " << size() << "[.Creating default " << element_name << " instead." << std::endl;
						ptr = new T();
					}
					else {
						ptr = new T((*this)(as_copy_of));
						if (name_member != 0) {
							if ((ptr->*name_member).empty()) {
								(ptr->*name_member) = "copy(";
								(ptr->*name_member) += cgv::utils::to_string(as_copy_of) + ")";
							}
						}
					}

				}
				size_t index = size();
				elements.push_back(ptr);
				finish_create(index, as_copy_of);
				return index;
			}
			void erase(size_t index) {
				if (index >= size()) {
					std::cerr << "WARNING: attempt to erase " << element_name << " with index " << index << " out of range [0," << size() << "[. Erase command ignored." << std::endl;
					return;
				}
				delete elements[index];
				elements[index] = 0;
				elements.erase(elements.begin() + index);
			}
			const T& operator () (size_t index) const {
				if (index >= size()) {
					std::cerr << "ERROR: attempt to access (read) " << element_name << " with index " << index << " out of range [0," << size() << "[." << std::endl;
					abort();
				}
				return *elements[index];
			}
			T& operator () (size_t index) {
				if (index >= size()) {
					std::cerr << "ERROR: attempt to access (write) " << element_name << " with index " << index << " out of range [0," << size() << "[." << std::endl;
					abort();
				}
				return *elements[index];
			}
		};

	}
}

#include <cgv/config/lib_end.h>
