#include <cgv/data/ascii_io_reflection_handlers.h>
#include <cgv/data/binary_io_reflection_handlers.h>
#include <cgv/base/register.h>
#include <iostream>
#include <cgv/type/reflect/debug_reflection_handler.h>
#include <cgv/type/reflect/set_reflection_handler.h>
#include <cgv/type/reflect/get_reflection_handler.h>
#include <cgv/type/reflect/reflect_enum.h>
#include <cgv/type/reflect/reflect_string.h>
#include <cgv/type/reflect/reflect_extern.h>
#include <cgv/utils/scan_enum.h>
#include <cgv/type/info/type_name.h>

#include <cgv/type/cond/is_base_of.h>

#include <cgv/math/vec.h>
#include <cgv/math/fvec.h>
#include <cgv/media/color.h>


using namespace cgv::type::reflect;

template <typename T, cgv::type::uint32_type N>
struct fvec_reflect : public cgv::math::fvec<T,N>
{
	bool self_reflect(reflection_handler& rh)
	{
		rh.reflect_member("coords", v);
	}
};

template <typename T, cgv::type::uint32_type N>
extern_reflection_traits<cgv::math::fvec<T,N>, fvec_reflect<T,N> > get_reflection_type(fvec_reflect<T,N>&) { return extern_reflection_traits<cgv::math::fvec<T,N>, fvec_reflect<T,N> >(); }

enum PointType { PT_REGULAR, PT_HOMOGENEUOS };

// example that reflects class in external function
struct point_type
{
	double x,y;
	unsigned id;
	PointType pt;
	std::vector<int> flags;

	point_type(unsigned nr = 0)
	{
		x = y = 0;
		id = nr;
		pt = PT_REGULAR;
		for (unsigned i=0; i<nr; ++i)
			flags.push_back(i);
	}
	bool operator == (const point_type& p) const 
	{
		return x == p.x &&
			y == p.y &&
			id == p.id &&
			pt == p.pt &&
			flags == p.flags;
	}
};

template <PointType PT>
struct XX;

template <> struct XX<PT_REGULAR>
{
};

// example that reflects class through string conversion
struct line_type
{
	double x[2], y[2];
	line_type() { x[0] = x[1] = y[0] = y[1] = 0; }
	bool operator == (const line_type& l) const 
	{
		return x[0] == l.x[0] &&
			x[1] == l.x[1] &&
			y[0] == l.y[0] &&
			y[1] == l.y[1];
	}
	bool operator != (const line_type& l) const { return ! operator == (l); }
	friend std::ostream& operator << (std::ostream& os, const line_type& lt);
	friend std::istream& operator >> (std::istream& is, line_type& lt);
};

std::ostream& operator << (std::ostream& os, const line_type& lt)
{
	return os << "(" << lt.x[0] << "," << lt.y[0] << ")->(" << lt.x[1] << "," << lt.y[1] << ")";
}

std::istream& operator >> (std::istream& is, line_type& lt)
{
	is.get();
	is >> lt.x[0];
	is.get();
	is >> lt.y[0];
	is.get();
	is.get();
	is.get();
	is.get();
	is >> lt.x[1];
	is.get();
	is >> lt.y[1];
	is.get();
	return is;
}

// example that reflects class through internal method
struct tgl_type : public point_type, public self_reflection_tag
{
	int i;
	PointType pts[3];
	std::vector<line_type> edges;
	std::string description;
	tgl_type() : point_type(2)
	{
		i = 2;
		pts[0] = pts[1] = PT_REGULAR;
		pts[2] = PT_HOMOGENEUOS;
		edges.push_back(line_type());
		edges.push_back(line_type());
		edges.push_back(line_type());
		description = "triangle";
	}
	bool operator == (const tgl_type& t) const 
	{
		return i == t.i &&
			pts[0] == t.pts[0] &&
			pts[1] == t.pts[1] &&
			pts[2] == t.pts[2] &&
			edges  == t.edges &&
			description == t.description &&
			(const point_type&)(*this) == (const point_type&) t;
	}
	bool self_reflect(reflection_handler& rh)
	{
		return rh.reflect_base(static_cast<point_type&>(*this)) &&
			   rh.reflect_member("description",description) &&
			   rh.reflect_member("i",i) &&
			   rh.reflect_member("pts",pts) &&
			   rh.reflect_member("edges", edges);
	}
};


struct point_type_reflect : public point_type
{
	bool self_reflect(reflection_handler& rh)
	{
		return rh.reflect_member("x", x) &&
			   rh.reflect_member("y", y) &&
			   rh.reflect_member("id", id) &&
			   rh.reflect_member("pt", pt) &&
			   rh.reflect_member("flags", flags);
	}
};

struct line_type_reflect : public line_type
{
	bool self_reflect(reflection_handler& rh)
	{
		return rh.reflect_member("x", x) &&
			   rh.reflect_member("y", y);
	}
};

enum_reflection_traits<PointType> get_reflection_traits(const PointType&) { 
	return enum_reflection_traits<PointType>("PT_REGULAR, PT_HOMOGENEUOS"); 
}

extern_reflection_traits<point_type, point_type_reflect> get_reflection_traits(const point_type&) { return extern_reflection_traits<point_type, point_type_reflect>(); }

extern_string_reflection_traits<line_type, line_type_reflect> get_reflection_traits(const line_type&) { return extern_string_reflection_traits<line_type, line_type_reflect>(); }


bool test_self_reflect()
{
	double dbl;
	unsigned uint;
	point_type p(3);
	line_type l;
	tgl_type t;

	set_member(t, "y", 2.0);
	TEST_ASSERT_EQ(t.y, 2.0);

	get_member(t, "y", dbl);
	TEST_ASSERT_EQ(t.y, dbl);


	set_member(t, "point_type.x", 1.0);
	TEST_ASSERT_EQ(t.x, 1.0);

	get_member(t, "point_type.x", dbl);
	TEST_ASSERT_EQ(t.x, dbl);


	set_member(t, "edges[2]", std::string("(-1,0)->(1,2)"));
	TEST_ASSERT_EQ(t.edges[2].y[1], 2.0);

	get_member(t, "edges[2]", l);
	TEST_ASSERT_EQ(t.edges[2], l);


	set_member(t, "edges.size", 4);
	TEST_ASSERT_EQ(t.edges.size(), 4);

	get_member(t, "edges.size", uint);
	TEST_ASSERT_EQ(t.edges.size(), uint);

	std::string str;
	set_member(t, "description", std::string("\ttab\n\t\r\b\"bell'"));
	TEST_ASSERT_EQ(t.description, "\ttab\n\t\r\b\"bell'");

	get_member(t, "description", str);
	TEST_ASSERT_EQ(t.description, str);

	set_member(p, "pt", PT_HOMOGENEUOS);
	TEST_ASSERT_EQ(p.pt, PT_HOMOGENEUOS);

	PointType pt;
	get_member(t, "pts[2]", str);
	get_member(t, "pts[2]", pt);

	std::stringstream ss;
	cgv::data::ascii_write_reflection_handler wsrh(ss, "triangle", 1001);
	wsrh.reflect_member("T", t);
	wsrh.close();

	std::stringstream ss1(ss.str());
	tgl_type t1;
	cgv::data::ascii_read_reflection_handler rsrh(ss1, "triangle", 1001);
	rsrh.reflect_member("T", t1);
	rsrh.close();

	TEST_ASSERT(t == t1);

	cgv::data::binary_write_reflection_handler bwsrh("e:\\temp\\temp.bin", "triangle", 1001);
	bwsrh.reflect_member("T", t);
	bwsrh.close();

	tgl_type t2;
	cgv::data::binary_read_reflection_handler brsrh("e:\\temp\\temp.bin", "triangle", 1001);
	brsrh.reflect_member("T", t2);
	brsrh.close();

	TEST_ASSERT(t == t2);


/*
	debug_reflection_handler srh;

	srh.reflect_member("t", t);
	srh.reflect_member("t1", t1);
	std::cout << srh.output << std::endl;

	std::cout << ss.str() << std::endl;
	*/

	return true;
}

#include <test/lib_begin.h>

extern CGV_API cgv::base::test_registration test_cb_self_reflect_reg("cgv::type::reflect::test_self_reflect", test_self_reflect);