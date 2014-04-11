/*
Copyright(c) 2014 Akihiro Nishimura

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/

#ifndef SIG_UTIL_HELPER_HPP
#define SIG_UTIL_HELPER_HPP

#include "sigutil.hpp"

/* 補助モジュール */

namespace sig
{
#undef max
#undef min

/* コンパイル時用 */

// 可変長 and
template <class B>
constexpr B And(B cond){
	return cond;
}
template <class Bf, class... Be>
constexpr Bf And(Bf cond, Be... conds){
	return cond && And(conds...);
}

// 可変長 or
template <class B>
constexpr B Or(B cond){
	return cond;
}
template <class Bf, class... Be>
constexpr Bf Or(Bf cond, Be... conds){
	return cond || Or(conds...);
}

// V1 > V2 -> true
template <class T1, class T2>
constexpr bool Greater(T1 v1, T2 v2){ return v1 > v2 ? true : false; };

// V1 < V2 -> true
template <class T1, class T2>
constexpr bool Less(T1 v1, T2 v2){ return v1 < v2 ? true : false; };


/* 実行時用 */

// xor
template <class B1, class B2>
inline bool Xor(B1 a, B2 b){ return (a && !b) || (!a && b); }

// AとBの真偽一致でtrueを返す (⇔ !xor)
template <class B1, class B2>
inline bool Consistency(B1 a, B2 b){ return (a && b) || (!a && !b); }


//2変数の差の絶対値を返す
template <class T1, class T2>
auto DeltaAbs(T1 v1, T2 v2) ->typename std::common_type<T1, T2>::type
{
	return v1 < v2 ? v2 - v1 : v1 - v2;
}

//浮動小数点型にも使える等値比較
template <class T1, class T2>
bool Equal(T1 v1, T2 v2)
{
	const auto dmin = std::numeric_limits<typename std::common_type<T1, T2>::type>::epsilon();

	return !(DeltaAbs(v1, v2) > dmin);
}

//指定範囲内の誤差を許した等値比較
template <class T1, class T2>
bool TolerantEqual(T1 v1, T2 v2, typename std::common_type<T1, T2>::type margin)
{
	return margin ? !(DeltaAbs(v1, v2) > margin) : Equal(v1, v2);
}
/*
//小数点以下の桁数取得 (ex: v=1.2300000 -> 2)
inline uint Precision(double v)
{
	uint keta = 0;
	double eps = 1.0 - std::numeric_limits<double>::epsilon();
	const uint max_keta = 15;
	const double dv = v - int(v);

	while (true){
		if (TolerantEqual(dv, 0, eps) || keta >= max_keta) break;
		eps /= 10;
		++keta;
	}

	return keta;
}*/

template <class T>
auto Min(T v)
{
	return v;
}
template <class T1, class T2>
auto Min(T1 v1, T2 v2)
{
	return std::min(v1, v2);
}
template <class T, class... Ts>
auto Min(T v1, Ts... vs)
{
	return std::min(v1, Min(vs...));
}


//範囲チェックと自動修正
template <class T, class U>
inline bool ModifyRange(T& val, U const& min, U const& max)
{
	if (val<min){ val = min; return false; }
	if (val>max){ val = max; return false; }
	return true;
}

template <class T, class U>
inline bool CheckRange(T const& val, U const& min, U const& max)
{
	if (val<min){ return false; }
	if (val>max){ return false; }
	return true;
}


template <class RC, class C>
auto Copy(C const& src) ->RC
{
	RC dest;
	for (auto const& e : src) container_traits<RC>::add_element(e);
	return dest;
}

}
#endif