#pragma once

#include <filesystem>
#include <string>
#include <functional>
#include <vector>

#include <clang-c/Index.h>

namespace lcdoc::clang
{
	using std::string;
	using std::function;
	using std::vector;
	using std::filesystem::path;

	string to_string(::CXString&& str);
	string to_string(::CXCursorKind&& kind);
	string to_string(::CXTypeKind&& kind);

	class CursorRef;

	class TypeRef
	{
	public:

		TypeRef(::CXType type) : m_type(type) {
		}

		string spelling() const {
			return to_string(clang_getTypeSpelling(m_type));
		}

		::CXTypeKind kind() const {
			return m_type.kind;
		}

		string kindSpelling() const {
			return to_string(clang_getTypeKindSpelling(this->kind()));
		}

		vector<TypeRef> argTypes() const;

		TypeRef canonical() const;

		CursorRef declaration() const;
		CursorRef definition() const;

		TypeRef resultType() const;

		TypeRef pointee() const {
			return clang_getPointeeType(m_type);
		}

		bool constQualified() const {
			return clang_isConstQualifiedType(m_type);
		}

		TypeRef named() const {
			return clang_Type_getNamedType(m_type);
		}

		string typedefName() const {
			return to_string(clang_getTypedefName(m_type));
		}

		bool volatileQualified() const {
			return clang_isVolatileQualifiedType(m_type);
		}

	private:
		::CXType m_type;
	};

	class CursorRef
	{
	public:

		using Visitor = function<::CXChildVisitResult(const CursorRef& cursor, const ::CXCursor& parent, const ::CXClientData& client_data)>;

		struct Location {

			Location(::CXSourceLocation loc) : m_loc(loc) {
				::CXFile file;
				clang_getSpellingLocation(m_loc, &file, &line, &column, &offset);
				fileName = to_string(clang_getFileName(file));
			}

			path fileName;
			unsigned line = 0;
			unsigned column = 0;
			unsigned offset = 0;

			bool isFromMainFile() const {
				return clang_Location_isFromMainFile(m_loc);
			}

		private:
			::CXSourceLocation m_loc;
		};

		CursorRef();
		CursorRef(nullptr_t);
		CursorRef(const CursorRef&) = default;
		CursorRef(CursorRef&&) = default;
		CursorRef(::CXCursor cursor);

		CursorRef& operator=(nullptr_t);
		CursorRef& operator=(const CursorRef&) = default;
		CursorRef& operator=(CursorRef&&) = default;
		CursorRef& operator=(::CXCursor cursor);

		operator bool() const;

		bool operator==(const CursorRef& other) const;
		bool operator==(const ::CXCursor& other) const;

		CursorRef canonical() const;

		string spelling() const;

		Location location() const;

		CursorRef definition() const;

		bool isDeclaration() const;

		bool isDefinition() const;

		::CXCursorKind kind() const;

		string briefCommentText() const;

		string rawCommentText() const;

		string mangled_name() const;

		TypeRef type() const;

		string tmp() const;

		void visitChildren(const Visitor& visitor) const;

		vector<CursorRef> listChildren(bool recursive = false) const;

		vector<CursorRef> argumnts() const;

		TypeRef resultType() const;

		CursorRef referenced() const {
			return clang_getCursorReferenced(m_cursor);
		}

		CursorRef semanticParent() const {
			return clang_getCursorSemanticParent(m_cursor);
		}

		CursorRef lexicalParent() const {
			return clang_getCursorLexicalParent(m_cursor);
		}

		bool isRoot() const {
			if (this->kind() == ::CXCursorKind::CXCursor_TranslationUnit) return true;
			if (!(bool)this->semanticParent()) return true;
			return false;
		}

		bool isTopLevel() const {
			return this->semanticParent().isRoot();
		}

		string displayName() const {
			return to_string(clang_getCursorDisplayName(m_cursor));
		}

		::CXCursor& handle() { return m_cursor; }
		const ::CXCursor& handle() const { return m_cursor; }

		bool isEnumDeclScoped() const {
			return clang_EnumDecl_isScoped(m_cursor);
		}

		TypeRef typedefUnderlyingType() const {
			return clang_getTypedefDeclUnderlyingType(m_cursor);
		}

	private:

		static bool are_equals(const ::CXCursor& left, const ::CXCursor& right);

	private:
		::CXCursor m_cursor;

		static Visitor m_visitor;
		static ::CXChildVisitResult m_cursorVisitor(::CXCursor cursor, ::CXCursor parent, ::CXClientData client_data);
	};
}