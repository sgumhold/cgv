#pragma once

#include "functions.h"
#include "mat.h"
#include "diag_mat.h"

#include <limits>
#include <algorithm>


#include "vec.h"
#include <complex>

namespace cgv {
namespace math {


//if x is an eigen vector of A the rayleigh quotient returns the corresponding eigenvalue 
template<typename T>
T rayleigh_quotient(mat<T>& A,vec<T>&x)
{
	return dot(x,A*x)/dot(x,x);
}




	 
template <typename T>
void rot(mat<T> &a, const T s, const T tau, const int i,

		const int j, const int k, const int l)

	{

		T g=a(i,j);

		T h=a(k,l);

		a(i,j)=g-s*(h+g*tau);

		a(k,l)=h+s*(g-h*tau);

	}



template <typename T>
void eigsrt(diag_mat<T> &d, mat<T> &v)

{

	unsigned k;

	unsigned n=d.size();

	for (unsigned i=0;i<n-1;i++) 

	{

		T p=d(k=i);

		for (unsigned j=i;j<n;j++)

			if (d(j) >= p) p=d(k=j);

		if (k != i) 

		{

			d(k)=d(i);
			d(i)=p;

			
			for (unsigned j=0;j<n;j++) 
			{

					p=v(j,i);
					v(j,i)=v(j,k);
					v(j,k)=p;

			}
		}
	}
}

///eigen decomposition of a symmetric matrix using the jacobi method
///v contains the eigenvectors
///d contains the eigenvalues
///a=v*d*transpose(v)
template <typename T>
bool eig_sym(const mat<T> &a, mat<T> &v, diag_mat<T> &d,bool ordering=true, unsigned maxiter=50)
{
	mat<T>aa = a;
	unsigned n =aa.nrows();

	v.identity(n);

	d.resize(n);

	unsigned nrot=0;

	const T eps = std::numeric_limits<T>::epsilon();

	T tresh,theta,tau,t,sm,s,h,g,c;

	vec<T> b(n),z;
	z.zeros(n);
	unsigned ip,iq;
	
	for(unsigned i = 0; i < n; i++)
		d(i)=b(i)=aa(i,i);

	for (unsigned i=1;i<=maxiter;i++) 

	{

			sm=(T)0.0;

			

			for (ip=0;ip<n-1;ip++) 

			{

				for (iq=ip+1;iq<n;iq++)

					sm += std::abs(aa(ip,iq));

			}

			if (sm == (T)0.0) 

			{

				if(ordering)

					eigsrt(d,v);

				return true;

			}

			if (i < 4)

				tresh=(T)(0.2*sm/(n*n));

			else

				tresh=(T)0.0;

			for (ip=0;ip<n-1;ip++) 

			{

				for (iq=ip+1;iq<n;iq++) 

				{

					g=((T)100.0)*std::abs(aa(ip,iq));

					if (i > 4 && g <= eps*std::abs(d(ip)) && g <= eps*std::abs(d(iq)))

							aa(ip,iq)=(T)0.0;

					else if (std::abs(aa(ip,iq)) > tresh) 

					{

						h=d(iq)-d(ip);

						if (g <= eps*std::abs(h))

							t=(aa(ip,iq))/h;

						else {

							theta=(T)(0.5*h/(aa(ip,iq)));

							t=(T)(1.0/(std::abs(theta)+sqrt(1.0+theta*theta)));

							if (theta < 0.0) t = -t;

						}

						c=(T)(1.0/sqrt(1+t*t));

						s=t*c;

						tau=(T)(s/(1.0+c));

						h=t*aa(ip,iq);

						z(ip) -= h;

						z(iq) += h;

						d(ip) -= h;

						d(iq) += h;

						aa(ip,iq)=(T)0.0;

						for (unsigned j=0;j<ip;j++)

							rot(aa,s,tau,j,ip,j,iq);	

						for (unsigned j=ip+1;j<iq;j++)

							rot(aa,s,tau,ip,j,j,iq);

						for (unsigned j=iq+1;j<n;j++)

							rot(aa,s,tau,ip,j,iq,j);

						for (unsigned j=0;j<n;j++)

							rot(v,s,tau,j,ip,j,iq);

						++nrot;

					}

				}

			}

			for (ip=0;ip<n;ip++) {

				b(ip) += z(ip);

				d(ip)=b(ip);

				z(ip)=(T)0.0;

			}

		}

		return false;
		//Too many iterations in routine jacobi
	
}



template <typename T>
struct Unsymmeig 
{
	int n;
	cgv::math::mat<T> a,zz;
	cgv::math::diag_mat<std::complex<T> > wri;
	cgv::math::vec<T> scale;
	cgv::math::vec<int> perm;
	bool yesvecs,hessen;

	Unsymmeig(const cgv::math::mat<T>&aa, bool yesvec=true, bool hessenb=false) :
		n(aa.nrows()), a(aa), zz(n,n,0.0), wri(n), scale(n), perm(n),
		yesvecs(yesvec), hessen(hessenb)
	{
		scale.ones();
		balance();
		if (!hessen) elmhes();
		if (yesvecs) {
			for (int i=0;i<n;i++)
				zz(i,i)=1.0;
			if (!hessen) eltran();
			hqr2();
			balbak();
			sortvecs();
		} else {
			hqr();
			sort();
		}
	}
	void balance();
	void elmhes();
	void eltran();
	void hqr();
	void hqr2();
	void balbak();
	void sort();
	void sortvecs();

	
};
template <typename T>
T SIGN(const T &a, const T &b)
	{return (T)(b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a));}
template <typename T>
void Unsymmeig<T>::balance()
{
	const T RADIX = std::numeric_limits<T>::radix;
	bool done=false;
	T sqrdx=RADIX*RADIX;
	while (!done) {
		done=true;
		for (int i=0;i<n;i++) {
			T r=0.0,c=0.0;
			for (int j=0;j<n;j++)
				if (j != i) {
					c += std::abs(a(j,i));
					r += std::abs(a(i,j));
				}
			if (c != 0.0 && r != 0.0) {
				T g=r/RADIX;
				T f=1.0;
				T s=c+r;
				while (c<g) {
					f *= RADIX;
					c *= sqrdx;
				}
				g=r*RADIX;
				while (c>g) {
					f /= RADIX;
					c /= sqrdx;
				}
				if ((c+r)/f < 0.95*s) 
				{
					done=false;
					g=1.0/f;
					scale[i] *= f;
					for (int j=0;j<n;j++) a(i,j) *= g;
					for (int j=0;j<n;j++) a(j,i) *= f;
				}
			}
		}
	}
}
template <typename T>
void Unsymmeig<T>::balbak()
{
	for (int i=0;i<n;i++)
		for (int j=0;j<n;j++)
			zz(i,j) *= scale[i];
}

template <typename T>
void Unsymmeig<T>::elmhes()
{
	for (int m=1;m<n-1;m++) {
		T x=0.0;
		int i=m;
		for (int j=m;j<n;j++) 
		{
			if (std::abs(a(j,m-1)) > std::abs(x)) 
			{
				x=a(j,m-1);
				i=j;
			}
		}
		perm[m]=i;
		if (i != m) 
		{
			for (int j=m-1;j<n;j++) std::swap(a(i,j),a(m,j));
			for (int j=0;j<n;j++) std::swap(a(j,i),a(j,m));
		}
		if (x != 0.0) 
		{
			for (i=m+1;i<n;i++) 
			{
				T y=a(i,m-1);
				if (y != 0.0)
				{
					y /= x;
					a(i,m-1)=y;
					for (int j=m;j<n;j++) a(i,j) -= y*a(m,j);
					for (int j=0;j<n;j++) a(j,m) += y*a(j,i);
				}
			}
		}
	}
}

template <typename T>
void Unsymmeig<T>::eltran()
{
	for (int mp=n-2;mp>0;mp--) 
	{
		for (int k=mp+1;k<n;k++)
			zz(k,mp)=a(k,mp-1);
		int i=perm[mp];
		if (i != mp) {
			for (int j=mp;j<n;j++) {
				zz(mp,j)=zz(i,j);
				zz(i,j)=0.0;
			}
			zz(i,mp)=1.0;
		}
	}
}

template <typename T>
void Unsymmeig<T>::hqr()
{
	int nn,m,l,k,j,its,i,mmin;
	T z,y,x,w,v,u,t,s,r,q,p,anorm=0.0;

	const T EPS=std::numeric_limits<T>::epsilon();
	for (i=0;i<n;i++)
		for (j=std::max(i-1,0);j<n;j++)
			anorm += std::abs(a(i,j));
	nn=n-1;
	t=0.0;
	while (nn >= 0) {
		its=0;
		do {
			for (l=nn;l>0;l--) {
				s=std::abs(a(l-1,l-1))+std::abs(a(l,l));
				if (s == 0.0) s=anorm;
				if (std::abs(a(l,l-1)) <= EPS*s) 
				{
					a(l,l-1) = 0.0;
					break;
				}
			}
			x=a(nn,nn);
			if (l == nn) {
				wri[nn--]=x+t;
			} else {
				y=a(nn-1,nn-1);
				w=a(nn,nn-1)*a(nn-1,nn);
				if (l == nn-1) 
				{
					p=0.5*(y-x);
					q=p*p+w;
					z=sqrt(std::abs(q));
					x += t;
					if (q >= 0.0) {
						z=p+SIGN(z,p);
						wri[nn-1]=wri[nn]=x+z;
						if (z != 0.0) wri[nn]=x-w/z;
					} else {
						wri[nn]=std::complex<T>(x+p,-z);
						wri[nn-1]=conj(wri[nn]);
					}
					nn -= 2;
				} else {
					if (its == 30) throw("Too many iterations in hqr");
					if (its == 10 || its == 20) {
						t += x;
						for (i=0;i<nn+1;i++) a(i,i) -= x;
						s=std::abs(a(nn,nn-1))+std::abs(a(nn-1,nn-2));
						y=x=0.75*s;
						w = -0.4375*s*s;
					}
					++its;
					for (m=nn-2;m>=l;m--) {
						z=a(m,m);
						r=x-z;
						s=y-z;
						p=(r*s-w)/a(m+1,m)+a(m,m+1);
						q=a(m+1,m+1)-z-r-s;
						r=a(m+2,m+1);
						s=std::abs(p)+std::abs(q)+std::abs(r);
						p /= s;
						q /= s;
						r /= s;
						if (m == l) break;
						u=std::abs(a(m,m-1))*(std::abs(q)+std::abs(r));
						v=std::abs(p)*(std::abs(a(m-1,m-1))+std::abs(z)+std::abs(a(m+1,m+1)));
						if (u <= EPS*v) break;
					}
					for (i=m;i<nn-1;i++) {
						a(i+2,i)=0.0;
						if (i != m) a(i+2,i-1)=0.0;
					}
					for (k=m;k<nn;k++) {
						if (k != m) {
							p=a(k,k-1);
							q=a(k+1,k-1);
							r=0.0;
							if (k+1 != nn) r=a(k+2,k-1);
							if ((x=std::abs(p)+std::abs(q)+std::abs(r)) != 0.0) {
								p /= x;
								q /= x;
								r /= x;
							}
						}
						if ((s=SIGN(sqrt(p*p+q*q+r*r),p)) != 0.0) {
							if (k == m) {
								if (l != m)
								a(k,k-1) = -a(k,k-1);
							} else
								a(k,k-1) = -s*x;
							p += s;
							x=p/s;
							y=q/s;
							z=r/s;
							q /= p;
							r /= p;
							for (j=k;j<nn+1;j++)
							{
								p=a(k,j)+q*a(k+1,j);
								if (k+1 != nn) {
									p += r*a(k+2,j);
									a(k+2,j) -= p*z;
								}
								a(k+1,j) -= p*y;
								a(k,j) -= p*x;
							}
							mmin = nn < k+3 ? nn : k+3;
							for (i=l;i<mmin+1;i++) 
							{
								p=x*a(i,k)+y*a(i,k+1);
								if (k+1 != nn) 
								{
									p += z*a(i,k+2);
									a(i,k+2) -= p*r;
								}
								a(i,k+1) -= p*q;
								a(i,k) -= p;
							}
						}
					}
				}
			}
		} while (l+1 < nn);
	}
}

template <typename T>
void Unsymmeig<T>::hqr2()
{
	int nn,m,l,k,j,its,i,mmin,na;
	T z,y,x,w,v,u,t,s,r,q,p,anorm=0.0,ra,sa,vr,vi;

	const T EPS=std::numeric_limits<T>::epsilon();
	for (i=0;i<n;i++)
		for (j=std::max(i-1,0);j<n;j++)
			anorm += std::abs(a(i,j));
	nn=n-1;
	t=0.0;
	while (nn >= 0) {
		its=0;
		do {
			for (l=nn;l>0;l--) {
				s=std::abs(a(l-1,l-1))+std::abs(a(l,l));
				if (s == 0.0) s=anorm;
				if (std::abs(a(l,l-1)) <= EPS*s) {
					a(l,l-1) = 0.0;
					break;
				}
			}
			x=a(nn,nn);
			if (l == nn) {
				wri(nn)=a(nn,nn)=x+t;
				nn--;
			} else {
				y=a(nn-1,nn-1);
				w=a(nn,nn-1)*a(nn-1,nn);
				if (l == nn-1) {
					p=0.5*(y-x);
					q=p*p+w;
					z=sqrt(std::abs(q));
					x += t;
					a(nn,nn)=x;
					a(nn-1,nn-1)=y+t;
					if (q >= 0.0) {
						z=p+SIGN(z,p);
						wri[nn-1]=wri[nn]=x+z;
						if (z != 0.0) wri[nn]=x-w/z;
						x=a(nn,nn-1);
						s=std::abs(x)+std::abs(z);
						p=x/s;
						q=z/s;
						r=sqrt(p*p+q*q);
						p /= r;
						q /= r;
						for (j=nn-1;j<n;j++) {
							z=a(nn-1,j);
							a(nn-1,j)=q*z+p*a(nn,j);
							a(nn,j)=q*a(nn,j)-p*z;
						}
						for (i=0;i<=nn;i++) {
							z=a(i,nn-1);
							a(i,nn-1)=q*z+p*a(i,nn);
							a(i,nn)=q*a(i,nn)-p*z;
						}
						for (i=0;i<n;i++) {
							z=zz(i,nn-1);
							zz(i,nn-1)=q*z+p*zz(i,nn);
							zz(i,nn)=q*zz(i,nn)-p*z;
						}
					} else {
						wri[nn]=std::complex<T>(x+p,-z);
						wri[nn-1]=std::conj(wri[nn]);
					}
					nn -= 2;
				} else {
					if (its == 30) throw("Too many iterations in hqr");
					if (its == 10 || its == 20) 
					{
						t += x;
						for (i=0;i<nn+1;i++) a(i,i) -= x;
						s=std::abs(a(nn,nn-1))+std::abs(a(nn-1,nn-2));
						y=x=0.75*s;
						w = -0.4375*s*s;
					}
					++its;
					for (m=nn-2;m>=l;m--) 
					{
						z=a(m,m);
						r=x-z;
						s=y-z;
						p=(r*s-w)/a(m+1,m)+a(m,m+1);
						q=a(m+1,m+1)-z-r-s;
						r=a(m+2,m+1);
						s=std::abs(p)+std::abs(q)+std::abs(r);
						p /= s;
						q /= s;
						r /= s;
						if (m == l) break;
						u=std::abs(a(m,m-1))*(std::abs(q)+std::abs(r));
						v=std::abs(p)*(std::abs(a(m-1,m-1))+std::abs(z)+std::abs(a(m+1,m+1)));
						if (u <= EPS*v) break;
					}
					for (i=m;i<nn-1;i++) {
						a(i+2,i)=0.0;
						if (i != m) a(i+2,i-1)=0.0;
					}
					for (k=m;k<nn;k++) {
						if (k != m) {
							p=a(k,k-1);
							q=a(k+1,k-1);
							r=0.0;
							if (k+1 != nn) r=a(k+2,k-1);
							if ((x=std::abs(p)+std::abs(q)+std::abs(r)) != 0.0) {
								p /= x;
								q /= x;
								r /= x;
							}
						}
						if ((s=SIGN(sqrt(p*p+q*q+r*r),p)) != 0.0) {
							if (k == m) {
								if (l != m)
								a(k,k-1) = -a(k,k-1);
							} else
								a(k,k-1) = -s*x;
							p += s;
							x=p/s;
							y=q/s;
							z=r/s;
							q /= p;
							r /= p;
							for (j=k;j<n;j++) {
								p=a(k,j)+q*a(k+1,j);
								if (k+1 != nn) {
									p += r*a(k+2,j);
									a(k+2,j) -= p*z;
								}
								a(k+1,j) -= p*y;
								a(k,j) -= p*x;
							}
							mmin = nn < k+3 ? nn : k+3;
							for (i=0;i<mmin+1;i++) {
								p=x*a(i,k)+y*a(i,k+1);
								if (k+1 != nn) {
									p += z*a(i,k+2);
									a(i,k+2) -= p*r;
								}
								a(i,k+1) -= p*q;
								a(i,k) -= p;
							}
							for (i=0; i<n; i++) {
								p=x*zz(i,k)+y*zz(i,k+1);
								if (k+1 != nn) {
									p += z*zz(i,k+2);
									zz(i,k+2) -= p*r;
								}
								zz(i,k+1) -= p*q;
								zz(i,k) -= p;
							}
						}
					}
				}
			}
		} while (l+1 < nn);
	}
	if (anorm != 0.0) {
		for (nn=n-1;nn>=0;nn--) {
			p=real(wri[nn]);
			q=imag(wri[nn]);
			na=nn-1;
			if (q == 0.0) {
				m=nn;
				a(nn,nn)=1.0;
				for (i=nn-1;i>=0;i--) {
					w=a(i,i)-p;
					r=0.0;
					for (j=m;j<=nn;j++)
						r += a(i,j)*a(j,nn);
					if (imag(wri[i]) < 0.0) {
						z=w;
						s=r;
					} else {
						m=i;
						
						if (imag(wri[i]) == 0.0) {
							t=w;
							if (t == 0.0)
								t=EPS*anorm;
							a(i,nn)=-r/t;
						} else {
							x=a(i,i+1);
							y=a(i+1,i);
							q=cgv::math::sqr(real(wri[i])-p)+cgv::math::sqr(imag(wri[i]));
							t=(x*s-z*r)/q;
							a(i,nn)=t;
							if (std::abs(x) > std::abs(z))
								a(i+1,nn)=(-r-w*t)/x;
							else
								a(i+1,nn)=(-s-y*t)/z;
						}
						t=std::abs(a(i,nn));
						if (EPS*t*t > 1)
							for (j=i;j<=nn;j++)
								a(j,nn) /= t;
					}
				}
			} else if (q < 0.0) {
				m=na;
				if (std::abs(a(nn,na)) > std::abs(a(na,nn))) {
					a(na,na)=q/a(nn,na);
					a(na,nn)=-(a(nn,nn)-p)/a(nn,na);
				} else {
					std::complex<T> temp=std::complex<T>(0.0,-a(na,nn))/std::complex<T>(a(na,na)-p,q);
					a(na,na)=real(temp);
					a(na,nn)=imag(temp);
				}
				a(nn,na)=0.0;
				a(nn,nn)=1.0;
				for (i=nn-2;i>=0;i--) {
					w=a(i,i)-p;
					ra=sa=0.0;
					for (j=m;j<=nn;j++) {
						ra += a(i,j)*a(j,na);
						sa += a(i,j)*a(j,nn);
					}
					if (imag(wri[i]) < 0.0) {
						z=w;
						r=ra;
						s=sa;
					} else {
						m=i;
						if (imag(wri[i]) == 0.0) {
							std::complex<T> temp = std::complex<T>(-ra,-sa)/std::complex<T>(w,q);
							a(i,na)=real(temp);
							a(i,nn)=imag(temp);
						} else {
							x=a(i,i+1);
							y=a(i+1,i);
							vr=cgv::math::sqr(real(wri(i))-p)+cgv::math::sqr(imag(wri(i)))-q*q;
							vi=2.0*q*(real(wri(i))-p);
							if (vr == 0.0 && vi == 0.0)
								vr=EPS*anorm*(std::abs(w)+std::abs(q)+std::abs(x)+std::abs(y)+std::abs(z));
							std::complex<T> temp=std::complex<T>(x*r-z*ra+q*sa,x*s-z*sa-q*ra)/
								std::complex<T>(vr,vi);
							a(i,na)=real(temp);
							a(i,nn)=imag(temp);
							if (std::abs(x) > std::abs(z)+std::abs(q)) {
								a(i+1,na)=(-ra-w*a(i,na)+q*a(i,nn))/x;
								a(i+1,nn)=(-sa-w*a(i,nn)-q*a(i,na))/x;
							} else {
								std::complex<T> temp=std::complex<T>(-r-y*a(i,na),-s-y*a(i,nn))/
									std::complex<T>(z,q);
								a(i+1,na)=real(temp);
								a(i+1,nn)=imag(temp);
							}
						}
					}
					t=std::max(std::abs(a(i,na)),std::abs(a(i,nn)));
					if (EPS*t*t > 1)
						for (j=i;j<=nn;j++) {
							a(j,na) /= t;
							a(j,nn) /= t;
						}
				}
			}
		}
		for (j=n-1;j>=0;j--)
			for (i=0;i<n;i++) {
				z=0.0;
				for (k=0;k<=j;k++)
					z += zz(i,k)*a(k,j);
				zz(i,j)=z;
			}
	}
}
template <typename T>
void Unsymmeig<T>::sort()
{
	int i;
	for (int j=1;j<n;j++)
	{
		std::complex<T> x=wri[j];
		for (i=j-1;i>=0;i--) {
			if (std::real(wri[i]) >= real(x)) break;
			wri[i+1]=wri[i];
		}
		wri[i+1]=x;
	}
}

template <typename T>
void Unsymmeig<T>::sortvecs()
{
	int i;
	cgv::math::vec<T> temp(n);
	for (int j=1;j<n;j++) {
		std::complex<T> x=wri[j];
		for (int k=0;k<n;k++)
			temp[k]=zz(k,j);
		for (i=j-1;i>=0;i--) {
			if (real(wri[i]) >= std::real(x)) break;
			wri[i+1]=wri[i];
			for (int k=0;k<n;k++)
				zz(k,i+1)=zz(k,i);
		}
		wri[i+1]=x;
		for (int k=0;k<n;k++)
			zz(k,i+1)=temp[k];
	}

}






//compute eigenvalues of a which is a real unsymmetric matrix
//if a is still in hessenberg form set hessenb to true (default is false)
template <typename T>
void eig_unsym(const cgv::math::mat<T> &a,cgv::math::diag_mat<std::complex<T> >& eigvals, bool hessenb=false)
{
	Unsymmeig<T> eigsolver((cgv::math::mat<T>)a,false,hessenb);
	eigvals = eigsolver.wri;

}


//compute eigenvectors and eigenvalues of matrix a which is a real unsymmetric matrix
//if a is still in hessenberg form set hessenb to true (default is false)
//eigenvectors are not normalized
template <typename T>
void eig_unsym(const cgv::math::mat<T>& a,cgv::math::mat<T>& eigvecs,cgv::math::diag_mat<std::complex<T> >& eigvals,bool normalize=true, bool hessenb=false)
{
	cgv::math::mat<T> A=a;
	Unsymmeig<T> eigsolver(A,true,hessenb);
	eigvals = eigsolver.wri;
	if(normalize)
	{
		for(unsigned i = 0; i < eigsolver.zz.ncols();i++)
			eigsolver.zz.set_col(i,cgv::math::normalize(eigsolver.zz.col(i)));
	}
	eigvecs=eigsolver.zz;

}

}}
