#include <cgv/base/register.h>
#include <cgv/type/variant.h>

using namespace cgv::base;
using namespace cgv::type;

bool test_variant()
{
	bool        a,  b   = true;
	int8_type   j8, i8  = -123;
	int16_type  j16,i16 = -12345;
	int32_type  j32,i32 = -1234567890;
	int64_type  j64,i64 = -8901234567890123456;
	uint8_type  v8, u8  = 234;
	uint16_type v16,u16 = 45678;
	uint32_type v32,u32 = 3456789012;
	uint64_type v64,u64 = 12345678901234567890;
	float       g32,f32 = -1.23456e-37f;
	double      g64,f64 = 1.2345678901234e299;
	std::string t,  s   = "hello world";

	// test variant get method
	TEST_ASSERT_EQ(b,   variant<bool>::get("bool", &b))
	TEST_ASSERT_EQ(i8,  variant<int8_type>::get("int8", &i8))
	TEST_ASSERT_EQ(i16, variant<int16_type>::get("int16", &i16))
	TEST_ASSERT_EQ(i32, variant<int32_type>::get("int32", &i32))
	TEST_ASSERT_EQ(i64, variant<int64_type>::get("int64", &i64))
	TEST_ASSERT_EQ(u8,  variant<uint8_type>::get("uint8", &u8))
	TEST_ASSERT_EQ(u16, variant<uint16_type>::get("uint16", &u16))
	TEST_ASSERT_EQ(u32, variant<uint32_type>::get("uint32", &u32))
	TEST_ASSERT_EQ(u64, variant<uint64_type>::get("uint64", &u64))
	TEST_ASSERT_EQ(f32, variant<flt32_type>::get("flt32", &f32))
	TEST_ASSERT_EQ(f64, variant<flt64_type>::get("flt64", &f64))
	TEST_ASSERT_EQ(s,   variant<std::string>::get("string", &s))

	TEST_ASSERT_EQ(i8,  variant<int16_type>::get("int8", &i8))
	TEST_ASSERT_EQ(i8,  variant<int32_type>::get("int8", &i8))
	TEST_ASSERT_EQ(i8,  variant<int64_type>::get("int8", &i8))
	TEST_ASSERT_EQ(u8,  variant<uint16_type>::get("uint8", &u8))
	TEST_ASSERT_EQ(u8,  variant<uint32_type>::get("uint8", &u8))
	TEST_ASSERT_EQ(u8,  variant<uint64_type>::get("uint8", &u8))
	TEST_ASSERT_EQ(variant<std::string>::get("int8", &i8), "-123")
	TEST_ASSERT_EQ(variant<std::string>::get("uint8", &u8), "234")
	TEST_ASSERT_EQ(variant<std::string>::get("uint16", &u16), "45678")

	// test variant set method
	set_variant(a, "bool", &b); TEST_ASSERT_EQ(a,b)
	set_variant(j8, "int8", &i8); TEST_ASSERT_EQ(i8,j8)
	set_variant(j16, "int16", &i16); TEST_ASSERT_EQ(i16,j16)
	set_variant(j32, "int32", &i32); TEST_ASSERT_EQ(i32,j32)
	set_variant(j64, "int64", &i64); TEST_ASSERT_EQ(i64,j64)
	set_variant(v8, "uint8", &u8); TEST_ASSERT_EQ(u8,v8)
	set_variant(v16, "uint16", &u16); TEST_ASSERT_EQ(u16,v16)
	set_variant(v32, "uint32", &u32); TEST_ASSERT_EQ(u32,v32)
	set_variant(v64, "uint64", &u64); TEST_ASSERT_EQ(u64,v64)
	set_variant(g32, "flt32", &f32); TEST_ASSERT_EQ(g32,f32)
	set_variant(g64, "flt64", &f64); TEST_ASSERT_EQ(g64,f64)
	set_variant(t, "string", &s); TEST_ASSERT_EQ(s,t)

	return true;
}

#include <test/lib_begin.h>

extern CGV_API test_registration variant_test_registration(
	"cgv::type::variant", test_variant);
