#pragma once

#include <cgv/math/fvec.h>
#include <cgv/math/functions.h>

namespace cgv {
	namespace math {

template <typename T, int N> 
class fpoly_mon : public fvec<T, N+1>
{
public:
	/// default constructor
	fpoly_mon() {}
	/// init all coefficients to v
	fpoly_mon(const T& v) : fvec<T, N + 1>(v) {}
	/// init from array
	fpoly_mon(const T* v_ptr) : fvec<T, N + 1>(N+1,v_ptr) {}
	/// construct from value list
	template <typename S>
	fpoly_mon(std::initializer_list<S> values) {
		//static_assert(values.size() == N+1, "Initializer list must have exactly N+1 entries");
		size_t i = 0;
		for (auto v : values)
			data()[i++] = T(v);
	}
	/// multiply two polynomials
	template <int M>
	fpoly_mon<T, N + M> operator * (const fpoly_mon<T,M>& p) const {
		fpoly_mon<T, N + M> r(T(0));
		for (int i = 0; i <= N; ++i) 
			for (int j = 0; j <= M; ++j)
				r[i + j] += data()[i] * p[j];
		return r;
	}
	/// perform polynomial division with respect to divident d and compute quotient q and remainder r
	template <int M>
	void div(const fpoly_mon<T, M>& d, fpoly_mon<T, N - M>& q, fpoly_mon<T, M - 1>& r) const {
		static_assert(M <= N, "divisor with larger degree than divident not allowed in polynomial division. Result is always 0 for quotient and divident for remainder.");
		r = fpoly_mon<T, M - 1>(data() + (N - M + 1));
		for (int j = 0; j <= N - M; ++j) {
			q[N - M - j] = r[M - 1] / d[M];
			for (int i = 1; i < M; ++i)
				r[M - i] = r[M - i - 1] - q[N - M - j] * d[M - i];
			r[0] = data()[N - M - j] - q[N - M - j] * d[0];
		}
	}
	/// negate polynomial
	void negate() {
		for (int i = 0; i <= N; ++i)
			data()[i] = -data()[i];
	}
	/// add two polynomials
	fpoly_mon<T, N> operator + (const fpoly_mon<T,N>& p) const {
		fpoly_mon<T, N> r;
		for (int i = 0; i <= N; ++i)
			r[i] = data()[i] + p[i];
		return r;
	}
	/// subtract polynomials
	fpoly_mon<T, N> operator - (const fpoly_mon<T,N>& p) const {
		fpoly_mon<T, N> r;
		for (int i = 0; i <= N; ++i)
			r[i] = data()[i] - p[i];
		return r;
	}
	/// evaluate at given parameter 
	T eval(const T& t) const {
		T r = data()[N]; 
		for (int i = N - 1; i >= 0; --i) 
			r = r * t + data()[i];
		return r;
	}
	/// jointly evaluate polynomial and derivative
	T eval_with_derivative(const T& t, T& d) const {
		if constexpr (N < 1) { 
			d = 0; 
			return data()[0]; 
		}
		else {
			T p = data()[N] * t + data()[N - 1];
			T dp = data()[N];
			for (int i = N - 2; i >= 0; --i) {
				dp = dp * t + p;
				p = p * t + data()[i];
			}
			d = dp;
			return p;
		}
	}
	/// compute derivative
	fpoly_mon<T, N-1> derive() const {
		fpoly_mon<T, N-1> r;
		r[0] = data()[1]; 
		for (int i = 2; i <= N; ++i) 
			r[i - 1] = i * data()[i];
		return r;
	}
};

/// implementation of schur chain to estimate root count on given interval
template <typename T, int N>
class sturm_chain_mon : public fvec<T, (N + 1)* (N + 2) / 2>
{
protected:
	/// count the number of sign changes in vector ignoring all contained zeros
	int count_sign_changes(const fvec<T, N + 1>& v) const {
		int cnt = 0, last_sgn = 0;
		for (int i = 0; i < N + 1; ++i) {
			int next_sgn = sgn(v[i]);
			if (last_sgn == 0)
				last_sgn = next_sgn;
			else if (next_sgn != 0) {
				cnt += last_sgn != next_sgn;
				last_sgn = next_sgn;
			}
		}
		return cnt;
	}
	template <int M> struct recursive_eval {
		static void call(const T* sc, const T& t, T*v) {
			*v = reinterpret_cast<const fpoly_mon<T, M>*>(sc)->eval(t);
			recursive_eval<M-1>::call(sc+M+1, t, v + 1);
		}
	};
	template <> struct recursive_eval<0> { static void call(const T* sc, const T& t, T* v) { *v = *sc;	} };

	template <int M>
	struct recursive_construct
	{
		static void call(const fpoly_mon<T, M + 2>& f, const fpoly_mon<T, M + 1>& g, T* sc) {
			fpoly_mon<T, 1> q;
			auto& r = *reinterpret_cast<fpoly_mon<T, M>*>(sc);
			f.div(g, q, r);
			r.negate();
			recursive_construct<M-1>::call(g, r, sc + M+1);
		}
	};
	template <>	struct recursive_construct<-1> { static void call(const fpoly_mon<T, 1>& f, const fpoly_mon<T, 0>& g, T* sc) {} };
public:
	/// access to polynomial
	fpoly_mon<T, N>& poly() { return *reinterpret_cast<fpoly_mon<T, N>*>(data()); }
	const fpoly_mon<T, N>& poly() const { return *reinterpret_cast<const fpoly_mon<T, N>*>(data()); }
	/// access to derivative
	fpoly_mon<T, N-1>& deri() { return *reinterpret_cast<fpoly_mon<T, N-1>*>(data()+N+1); }
	const fpoly_mon<T, N-1>& deri() const { return *reinterpret_cast<const fpoly_mon<T, N-1>*>(data()+N+1); }
	/// construct from fpoly in monom basis
	sturm_chain_mon(const fpoly_mon<T, N>& p) {
		poly() = p;
		deri() = p.derive();
		recursive_construct<N-2>::call(poly(), deri(), data() + 2*N+1);
	}
	/// evaluate
	fvec<T, N + 1> evaluate(const T& t) const {
		fvec<T, N + 1> v;
		v[0] = poly().eval_with_derivative(t, v[1]);
		recursive_eval<N-2>::call(data()+2*N+1, t, v.data()+2);
		return v;
	}
	/// estimate root count
	int estimate_nr_roots_on_interval(const T& a, const T& b) const {
		return count_sign_changes(evaluate(a)) - count_sign_changes(evaluate(b));
	}
};
	}
}