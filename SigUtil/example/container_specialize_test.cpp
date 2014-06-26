#include "container_specialize_test.h"
#include "../lib/traits/container_traits.hpp"

// ���[�U��`�N���X�ɃC�e���[�^��Ԃ�begin, end�����o�֐����`���A
// sig���O��Ԃœ��ꉻ���邱�ƂŃ��C�u�������̃R���e�i���󂯎��֐����g�p�\�ɂȂ�
template <class T>
class MyVector3
{
	std::array<T, 3> vec_;
	
public:
	explicit MyVector3(T x, T y, T z) : vec_{{ x, y, z }} {}

	auto begin() const ->decltype(vec_.begin()){ return vec_.begin(); }
	auto end() const ->decltype(vec_.end()){ return vec_.end(); }

	size_t size() const{ return 3; }	// �������������������
};

namespace sig
{
	// �R���e�i�̐����ɍ��킹�āA�K�؂ɓ��ꉻ����N���X��I�Ԃ��Ƃ𐄏�
	template<template<class> class C, class T>
	struct static_container_traits<C<T>>
	{
		static const bool exist = true;		// ���ꉻ�ł��Ă��邩

		using value_type = T;

		template<class U>
		using rebind = C<U>;

		static void add_element(C<T>& c, const T& t)
		{
			c.push_back(t);
		}
	};

	// ���ꉻ�����N���X��container_traits�Ɍp��(�o�^���)
	template<class... Args>
	struct container_traits<MyVector3<Args...>> : public static_container_traits<MyVector3<Args...>>
	{};
}


void ContainerSpecializeTest()
{
	MyVector3<double> vec1(1, 1.5, 2.5);
	MyVector3<int> vec2(1, -1, 1);

	auto euc = sig::euclidean_distance(vec1, vec2);
	assert(sig::equal(euc, sqrt(pow(2.5, 2)+pow(1.5, 2))));
}
