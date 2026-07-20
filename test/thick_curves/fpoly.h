#pragma once

#include <cgv/math/fvec.h>
#include <cgv/math/functions.h>

namespace cgv {
	namespace math {

template <typename T, int N> class fpoly_ber;

template <typename T, int N> class sturm_chain_mon;

int binom(int n, int k) {
	std::vector<std::vector<int>> dp(n + 1, std::vector<int>(k + 1));
	for (int i = 0; i <= n; i++)
		for (int j = 0; j <= std::min(i, k); j++)
			dp[i][j] = (j == 0 || j == i) ? 1 : dp[i][j] = dp[i - 1][j - 1] + dp[i - 1][j];
	return dp[n][k];
}

/// count the number of sign changes in vector ignoring all contained zeros
template <typename T, int N>
int count_sign_changes(const fvec<T, N>& v) {
	int prev_sgn;
	int i = 0;
	while (i < N && (prev_sgn=sgn(v[i])) == 0)
		++i;
	int last = N - 1;
	while (sgn(v[last]) == 0)
		if (--last <= i)
			return 0;
	int cnt = 0;
	for (++i; i <= last; ++i) {
		int next_sgn;
		while ((next_sgn=sgn(v[i])) == 0)
			++i;
		cnt += prev_sgn != next_sgn;
		prev_sgn = next_sgn;
	}
	return cnt;
}

/// count the number of sign changes in vector ignoring all contained zeros
template <typename T, int N>
void suggest_splits(const fvec<T, N>& v, int* splits) {
	int prev_sgn;
	int i = 0;
	while (i < N && (prev_sgn = sgn(v[i])) == 0)
		++i;
	int last = N - 1;
	while (sgn(v[last]) == 0)
		if (--last <= i)
			return;
	int cnt = 0;
	splits[0] = i;
	for (++i; i <= last; ++i) {
		int next_sgn;
		while ((next_sgn = sgn(v[i])) == 0)
			++i;
		if (prev_sgn == next_sgn) {
			if ((v[i] > v[splits[cnt]]) == (next_sgn == 1))
				splits[cnt] = i;
		}
		else {
			splits[++cnt] = i;
			prev_sgn = next_sgn;
		}
	}
}

template <typename T, int N> 
class fpoly_mon : public fvec<T, N+1>
{
	template <int M> struct recursive_down {
		static int compute_roots(fpoly_mon<T,N>& This, const T& a, const T& b, T* roots, const T& eps) {
			return This.down<M>().compute_roots(a, b, roots, eps);
		}
	};
	template <> struct recursive_down<1> {  static int compute_roots(fpoly_mon<T, N>& This, const T& a, const T& b, T* roots, const T& eps) { return 0; } };
	static const int* b2m_coeffs() {
		static int coeffs[(N+1)*(N+2)/2], *coeff_ptr = 0;
		if (coeff_ptr == 0) {
			int* c_ptr = coeff_ptr = coeffs;
			for (int i = 0; i <= N; ++i)
				for (int j = 0; j <= i; ++j)
					(*c_ptr++) = (((i + j) % 2 == 1) ? -1 : 1) * binom(N, i) * binom(i, j);
		}
		return coeff_ptr;
	}
public:
	/// default constructor
	fpoly_mon() {}
	/// coordinate type templated copy constructor
	template <typename S>
	fpoly_mon(const fpoly_mon<S, N>& p) : fvec<T, N + 1>(p) {}
	/// init all coefficients to v
	fpoly_mon(const T& v) : fvec<T, N + 1>(v) {}
	/// init from array
	fpoly_mon(const T* v_ptr) : fvec<T, N + 1>(N+1,v_ptr) {}
	/// construct from Bernstein representation
	fpoly_mon(const fpoly_ber<T, N>& b) : fvec<T, N + 1>(T(0)) {
		const int* c_ptr = b2m_coeffs();
		for (int i = 0; i <= N; ++i)
			for (int j = 0; j <= i; ++j) 
				data()[i] += (*c_ptr++)*b[j];
	}
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
	template <int M> fpoly_mon<T, M>& down() { return *reinterpret_cast<fpoly_mon<T, M>*>(this); }
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
		if (N < 1) { 
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
		if (std::abs(data()[N]) > eps) {
			sturm_chain_mon<T, N> sc(*this);
			auto Is = sc.eliminate_roots(a, b, eps);
			int cnt = 0;
			for (auto I : Is)
				roots[cnt++] = sc.find_root(I[0], I[1], eps);
			return cnt;
		}
		else
			return recursive_down<N-1>::compute_roots(*this, a, b, roots, eps);
	}
};

template <typename T, int N>
class fpoly_ber : public fvec<T, N + 1>
{
	template <int M, int S> struct recursive_down {
		static int compute_roots(fpoly_ber<T, N>& This, const T& a, const T& b, T* roots, const T& eps) {
			return This.down<M>().compute_roots<S>(a, b, roots, eps);
		}
	};
	template <int S> struct recursive_down<1,S> { static int compute_roots(fpoly_ber<T, N>& This, const T& a, const T& b, T* roots, const T& eps) { return 0; } };
	static const T* m2b_coeffs() {
		static T coeffs[(N + 1) * (N + 2) / 2], * coeff_ptr = 0;
		if (coeff_ptr == 0) {
			T* c_ptr = coeff_ptr = coeffs;
			for (int i = 0; i <= N; ++i)
				for (int j = 0; j <= i; ++j)
					(*c_ptr++) = T(binom(i, j)) / binom(N, j);
		}
		return coeff_ptr;
	}
	static const T* eval_coeffs() {
		static T coeffs[N+1], * coeff_ptr = 0;
		if (coeff_ptr == 0) {
			T* c_ptr = coeff_ptr = coeffs;
			T nck = T(1);
			for (int i = 1; i <= N; ++i) {
				nck = nck * T(N - i + 1) / T(i);
				(*c_ptr++) = nck;
			}
		}
		return coeff_ptr;
	}
	template <int M>
	static const T* prod_coeffs() {
		static T coeffs[(N + M + 1) * (N + M + 1)], * coeff_ptr = 0;
		if (coeff_ptr == 0) {
			T* c_ptr = coeff_ptr = coeffs;
			for (int k = 0; k <= N + M; ++k)
				for (int i = std::max(0, k - M); i <= std::min(N, k); ++i)
					(*c_ptr++) = T(binom(N, i)) * T(binom(M, k - i)) / T(binom(N + M, k));
		}
		return coeff_ptr;
	}
public:
	/// default constructor
	fpoly_ber() {}
	/// coordinate type templated copy constructor
	template <typename S>
	fpoly_ber(const fpoly_ber<S, N>& p) : fvec<T, N + 1>(p) {}
	/// init all coefficients to v
	fpoly_ber(const T& v) : fvec<T, N + 1>(v) {}
	/// init from array
	fpoly_ber(const T* v_ptr) : fvec<T, N + 1>(N + 1, v_ptr) {}
	/// construct from monom representation
	fpoly_ber(const fpoly_mon<T, N>& a) : fvec<T, N + 1>(T(0)) {
		const T* c_ptr = m2b_coeffs();
		for (int i = 0; i <= N; ++i)
			for (int j = 0; j <= i; ++j)
				data()[i] += (*c_ptr++) * a[j];
	}
	/// convert to lower dimensional 
	template <int M> fpoly_ber<T, M>& down() { return *reinterpret_cast<fpoly_ber<T, M>*>(this); }
	/// construct from value list
	template <typename S> fpoly_ber(std::initializer_list<S> values) {
		//static_assert(values.size() == N+1, "Initializer list must have exactly N+1 entries");
		size_t i = 0;
		for (auto v : values)
			data()[i++] = T(v);
	}
	/// multiply two polynomials
	template <int M>
	fpoly_ber<T, N + M> operator * (const fpoly_ber<T, M>& q) const {
		fpoly_ber<T, N + M> r(T(0));
		const T* c_ptr = prod_coeffs<M>();
		for (int k = 0; k <= N + M; ++k)
			for (int i = std::max(0, k - M); i <= std::min(N, k); ++i)
				r[k] += (*c_ptr++) * data()[i] * q[k - i];
		return r;
	}
	/// perform polynomial division with respect to divident d and compute quotient q and remainder r
	template <int M>
	void div(const fpoly_ber<T, M>& d, fpoly_ber<T, N - M>& q, fpoly_ber<T, M - 1>& r) const {
		static_assert(M <= N, "divisor with larger degree than divident not allowed in polynomial division. Result is always 0 for quotient and divident for remainder.");
		r = fpoly_ber<T, M - 1>(data() + (N - M + 1));
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
	fpoly_ber<T, N> operator + (const fpoly_ber<T, N>& p) const {
		fpoly_ber<T, N> r;
		for (int i = 0; i <= N; ++i)
			r[i] = data()[i] + p[i];
		return r;
	}
	/// subtract polynomials
	fpoly_ber<T, N> operator - (const fpoly_ber<T, N>& p) const {
		fpoly_ber<T, N> r;
		for (int i = 0; i <= N; ++i)
			r[i] = data()[i] - p[i];
		return r;
	}
	/// Chudy and Pawel evaluation at given parameter with one of implementations of the mix operation: 0 ... default, 1 ... fma, 2 ... twice fma
	template <int mix_impl = 1>
	T eval_chudy_pawel(const T& t) const {
		T p = data()[0];
		T h = T(1);
		T u = T(1) - t;
		int n1 = N + 1;
		if (t <= T(0.5)) {
			u = t / u;
			for (int i = 1; i <= N; i++) {
				h = h * u * (n1 - i);
				h = h / (i + h);
				T h1 = T(1) - h;
				if (mix_impl == 0)
					p = h1 * p + h * data()[i];
				else if (mix_impl == 1)
					p = std::fma(h1, p, h * data()[i]);
				else
					p = std::fma(data()[i], h, std::fma(p, -h, p));
			}
		}
		else {
			u = u / t;
			for (int i = 1; i <= N; i++) {
				h = h * (n1 - i);
				h = h / (i * u + h);
				T h1 = T(1) - h;
				if (mix_impl == 0)
					p = h1 * p + h * data()[i];
				else if (mix_impl == 1)
					p = std::fma(h1, p, h * data()[i]);
				else
					p = std::fma(data()[i], h, std::fma(p, -h, p));
			}
		}
		return p;
	}
	/// de Casteljau evaluation at given parameter with one of implementations of the mix operation: 0 ... default, 1 ... fma, 2 ... twice fma
	template <int mix_impl = 1>
	T eval_de_casteljau(const T& t) const {
		T b[N + 1]; std::copy(data(), data() + size(), b);
		T omt = T(1) - t;
		for (size_t r = 1; r <= N; ++r)
			for (size_t i = 0; i <= N - r; ++i)
				if (mix_impl == 0)
					b[i] = omt * b[i] + t * b[i + 1];
				else if (mix_impl == 1)
					b[i] = std::fma(b[i], omt, t * b[i + 1]);
				else
					b[i] = std::fma(b[i + 1], t, std::fma(b[i], -t, b[i]));
		return b[0];
	}
	/// use de Casteljau to split curve into two parts at parameter t, left side overwrites this poly and right side is returned
	fpoly_ber<T,N> split_de_casteljau(const T& t) {
		fpoly_ber<T, N> q;
		T b[N + 1]; std::copy(data(), data() + size(), b);
		T omt = T(1) - t;
		for (int r = 1; r <= N; ++r) {
			data()[r-1] = b[0];
			q[N-r+1] = b[N-r+1];
			for (int i = 0; i <= N - r; ++i) 
				b[i] = omt * b[i] + t * b[i + 1];
		}
		data()[N] = q[0] = b[0];
		return q;
	}
	/// linear time constraint storage evaluation at given parameter with one of three implementations of the mix operation: 0 ... default, 1 ... fma, 2 ... twice fma
	template <int mix_impl = 1>
	T eval_ltcs(const T& t) const {
		T p = data()[0];
		const T omt = T(1) - t;
		T ti = t;
		const T* c_ptr = eval_coeffs();
		for (int i = 1; i <= N; ++i) {
			if (mix_impl == 0)
				p = p * omt + (*c_ptr++) * ti * data()[i];
			else if (mix_impl == 1)
				p = std::fma(p, omt, (*c_ptr++) * ti * data()[i]);
			else
				p = std::fma(ti * (*c_ptr++), data()[i], std::fma(-t, p, p));
			ti *= t;
		}
		return p;
	}
	/// uses suggested implementation of evaluation
	T eval(const T& t) const {
		if (N == 0)
			return data()[0];
		return eval_ltcs<0>(t);
	}
	/// jointly evaluate polynomial and derivative is not more efficient than computing derivative first and then evaluating it
	T eval_with_derivative(const T& t, T& d) const {
		d = N == 0 ? T(0) : derive().eval(t);
		return eval(t);
	}
	/// compute derivative
	fpoly_ber<T, N-1> derive() const {
		fpoly_ber<T, N - 1> d;
		for (int i = 0; i < N; ++i)
			d[i] = N * (data()[i + 1] - data()[i]);
		return d;
	}
	/// construct up to N intervals that contain individual roots within [a,b]
	int eliminate_roots(const T& a, const T& b, cgv::math::fvec<T, 2>* I, T eps) const {
		fpoly_ber<T, N> P[N];
		P[0] = *this;
		if (a > T(0)) {
			fpoly_ber<T, N> q = P[0].split_de_casteljau(a);
			P[0] = q;
			if (b < T(1))
				P[0].split_de_casteljau((b-a) / (T(1) - a));
		}
		else if (b < T(1))
			P[0].split_de_casteljau(b);
		int cnt = count_sign_changes(P[0]);
		// less in less or equal is necessary for float precision to avoid accessing invalid memory
		if (cnt == 0)
			return 0;
		I[0] = { a,b };
		if (cnt == 1)
			return 1;
		// keep track of current interval
		int root_cnt[N];
		root_cnt[0] = cnt;
		int i = 0, n = 1;
		while (i<n) {
			// if current interval only one root left, advance i
			int current_cnt = root_cnt[i];
			if (current_cnt == 1) {
				++i;
				continue;
			}
			// otherwise split in half
			T split = T(0.5) * (I[i][0] + I[i][1]);
			// avoid infinite subdivision
			if (std::abs(I[i][1] - I[i][0]) < eps) {
				++i;
				continue;
			}
			// use de Casteljau splitting on polynomial
			fpoly_ber<T, N> q = P[i].split_de_casteljau(T(.5));
			// if left side has no roots, replace with right side
			int left_cnt = count_sign_changes(P[i]);
			if (left_cnt == 0) {
				P[i] = q;
				I[i][0] = split;
				root_cnt[i] = count_sign_changes(q);
				// if no more roots found, remove interval
				if (root_cnt[i] == 0) {
					if (n == i+1)
						return n-1;
					P[i] = P[n - 1];
					root_cnt[0] = root_cnt[n - 1];
					I[i] = I[n - 1];
					--n;
					continue;
				}
			}
			else {
				T tmp = I[i][1];
				I[i][1] = split;
				root_cnt[i] = left_cnt;
				int right_cnt = count_sign_changes(q);
				if (right_cnt > 0) {
					I[n] = { split, tmp };
					P[n] = q;
					root_cnt[n++] = right_cnt;
				}
			}
		}
		return n;
	}
	/// construct up to N intervals that contain individual roots within [a,b]
	int eliminate_roots_improved(const T& a, const T& b, cgv::math::fvec<T, 2>* I, T eps) const {
		int cnt = count_sign_changes(*this);
		// less in less or equal is necessary for float precision to avoid accessing invalid memory
		if (cnt == 0)
			return 0;
		I[0] = { a,b };
		if (cnt == 1)
			return 1;
		// keep track of current interval
		fpoly_ber<T, N> P[N];
		int root_cnt[N];
		root_cnt[0] = cnt;
		P[0] = *this;
		int i = 0, n = 1;
		while (i<n) {
			// if current interval only one root left, advance i
			int current_cnt = root_cnt[i];
			if (current_cnt == 1) {
				++i;
				continue;
			}
			int prev_split = 0;
			int splits[N+1];
			suggest_splits(P[i], splits);
			T begin = I[i][0];
			T end   = I[i][1];
			T dit   = (I[i][1]-begin)/N;
			int insert = i;
			for (int j = 1; j < current_cnt; ++j) {				
				T split = begin + T(splits[j]) * dit; // absolute split location
				fpoly_ber<T, N> q = P[insert].split_de_casteljau(T(splits[j] - prev_split) / (N - prev_split));
				prev_split = splits[j];
				if ((root_cnt[insert] = count_sign_changes(P[insert])) > 0) {
					I[insert][1] = split;
					insert = (n += (insert == n));
				}
				I[insert] = { split, end };
				P[insert] = q;
			}
			if ((root_cnt[insert] = count_sign_changes(P[insert])) > 0)
				n += (insert == n);
			else if (i == insert) {
				if (--n == i)
					return n;
				P[i] = P[n];
				root_cnt[0] = root_cnt[n];
				I[i] = I[n];
			}
		}
		return n;
	}
	/// compute roots of polynom with bernstein clipping
	int compute_roots_direct(const T& a, const T& b, T* roots, const T& eps) {
		if (count_sign_changes(*this) == 0)
			return 0;
		std::vector<std::pair<fpoly_ber<T, N>, fvec<T, 2>>> P;
		P.push_back({ *this,fvec<T,2>(a,b) });
		int cnt = 0;
		do {
			T split = T(.5) * (P.back().second[0] + P.back().second[1]);
			if (P.back().second[1] - P.back().second[0] > eps) {
				fpoly_ber<T, N> p = P.back().first.split_de_casteljau(T(.5));
				if (count_sign_changes(P.back().first) == 0) {
					if (count_sign_changes(p) > 0) {
						P.back().first = p;
						P.back().second[0] = split;
						continue;
					}
				}
				else {
					T tmp = P.back().second[1];
					P.back().second[1] = split;
					if (count_sign_changes(p) > 0)
						P.push_back({ p, fvec<T,2>(split,tmp) });
					continue;
				}
			}
			else
				roots[cnt++] = split;
			P.pop_back();
		} while (!P.empty());
		return cnt;
	}
	/// find root with Newton's method in given interval
	T find_root(const T& a, const T& b, const T& eps) {
		if (a == b)
			return a;
		T t0 = a, t1 = b;
		T y0 = eval(t0);
		fpoly_ber<T,N-1> deri = derive();
		while (true) {
			// interval bisection
			T tm = (t0 + t1) / 2;
			if (t1 - t0 <= eps)
				return tm;
			T ym = eval(tm);
			// try to improve with Newton step
			T tn = tm - ym / deri.eval(tm);
			if (tn > t0 && tn < t1) { // valid Newton step
				T yn = eval(tn);
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
	/// compute roots of Bernstein polynom with different strategies and return count
	template <int S = 0>
	int compute_roots(const T& a, const T& b, T* roots, const T& eps) {
		if (std::abs(data()[N]) < eps)
			return recursive_down<N-1,S>::compute_roots(*this, a, b, roots, eps);
//			return compute_roots_direct(a, b, roots, eps);
		cgv::math::fvec<T, 2> I[N];
		int cnt = (S == 0) ? eliminate_roots(a, b, I, eps) : eliminate_roots_improved(a, b, I, eps);
		for (int i=0; i<cnt; ++i)
			roots[i] = find_root(I[i][0], I[i][1], eps);
		return cnt;
	}
};

/// implementation of schur chain to estimate root count on given interval
template <typename T, int N>
class sturm_chain_mon : public fvec<T, (N + 1)* (N + 2) / 2>
{
protected:
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