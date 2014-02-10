#include "string_test.h"

//SIG_ENABLE_BOOST = 1 �̍ۂɂ�boost::optional���L���ɂȂ�

using TVec = std::vector<std::string>;
using TVecw = std::vector<std::wstring>;
using TVec2 = std::vector<std::vector<std::string>>;

void RegexTest()
{
	//�G�X�P�[�v����������������擾
	auto escaped1 = sig::RegexEscaper("? or (lol) must be escaped");
	auto escaped2 = sig::RegexEscaper(L"?�Ƃ�(��)�̓G�X�P�[�v���ׂ�����");

	auto test1 = R"(\? or \(lol\) must be escaped)";	assert(escaped1 == test1);
	auto test2 = LR"(\?�Ƃ�\(��\)�̓G�X�P�[�v���ׂ�����)";	assert(escaped2 == test2);
	

	//�G�X�P�[�v���������� std::regex(or std::wregex)���擾
	auto reg = sig::RegexMaker(L"(��)");
	auto replaced = std::regex_replace(L"�Ă���(��)�ł�", reg, L"");

	assert(replaced == L"�Ă��Ƃł�");


	//���K�\���Ō���
	auto matches1 = sig::RegexSearch("test tes1a tes2b", std::regex("tes(\\d)(\\w)"));
	auto matches2 = sig::RegexSearch<std::list>("search�u? or (lol) must be escaped�v", std::regex(escaped1));
	
	TVec2 test3 = { { "tes1a", "1", "a" }, { "tes2b", "2", "b" } };
	auto test4 = "? or (lol) must be escaped";

#if SIG_ENABLE_BOOST
	for (int i=0; i < matches1->size(); ++i){
		for (int j = 0; j < (*matches1)[i].size(); ++j){
			assert((*matches1)[i][j] == test3[i][j]);
		}
	}

	for (auto m : *matches2){
		assert( m.front() == test4);
	}
#else
	for (int i=0; i < matches1.size(); ++i){
		for (int j = 0; j < matches1[i].size(); ++j){
			assert(matches1[i][j] == test3[i][j]);
		}
	}

	for (auto m : matches2){
		assert( m.front() == test4);
	}
#endif

}

void TagDealerTest()
{
	sig::TagDealer<std::string> tag_dealer("<", ">");

	auto encoded = tag_dealer.Encode("test", "TAG");
	
	assert(encoded == "<TAG>test<TAG>");					

	auto decoded = tag_dealer.Decode(encoded, "TAG");
	auto ignored = tag_dealer.Decode(encoded, "HOO");

#if SIG_ENABLE_BOOST
	if (decoded) assert(*decoded == "test");
	if (ignored) assert(false);				//ignored == nothing
#else
	assert(decoded == "test");
	assert(ignored == "");
#endif
	
	//�܂Ƃ߂ăG���R�[�h
	//encoded_vec = "<TAG1>str1<TAG1><TAG2>str2<TAG2><TAG3>str3<TAG3>"
	auto encoded_vec = tag_dealer.Encode(std::list<std::string>{"str1", "str2", "str3"}, std::list<std::string>{"TAG1", "TAG2", "TAG3"});

	TVec test{ "str1", "str3" };

#if SIG_ENABLE_BOOST
	//�܂Ƃ߂ăf�R�[�h
	auto decoded_vec = tag_dealer.Decode(encoded_vec, std::deque<std::string>{"TAG1", "TAG3"});

	if (decoded){
		for (int i=0; i< decoded_vec->size(); ++i) assert((*decoded_vec)[i] == test[i]);
	}
#endif
}

void SplitTest()
{
	std::string str{ "one, 2, �Q "};
	//,���f���~�^�Ƃ��ĕ���
	auto split1 = sig::Split(str, ",");

	TVec test1{ "one", " 2", " �Q " };
	for (int i = 0; i<split1.size(); ++i) assert(split1[i] == test1[i]);

	//�R���e�i(�V�[�P���X����)�̖����I�w�����
	auto split2 = sig::Split<std::list>(L"https://github.com/regenschauer490/Utility", L"/");

	TVecw test2{ L"https:", L"github.com", L"regenschauer490", L"Utility" };
	auto sp2it = split2.begin();
	for (int i = 0; i<split2.size(); ++i, ++sp2it) assert(*sp2it == test2[i]);

	//�f���~�^�Ԃɉ����Ȃ���Ζ���
	auto split3 = sig::Split("10 100  1000", " ");	
	auto split4 = sig::Split(" ,, ,  ,", ",");

	TVec test3{ "10", "100", "1000"};
	for (int i = 0; i<split3.size(); ++i) assert(split3[i] == test3[i]);
	TVec test4{ " ", " ", "  " };
	for (int i = 0; i<split4.size(); ++i) assert(split4[i] == test4[i]);

	auto sentence = R"(1
2

4
)";
	//�s���ɕ����i��s�����j
	auto split5 = sig::Split(sentence, "\n");
	TVec test5 = { "1", "2", "4" };
	for (int i = 0; i<split5.size(); ++i) assert(split5[i] == test5[i]);
}

void CatStrTest()
{
	//������̌���
	auto cat1 = sig::CatStr(std::vector<std::string>{"eins", "zwei", "drei"}, "");
	auto cat2 = sig::CatStr(std::list<std::wstring>{L"eins", L"zwei", L"drei"}, L",");
	auto cat3 = sig::CatStr(std::set<int>{1, 2, 3}, "\n");

	assert(cat1 == "einszweidrei");
	assert(cat2 == L"eins,zwei,drei");
	assert(cat3 == "1\n2\n3");
}

void StrConvertTest()
{
	//�}���`���� <-> ���C�h���� �ϊ�
	auto wstr = sig::STRtoWSTR("test�Ă���");
	auto str = sig::WSTRtoSTR(L"test�Ă���");

	assert(wstr == L"test�Ă���");
	assert(str == "test�Ă���");

	//�܂Ƃ߂ĕϊ�
	auto wstr_vec = sig::STRtoWSTR(std::vector<std::string>{"��", "��", "�Q"});
	TVecw test{ L"��", L"��", L"�Q" };
	for (int i = 0; i<wstr_vec.size(); ++i) assert(wstr_vec[i] == test[i]);
}

void ZenHanTest()
{
	//�S�p <-> ���p �ϊ��N���X
	auto& replacer = sig::ZenHanReplace::GetInstance();

	std::wstring sentence1 = L"�`B�b�A��E�P2�R�M���߃|";

	replacer.Alphabet_Zen2Han(sentence1);
	assert(sentence1 == L"ABC�A��E�P2�R�M���߃|");

	replacer.Katakana_Zen2Han(sentence1);
	assert(sentence1 == L"ABC����P2�R�޶�����");

	replacer.Number_Zen2Han(sentence1);
	assert(sentence1 == L"ABC���123�޶�����");


	std::wstring sentence2 = L"�`B�b�A��E�P2�R�M���߃|";

	replacer.Alphabet_Han2Zen(sentence2);
	assert(sentence2 == L"�`�a�b�A��E�P2�R�M���߃|");

	replacer.Katakana_Han2Zen(sentence2);
	assert(sentence2 == L"�`�a�b�A�C�E�P2�R�M�K�y�|");

	replacer.Number_Han2Zen(sentence2);
	assert(sentence2 == L"�`�a�b�A�C�E�P�Q�R�M�K�y�|");
}
