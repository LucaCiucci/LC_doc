
#include "TranslationUnit.hpp"

namespace lcdoc::clang
{
	TranslationUnit::TranslationUnit(const Index& idx, const path& srcFile, const vector<string>& clang_args)
	{
		vector<const char*> cargs;
		for (const auto& arg : clang_args)
			cargs.push_back(arg.c_str());

		// example:
		// // This will load all the symbols from 'IndexTest.c', excluding symbols
		// // from 'IndexTest.pch'.
		// char* args[] = { "-Xclang", "-include-pch=IndexTest.pch" };
		// TU = clang_createTranslationUnitFromSourceFile(Idx, "IndexTest.c", 2, args,
		// 	0, 0);
		// clang_visitChildren(clang_getTranslationUnitCursor(TU),
		// 	TranslationUnitVisitor, 0);
		// clang_disposeTranslationUnit(TU);

		m_TU = clang_createTranslationUnitFromSourceFile(
			idx.handle(),               // index
			srcFile.string().c_str(),   // file
			cargs.size(), cargs.data(), // command line clang args
			0, nullptr                  // unsave files
		);
	}

	TranslationUnit::~TranslationUnit()
	{
		clang_disposeTranslationUnit(m_TU);
	}

	CursorRef TranslationUnit::cursor()
	{
		return clang_getTranslationUnitCursor(m_TU);
	}
}