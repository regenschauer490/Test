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


}

#endif