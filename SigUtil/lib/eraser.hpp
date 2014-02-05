#ifndef __SIG_UTIL_ERASER__
#define __SIG_UTIL_ERASER__

#include "sigutil.hpp"
#include <iterator>

/* �폜�E�t�B���^�֘A */

namespace sig{

	//�R���e�i�̗v�f����d���������̂��폜
	//data: �R���e�i
	//need_removes: �폜�����v�f��߂�l�Ŏ󂯎�邩
	//return -> �폜�v�f
	template <class Container>
	Container RemoveDuplicates(Container& data, bool need_removes)
	{
		using T = typename container_traits<Container>::value_type;
		std::unordered_map<T, bool> rmv;
		Container result, removed;

		for (auto it = std::begin(data), end = std::end(data); it != end;){
			if (!rmv.count(*it)){
				container_traits<Container>::add_element(result, std::move(*it));
				rmv[*it] = true;
				++it;
				continue;
			}
			else if(need_removes){
				container_traits<Container>::add_element(removed, std::move(*it));
			}
			++rmv[*it];
			it = data.erase(it);
			end = data.end();
		}

		data = std::move(result);
		return std::move(removed);
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