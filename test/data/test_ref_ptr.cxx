#include <iostream>
#include <cgv/data/ref_counted.h>
#include <cgv/data/ref_ptr.h>
#include <cgv/data/ref_arr.h>

using namespace cgv::data;

class T;

typedef ref_ptr<T,true> T_ptr;

class T : public ref_counted
{
protected:
	friend class ref_ptr_impl<T,true>;
	~T()
	{
		std::cout << "T::~T()" << std::endl;
	}
public:
	T()
	{
		std::cout << "T::T()" << std::endl;
	}
};



class A;
class B;

typedef ref_ptr<A,true> A_ptr;
typedef ref_ptr<B,true> B_ptr;

class A : public ref_counted
{
protected:
	friend class ref_ptr_impl<A,true>;
	int index;
	B_ptr b_ptr;
	~A() 
	{
		std::cout << "A::~A()" << std::endl;
	}
public:
	A(int i) : index(i) 
	{
		std::cout << "A::A(" << i << ")" << std::endl;
	}
	void set_B(B_ptr bp);
	void clear_B();
};


void A::set_B(B_ptr bp)
{
	b_ptr = bp;
}
void A::clear_B()
{
	b_ptr.clear();
}

class B : public ref_counted
{
protected:
	friend class ref_ptr_impl<B,true>;
	std::string str;
	A_ptr a_ptr;
	~B() 
	{
		std::cout << "B::~B()" << std::endl;
	}
public:
	/// declare constructor protected, in order to not allow members of type B in other classes
	B(const std::string& s) : str(s) 
	{
		std::cout << "B::B(" << s.c_str() << ")" << std::endl;
	}
	void set_A(A_ptr ap);
};

void B::set_A(A_ptr ap)
{
	a_ptr = ap;
}

/*
class C
{
protected:
	A a;
	B b;
public:
	C() : a(4), b("test") {}
};
*/

void test_ref_ptr()
{
//	T t0;
	T_ptr t(new T());

	if (t)
		std::cout << "t is defined" << std::endl;
	t.clear();
	if (!t)
		std::cout << "t is empty" << std::endl;
	A_ptr a(new A(5));
	B_ptr b(new B("hello world"));
	a->set_B(b);
	b->set_A(a);	
	A_ptr a1;
	a1 = a;
	A_ptr a2(a1);
	std::cout << "before break loop #a = " << a.get_count() << std::endl;
	a->clear_B();
	std::cout << "before destruct" << std::endl;

	const char* src = "Hello my array world";
	size_t n = std::string(src).size();
	ref_arr<unsigned char> arr(new unsigned char[n+1]);
	std::copy(src, src+n+1, arr.ptr());
	std::cout << arr.ptr() << "::" << arr(10).ptr() << std::endl;
}