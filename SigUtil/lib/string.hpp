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

#ifndef __SIG_UTIL_STRING__
#define __SIG_UTIL_STRING__

#include "sigutil.hpp"
#include "functional.hpp"
#include <regex>
#include <stdlib.h>
#include <sstream>

/* �����񏈗��֘A */

namespace sig{

	//expression�Ɋ܂܂�镶���Ɋւ��āA���K�\���̓��ꕶ�����G�X�P�[�v����
	inline auto RegexEscaper(std::string const& expression) ->std::string
	{
		static const std::regex escape_reg("([(){}\\[\\]|^?$.+*\\\\])");
		return std::regex_replace(expression, escape_reg, "\\$1");
	}
	inline auto RegexEscaper(std::wstring const& expression) ->std::wstring
	{
		static const std::wregex escape_reg(L"([(){}\\[\\]|^?$.+*\\\\])");
		return std::regex_replace(expression, escape_reg, L"\\$1");
	}

	//�����ŃG�X�P�[�v����std::regex or std::wregex ��Ԃ�
	template <class T>
	auto RegexMaker(T const& expression) ->typename Str2RegexSelector<TString<T>>::regex
	{
		return typename Str2RegexSelector<TString<T>>::regex(RegexEscaper(expression));
	}


	//std::regex_search �̃��b�p�֐��B
	//return -> maybe ? [�}�b�`�����ӏ��̏���][�}�b�`���̎Q�Ƃ̏���. 0�͑S��, 1�ȍ~�͎Q�Ɖӏ�] : nothing
	//��F
	//src = "test tes1 tes2"
	//expression = std::regex("tes(\\d)")
	//return -> [[tes1, 1], [tes2, 2]]
	template <template <class T_, class = std::allocator<T_>> class Container = std::vector, class T = std::string >
	auto RegexSearch(T src, typename Str2RegexSelector<TString<T>>::regex const& expression) ->typename Just< Container< Container<TString<T>>>>::type
	{
		Container<Container<TString<T>>> d;
		typename Str2RegexSelector<TString<T>>::smatch match;
		auto tmp = TString<T>(src);

		while (std::regex_search(tmp, match, expression)){
			d.push_back(Container<TString<T>>());
			for (auto const& m : match) d.back().push_back(m);
			tmp = match.suffix().str();
		}

		return d.empty() ? Nothing(std::move(d)) : typename Just<Container<Container<TString<T>>>>::type(std::move(d));
	}

/*
	//expression�Ɋ܂܂�镶���Ɋւ��āA���K�\���̓��ꕶ�����G�X�P�[�v���Ă��珈���i�����j
	template < class T, template < class T_, class = std::allocator<T_>> class Container = std::vector >
	auto RegexSearch(T src, T expression) ->typename Just< Container< Container<String<T>>>>::type
	{
		return RegexSearch(src, RegexMaker(expression));
	}
*/

	//HTML���Ƀ^�O���G���R�[�h�E�f�R�[�h����
	//��F�@<TAG>text<TAG>
	template <class String>
	class TagDealer
	{
		const String tel_;
		const String ter_;

	public:
		TagDealer(String tag_encloser_left, String tag_encloser_right) : tel_(tag_encloser_left), ter_(tag_encloser_right){};

		String Encode(String const& src, String const& tag) const{
			auto tag_str = tel_ + tag + ter_;
			return tag_str + src + tag_str;
		}
		
		auto Decode(String const& src, String const& tag) ->typename Just<String>::type const{
			auto tag_str = tel_ + tag + ter_;
			auto parse = Split(" " + src, tag_str);
			return parse.size() < 2 ? Nothing(String()) : typename Just<String>::type(parse[1]);
		}

		template < template < class T_, class Allocator = std::allocator<T_>> class Container >
		String Encode(Container<String> const& src, Container<String> const& tag) const;

#if SIG_ENABLE_BOOST
		template < template < class T_, class Allocator = std::allocator<T_>> class Container >
		auto Decode(String const& src, Container<String> const& tag) ->typename Just<Container<String>>::type const;
#endif
	};

	template <class String>
	template < template < class T_, class Allocator = std::allocator<T_>> class Container >
	String TagDealer<String>::Encode(Container<String> const& src, Container<String> const& tag) const
	{
		auto size = std::min(src.size(), tag.size());
		auto calc = ZipWith([&](Container<String>::value_type s, Container<String>::value_type t){ return Encode(s, t); }, src, tag);
		return std::accumulate(calc.begin(), calc.end(), String(""));
	}

#if SIG_ENABLE_BOOST
	template <class String>
	template < template < class T_, class Allocator = std::allocator<T_>> class Container >
	auto TagDealer<String>::Decode(String const& src, Container<String> const& tag) ->typename Just<Container<String>>::type const
	{
		Container<String> result;
		for (auto const& e : tag){
			if (auto d = Decode(src, e)) result.push_back(*d);
		}
		return result.empty() ? Nothing(String()) : typename Just<Container<String>>::type(std::move(result));
	}
#endif

	//������(src)�����镶����(delim)��ڈ�ɕ�������
	//�߂�l�̃R���e�i�̓f�t�H���g�ł�vector
	//��Fsrc="one, 2, �Q ", delim="," -> return vector<string>{"one", " 2", " �Q "}
	template <template <class T_, class = std::allocator<T_>> class Container = std::vector,  class String = std::string >
	auto Split(String src, typename std::common_type<String>::type const& delimiter) ->Container<typename StringId<String>::type>
	{
		Container<String> result;
		int const mag = delimiter.size();
		int cut_at;

		if (!mag) return Container<String>{src};

		while ((cut_at = src.find(delimiter)) != src.npos){
			if (cut_at > 0) result.push_back(src.substr(0, cut_at));
			src = src.substr(cut_at + mag);
		}
		if (src.length() > 0){
			result.push_back(src);
		}

		return std::move(result);
	}

	template < template <class T_, class = std::allocator<T_>> class Container = std::vector >
	Container<std::string> Split(char const* const src, char const* const delimiter)
	{
		return Split<Container>(std::string(src), std::string(delimiter));
	}

	template < template < class T_, class = std::allocator<T_>> class Container = std::vector >
	Container<std::wstring> Split(wchar_t const* const src, wchar_t const* const delimiter)
	{
		return Split<Container>(std::wstring(src), std::wstring(delimiter));
	}


	//�R���e�i�Ɋi�[���ꂽ�S���������������1�̕������(delimiter�ŋ�؂�w��)
	template <class C, class T>
	auto CatStr(C const& container, T delimiter)
	{
		typename SStreamSelector<typename container_traits<C>::value_type>::ostringstream osstream;
		if (container.empty()) return osstream.str();

		osstream << *container.begin();
		for (auto it = ++container.begin(), end = container.end(); it != end; ++it){
			osstream << delimiter << *it;
		}
		return osstream.str();
	}


	//UTF-16 to Shift-JIS
	inline std::string WSTRtoSTR(const std::wstring &src)
	{
		size_t mbs_size = src.length() * MB_CUR_MAX + 1;
		if (mbs_size < 2 || src == L"\0") return std::string();
		char *mbs = new char[mbs_size];
		size_t num;

		wcstombs_s(&num, mbs, mbs_size, src.c_str(), src.length() * MB_CUR_MAX + 1);
		std::string dest(mbs);
		delete[] mbs;

		return std::move(dest);
	}

	template < template < class T_, class = std::allocator<T_>> class Container >
	Container<std::string> WSTRtoSTR(Container<std::wstring> const& strvec)
	{
		Container<std::string> result;
		for (auto const& str : strvec) result.push_back(WSTRtoSTR(str));
		return std::move(result);
	}

	//Shift-JIS to UTF-16
	inline std::wstring STRtoWSTR(const std::string &src)
	{
		size_t wcs_size = src.length() + 1;
		if (wcs_size < 2 || src == "\0") return std::wstring();
		//std::cout << src << std::endl;
		wchar_t *wcs = new wchar_t[wcs_size];
		size_t num;

		mbstowcs_s(&num, wcs, wcs_size, src.c_str(), src.length() + 1);
		std::wstring dest(wcs);
		delete[] wcs;

		return std::move(dest);
	}

	template < template < class T_, class = std::allocator<T_>> class Container >
	Container<std::wstring> STRtoWSTR(Container<std::string> const& strvec)
	{
		Container<std::wstring> result;
		for (auto const& str : strvec) result.push_back(STRtoWSTR(str));
		return std::move(result);
	}

	//�S�p�E���p�����̒u���������s��
	class ZenHanReplace{
		std::unordered_map<wchar_t, wchar_t> alphabet_;			//�S�p�A���t�@�x�b�g -> ���p�A���t�@�x�b�g
		std::unordered_map<wchar_t, wchar_t> number_;			//�S�p���� -> ���p����
		std::unordered_map<wchar_t, wchar_t> katakana_;			//���p�J�^�J�i -> �S�p�J�^�J�i
		std::unordered_map<std::wstring, wchar_t> katakana_d_;	//(����) ���p�J�^�J�i -> �S�p�J�^�J�i

	private:
		ZenHanReplace(){
			wchar_t zen1[2] = L"�`";
			char han1 = 'A';

			for (uint i = 0; i < 26; ++i, ++zen1[0], ++han1){
				alphabet_[zen1[0]] = han1;
			}

			wchar_t zen2 = L'��';
			wchar_t han2 = 'a';
			for (uint i = 0; i < 26; ++i, ++zen2, ++han2){
				alphabet_[zen2] = han2;
			}

			number_[L'�O'] = L'0';
			number_[L'�P'] = L'1';
			number_[L'�Q'] = L'2';
			number_[L'�R'] = L'3';
			number_[L'�S'] = L'4';
			number_[L'�T'] = L'5';
			number_[L'�U'] = L'6';
			number_[L'�V'] = L'7';
			number_[L'�W'] = L'8';
			number_[L'�X'] = L'9';

			katakana_[L'�'] = L'�[';
			katakana_[L'�'] = L'�@';
			katakana_[L'�'] = L'�B';
			katakana_[L'�'] = L'�D';
			katakana_[L'�'] = L'�F';
			katakana_[L'�'] = L'�H';
			katakana_[L'�'] = L'�A';
			katakana_[L'�'] = L'�C';
			katakana_[L'�'] = L'�E';
			katakana_[L'�'] = L'�G';
			katakana_[L'�'] = L'�I';

			wchar_t han3 = L'�';
			wchar_t zen3 = L'�J';
			for (uint i = 0; i < 12; ++i, ++han3, zen3 += 2){
				katakana_[han3] = zen3;
			}
			katakana_[L'�'] = L'�b';
			katakana_[L'�'] = L'�e';
			katakana_[L'�'] = L'�g';
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
			katakana_[L'�'] = L'��';
			katakana_[L'�'] = L'��';
			katakana_[L'�'] = L'��';
			katakana_[L'�'] = L'��';
			katakana_[L'�'] = L'��';
			katakana_[L'�'] = L'��';
			han3 += 3; zen3 += 6;
			for (uint i = 0; i < 5; ++i, ++han3, ++zen3){
				katakana_[han3] = zen3;
			}
			katakana_[L'�'] = L'��';
			katakana_[L'�'] = L'��';
			katakana_[L'�'] = L'��';


			wchar_t han4[3] = L"��";
			wchar_t zen4 = L'�K';
			for (uint i = 0; i < 12; ++i, ++han4[0], zen4 += 2){
				katakana_d_[han4] = zen4;
			}
			katakana_d_[L"��"] = L'�d';
			katakana_d_[L"��"] = L'�f';
			katakana_d_[L"��"] = L'�h';
			han4[0] += 8; zen4 += 12;
			for (uint i = 0; i < 5; ++i, ++han4[0], zen4 += 3){
				katakana_d_[han4] = zen4;
			}
			wchar_t han5[3] = L"��";
			wchar_t zen5 = L'�p';
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

		void Alphabet_Zen2Han(std::wstring& sentence) const{
			for (auto& c : sentence){
				if (alphabet_.count(c)) c = alphabet_.at(c);
			}
		}
		void Alphabet_Han2Zen(std::wstring& sentence) const{
			for (auto& c : sentence){
				for (auto const& v : alphabet_){
					if (v.second == c) c = v.first;
				}
			}
		}

		void Number_Zen2Han(std::wstring& sentence) const{
			for (auto& c : sentence){
				if (number_.count(c)) c = number_.at(c);
			}
		}
		void Number_Han2Zen(std::wstring& sentence) const{
			for (auto& c : sentence){
				for (auto const& v : number_){
					if (v.second == c) c = v.first;
				}
			}
		}

		void Katakana_Han2Zen(std::wstring& sentence) const{
			for (unsigned i = 1; i < sentence.size(); ++i){
				if (sentence[i] == L'�' || sentence[i] == L'�'){
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
		void Katakana_Zen2Han(std::wstring& sentence) const{
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