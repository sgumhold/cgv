#pragma once

#include <cgv/math/fvec.h>
#include <cgv/math/functions.h>

namespace cgv {
	namespace math {

template <typename T, int N> class sturm_chain_mon;

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
	/// construct from a vector of roots
	fpoly_mon(const std::vector<T>& roots) : fvec<T, N + 1>(T(0)) {
		if (roots.size() == 0)
			return;
		data()[0] = -roots[0];
		data()[1] = T(1);
		for (int i = 1; i < roots.size(); ++i) {
			T v = -roots[i];
			data()[i + 1] = data()[i];
			for (int j = i; j > 0; --j)
				data()[j] = data()[j] * v + data()[j - 1];
			data()[0] *= v;
		}
	}
	/// convert to lower dimensional 
	template <int M> fpoly_mon<T,M>& down() { return *reinterpret_cast<fpoly_mon<T, M>*>(this) }
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
	/// compute roots of polynom and return count
	int compute_roots(const T& a, const T& b, T* roots, const T& eps) {
		sturm_chain_mon<T, N> sc(*this);
		auto Is = sc.eliminate_roots(a, b, eps);
		int cnt = 0;
		for (auto I : Is)
			roots[cnt++] = sc.find_root(I[0], I[1], eps);
		return cnt;
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
	/// return list of interval boundary pairs that contain one root each
	std::vector<cgv::math::fvec<T,2>> eliminate_roots(const T& a, const T& b, T eps) const {
		int cnt_a = count_sign_changes(evaluate(a));
		int cnt_b = count_sign_changes(evaluate(b));
		// less in less or equal is necessary for float precision to avoid accessing invalid memory
		if (cnt_a <= cnt_b)
			return {};
		if (cnt_b+1 == cnt_a)
			return { { a,b } };
		T upper_bounds[N+1], lower_bounds[N+1];
		int cnts[N+1], nexts[N+1];
		int i = 0, j = cnt_a - cnt_b;
		upper_bounds[i] = a;
		cnts[i] = cnt_a;
		nexts[i] = j;
		lower_bounds[j] = b;
		cnts[j] = cnt_b;
		while (i < j) {
			int k = nexts[i];
			if (k == i + 1) {
				i = k;
				continue;
			}
			T split = T(0.5) * (upper_bounds[i] + lower_bounds[k]);
			// clamping is necessary for float precision to avoid invalid memory access
			int cnt_split = clamp(count_sign_changes(evaluate(split)),cnt_b,cnt_a);
			int l = cnt_a - cnt_split;
			if (lower_bounds[k] - upper_bounds[i] <= eps) {
				upper_bounds[i] = lower_bounds[k];
				for (int j = i + 1; j < k; ++j)
					lower_bounds[j] = upper_bounds[j] = split;
				i = k;
			}
			if (l == i)
				upper_bounds[i] = split;
			else if (l == k)
				lower_bounds[k] = split;
			else {
				lower_bounds[l] = upper_bounds[l] = split;
				cnts[l] = cnt_split;
				nexts[l] = nexts[i];
				nexts[i] = l;
			}
		}
		std::vector<cgv::math::fvec<T, 2>> res;
		for (i = 0; i < j; ++i)
			res.push_back({ upper_bounds[i], lower_bounds[i + 1] } );
		return res;
	}
	/// find root in interval
	T find_root(const T& a, const T& b, const T& eps)
	{
		if (a == b)
			return a;
		T t0 = a, t1 = b;
		T y0 = poly().eval(t0);
		while (true) {
			// interval bisection
			T tm = (t0 + t1) / 2;
			if (t1 - t0 <= eps)
				return tm;
			T ym = poly().eval(tm);
			// try to improve with Newton step
			T tn = tm - ym / deri().eval(tm);
			if (tn > t0 && tn < t1) { // valid Newton step
				T yn = poly().eval(tn);
				if (yn < ym) {
					if (std::abs(tm - tn) <= eps)
						return tn;
					else {
						tm = tn;
						ym = yn;
					}
				}
			}
			// select interval
			if (sgn(y0) == sgn(ym)) {
				t0 = tm;
				y0 = ym;
			}
			else
				t1 = tm;
		}
		return a;
	}
};
	}
}