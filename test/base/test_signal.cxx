#include <cgv/base/signal.h>
#include <cgv/base/register.h>
#include <cgv/base/rebind.h>
#include <cgv/base/bool_signal.h>
#include <iostream>

using namespace cgv::base;

void test_func(int i, const std::string& s)
{
//	std::cout << "test_func(" << i << ",'" << s.c_str() << "')" << std::endl;
}

bool bool_test_func(int i)
{
//	std::cout << "bool_test_func(" << i << ") ==> " << (i==0?"true":"false") << std::endl;
	return i == 0;
}

/*! derive your class from cgv::base::tracker to automatically 
    disconnect from signals on destruction */
struct X : public tacker
{
	int a;
	X(int _a) : a(_a) {}
	void test(int i, const std::string& s)
	{
//		std::cout << "X[" << a << "]::test(" << i << ",'" << s.c_str() << "')" << std::endl;
	}
	void operator () (float f) const
	{
//		std::cout << "X[" << a << "]::() (" << f << ")" << std::endl;
	}
	bool bool_test(int i) const
	{
//		std::cout << "X[" << a << "]::bool_test(" << i << ") ==> " << (i==a?"true":"false") << std::endl;
		return i == a;
	}
};

struct Y : public tacker
{
	int a;
	Y(int _a = 0) : a(_a) {}
	void operator () (float f)
	{
//		std::cout << "Y[" << a << "]::() (" << f << ")" << std::endl;
	}

};

bool test_signal()
{
	signal<int,const std::string&> sig;
	X x0(0);

	connect(sig, test_func);
	connect(sig, &x0, &X::test);
	{
		X x1(1);
		Y *y_tmp;
		signal<float> sigf;

		connect(sig, &x1, &X::test);
		connect(sigf, x0);
		connect(sigf, x1);
		{
			X x2(2);
			Y y1(1), y2(5);
			connect(sig, &x2, &X::test);
			connect(sigf, x2);
			connect(sigf, x2);
			connect(sigf, y1);
			y_tmp = &connect_copy(sigf, y2);

			sig(1,"first");
			sigf(1.0f);
		}
		sig(2,"second");
		sigf(2.0f);
		disconnect(sigf, *y_tmp);
		sigf(3.0f);
	}
	sig(3,"third");
	disconnect(sig, test_func);
	sig(4,"fourth");

	bool res;
	bool_signal<int> bsig;
	X x1(1),x2(2),x3(3);

	connect(bsig, bool_test_func);
	connect(bsig, &x0, &X::bool_test);
	connect(bsig, &x1, &X::bool_test);
	connect(bsig, &x2, &X::bool_test);
	connect(bsig, &x3, &X::bool_test);

	res = bsig(0);
	TEST_ASSERT_EQ(res, false);
	res = bsig(1);
	TEST_ASSERT_EQ(res, false);
	bsig.set_options("+&");
	res = bsig(2);
	TEST_ASSERT_EQ(res, false);

	bsig.set_options("*|");
	res = bsig(5);
	TEST_ASSERT_EQ(res, false);
	res = bsig(3);
	TEST_ASSERT_EQ(res, true);
	res = bsig(1);
	TEST_ASSERT_EQ(res, true);

	bsig.set_options("+|");
	res = bsig(5);
	TEST_ASSERT_EQ(res, false);
	res = bsig(1);
	TEST_ASSERT_EQ(res, true);
	return true;
}

test_registration test_cb_signal_reg("cgv::base::test_signal", test_signal);