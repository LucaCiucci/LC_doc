
#pragma once

#include <clang-c/Index.h>

#include "Cursor.hpp"

namespace lcdoc::clang
{
	class Index
	{
	public:

		Index() {
			// excludeDeclsFromPCH = 1, displayDiagnostics=1
			m_idx = clang_createIndex(1, 1);
		}

		~Index() {
			clang_disposeIndex(m_idx);
		}

		::CXIndex handle() const {
			return m_idx;
		}

	private:
		::CXIndex m_idx = nullptr;
	};
}