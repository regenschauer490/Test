#include "file_test.hpp"

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

	std::wcout << fpass1 << std::endl << std::endl;

	std::vector<std::string> blghost_text{
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

	//ofstream��n���ĕۑ�
	std::ofstream ofs(fpass1, std::ios::out | std::ios::app);
	sig::SaveLine("test write 0", ofs);		//1�s�ۑ�
	sig::SaveLine(blghost_text, ofs);		//�S�s�ۑ�
	ofs.close();

	//�ȉ� ���񂽂�ۑ���

	//1�s�ۑ��i�㏑���j
	sig::SaveLine(L"test write ��", fpass2);
	//1�s�ۑ��i�ǋL�j
	sig::SaveLine(L"test write ��", fpass2, sig::WriteMode::append);
	//�S�s�ۑ��i�㏑���j
	sig::SaveLine(blghost_text, fpass3);

	//���l�f�[�^�̕ۑ��i�㏑���A1�s���ۑ��j
	sig::SaveLineNum(std::vector<int>{1, 2, 3, 4, 5}, fpass4);
	//���l�f�[�^�̕ۑ��i�ǋL�A�J���}�����ŕۑ��j
	sig::SaveLineNum(std::list<double>{1.1, 2.2, 3.3}, fpass4, sig::WriteMode::append, ",");


	//�ȉ� ���񂽂�ǂݍ��݁�

#if SIG_ENABLE_BOOST
	auto test1 = sig::ReadLine<std::string>(fpass1);
	auto test2 = sig::ReadLine<std::wstring>(fpass2);

	if (test1){
		for (auto e : *test1) std::cout << e << std::endl;
	}
	std::cout << std::endl;
	if (test2){
		for (auto e : *test2) std::wcout << e << std::endl;
	}
#else
	std::vector<std::string> test1;
	std::vector<std::wstring> test2;

	sig::ReadLine(test1, fpass1);
	sig::ReadLine(test2, fpass2);

	for (auto e : test1) std::cout << e << std::endl;
	std::cout << std::endl;
	for (auto e : test2) std::wcout << e << std::endl;
#endif
}