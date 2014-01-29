#include "string_test.hpp"

//SIG_ENABLE_BOOST = 1 �̍ۂɂ�boost::optional���L���ɂȂ�

void RegexTest()
{
	auto escaped1 = sig::RegexEscaper("? or (lol) must be escaped");
	auto escaped2 = sig::RegexEscaper(L"?�Ƃ�(��)�̓G�X�P�[�v���ׂ�����");

	std::cout << escaped1 << std::endl;			//\? or \(lol\) must be escaped
	std::wcout << escaped2 << std::endl;		//\?�Ƃ�\(��\)�̓G�X�P�[�v���ׂ�����
	
	auto reg = sig::RegexMaker(L"(��)");
	auto replaced = std::regex_replace(L"�Ă���(��)�ł�", reg, L"");

	std::wcout << replaced << std::endl;		//�Ă��Ƃł�

	auto matches1 = sig::RegexSearch("test tes1a tes2b", std::regex("tes(\\d)(\\w)"));
	auto matches2 = sig::RegexSearch("search�u? or (lol) must be escaped�v", std::regex(escaped1));
	
#if SIG_ENABLE_BOOST
	for (auto m : *matches1) std::cout << m[0] << "," << m[1] << "," << m[2] << std::endl;
	//tes1,1,a
	//tes2,2,b

	for (auto m : *matches2) std::cout << m[0] << std::endl;
	//? or (lol) must be escaped
#else
	for (auto m : matches1) std::cout << m[0] << "," << m[1] << "," << m[2] << std::endl;
	//tes1,1,a
	//tes2,2,b

	for (auto m : matches2) std::cout << m[0] << std::endl;
	//? or (lol) must be escaped
#endif

}

void TagDealerTest()
{
	sig::TagDealer<std::string> tag_dealer("<", ">");

	auto encoded = tag_dealer.Encode("test", "TAG");
	std::cout << encoded << std::endl;					//<TAG>test<TAG>

	auto decoded = tag_dealer.Decode(encoded, "TAG");
	auto ignored = tag_dealer.Decode(encoded, "HOO");

#if SIG_ENABLE_BOOST
	if (decoded) std::cout << *decoded << std::endl;	//test
	if (ignored) std::cout << *ignored << std::endl;	//(�o�͂Ȃ�)
#else
	std::cout << decoded << std::endl;		//test
	std::cout << ignored << std::endl;		//(�o�͂Ȃ�)
#endif
	
	//�܂Ƃ߂ăG���R�[�h -> <TAG1>str1<TAG1><TAG2>str2<TAG2><TAG3>str3<TAG3>
	auto encoded_vec = tag_dealer.Encode(std::vector<std::string>{"str1", "str2", "str3"}, std::vector<std::string>{"TAG1", "TAG2", "TAG3"});

#if SIG_ENABLE_BOOST
	//�܂Ƃ߂ăf�R�[�h
	auto decoded_vec = tag_dealer.Decode(encoded_vec, std::vector<std::string>{"TAG1", "TAG3"});

	if (decoded){
		for (auto s : *decoded_vec) std::cout << s << std::endl;
		//str1
		//str3
	}
#endif
}

void SplitTest()
{
	std::string str{ "one, 2, �Q "};

	auto split1 = sig::Split(str, ",");
	for (auto& s : split1) std::cout << "[" << s << "]" << std::endl;
	//[one]
	//[ 2]
	//[ �Q ]
	

	auto split2 = sig::Split<std::list>(L"https://github.com/regenschauer490/Utility", L"/");
	for (auto& s : split2) std::wcout << L"[" << s << L"]" << std::endl;
	//[https::]
	//[github.com]
	//[regenschauer490]
	//[Utility]
}

void CatStrTest()
{
	auto cat1 = sig::CatStr(std::vector<std::string>{"eins", "zwei", "drei"});
	auto cat2 = sig::CatWStr(std::vector<std::wstring>{L"eins", L"zwei", L"drei"}, L",");

	std::cout << cat1 << std::endl;			//einszweidrei
	std::wcout << cat2 << std::endl;		//eins,zwei,drei,
}

void StrConvertTest()
{
	auto wstr = sig::STRtoWSTR("test�Ă���");
	auto str = sig::WSTRtoSTR(L"test�Ă���");

	auto wstr_vec = sig::STRtoWSTR(std::vector<std::string>{"��", "��", "�Q"});
}
