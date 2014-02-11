#include "tool_test.h"

#if SIG_WINDOWS_ENV
#include <windows.h>
#endif

//SIG_ENABLE_BOOST = 1 �̍ۂɂ�boost::optional���L���ɂȂ�

void RandomTest()
{
	sig::SimpleRandom<int> rand_maker(-10, 10, true);	//[-10, 10]�̈�l���z�����A�f�o�b�O���[�h(�V�[�h�Œ�)
	
	std::cout << rand_maker() << std::endl;

	auto rints = sig::RandomUniqueNumbers(10, 0, 30, true);	//[0, 30]�̈�l���z����10�d�������T���v��

	for (auto v : rints) std::cout << v << std::endl;
}

void TimeWatchTest()
{
	sig::TimeWatch tw;		//�v���J�n

#if SIG_WINDOWS_ENV
	Sleep(100);

	tw.Save();		//�����܂ł̃^�C�����L�^

	Sleep(200);

	tw.Save();		//�����܂ł̃^�C�����L�^

	Sleep(300);

	tw.Stop();		//�ꎞ��~

	Sleep(10);

	tw.Save();		//�����܂ł̃^�C�����L�^

	Sleep(20);

	tw.ReStart();	//�v���ĊJ

	Sleep(400);

	tw.Save();		//�����܂ł̃^�C�����L�^

#if SIG_ENABLE_BOOST
	assert(sig::TolerantEqual(*tw.GetLapTime(0), 100, 1));		//100 �} 1 (ms)
	assert(sig::TolerantEqual(*tw.GetLapTime(1), 200, 1));		//200 �} 1 (ms)
	assert(sig::TolerantEqual(*tw.GetLapTime(2), 300, 1));		//300 �} 1 (ms)
	assert(sig::TolerantEqual(*tw.GetLapTime(3), 400, 1));		//400 �} 1 (ms)
	assert(! tw.GetLapTime(4));

	assert(sig::TolerantEqual(*tw.GetSplitTime(0), 100, 1));	//100 �} 1 (ms)
	assert(sig::TolerantEqual(*tw.GetSplitTime(1), 300, 1));	//300 �} 1 (ms)
	assert(sig::TolerantEqual(*tw.GetSplitTime(2), 600, 1));	//600 �} 1 (ms)
	assert(sig::TolerantEqual(*tw.GetSplitTime(3), 1000, 1));	//1000 �} 1 (ms)
#else
	assert(sig::TolerantEqual(tw.GetLapTime(0), 100, 1));		//100 �} 1 (ms)
	assert(sig::TolerantEqual(tw.GetLapTime(1), 200, 1));		//200 �} 1 (ms)
	assert(sig::TolerantEqual(tw.GetLapTime(2), 300, 1));		//300 �} 1 (ms)
	assert(sig::TolerantEqual(tw.GetLapTime(3), 400, 1));		//400 �} 1 (ms)
	assert(! tw.GetLapTime(4));

	assert(sig::TolerantEqual(tw.GetSplitTime(0), 100, 1));		//100 �} 1 (ms)
	assert(sig::TolerantEqual(tw.GetSplitTime(1), 300, 1));		//300 �} 1 (ms)
	assert(sig::TolerantEqual(tw.GetSplitTime(2), 600, 1));		//600 �} 1 (ms)
	assert(sig::TolerantEqual(tw.GetSplitTime(3), 1000, 1));	//1000 �} 1 (ms)
#endif
	assert(sig::TolerantEqual(tw.GetTotalTime(), 1000, 1));				//1000 �} 1 (ms)
	assert(sig::Equal(tw.GetTotalTime<std::chrono::seconds>(), 1));		//1 (s)
	assert(sig::TolerantEqual(tw.GetTotalTime<std::chrono::microseconds>(), 1000000, 100));	//1000000  �} 100 (��s)
#else

#endif
}

void HistgramTest()
{
	std::vector<int> data1{-100, -10, -6, -5, -1, 0, 3, 5, 5, 6, 6, 6, 7, 9, 10};

	sig::Histgram<int, 10> hist(-10, 10);	//int�^�A�r����10�A[-10�`10�j�͈̔͂̐��l���W�v

	hist.Count(data1);

	hist.Print();					//�R���\�[���֏o��
	hist.Print(L"./example/test_file/histgram1.txt");	//�t�@�C���֏o��
	/*
	-- Histgram --

	[-��,-10)�F0
	[-10, -8)�F1 |
	[ -8, -6)�F0
	[ -6, -4)�F2 ||
	[ -4, -2)�F0
	[ -2,  0)�F1 |
	[  0,  2)�F1 |
	[  2,  4)�F1 |
	[  4,  6)�F2 ||
	[  6,  8)�F4 ||||
	[  8, 10)�F1 |
	[ 10,+��)�F1 |
	*/

	auto count = hist.GetCount();		//0 �` BIN_NUM-1 �̕p�x���擾
	assert(count[2], 2);				//[ -6, -4)�̌�

	auto c2 = hist.GetCount(2);
	auto c100 = hist.GetCount(100);

#if SIG_ENABLE_BOOST
	if(c2){
		assert(std::get<1>(*c2) == -6);
		assert(std::get<2>(*c2) == -4);
		assert(std::get<0>(*c2) == 2);
	}
	assert(!c100);
#else
	assert(std::get<1>(c2) == -6);
	assert(std::get<2>(c2) == -4);
	assert(std::get<0>(c2) == -2);

	assert(std::get<1>(c100) == 0);
	assert(std::get<2>(c100) == 0);
	assert(std::get<0>(c100) == 0);
#endif

	bool over = hist.IsOverRange();	//�����ݒ�͈̔͊O�̒l�����݂�����
	assert(over, 1);				//�͈͊O�̒l�̌��F1
	

	sig::Histgram<double, 15> hist2(0, 1);						//double�^�A�r����15�A[0�`1�j�͈̔͂̐��l���W�v
	auto rand_maker = sig::SimpleRandom<double>(0, 1.5, true);	//[0, 1.5]�̈�l���z�����A�f�o�b�O���[�h(�V�[�h�Œ�)
	
	for (int i = 0; i<100; ++i) hist2.Count(rand_maker());

	hist2.Print();
	hist2.Print(L"./example/test_file/histgram2.txt");
	/*
	-- Histgram --

	[-��   , 0.000)�F 0
	[ 0.000, 0.067)�F 6 ||||||
	[ 0.067, 0.133)�F 6 ||||||
	[ 0.133, 0.200)�F 5 |||||
	[ 0.200, 0.267)�F 6 ||||||
	[ 0.267, 0.333)�F 4 ||||
	[ 0.333, 0.400)�F 5 |||||
	[ 0.400, 0.467)�F 2 ||
	[ 0.467, 0.533)�F11 |||||||||||
	[ 0.533, 0.600)�F 2 ||
	[ 0.600, 0.667)�F 3 |||
	[ 0.667, 0.733)�F 5 |||||
	[ 0.733, 0.800)�F 3 |||
	[ 0.800, 0.867)�F 4 ||||
	[ 0.867, 0.933)�F 7 |||||||
	[ 0.933, 1.000)�F 1 |
	[ 1.000,+��   )�F30 ||||||||||||||||||||||||||||||
	*/
}

void PercentTest()
{
	sig::Percent pc1(100);
	sig::Percent pc2(50);
	sig::Percent pc3(0);
	sig::Percent pc4(0);

	pc3 = 1;					//copy assignment. pc4 == 1%
	pc4 = sig::Percent::Unit();	//copy assignment. pc3 == 100%

	assert(pc1.GetPercent() == 100 && pc1.GetDouble() == 1);	//100% = 1
	assert(pc2.GetPercent() == 50 && pc2.GetDouble() == 0.5);	//50% = 0.5
	assert(pc3.GetPercent() == 1 && pc3.GetDouble() == 0.01);	//1% = 0.01
	assert(pc1 == pc4);
}