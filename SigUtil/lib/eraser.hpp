#ifndef __SIG_UTIL_ERASER__
#define __SIG_UTIL_ERASER__

#include "sigutil.hpp"
#include <iterator>

/* �폜�E�t�B���^�֘A */

namespace sig{

	//�R���e�i�̗v�f����d���������̂��폜
	//data: �R���e�i
	//need_removes: �폜�����v�f��߂�l�Ŏ󂯎�邩
	//is_sorted: �R���e�i���\�[�g�ς݂� 
	//return -> �폜�v�f
	template <class Container>
	Container RemoveDuplicates(Container& data, bool need_removes, bool is_sorted = false)
	{
		typedef typename Container::value_type T;
		std::unordered_map<T, bool> rmv;
		Container result, removed;

		for (auto& v : data){
			if (!rmv.count(v)){
				result.push_back(std::move(v));
				rmv[v] = true;
			}
			else if(need_removes){
				removed.push_back(std::move(v));
			}
			++rmv[v];
		}

		data = std::move(result);
		return std::move(removed);
	}
/*	template <class Container, typename std::enable_if< std::is_same<typename std::iterator_traits<typename Container::iterator>::iterator_category, std::random_access_iterator_tag >::value >::type*& = enabler>
	Container RemoveDuplicates(Container& data, bool need_removes, bool is_sorted = false)
	{
		if (!is_sorted) std::sort(std::begin(data), std::end(data));

		auto end = std::unique(std::begin(data), std::end(data));
		auto removes = need_removes ? Container(end, std::end(data)) : Container();
		data.erase(end, std::end(data));

		return std::move(removes);
	}
	template <class T>
	std::list<T> RemoveDuplicates(std::list<T>& data, bool need_removes, bool is_sorted = false)
	{
		if (!is_sorted) data.sort();

		auto end = std::unique(std::begin(data), std::end(data));
		auto removes = need_removes ? Container(end, std::end(data)) : Container();
		data.erase(end, std::end(data));

		return std::move(removes);
	}
*/

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
		for (auto it = list.begin(), end = list.end(); it != end;){
			if (*it == remove){
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
		for (auto it = list.begin(), end = list.end(); it != end;){
			if (remove_pred(*it)){
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
		if (!list.empty()) list.erase(std::remove_if(list.begin(), list.end(), remove_pred), list.end());
		return presize != list.size();
	}

}

#endif