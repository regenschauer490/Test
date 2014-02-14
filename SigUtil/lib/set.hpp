#ifndef __SIG_UTIL_SET__
#define __SIG_UTIL_SET__

#include "sigutil.hpp"
#include "tool.hpp"

/* �R���e�i�̗v�f�ɑ΂��Ă悭�s������ */

namespace sig
{



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


	//vector, list �̐ϏW�������߂�(�v�f����1��). [����v���FT::operator==()]
	template <class T, template<class T_, class = std::allocator<T_>> class Container>
	Container<T> SetIntersection(Container<T> const& src1, Container<T> const& src2)
	{
		Container<T> result;

		for (T const& e1 : src1){
			for (T const& e2 : src2){
				if (e1 == e2 && [&result, &e2]()->bool{
					for (T const& r : result){
						if (r == e2) return false;
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

		for (T const& e1 : src1){
			for (T const& e2 : src2){
				if (e1 == e2) result.insert(e2);
			}
		}
		return move(result);
	}

	//unordered_map �̐ϏW�������߂�(bool key ? �L�[�Ŕ�r�y��1�����̗v�f���擾�z : ������v). [����v���FK::operator==(), V::operator==()]
	template <class K, class V>
	std::unordered_map<K, V> SetIntersection(std::unordered_map<K, V> const& src, std::unordered_map<K, V> const& other, bool const key)
	{
		std::unordered_map<K, V> result;

		for (auto const& e : src){
			for (auto const& o : other){
				if (key && e.first == o.first) result.insert(e);
				else if (e == o) result.insert(e);
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

		for (T const& s : sum){
			if ([&intersection, &s]()->bool{
				for (T const& i : intersection){
					if (s == i) return false;
				}
				return true;
			}() && [&result, &s]()->bool{
				for (T const& r : result){
					if (s == r) return false;
				}
				return true;
			}()){
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

		for (T const& s : sum){
			if ([&intersection, &s]()->bool{
				for (T const& i : intersection){
					if (s == i) return false;
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
	std::unordered_map<K, V> SetDifference(std::unordered_map<K, V> const& src1, std::unordered_map<K, V> const& src2, bool const key)
	{
		std::unordered_map<K, V> result, sum(src1);
		sum.insert(src2.begin(), src2.end());

		auto intersection = SetIntersection(src1, src2, key);

		for (auto const& s : sum){
			if ([&intersection, &s, key]()->bool{
				for (auto const& i : intersection){
					if (key && s.first == i.first) return false;
					else if (!key && s == i) return false;
				}
				return true;
			}()){
				result.insert(s);
			}
		}
		return move(result);
	}


}

#endif