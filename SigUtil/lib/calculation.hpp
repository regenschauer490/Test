/*
Copyright(c) 2014 Akihiro Nishimura

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/

#ifndef SIG_UTILUTIL_CALCULATION_HPP
#define SIG_UTILUTIL_CALCULATION_HPP

#include "sigutil.hpp"
#include "functional.hpp"

namespace sig
{
	//�񍀉��Z (scalar and scalar)
	template <class OP, class T1, class T2>
	auto BinaryOperation(OP func, T1 v1, T2 v2) ->decltype(v1 + v2)
	{
		return func(v1, v2);
	}

	//�񍀉��Z (element-wise: container and container)
	template <class OP, class C1, class C2>
	auto BinaryOperation(OP func, C1 const& c1, C2 const& c2)
		->typename container_traits<C1>::template rebind<decltype(eval(
		func,
		std::declval<typename container_traits<C1>::value_type>(),
		std::declval<typename container_traits<C2>::value_type>()
		))>
	{
		using T1 = typename container_traits<C1>::value_type;
		using T2 = typename container_traits<C2>::value_type;
		return ZipWith([](ParamType<T1> v1, ParamType<T2> v2){ return v1 + v2; }, c1, c2);
	}

	//�񍀉��Z (element-wise: container and scalar)
	template <class OP, class C, class T, class = decltype(std::declval<typename container_traits<C>::value_type>() + std::declval<T>())>
	auto BinaryOperation(OP func, C const& c, T v)
		->typename container_traits<C>::template rebind<decltype(eval(
		func,
		std::declval<typename container_traits<C>::value_type>(),
		v
		))>
	{
		using CT = typename container_traits<C>::value_type;
		using RT = decltype(eval(func, std::declval<typename container_traits<C>::value_type>(), v));
		using R = typename container_traits<C>::template rebind<RT>;

		R r;
		for (ParamType<CT> vc : c) container_traits<R>::add_element(r, vc + v);
		return std::move(r);
	}

	//�񍀉��Z (element-wise: scalar and container)
	template <class OP, class T, class C, class = decltype(std::declval<typename container_traits<C>::value_type>() + std::declval<T>())>
	auto BinaryOperation(OP func, T v, C const& c)
		->typename container_traits<C>::template rebind<decltype(eval(
		func,
		std::declval<typename container_traits<C>::value_type>(),
		v
		))>
	{
		return Plus(c, v);
	}


	/* �����揜�̉��Z����ʓI�ɋL�q���邽�߂̊֐��Q */

#define SIG_MakeBinaryOperation(FunctionName, Operator)\
	template <class T1, class T2>\
	auto FunctionName(T1 v1, T2 v2) ->decltype(v1 Operator v2)\
	{\
		return v1 Operator v2;\
	}\
\
	template <class C1, class C2, class = typename container_traits<C1>::value_type, class = typename container_traits<C2>::value_type>\
	auto FunctionName(C1 const& c1, C2 const& c2)\
	{\
		using T1 = typename container_traits<C1>::value_type;\
		using T2 = typename container_traits<C2>::value_type;\
		return BinaryOperation([](T1 v1, T2 v2){ return v1 Operator v2; }, c1, c2);\
	}\
\
	template <class C, class T, class = decltype(std::declval<typename container_traits<C>::value_type>() Operator std::declval<T>())>\
	auto FunctionName(C const& c, T v)\
	{\
		using CT = typename container_traits<C>::value_type;\
		return BinaryOperation([](CT v1, T v2){ return v1 Operator v2; }, c, v);\
	}\
\
	template <class T, class C, class = decltype(std::declval<typename container_traits<C>::value_type>() Operator std::declval<T>())>\
	auto FunctionName(T v, C const& c)\
	{\
		return FunctionName(c, v);\
	}\

	SIG_MakeBinaryOperation(Plus, +);

	SIG_MakeBinaryOperation(Minus, -);

	SIG_MakeBinaryOperation(Mult, *);

	SIG_MakeBinaryOperation(Div, / );

}
#endif