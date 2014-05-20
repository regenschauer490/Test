/*
Copyright(c) 2014 Akihiro Nishimura

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/

#ifndef SIG_UTIL_DEBUG_HPP
#define SIG_UTIL_DEBUG_HPP

#include <assert.h>
#include "../lib/helper.hpp"

/* �f�o�b�O�p�c�[�� */

namespace sig
{

struct DebugEqual{
	template <class T1, class T2>
	bool operator()(T1 v1, T2 v2) const{
		//std::cout << "v1:" << v1 << ", v2:" << v2 << std::endl;
		assert(equal(v1, v2)); return equal(v1, v2); 
	}
};

};

#endif
