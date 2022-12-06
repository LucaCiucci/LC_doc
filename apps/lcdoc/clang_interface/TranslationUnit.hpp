#pragma once

#include <filesystem>

#include <clang-c/Index.h>

#include "Index.hpp"
#include "Cursor.hpp"

namespace lcdoc::clang
{
	using std::filesystem::path;
	using std::vector;

	class TranslationUnit
	{
	public:

		// TranslationUnit(const ::CXIndex& idx) {
		// 	// IndexTest.pch was produced with the following command:
		// 	// "clang -x c IndexTest.h -emit-ast -o IndexTest.pch"
		// 	m_TU = clang_createTranslationUnit(idx, "IndexTest.pch");
		// }

		TranslationUnit(const Index& idx, const path& srcFile, const vector<string>& clang_args);

		~TranslationUnit();

		CursorRef cursor();

	private:
		::CXTranslationUnit m_TU = nullptr;
	};
}