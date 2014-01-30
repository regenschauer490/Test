#include "file_test.hpp"

//SIG_ENABLE_BOOST = 1 �̍ۂɂ�boost::optional���L���ɂȂ�
//�������@�̗D�揇�ʂ� SIG_WINDOWS_ENV(windows.h�g�p) > SIG_ENABLE_BOOOST(boost::filesystem�g�p)

auto pass = sig::DirpassTailModify(L"../SigUtil/test/file_test", true);

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
	auto fpass3 = pass + L"blghost.txt";

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

	//1�s�ۑ��i�㏑���j
	sig::SaveLine("test write 1", fpass2, sig::WriteMode::overwrite);
	//1�s�ۑ��i�ǋL�j
	sig::SaveLine("test write 2", fpass2, sig::WriteMode::append);
	//�S�s�ۑ��i�㏑���j
	sig::SaveLine(blghost_text, fpass3, sig::WriteMode::overwrite);
}