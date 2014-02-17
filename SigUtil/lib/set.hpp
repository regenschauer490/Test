#ifndef __SIG_UTIL_SET__
#define __SIG_UTIL_SET__

#include "sigutil.hpp"
#include "tool.hpp"

/* �W������ */

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