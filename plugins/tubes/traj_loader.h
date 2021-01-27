#pragma once

// C++ STL
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

// CGV framework core
#include <cgv/base/base.h>
#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/media/color.h>


/// forward declaration of the main trajectory loader class
template <class flt_type>
class traj_manager;


/// enumeration of known visual attributes
enum class VisualAttrib
{
	TANGENT, RADIUS, COLOR
};

/// a map of what visual attributes correspond to what data attributes
typedef std::unordered_map<VisualAttrib, std::string> visual_attribute_mapping;


/// RAII-type helper that trajectory format handlers can use to ensure reset of the stream position after
/// can_handle() queries
struct stream_pos_guard
{
	/// the stream to guard
	std::istream &stream;

	/// the position the stream will be reset to
	const std::istream::pos_type g;

	/// construct the guard for the given stream
	stream_pos_guard(std::istream& stream) : stream(stream), g(stream.tellg())
	{}

	/// the destructor
	~stream_pos_guard() { stream.seekg(g); }
};


/// class encapsulating an attribute type
template<class flt_type>
class traj_attribute
{

public:

	/// real number type
	typedef flt_type real;

	/// 2D vector type
	typedef cgv::math::fvec<real, 2> Vec2;

	/// 3D vector type
	typedef cgv::math::fvec<real, 3> Vec3;

	/// 4D vector type
	typedef cgv::math::fvec<real, 4> Vec4;

	/// color type
	typedef cgv::media::color<float, cgv::media::RGB, cgv::media::NO_ALPHA> rgb;

	/// enumerates all supported attribute types
	enum Type
	{
		REAL, VEC2, VEC3, VEC4, COLOR_RGB
	};

	/// base container class for having a common cleanup hook
	struct container_base
	{
		/// virtual base destructor - causes vtable creation
		virtual ~container_base();
	};

	/// generic container type for storing the actual attribute data
	template <class T>
	struct container : public container_base
	{
		/// actual element type
		typedef T elem_type;

		/// attribute data vector
		std::vector<elem_type> data;

		/// construct with the given actual data
		container(std::vector<elem_type> &&data)
			: data(std::move(data))
		{}
	};


protected:

	/// type of the represented attribute
	Type _type;

	/// opaque data
	container_base *data;


public:

	/// construct with scalar attribute data
	traj_attribute(std::vector<real> &&source);

	/// construct with 2D vector attribute data
	traj_attribute(std::vector<Vec2> &&source);

	/// construct with 3D vector attribute data
	traj_attribute(std::vector<Vec3> &&source);

	/// construct with 4D vector attribute data
	traj_attribute(std::vector<Vec4> &&source);

	/// construct with RGB color attribute data
	traj_attribute(std::vector<rgb> &&source);

	/// the destructor
	~traj_attribute();

	/// report type of the stored attribute
	Type type (void) const { return _type; }

	/// access the attribute data as if it was of the specified type
	template <class T>
	std::vector<T>& get_data (void);

	/// access the attribute data as if it was of scalar type
	template <>
	std::vector<real>& get_data<real>(void) { return dynamic_cast<container<real>*>(data)->data; }

	/// access the attribute data as if it was of type Vec2
	template <>
	std::vector<Vec2>& get_data<Vec2>(void) { return dynamic_cast<container<Vec2>*>(data)->data; }

	/// access the attribute data as if it was of type Vec3
	template <>
	std::vector<Vec3>& get_data<Vec3>(void) { return dynamic_cast<container<Vec3>*>(data)->data; }

	/// access the attribute data as if it was of type Vec4
	template <>
	std::vector<Vec4>& get_data<Vec4>(void) { return dynamic_cast<container<Vec4>*>(data)->data; }

	/// access the attribute data as if it was of type rgb color
	template <>
	std::vector<rgb>& get_data<rgb>(void) { return dynamic_cast<container<rgb>*>(data)->data; }

	/// append data from other attribute, which must be of the same type
	void append_data (const traj_attribute &other);

	/// return a string representing the attribute data type
	const std::string& type_string (void) const;
};

/// a map of attribute names to their data
template <class flt_type>
using attribute_map = std::unordered_map<std::string, traj_attribute<flt_type> >;


/// abstract handler interface that can be implemented to support specific trajectory file formats
template <class flt_type>
class traj_format_handler : public cgv::base::base
{

	// interfacing
	friend class traj_manager<flt_type>;


public:

	/// real number type
	typedef flt_type real;

	/// 2D vector type
	typedef cgv::math::fvec<real, 2> Vec2;

	/// 3D vector type
	typedef cgv::math::fvec<real, 3> Vec3;

	/// 4D vector type
	typedef cgv::math::fvec<real, 4> Vec4;

	/// color type
	typedef cgv::media::color<float, cgv::media::RGB, cgv::media::NO_ALPHA> rgb;


private:

	/// implementation forward
	struct Impl;

	/// implementation handle
	Impl *pimpl;


protected:

	/// explicitely reset all additional fields in the state of a derived handler
	virtual void cleanup (void) = 0;

	/// write-access the "special" positions attribute
	std::vector<Vec3>& positions (void);

	/// write-access the map containing all generic attributes
	attribute_map<flt_type>& attributes (void);

	/// write-access the indices, which store connectivity with line-list semantics (i.e. pairs of indices form a trajectory segment)
	std::vector<unsigned>& indices (void);


public:

	/// default constructor
	traj_format_handler();

	/// virtual base destructor - causes vtable creation
	virtual ~traj_format_handler();

	/// Test if the given data stream can be handled by this handler. At the minimum, the handler must be able to extract sample
	/// positions from the data stream when reporting true.
	virtual bool can_handle (std::istream &contents) const = 0;

	/// parse the given stream containing the file contents to load trajectories stored in it (optionally offsetting sample indices by
	/// the specified amount) and report whether any data was loaded
	virtual bool read (std::istream &contents, unsigned idx_offset=0) = 0;

	/// explicitely reset implementation-specific state
	void clear (void);

	/// check if the handler currently stores valid loaded data
	virtual bool has_data (void) const = 0;

	/// access the "special" positions attribute
	const std::vector<Vec3>& positions (void) const;

	/// access the map containing all generic attributes
	const attribute_map<flt_type>& attributes (void) const;

	/// access the indices, which store connectivity with line-list semantics (i.e. pairs of indices form a trajectory segment)
	const std::vector<unsigned>& indices (void) const;

	/// report the average length of line segments
	virtual real avg_segment_length (void) const = 0;

	/// suggest a default visual attribute mapping inferred from the loaded data
	virtual const visual_attribute_mapping& suggest_mapping (void) const = 0;
};


/// common trajectory data loader class
template <class flt_type>
class traj_manager
{

public:

	/// real number type
	typedef flt_type real;

	/// 2D vector type
	typedef typename traj_format_handler<real>::Vec2 Vec2;

	/// 3D vector type
	typedef typename traj_format_handler<real>::Vec3 Vec3;

	/// 4D vector type
	typedef typename traj_format_handler<real>::Vec4 Vec4;

	/// color type
	typedef typename traj_format_handler<real>::rgb rgb;

	/// encapsulates data for all visual attributes for use by renderers
	struct render_data
	{
		typedef flt_type real;
		typedef typename traj_manager<real>::Vec2 Vec2;
		typedef typename traj_manager<real>::Vec3 Vec3;
		typedef typename traj_manager<real>::Vec4 Vec4;
		typedef typename traj_manager<real>::rgb rgb;

		std::vector<Vec3> positions;
		std::vector<Vec4> tangents;
		std::vector<real> radii;
		std::vector<rgb> colors;
		std::vector<unsigned> indices;
	};


private:

	/// implementation forward
	struct Impl;

	/// implementation handle
	Impl *pimpl;


public:

	/// default constructor
	traj_manager();

	/// the destructor
	~traj_manager();

	/// test if the given file or directory can be loaded
	bool can_load (const std::string &path) const;

	/// load trajectories from a file or directory and add them to the internal database, reporting if anything was loaded
	bool load (const std::string &path);

	/// reset internal trajectory database (e.g. for loading a new, unrelated set of trajectories into this existing instance)
	void clear (void);

	/// check if the loader currently stores valid loaded data
	bool has_data (void) const;

	/// returns the visual attributes of all dataset laid out in a way suitable for rendering
	const render_data& get_render_data (void);
};


/// converts 255-based 3-vectors to 1-based float RGB colors
template <class vec_type>
typename traj_format_handler<float>::rgb vec3_to_rgb (const vec_type &v)
{
	return traj_format_handler<float>::rgb(float(v.x())/255.f, float(v.y())/255.f, float(v.z())/255.f);
}

/// constructs a 4-vector from a 3-vector and a scalar
template <class vec3_type, class scalar_type>
cgv::math::fvec<typename vec3_type::value_type, 4> vec4_from_vec3s (const vec3_type& v, scalar_type s)
{
	return cgv::math::fvec<typename vec3_type::value_type, 4>(v.x(), v.y(), v.z(), vec3_type::value_type(s));
}
