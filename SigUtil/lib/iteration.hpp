/*
Copyright(c) 2014 Akihiro Nishimura

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/

#ifndef SIG_UTIL_FOREACH_HPP
#define SIG_UTIL_FOREACH_HPP

#include "helper.hpp"
#include "container_helper.hpp"

/* ���䂢�Ƃ���Ɏ肪�͂����������֐� */

namespace sig
{
#undef max
#undef min

	// �����R���e�i�𔽕����� (�擪���珇�ԁA�C���f�b�N�X�͋���)
	// �����œn���֐��I�u�W�F�N�g(�⃉���_)�̈������Q��(&)�ɂ��邱�ƂŕύX������\
	template <class F, class... Cs>
	auto for_each(F const& func, Cs&... containers)
	{
		const uint length = min(containers.size()...);
		iterative_assign(length, func, std::begin(containers)...);
	}
	
	// �����R���e�i�𔽕����� (�擪���珇�ԁA�C���f�b�N�X�͋���)
	// 1��������1���C���N�������g�����ϐ������� (init�͂��̏����l)
	// �����œn���֐��I�u�W�F�N�g(�⃉���_)�̈������Q��(&)�ɂ��邱�ƂŕύX������\
	template <class F, class... Cs>
	auto for_each(F const& func, int init, Cs&... containers)
	{
		const uint length = min(containers.size()...);
		iterative_assign(length, init, func, std::begin(containers)...);
	}


	// �R���e�i�ւ̑�����Z (element-wise: container and container)
	template <class C1, class C2, typename std::enable_if<container_traits<C1>::exist && container_traits<C2>::exist>::type*& = enabler>
	void compound_assignment(
		std::function<void(typename container_traits<C1>::value_type&, ParamType<typename container_traits<C2>::value_type>)> const& assign_op,
		C1& dest,
		C2 const& src)
	{
		for_each(assign_op, dest, src);
	}

	// �R���e�i�ւ̑�����Z (element-wise: container and scalar)
	template <class C, class T, typename std::enable_if<container_traits<C>::exist && !container_traits<T>::exist>::type*& = enabler>
	void compound_assignment(
		std::function<void(typename container_traits<C>::value_type&, ParamType<T>)> const& assign_op,
		C& dest,
		T src)
	{
		for (auto& e : dest){
			assign_op(e, src);
		}
	}
}

#endif