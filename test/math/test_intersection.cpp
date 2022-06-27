#include <gtest/gtest.h>

#include <cgv/math/intersection.h>

typedef cgv::math::fvec<float, 3> vec3;

TEST(ray_box_intersection, computesHitAndMissCorrectly)
{
	struct Param
	{
		vec3 origin;
		vec3 direction;
		cgv::math::RayBoxIntersectionResult<float> expected;
	};
	std::vector<Param> params = {
		  {vec3(-1, 0.5, 0.5), vec3(1, 0, 0), {true, 1, 2}},		 // hit x
		  {vec3(0.5, -1, 0.5), vec3(0, 1, 0), {true, 1, 2}},		 // hit y
		  {vec3(0.5, 0.5, -1), vec3(0, 0, 1), {true, 1, 2}},		 // hit z
		  {vec3(2, 0.5, 0.5), vec3(-1, 0, 0), {true, 1, 2}},		 // hit -x
		  {vec3(0.5, 2, 0.5), vec3(0, -1, 0), {true, 1, 2}},		 // hit -y
		  {vec3(0.5, 0.5, 2), vec3(0, 0, -1), {true, 1, 2}},		 // hit -z
		  {vec3(-0.5, -0.5, -0.5), vec3(1, 1, 1), {true, 0.5, 1.5}}, // hit diagonal
		  {vec3(-1, -0.5, -0.5), vec3(1, 0, 0), {false, 0, 0}},		 // miss x
		  {vec3(-0.5, -1, -0.5), vec3(0, 1, 0), {false, 0, 0}},		 // miss y
		  {vec3(-0.5, -0.5, -1), vec3(0, 0, 1), {false, 0, 0}},		 // miss z
		  {vec3(-1.18034, 5.67467, 1.23563), vec3(0.0083086, -0.0362171, -0.00454671), {true, 142.06245, 156.68483}},
	};
	for (auto& param : params) {
		auto result = cgv::math::ray_box_intersection<float>( //
			  param.origin,									  //
			  param.direction,								  //
			  vec3(0),										  //
			  vec3(1)										  //
		);

		ASSERT_EQ(param.expected.hit, result.hit);
		ASSERT_FLOAT_EQ(param.expected.t_near, result.t_near);
		ASSERT_FLOAT_EQ(param.expected.t_far, result.t_far);
	}
}
