#include <cgv/base/register.h>
#include <cgv/data/component_format.h>
#include <cgv/data/data_format.h>
#include <cgv/utils/convert.h>

using namespace cgv::base;
using namespace cgv::data;
using cgv::utils::to_string;

bool test_packing_info()
{ 
	packing_info pi1;
	packing_info pi2(1,7);
	packing_info pi3(1,2,3,4,5);
	packing_info pi4(3);
	TEST_ASSERT(pi1.align(3,8) == 8)
	TEST_ASSERT(!pi1.is_packing() && pi2.is_packing() && pi3.is_packing() && !pi4.is_packing())
	TEST_ASSERT(pi1.get_component_alignment() == 1 && 
	            pi2.get_component_alignment() == 1 && 
					pi3.get_component_alignment() == 1 && 
					pi4.get_component_alignment() == 3)
	TEST_ASSERT(pi1.get_bit_depth(0) == 0 &&
	            pi1.get_bit_depth(1) == 0 &&
					pi1.get_bit_depth(2) == 0 &&
					pi1.get_bit_depth(3) == 0)
	TEST_ASSERT(pi2.get_bit_depth(0) == 7 &&
	            pi2.get_bit_depth(1) == 7 &&
					pi2.get_bit_depth(2) == 7 &&
					pi2.get_bit_depth(3) == 7)
	TEST_ASSERT(pi3.get_bit_depth(0) == 2 &&
	            pi3.get_bit_depth(1) == 3 &&
					pi3.get_bit_depth(2) == 4 &&
					pi3.get_bit_depth(3) == 5)
	TEST_ASSERT(pi4.get_bit_depth(0) == 0 &&
	            pi4.get_bit_depth(1) == 0 &&
					pi4.get_bit_depth(2) == 0 &&
					pi4.get_bit_depth(3) == 0)
	unsigned char c[4] = { 0x3a, 0x5f, 0xf1, 0xec };
	TEST_ASSERT_EQ(pi2.get_unsigned(0, c), 0x3a)
	TEST_ASSERT_EQ(pi2.get_unsigned(1, c), 0x3e)
	return true;
}
bool test_component_format()
{
	const char* cfd[] = {
		"uint8:3|4[R,G,B,A]",
		"uint8[R:5,G:6,B:5]",
		"flt32[px,py]",
		0
	};
	unsigned int i=0;
	do {
		TEST_ASSERT_EQ(to_string(component_format(cfd[i])), cfd[i])
		++i;
	} while (cfd[i]);

	component_format cf1(TI_UINT16, "R,G,B,A");
	component_format cf2(TI_UINT16, "R,G,B", 12, 10, 12, 10);
	component_format cf3(TI_INT8, CF_P3D);
	component_format cf4("flt16[L,A]");
	component_format cf5("uint8:5|6[Y,U,V]");
	TEST_ASSERT_EQ(cf1, component_format("uint16[R,G,B,A]"));
	TEST_ASSERT_EQ(cf1.get_nr_components(), 4);
	TEST_ASSERT_EQ(cf2.get_nr_components(), 3);
	TEST_ASSERT_EQ(cf3.get_nr_components(), 3);
	TEST_ASSERT_EQ(cf4.get_nr_components(), 2);
	TEST_ASSERT_EQ(cf5.get_nr_components(), 3);
	TEST_ASSERT_EQ(std::string(cf1.get_component_name(0)), "R");
	TEST_ASSERT_EQ(std::string(cf2.get_component_name(0)), "R");
	TEST_ASSERT_EQ(std::string(cf3.get_component_name(0)), "px");
	TEST_ASSERT_EQ(std::string(cf4.get_component_name(0)), "L");
	TEST_ASSERT_EQ(std::string(cf5.get_component_name(0)), "Y");

	TEST_ASSERT_EQ(to_string(component_format("uint16[R,G,B,A]")), "uint16[R,G,B,A]");
	TEST_ASSERT_EQ(cf1.get_standard_component_format(), CF_RGBA);
	TEST_ASSERT_EQ(cf2.get_standard_component_format(), CF_RGB);
	TEST_ASSERT_EQ(cf3.get_standard_component_format(), CF_P3D);
	TEST_ASSERT_EQ(cf4.get_standard_component_format(), CF_LA);
	TEST_ASSERT_EQ(cf5.get_standard_component_format(), CF_UNDEF);
	TEST_ASSERT_EQ(component_format(TI_UINT8,CF_T4D).get_standard_component_format(), CF_T4D);

	return true;
}

bool test_data_format()
{
	const char* dfd[] = {
		"int8[R,G,B](277|4,233)",
		"flt32[px,py,pz](256,256,128,32)",
		"uint8[R,G,B](127|8,256)",
		"uint8[R,G,B](127:1,256:0)",
		"uint16[L:12,A:12]|4(32,32,32)",
		0
	};
	unsigned int i=0;
	do {
		TEST_ASSERT_EQ(to_string(data_format(dfd[i])), dfd[i])
		++i;
	} while (dfd[i]);
	return true;
}

enum Endian
{
	E_LITTLE = 0,
	E_BIG_32 = 1,
	E_BIG_64 = 2
};

Endian get_endian()
{
	uint64_t i = 0x0200000001000000L;
	return Endian(reinterpret_cast<uint8_t&>(i));
}

template <Endian src, Endian tar, typename T>
void convert_endian(T& value)
{
	if (src == tar)
		return;
	if (src > E_LITTLE && tar > E_LITTLE) {
		if (sizeof(T) == 8) {
			uint32_t* ui32_ptr = reinterpret_cast<uint32_t*>(&value);
			std::swap(ui32_ptr[0], ui32_ptr[1]);
		}
	}
	else {
		if ((src == E_BIG_32 || tar == E_BIG_32) && (sizeof(T) == 8)) {
			uint32_t* ui32_ptr = reinterpret_cast<uint32_t*>(&value);
			convert_endian<src,tar>(ui32_ptr[0]);
			convert_endian<src,tar>(ui32_ptr[1]);
		}
		else {
			uint8_t* byte_ptr = reinterpret_cast<uint8_t*>(&value);
			for (size_t i = 0; 2 * i < sizeof(T); ++i)
				std::swap(byte_ptr[i], byte_ptr[sizeof(T) - i - 1]);
		}
	}
}

template <typename T>
void show_byte_order(T& value)
{
	uint8_t* byte_ptr = reinterpret_cast<uint8_t*>(&value);
	for (size_t i = 0; i < sizeof(T); ++i) {
		if (i > 0)
			std::cout << ", ";
		std::cout << int(byte_ptr[i]);
	}
	std::cout << std::endl;
}

bool test_data_endian()
{
	uint16_t i16 = 0x0201;
	uint32_t i32 = 0x04030201;
	uint64_t i64 = 0x0807060504030201L;
	/*
	show_byte_order(i16);
	swap_endian(i16);
	show_byte_order(i16);
	swap_endian(i16);
	show_byte_order(i16);

	show_byte_order(i32);
	swap_endian(i32);
	show_byte_order(i32);
	swap_endian(i32);
	show_byte_order(i32);

	show_byte_order(i64);
	swap_endian(i64);
	show_byte_order(i64);
	swap_endian(i64);
	show_byte_order(i64);
	*/

	std::cout << "\n";
	show_byte_order(i64);
	convert_endian<E_LITTLE, E_BIG_32>(i64); show_byte_order(i64);
	convert_endian<E_BIG_32, E_LITTLE>(i64); show_byte_order(i64);
	convert_endian<E_LITTLE, E_BIG_64>(i64); show_byte_order(i64);
	convert_endian<E_BIG_64, E_LITTLE>(i64); show_byte_order(i64);
	convert_endian<E_BIG_32, E_BIG_64>(i64); show_byte_order(i64);
	convert_endian<E_BIG_64, E_BIG_32>(i64); show_byte_order(i64);

	Endian e = get_endian();
	switch (e) {
	case E_LITTLE: std::cout << "have little" << std::endl; break;
	case E_BIG_32: std::cout << "have big 32" << std::endl; break;
	case E_BIG_64: std::cout << "have big 64" << std::endl; break;
	}
	return true;
}

#include <test/lib_begin.h>

extern CGV_API test_registration packing_info_test_registration(
	"cgv::base::packing_info", test_packing_info);

extern CGV_API test_registration component_format_test_registration(
	"cgv::base::component_format", test_component_format);

extern CGV_API test_registration data_format_test_registration(
	"cgv::base::data_format", test_data_format);

extern CGV_API test_registration data_endian_test_registration(
	"cgv::base::data_format", test_data_endian);