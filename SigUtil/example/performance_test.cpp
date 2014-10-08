#include "performance_test.h"
#include "../lib/tools.hpp"
#include "../lib/file.hpp"
#include "../lib/string.hpp"
#include "../lib/distance/cosine_similarity.hpp"

// split�֐��̏������Ԃ̊m�F
void SplitPerformanceTest()
{
	auto long_text = sig::fromJust(sig::read_line<std::string>(SIG_TO_FPSTR("../SigUtil/example/test_file/long_text.txt")));

	sig::TimeWatch tw;
	auto split = sig::split(long_text[0], ",");
	tw.save();
	std::cout << "split time(ms): " << tw.get_total_time() << std::endl;
}

// optional�̗L���ɂ��A�������Ԃ̈Ⴂ�̊m�F
void OptionalPerformanceTest()
{
	const int ITERATION = 10;

	/*
	// �����R�[�h�ϊ��^�X�N
	const int N = 1000;
	const auto str = "test string";

	sig::TimeWatch tw1;
	for (int loop = 0; loop<ITERATION; ++loop){
		sig::array<std::wstring, N> ar;

		for (int i = 0; i<N; ++i){
			ar.push_back(sig::str_to_wstr(str));
		}

		tw1.save();
	}

	std::cout << "str_to_wstr time: " << tw1.get_total_time<std::chrono::milliseconds>() << std::endl;
	*/
	/*
	<VC++120>
	[debug]
	raw		952, 977, 916 ms
	maybe	970, 940,  ms

	[release]
	raw		813, 827, 828 ms
	maybe	845, 870, 811 ms
	*/


	// �����v�Z�^�X�N
	const int M = 10000;
	const int DIM = 10;
	std::vector<double> dot1, dot2;
	sig::SimpleRandom<double> drand(-1.0, 1.0, false);

	for (int i=0; i<DIM; ++i){
		dot1.push_back(drand());
		dot2.push_back(drand());
	}

	sig::TimeWatch tw2;
	for (int loop = 0; loop < ITERATION; ++loop){
		sig::array<double, M> ar;

		for (int i = 0; i < M; ++i){
			ar[i] = sig::cosine_similarity(dot1, dot2);
		}
		tw2.save();
	}

	std::cout << "cosine_simirality time: " << tw2.get_total_time<std::chrono::milliseconds>() << std::endl;

	/*
	<VC++120>
	[debug]
	raw		2081, 1976, 1996 ms
	maybe	2077, 2110, 2071 ms

	[release]
	raw		95, 102, 96 ms
	maybe	97, 107, 98 ms
	*/
}
