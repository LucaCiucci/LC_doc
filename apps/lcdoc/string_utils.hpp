#pragma once

#include <vector>
#include <string>
#include <ranges>
#include <functional>
#include <cassert>

// !!!
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

namespace lcdoc
{
	using std::vector;
	using std::string;

	constexpr vector<string> split(string str, const string& delimiter, bool skipEmpty = false);
	constexpr string join(const vector<string>& strs, const string& delimiter);
	constexpr string indent(string str, int count = 1, const string& style = "    ");

	template <class Tin>
	constexpr vector<string> to_stringv(const vector<Tin>& range, const std::function<string(const Tin&)>& transformer)
	{
		vector<string> result;
		result.reserve(range.size());
		for (const auto& e : range)
			result.push_back(transformer(e));
		return result;
	}

	constexpr vector<string> split(string str, const string& delimiter, bool skipEmpty)
	{
		// https://gist.github.com/tcmug/9712f9192571c5fe65c362e6e86266f8
		std::vector<string> result;
		size_t from = 0, to = 0;
		while (string::npos != (to = str.find(delimiter, from))) {
			result.push_back(str.substr(from, to - from));
			from = to + delimiter.length();
		}
		result.push_back(str.substr(from, to));
		return result;
	}

	constexpr string join(const vector<string>& strs, const string& delimiter)
	{
		string result;
		for (auto it = strs.begin(); it != strs.end() && std::next(it) != strs.end(); ++it)
			result += *it + delimiter;
		if (!strs.empty())
			result += strs.back();
		return result;
	}

	constexpr string indent(string str, int count, const string& style)
	{
		string indent;
		for (int i = 0; i < count; ++i)
			indent += style;
		auto pieces = split(str, "\n");
		for (auto& piece : pieces)
			piece = indent + piece;
		return join(pieces, "\n");
	}

	namespace {
		// https://en.cppreference.com/w/cpp/string/basic_string/replace
		inline std::size_t replace_all(std::string& inout, std::string_view what, std::string_view with)
		{
			std::size_t count{};
			for (std::string::size_type pos{};
				inout.npos != (pos = inout.find(what.data(), pos, what.length()));
				pos += with.length(), ++count) {
				inout.replace(pos, what.length(), with.data(), with.length());
			}
			return count;
		}

		string escapeHtml(const string& text) {
			// TODO missing &, unicode, ....
			auto result = text;
			replace_all(result, "<", "&lt;");
			replace_all(result, ">", "&gt;");
			return result;
		}

		string unescapeHtml(const string& text) {
			// TODO missing &, unicode, ....
			auto result = text;
			replace_all(result, "&lt;", "<");
			replace_all(result, "&gt;", ">");
			return result;
		}

		// TODO escape/unescape string literal
	}

	inline vector<string> operator|(const vector<string>& left, const vector<string>& right) {
		vector<string> result;
		result.reserve(left.size() + right.size());

		for (const auto& piece : left)
			result.push_back(piece);
		for (const auto& piece : right)
			result.push_back(piece);

		return result;
	}

	inline vector<string> operator|(vector<string>&& left, const vector<string>& right) {
		left.insert(left.end(), right.begin(), right.end());
		return left;
	}

	inline vector<string> operator|(const vector<string>& left, vector<string>&& right) {
		right.insert(right.begin(), left.begin(), left.end());
		return right;
	}

	inline vector<string> operator|(vector<string>&& left, vector<string>&& right) {
		// not implemented, yet
		assert(0);
		return left | right;
	}

	struct extractMeta_result {
		string meta;
		string content;
	};
	inline extractMeta_result extractMeta(const string& raw)
	{
		extractMeta_result result;

		const string sep = "\n---\n";
		const auto pos = raw.find(sep);
		if (pos == std::string::npos)
		{
			result.content = raw;
			return result;
		}

		result.meta = raw.substr(0, pos);
		result.content = raw.substr(pos + sep.length());
		return result;
	}

	nlohmann::json to_json(const YAML::Node& node);
}