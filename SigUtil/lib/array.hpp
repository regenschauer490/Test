/*
Copyright(c) 2014 Akihiro Nishimura

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/

#ifndef SIG_UTIL_ARRAY_HPP
#define SIG_UTIL_ARRAY_HPP

#include <array>
#include <initializer_list>

namespace sig
{
	//�ő�e�ʂ��Œ�̐ÓI�z��
	//std::array�̗��֐����g�����A�Œ蒷��std::vector�̊��o�Ŏg�p�ł���
	//��{�I�ɏ��������ɑS�v�f�ɒl������g������z�肵�Ă��邪�A�����I�ɖ����ɑ�����Ă������Ƃ��\
	//�����o�֐�begin()����end()�œ�����͈͂́A�擪����l���������Ă��閖���܂łƂȂ�([0]�`[N-1]�܂łł͂Ȃ�)
	template <class T, size_t N>
	class Array
	{
	public:
		using value_type = T;
		using reference = T&;
		using const_reference = T const&;
		using pointer = T*;
		using const_pointer = T const*;
		using iterator = typename std::array<T, N>::iterator;
		using const_iterator = typename std::array<T, N>::const_iterator;
		using reverse_iterator = typename std::array<T, N>::reverse_iterator;
		using const_reverse_iterator = typename std::array<T, N>::const_reverse_iterator;
		using size_type = size_t;
		using difference_type = typename std::array<T, N>::difference_type;
		using result_type = typename boost::call_traits<T>::value_type;
		using param_type = typename boost::call_traits<T>::param_type;

	private:
		std::array<T, N> array_;
		size_type tail_;				//���ݎg�p���Ă���Y������

	private:
		template <class OP, class C1, class C2>
		void CompoundAssignment_(OP const& assign_op, C1& dest, C2 const& src)
		{
			auto it1 = std::begin(dest), end1 = std::end(dest);
			auto it2 = std::begin(src), end2 = std::end(src);

			while (it1 != end1 && it2 != end2){
				assign_op(*it1, *it2);
				++it1;
				++it2;
			}
		}

		void RangeCheck_(size_type index){
			if (index >= tail_){
				throw out_of_range("invalid sig::array<T, N> subscript");
			}
		}

		//pos�ȍ~�̗v�f��1��ւ��炷
		iterator BackShift_(const_iterator pos){
			auto pit = rbegin();
			for (auto it = pit; pit.base() != pos; pit = it++){
				*pit = std::move(*it);
			}
			return pit.base();
		}

		//pos�ȍ~�̗v�f��1�O�ւ��炷
		iterator FrontShift_(iterator pos){
			iterator pit = pos;
			for (auto end = this->end(); pos != end; pit = pos++){
				*pit = std::move(*pos);
			}
			return pit;
		}

	public:
		Array() : tail_(0) {}

		Array(uint size, T val) : tail_(0){ for(uint i=0; i<size; ++i) push_back(val); }

		explicit Array(std::initializer_list<T> init) : tail_(0){
			assert(init.size() <= N);
			CompoundAssignment_([this](T& dest, ParamType<T> src){ dest = src; ++tail_; }, array_, init);
		}
		explicit Array(std::array<T,N> const& src) : tail_(0){
			CompoundAssignment_([this](T& dest, ParamType<T> src){ dest = src; ++tail_; }, array_, src);
		}

		Array(Array const& src) : tail_(src.tail_){
			CompoundAssignment_([this](T& dest, ParamType<T> src){ dest = src; }, array_, src.array_);
		}

		Array& operator=(Array src){
			swap(src);
			return *this;
		}
		Array& operator=(std::initializer_list<T> init){
			Array copy(init);
			swap(copy);
			return *this;
		}
		Array& operator=(std::array<T,N> const& src){
			Array copy(src);
			swap(copy);
			return *this;
		}

		iterator begin() noexcept{ return array_.begin(); }
		const_iterator begin() const noexcept{ return array_.begin(); }
		const_iterator cbegin() const noexcept{ return array_.cbegin(); }

		iterator end() noexcept{ return array_.begin() + tail_; }
		const_iterator end() const noexcept{ return array_.begin() + tail_; }
		const_iterator cend() const noexcept{ return array_.cbegin() + tail_; }

		reverse_iterator rbegin() noexcept{ return array_.rend() - tail_; }
		const_reverse_iterator rbegin() const noexcept{ return array_.rend() - tail_; }
		const_reverse_iterator crbegin() const noexcept{ return array_.crend() - tail_; }

		reverse_iterator rend() noexcept{ return array_.rend(); }
		const_reverse_iterator rend() const noexcept{ return array_.rend(); }
		const_reverse_iterator crend() const noexcept{ return array_.crend(); }

		reference at(size_type pos){ RangeCheck_(pos); return array_.at(pos); }
		const_reference at(size_type pos) const{ RangeCheck_(pos); return array_.at(pos); }

		//tail_ �ȍ~�̓Y���A�N�Z�X�͖��ۏ�
		reference operator [](size_type pos){ return array_[pos]; }
		const_reference operator [](size_type pos) const{ return array_[pos]; }

		reference front(){ return array_.front(); }
		const_reference front() const{ return array_.front(); }

		reference back(){ return array_[tail_]; }
		const_reference back() const{ return array_[tail_]; }

		bool empty() const noexcept{ return tail_ == 0; }

		size_type size() const noexcept{ return tail_; }

		size_type max_size() const noexcept{ return array_.max_size; }

		value_type* data() noexcept{ return array_.data(); }
		value_type const* data() const noexcept{ return array_.data(); }

		void fill(value_type const& val){ array_.fill(val); tail_ = N; }

		void swap(Array& other) /*noexcept(noexcept(swap(std::declval<value_type&>(), std::declval<value_type&>())))*/{
			array_.swap(other.array_); 
			std::swap(tail_, other.tail_);
		}

		void push_back(value_type const& val){ assert(tail_ < N); array_[tail_++] = val; }
		void push_back(value_type&& val){ assert(tail_ < N); array_[tail_++] = std::forward<value_type>(val); }

		void pop_back() noexcept{ if(tail_ > 0) --tail_; }

		iterator insert(const_iterator position, const value_type& val){
			assert(tail_ < N);
			++tail_;
			auto p = BackShift_(position);
			*p = val;
			return p;
		}
		iterator insert(const_iterator position, value_type&& val){
			assert(tail_ < N);
			++tail_;
			auto p = BackShift_(position);
			*p = std::forward<value_type>(val);
			return p;
		}

		iterator erase(iterator position){
			auto p = FrontShift_(position);
			--tail_;
			return p;
		}

		void clear() noexcept{ tail_ = 0; }
	};
}
#endif