﻿/*
Copyright(c) 2014 Akihiro Nishimura

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/

#ifndef SIG_UTIL_FILE_HPP
#define SIG_UTIL_FILE_HPP

#include "string.hpp"
#include "helper.hpp"

#include <fstream>
#include <locale>

#if SIG_MSVC_ENV
	#include <windows.h>
#elif SIG_USE_BOOST_FILESYSTEM
	#include <boost/filesystem.hpp>
	namespace fs = boost::filesystem;
#endif


/* 入出力関連 */

namespace sig
{

// ディレクトリ・ファイルパスの末尾に'/'or'\'があるかチェックし、付けるか外すかどうか指定
inline auto modify_dirpass_tail(FilepassString const& directory_pass, bool const has_slash) ->FilepassString
{
	if (directory_pass.empty()) return directory_pass;

	auto tail = directory_pass.back();

	if (has_slash){
		//付ける場合
		if (tail == '/' || tail == '\\') return directory_pass;
#if SIG_MSVC_ENV
		else return (directory_pass + L"/");
#else
		else return (directory_pass + "/");
#endif
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

// 指定ディレクトリにあるファイル名を取得
// 必要条件：VisualStudio環境 または boostのインクルード
// directry_pass：調べたいディレクトリのパス
// hidden_file：true->隠しファイルのみ, false->非隠しファイルのみ (Windows, Linux環境のみ)
// extension：拡張子指定(オプション)
// 読み込み失敗: return -> nothing or empty-vector
inline auto get_file_names(
	FilepassString const& directory_pass,
	bool hidden_file,
	std::wstring extension = L""
	) ->Just<std::vector<std::wstring>>::type
{
	typedef std::vector<std::wstring> ResultType;
	ResultType result;

#if SIG_MSVC_ENV
	WIN32_FIND_DATA fd;
	auto query = extension.empty() ? L"?*" : L"*" + extension;
	auto pass = modify_dirpass_tail(directory_pass, true) + query;
	auto hFind = FindFirstFile(pass.c_str(), &fd);

	if (hFind == INVALID_HANDLE_VALUE){
		return Nothing(std::move(result));
	}
	else{
		do{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && Consistency(hidden_file, (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)))
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
		if (!fs::is_directory(*it) && Consistency(hidden_file, IsHidden(*it))){
			auto leaf = sig::split(it->path().wstring(), L"/").back();
			if (extension.empty()) result.push_back(leaf);
			else{
				auto query = L".*(" + escape_regex(extension) + L")";
				auto ext = sig::regex_search(leaf, SIG_WRegex(query));
				if (is_container_valid(ext) && fromJust(ext)[0][1] == extension) result.push_back(leaf);
			}
		}
	}
	return Just<ResultType>::type(std::move(result));
#else
	std::cout << "I don't support this envirnment which is default. please include boost if any." << std::endl; 
	assert(false);
#endif
}

// 指定ディレクトリにあるフォルダ名を取得
// 必要条件：VisualStudio環境 または boostのインクルード
// directry_pass：調べたいディレクトリのパス
// hidden_file：true->隠しファイルのみ, false->非隠しファイルのみ (Windows, Linux環境のみ)
// 読み込み失敗: return -> nothing or empty-vector
inline auto get_folder_names(
	FilepassString const& directory_pass,
	bool hidden_file
	) ->Just<std::vector<std::wstring>>::type
{
	typedef std::vector<std::wstring> ResultType;
	ResultType result;

#if SIG_MSVC_ENV
	WIN32_FIND_DATA fd;
	auto pass = modify_dirpass_tail(directory_pass, true) + L"*";
	auto hFind = FindFirstFile(pass.c_str(), &fd);

	if (hFind == INVALID_HANDLE_VALUE){
		return Nothing(std::move(result));
	}
	else{
		do{
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && Consistency(hidden_file, (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)))
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
		if (fs::is_directory(*it) && Consistency(hidden_file, IsHidden(*it))){
			result.push_back(sig::split(it->path().wstring(), L"/").back());
		}
	}
	return Just<ResultType>::type(std::move(result));
#else
	std::cout << "I don't support this envirnment which is default. please include boost if any." << std::endl; 
	assert(false);
#endif
}


//overwrite：上書き, append：末尾追記
enum class WriteMode{ overwrite, append };

//ファイル内容の初期化
// file_pass: 保存先のディレクトリとファイル名（フルパス）
inline void clear_file(FilepassString const& file_pass)
{
	std::ofstream ofs(file_pass);
	ofs << "";
}

#define SIG_FILE_LOCALE_INIT\
	static bool first = true;\
	if (first){\
		std::locale::global(std::locale(""));\
		first = false;\
	}

/* Save Text */

// ファイルへ1行ずつ保存
// src: 保存対象
// ofs: std::ofstream or std::wofstream
template <class T, typename std::enable_if<!container_traits<T>::exist>::type*& = enabler>
inline void save_line(
	T src,
	typename impl::FStreamSelector<T>::ofstream& ofs)
{
	ofs << src << std::endl;
}

// ファイルへ1行ずつ保存
// src: 保存対象(コンテナ)
// ofs: std::ofstream or std::wofstream
template <class C, typename std::enable_if<container_traits<C>::exist>::type*& = enabler>
void save_line(
	C const& src,
	typename impl::FStreamSelector<typename container_traits<C>::value_type>::ofstream& ofs)
{
	typename impl::FStreamSelector<typename container_traits<C>::value_type>::ostreambuf_iterator streambuf_iter(ofs);
	for (auto const& str : src){
		std::copy(str.begin(), str.end(), streambuf_iter);
		streambuf_iter = '\n';
	}
}

// ファイルへ1行ずつ保存
// src: 保存対象
// file_pass: 保存先のディレクトリとファイル名（フルパス）
// open_mode: 上書き(overwrite) or 追記(append)
template <class T, typename std::enable_if<!container_traits<T>::exist>::type*& = enabler>
void save_line(
	T src,
	FilepassString const& file_pass,
	WriteMode mode = WriteMode::overwrite)
{
	SIG_FILE_LOCALE_INIT

	const auto open_mode = mode == WriteMode::overwrite ? std::ios::out : std::ios::out | std::ios::app;
	typename impl::FStreamSelector<T>::ofstream ofs(file_pass, open_mode);
	save_line(src, ofs);
}

// ファイルへ1行ずつ保存
// src: 保存対象(コンテナ)
// file_pass: 保存先のディレクトリとファイル名（フルパス）
// open_mode: 上書き(overwrite) or 追記(append)
template <class C, typename std::enable_if<container_traits<C>::exist>::type*& = enabler>
void save_line(
	C const& src,
	FilepassString const& file_pass,
	WriteMode mode = WriteMode::overwrite)
{
	SIG_FILE_LOCALE_INIT

	const auto open_mode = mode == WriteMode::overwrite ? std::ios::out : std::ios::out | std::ios::app;
	typename impl::FStreamSelector<typename container_traits<C>::value_type>::ofstream ofs(file_pass, open_mode);
	save_line(src, ofs);
}


// 数値列(ex:ベクトル)の保存
// src: 保存対象(コンテナ)
// file_pass: 保存先のディレクトリとファイル名（フルパス）
// delimiter: 数値間の区切り文字を指定（ex: delimiter = "," で簡易CSV）
// open_mode: 上書き(overwrite) or 追記(append)
template <class C, typename std::enable_if<container_traits<C>::exist && !container_traits<typename container_traits<C>::value_type>::exist>::type*& = enabler>
void save_num(
	C const& src,
	FilepassString const& file_pass,
	std::string delimiter,
	WriteMode mode = WriteMode::overwrite)
{
	save_line(cat_str(src, delimiter), file_pass, mode);
}

// 2次元配列の数値(ex:行列)を保存
// src: 保存対象(コンテナのコンテナ)
// file_pass: 保存先のディレクトリとファイル名（フルパス）
// delimiter: 数値間の区切り文字を指定
// open_mode: 上書き(overwrite) or 追記(append)
template <class CC, typename std::enable_if<container_traits<typename container_traits<CC>::value_type>::exist>::type*& = enabler>
void save_num(
	CC const& src,
	FilepassString const& file_pass,
	std::string delimiter,
	WriteMode mode = WriteMode::overwrite)
{
	std::vector<std::string> tmp;
	for (auto const& line : src){
		tmp.push_back(cat_str(line, delimiter));
	}
	save_line(tmp, file_pass, mode);
}


/* Read Text */

template <class R>
using IfsSelector = typename impl::SameIf<R, std::string, std::ifstream, std::wifstream>::type;

// ファイルから1行ずつ読み込む
// empty_dest: 保存先の空のコンテナ
// ifs: std::ifstream or std::wifstream
// conv: 読み込んだ文字列から任意型Rへの変換関数(文字列 -> 数値型へはread_numを推奨) string or wstring -> R
template <class C, class R = typename container_traits<C>::value_type>
bool read_line(
	C& empty_dest,
	IfsSelector<R>& ifs,
	std::function< R(typename impl::SameIf<R, std::string, std::string, std::wstring>::type) > const& conv = nullptr)
{
	static_assert(std::is_same<R, typename container_traits<C>::value_type>::value, "error in read_line: R and C::value_type don't match");

	typename impl::SameIf<R, std::string, std::string, std::wstring>::type line;

	SIG_FILE_LOCALE_INIT

	while (ifs && std::getline(ifs, line)){
		conv ? container_traits<C>::add_element(empty_dest, conv(std::move(line))) : container_traits<C>::add_element(empty_dest, std::move(line));
	}
	return static_cast<bool>(ifs);
}

// ファイルから1行ずつ読み込む
// empty_dest: 保存先の空のコンテナ
// file_pass: 読み込み先のディレクトリとファイル名（フルパス）
// conv: 読み込んだ文字列から任意型Rへの変換関数(文字列 -> 数値型へはread_numを推奨) string or wstring -> R
template <class C, class R = typename container_traits<C>::value_type>
bool read_line(
	C& empty_dest,
	FilepassString const& file_pass,
	std::function< R(typename impl::SameIf<R, std::string, std::string, std::wstring>::type)> const& conv = nullptr)
{
	IfsSelector<R> ifs(file_pass);
	if (!ifs){
		//FileOpenErrorPrint(file_pass);
		return false;
	}
	return read_line(empty_dest, ifs, conv);
}

// ファイルから1行ずつ読み込み、結果を返す
// R: 返す文字列型(std::string or std::wstring)
// ifs: std::ifstream or std::wifstream
// 読込失敗: return -> nothing (boost非使用時は空のコンテナ)
template <class R, class C = std::vector<R>>
auto read_line(IfsSelector<R>& ifs) ->typename Just<C>::type
{
	C tmp;
	read_line(tmp, ifs);
	return tmp.size() ? typename Just<C>::type(std::move(tmp)) : Nothing(std::move(tmp));
}

// ファイルから1行ずつ読み込み、結果を返す
// R: 返す文字列型(std::string or std::wstring)
// file_pass: 読み込み先のディレクトリとファイル名（フルパス）
// 読込失敗: return -> nothing (boost非使用時は空のコンテナ)
template <class R, class C = std::vector<R>>
auto read_line(FilepassString const& file_pass) ->typename Just<C>::type
{
	IfsSelector<R> ifs(file_pass);
	if (!ifs){
		//FileOpenErrorPrint(file_pass);
		return Nothing(C());
	}
	return read_line<R, C>(ifs);
}

template <class R, class C = std::vector<R>>
auto read_line(FilepassStringC file_pass) ->typename Just<C>::type
{
	return read_line<R, C>(static_cast<impl::TString<FilepassStringC>>(file_pass));
}


// 数値列を読み込む
// empty_dest: 保存先の空のコンテナ
// file_pass: 読み込み先のディレクトリとファイル名（フルパス）
// delimiter: 数値間の区切り文字を指定 (デフォルトは行区切り)
// return -> 読み込みの成否
template <class C, class RT = typename container_traits<C>::value_type, typename std::enable_if<!container_traits<typename container_traits<C>::value_type>::exist>::type*& = enabler>
bool read_num(
	C& empty_dest,
	FilepassString const& file_pass,
	std::string delimiter = "\n")
{
	auto read_str = read_line<std::string>(file_pass);

	if (!is_container_valid(read_str)) return false;

	if (delimiter == "\n"){
		for (auto const& line : fromJust(read_str)){
			container_traits<C>::add_element(empty_dest, impl::Str2NumSelector<RT>()(line));
		}
	}
	else{
		auto sp = split(fromJust(read_str)[0], delimiter);
		for (auto v : sp) container_traits<C>::add_element(empty_dest, impl::Str2NumSelector<RT>()(v));
	}
	return true;
}

// 2次元配列の数値(ex:行列)を読み込む
// empty_dest: 保存先の空のコンテナのコンテナ
// file_pass: 読み込み先のディレクトリとファイル名（フルパス）
// delimiter: 数値間の区切り文字を指定
// return -> 読み込みの成否
template <class CC, class RC = typename container_traits<CC>::value_type, class RT = typename container_traits<RC>::value_type>
bool read_num(
	CC& empty_dest,
	FilepassString const& file_pass, 
	std::string delimiter)
{
	auto read_str = read_line<std::string>(file_pass);
	if (!is_container_valid(read_str)) return false;

	for (auto const& line : fromJust(read_str)){
		RC tmp;
		auto sp = split(line, delimiter);

		for (auto const& v : sp){
			container_traits<RC>::add_element(tmp, impl::Str2NumSelector<RT>()(v));
		}
		container_traits<CC>::add_element(empty_dest, std::move(tmp));
	}
	return true;
}

// 数値列を読み込み、結果のコンテナを返す
// file_pass: 読み込み先のディレクトリとファイル名（フルパス）
// delimiter: 数値間の区切り文字を指定 (デフォルトは行区切り)
// 読込失敗: return -> nothing (boost非使用時は空のコンテナ)
template <class C, typename std::enable_if<container_traits<C>::exist && !container_traits<typename container_traits<C>::value_type>::exist>::type*& = enabler>
auto read_num(
	FilepassString const& file_pass,
	std::string delimiter = "\n"
	) ->typename Just<C>::type
{
	C tmp;
	read_num(tmp, file_pass, delimiter);
	return tmp.size() ? typename Just<C>::type(std::move(tmp)) : Nothing(std::move(tmp));
}

// 2次元配列の数値(ex:行列)を読み込む
// file_pass: 読み込み先のディレクトリとファイル名（フルパス）
// delimiter: 数値間の区切り文字を指定
// 読込失敗: return -> nothing (boost非使用時は空のコンテナ)
template <class CC, typename std::enable_if<container_traits<typename container_traits<CC>::value_type>::exist>::type*& = enabler>
auto read_num(
	FilepassString const& file_pass,
	std::string delimiter
	) ->typename Just<CC>::type
{
	CC tmp;
	read_num(tmp, file_pass, delimiter);
	return tmp.size() ? typename Just<CC>::type(std::move(tmp)) : Nothing(std::move(tmp));
}

	/*
	//csvで保存
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
