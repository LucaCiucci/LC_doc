#pragma once

#include "clang_interface/Index.hpp"
#include "Symbol.hpp"

namespace lcdoc
{
	using std::string;
	using std::vector;
	using std::filesystem::path;

	class CXXDocumentParser
	{
	public:

		SymbolRegistry registry;

		void parse(const path& fileName, const vector<string>& args);

	private:
		clang::Index m_index;
	};

	void ci();
}