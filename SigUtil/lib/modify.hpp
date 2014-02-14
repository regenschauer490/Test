/*
The MIT License(MIT)

Copyright(c) 2014 Akihiro Nishimura

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files(the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __SIG_UTIL_ERASER__
#define __SIG_UTIL_ERASER__

#include "sigutil.hpp"
#include <iterator>

/* �\�[�g�E�폜�Ȃǂ̃R���e�i�ɑ΂���ύX���� */

namespace sig
{
#if _MSC_VER > 1800
	template <class T, class D = void> struct RI{ static const bool value = false; };
	template <class T> struct RI<T, decltype(std::declval<T>()[0], void())>{ static const bool value = true; };

	template <class T, class D = void> struct CMPOP{ static const bool value = false; };
	template <class T> struct CMPOP<T, decltype(std::declval<T>() < std::declval<T>(), void())>{ static const bool value = true; };

	//�����ĕ��ʂȃ\�[�g�̃��b�p
	template <class Container, typename std::enable_if<RI<Container>::value>::type*& = enabler>
	void Sort(Container& data){
		std::sort(std::begin(data), std::end(data));
	}
	template <class Container, typename std::enable_if<!RI<Container>::value && CMPOP<Container>::value>::type*& = enabler>
	void Sort(Container& data){
		data.sort();
	}
#endif
	
	//�\�[�g�O��index��ێ����ă\�[�g
	//ex: [30, 50, -10, 0] -> ([-10, 0, 30, 50], [2, 3, 0, 1])
	template <class C>
	auto SortWithIndex(C const& container, bool small_to_large)
	{
		using T = typename container_traits<C>::value_type;
		std::tuple<C, std::vector<uint>> result;

		auto it = std::begin(vec);
		for (uint i = 0; i < vec.size(); ++i, ++it){
			std::get<0>(result[i]) = i;
			std::get<1>(result[i]) = *it;
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

	//2�̃R���e�i�̗v�f��Ή������Ȃ���V���b�t��
	template < class T1, class T2, template < class T_, class = std::allocator<T_>> class Container1, template < class T_, class = std::allocator<T_>> class Container2>
	void Shuffle(Container1<T1>& data1, Container2<T2>& data2)
	{
		uint size = std::min(data1.size(), data2.size());
		auto rnum = RandomUniqueNumbers(size, 0, size - 1, false);
		auto copy1 = std::move(data1);
		auto copy2 = std::move(data2);

		data1.resize(copy1.size());
		data2.resize(copy2.size());
		for (uint i = 0; i<size; ++i){
			data1[rnum[i]] = std::move(copy1[i]);
			data2[rnum[i]] = std::move(copy2[i]);
		}
	}


	//�R���e�i�̗v�f����d���������̂��폜
	//container: �����ΏۃR���e�i
	//need_removes: �폜�����v�f��߂�l�Ŏ󂯎�邩
	//return -> �폜�v�f
	template <class C>
	C RemoveDuplicates(C& container, bool need_removes)
	{
		using T = typename container_traits<C>::value_type;
		std::unordered_map<T, bool> rmv;
		C result, removed;

		for (auto it = std::begin(container), end = std::end(container); it != end;){
			if (!rmv.count(*it)){
				container_traits<C>::add_element(result, std::move(*it));
				rmv[*it] = true;
				++it;
				continue;
			}
			else if(need_removes){
				container_traits<C>::add_element(removed, std::move(*it));
			}
			++rmv[*it];
			it = container.erase(it);
			end = container.end();
		}

		container = std::move(result);
		return std::move(removed);
	}

#if SIG_ENABLE_BOOST
#define Sig_Eraser_ParamType1 typename boost::call_traits<typename container_traits<C>::value_type>::param_type
#else
#define Sig_Eraser_ParamType1 typename std::common_type<typename container_traits<C>::value_type>::type const&
#endif

	//�R���e�i����w��v�f��1�폜
	//container: �R���e�i
	//remove: �폜�v�f
	//return -> �폜�v�f�����݂�����
	template <class C>
	bool RemoveOne(C& container, Sig_Eraser_ParamType1 remove)
	{
		for (auto it = std::begin(container), end = std::end(container); it != end;){
			if (*it == remove){
				container.erase(it);
				return true;
			}
			else ++it;
		}
		return false;
	}

	//�R���e�i����q������𖞂����v�f��1�폜
	//container: �R���e�i
	//remove_pred: �폜���ʊ֐�
	//return -> �폜�v�f�����݂�����
	template <class Pred, class C>
	bool RemoveOneIf(C& container, Pred remove_pred)
	{
		for (auto it = std::begin(container), end = std::end(container); it != end;){
			if (remove_pred(*it)){
				container.erase(it);
				return true;
			}
			else ++it;
		}
		return false;
	}

	//�R���e�i����w��v�f��S�폜
	//container: �R���e�i
	//remove: �폜�v�f
	//return -> �폜�v�f�����݂�����
	template <class C>
	bool RemoveAll(C& container, Sig_Eraser_ParamType1 remove)
	{
		uint presize = container.size();
		if (!container.empty()) Erase(container, remove);
		return presize != container.size();
	}


	//�R���e�i����q������𖞂����v�f��S�폜
	//container: �R���e�i
	//remove_pred: �폜���ʊ֐�
	//return -> �폜�v�f�����݂�����
	template <class Pred, class C>
	bool RemoveAllIf(C& container, Pred remove_pred)
	{
		uint presize = container.size();
		if (!container.empty()) EraseIf(container, remove_pred);
		return presize != container.size();
	}

}

#endif