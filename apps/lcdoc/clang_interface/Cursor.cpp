
#include "Cursor.hpp"


namespace lcdoc::clang
{
	string to_string(::CXString&& str)
	{
		if (str.data == nullptr)
			return {};

		string result((const char*)str.data);
		clang_disposeString(str);
		return result;
	}

	string to_string(::CXCursorKind&& kind)
	{
		return to_string(clang_getCursorKindSpelling(kind));
	}

	string to_string(::CXTypeKind&& kind)
	{
		return to_string(clang_getTypeKindSpelling(kind));
	}

	vector<TypeRef> TypeRef::argTypes() const
	{
		int num = clang_getNumArgTypes(m_type);
		vector<TypeRef> result;
		for (int i = 0; i < num; ++i)
			result.push_back(clang_getArgType(m_type, i));
		return result;
	}

	TypeRef TypeRef::canonical() const
	{
		return clang_getCanonicalType(m_type);
	}

	CursorRef TypeRef::declaration() const
	{
		return clang_getTypeDeclaration(m_type);
	}

	CursorRef TypeRef::definition() const
	{
		return this->declaration().definition();
	}

	TypeRef TypeRef::resultType() const
	{
		return clang_getResultType(m_type);
	}

	CursorRef::Visitor CursorRef::m_visitor = {};

	CursorRef::CursorRef():
		m_cursor(clang_getNullCursor())
	{
	}

	CursorRef::CursorRef(nullptr_t) :
		CursorRef()
	{
	}

	CursorRef::CursorRef(::CXCursor cursor) :
		m_cursor(cursor)
	{
	}

	CursorRef& CursorRef::operator=(nullptr_t)
	{
		return *this = CursorRef();
	}

	CursorRef& CursorRef::operator=(::CXCursor cursor)
	{
		m_cursor = cursor;
		return *this;
	}

	CursorRef::operator bool() const
	{
		return !are_equals(m_cursor, clang_getNullCursor());
	}

	bool CursorRef::operator==(const CursorRef& other) const
	{
		return are_equals(m_cursor, other.m_cursor);
	}

	bool CursorRef::operator==(const ::CXCursor& other) const
	{
		return are_equals(m_cursor, other);
	}

	CursorRef CursorRef::canonical() const
	{
		return clang_getCanonicalCursor(m_cursor);
	}

	string CursorRef::spelling() const
	{
		return to_string(clang_getCursorSpelling(m_cursor));
	}

	CursorRef::Location CursorRef::location() const
	{
		return clang_getCursorLocation(m_cursor);
	}

	CursorRef CursorRef::definition() const
	{
		return clang_getCursorDefinition(m_cursor);
	}

	bool CursorRef::isDeclaration() const
	{
		return clang_isDeclaration(this->kind());
	}

	bool CursorRef::isDefinition() const
	{
		return clang_isCursorDefinition(m_cursor);
	}

	::CXCursorKind CursorRef::kind() const
	{
		return clang_getCursorKind(m_cursor);
	}

	string CursorRef::briefCommentText() const
	{
		return to_string(clang_Cursor_getBriefCommentText(m_cursor));
	}

	string CursorRef::rawCommentText() const
	{
		return to_string(clang_Cursor_getRawCommentText(m_cursor));
	}

	string CursorRef::mangled_name() const
	{
		return to_string(clang_Cursor_getMangling(m_cursor));
	}

	TypeRef CursorRef::type() const
	{
		return clang_getCursorType(m_cursor);
	}

	void CursorRef::visitChildren(const Visitor& visitor) const
	{
		m_visitor = visitor;
		{
			// This will load all the symbols from 'IndexTest.pch'
			clang_visitChildren(
				m_cursor,
				m_cursorVisitor,
				nullptr
			);
		}
		m_visitor = {};
	}

	vector<CursorRef> CursorRef::listChildren(bool recursive) const
	{
		vector<CursorRef> children;
		this->visitChildren([&](const CursorRef& cursor, const ::CXCursor& parent, const ::CXClientData& client_data) {
			children.push_back(cursor);
			return recursive ? ::CXChildVisitResult::CXChildVisit_Recurse : ::CXChildVisitResult::CXChildVisit_Continue;
		});
		return children;
	}

	vector<CursorRef> CursorRef::argumnts() const
	{
		vector<CursorRef> arguments;
		int nArgs = clang_Cursor_getNumArguments(m_cursor);
		for (int i = 0; i < nArgs; ++i)
			arguments.push_back(clang_Cursor_getArgument(m_cursor, i));
		return arguments;
	}

	TypeRef CursorRef::resultType() const
	{
		return clang_getCursorResultType(m_cursor);
	}

	bool CursorRef::are_equals(const ::CXCursor& left, const ::CXCursor& right)
	{
		if (left.kind != right.kind)
			return false;
		if (left.xdata != right.xdata)
			return false;
		for (size_t i = 0; i < std::size(left.data); ++i)
			if (left.data[i] != right.data[i])
				return false;
		return true;
	}

	::CXChildVisitResult CursorRef::m_cursorVisitor(::CXCursor cursor, ::CXCursor parent, ::CXClientData client_data) {
		if (m_visitor)
		{
			CursorRef cursorRef(cursor);
			return m_visitor(cursorRef, parent, client_data);
		}
		return CXChildVisitResult::CXChildVisit_Break;
	}
}