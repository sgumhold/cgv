
// C++ STL
#include <iostream>
#include <fstream>
#include <algorithm>
#include <utility>

// CGV framework core
#include <cgv/base/base.h>
#include <cgv/base/register.h>
#include <cgv/utils/file.h>
#include <cgv/utils/dir.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/advanced_scan.h>

// implemented header
#include "traj_loader.h"


////
// Local types and variables

// anonymous namespace begin
namespace {

namespace type_str {
	static const std::string REAL       = ":REAL";
	static const std::string VEC2       = ":VEC2";
	static const std::string VEC3       = ":VEC3";
	static const std::string VEC4       = ":VEC4";
	static const std::string COLOR_RGB  = ":RGB";
	static const std::string ERROR_TYPE = ":ERROR_TYPE";
};

// trajectory handler registry
template <class flt_type>
struct trajectory_handler_registry : public cgv::base::base, public cgv::base::registration_listener
{
	static std::vector<cgv::base::base_ptr>& handlers (void)
	{
		static std::vector<cgv::base::base_ptr> _handlers;
		return _handlers;
	}
	std::string get_type_name() const { return "trajectory_handler_registry"; }

	void register_object (cgv::base::base_ptr object, const std::string &)
	{
		if (object->get_interface<traj_format_handler<flt_type> >())
			handlers().push_back(object);
	}
	void unregister_object(cgv::base::base_ptr object, const std::string &)
	{
		for (unsigned i=0; i<handlers().size(); i++)
		{
			if (object == handlers()[i])
				handlers().erase(handlers().begin()+i);
		}
	}
};

// Anonymous namespace end
}


////
// Class implementation - traj_attribute

template <class flt_type>
traj_attribute<flt_type>::container_base::~container_base()
{}

template <class flt_type>
traj_attribute<flt_type>::traj_attribute (unsigned components) : data(nullptr)
{
	switch (components)
	{
		case 1:
			_type = REAL;
			data = new container<real>();
			return;
		case 2:
			_type = VEC2;
			data = new container<Vec2>();
			return;
		case 3:
			_type = VEC3;
			data = new container<Vec3>();
			return;
		case 4:
			_type = VEC4;
			data = new container<Vec4>();
			return;

		default:
			/* DoNothing() */;
	}
}

template <class flt_type>
traj_attribute<flt_type>::traj_attribute (std::vector<real> &&source)
	: _type(REAL), data(nullptr)
{
	data = new container<real>(std::move(source));
}

template <class flt_type>
traj_attribute<flt_type>::traj_attribute (std::vector<Vec2> &&source)
	: _type(VEC2), data(nullptr)
{
	data = new container<Vec2>(std::move(source));
}

template <class flt_type>
traj_attribute<flt_type>::traj_attribute (std::vector<Vec3> &&source)
	: _type(VEC3), data(nullptr)
{
	data = new container<Vec3>(std::move(source));
}

template <class flt_type>
traj_attribute<flt_type>::traj_attribute (std::vector<Vec4> &&source)
	: _type(VEC4), data(nullptr)
{
	data = new container<Vec4>(std::move(source));
}

template <class flt_type>
traj_attribute<flt_type>::traj_attribute (std::vector<rgb> &&source)
	: _type(COLOR_RGB), data(nullptr)
{
	data = new container<rgb>(std::move(source));
}

template <class flt_type>
traj_attribute<flt_type>::~traj_attribute()
{
	if (data)
		delete data;
}

template <class flt_type>
void traj_attribute<flt_type>::append_data (const traj_attribute &other)
{
	switch (_type)
	{
		case REAL:
		{
			auto &other_data = dynamic_cast<container<real>*>(other.data)->data;
			auto &my_data = dynamic_cast<container<real>*>(data)->data;
			my_data.insert(my_data.end(), other_data.begin(), other_data.end());
			return;
		}
		case VEC2:
		{
			auto &other_data = dynamic_cast<container<Vec2>*>(other.data)->data;
			auto &my_data = dynamic_cast<container<Vec2>*>(data)->data;
			my_data.insert(my_data.end(), other_data.begin(), other_data.end());
			return;
		}
		case VEC3:
		{
			auto &other_data = dynamic_cast<container<Vec3>*>(other.data)->data;
			auto &my_data = dynamic_cast<container<Vec3>*>(data)->data;
			my_data.insert(my_data.end(), other_data.begin(), other_data.end());
			return;
		}
		case VEC4:
		{
			auto &other_data = dynamic_cast<container<Vec4>*>(other.data)->data;
			auto &my_data = dynamic_cast<container<Vec4>*>(data)->data;
			my_data.insert(my_data.end(), other_data.begin(), other_data.end());
			return;
		}
		case COLOR_RGB:
		{
			auto &other_data = dynamic_cast<container<rgb>*>(other.data)->data;
			auto &my_data = dynamic_cast<container<rgb>*>(data)->data;
			my_data.insert(my_data.end(), other_data.begin(), other_data.end());
			return;
		}
	}
}

template <class flt_type>
const std::string& traj_attribute<flt_type>::type_string (void) const
{
	switch (_type)
	{
		case REAL:
			return type_str::REAL;
		case VEC2:
			return type_str::VEC2;
		case VEC3:
			return type_str::VEC3;
		case VEC4:
			return type_str::VEC4;
		case COLOR_RGB:
			return type_str::COLOR_RGB;

		default:
			return type_str::ERROR_TYPE;
	}
}


////
// Class implementation - traj_format_handler

template <class flt_type>
struct traj_format_handler<flt_type>::Impl
{
	// types
	typedef typename traj_format_handler::Vec3 Vec3;

	// fields
	std::vector<Vec3> positions;
	attribute_map<flt_type> attribs;
	std::vector<unsigned> indices;
};

template <class flt_type>
traj_format_handler<flt_type>::traj_format_handler() : pimpl(nullptr)
{
	pimpl = new Impl;
}

template <class flt_type>
traj_format_handler<flt_type>::~traj_format_handler()
{
	if (pimpl)
		delete pimpl;
}

template <class flt_type>
void traj_format_handler<flt_type>::clear (void)
{
	// shortcut for saving one indirection
	auto &impl = *pimpl;

	// call cleanup hook
	cleanup();

	// clear all fields
	impl.positions.clear();
	impl.attribs.clear();
	impl.indices.clear();
}

template <class flt_type>
std::vector<typename traj_format_handler<flt_type>::Vec3>& traj_format_handler<flt_type>::positions (void)
{
	return pimpl->positions;
}

template <class flt_type>
typename attribute_map<flt_type>& traj_format_handler<flt_type>::attributes (void)
{
	return pimpl->attribs;
}

template <class flt_type>
std::vector<unsigned>& traj_format_handler<flt_type>::indices (void)
{
	return pimpl->indices;
}

template <class flt_type>
const std::vector<typename traj_format_handler<flt_type>::Vec3>& traj_format_handler<flt_type>::positions(void) const
{
	return pimpl->positions;
}

template <class flt_type>
const attribute_map<flt_type>& traj_format_handler<flt_type>::attributes (void) const
{
	return pimpl->attribs;
}

template <class flt_type>
const std::vector<unsigned>& traj_format_handler<flt_type>::indices (void) const
{
	return pimpl->indices;
}


////
// Class implementation - traj_loader

template <class flt_type>
struct traj_manager<flt_type>::Impl
{
	// types
	typedef typename traj_manager::real real;
	typedef typename traj_manager::Vec2 Vec2;
	typedef typename traj_manager::Vec3 Vec3;
	typedef typename traj_manager::Vec4 Vec4;
	typedef typename traj_manager::rgb rgb;

	// helper classes
	class traj_dataset
	{
		friend class traj_manager<flt_type>;
		friend struct traj_manager<flt_type>::Impl;
	private:
		std::string name;
		std::vector<Vec3> positions;
		attribute_map<flt_type> attribs;
		std::vector<unsigned> indices;
		visual_attribute_mapping visual_map;
	public:
		traj_dataset(const traj_dataset &other)
			: name(other.name), positions(other.positions), attribs(other.attribs), indices(other.indices),
			  visual_map(other.visual_map)
		{}
		traj_dataset(traj_dataset &&other)
			: name(std::move(other.name)), positions(std::move(other.positions)), attribs(std::move(other.attribs)),
			  indices(std::move(other.indices)), visual_map(std::move(other.visual_map))
		{}
		traj_dataset(const std::string &name) : name(name) {}
		traj_dataset& operator= (const traj_dataset &other)
		{
			name = other.name;
			positions = other.positions;
			attribs = other.attribs;
			indices = other.indices;
			visual_map = other.visual_map;
			return *this;
		}
		traj_dataset& operator= (traj_dataset &&other)
		{
			name = std::move(other.name); other.name.clear();
			positions = std::move(other.positions); other.positions.clear();
			attribs = std::move(other.attribs); other.attribs.clear();
			indices = std::move(other.indices); other.indices.clear();
			visual_map = std::move(other.visual_map); other.visual_map.clear();
			return *this;
		}
	};

	// fields
	std::vector<std::unique_ptr<traj_dataset> > datasets;
	render_data rd;
	bool dirty = true;

	// helper methods
	Impl()
	{}
	~Impl()
	{}
	static const bool find_visual_attrib (std::string *out, const visual_attribute_mapping &map, VisualAttrib attrib)
	{
		auto it = map.find(attrib);
		if (it != map.end())
		{
			*out = it->second;
			return true;
		}
		else
			return false;
	}
};

template <class flt_type>
traj_manager<flt_type>::traj_manager() : pimpl(nullptr)
{
	pimpl = new Impl;
}

template <class flt_type>
traj_manager<flt_type>::~traj_manager()
{
	if (pimpl)
		delete pimpl;
}

template <class flt_type>
bool traj_manager<flt_type>::can_load (const std::string &path) const
{
	// shortcut for saving one indirection
	auto &impl = *pimpl;

	// handle path pointing to a directory
	if (cgv::utils::dir::exists(path))
	{
		// not supported yet!
		std::cout << "traj_loader: WARNING - loading a directory is not yet supported!" << std::endl << std::endl;
		return false;
	}

	// assume it's a file that can just be opened
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cout << "traj_loader: cannot open file:" << std::endl << '"'<<path<<'"' << std::endl << std::endl;
		return false;
	}

	// test if we find a suitable handler
	auto &handlers = trajectory_handler_registry<real>::handlers();
	for (auto &h : handlers)
		if (h->get_interface<traj_format_handler<flt_type> >()->can_handle(file))
			// yes we can...
			return true;
	// no we can't...
	return false;
}

template <class flt_type>
bool traj_manager<flt_type>::load (const std::string &path)
{
	// shortcut for saving one indirection
	auto &impl = *pimpl;

	// handle path pointing to a directory
	if (cgv::utils::dir::exists(path))
	{
		// not supported yet!
		std::cout << "traj_loader: WARNING - loading a directory is not yet supported!" << std::endl << std::endl;
		return false;
	}

	// assume it's a file that can just be opened
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cout << "traj_loader: cannot open file:" << std::endl << '"'<<path<<'"' << std::endl << std::endl;
		return false;
	}

	// delegate actual loading to first suitable handler
	auto &handlers = trajectory_handler_registry<real>::handlers();
	traj_format_handler<real> *handler = nullptr;
	for (auto &_h : handlers)
	{
		auto h = _h->get_interface<traj_format_handler<flt_type> >();
		if (h->can_handle(file))
		{
			if (h->read(file))
			{
				handler = h;
				break;
			}
		}
	}

	// post-process
	if (handler)
	{
		// add a dataset for the loaded trajectories
		std::unique_ptr<Impl::traj_dataset> new_dataset(new Impl::traj_dataset(path));
		new_dataset->positions = std::move(handler->positions());
		new_dataset->attribs = std::move(handler->attributes());
		new_dataset->indices = std::move(handler->indices());
		new_dataset->visual_map = handler->suggest_mapping();
		handler->clear();
		impl.datasets.emplace_back(std::move(new_dataset));
		impl.dirty = true; // we will need to rebuild the render data

		// done
		return true;
	}

	// indicate that we ended up not actually loading anything
	return false;
}

template <class flt_type>
void traj_manager<flt_type>::clear (void)
{
	pimpl->datasets.clear();
	pimpl->dirty = true;
}

template <class flt_type>
bool traj_manager<flt_type>::has_data (void) const
{
	return !(pimpl->datasets.empty());
}

template <class flt_type>
const typename traj_manager<flt_type>::render_data& traj_manager<flt_type>::get_render_data (void)
{
	// shortcut for saving one indirection
	auto &impl = *pimpl;

	// check if the render data needs to be rebuild
	if (impl.dirty)
	{
		impl.rd.positions.clear();
		impl.rd.tangents.clear();
		impl.rd.radii.clear();
		impl.rd.colors.clear();
		impl.rd.indices.clear();
		for (auto &d_ptr : impl.datasets)
		{
			// convencience shorthand
			auto &dataset = *d_ptr;

			// account for vao content contributed by previously processed datasets
			unsigned idx_offset = (unsigned)impl.rd.positions.size();

			// copy special attributes
			impl.rd.positions.insert(
				impl.rd.positions.end(), dataset.positions.begin(), dataset.positions.end()
			);
			std::transform(
				dataset.indices.begin(), dataset.indices.end(), std::back_inserter(impl.rd.indices),
				[idx_offset] (unsigned index) -> unsigned { return index + idx_offset; }
			);

			// copy mapped attributes
			std::string attrib_name;
			if (Impl::find_visual_attrib(&attrib_name, dataset.visual_map, VisualAttrib::TANGENT))
			{
				auto & data = dataset.attribs.find(attrib_name)->second.get_data<Vec4>();
				impl.rd.tangents.insert(impl.rd.tangents.end(), data.begin(), data.end());
			}
			if (Impl::find_visual_attrib(&attrib_name, dataset.visual_map, VisualAttrib::RADIUS))
			{
				auto &data = dataset.attribs.find(attrib_name)->second.get_data<real>();
				impl.rd.radii.insert(impl.rd.radii.end(), data.begin(), data.end());
			}
			if (Impl::find_visual_attrib(&attrib_name, dataset.visual_map, VisualAttrib::COLOR))
			{
				auto &data = dataset.attribs.find(attrib_name)->second.get_data<rgb>();
				impl.rd.colors.insert(impl.rd.colors.end(), data.begin(), data.end());
			}
		}
		impl.dirty = false;
	}

	// done
	return impl.rd;
}


////
// Explicit template instantiations

// Only float and double variants are intended
template class traj_attribute<float>;
template class traj_attribute<double>;
template class traj_format_handler<float>;
template class traj_format_handler<double>;
template class traj_manager<float>;
template class traj_manager<double>;


////
// Object registration

// The trajectory format handler registry
cgv::base::object_registration<trajectory_handler_registry<float> > flt_trj_registry("trajectory handler registry (float)");
cgv::base::object_registration<trajectory_handler_registry<double> > dbl_trj_registry("trajectory handler registry (double)");
