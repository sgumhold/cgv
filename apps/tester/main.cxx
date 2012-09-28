#include <cgv/base/register.h>
#include <iostream>

using namespace cgv::base;


struct test_listener : public base, public registration_listener
{
	static std::vector<base_ptr> tests;
	void register_object(base_ptr object, const std::string& options)
	{
		if (object->get_interface<test>())
			tests.push_back(object);
	}
	void unregister_object(base_ptr object, const std::string& options)
	{
	}
	static bool perform_tests()
	{
		if (tests.size() == 0) {
			std::cout << "no tests registered" << std::endl;
			return true;
		}
		unsigned int succeeded = 0;
		for (unsigned int i=0; i<tests.size(); ++i) {
			test* t = tests[i]->get_interface<test>();
			std::cout << "test " << t->get_test_name().c_str() << ":";
			std::cout.flush();
			cgv::base::test::nr_failed = 0;
			if (t->exec_test() && (cgv::base::test::nr_failed == 0)) {
				++succeeded;
				std::cout << "ok";
			}
			else
				std::cout << "failed";
			std::cout << std::endl;
		}
		if (succeeded == tests.size()) {
			std::cout << "all tests successful" << std::endl;
			return true;
		}
		else
			std::cout << (tests.size()-succeeded) << " tests of " << tests.size() << " failed" << std::endl;
		return false;
	}
};

std::vector<base_ptr> test_listener::tests;

int main(int argc, char** argv)
{
	register_object(new test_listener());
	enable_registration();
	process_command_line_args(argc, argv);
	bool res = test_listener::perform_tests();
#if _MSC_VER >= 1600
	std::cin.get();
#endif
	return res ? 0 : -1;
}
