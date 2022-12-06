#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <map>
#include <memory>
#include <ranges>
#include <concepts>
#include <set>
#include <list>

// !!!
#include "string_utils.hpp"
#include "clang-c/Index.h"

namespace lcdoc
{
	using std::string;
	using std::vector;
	using std::list;
	using std::map;
	using std::set;
	using std::unique_ptr;
	using std::shared_ptr;
	using std::weak_ptr;
	using std::filesystem::path;

	class SymbolRegistry;
	class Symbol;
	class CXXSymbol;
	class EnumSymbol;
	class TypedefSymbol;

	class DocumentationString
	{
	public:
		string brief;
		string raw;
	};

	struct Location
	{
		path fileName;
		int line = 0;
		int column = 0;
		int offset = 0;

		bool operator==(const Location&) const = default;

		operator bool() const { return *this == Location(); }
	};

	class Type // abstract
	{
	public:

		virtual string spelling() const = 0;

		virtual ~Type() = default;

	private:
	};

	class KeywordType : public virtual Type {};

	class CXXType : public virtual Type
	{
	public:

		bool constQualified = false;
		bool volatileQualified = false;
	};

	class UnexposedType final: public CXXType
	{
	public:

		string spelling() const override {
			return "<unexposed>";
		}

	private:
	};

	class TypedefType final : public CXXType
	{
	public:

		string spelling() const override {
			return "<typedef-type>";
		}

		std::shared_ptr<TypedefSymbol> symbol;
		string typedefName;

	private:
	};

	class ElaboratedType final : public CXXType
	{
	public:

		string spelling() const override {
			// TODO
			return "<elaborated>";
		}

		std::shared_ptr<CXXType> named;

		// TODO maybe see clang_getCursorReferenceNameRange ?
	private:
	};

	class RecordType final : public CXXType
	{
	public:

		string spelling() const override {
			// TODO
			return "<record>";
		}

		std::shared_ptr<Symbol> recorded;

		// TODO maybe see clang_getCursorReferenceNameRange ?
	private:
	};

	class EnumType final : public CXXType
	{
	public:
		string spelling() const override {
			return "<enum>";
		}

		shared_ptr<EnumSymbol> enumSymbol;
	};

	class PointerLikeType : public CXXType
	{
	public:

		std::shared_ptr<CXXType> pointee;
	};
	class PointerType         final : public PointerLikeType { public: string spelling() const override { return         "<pointer>"; } };
	class LValueReferenceType final : public PointerLikeType { public: string spelling() const override { return "<LValueReference>"; } };
	class RValueReferenceType final : public PointerLikeType { public: string spelling() const override { return "<RValueReference>"; } };

	class BasicCXXType : public CXXType
	{
	public:

	private:
	};

	namespace basic_types
	{
		template <::CXTypeKind _kind> struct get_basic_type;

#define lc_tmp_declare_basic(kind, name, str) class name final : public BasicCXXType { public: string spelling() const override { return str; } }; template <> struct get_basic_type<kind> { using type = name; };
#define lc_tmp_declare_basic_keyword(kind, name, str) class name final : public BasicCXXType, public KeywordType { public: string spelling() const override { return str; } }; template <> struct get_basic_type<kind> { using type = name; };
#define lc_tmp_decl_unhandled(cx_type) lc_tmp_declare_basic(cx_type,cx_type##_Type,#cx_type)

		// CXType_Invalid   -> UnexposedType
		// CXType_Unexposed -> BasicCXXType

		lc_tmp_declare_basic_keyword(      CXType_Void,       VoidType,               "void");
		lc_tmp_declare_basic_keyword(      CXType_Bool,       BoolType,               "bool");
		lc_tmp_declare_basic(    CXType_Char_U,      CharUType,      "CXType_Char_U");
		lc_tmp_declare_basic(     CXType_UChar,      UCharType,       "CXType_UChar");
		lc_tmp_declare_basic(    CXType_Char16,     Char16Type,      "CXType_Char16");
		lc_tmp_declare_basic(    CXType_Char32,     Char32Type,      "CXType_Char32");
		lc_tmp_declare_basic(    CXType_UShort,     UShortType,     "unsigned short");
		lc_tmp_declare_basic_keyword(      CXType_UInt,       UIntType,           "unsigned");
		lc_tmp_declare_basic_keyword(     CXType_ULong,      ULongType,      "unsigned long");
		lc_tmp_declare_basic_keyword( CXType_ULongLong,  ULongLongType, "unsigned long long");
		lc_tmp_declare_basic(   CXType_UInt128,    UInt128Type,     "CXType_UInt128");
		lc_tmp_declare_basic_keyword(    CXType_Char_S,      CharSType,               "char");
		lc_tmp_declare_basic(     CXType_SChar,      SCharType,       "CXType_SChar");
		lc_tmp_declare_basic(     CXType_WChar,      WCharType,       "CXType_WChar");
		lc_tmp_declare_basic_keyword(     CXType_Short,      ShortType,              "short");
		lc_tmp_declare_basic_keyword(       CXType_Int,        IntType,                "int");
		lc_tmp_declare_basic_keyword(      CXType_Long,       LongType,               "long");
		lc_tmp_declare_basic_keyword(  CXType_LongLong,   LongLongType,          "long long");
		lc_tmp_declare_basic(    CXType_Int128,     Int128Type,      "CXType_Int128");
		lc_tmp_declare_basic_keyword(     CXType_Float,      FloatType,              "float");
		lc_tmp_declare_basic_keyword(    CXType_Double,     DoubleType,             "double");
		lc_tmp_declare_basic_keyword(CXType_LongDouble, LongDoubleType,        "long double");
		lc_tmp_declare_basic_keyword(   CXType_NullPtr,    NullPtrType,       "nullptr_t???");
		lc_tmp_decl_unhandled(CXType_Overload);
		lc_tmp_decl_unhandled(CXType_Dependent);
		lc_tmp_decl_unhandled(CXType_ObjCId);
		lc_tmp_decl_unhandled(CXType_ObjCClass);
		lc_tmp_decl_unhandled(CXType_ObjCSel);
		lc_tmp_decl_unhandled(CXType_Float128);
		lc_tmp_decl_unhandled(CXType_Half);
		lc_tmp_decl_unhandled(CXType_Float16);
		lc_tmp_decl_unhandled(CXType_ShortAccum);
		lc_tmp_decl_unhandled(CXType_Accum);
		lc_tmp_decl_unhandled(CXType_LongAccum);
		lc_tmp_decl_unhandled(CXType_UShortAccum);
		lc_tmp_decl_unhandled(CXType_UAccum);
		lc_tmp_decl_unhandled(CXType_ULongAccum);
		lc_tmp_decl_unhandled(CXType_BFloat16);
		lc_tmp_decl_unhandled(CXType_Ibm128);
		//lc_tmp_decl_unhandled(CXType_FirstBuiltin);
		//lc_tmp_decl_unhandled(CXType_LastBuiltin);
		lc_tmp_decl_unhandled(CXType_Complex);
		//lc_tmp_decl_unhandled(CXType_Pointer);
		lc_tmp_decl_unhandled(CXType_BlockPointer);
		//lc_tmp_decl_unhandled(CXType_LValueReference);
		//lc_tmp_decl_unhandled(CXType_RValueReference);
		//lc_tmp_decl_unhandled(CXType_Record);
		//lc_tmp_decl_unhandled(CXType_Enum);
		//lc_tmp_decl_unhandled(CXType_Typedef);
		lc_tmp_decl_unhandled(CXType_ObjCInterface);
		lc_tmp_decl_unhandled(CXType_ObjCObjectPointer);
		lc_tmp_decl_unhandled(CXType_FunctionNoProto);
		lc_tmp_decl_unhandled(CXType_FunctionProto);
		lc_tmp_decl_unhandled(CXType_ConstantArray);
		lc_tmp_decl_unhandled(CXType_Vector);
		lc_tmp_decl_unhandled(CXType_IncompleteArray);
		lc_tmp_decl_unhandled(CXType_VariableArray);
		lc_tmp_decl_unhandled(CXType_DependentSizedArray);
		lc_tmp_decl_unhandled(CXType_MemberPointer);
		lc_tmp_decl_unhandled(CXType_Auto);
		//lc_tmp_decl_unhandled(CXType_Elaborated);
		lc_tmp_decl_unhandled(CXType_Pipe);
		lc_tmp_decl_unhandled(CXType_OCLImage1dRO);
		lc_tmp_decl_unhandled(CXType_OCLImage1dArrayRO);
		lc_tmp_decl_unhandled(CXType_OCLImage1dBufferRO);
		lc_tmp_decl_unhandled(CXType_OCLImage2dRO);
		lc_tmp_decl_unhandled(CXType_OCLImage2dArrayRO);
		lc_tmp_decl_unhandled(CXType_OCLImage2dDepthRO);
		lc_tmp_decl_unhandled(CXType_OCLImage2dArrayDepthRO);
		lc_tmp_decl_unhandled(CXType_OCLImage2dMSAARO);
		lc_tmp_decl_unhandled(CXType_OCLImage2dArrayMSAARO);
		lc_tmp_decl_unhandled(CXType_OCLImage2dMSAADepthRO);
		lc_tmp_decl_unhandled(CXType_OCLImage2dArrayMSAADepthRO);
		lc_tmp_decl_unhandled(CXType_OCLImage3dRO);
		lc_tmp_decl_unhandled(CXType_OCLImage1dWO);
		lc_tmp_decl_unhandled(CXType_OCLImage1dArrayWO);
		lc_tmp_decl_unhandled(CXType_OCLImage1dBufferWO);
		lc_tmp_decl_unhandled(CXType_OCLImage2dWO);
		lc_tmp_decl_unhandled(CXType_OCLImage2dArrayWO);
		lc_tmp_decl_unhandled(CXType_OCLImage2dDepthWO);
		lc_tmp_decl_unhandled(CXType_OCLImage2dArrayDepthWO);
		lc_tmp_decl_unhandled(CXType_OCLImage2dMSAAWO);
		lc_tmp_decl_unhandled(CXType_OCLImage2dArrayMSAAWO);
		lc_tmp_decl_unhandled(CXType_OCLImage2dMSAADepthWO);
		lc_tmp_decl_unhandled(CXType_OCLImage2dArrayMSAADepthWO);
		lc_tmp_decl_unhandled(CXType_OCLImage3dWO);
		lc_tmp_decl_unhandled(CXType_OCLImage1dRW);
		lc_tmp_decl_unhandled(CXType_OCLImage1dArrayRW);
		lc_tmp_decl_unhandled(CXType_OCLImage1dBufferRW);
		lc_tmp_decl_unhandled(CXType_OCLImage2dRW);
		lc_tmp_decl_unhandled(CXType_OCLImage2dArrayRW);
		lc_tmp_decl_unhandled(CXType_OCLImage2dDepthRW);
		lc_tmp_decl_unhandled(CXType_OCLImage2dArrayDepthRW);
		lc_tmp_decl_unhandled(CXType_OCLImage2dMSAARW);
		lc_tmp_decl_unhandled(CXType_OCLImage2dArrayMSAARW);
		lc_tmp_decl_unhandled(CXType_OCLImage2dMSAADepthRW);
		lc_tmp_decl_unhandled(CXType_OCLImage2dArrayMSAADepthRW);
		lc_tmp_decl_unhandled(CXType_OCLImage3dRW);
		lc_tmp_decl_unhandled(CXType_OCLSampler);
		lc_tmp_decl_unhandled(CXType_OCLEvent);
		lc_tmp_decl_unhandled(CXType_OCLQueue);
		lc_tmp_decl_unhandled(CXType_OCLReserveID);
		lc_tmp_decl_unhandled(CXType_ObjCObject);
		lc_tmp_decl_unhandled(CXType_ObjCTypeParam);
		lc_tmp_decl_unhandled(CXType_Attributed);
		lc_tmp_decl_unhandled(CXType_OCLIntelSubgroupAVCMcePayload);
		lc_tmp_decl_unhandled(CXType_OCLIntelSubgroupAVCImePayload);
		lc_tmp_decl_unhandled(CXType_OCLIntelSubgroupAVCRefPayload);
		lc_tmp_decl_unhandled(CXType_OCLIntelSubgroupAVCSicPayload);
		lc_tmp_decl_unhandled(CXType_OCLIntelSubgroupAVCMceResult);
		lc_tmp_decl_unhandled(CXType_OCLIntelSubgroupAVCImeResult);
		lc_tmp_decl_unhandled(CXType_OCLIntelSubgroupAVCRefResult);
		lc_tmp_decl_unhandled(CXType_OCLIntelSubgroupAVCSicResult);
		lc_tmp_decl_unhandled(CXType_OCLIntelSubgroupAVCImeResultSingleRefStreamout);
		lc_tmp_decl_unhandled(CXType_OCLIntelSubgroupAVCImeResultDualRefStreamout);
		lc_tmp_decl_unhandled(CXType_OCLIntelSubgroupAVCImeSingleRefStreamin);
		lc_tmp_decl_unhandled(CXType_OCLIntelSubgroupAVCImeDualRefStreamin);
		lc_tmp_decl_unhandled(CXType_ExtVector);
		lc_tmp_decl_unhandled(CXType_Atomic);

#undef tmp_declare_basic
#undef tmp_decl_unhandled
	}

	struct SymbolIdPart : string
	{
		using string::string;

		SymbolIdPart(const string& id, const string& spelling, const string& display) :
			string(id),
			spelling(spelling),
			display(display)
		{
		}

		string spelling;
		string display;

		constexpr std::strong_ordering operator<=>(const SymbolIdPart& other) const {
			if (this->display != other.display)
				return this->display <=> other.display;
			if (this->spelling != other.spelling)
				return this->spelling <=> other.spelling;
			return ((const string&)*this) <=> ((const string&)other);
		}
	};

	struct SymbolId : vector<SymbolIdPart>
	{
		using vector<SymbolIdPart>::vector;

		constexpr SymbolId(const SymbolId&) = default;

		// TODO non funziona bene
		constexpr std::strong_ordering operator<=>(const SymbolId& other) const {
			auto it = this->begin();
			auto otherIt = other.begin();
			for (; it != this->end() && otherIt != other.end(); ++it, ++otherIt)
				if (*it != *otherIt)
					return *it <=> *otherIt;
			if (it != this->end())
				return std::strong_ordering::greater;
			if (otherIt != other.end())
				return std::strong_ordering::less;
			return std::strong_ordering::equal;
		}

		constexpr string to_string() const {
			vector<string> ss;
			for (const auto& part : *this)
				ss.push_back(part);
			return join(ss, "::");
		}

		constexpr string spelling() const {
			vector<string> ss;
			for (const auto& part : *this)
				ss.push_back(part.spelling);
			return join(ss, "::");
		}

		constexpr string to_display_string() const {
			vector<string> ss;
			for (const auto& part : *this)
				ss.push_back(part.display);
			return join(ss, "::");
		}

		constexpr operator bool() const {
			return !this->empty();
		}

		constexpr SymbolId operator+(const SymbolId& other) const {
			SymbolId result = *this;
			for (const auto& piece : other)
				result.push_back(piece);
			return result;
		}

		constexpr SymbolId operator+(const SymbolIdPart& piece) const {
			return *this + SymbolId({ piece });
		}
	};

	class Symbol
	{
	public:

		Symbol(const SymbolIdPart& idPart) : m_idPart(idPart) {}

		virtual ~Symbol() = default;

		virtual string kindSpelling() const = 0;

		DocumentationString docStr;

		string spelling;
		string displayName;
		//string canonicalSpelling;

		set<Location> declarations;
		set<Location> definitions;

		bool exposed = false;

		const SymbolIdPart& idPart() const {
			return m_idPart;
		}

		SymbolId id() const {
			SymbolId id;
			if (auto parent = this->parent.lock())
				id = parent->id();
			id.push_back(this->idPart());
			return id;
		}

		weak_ptr<Symbol> parent;

	private:
		const SymbolIdPart m_idPart;
	};

	class CXXSymbol : public Symbol
	{
	public:

		using Symbol::Symbol;

	private:
	};

	// ????????????
	class UnexposedDeclarationSymbol : public CXXSymbol
	{
	public:
		using CXXSymbol::CXXSymbol;

		string kindSpelling() const override { return "<-unexposed->"; };
	};

	class TypedefSymbol : public CXXSymbol
	{
	public:
		using CXXSymbol::CXXSymbol;

		string kindSpelling() const override { return "<typedef-symbol>"; };

		shared_ptr<CXXType> underlying;
	};

	struct NamespaceSymbol : public CXXSymbol
	{
	public:

		using CXXSymbol::CXXSymbol;

		string kindSpelling() const override { return "<namespace>"; };

	private:

	};

	class EnumSymbol : public CXXSymbol
	{
	public:

		using CXXSymbol::CXXSymbol;

		string kindSpelling() const override { return "<enum-symbol>"; };

		bool scoped = false;

	private:

	};

	struct FuncArg
	{
		shared_ptr<Type> type;
		string name;
	};

	struct FunctionSignature
	{
		shared_ptr<Type> ret;
		vector<FuncArg> args;
	};

	class FunctionSymbol : public CXXSymbol
	{
	public:

		using CXXSymbol::CXXSymbol;

		string kindSpelling() const override { return "<function>"; };

		//string name;
		string mangling;
		unique_ptr<FunctionSignature> signature;

	private:
	};

	class StructLikeSymbol : public CXXSymbol
	{
	public:
		using CXXSymbol::CXXSymbol;

		string kindSpelling() const override { return "<struct-like>"; };
	};

	class StructSymbol : public StructLikeSymbol
	{
	public:

		using StructLikeSymbol::StructLikeSymbol;

		string kindSpelling() const override { return "<struct>"; };

	private:
	};

	class ClassSymbol : public StructLikeSymbol
	{
	public:

		using StructLikeSymbol::StructLikeSymbol;

		string kindSpelling() const override { return "<class>"; };

	private:
	};

	class SymbolRegistry
	{
	public:

		struct UnhandledDecl {
			string name;
			Location location;
		};

		map<SymbolId, shared_ptr<Symbol>> symbolsById;
		list<UnhandledDecl> unhandledDecls;

		void add(const shared_ptr<Symbol>& symbol) {
			if (symbol)
				this->symbolsById[symbol->id()] = symbol;
		}

		shared_ptr<Symbol> findFromId(const SymbolId& id) const {
			auto it = this->symbolsById.find(id);
			if (it != this->symbolsById.end())
				return it->second;
			return nullptr;
		}

	private:

	};
}