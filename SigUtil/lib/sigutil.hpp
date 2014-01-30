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

#include <string>
#include <vector>

#include <stdio.h>
#include <iostream>
#include <locale>
#include <fstream>
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
#include <chrono>
#include <numeric>
#include <regex>
#include <utility>

#if SIG_ENABLE_BOOST

#include <boost/optional.hpp>
#include <boost/format.hpp>
#include <boost/call_traits.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/serialization/serialization.hpp>

#endif

namespace sig{
#undef max
#undef min

	/* typedef */
	typedef unsigned long uint;
	typedef std::shared_ptr< std::string > StrPtr;
	typedef std::shared_ptr< std::string const > C_StrPtr;
	typedef std::shared_ptr< std::wstring > WStrPtr;
	typedef std::shared_ptr< std::wstring const > C_WStrPtr;
	
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
	template <class T> struct MaybeReturn{ typedef maybe<T> type; };
	template <class T> auto Nothing(T const& default_value)-> decltype(nothing){ return nothing; }
#else
	template <class T> struct MaybeReturn{ typedef T type; };
	template <class T> T Nothing(T&& default_value){ return std::forward<T>(default_value); }
#endif


	template <class Container, class Sfinae = void> struct ContainerConstructor{ typedef Container type; };
	template <class Container> struct ContainerConstructor<Container, typename std::enable_if<std::is_array<Container>::value>::type>{ typedef std::array<std::remove_extent<Container>, std::rank<Container>::value> type; };
	
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


/* �֐��^�v���O���~���O */

	//[a] -> (a -> r) -> [r]
	//�߂�l�̌^R�͖����I�Ɏw�肪�K�v
/*	template <class Out, class In, class Func, typename std::enable_if<HasBegin<Out>(0)>::type*& = enabler>
	Out Map(In const& list, Func func)
	{
		Out result;
		//Reserve(result, list.size());
		std::transform(std::begin(list), std::end(list), std::inserter(result, std::begin(result)), func);
		return std::move(result);
	}

	template <class Out, class In, class Func, typename std::enable_if<!HasBegin<Out>(0)>::type*& = enabler>
	std::vector<Out> Map(In const& list, Func func)
	{
		std::vector<Out> result;
		//Reserve(result, list.size());
		std::transform(std::begin(list), std::end(list), std::inserter(result, std::begin(result)), func);
		return std::move(result);
	}
*/

	template <class R, class A, template <class T, class = std::allocator<T>> class Container>
	Container<R> Map(Container<A> const& list, std::function<typename std::common_type<R>::type(typename std::common_type<A>::type)> const& func)
	{
		Container<R> result;
		Reserve(result, list.size());
		std::transform(list.begin(), list.end(), std::back_inserter(result), func);
		return std::move(result);
	}
	
	template <class R, class A, std::size_t N, template <class T, size_t N> class Array>
	Array<R, N> Map(Array<A, N> const& list, std::function<typename std::common_type<R>::type(typename std::common_type<A>::type)> const& func)
	{
		Array<R, N> result;
		std::transform(list.begin(), list.end(), result.begin(), func);
		return result;
	}

	template <class R, class A, std::size_t N>
	std::array<R, N> Map(A const(&list)[N], std::function<typename std::common_type<R>::type(typename std::common_type<A>::type)> const& func)
	{
		std::array<R, N> result;
		std::transform(list, list + N, result.begin(), func);
		return result;
	}

	//[a] -> [b] -> (a -> b -> r) -> [r]
	//�߂�l�̌^R�́A�����I�Ɏw�肷��K�v����
	template < class R, class A, class B, template < class T, class = std::allocator<T>> class Container>
	Container<R> ZipWith(Container<A> const& list1, Container<B> const& list2, std::function<typename std::common_type<R>::type(typename std::common_type<A>::type, typename std::common_type<B>::type)> const& func)
	{
		const uint length = list1.size() < list2.size() ? list1.size() : list2.size();
		Container<R> result;

		uint i = 0;
		for (auto it1 = list1.begin(), it2 = list2.begin(), end1 = list1.end(), end2 = list2.end(); i < length; ++i, ++it1, ++it2) result.push_back(func(*it1, *it2));

		return std::move(result);
	}

#if SIG_ENABLE_BOOST
	//[a] -> b -> (a -> b -> r) -> [r]
	//�߂�l�̌^R�́A�����I�Ɏw�肷��K�v����
	template < class R, class A, class B, template < class T, class = std::allocator<T >> class Container>
		Container<R> ZipWith(Container<A> const& list1, typename boost::call_traits<B>::param_type val, std::function<typename std::common_type<R>::type(typename std::common_type<A>::type, typename std::common_type<B>::type)> const& func)
	{
		Container<R> result;

		uint i = 0;
		for (auto it1 = list1.begin(), end1 = list1.end(); i < list1.size(); ++i, ++it1) result.push_back(func(*it1, val));

		return std::move(result);
	}
#endif

	//[a] -> b -> (a -> common<a,b> -> common<a,b>) -> common<a,b>
	//std::accumulate�Ƃ͈Ⴂ�A�����l�̌^B�ł͂Ȃ�A��B���ÖٓI�ɕϊ������^�ɏW��
	template < class A, class B, template < class T, class = std::allocator<T>> class Container>
		typename std::common_type<A, B>::type Accumulate(Container<A> const& list, B init, std::function<typename std::common_type<A>::type(typename std::common_type<A>::type, typename std::common_type<A, B>::type)> const& func){
			typename std::common_type<A, B>::type result = init;
			for(auto const& e : list)  result = func(e, result);
			return result;
		}


/* ���� */

	//���������Ɏw�肵���͈͂̈�l���z�����𔭐�������N���X
	//�f�t�H���g: ���������� -> �����Z���k�c�C�X�^�[
	template <class NumType, class Engine = std::mt19937>
	class SimpleRandom {
		Engine _engine;		//���������A���S���Y�� 
		typename std::conditional <
			std::is_integral<NumType>::value,
			std::uniform_int_distribution<int>,
			std::uniform_real_distribution<double>
		> ::type _dist;		//�m�����z

	public:
		SimpleRandom(NumType min, NumType max, bool debug) : _engine(
			[debug](){
				std::random_device rnd;
				std::vector<uint> v(10);
				if (debug) std::fill(v.begin(), v.end(), 0);
				else std::generate(v.begin(), v.end(), std::ref(rnd));

				return Engine(std::seed_seq(v.begin(), v.end()));
		}()
			),
			_dist(min, max){}

		NumType operator()(){
			return _dist(_engine);
		}
	};


	//�d���̖������������𐶐�
	template < template < class T, class = std::allocator<T>> class Container = std::vector >
	Container<int> RandomUniqueNumbers(std::size_t n, int min, int max, bool debug) {
		std::unordered_set<int> match;
		Container<int> result;
		static SimpleRandom<int> Rand(0, max - min, debug);

		int r;
		for (int i = 0; i < n; ++i){
			do{
				r = min + Rand();
			} while (match.find(r) != match.end());

			match.insert(r);
			result.push_back(r);
		}

		return std::move(result);
	}


/* �֗��֐� */

	//xor
	inline bool BoolXor(bool A, bool B){ return (A && !B) || (!A && B); }

	//A��B�̐^�U��v��true��Ԃ� (�� !xor)
	inline bool BoolConsistency(bool A, bool B){ return (A && B) || (!A && !B); }

	//���������_�^�ɂ��g���铙�l��r�֐�
	template <class T1, class T2>
	bool Equal(T1 v1, T2 v2)
	{
		const auto dmin = std::numeric_limits<std::common_type<T1, T2>::type>::min();

		return std::abs(v1 - v2) < dmin;
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

	//�\�[�g�O��index��ێ����ă\�[�g
	template <class T>
	std::vector< std::tuple<uint, T> > SortWithIndex(std::vector<T> const& vec, bool const small_to_large)
	{
		std::vector< std::tuple<uint, T> > result(vec.size());

		for (uint i = 0; i < vec.size(); ++i){
			std::get<0>(result[i]) = i;
			std::get<1>(result[i]) = vec[i];
		}
		if (small_to_large) std::sort(result.begin(), result.end(), [](std::tuple<uint, T> const& a, std::tuple<uint, T> const& b){ return std::get<1>(a) < std::get<1>(b); });
		else std::sort(result.begin(), result.end(), [](std::tuple<uint, T> const& a, std::tuple<uint, T> const& b){ return std::get<1>(b) < std::get<1>(a); });

		return std::move(result);
	}

	//�R���e�i�̗v�f���V���b�t��
	template <class T, template < class T_, class = std::allocator<T_>> class Container>
	void Shuffle(Container<T>& data)
	{
		static SimpleRandom<double> myrand(0.0, 1.0, false);
		std::random_shuffle(data.begin(), data.end(), [&](uint max)->uint{ return myrand() * max; });
	}

	//2�̃R���e�i�̗v�f��Ή������Ȃ���\�[�g
	template < class T1, class T2, template < class T_, class = std::allocator<T_>> class Container1, template < class T_, class = std::allocator<T_>> class Container2>
	void Shuffle(Container1<T1>& data1, Container2<T2>& data2)
	{
		uint size = std::min(data1.size(), data2.size());
		auto rnum = RandomUniqueNumbers(size, 0, size-1, false);
		auto copy1 = std::move(data1);
		auto copy2 = std::move(data2);

		data1.resize(copy1.size());
		data2.resize(copy2.size());
		for (uint i=0; i<size; ++i){
			data1[rnum[i]] = std::move(copy1[i]);
			data2[rnum[i]] = std::move(copy2[i]);
		}
	}

#if SIG_ENABLE_BOOST

	//�����ɍœK�Ȓl�Ƃ���Index��T��.�@comp(��r�Ώےl, �b��ŏ��l)
	template < class T, class CMP, template < class T, class = std::allocator<T>> class Container >
	maybe<std::tuple<T, uint>> SearchIndex(Container<T> const& src, CMP comp)
	{
		if (src.empty()) return nothing;
		
		T val = src[0];
		uint index = 0;

		for (uint i = 0, size = src.size(); i < size; ++i){
			if (comp(src[i], val)){
				val = src[i];
				index = i;
			}
		}

		return std::make_tuple(val, index);
	}

#endif

	//�R���e�i�ւ̑�����Z ([a], [b], (a -> b -> a))
	template < class T1, class T2, template < class T_, class = std::allocator<T_>> class Container>
		void CompoundAssignment(Container<T1>& list1, Container<T2> const& list2, std::function<typename std::common_type<T1>::type(typename std::common_type<T1>::type, typename std::common_type<T2>::type)> const& op)
	{
		const uint length = list1.size() < list2.size() ? list1.size() : list2.size();

		for (uint i = 0; i < length; ++i) list1[i] = op(list1[i], list2[i]);
	}

	//�R���e�i�ւ̑�����Z ([a], b, (a -> b -> a))
	template < class T1, class T2, template < class T_, class = std::allocator<T_>> class Container>
		void CompoundAssignment(Container<T1>& list1, T2 const& v, std::function<typename std::common_type<T1>::type(typename std::common_type<T1>::type, typename std::common_type<T2>::type)> const& op)
	{
		for (uint i = 0, length = list1.size(); i < length; ++i) list1[i] = op(list1[i], v);
	}

	//�l���w���������
	template < class T, template < class T_, class = std::allocator<T_>> class Container = std::vector>
		Container<T> Fill(T const& value, uint count)
	{
		Container<T> tmp;
		tmp.reserve(count);
		for (uint i = 0; i < count; ++i) tmp.push_back(value);
		return std::move(tmp);
	}

	//�����֐���ʂ��Ēl�𐶐�����
	//args -> generator: �����֐�.�����̓��[�vindex
	template < class T, template < class T_, class = std::allocator<T_>> class Container = std::vector>
		Container<T> Generate(std::function<T(int)> const& generator, uint count)
	{
		Container<T> tmp;
		tmp.reserve(count);
		for (uint i = 0; i < count; ++i) tmp.push_back(generator(i));
		return std::move(tmp);
	}

/* �֗��A�C�e�� */

#if SIG_ENABLE_BOOST
	//�p�[�Z���g�^
	class Percent
	{
		int _percent;

	public:
		explicit Percent(int percent) : _percent(percent){}

		int GetPercent() const{ return _percent; }
		double GetDouble() const{ return _percent * 0.01; }

		static Percent const& Unit(){ static const Percent unit(100); return unit; }

		Percent operator=(Percent src){ _percent = src._percent; return *this; }
		Percent operator=(int src){ _percent = src; return *this; }
		
		bool operator==(Percent obj) const{ return _percent == obj._percent; }

		bool operator!=(Percent obj) const{ return _percent != obj._percent; }

	private:
		friend class boost::serialization::access;

		template <class Archive>
		void serialize(Archive& ar, unsigned int version)
		{
			ar & _percent;
		}

		template <class Archive>
		friend void save_construct_data(Archive & ar, Percent const* p, unsigned int version){};

		template <class Archive>
		friend void load_construct_data(Archive & ar, Percent* p, unsigned int version){
			::new(p) Percent(0);
		}
	};

#endif

	//�^�C���E�H�b�`
	class TimeWatch{
		typedef std::chrono::system_clock::time_point TIME;
		TIME st, ed;
	
		void Init(){
			ed = std::chrono::system_clock::now();
			st = std::chrono::system_clock::now();
		}

	public:
		TimeWatch(){ Init(); }

		void ReStart(){ Init(); }

		void Stop(){
			ed = std::chrono::system_clock::now();
		}
		
		//template<class TimeUnit = std::chrono::milliseconds> VC++2012 ���Ή�
		template<class TimeUnit>
		long long GetTime(){
			return std::chrono::duration_cast<TimeUnit>(ed - st).count();
		}
	};
		
	//�q�X�g�O����
	//template <�v�f�̌^, �x��>
	template <class T, size_t BIN_NUM>
	class Histgram{
		T const _min;
		T const _max;
		double const _delta;
		std::array<uint, BIN_NUM + 2> _count;	//[0]: x < min, [BIN_NUM-1]: max <= x
		uint _num;

	public:
		//�v�f�͈̔͂��w��
		Histgram(T min, T max) : _min(min), _max(max), _delta(((double) max - min) / BIN_NUM), _num(0){
			assert(_delta > 0);
			for (auto& ct : _count) ct = 0;
		}

		//�v�f��bin�ɐU�蕪���ăJ�E���g
		void Count(T value){
			for (uint i = 0; i < BIN_NUM + 1; ++i){
				if (value < _delta*i + _min){
					++_num;
					++_count[i];
					return;
				}
			}
			++_count[BIN_NUM + 1];
		}

		template < template < class TT, class = std::allocator<TT >> class Container>
			void Count(Container<T> const& values){
				for (auto e : values) Count(e);
			}

			//bin�O�̗v�f�����݂�����
			bool IsOverRange() const{ return _count[0] || _count[BIN_NUM + 1]; }

			//double GetAverage() const{ return std::accumulate(_count.begin(), _count.end(), 0, [](T total, T next){ return total + next; }) / static_cast<double>(_num); }

			//�p�x���擾
			std::array<uint, BIN_NUM> GetCount() const{
				std::array<uint, BIN_NUM> tmp;
				for (uint i = 0; i < BIN_NUM; ++i) tmp[i] = _count[i + 1];
				return std::move(tmp);
			}

#if SIG_ENABLE_BOOST
			//bin�Ԗ�(0 �` BIN_NUM-1)�̕p�x���擾
			//return -> tuple<�p�x, �͈͍ŏ��l(�ȏ�), �͈͍ő�l(����)>
			maybe<std::tuple<uint, int, int>> GetCount(uint bin) const{ return bin < BIN_NUM ? maybe < std::tuple < uint, int, int >> (std::make_tuple(_count[bin + 1], _delta*bin + _min, _delta*(bin + 1) + _min)) : nothing; }
#else
			std::tuple<uint,int,int> GetCount(uint bin) const{ return bin < BIN_NUM ? std::make_tuple(_count[bin+1], _delta*bin+_min, _delta*(bin+1)+_min) : throw std::out_of_range("Histgram::Get, bin=" + std::to_string(bin)); }
#endif

			void Print() const{
				auto IsPlus = [](double v){ return v < 0 ? false : true; };

				auto IntDigit = [](double v){ return log10(v) + 1; };

				auto Space = [](int num){
					std::string space;
					for (int i = 0; i < num; ++i) space.append(" ");
					return std::move(space);
				};

				int const rketa = IntDigit(_max);
				int const disp_precision = rketa > 2 ? 0 : 2-rketa;
				int const keta = std::max(rketa, std::min((int) Precision(_delta), disp_precision) + 2);
				int const ctketa = log10(*std::max_element(_count.begin(), _count.end())) + 1;
				T const dbar = _num < 100 ? 1.0 : _num*0.01;

				/*
				std::string offset1, offset2;
				if (keta < 3) offset1.append(2 - keta, ' ');
				else offset2.append(keta - 3, ' ');*/

				std::cout << "\n-- Histgram --\n";
				for (int i = 0; i < BIN_NUM + 2; ++i){
					auto low = _delta*(i - 1) + _min;
					auto high = _delta*i + _min;

					if (i == 0) std::cout << std::fixed << std::setprecision(disp_precision) << "\n[-��" << Space(keta-2) << "," << std::setw(keta + 1) << high << ")" << "�F" << std::setw(ctketa) << _count[i] << " ";
					else if (i == BIN_NUM + 1) std::cout << std::fixed << std::setprecision(disp_precision) << "\n[" << std::setw(keta + 1) << low << ",+��" << Space(keta - 2) << ")" << "�F" << std::setw(ctketa) << _count[i] << " ";
					else std::cout << std::fixed << std::setprecision(disp_precision) << "\n[" << std::setw(keta+1) << low << "," << std::setw(keta+1) << high << ")" << "�F" << std::setw(ctketa) << _count[i] << " ";
					
					for (int j = 1; dbar*j <= _count[i]; ++j) printf("|");
				}
				std::cout << std::resetiosflags(std::ios_base::floatfield) << "\n\n";
			}
	};

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

	// �폜�֘A�̊֐��Q
	namespace Eraser{

		#define Sig_Eraser_RemoveDuplicates_Impl(Container) \
			auto end = std::unique(list.begin(), list.end());\
			auto removes = need_removes ? Container(end, list.end()) : Container();\
			list.erase(end, list.end())\


		//�R���e�i�̗v�f����d���������̂��폜
		//args -> list: �R���e�i, need_removes: �폜�����v�f��߂�l�Ŏ󂯎�邩, is_sorted: �R���e�i���\�[�g�ς݂� 
		//return -> �폜�v�f
		template <class T, template<class T_, class = std::allocator<T_>> class Container>
		inline Container<T> RemoveDuplicates(Container<T>& list, bool need_removes, bool is_sorted = false)
		{
			if (!is_sorted) std::sort(list.begin(), list.end());

			Sig_Eraser_RemoveDuplicates_Impl(Container<T>);

			return std::move(removes);
		}
		template < class T>
		inline std::list<T> RemoveDuplicates(std::list<T>& list, bool need_removes, bool is_sorted = false)
		{
			if (!is_sorted) list.sort();

			Sig_Eraser_RemoveDuplicates_Impl(std::list<T>);

			return std::move(removes);
		}
	
		#if SIG_ENABLE_BOOST
		#define Sig_Eraser_ParamType1 typename boost::call_traits<T>::param_type
		#else
		#define Sig_Eraser_ParamType1 typename std::common_type<T>::type const&
		#endif

		//�R���e�i����w��v�f��1�폜
		//args -> list: �R���e�i, remove: �폜�v�f
		//return -> �폜�v�f�����݂�����
		template <class T, template<class T_, class = std::allocator<T_>> class Container >
		inline bool RemoveOne(Container<T>& list, Sig_Eraser_ParamType1 remove)
		{
			for(auto it =list.begin(), end = list.end(); it != end;){
				if(*it == remove){
					list.erase(it);
					return true;
				}
				else ++it;
			}
			return false;
		}

		//�R���e�i����q������𖞂����v�f��1�폜
		//args -> list: �R���e�i, remove_pred: �폜���ʊ֐�
		//return -> �폜�v�f�����݂�����
		template <class Pred, class T, template<class T_, class = std::allocator<T_>> class Container >
		inline bool RemoveOneIf(Container<T>& list, Pred remove_pred)
		{
			for(auto it =list.begin(), end = list.end(); it != end;){
				if(remove_pred(*it)){
					list.erase(it);
					return true;
				}
				else ++it;
			}
			return false;
		}

		//�R���e�i����w��v�f��S�폜
		//args -> list: �R���e�i, remove: �폜�v�f
		//return -> �폜�v�f�����݂�����
		template < class T, template < class T_, class = std::allocator<T_>> class Container >
		inline bool RemoveAll(Container<T>& list, Sig_Eraser_ParamType1 remove)
		{
			uint presize = list.size();
			if (!list.empty()) list.erase(std::remove(list.begin(), list.end(), remove), list.end());
			return presize != list.size();
		}

		//�R���e�i����q������𖞂����v�f��S�폜
		//args -> list: �R���e�i, remove_pred: �폜���ʊ֐�
		//return -> �폜�v�f�����݂�����
		template <class Pred, class T, template<class T_, class = std::allocator<T_>> class Container >
		inline bool RemoveAllIf(Container<T>& list, Pred remove_pred)
		{
			uint presize = list.size();
			if(!list.empty()) list.erase( std::remove_if(list.begin(), list.end(), remove_pred), list.end());
			return presize != list.size();
		}
	}


/* �W������ */
	namespace Set{

		//vector, list �̐ϏW�������߂�(�v�f����1��). [����v���FT::operator==()]
		template <class T, template<class T_, class = std::allocator<T_>> class Container>
		Container<T> SetIntersection(Container<T> const& src1, Container<T> const& src2)
		{
			Container<T> result;

			for(T const& e1 : src1){
				for(T const& e2 : src2){
					if(e1 == e2 && [&result, &e2]()->bool{
						for(T const& r : result){
							if(r == e2) return false;
						}
						return true;
					}()){
						result.push_back(e2);
					}
				}
			}
			return move(result);
		}

		//unordered_set �̐ϏW�������߂�.[����v���FT::operator==()]
		template <class T>
		std::unordered_set<T> SetIntersection(std::unordered_set<T> const& src1, std::unordered_set<T> const& src2)
		{
			std::unordered_set<T> result;

			for(T const& e1 : src1){
				for(T const& e2 : src2){
					if(e1 == e2) result.insert(e2);
				}
			}
			return move(result);
		}

		//unordered_map �̐ϏW�������߂�(bool key ? �L�[�Ŕ�r�y��1�����̗v�f���擾�z : ������v). [����v���FK::operator==(), V::operator==()]
		template <class K, class V>
		std::unordered_map<K,V> SetIntersection(std::unordered_map<K,V> const& src, std::unordered_map<K,V> const& other, bool const key)
		{
			std::unordered_map<K,V> result;

			for(auto const& e : src){
				for(auto const& o : other){
					if(key && e.first == o.first) result.insert(e);
					else if(e == o) result.insert(e);
				}
			}
			return move(result);
		}


		//vector, list �̍��W�������߂�(�v�f����1��). [����v���FT::operator==()]
		template <class T, template<class T_, class = std::allocator<T_>> class Container>
		Container<T> SetDifference(Container<T> const& src1, Container<T> const& src2)
		{
			Container<T> result, sum(src1);
			sum.insert(sum.end(), src2.begin(), src2.end());

			auto intersection = SetIntersection(src1, src2);

			for(T const& s : sum){
				if([&intersection, &s]()->bool{
					for(T const& i : intersection){
						if(s == i) return false;
					}
					return true;
				}() && [&result, &s]()->bool{
					for(T const& r : result){
						if(s == r) return false;
					}
					return true;
				}()	){
					result.push_back(s);
				}
			}
			return move(result);
		}

		//unordered_set �̍��W�������߂�.[����v���FT::operator==()]
		template <class T>
		std::unordered_set<T> SetDifference(std::unordered_set<T> const& src1, std::unordered_set<T> const& src2)
		{
			std::unordered_set<T> result, sum(src1);
			sum.insert(src2.begin(), src2.end());

			auto intersection = SetIntersection(src1, src2);

			for(T const& s : sum){
				if([&intersection, &s]()->bool{
					for(T const& i : intersection){
						if(s == i) return false;
					}
					return true;
				}()){
					result.insert(s);
				}
			}
			return move(result);
		}

		//unordered_map �̍��W�������߂�(bool key ? �L�[�Ŕ�r : ������v). [����v���FK::operator==(), V::operator==()]
		template <class K, class V>
		std::unordered_map<K,V> SetDifference(std::unordered_map<K,V> const& src1, std::unordered_map<K,V> const& src2, bool const key)
		{
			std::unordered_map<K,V> result, sum(src1);
			sum.insert(src2.begin(), src2.end());

			auto intersection = SetIntersection(src1, src2, key);

			for(auto const& s : sum){
				if([&intersection, &s, key]()->bool{
					for(auto const& i : intersection){
						if(key && s.first == i.first) return false;
						else if(!key && s == i) return false;
					}
					return true;
				}()){
					result.insert(s);
				}
			}
			return move(result);
		}

	}

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

#endif UTILITY