#ifndef NDEBUG
#	include <iostream>
#endif

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

signal_base::~signal_base() noexcept
{
	disconnect_all();
}

/// only use this if you exactly know what to do!
void signal_base::connect_abst(functor_base* fp)
{
	connect(fp);
}

void signal_base::link(receiver& rcvr)
{
	if (rcvr.tacker)
		rcvr.tacker->tack(this);
}

void signal_base::unlink(receiver& rcvr)
{
	if (rcvr.tacker)
		rcvr.tacker->untack(this);
}

void signal_base::connect(functor_base* fp)
{
	receivers.push_back({std::unique_ptr<functor_base>{fp}, fp->get_tacker()});
	link(receivers.back());
}

void signal_base::disconnect(const functor_base* fp)
{
	auto found = false;
	for (auto rcvr = receivers.rbegin(); rcvr != receivers.rend(); ++rcvr) {
		if (*rcvr->functor != *fp) continue;
		found = true;
		unlink(*rcvr);
		receivers.erase(rcvr.base() - 1);
	}
	#ifndef NDEBUG
		if (!found)
			std::cerr << "Attempted to disconnect a functor from a signal it is not connected to.\n";
	#endif
}

void signal_base::disconnect(const tacker* c)
{
	auto found = false;
	for (auto rcvr = receivers.rbegin(); rcvr != receivers.rend(); ++rcvr) {
		if (rcvr->tacker != c) continue;
		found = true;
		unlink(*rcvr);
		receivers.erase(rcvr.base() - 1);
	}
	#ifndef NDEBUG
		if (!found)
			std::cerr << "Attempted to disconnect a tacker from a signal it is not connected to.\n";
	#endif
}

void signal_base::disconnect_all()
{
	for (auto& rcvr : receivers)
		unlink(rcvr);
	receivers.clear();
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


void tacker::tack(signal_base* s) const
{
	++signals[s];
}
void tacker::untack(signal_base* s) const
{
	const auto s_it = signals.find(s);
	if (s_it == signals.end()) {
		#ifdef NDEBUG
			std::cout << "Attempted to untack a signal the tacker is not connected to.\n";
		#endif
		return;
	}

	if (--s_it->second == 0)
		signals.erase(s_it);
}

void tacker::untack_all() const
{
	for (auto [signal, num_tacks] : signals) {
		auto& receivers = signal->receivers;
		auto removed = 0u;
		for (auto rcvr = receivers.rbegin(); rcvr != receivers.rend(); ++rcvr) {
			if (rcvr->tacker != this) continue;
			receivers.erase(rcvr.base() - 1);
			if (++removed == num_tacks) break;
		}
	}
}

tacker::~tacker() noexcept
{
	untack_all();
}


	}
}
