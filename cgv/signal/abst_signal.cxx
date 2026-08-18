#ifndef NDEBUG
#	include <iostream>
#endif

#include <algorithm>

#include <cgv/signal/abst_signal.h>
#include <cgv/signal/signal.h>


#define MSG_ERROR "\x1b[1;31m[error]\x1b[39m[cgv::signal]\x1b[m"


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
	if (rcvr.tacker_ptr)
		rcvr.tacker_ptr->tack(this);
}

void signal_base::unlink(receiver& rcvr)
{
	if (rcvr.tacker_ptr)
		rcvr.tacker_ptr->untack(this);
}

void signal_base::connect(functor_base* fp)
{
	receivers.push_back({std::unique_ptr<functor_base>{fp}, fp->get_tacker()});
	link(receivers.back());
}

void signal_base::disconnect(const functor_base* fp)
{
	auto found = false;
	receivers.erase(std::remove_if(receivers.begin(), receivers.end(), [this, &found, fp](receiver& rcvr) {
		if(*rcvr.functor == *fp) {
			found = true;
			unlink(rcvr);
			return true;
		}
		return false;
	}), receivers.end());

	#ifndef NDEBUG
		if (!found)
			std::cerr << MSG_ERROR" Attempted to disconnect a functor from a signal it is not connected to.\n";
	#endif
}

void signal_base::disconnect(const tacker* c)
{
	auto found = false;
	receivers.erase(std::remove_if(receivers.begin(), receivers.end(), [this, &found, c](receiver& rcvr) {
		if(rcvr.tacker_ptr == c) {
			found = true;
			unlink(rcvr);
			return true;
		}
		return false;
	}), receivers.end());

	#ifndef NDEBUG
		if (!found)
			std::cerr << MSG_ERROR" Attempted to disconnect a tacker from a signal it is not connected to.\n";
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
		#ifndef NDEBUG
			std::cout << MSG_ERROR" Attempted to untack a signal for a tacker that is not connected to it.\n";
		#endif
		return;
	}

	if (--s_it->second == 0)
		signals.erase(s_it);
}

void tacker::untack_all() const
{
	for (auto& signal : signals) {
		auto& receivers = signal.first->receivers;
		int removed = 0;
		bool done = false;
		receivers.erase(std::remove_if(receivers.begin(), receivers.end(), [this, &signal, &done, &removed](signal_base::receiver& rcvr) {
			if(!done && rcvr.tacker_ptr == this) {
				if(++removed == signal.second)
					done = true;
				return true;
			}
			return false;
		}), receivers.end());
	}
}

tacker::~tacker() noexcept
{
	untack_all();
}


	}
}
