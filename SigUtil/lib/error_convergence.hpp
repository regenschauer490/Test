/*
Copyright(c) 2014 Akihiro Nishimura

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/

#ifndef SIG_UTIL_ERROR_CONVERGENCE_HPP
#define SIG_UTIL_ERROR_CONVERGENCE_HPP

#include "sigutil.hpp"
#include "distance/norm.hpp"

namespace sig
{
// �덷�(���)
struct AbsoluteError
{
	template <class T, class F>
	double operator()(T const& value, T const& last_value, F const& norm_func) const
	{
		return norm_func(value, last_value);
	}
};

const AbsoluteError absolute_error;

// �덷�(����)
struct RelativeError
{
	template <class T, class F>
	double operator()(T const& value, T const& last_value, F const& norm_func) const
	{
		return norm_func(value, last_value) / norm_func(last_valu);
	}
};

const RelativeError relative_error;


// ��������̊Ǘ�
class ManageConvergenceSimple
{
	const double epsilon_;
	sig::Just<double>::type last_value_;

public:
	// epsilon: ��������p�̒萔
	ManageConvergenceSimple(double epsilon) : epsilon_(epsilon), last_value_(Nothing(0)) {}

	// ��Ԃ̍X�V�Ǝ�������
	// return -> true(����), false(������)
	bool update(double value){
		if (last_value_ && sig::abs_delta(value, sig::fromJust(last_value_)) < epsilon_) return true;
		sig::fromJust(last_value_) = value;
		return false;
	}

	double get_value() const{ return last_value_ ? *last_value_ : -1; }
};

// ��������̌v�Z�ƊǗ�
template <class T, class C = RelativeError, class F = sig::Norm<2>>
class ManageConvergence
{
	const double epsilon_;
	const F norm_func_;
	const C criteria_;
	typename sig::Just<T>::type last_value_;

public:
	// epsilon: ��������p�̒萔, norm_function: �m�����֐�(1����,2������operator()���`�����֐��I�u�W�F�N�g)
	ManageConvergence(double epsilon, F norm_func_ = sig::norm_L2) : epsilon_(epsilon), norm_func_(norm_func), last_value_(Nothing(T())) {}

	// ��Ԃ̍X�V�Ǝ�������
	// return -> true(����), false(������)
	bool update(T value){
		if (last_value_ && criteria_(value, sig::fromJust(last_value), norm_func_) < epsilon_) return true;
		sig::fromJust(last_value_) = value;
		return false;
	}
};

}
#endif