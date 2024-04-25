#include <math.h>
#include <cgv/math/interval.h>
#include <cgv/base/register.h>

using namespace cgv::base;
using namespace cgv::math;

bool test_interval()
{
	interval<float> I1(1.3f, -0.9f);
	TEST_ASSERT(I1.contains(0));
	TEST_ASSERT(I1.contains(1.3f));
	TEST_ASSERT(I1.contains(-0.9f));
	interval<float> I2(0.3f, -1.9f);
	TEST_ASSERT_EQ(I1.intersection(I2).lower_bound, -0.9f);
	TEST_ASSERT_EQ(I1.intersection(I2).upper_bound, 0.3f);
	TEST_ASSERT_EQ(I1*2.0f, interval<float>(-1.8f, 2.6f));
	TEST_ASSERT(fabs((I1*I2).intersect(interval<float>(-2.47f, 1.71f)).size() - (I1*I2).size()) < 1e-6f);
	return true;
}

#include <test/lib_begin.h>

extern CGV_API test_registration test_cb_interval_reg("cgv::math::interval", test_interval);
