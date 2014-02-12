#ifndef __SIG_UTIL_H__
#define __SIG_UTIL_H__

#define SIG_ENABLE_BOOST 1

#ifdef _WIN32
#define SIG_WINDOWS_ENV 1
#elif _WIN64
#define SIG_WINDOWS_ENV 1
#else
#define SIG_WINDOWS_ENV 0
#endif

#include <assert.h>
#include <string>
#include <vector>

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <random>
#include <functional>
#include <algorithm>
#include <array>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <numeric>
#include <regex>
#include <utility>

#include "container_helper.hpp"
#include "type_map.hpp"

#if SIG_ENABLE_BOOST

#include <boost/optional.hpp>
#include <boost/format.hpp>
#include <boost/call_traits.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>

#endif

namespace sig{
#undef max
#undef min

	/* typedef */
	using uint = unsigned long;
	using StrPtr = std::shared_ptr< std::string >;
	using C_StrPtr = std::shared_ptr< std::string const >;
	using WStrPtr = std::shared_ptr< std::wstring >;
	using C_WStrPtr = std::shared_ptr< std::wstring const >;

	using std::placeholders::_1;
	using std::placeholders::_2;
	
#if SIG_ENABLE_BOOST
	template <typename T>
	using maybe = boost::optional<T>;

	auto const nothing = boost::none;
#endif


/* �w���p�֐��E�w���p�N���X */
	struct NullType{};
	
	extern void* enabler;

	//maybe�̗L���E�����Ɋ֌W�Ȃ��L�q���邽�߂̂���
#if SIG_ENABLE_BOOST
	template <class T> struct Just{ typedef maybe<T> type; };
	template <class T> auto Nothing(T const& default_value)-> decltype(nothing){ return nothing; }
#else
	template <class T> struct Just{ typedef T type; };
	template <class T> T Nothing(T&& default_value){ return std::forward<T>(default_value); }
#endif

#ifdef _WIN32
	using FileString = std::wstring;
#elif _WIN64
	using FileString = std::wstring;
#else
	using FileString = std::string;
#endif

	//template <class T> struct NullDummy{ typedef T type; };
	//template <> struct NullDummy<void>{ typedef NullType type; };

	/*
#if _MSC_VER > 1800
	//container_traits�̊g��
	template<class C>
	struct map_associative_container_traits;

	template<template<class , class, class, class> class C, class K, class T, template<class> class O, class A>
	struct map_associative_container_traits<C<K, T, O<K>, A>>
	{
		using key_type = K;

		using value_type = T;

		using pair_type = std::pair<const K, T>;

		static void add_element(C<K, T, O<K>, A>& c, const pair_type& t)
		{
			c.insert(t);
		}

		static void concat(C<K, T, O<K>, A>& lhs, const C<K, T, O<K>, A>& rhs)
		{
			lhs.insert(rhs.begin(), rhs.end());
		}

		template<class U>
		using rebind = C<K, U, O<K>, typename A::template rebind<U>::other>;
	};

	template<class... Args>
	struct container_traits<std::map<Args...>> : public map_associative_container_traits<std::map<Args...>>
	{};

	template<class... Args>
	struct container_traits<std::multimap<Args...>> : public map_associative_container_traits<std::multimap<Args...>>
	{};
#endif
*/
	//template <class Container, class Sfinae = void> struct ContainerConstructor{ typedef Container type; };
	//template <class Container> struct ContainerConstructor<Container, typename std::enable_if<std::is_array<Container>::value>::type>{ typedef std::array<std::remove_extent<Container>, std::rank<Container>::value> type; };
	
	//template <class T, class D = void> struct HasBegin : std::true_type{};
	//template <class T> struct HasBegin<T, decltype(std::declval<T>().begin())> : std::false_type{};

	//template <class T> constexpr auto HasBegin(int) ->decltype(std::declval<T>().begin()){ return true; }
	//template <class T> constexpr bool HasBegin(...){ return false; }

	//template <typename T> constexpr auto has_reserve_method(int) -> decltype(std::declval<T>().reserve(0), bool()) { return true; }
	//template <typename T> constexpr bool has_reserve_method(...) { return false; }

	/*template <typename T> auto Reserve(T& t, size_t n) -> typename std::enable_if<has_reserve_method<T>(0), void>::type {
		std::cout << "true" << std::endl;
		t.reserve(n);
	}

	template <typename T> auto Reserve(T& t, size_t n) -> typename std::enable_if<!has_reserve_method<T>(0), void>::type {
		std::cout << "false" << std::endl;
	}*/
	
	/*template <typename T, typename std::enable_if<has_reserve_method<T>(0)>::type *& = enabler> void Reserve(T& t, size_t n){
		std::cout << "true" << std::endl;
		t.reserve(n);
	}

	template <typename T, typename std::enable_if<!has_reserve_method<T>(0)>::type *& = enabler> void Reserve(T& t, size_t n) {
		std::cout << "false" << std::endl;
	}*/

	template <class T> auto Reserve(T& t, size_t n) ->decltype(t.reserve(n), void()){ t.reserve(n); std::cout << "true" << std::endl; }
	template <class T> void Reserve(T& t, size_t n){ std::cout << "false" << std::endl; }


/* �֗��֐� */

	//xor
	inline bool BoolXor(bool A, bool B){ return (A && !B) || (!A && B); }

	//A��B�̐^�U��v��true��Ԃ� (�� !xor)
	inline bool BoolConsistency(bool A, bool B){ return (A && B) || (!A && !B); }


	template <class T1, class T2>
	auto DeltaAbs(T1 v1, T2 v2) ->typename std::common_type<T1, T2>::type
	{
		return v1 < v2 ? v2 - v1 : v1 - v2;
	}

	//���������_�^�ɂ��g���铙�l��r
	template <class T1, class T2>
	bool Equal(T1 v1, T2 v2)
	{
		const auto dmin = std::numeric_limits<typename std::common_type<T1, T2>::type>::min();

		return !(DeltaAbs(v1, v2) > dmin);
	}

	//�w��͈͓��̌덷�����������l��r
	template <class T1, class T2>
	bool TolerantEqual(T1 v1, T2 v2, typename std::common_type<T1, T2>::type margin)
	{
		return margin ? !(DeltaAbs(v1, v2) > margin) : Equal(v1, v2);
	}

	//�����_�ȉ��̌����擾 (ex: v=1.2300000 -> 2)
	inline uint Precision(double v)
	{
		uint keta = 0;
		double dv = v - int(v);
		
		while (true){
			if (Equal(dv, 0) || keta >= 15) break;
			v *= 10;
			dv = v - int(v);
			++keta;
		}

		return keta;
	}

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


/* �C���E�␳�E�ǉ��E�폜 */

	//�͈̓`�F�b�N�Ǝ����C��
	template <class T, class U>
	inline bool ModifyRange(T& val, U const& min, U const& max)
	{
		if(val<min){ val = min; return false; }
		if(val>max){ val = max; return false; }
		return true;
	}

	template <class T, class U>
	inline bool CheckRange(T const& val, U const& min, U const& max)
	{
		if(val<min){ return false; }
		if(val>max){ return false; }
		return true;
	}


/* �W������ */

/*	template < template<class T, class = std::allocator<T>> class Container >
	inline void Print(Container<std::string> const& container, char const* const delimiter = "\n")
	{
		std::copy(container.begin(), container.end(), std::ostream_iterator<std::string>(std::cout, delimiter));
	}

	template < template<class T, class = std::allocator<T>> class Container >
	inline void Print(Container<std::wstring> const& container, wchar_t const* const delimiter = L"\n")
	{
		std::copy(container.begin(), container.end(), std::ostream_iterator<std::wstring>(std::wcout, delimiter));
	}
*/

	inline void Print(std::string const& text, char const* const delimiter = "\n")
	{
		std::cout << text << delimiter;
	}

	inline void Print(std::wstring const& text, wchar_t const* const delimiter = L"\n")
	{
		std::wcout << text << delimiter;
	}

	template < class T, template < class T_, class = std::allocator<T_>> class Container, typename std::enable_if<!std::is_same<T, std::wstring>::value>::type*& = enabler>
	inline void Print(Container<T> const& container, char const* const delimiter = "\n")
	{
		std::copy(container.begin(), container.end(), std::ostream_iterator<T>(std::cout, delimiter));
	}

	template<template<class ...> class Container>
	inline void Print(Container<std::wstring> const& container, wchar_t const* const delimiter = L"\n")
	{
		std::copy(container.begin(), container.end(), std::ostream_iterator<std::wstring>(std::wcout, delimiter));
	}


}

#endif
