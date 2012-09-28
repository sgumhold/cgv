#include <cgv/base/call_interface_impl.h>
#include <cgv/type/reflect/method_interface_impl.h>
#include <cgv/type/reflect/reflection_handler.h>
#include <cgv/base/base.h>
#include <cgv/base/register.h>
#include <cgv/type/traits/method_pointer.h>
#include <cgv/type/info/type_name.h>
#include <cgv/type/variant.h>
#include <iostream>
#include <vector>

using namespace cgv::base;
using namespace cgv::type::reflect;
/*
void set()
{
	if (get_type())
		if (use_type())
			on_set();
			return true;
	set_self_reflect();
	on_set();
}

~()
{
	enum_members_and_methods_refs
		announce_destruction
}

find_ref_manager(&)
    check_value
	value_change
	destruct

	set_value
	get_value

find_method_manager
    destruct

control
	manager m;
	my_action
		m->set_value
	on_value_change
	on_destruct
*/

struct property_test_object : public base
{
	int n;
	std::string s;
	double d;
	void inc_n() { ++n; }
	double add(double delta) { return d += delta; }
	void show() const { std::cout << "hello show" << std::endl; }
	int add_int(double v) const { return (int) (d+v); }
	property_test_object()
	{
		n = 10;
		s = "hello";
		d = 1.7;
	}
	void self_reflect(reflection_handler& srh)
	{
		srh.reflect_member("n", n) &&
		srh.reflect_member("s", s) &&
		srh.reflect_member("d", d) &&
		srh.reflect_method("inc_n", &property_test_object::inc_n) &&
		srh.reflect_method("add", &property_test_object::add) &&
		srh.reflect_method("show", &property_test_object::show) &&
		srh.reflect_method("add_int", &property_test_object::add_int);
	}
};

bool test_property()
{
	base_ptr pto(new property_test_object);
	TEST_ASSERT_EQ(pto->get_property_declarations(),
		           "n:int32;s:string;d:flt64;inc_n();add(flt64):flt64;show();add_int(flt64):int32");
	TEST_ASSERT_EQ(pto->get<int>("n"), 10);
	pto->set("n", 4);
	TEST_ASSERT_EQ(pto->get<int>("n"), 4);
	double v;
	float arg = 17.5f;
	eval(pto, "add", arg, v);
	TEST_ASSERT_EQ(pto->get<double>("d"), 1.7+17.5f);
	TEST_ASSERT_EQ(pto->get<double>("d"), v);
	exec(pto, "inc_n");
	TEST_ASSERT_EQ(pto->get<int>("n"), 5);
	int res;
	eval(pto, "add_int", arg, res); 
	TEST_ASSERT_EQ(res, 36);
	return true;
}

#include <test/lib_begin.h>

extern CGV_API test_registration test_cb_property_reg("cgv::base::test_property", test_property);