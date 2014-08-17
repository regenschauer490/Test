﻿/*
Copyright(c) 2014 Akihiro Nishimura

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/

#ifndef SIG_UTIL_TOOL_HPP
#define SIG_UTIL_TOOL_HPP

#include "sigutil.hpp"
#include "helper.hpp"

#include <chrono>
#include <fstream>

#if SIG_ENABLE_BOOST
#include <boost/serialization/serialization.hpp>
#endif


/* 便利ツール */

namespace sig
{
//初期化時に指定した範囲の一様分布乱数を発生させるクラス
//デフォルト: 乱数生成器 -> メルセンヌツイスター
template <class NumType, class Engine = std::mt19937>
class SimpleRandom {
	Engine engine_;		//乱数生成アルゴリズム 
	typename std::conditional <
		std::is_integral<NumType>::value,
		std::uniform_int_distribution<int>,
		std::uniform_real_distribution<double>
	> ::type dist_;		//確率分布

public:
	// min:	乱数最小値
	// max:	乱数最大値
	// debug:	乱数のシードを固定するか
	SimpleRandom(NumType min, NumType max, bool debug) : engine_(
		[debug](){
		std::random_device rnd;
		std::vector<unsigned long> v(10);
		if (debug) std::fill(v.begin(), v.end(), 0);
		else std::generate(v.begin(), v.end(), std::ref(rnd));
		std::seed_seq seq(v.begin(), v.end());

		return Engine{seq};
	}()
		),
		dist_(min, max){}

	NumType operator()(){
		return dist_(engine_);
	}
};


// 重複の無い一様分布の整数乱数を生成
// n:	発生させる値の個数
// min:	最小値
// max:	最大値
// debug:	乱数のシードを固定するか
template < template < class T, class = std::allocator<T>> class Container = std::vector >
Container<int> random_unique_numbers(uint n, int min, int max, bool debug) {
	std::unordered_set<int> match;
	Container<int> result;
	SimpleRandom<int> Rand(0, max - min, debug);

	int r;
	for (uint i = 0; i < n; ++i){
		do{
			r = min + Rand();
		} while (match.find(r) != match.end());

		match.insert(r);
		result.push_back(r);
	}

	return result;
}

//タイムウォッチ
class TimeWatch{
	typedef std::chrono::system_clock::time_point Time;
	typedef decltype(std::declval<Time>() - std::declval<Time>()) Duration;

	Time st;
	std::vector<Duration> laps;
	std::vector<Duration> cache;
	bool is_run;

private:
	void init(){
		st = std::chrono::system_clock::now();
		laps.clear();
		cache.clear();
	}

	Duration d_accumulate(std::vector<Duration> const& ds, uint end) const{
		return std::accumulate(ds.begin(), ds.begin() + end, Duration(), [&](Duration const& sum, Duration const& v){
			return sum + v;
		});
	}

public:
	TimeWatch(){
		init();
		is_run = true;
	}

	//初期化して停止
	void reset(){
		init();
		is_run = false;
	}

	//停止
	void stop(){
		if (is_run)	cache.push_back(std::chrono::system_clock::now() - st);
		is_run = false;
	}
		
	//停止解除
	void restart(){
		st = std::chrono::system_clock::now();
		is_run = true;
	}

	void save(){
		if (is_run){
			auto now = std::chrono::system_clock::now();
			cache.push_back(now - st);
			st = std::move(now);
		}

		laps.push_back(d_accumulate(cache, cache.size()));
		cache.clear();
	}

	//トータルの時間を取得
	//template引数で時間の単位を指定
	template<class TimeUnit = std::chrono::milliseconds>
	long get_total_time(){
		return std::chrono::duration_cast<TimeUnit>(d_accumulate(laps, laps.size())).count();
	}

	//指定した区間までのトータル時間(スプリットタイム)を取得
	//template引数で時間の単位を指定
	template<class TimeUnit = std::chrono::milliseconds>
	auto get_split_time(uint index) ->Just<long>{
		return index < laps.size()
			? Just<long>(std::chrono::duration_cast<TimeUnit>(d_accumulate(laps, index+1)).count())
			: Nothing(-1);
	}

	//指定した区間の時間(ラップタイム)を取得
	//template引数で時間の単位を指定
	template<class TimeUnit = std::chrono::milliseconds>
	auto get_lap_time(uint index) ->Just<long>{
		return index < laps.size() ? Just<long>(std::chrono::duration_cast<TimeUnit>(laps[index]).count()) : Nothing(-1);
	}
};

//ヒストグラム
//template <要素の型, 度数>
template <class T, size_t BIN_NUM>
class Histgram{
	T const _min;
	T const _max;
	double const _delta;
	std::array<uint, BIN_NUM + 2> _count;	//[0]: x < min, [BIN_NUM-1]: max <= x
	uint _num;

private:
	void print_base_(std::ostream& ofs) const{
		//auto IsPlus = [](double v){ return v < 0 ? false : true; };

		auto IntDigit = [](double v){ return log10(v) + 1; };

		auto FirstZero = [](double v){
			uint keta = 0;
			if (equal(v, 0)) return keta;
			while (static_cast<int>(v * std::pow(10, keta)) == 0) ++keta;
			return keta;
		};

		auto Space = [](int num){
			std::string space;
			for (int i = 0; i < num; ++i) space.append(" ");
			return space;
		};

		auto Precision = [](double v)
		{
			uint keta = 0;
			double dv = v - int(v);

			while (true){
				if (equal(dv, 0) || keta >= 15) break;
				v *= 10;
				dv = v - int(v);
				++keta;
			}

			return keta;
		};

		int const rketa = IntDigit(_max);
		int const disp_precision = typename std::conditional<std::is_integral<T>::value, std::true_type, std::false_type>::type{}
			? 0
			: IntDigit(_delta) > 1
				? 0
				: FirstZero(_delta) +1;
		int const keta = std::max(rketa, std::min((int) Precision(_delta), disp_precision) + 2);
		int const ctketa = log10(*std::max_element(_count.begin(), _count.end())) + 1;
		T const dbar = _num < 100 ? 1.0 : _num*0.01;

		/*
		std::string offset1, offset2;
		if (keta < 3) offset1.append(2 - keta, ' ');
		else offset2.append(keta - 3, ' ');*/

		ofs << "\n-- Histgram --\n";
		for (int i = 0; i < BIN_NUM + 2; ++i){
			auto low = _delta*(i - 1) + _min;
			auto high = _delta*i + _min;

			if (i == 0) ofs << std::fixed << std::setprecision(disp_precision) << "\n[-∞" << Space(keta - 2) << "," << std::setw(keta + 1) << high << ")" << "：" << std::setw(ctketa) << _count[i] << " ";
			else if (i == BIN_NUM + 1) ofs << std::fixed << std::setprecision(disp_precision) << "\n[" << std::setw(keta + 1) << low << ",+∞" << Space(keta - 2) << ")" << "：" << std::setw(ctketa) << _count[i] << " ";
			else ofs << std::fixed << std::setprecision(disp_precision) << "\n[" << std::setw(keta + 1) << low << "," << std::setw(keta + 1) << high << ")" << "：" << std::setw(ctketa) << _count[i] << " ";

			for (uint j = 1; dbar*j <= _count[i]; ++j) ofs << "|";
		}
		ofs << std::resetiosflags(std::ios_base::floatfield) << "\n\n";
	}

public:
	//要素の範囲を指定
	Histgram(T min, T max) : _min(min), _max(max), _delta(((double) max - min) / BIN_NUM), _num(0){
		assert(_delta > 0);
		for (auto& ct : _count) ct = 0;
	}

	//要素をbinに振り分けてカウント
	void count(T value){
		for (uint i = 0; i < BIN_NUM + 1; ++i){
			if (value < _delta*i + _min){
				++_num;
				++_count[i];
				return;
			}
		}
		++_count[BIN_NUM + 1];
	}

	template <class Container>
	void count(Container const& values){
		for (auto const& e : values) count(e);
	}

	//bin外の要素が存在したか
	bool is_over_range() const{ return _count[0] || _count[BIN_NUM + 1]; }

	//double GetAverage() const{ return std::accumulate(_count.begin(), _count.end(), 0, [](T total, T next){ return total + next; }) / static_cast<double>(_num); }

	//頻度を取得
	auto get_count() const -> std::array<uint, BIN_NUM>{
		std::array<uint, BIN_NUM> tmp;
		for (uint i = 0; i < BIN_NUM; ++i) tmp[i] = _count[i + 1];
		return tmp;
	}

	//bin番目(0 ～ BIN_NUM-1)の頻度を取得
	//return -> tuple<頻度, 範囲最小値(以上), 範囲最大値(未満)>
	auto get_count(uint bin) const ->Just<std::tuple<uint, int, int>>{
		return bin < BIN_NUM
			? Just<std::tuple<uint, int, int>>(std::make_tuple(_count[bin + 1], _delta*bin + _min, _delta*(bin + 1) + _min))
			: Nothing(std::make_tuple(0u, 0, 0));
	}
		
	void print() const{ print_base_(std::cout); }

	//ファイルへ出力
	void print(FilepassString const& file_pass) const{
		std::ofstream ofs(file_pass);
		print_base_(ofs);
	}
};

//パーセント型
class Percent
{
	int percent_;

public:
	explicit Percent(int percent) : percent_(percent){}

	int get_percent() const{ return percent_; }
	double get_double() const{ return percent_ * 0.01; }

	static Percent const& unit(){ static const Percent unit(100); return unit; }

	Percent operator=(Percent src){ percent_ = src.percent_; return *this; }
	Percent operator=(int src){ percent_ = src; return *this; }

	bool operator==(Percent obj) const{ return percent_ == obj.percent_; }

	bool operator!=(Percent obj) const{ return percent_ != obj.percent_; }

#if SIG_ENABLE_BOOST
private:
	friend class boost::serialization::access;

	template <class Archive>
	void serialize(Archive& ar, unsigned int version)
	{
		ar & percent_;
	}

	template <class Archive>
	friend void save_construct_data(Archive & ar, Percent const* p, unsigned int version){};

	template <class Archive>
	friend void load_construct_data(Archive & ar, Percent* p, unsigned int version){
		::new(p) Percent(0);
	}
#endif
};

}
#endif
