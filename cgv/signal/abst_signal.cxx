#include <cgv/signal/abst_signal.h>
#include "signal.h"

namespace cgv {
	namespace signal {

void connect(signal<>& s, void(*fp)()) 
{
	s.connect(function_functor<0>(fp)); 
}

void disconnect(signal<>& s, void(*fp)()) 
{
	s.disconnect(function_functor<0>(fp)); 
}

signal_base::~signal_base()
{
	disconnect_all();
}

/// return the number of connected functors
unsigned signal_base::get_nr_functors() const
{
	return (unsigned) functors.size();
}

/// only use this if you exactly know what to do!
void signal_base::connect_abst(functor_base* fp)
{
	connect(fp);
}

void signal_base::link(functor_base* fp) 
{
	const tacker* t = fp->get_tacker();
	if (t)
		t->tack(this);
}

void signal_base::unlink(functor_base* fp) 
{
	const tacker* t = fp->get_tacker();
	if (t)
		t->untack(this);
	delete fp;
}

void signal_base::connect(functor_base* fp) 
{
	link(fp);
	functors.push_back(fp);
}

void signal_base::disconnect(const functor_base* fp)
{
	unsigned int i;
	for (i=0; i<functors.size(); ++i) {
		if (*functors[i] == *fp) {
			unlink(functors[i]);
			functors.erase(functors.begin()+i);
			--i;
		}
	}
}

void signal_base::disconnect(const tacker* c)
{
	unsigned int i;
	for (i=0; i<functors.size(); ++i) {
		if (functors[i]->get_tacker() == c) {
			unlink(functors[i]);
			functors.erase(functors.begin()+i);
			--i;
		}
	}
}


void signal_base::disconnect_all()
{
	unsigned int i;
	for (i=0; i<functors.size(); ++i)
		unlink(functors[i]);
	functors.clear();
}

functor_base::~functor_base() 
{
}

const tacker* functor_base::get_tacker() const
{
	return dynamic_cast<const tacker*>(this); 
}

bool functor_base::operator == (const functor_base& f) const
{
	const void *p1, *p2, *q1, *q2;
	put_pointers(p1,p2);
	f.put_pointers(q1,q2);
	return p1 == q1 && p2 == q2;
}

tacker::tacker()
{
}

/// reimplement copy constructor to avoid copying the signal map
tacker::tacker(const tacker&)
{
}

void tacker::tack(signal_base* s) const
{
	++signals[s];
}
void tacker::untack(signal_base* s) const
{
	if (--signals[s] <= 0)
		signals.erase(signals.find(s));
}

void tacker::untack_all() const
{
	while (!signals.empty())
		signals.begin()->first->disconnect(this);
}

tacker::~tacker()
{
	untack_all();
}


	}
}