/*
The MIT License(MIT)

Copyright(c) 2014 Akihiro Nishimura

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files(the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __SIG_UTIL_FILE__
#define __SIG_UTIL_FILE__

#include "string.hpp"
#include "sigutil.hpp"

#include <fstream>
#include <codecvt>
#include <locale>

#if SIG_WINDOWS_ENV
#include <windows.h>
#endif

#if SIG_ENABLE_BOOST
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
#endif

/* ���o�͊֘A */

namespace sig{

	//�f�B���N�g���E�t�@�C���p�X�̖�����'/'or'\'�����邩�`�F�b�N���A�t���邩�O�����ǂ����w��
	inline std::wstring DirpassTailModify(FileString const& directory_pass, bool const has_slash)
	{
		if (directory_pass.empty()) return directory_pass;

		auto tail = directory_pass.back();

		if (has_slash){
			//�t����ꍇ
			if (tail == '/' || tail == '\\') return directory_pass;
			else return (directory_pass + L"/");
		}
		else{
			if (tail != '/' && tail != '\\') return directory_pass;
			else{
				auto tmp = directory_pass;
				tmp.pop_back();
				return tmp;
			}
		}
	};


	//�w��f�B���N�g���ɂ���t�@�C�������擾
	//directry_pass�F���ׂ����f�B���N�g���̃p�X
	//hidden_file�Ftrue->�B���t�@�C���̂�, false->��B���t�@�C���̂� (Windows, Linux���̂�)
	//extension�F�g���q�w��(�I�v�V����)
	//�ǂݍ��ݎ��s: return -> nothing or empty-vector
	inline auto GetFileNames(FileString const& directory_pass, bool hidden_file, std::wstring extension = L"") ->Just<std::vector<std::wstring>>::type
	{
		typedef std::vector<std::wstring> ResultType;
		ResultType result;

#if SIG_WINDOWS_ENV
		WIN32_FIND_DATA fd;
		auto query = extension.empty() ? L"?*" : L"*" + extension;
		auto pass = DirpassTailModify(directory_pass, true) + query;
		auto hFind = FindFirstFile(pass.c_str(), &fd);

		if (hFind == INVALID_HANDLE_VALUE){
			return Nothing(std::move(result));
		}
		else{
			do{
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && BoolConsistency(hidden_file, (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)))
				{
					result.push_back(std::wstring(fd.cFileName));
				}
			} while (FindNextFile(hFind, &fd));

			FindClose(hFind);
			return Just<ResultType>::type(std::move(result));
		}
#elif SIG_ENABLE_BOOST
		auto IsHidden = [](fs::path const& p){
			auto name = p.filename();
			if (name.c_str()[0] == '.' && name != ".." &&	name != ".") return true;
			else return false;
		};

		fs::directory_iterator end;
		for (fs::directory_iterator it(directory_pass); it != end; ++it)
		{
			if (!fs::is_directory(*it) && BoolConsistency(hidden_file, IsHidden(*it))){
				auto leaf = sig::Split(it->path().wstring(), L"\\").back();
				if (extension.empty()) result.push_back(leaf);
				else{
					auto query = L".*(" + RegexEscaper(extension) + L")";
					auto ext = RegexSearch(leaf, std::wregex(query));
					if (ext && (*ext)[0][1] == extension) result.push_back(leaf);
				}
			}
		}
		return Just<ResultType>::type(std::move(result));
#else
		static_asseet(false, "this OS is not support. please include boost if any.");
#endif
	}


	//�w��f�B���N�g���ɂ���t�H���_�����擾
	//directry_pass�F���ׂ����f�B���N�g���̃p�X
	//hidden_file�Ftrue->�B���t�@�C���̂�, false->��B���t�@�C���̂� (Windows, Linux���̂�)
	//�ǂݍ��ݎ��s: return -> nothing or empty-vector
	inline auto GetFolderNames(FileString const& directory_pass, bool hidden_file) ->Just<std::vector<std::wstring>>::type
	{
		typedef std::vector<std::wstring> ResultType;
		ResultType result;

#if SIG_WINDOWS_ENV
		WIN32_FIND_DATA fd;
		auto pass = DirpassTailModify(directory_pass, true) + L"*";
		auto hFind = FindFirstFile(pass.c_str(), &fd);

		if (hFind == INVALID_HANDLE_VALUE){
			return Nothing(std::move(result));
		}
		else{
			do{
				if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && BoolConsistency(hidden_file, (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)))
				{
					auto tmp = std::wstring(fd.cFileName);
					if(tmp != L"." && tmp != L"..") result.push_back(tmp);
				}
			} while (FindNextFile(hFind, &fd));

			FindClose(hFind);
			return Just<ResultType>::type(std::move(result));
		}
#elif SIG_ENABLE_BOOST
		auto IsHidden = [](fs::path const& p){
			auto name = p.filename();
			if (name.c_str()[0] == '.' && name != ".." &&	name != ".") return true;
			else return false;
		};

		fs::directory_iterator end;
		for (fs::directory_iterator it(directory_pass); it != end; ++it)
		{
			if (fs::is_directory(*it) && BoolConsistency(hidden_file, IsHidden(*it))){
				result.push_back(sig::Split(it->path().wstring(), L"\\").back());
			}
		}
		return Just<ResultType>::type(std::move(result));
#else
		static_asseet(false, "this OS is not support. please include boost if any.");
#endif
	}


	//overwrite�F�㏑��, append�F�����ǋL
	enum class WriteMode{ overwrite, append };

	//�t�@�C�����e�̏�����
	inline void FileClear(FileString const& file_pass)
	{
		std::ofstream ofs(file_pass);
		ofs << "";
	}


	//-- Save Text

	template <class S>
	void SaveLine(S const& src, typename FStreamSelector<TString<S>>::ofstream& ofs)
	{
		ofs << src << std::endl;
	}

	template <class C>
	void SaveLine(C const& src, typename FStreamSelector<typename container_traits<C>::value_type>::ofstream& ofs)
	{
		typename FStreamSelector<typename container_traits<C>::value_type>::ostreambuf_iterator streambuf_iter(ofs);
		for (auto const& str : src){
			std::copy(str.begin(), str.end(), streambuf_iter);
			streambuf_iter = '\n';
		}
	}

	//�t�@�C����1�s���ۑ�
	//src: �ۑ��Ώ�
	//file_pass: �ۑ���̃f�B���N�g���ƃt�@�C�����i�t���p�X�j
	//open_mode: �㏑��(overwrite) or �ǋL(append)
	template <class S, typename std::enable_if<container_traits<TString<S>>::is_string>::type*& = enabler>
	void SaveLine(S src, FileString const& file_pass, WriteMode mode = WriteMode::overwrite)
	{
		static bool first = true;
		if (first){
			std::locale::global(std::locale(""));
			first = false;
		}

		std::ios::open_mode const open_mode = mode == WriteMode::overwrite ? std::ios::out : std::ios::out | std::ios::app;
		typename FStreamSelector<TString<S>>::ofstream ofs(file_pass, open_mode);
		SaveLine(src, ofs);
	}
	//�܂Ƃ߂ĕۑ� (�V�[�P���X�ȓ��I�R���e�i�̂ݑΉ�)
	template <class C, typename std::enable_if<!container_traits<C>::is_string>::type*& = enabler>
	void SaveLine(C const& src, FileString const& file_pass, WriteMode mode = WriteMode::overwrite)
	{
		static bool first = true;
		if (first){
			std::locale::global(std::locale(""));
			first = false;
		}

		std::ios::open_mode const open_mode = mode == WriteMode::overwrite ? std::ios::out : std::ios::out | std::ios::app;
		typename FStreamSelector<typename container_traits<C>::value_type>::ofstream ofs(file_pass, open_mode);
		SaveLine(src, ofs);
	}

	//�ۑ�����f�[�^�����l�̏ꍇ
	//�f�t�H���g�ł�1�v�f1�s�ŕۑ��idelimiter�ŕύX�j
	template <class C>
	void SaveNum(C const& src, FileString const& file_pass, WriteMode mode = WriteMode::overwrite, std::string delimiter = "\n")
	{
		SaveLine(CatStr(src, delimiter), file_pass, mode);
	}


	//-- Read Text

	template <class R>
	using IfsSelector = typename std::conditional<std::is_same<R, std::string>::value, std::ifstream, std::wifstream>::type;

	//conv: �ǂݍ��񂾕����񂩂�C�ӌ^R�ւ̕ϊ��֐�(������ -> ���l�^�ւ�ReadNum�𐄏�) string or wstring -> R
	template <class C, class R = typename container_traits<C>::value_type>
	void ReadLine(C& empty_dest,
		IfsSelector<R>& ifs,
		std::function< R(typename std::conditional<std::is_same<R, std::string>::value, std::string, std::wstring>::type) > const& conv = nullptr)
	{
		static_assert(std::is_same<R, typename container_traits<C>::value_type>::value, "error in ReadLine: R and C::value_type don't match");

		typename std::conditional<std::is_same<R, std::string>::value, std::string, std::wstring>::type line;

		static bool first = true;
		if (first){
			std::locale::global(std::locale(""));
			first = false;
		}

		while (ifs && std::getline(ifs, line)){
			conv ? container_traits<C>::add_element(empty_dest, conv(std::move(line))) : container_traits<C>::add_element(empty_dest, std::move(line));
		}
	}

	template <class C, class R = typename container_traits<C>::value_type>
	void ReadLine(C& empty_dest,
		FileString const& file_pass,
		std::function< R(typename std::conditional<std::is_same<R, std::string>::value, std::string, std::wstring>::type)> const& conv = nullptr)
	{
		IfsSelector<R> ifs(file_pass);
		if (!ifs){
			std::wcout << L"file open error: " << file_pass << std::endl;
			return;
		}
		ReadLine(empty_dest, ifs, conv);
	}

	template <class C, class R = typename container_traits<C>::value_type>
	void ReadNum(C& empty_dest, FileString const& file_pass, std::string delimiter = "")
	{
		static_assert(std::is_same<R, typename container_traits<C>::value_type>::value, "error in ReadNum: R and C::value_type don't match");

		IfsSelector<std::string> ifs(file_pass);
		std::string line;

		static bool first = true;
		if (first){
			std::locale::global(std::locale(""));
			first = false;
		}

		if (!ifs){
			std::wcout << L"file open error: " << file_pass << std::endl;
			return;
		}
		while (ifs && std::getline(ifs, line)){
			auto split = Split(line, delimiter);
			for (auto v : split) container_traits<C>::add_element(empty_dest, Str2NumSelector<R>()(v));
		}
	}

	//�ǂݍ��ݎ��s: return -> nothing
	template <class R, class C = std::vector<R>>
	auto ReadLine(IfsSelector<R>& ifs) ->typename Just<C>::type
	{
		C tmp;
		ReadLine(tmp, ifs);
		return tmp.size() ? typename Just<C>::type(std::move(tmp)) : Nothing(std::move(tmp));
	}

	template <class R, class C = std::vector<R>>
	auto ReadLine(FileString const& file_pass) ->typename Just<C>::type
	{
		IfsSelector<R> ifs(file_pass);
		if (!ifs){
			std::wcout << L"file open error: " << file_pass << std::endl;
			return Nothing(C());
		}
		return ReadLine<R, C>(ifs);
	}

	template <class R, class C = std::vector<R>>
	auto ReadNum(FileString const& file_pass, std::string delimiter = "") ->typename Just<C>::type
	{
		C tmp;
		ReadNum(tmp, file_pass, delimiter);
		return tmp.size() ? typename Just<C>::type(std::move(tmp)) : Nothing(std::move(tmp));
	}

	/*
	//csv�ŕۑ�
	template <class Num>
	void SaveCSV(std::vector<std::vector<Num>> const& data, std::vector<std::string> const& row_names, std::vector<std::string> const& col_names, std::wstring const& out_fullpass)
	{
		std::ofstream ofs(out_fullpass);

		//first row: field name
		ofs << ",";
		for (uint i = 1; i < data[0].size() + 1; ++i){
			auto name = i - 1 < col_names.size() ? col_names[i - 1] : "";
			ofs << i << ". " << name << ",";
		}
		ofs << "\n";

		//first col: field name
		for (uint j = 0; j < data.size(); ++j){
			auto name = j < row_names.size() ? row_names[j] : "";
			ofs << j + 1 << ". " << name << ",";

			for (auto e : data[j]){
				ofs << e << ",";
			}
			ofs << "\n";
		}
	}
	*/
}

#endif