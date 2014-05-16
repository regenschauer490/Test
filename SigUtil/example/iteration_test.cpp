#include "iteration_test.h"
#include "debug.hpp"

const std::array<int, 4> data0{ { 1, 2, 3, 4 } };
const std::vector<int> data1{1, 2, 3};
const std::list<int> data2{ 1, 1, 1 };
const std::multiset<double, std::greater<double>> data3{ 1.1, 2.2, 3.3 };
const std::unordered_multiset<char> data4{ 'a', 'b', 'c', 'd' };
const sig::array<std::string, 3> data5{ "A", "B", "C" };

void ForeachTest()
{
	auto data1_t = data1;
	auto data2_t = data2;
	auto data5_t = data5;

	// �v�f���ɁAdata1_t��data0�̒l��������
	sig::for_each([](int& a, int b){ a += b; }, data2_t, data0);

	sig::zipWith(
		sig::DebugEqual(),
		data2_t,
		sig::zipWith(std::plus<int>(), data2, data0)
	);

	// ���[�v�ϐ����p�ӂ��ꂽ�� (��2�����Ń��[�v�ϐ��̏����l���w��)
	// ���F���[�v�ϐ��̒l �� �C���f�b�N�X
	sig::for_each([](int i, int a, int& b){ b += (a + i); }, 1, data0, data1_t);

	auto data1_tt = data1;
	for (unsigned i = 0; i<data1.size(); ++i){
		data1_tt[i] += (data0[i] + i+1);
	}
	sig::zipWith(
		sig::DebugEqual(),
		data1_t,
		data1_tt
	);

	// ��L2�֐��͉ϒ�
	sig::for_each([](int i, int a, int b, double c, char d, std::string& e)
	{
		e.append(std::to_string(i) + d + std::to_string(a + b) + d + std::to_string(c)); 
	}
	, -1, data1, data2, data3, data4, data5_t);
}

void CompoundAssignmentTest()
{
	auto data1_t = data1;
	auto data2_t = data2;

	// �������(�R���e�i�̊e�v�f�ɑ΂��ĉ��炩�̉��Z���s���A���Z��̒l��������)
	sig::compound_assignment([](int& v1, int v2){ v1 += v2; }, data1_t, 1);
	sig::zipWith(
		sig::DebugEqual(),
		data1_t,
		sig::zipWith(std::plus<int>(), data1, sig::replicate(data1.size(), 1))
		);

	sig::compound_assignment([](int& v1, int v2){ v1 -= v2; }, data2_t, data0);
	sig::zipWith(
		sig::DebugEqual(),
		data2_t,
		sig::zipWith(std::minus<int>(), data2, data0)
		);

	std::vector<std::string> str_set{ "ein", "zwei", "drei" };
	sig::compound_assignment([](std::string& str, std::string add){ str += add; }, str_set, std::string("-hander"));

}
