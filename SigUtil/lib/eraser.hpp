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

/* 削除・フィルタ関連 */

namespace sig{

	//コンテナの要素から重複したものを削除
	//container: 処理対象コンテナ
	//need_removes: 削除した要素を戻り値で受け取るか
	//return -> 削除要素
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

	//コンテナから指定要素を1つ削除
	//container: コンテナ
	//remove: 削除要素
	//return -> 削除要素が存在したか
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

	//コンテナから述語条件を満たす要素を1つ削除
	//container: コンテナ
	//remove_pred: 削除判別関数
	//return -> 削除要素が存在したか
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

	//コンテナから指定要素を全削除
	//container: コンテナ
	//remove: 削除要素
	//return -> 削除要素が存在したか
	template <class C>
	bool RemoveAll(C& container, Sig_Eraser_ParamType1 remove)
	{
		uint presize = container.size();
		if (!container.empty()) Erase(container, remove);
		return presize != container.size();
	}


	//コンテナから述語条件を満たす要素を全削除
	//container: コンテナ
	//remove_pred: 削除判別関数
	//return -> 削除要素が存在したか
	template <class Pred, class C>
	bool RemoveAllIf(C& container, Pred remove_pred)
	{
		uint presize = container.size();
		if (!container.empty()) EraseIf(container, remove_pred);
		return presize != container.size();
	}

}

#endif