#include "file_test.h"

//SIG_ENABLE_BOOST = 1 �̍ۂɂ�boost::optional���L���ɂȂ�
//�������@�̗D�揇�ʂ� SIG_WINDOWS_ENV(windows.h�g�p) > SIG_ENABLE_BOOOST(boost::filesystem�g�p)

auto pass = sig::DirpassTailModify(L"../SigUtil/example/file_test", true);

void GetDirectoryNamesTest()
{
	std::wcout << pass << std::endl << std::endl;

	auto file_names = sig::GetFileNames(pass, false);
	auto text_file_names = sig::GetFileNames(pass, false, L".txt");
	auto old_text_file_names = sig::GetFileNames(pass, false, L".old.txt");
	auto hidden_text_file_names = sig::GetFileNames(pass, true, L".txt");

#if SIG_ENABLE_BOOST
	if(file_names){
		for (auto fn : *file_names) std::wcout << fn << std::endl;
	}
	std::cout << std::endl;
	if(text_file_names){
		for (auto fn : *text_file_names) std::wcout << fn << std::endl;
	}
	std::cout << std::endl;
	if(old_text_file_names){
		for (auto fn : *old_text_file_names) std::wcout << fn << std::endl;
	}
	std::cout << std::endl;
	if(hidden_text_file_names){
		for (auto fn : *hidden_text_file_names) std::wcout << fn << std::endl;
	}
	std::cout << std::endl;
#else
	for (auto fn : file_names) std::wcout << fn << std::endl;
	std::cout << std::endl;
	for (auto fn : text_file_names) std::wcout << fn << std::endl;
	std::cout << std::endl;
	for (auto fn : old_text_file_names) std::wcout << fn << std::endl;
	std::cout << std::endl;
	for (auto fn : hidden_text_file_names) std::wcout << fn << std::endl;
	std::cout << std::endl;
#endif

	auto folder_names = sig::GetFolderNames(pass, false);
	auto hidden_folder_names = sig::GetFolderNames(pass, true);

#if SIG_ENABLE_BOOST
	if (folder_names){
		for (auto fn : *folder_names) std::wcout << fn << std::endl;
	}
	std::cout << std::endl;
	if (hidden_folder_names){
		for (auto fn : *hidden_folder_names) std::wcout << fn << std::endl;
	}
#else
	for (auto fn : folder_names) std::wcout << fn << std::endl;
	std::cout << std::endl;
	for (auto fn : hidden_folder_names) std::wcout << fn << std::endl;
#endif
}

void FileSaveLoadTest()
{
	auto fpass1 = pass + L"test.txt";
	auto fpass2 = pass + L"test2.txt";
	auto fpass3 = pass + L"test3.txt";
	auto fpass4 = pass + L"test4.txt";
	auto fpass5 = pass + L"test5.txt";

	std::wcout << fpass1 << std::endl << std::endl;

	std::vector<std::wstring> blghost_text1{
		L"O.K.",
		L"CLOSE INITIALIZE SEQUENCE.",
		L"AND",
		L"BIGIN TO LOG ON",
		L"B.L.GHOST STRATEGY RECORDER."
	};

	std::list<std::string> blghost_text2{
		"HAIL TO YOU,MY FELLOW.",
		"I DON'T KNOW YOUR INFERNAL DAYS.",
		"HOWEVER.",
		"THERE IS NO CAUSE FOR CONCERN",
		"'CAUSE I'M ALWAYS CLOSE TO YOU.",
		"",
		"SO,YOUR PAIN IS MINE.",
		"KEEP YOUR DIGNITY."
	};
	

	//�����̓��e�̃N���A
	sig::FileClear(fpass1);
	sig::FileClear(fpass5);

	//ofstream��n���ĕۑ�
	std::wofstream ofs(fpass1, std::ios::out | std::ios::app);
	sig::SaveLine(L"test write 0", ofs);		//1�s�ۑ�
	sig::SaveLine(blghost_text1, ofs);		//�S�s�ۑ�
	ofs.close();

	//�ȉ� ���񂽂�ۑ���

	//1�s�ۑ��i�㏑���j
	sig::SaveLine(L"test write ��", fpass2);
	//1�s�ۑ��i�ǋL�j
	sig::SaveLine(L"test write ��", fpass2, sig::WriteMode::append);
	//�S�s�ۑ��i�㏑���j
	sig::SaveLine(blghost_text2, fpass3);

	//���l�f�[�^�̕ۑ��i�㏑���A1�s���ۑ��j
	sig::SaveNum(std::vector<int>{1, 2, 3, 4, 5}, fpass4);
	//���l�f�[�^�̕ۑ��i�ǋL�A�J���}�����ŕۑ��j
	sig::SaveNum(std::list<double>{1.1, 2.2, 3.3}, fpass5, sig::WriteMode::append, ",");


	//�ȉ� ���񂽂�ǂݍ��݁�

#if SIG_ENABLE_BOOST
	auto test1 = sig::ReadLine<std::string>(fpass1);
	auto test2 = sig::ReadLine<std::wstring, std::list>(fpass2);
	auto test_num1 = sig::ReadNum<int>(fpass4);
	auto test_num2 = sig::ReadNum<double, std::list>(fpass5, ",");

	if (test1){
		for (auto e : *test1) std::cout << e << std::endl;
	}
	std::cout << std::endl;
	if (test2){
		for (auto e : *test2) std::wcout << e << std::endl;
	}
	std::cout << std::endl;
	if (test_num1){
		for (auto e : *test_num1) std::wcout << e << std::endl;
	}
	std::cout << std::endl;
	if (test_num2){
		for (auto e : *test_num2) std::wcout << e << std::endl;
	}
#else
	std::vector<std::string> test1;
	std::list<std::wstring> test2;
	std::vector<int> test_num1;
	std::list<double> test_num2;

	sig::ReadLine(test1, fpass1);
	sig::ReadLine(test2, fpass2);
	sig::ReadNum<int>(test_num1, fpass4);
	sig::ReadNum<double>(test_num2, fpass5, ",");

	for (auto e : test1) std::cout << e << std::endl;
	std::cout << std::endl;
	for (auto e : test2) std::wcout << e << std::endl;
	std::cout << std::endl;
	for (auto e : test_num1) std::wcout << e << std::endl;
	std::cout << std::endl;
	for (auto e : test_num2) std::wcout << e << std::endl;
#endif
}