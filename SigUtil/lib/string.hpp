﻿/*
Copyright(c) 2014 Akihiro Nishimura

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/

#ifndef SIG_UTIL_STRING_HPP
#define SIG_UTIL_STRING_HPP

#include "sigutil.hpp"
#include "type_map.hpp"
#include "functional.hpp"
#include <regex>
#include <stdlib.h>
#include <sstream>

#if SIG_MSVC_ENV
#include <windows.h>
#endif

#include <codecvt>

#include "tool.hpp"

/* 文字列処理関連 */

namespace sig{

#if SIG_MSVC_ENV

// expressionに含まれる文字に関して、正規表現の特殊文字をエスケープする
inline auto escape_regex(std::string const& expression) ->std::string
{
	static const SIG_Regex escape_reg(R"(([(){}\[\]|^?$.+*\\]))");
	return SIG_RegexReplace(expression, escape_reg, std::string(R"(\$1)"));
}
inline auto escape_regex(std::wstring const& expression) ->std::wstring
{
	static const SIG_WRegex escape_reg(LR"(([(){}\[\]|^?$.+*\\]))");
	return SIG_RegexReplace(expression, escape_reg, std::wstring(LR"(\$1)"));
}

// エスケープ処理を行い、std::regex or std::wregexを返す
template <class S>
auto make_regex(S const& expression) ->typename Str2RegexSelector<TString<S>>::regex
{
	return typename Str2RegexSelector<TString<S>>::regex(escape_regex(expression));
}
#endif

// std::regex_search のラッパ関数
// src: 探索対象の文字列
// expression: 正規表現 ((windows)std::regex or (others)boost::regex)
// return -> (探索の成否) ? [マッチした箇所の順番][マッチ内の参照の順番. 0は全文, 1以降は参照箇所] : nothing
// 例：
// src = "test tes1 tes2"
// expression = std::regex("tes(\\d)")
// return -> [[tes1, 1], [tes2, 2]]
template <class S>
auto regex_search(
	S src,
	typename Str2RegexSelector<TString<S>>::regex const& expression
	) ->typename Just<std::vector<std::vector<TString<S>>>>::type
{
	using R = std::vector<std::vector<TString<S>>>;
	R d;
	typename Str2RegexSelector<TString<S>>::smatch match;
	auto tmp = TString<S>(src);

	while (SIG_RegexSearch(tmp, match, expression)){
		d.push_back(std::vector<TString<S>>());
		for (auto const& m : match) d.back().push_back(m);
		tmp = match.suffix().str();
	}

	return d.empty() ? Nothing(std::move(d)) : typename Just<R>::type(std::move(d));
}

// 文字列(src)をある文字列(delimiter)を目印に分割する
// src: 分割対象の文字列
// delimiter: 分割の目印となる文字列
// return -> 分割後の文字列が格納されたシーケンスコンテナ(templateパラメータでコンテナの種類を指定可)
// 例：
// src="one, 2, 参 ", delim="," -> return vector<string>{"one", " 2", " 参 "}
template <template <class T_, class = std::allocator<T_>> class CSeq = std::vector, class S = std::string >
auto split(
	S src,
	TString<S> const& delimiter
	) ->CSeq<TString<S>>
{
	CSeq<S> result;
	int const mag = delimiter.size();
	int cut_at;

	if (!mag) return CSeq<S>{src};

	while ((cut_at = src.find(delimiter)) != src.npos){
		if (cut_at > 0) result.push_back(src.substr(0, cut_at));
		src = src.substr(cut_at + mag);
	}
	if (src.length() > 0){
		result.push_back(src);
	}

	return result;
}

template < template <class T_, class = std::allocator<T_>> class CSeq = std::vector >
CSeq<std::string> split(char const* const src, char const* const delimiter)
{
	return split<CSeq>(std::string(src), std::string(delimiter));
}

template < template < class T_, class = std::allocator<T_>> class CSeq = std::vector >
CSeq<std::wstring> split(wchar_t const* const src, wchar_t const* const delimiter)
{
	return split<CSeq>(std::wstring(src), std::wstring(delimiter));
}


// コンテナに格納された全文字列を結合して1つの文字列に(delimiterで区切り指定)
// container: 文字列が格納されたコンテナ
// delimiter: 結合する文字列間に挿入される文字列
// return -> 結合した文字列
template <class C, class T>
auto cat_str(C const& container, T delimiter)
{
	typename SStreamSelector<typename container_traits<C>::value_type>::ostringstream osstream;
	if (container.empty()) return osstream.str();

	osstream << *std::begin(container);
	for (auto it = ++std::begin(container), end = std::end(container); it != end; ++it){
		osstream << delimiter << *it;
	}
	return osstream.str();
}


// ワイド文字 -> マルチバイト文字 (ex: Windows環境では UTF-16 -> Shift-JIS)
// src:	変換対象の文字列
// return -> 変換後の文字列
inline auto wstr_to_str(std::wstring const& src) ->std::string //Just<std::string>::type
{
	size_t mbs_size = src.length() * MB_CUR_MAX + 1;
	if (mbs_size < 2 || src == L"\0") return std::string();
	char *mbs = new char[mbs_size];

#if SIG_MSVC_ENV && SIG_DEBUG_MODE
	size_t num;
	int error = wcstombs_s(&num, mbs, mbs_size, src.c_str(), src.length() * MB_CUR_MAX + 1);
#else
	int error = wcstombs(mbs, src.c_str(), src.length() * MB_CUR_MAX + 1);
#endif
	std::string dest(mbs);
	delete[] mbs;

	return error ? std::string() : dest; //error ? Nothing(dest) : Just<std::string>::type(std::move(dest));
}

// src: 変換対象の文字列が格納されたコンテナ
// return -> 変換後の文字列が格納されたコンテナ
template <class C, typename std::enable_if<std::is_same<typename container_traits<C>::value_type, std::wstring>::value>::type*& = enabler>
auto wstr_to_str(C const& strvec)
{
	using R = typename container_traits<C>::template rebind<std::string>;
	R result;
	for (auto const& str : strvec){
		std::string r = wstr_to_str(str);
		if (!r.empty()) container_traits<R>::add_element(result, std::move(r));
		//result.push_back(fromJust(wstr_to_str(str)));
	}
	return result;
}

// マルチバイト文字 -> ワイド文字 (ex: Windows環境では Shift-JIS -> UTF-16)
// src:	変換対象の文字列
// return -> 変換後の文字列
inline auto str_to_wstr(std::string const& src) ->std::wstring //Just<std::wstring>::type
{
	size_t wcs_size = src.length() + 1;
	if (wcs_size < 2 || src == "\0") return std::wstring();
	wchar_t *wcs = new wchar_t[wcs_size];

#if SIG_MSVC_ENV && SIG_DEBUG_MODE
	size_t num;
	int error = mbstowcs_s(&num, wcs, wcs_size, src.c_str(), src.length() + 1);
#else
	int error = mbstowcs(wcs, src.c_str(), src.length() + 1);
#endif
	std::wstring dest(wcs);
	delete[] wcs;
	return error ? std::wstring() : dest; //error ? Nothing(dest) : Just<std::wstring>::type(std::move(dest));
}

// src: 変換対象の文字列が格納されたコンテナ
// return -> 変換後の文字列が格納されたコンテナ
template <class C, typename std::enable_if<std::is_same<typename container_traits<C>::value_type, std::string>::value>::type*& = enabler>
auto str_to_wstr(C const& strvec)
{
	using R = typename container_traits<C>::template rebind<std::wstring>;
	R result;
	for (auto const& str : strvec){
		std::wstring r = str_to_wstr(str);
		if (!r.empty()) container_traits<R>::add_element(result, std::move(r));
		//result.push_back(fromJust(str_to_wstr(str)));
	}
	return result;
}


// UTF-8 -> UTF-16
inline auto utf8_to_utf16(std::string const& src) ->std::u16string
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	std::u16string dest = convert.from_bytes(src);
	return dest;
}

// UTF-16 -> UTF-8
inline auto utf16_to_utf8(std::u16string const& src) ->std::string
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	std::string dest = convert.to_bytes(src);
	return dest;
}

// UTF-8 -> UTF-32
inline auto utf8_to_utf32(std::string const& src) ->std::u32string
{
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf32conv;
	std::u32string dest = utf32conv.from_bytes(src);
	return dest;
}

// UTF-32 -> UTF-8
inline auto utf32_to_utf8(std::u32string const& src) ->std::string
{
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf32conv;
	std::string dest = utf32conv.to_bytes(src);
	return dest;
}

#if SIG_MSVC_ENV

// ShiftJIS -> UTF-16
inline auto sjis_to_utf16(std::string const& src) ->std::u16string //Just<std::u16string>::type
{
	const int dest_size = ::MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, NULL, 0);
	BYTE* buf = new BYTE[dest_size * 2 + 2];

	bool is_succeed = ::MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, reinterpret_cast<LPWSTR>(buf), dest_size);
	std::u16string dest(reinterpret_cast<char16_t*>(buf));
	delete[] buf;

	return is_succeed ? dest : std::u16string(); //is_succeed ? Just<std::u16string>::type(std::move(dest)) : Nothing(std::u16string());
}

// UTF-16 -> ShiftJIS
inline auto utf16_to_sjis(std::u16string const& src) ->std::string //Just<std::string>::type
{
	LPCWSTR srcp = reinterpret_cast<LPCWSTR>(src.c_str());
	const int dest_size = ::WideCharToMultiByte(CP_ACP, 0, srcp, -1, NULL, 0, NULL, NULL);
	BYTE* buf = new BYTE[dest_size * 2];

	bool is_succeed = ::WideCharToMultiByte(CP_ACP, 0, srcp, -1, reinterpret_cast<LPSTR>(buf), dest_size, NULL, NULL);
	std::string dest(reinterpret_cast<char*>(buf));
	delete[] buf;

	return is_succeed ? dest : std::string(); //is_succeed ? Just<std::string>::type(std::move(dest)) : Nothing(std::string());
}

// ShiftJIS -> UTF-8
inline auto sjis_to_utf8(std::string const& src) ->std::string
{
	auto utf16 = sjis_to_utf16(src);
	return utf16_to_utf8(utf16);
}

// UTF-8 -> ShiftJIS
inline auto utf8_to_sjis(std::string const& src) ->std::string
{
	auto utf16 = utf8_to_utf16(src);
	return utf16_to_sjis(utf16);
}
	
#endif


//HTML風にタグをエンコード・デコードする
//例： <TAG>text<TAG>
template <class String>
class TagDealer
{
	const String tel_;
	const String ter_;

public:
	//左右それぞれの囲み文字を指定(ex. left = "<", right= ">")
	TagDealer(String tag_encloser_left, String tag_encloser_right) : tel_(tag_encloser_left), ter_(tag_encloser_right){};

	String encode(String const& src, String const& tag) const{
		auto tag_str = tel_ + tag + ter_;
		return tag_str + src + tag_str;
	}

	auto decode(String const& src, String const& tag) ->typename Just<String>::type const{
		auto tag_str = tel_ + tag + ter_;
		auto parse = split(" " + src, tag_str);
		return parse.size() < 2 ? Nothing(String()) : typename Just<String>::type(parse[1]);
	}

	template < template < class T_, class Allocator = std::allocator<T_>> class Container >
	String encode(Container<String> const& src, Container<String> const& tag) const;

#if SIG_ENABLE_BOOST
	template < template < class T_, class Allocator = std::allocator<T_>> class Container >
	auto decode(String const& src, Container<String> const& tag) ->typename Just<Container<String>>::type const;
#endif
};

template <class String>
template < template < class T_, class Allocator = std::allocator<T_>> class Container >
String TagDealer<String>::encode(Container<String> const& src, Container<String> const& tag) const
{
	auto size = Min(src.size(), tag.size());
	auto calc = zipWith([&](typename Container<String>::value_type s, typename Container<String>::value_type t){ return encode(s, t); }, src, tag);
	return std::accumulate(calc.begin(), calc.end(), String(""));
}

#if SIG_ENABLE_BOOST
template <class String>
template < template < class T_, class Allocator = std::allocator<T_>> class Container >
auto TagDealer<String>::decode(String const& src, Container<String> const& tag) ->typename Just<Container<String>>::type const
{
	Container<String> result;
	for (auto const& e : tag){
		if (auto d = decode(src, e)) result.push_back(*d);
	}
	return result.empty() ? Nothing(String()) : typename Just<Container<String>>::type(std::move(result));
}
#endif

//全角・半角文字の置換処理を行う
class ZenHanReplace{
	std::unordered_map<wchar_t, wchar_t> alphabet_;			//全角アルファベット -> 半角アルファベット
	std::unordered_map<wchar_t, wchar_t> number_;			//全角数字 -> 半角数字
	std::unordered_map<wchar_t, wchar_t> katakana_;			//半角カタカナ -> 全角カタカナ
	std::unordered_map<std::wstring, wchar_t> katakana_d_;	//(濁音) 半角カタカナ -> 全角カタカナ

private:
	ZenHanReplace(){
		wchar_t zen1[2] = L"Ａ";
		char han1 = 'A';

		for (uint i = 0; i < 26; ++i, ++zen1[0], ++han1){
			alphabet_[zen1[0]] = han1;
		}

		wchar_t zen2 = L'ａ';
		wchar_t han2 = 'a';
		for (uint i = 0; i < 26; ++i, ++zen2, ++han2){
			alphabet_[zen2] = han2;
		}

		number_[L'０'] = L'0';
		number_[L'１'] = L'1';
		number_[L'２'] = L'2';
		number_[L'３'] = L'3';
		number_[L'４'] = L'4';
		number_[L'５'] = L'5';
		number_[L'６'] = L'6';
		number_[L'７'] = L'7';
		number_[L'８'] = L'8';
		number_[L'９'] = L'9';
		katakana_[L'ｰ'] = L'ー';
		katakana_[L'ｧ'] = L'ァ';
		katakana_[L'ｨ'] = L'ィ';
		katakana_[L'ｩ'] = L'ゥ';
		katakana_[L'ｪ'] = L'ェ';
		katakana_[L'ｫ'] = L'ォ';
		katakana_[L'ｱ'] = L'ア';
		katakana_[L'ｲ'] = L'イ';
		katakana_[L'ｳ'] = L'ウ';
		katakana_[L'ｴ'] = L'エ';
		katakana_[L'ｵ'] = L'オ';
		
		wchar_t han3 = L'ｶ';
		wchar_t zen3 = L'カ';
		for (uint i = 0; i < 12; ++i, ++han3, zen3 += 2){
			katakana_[han3] = zen3;
		}
		katakana_[L'ｯ'] = L'ッ';
		katakana_[L'ﾃ'] = L'テ';
		katakana_[L'ﾄ'] = L'ト';
		han3 += 2; zen3 += 6;
		for (uint i = 0; i < 6; ++i, ++han3, ++zen3){
			katakana_[han3] = zen3;
		}
		for (uint i = 0; i < 5; ++i, ++han3, zen3 += 3){
			katakana_[han3] = zen3;
		}
		for (uint i = 0; i < 5; ++i, ++han3, ++zen3){
			katakana_[han3] = zen3;
		}
		katakana_[L'ﾔ'] = L'ヤ';
		katakana_[L'ｬ'] = L'ャ';
		katakana_[L'ﾕ'] = L'ユ';
		katakana_[L'ｭ'] = L'ュ';
		katakana_[L'ﾖ'] = L'ヨ';
		katakana_[L'ｮ'] = L'ョ';
		han3 += 3; zen3 += 6;
		for (uint i = 0; i < 5; ++i, ++han3, ++zen3){
			katakana_[han3] = zen3;
		}
		katakana_[L'ﾜ'] = L'ワ';
		katakana_[L'ｦ'] = L'ヲ';
		katakana_[L'ﾝ'] = L'ン';

		wchar_t han4[3] = L"ｶﾞ";
		wchar_t zen4 = L'ガ';
		for (uint i = 0; i < 12; ++i, ++han4[0], zen4 += 2){
			katakana_d_[han4] = zen4;
		}
		katakana_d_[L"ﾂﾞ"] = L'ヅ';
		katakana_d_[L"ﾃﾞ"] = L'デ';
		katakana_d_[L"ﾄﾞ"] = L'ド';
		han4[0] += 8; zen4 += 12;
		for (uint i = 0; i < 5; ++i, ++han4[0], zen4 += 3){
			katakana_d_[han4] = zen4;
		}
		wchar_t han5[3] = L"ﾊﾟ";
		wchar_t zen5 = L'パ';
		for (uint i = 0; i < 5; ++i, ++han5[0], zen5 += 3){
			katakana_d_[han5] = zen5;
		}
	}

	ZenHanReplace(const ZenHanReplace&) = delete;

public:
	static ZenHanReplace& GetInstance(){
		static ZenHanReplace instance;
		return instance;
	}

	void alphabet_zen2han(std::wstring& sentence) const{
		for (auto& c : sentence){
			if (alphabet_.count(c)) c = alphabet_.at(c);
		}
	}
	void alphabet_han2zen(std::wstring& sentence) const{
		for (auto& c : sentence){
			for (auto const& v : alphabet_){
				if (v.second == c) c = v.first;
			}
		}
	}

	void number_zen2han(std::wstring& sentence) const{
		for (auto& c : sentence){
			if (number_.count(c)) c = number_.at(c);
		}
	}
	void number_han2zen(std::wstring& sentence) const{
		for (auto& c : sentence){
			for (auto const& v : number_){
				if (v.second == c) c = v.first;
			}
		}
	}

	void katakana_han2zen(std::wstring& sentence) const{
		for (unsigned i = 1; i < sentence.size(); ++i){
			if (sentence[i] == L'ﾞ' || sentence[i] == L'ﾟ'){
				auto ttmp = std::wstring(sentence.substr(i-1, 2));
				if (katakana_d_.count(ttmp)){
					sentence.replace(i - 1, 2, 1, katakana_d_.at(ttmp));
					--i;
				}
			}
		}
		for (auto& c : sentence){
			if (katakana_.count(c)) c = katakana_.at(c);
		}
	}
	void katakana_zen2han(std::wstring& sentence) const{
		for (uint i=0; i<sentence.size(); ++i){
			for (auto const& v : katakana_d_){
				if (v.second == sentence[i]){
					sentence.replace(i, 1, v.first);
				}
			}
		}
		for (auto& c : sentence){
			for (auto const& v : katakana_){
				if (v.second == c) c = v.first;
			}
		}
	}

};

}
#endif