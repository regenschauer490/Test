﻿/*
Copyright(c) 2014 Akihiro Nishimura

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/

#ifndef SIG_UTIL_FUNCTIONAL_HPP
#define SIG_UTIL_FUNCTIONAL_HPP

#include "helper.hpp"
#include "container_helper.hpp"

/* 関数型プログラミング サポート */

namespace sig
{
	//n引数高階関数
	template <class F, class C1, class... Cs>
	auto variadicZipWith(F const& func, C1 const& container1, Cs const&... containers)
	{
		using OutputType = typename container_traits<C1>::template rebind<decltype(eval(
			func,
			std::declval<typename container_traits<C1>::value_type>(),
			std::declval<typename container_traits<Cs>::value_type>()...
		))>;

		OutputType result;
		const uint length = Min(container1.size(), containers.size()...);
		Iterate1(length, result, func, std::begin(container1), std::begin(containers)...);

		return result;
	}

	//(a -> b) -> [a] -> [b]
	//1引数高階関数
	template <class F, class C>
	auto map(F const& func, C const& container)
	{
		return variadicZipWith(func, container);
	}


	//(a -> b -> c) -> [a] -> [b] -> [c]
	//2引数高階関数
	template <class F, class C1, class C2>
	auto zipWith(F const& func, C1 const& container1, C2 const& container2)
	{
		return variadicZipWith(func, container1, container2);
	}


	// (a -> b -> a) -> a -> [b] -> a
	// リストの先頭からたたみ込み
	template <class F, class T, class C>
	auto foldl(F func, T init, C container)
	{
		return std::accumulate(std::begin(container), std::end(container), init, func);
	}

	// (a -> b -> b) -> b -> [a] -> b
	// リストの末尾からたたみ込み
	template <class F, class T, class C>
	auto foldr(F func, T init, C container)
	{
		return std::accumulate(std::rbegin(container), std::rend(container), init, func);
	}


	//[a] -> [b] -> ... -> [(a, b, ...)]
	//複数のコンテナから、タプルのコンテナを作る (第1引数のコンテナが戻り値のコンテナとなる)
	template <class... Cs
#ifndef SIG_MSVC_LT1800
		, typename std::enable_if< And(container_traits<Cs>::exist...) >::type*& = enabler
#endif
		>
	auto zip(Cs const&... containers)
	{
		return variadicZipWith([](typename container_traits<Cs>::value_type const&... vs){
			return std::make_tuple(vs...);
		}, containers...);
	}


#ifndef SIG_MSVC_LT1800

	//for rvalue reference
	template <class C1, class... Cs,
		typename std::enable_if<!std::is_lvalue_reference<C1>::value && !And(std::is_lvalue_reference<Cs>::value...)>::type*& = enabler,
		typename std::enable_if< And(container_traits<Cs>::exist...) >::type*& = enabler
	>
	auto zip(C1&& container1, Cs&&... containers)
	{
		using OutputType = typename container_traits<C1>::template rebind<
			std::tuple<typename container_traits<C1>::value_type, typename container_traits<Cs>::value_type...>
		>;

		const uint length = Min(container1.size(), containers.size()...);
		OutputType result;
		Iterate1(length, result, [](typename container_traits<C1>::value_type&& v1, typename container_traits<Cs>::value_type&&... vs){
			return std::make_tuple(std::move(v1), std::move(vs)...);
		}, std::make_move_iterator(std::begin(container1)), std::make_move_iterator(std::begin(containers))...);

		return result;
	}

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 8)
	template <class TC, size_t... I>
	auto ZipImpl_(TC const& t_containers, std::index_sequence<I...>)
	{
		return zip(std::get<I>(t_containers)...);
	}
	template <class TC, size_t... I>
	auto ZipImpl_(TC&& t_containers, std::index_sequence<I...>)
	{
		return zip(std::get<I>(std::forward<TC>(t_containers))...);
	}

	//([a], [b], ...) -> [(a, b, ...)]
	//コンテナのタプルから、タプルのコンテナを作る
	template <class... Cs, typename Indices = std::make_index_sequence<sizeof...(Cs)>
	auto zip(std::tuple<Cs...> const& t_containers)
	{
		return ZipImpl_(t_containers, Indices());
	}

	template <class... Cs, typename Indices = std::make_index_sequence<sizeof...(Cs)>
	auto zip(std::tuple<Cs...>&& t_containers)
	{
		return ZipImpl_(std::move(t_containers), Indices());
	}
#endif
#endif

	//[(a, b, ...)] -> [a0]
	//タプルのコンテナから、指定したコンテナを取り出す
	template <size_t Index, class CT>
	auto unzip(CT const& c_tuple)
	{
		using T = typename std::tuple_element<Index, typename container_traits<CT>::value_type>::type;
		using C = std::vector<T>;
		C result;

		for (auto const& e : c_tuple){
			container_traits<C>::add_element(result, std::get<Index>(e));
		}
		return result;
	}

	template <uint Index, class CT, typename std::enable_if<!std::is_lvalue_reference<CT>::value>::type*& = enabler>
	auto unzip(CT&& c_tuple)
	{
		using T = typename std::tuple_element<Index, typename container_traits<CT>::value_type>::type;
		using C = std::vector<T>;
		C result;

		for (auto& e : c_tuple){
			container_traits<C>::add_element(result, std::get<Index>(std::move(e)));
		}
		return result;
	}

	template<class CT, size_t I = 0, typename std::enable_if<I + 1 == std::tuple_size<typename container_traits<CT>::value_type>::value, void>::type*& = enabler>
	auto UnzipImpl_(CT const& c_tuple)
	{
		return std::make_tuple(unzip<I>(c_tuple));
	}
	template<class CT, size_t I = 0, typename std::enable_if<I + 1 < std::tuple_size<typename container_traits<CT>::value_type>::value, void>::type*& = enabler>
	auto UnzipImpl_(CT const& c_tuple)
	{
		return std::tuple_cat(std::make_tuple(unzip<I>(c_tuple)), UnzipImpl_<CT, I + 1>(c_tuple));
	}

	template<class CT, uint I = 0, typename std::enable_if<I + 1 == std::tuple_size<typename container_traits<CT>::value_type>::value, void>::type*& = enabler>
	auto UnzipImpl_(CT&& c_tuple)
	{
		return std::make_tuple(unzip<I>(std::forward<CT>(c_tuple)));
	}
	template<class CT, uint I = 0, typename std::enable_if<I + 1 < std::tuple_size<typename container_traits<CT>::value_type>::value, void>::type*& = enabler>
	auto UnzipImpl_(CT&& c_tuple)
	{
		return std::tuple_cat(std::make_tuple(unzip<I>(std::forward<CT>(c_tuple))), UnzipImpl_<CT, I + 1>(std::forward<CT>(c_tuple)));
	}

	//[(a, b, ...)] -> ([a], [b], ...)
	//タプルのコンテナから、コンテナのタプルを作る
	template <class CT>
	auto unzip(CT const& c_tuple)
	{
		return UnzipImpl_(c_tuple);
	}

	template <class CT, typename std::enable_if<!std::is_lvalue_reference<CT>::value>::type*& = enabler>
	auto unzip(CT&& c_tuple)
	{
		return UnzipImpl_(std::forward<CT>(c_tuple));
	}

	//uint -> a -> [a]
	//値を複製したコンテナを返す
	template <class T, class C = std::vector<T>>
	C replicate(uint n, T const& value)
	{
		C result;
		for (uint i = 0; i<n; ++i) container_traits<C>::add_element(result, value);
		return result;
	}

	template <class T, class C = std::vector<T>>
	C arithSequence(T st, T d, uint n)
	{
		C result;
		for (uint i = 0; i<n; ++i) container_traits<C>::add_element(result, st + i*d);
		return result;
	}

	//[a] -> [a]
	//コンテナの要素を逆転させたコンテナを返す
	template <class C>
	C reverse(C const& container)
	{
		C result = container;
		std::reverse(std::begin(result), std::end(result));
		return result;
	}

	//[a] -> [a] -> [a]
	//コンテナの結合
	template <class C>
	C merge(C const& container1, C const& container2)
	{
		auto result = container1;
		container_traits<C>::concat(result, container2);
		return result;
	}

	//[a] -> [b] -> [c]
	//コンテナの結合
	template <class C, class C1, class C2>
	C merge(C1 const& container1, C2 const& container2)
	{
		C result;
		for (auto v : container1) container_traits<C>::add_element(result, v);
		for (auto v : container2) container_traits<C>::add_element(result, v);
		return result;
	}

	//uint -> [a] -> [a]
	//コンテナの先頭からn個を取り出したコンテナを返す (順序の無いコンテナでは実装依存)
	template <class C>
	C take(uint n, C const& container)
	{
		C result;
		uint i = 0;
		for (auto it = std::begin(container); i < n; ++i, ++it) container_traits<C>::add_element(result, *it);
		return result;
	}

	//uint -> [a] -> [a]
	//コンテナの先頭からn個を削除したコンテナを返す (順序の無いコンテナでは実装依存)
	template <class C>
	C drop(uint n, C const& container)
	{
		C result;
		uint i = 0;
		auto it = std::begin(container), end = std::end(container);
		for (; i < n && it != end; ++i, ++it) ;
		for (; it != end; ++i, ++it) container_traits<C>::add_element(result, *it);
		return result;
	}

#ifndef SIG_MSVC_LT1800
	//(a -> a -> bool) -> [a] -> [a]
	//比較関数を指定してソート
	template <class F, class C, typename std::enable_if<HasRandomIter<C>::value, void>::type*& = enabler>
	auto sort(F const& binary_op, C const& data){
		C result = data;
		std::sort(std::begin(result), std::end(result), binary_op);
		return result;
	}
	template <class F, class C, typename std::enable_if<!HasRandomIter<C>::value, void>::type*& = enabler>
	auto sort(F const& binary_op, C const& data){
		C result = data;
		result.sort(binary_op);
		return result;
	}
#endif
}

#endif