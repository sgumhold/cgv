#include <cgv/base/register.h>
#include <cgv/media/text/scan.h>

using namespace cgv::base;
using namespace cgv::media::text;

bool test_scan()
{
//	std::cout << "\ntest_scan()\n==============\n" << std::endl;
	TEST_ASSERT_EQ(get_element_index(std::string("a b"),std::string("ds;khauds;a b")),2);
	TEST_ASSERT(is_element(std::string(),std::string()));
	TEST_ASSERT_EQ(get_element_index(std::string(),std::string()),0);
	TEST_ASSERT_EQ(get_element_index(std::string("a b"),std::string("ds;khauds;a b ")),-1);
	TEST_ASSERT_EQ(get_element_index(std::string("a b"),std::string("a b;ds;khauds;a b ")),0);
	TEST_ASSERT_EQ(get_element_index(std::string("a b"),std::string(";a b;a b ")),1);
	return true;
}

#include <test/lib_begin.h>

extern CGV_API test_registration test_cb_scan_reg("cgv::media::text::scan", test_scan);