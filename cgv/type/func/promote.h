#pragma once 

#include <cgv/type/func/drop_const.h>
#include <cgv/type/func/make_const.h>
#include <cgv/type/ctrl/if_.h>

namespace cgv {
	namespace type {
		namespace func {
			namespace Promote {
				/// the promote rank template defines a rank for each type which is used to automatically detect the promoted type of a pair of types
				template <typename T> struct promotion_rank { enum { rank = 1000 }; };
				template <> struct promotion_rank<bool>           { enum { rank =   0 }; };
				template <> struct promotion_rank<char>           { enum { rank =  10 }; };
				template <> struct promotion_rank<unsigned char>  { enum { rank =  20 }; };
				template <> struct promotion_rank<short>          { enum { rank =  30 }; };
				template <> struct promotion_rank<unsigned short> { enum { rank =  40 }; };
				template <> struct promotion_rank<int>            { enum { rank =  50 }; };
				template <> struct promotion_rank<unsigned int>   { enum { rank =  60 }; };
				template <> struct promotion_rank<long>           { enum { rank =  70 }; };
				template <> struct promotion_rank<unsigned long>  { enum { rank =  80 }; };
				template <> struct promotion_rank<float>          { enum { rank =  90 }; };
				template <> struct promotion_rank<double>         { enum { rank = 100 }; };
			}
			/** determine the type that should be used to represent the result of an operator or function applies to two different types */
			template <typename T1, typename T2> 
			struct promote
			{
				static const bool favour_first = 
					(unsigned int) Promote::promotion_rank<T1>::rank >(unsigned int) Promote::promotion_rank<T2>::rank;
				typedef typename ctrl::if_<favour_first, T1, T2>::type type;
			};
			template <> struct promote<int,float>			{ typedef double type; };
			template <> struct promote<float,int>			{ typedef double type; };
			template <> struct promote<unsigned int,float>  { typedef double type; };
			template <> struct promote<float,unsigned int>  { typedef double type; };
			template <> struct promote<long,float>		    { typedef double type; };
			template <> struct promote<float,long>		    { typedef double type; };
			template <> struct promote<unsigned long,float> { typedef double type; };
			template <> struct promote<float,unsigned long> { typedef double type; };
		}
	}
}
