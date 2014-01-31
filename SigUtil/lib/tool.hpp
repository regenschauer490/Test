#ifndef __SIG_UTIL_TOOL__
#define __SIG_UTIL_TOOL__

#include "sigutil.hpp"
#include <chrono>

/* �֗��c�[�� */

namespace sig{

	//�^�C���E�H�b�`
	class TimeWatch{
		typedef std::chrono::system_clock::time_point Time;
		typedef std::tuple<Time, Time> Lap;

		Time st, ed;
		std::vector<Lap> laps;
		bool is_run;

	private:
		void Init(){
			ed = std::chrono::system_clock::now();
			st = std::chrono::system_clock::now();
			laps.clear();
		}

	public:
		TimeWatch(){
			Init();
			is_run = true;
		}

		//���������Ē�~
		void Reset(){
			Init();
			is_run = false;
		}

		//��~ (�{���b�v�L�^)
		void Stop(){
			if (is_run){
				ed = std::chrono::system_clock::now();
				laps.push_back(std::make_tuple(st, ed));
			}
			is_run = false;
		}
		
		//��~����
		void ReStart(){
			st = std::chrono::system_clock::now();
			is_run = true;
		}

		//�g�[�^���̎��Ԃ��擾
		//template�����Ŏ��Ԃ̒P�ʂ��w��
		template<class TimeUnit = std::chrono::milliseconds>
		auto GetTotalTime() ->typename MaybeReturn<long>::type{
			return GetSplitTime<TimeUnit>(laps.size() -1);
		}

		//�w�肵����Ԃ܂ł̃g�[�^������(�X�v���b�g�^�C��)���擾
		//template�����Ŏ��Ԃ̒P�ʂ��w��
		template<class TimeUnit = std::chrono::milliseconds>
		auto GetSplitTime(uint index) ->typename MaybeReturn<long>::type{
			return index < laps.size()
				? typename MaybeReturn<long>::type(
					std::accumulate(laps.begin(), laps.begin() +index+1, 0L, [&](long sum, Lap const& v){
						return sum + std::chrono::duration_cast<TimeUnit>(std::get<1>(v) -std::get<0>(v)).count();
					})
						)
				: Nothing(-1);
		}

		//�w�肵����Ԃ̎���(���b�v�^�C��)���擾
		//template�����Ŏ��Ԃ̒P�ʂ��w��
		template<class TimeUnit = std::chrono::milliseconds>
		auto GetLapTime(uint index) ->typename MaybeReturn<long>::type{
			return index < laps.size() ? typename MaybeReturn<long>::type(std::chrono::duration_cast<TimeUnit>(std::get<1>(laps[index]) - std::get<0>(laps[index])).count()) : Nothing(-1);
		}
	};

	//�q�X�g�O����
	//template <�v�f�̌^, �x��>
	template <class T, size_t BIN_NUM>
	class Histgram{
		T const _min;
		T const _max;
		double const _delta;
		std::array<uint, BIN_NUM + 2> _count;	//[0]: x < min, [BIN_NUM-1]: max <= x
		uint _num;

	public:
		//�v�f�͈̔͂��w��
		Histgram(T min, T max) : _min(min), _max(max), _delta(((double) max - min) / BIN_NUM), _num(0){
			assert(_delta > 0);
			for (auto& ct : _count) ct = 0;
		}

		//�v�f��bin�ɐU�蕪���ăJ�E���g
		void Count(T value){
			for (uint i = 0; i < BIN_NUM + 1; ++i){
				if (value < _delta*i + _min){
					++_num;
					++_count[i];
					return;
				}
			}
			++_count[BIN_NUM + 1];
		}

		template < template < class TT, class = std::allocator<TT >> class Container>
		void Count(Container<T> const& values){
			for (auto e : values) Count(e);
		}

		//bin�O�̗v�f�����݂�����
		bool IsOverRange() const{ return _count[0] || _count[BIN_NUM + 1]; }

		//double GetAverage() const{ return std::accumulate(_count.begin(), _count.end(), 0, [](T total, T next){ return total + next; }) / static_cast<double>(_num); }

		//�p�x���擾
		std::array<uint, BIN_NUM> GetCount() const{
			std::array<uint, BIN_NUM> tmp;
			for (uint i = 0; i < BIN_NUM; ++i) tmp[i] = _count[i + 1];
			return std::move(tmp);
		}

#if SIG_ENABLE_BOOST
		//bin�Ԗ�(0 �` BIN_NUM-1)�̕p�x���擾
		//return -> tuple<�p�x, �͈͍ŏ��l(�ȏ�), �͈͍ő�l(����)>
		maybe<std::tuple<uint, int, int>> GetCount(uint bin) const{ return bin < BIN_NUM ? maybe < std::tuple < uint, int, int >>(std::make_tuple(_count[bin + 1], _delta*bin + _min, _delta*(bin + 1) + _min)) : nothing; }
#else
		std::tuple<uint, int, int> GetCount(uint bin) const{ return bin < BIN_NUM ? std::make_tuple(_count[bin + 1], _delta*bin + _min, _delta*(bin + 1) + _min) : throw std::out_of_range("Histgram::Get, bin=" + std::to_string(bin)); }
#endif

		void Print() const{
			auto IsPlus = [](double v){ return v < 0 ? false : true; };

			auto IntDigit = [](double v){ return log10(v) + 1; };

			auto Space = [](int num){
				std::string space;
				for (int i = 0; i < num; ++i) space.append(" ");
				return std::move(space);
			};

			int const rketa = IntDigit(_max);
			int const disp_precision = rketa > 2 ? 0 : 2 - rketa;
			int const keta = std::max(rketa, std::min((int) Precision(_delta), disp_precision) + 2);
			int const ctketa = log10(*std::max_element(_count.begin(), _count.end())) + 1;
			T const dbar = _num < 100 ? 1.0 : _num*0.01;

			/*
			std::string offset1, offset2;
			if (keta < 3) offset1.append(2 - keta, ' ');
			else offset2.append(keta - 3, ' ');*/

			std::cout << "\n-- Histgram --\n";
			for (int i = 0; i < BIN_NUM + 2; ++i){
				auto low = _delta*(i - 1) + _min;
				auto high = _delta*i + _min;

				if (i == 0) std::cout << std::fixed << std::setprecision(disp_precision) << "\n[-��" << Space(keta - 2) << "," << std::setw(keta + 1) << high << ")" << "�F" << std::setw(ctketa) << _count[i] << " ";
				else if (i == BIN_NUM + 1) std::cout << std::fixed << std::setprecision(disp_precision) << "\n[" << std::setw(keta + 1) << low << ",+��" << Space(keta - 2) << ")" << "�F" << std::setw(ctketa) << _count[i] << " ";
				else std::cout << std::fixed << std::setprecision(disp_precision) << "\n[" << std::setw(keta + 1) << low << "," << std::setw(keta + 1) << high << ")" << "�F" << std::setw(ctketa) << _count[i] << " ";

				for (int j = 1; dbar*j <= _count[i]; ++j) printf("|");
			}
			std::cout << std::resetiosflags(std::ios_base::floatfield) << "\n\n";
		}
	};

}

#endif