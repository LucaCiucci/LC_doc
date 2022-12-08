
// !!!
#include <iostream>
#include <cassert>

#include "clang_interface/TranslationUnit.hpp"

#include "cxx_parser.hpp"

namespace
{
	using std::make_shared;
	using std::make_unique;

	namespace gt
	{
		using namespace lcdoc;
		using namespace lcdoc::clang;

		void throwAssert(bool condition, const std::string& message = "")
		{
			if (!condition)
				throw std::runtime_error(message);
		}

		shared_ptr<CXXType> make_type(const clang::TypeRef& ctype, SymbolRegistry& registry);
		shared_ptr<CXXType> to_type(const clang::TypeRef& ctype, SymbolRegistry& registry);

		Location to_location(const CursorRef::Location& loc)
		{
			Location result;
			result.fileName = loc.fileName;
			result.line = loc.line;
			result.column = loc.column;
			result.offset = loc.offset;
			return result;
		}

		SymbolIdPart build_idPart(const CursorRef& cursor)
		{
			if (!cursor)
				return {};

			if (cursor.kind() == ::CXCursorKind::CXCursor_FunctionDecl)
				return { cursor.mangled_name(), cursor.spelling(), cursor.displayName() };
			const std::set<::CXCursorKind> plain_name = {
				::CXCursorKind::CXCursor_Namespace,
				::CXCursorKind::CXCursor_StructDecl,
				::CXCursorKind::CXCursor_ClassDecl,
				// TOOD templates
				// ...
			};
			if (plain_name.find(cursor.kind()) != plain_name.end())
				return { cursor.spelling(), cursor.spelling(), cursor.displayName() };

			std::cerr << to_string(cursor.kind()) << std::endl;
			//assert(("not handled", false));

			return {};
		}

		SymbolId build_id(const CursorRef& cursor)
		{
			if (!cursor)
				return {};

			const SymbolIdPart this_id = build_idPart(cursor);

			if (this_id.empty())
				return {};

			if (cursor.isTopLevel())
				return { this_id };

			return build_id(cursor.semanticParent()) + this_id;
		}

		shared_ptr<Symbol> find(const CursorRef& cursor, const SymbolRegistry& registry)
		{
			if (!cursor)
				return nullptr;

			auto id = build_id(cursor);

			auto sym = registry.findFromId(id);

			return sym;
		}

		shared_ptr<UnexposedDeclarationSymbol> handleUnexposedDecl(const CursorRef& cursor, SymbolRegistry& registry);
		shared_ptr<NamespaceSymbol> handleNamespaceDecl(const CursorRef& cursor, SymbolRegistry& registry);
		shared_ptr<FunctionSymbol> handleFunctionDecl(const CursorRef& cursor, SymbolRegistry& registry);
		shared_ptr<ClassSymbol> handleClassDecl(const CursorRef& cursor, SymbolRegistry& registry);
		shared_ptr<EnumSymbol> handleEnumDecl(const CursorRef& cursor, SymbolRegistry& registry);
		shared_ptr<TypedefSymbol> handleTypedefDecl(const CursorRef& cursor, SymbolRegistry& registry);

		shared_ptr<Symbol> record(const CursorRef& cursor, SymbolRegistry& registry)
		{
			auto sym = find(cursor, registry);
			if (sym)
				return sym;

			switch (cursor.kind())
			{
			case ::CXCursor_UnexposedDecl:
				sym = handleUnexposedDecl(cursor, registry);
				break;
			case ::CXCursorKind::CXCursor_FunctionDecl:
				sym = handleFunctionDecl(cursor, registry);
				break;
			case ::CXCursorKind::CXCursor_Namespace:
				sym = handleNamespaceDecl(cursor, registry);
				break;
			case ::CXCursorKind::CXCursor_ClassDecl:
				sym = handleClassDecl(cursor, registry);
				break;
			case ::CXCursorKind::CXCursor_EnumDecl:
				sym = handleEnumDecl(cursor, registry);
				break;
			case ::CXCursorKind::CXCursor_TypedefDecl:
				sym = handleTypedefDecl(cursor, registry);
				break;
			default:
				//assert(0);
				break;
			}

			//assert((bool)sym);
			if (sym)
			{
				const auto cursorLocation = cursor.location();
				if (cursorLocation.isFromMainFile())
					sym->exposed = true;

				// ? necessary?
				if (sym->exposed)
					sym->declarations.insert(to_location(cursor.definition().location()));

				if (cursor.isDeclaration())
					sym->declarations.insert(to_location(cursorLocation));

				if (cursor.isDefinition())
					sym->definitions.insert(to_location(cursorLocation));

				if (cursor.isDefinition() || cursor.isDeclaration())
				{
					const string brief = cursor.briefCommentText();
					const string raw = cursor.rawCommentText();

					if (!brief.empty())
						sym->docStr.brief = brief;

					if (!raw.empty() && raw.length() > sym->docStr.raw.length())
						sym->docStr.raw = raw;
				}

				registry.add(sym);
			}

			return sym;
		}

		void setParent(const shared_ptr<Symbol> symbol, const CursorRef& cursor, SymbolRegistry& registry)
		{
			if (cursor.isTopLevel())
				// nothing to do
				return;

			auto parent = record(cursor.semanticParent(), registry);
			throwAssert((bool)parent);
			symbol->parent = parent;
		}

		shared_ptr<UnexposedDeclarationSymbol> handleUnexposedDecl(const CursorRef& cursor, SymbolRegistry& registry)
		{
			if (cursor.kind() != ::CXCursorKind::CXCursor_UnexposedDecl)
				return nullptr;

			const auto part = build_idPart(cursor);
			auto UD = make_shared<UnexposedDeclarationSymbol>(part);
			setParent(UD, cursor, registry);

			UD->spelling = cursor.spelling();
			UD->displayName = cursor.displayName();

			return UD;
		}

		shared_ptr<NamespaceSymbol> handleNamespaceDecl(const CursorRef& cursor, SymbolRegistry& registry)
		{
			if (cursor.kind() != ::CXCursorKind::CXCursor_Namespace)
				return nullptr;

			const auto part = build_idPart(cursor);
			auto N = make_shared<NamespaceSymbol>(part);
			setParent(N, cursor, registry);

			N->spelling = cursor.spelling();
			N->displayName = cursor.displayName();

			return N;
		}

		shared_ptr<FunctionSymbol> handleFunctionDecl(const CursorRef& cursor, SymbolRegistry& registry)
		{
			if (cursor.kind() != ::CXCursorKind::CXCursor_FunctionDecl)
				return nullptr;

			const auto part = build_idPart(cursor);
			auto f = make_shared<FunctionSymbol>(part);
			setParent(f, cursor, registry);

			f->spelling = cursor.spelling();
			f->displayName = cursor.displayName();
			f->mangling = cursor.mangled_name();

			if (!f->signature)
			{
				f->signature = make_unique<FunctionSignature>();
				f->signature->ret = to_type(cursor.resultType(), registry);
				for (const auto& a : cursor.argumnts())
				{
					FuncArg arg;
					arg.name = a.spelling();
					arg.type = to_type(a.type(), registry);
					f->signature->args.push_back(std::move(arg));
				}
			}

			return f;
		}

		shared_ptr<ClassSymbol> handleClassDecl(const CursorRef& cursor, SymbolRegistry& registry)
		{
			if (cursor.kind() != ::CXCursorKind::CXCursor_ClassDecl)
				return nullptr;

			const auto part = build_idPart(cursor);
			auto c = make_shared<ClassSymbol>(part);
			setParent(c, cursor, registry);

			c->spelling = cursor.spelling();
			c->displayName = cursor.displayName();

			return c;
		}

		shared_ptr<EnumSymbol> handleEnumDecl(const CursorRef& cursor, SymbolRegistry& registry)
		{
			if (cursor.kind() != ::CXCursorKind::CXCursor_EnumDecl)
				return nullptr;

			const auto part = build_idPart(cursor);
			auto e = make_shared<EnumSymbol>(part);
			setParent(e, cursor, registry);

			e->spelling = cursor.spelling();
			e->displayName = cursor.displayName();

			e->scoped = cursor.isEnumDeclScoped();

			return e;
		}

		shared_ptr<TypedefSymbol> handleTypedefDecl(const CursorRef& cursor, SymbolRegistry& registry)
		{
			if (cursor.kind() != ::CXCursorKind::CXCursor_TypedefDecl)
				return nullptr;

			const auto part = build_idPart(cursor);
			auto tdef = make_shared<TypedefSymbol>(part);
			setParent(tdef, cursor, registry);

			tdef->spelling = cursor.spelling();
			tdef->displayName = cursor.displayName();

			tdef->underlying = to_type(cursor.typedefUnderlyingType(), registry);

			return tdef;
		}

		shared_ptr<CXXType> make_type(const clang::TypeRef& ctype, SymbolRegistry& registry)
		{
			shared_ptr<CXXType> result;
			switch (ctype.kind())
			{
			case ::CXTypeKind::CXType_Invalid:
				return nullptr;
			case ::CXTypeKind::CXType_Unexposed:
				return make_shared<UnexposedType>();
			case ::CXTypeKind::CXType_Elaborated:
			{
				auto elaborated = make_shared<ElaboratedType>();
				auto named = to_type(ctype.named(), registry);
				elaborated->named = named;
				result = elaborated;
				break;
			}
			case CXType_Record:
			{
				auto rec = make_shared<RecordType>();
				rec->recorded = record(ctype.declaration(), registry); // canonical?
				result = rec;
				break;
			}
			case ::CXTypeKind::CXType_Pointer:
			{
				auto p = make_shared<PointerType>();
				p->pointee = to_type(ctype.pointee(), registry);
				result = p;
				break;
			}
			case ::CXTypeKind::CXType_LValueReference:
			{
				auto r = make_shared<LValueReferenceType>();
				r->pointee = to_type(ctype.pointee(), registry);
				result = r;
				break;
			}
			case ::CXTypeKind::CXType_RValueReference:
			{
				auto r = make_shared<RValueReferenceType>();
				r->pointee = to_type(ctype.pointee(), registry);
				result = r;
				break;
			}
			case ::CXTypeKind::CXType_Enum:
			{
				auto e = make_shared<EnumType>();
				e->enumSymbol = std::dynamic_pointer_cast<EnumSymbol>(record(ctype.declaration(), registry));
				result = e;
				break;
			}
			case ::CXTypeKind::CXType_Typedef:
			{
				auto tdef = make_shared<TypedefType>();
				tdef->symbol = std::dynamic_pointer_cast<TypedefSymbol>(record(ctype.declaration(), registry));
				tdef->typedefName = ctype.typedefName();
				result = tdef;
				break;
			}

#define lc_tmp_basic_case(cx_type) case cx_type: result = make_shared<basic_types::get_basic_type<cx_type>::type>(); break;

			// BEGIN Basic Types
#if true
			lc_tmp_basic_case(::CXTypeKind::CXType_Void);
			lc_tmp_basic_case(::CXTypeKind::CXType_Bool);
			lc_tmp_basic_case(::CXTypeKind::CXType_Char_U);
			lc_tmp_basic_case(::CXTypeKind::CXType_UChar);
			lc_tmp_basic_case(::CXTypeKind::CXType_Char16);
			lc_tmp_basic_case(::CXTypeKind::CXType_Char32);
			lc_tmp_basic_case(::CXTypeKind::CXType_UShort);
			lc_tmp_basic_case(::CXTypeKind::CXType_UInt);
			lc_tmp_basic_case(::CXTypeKind::CXType_ULong);
			lc_tmp_basic_case(::CXTypeKind::CXType_ULongLong);
			lc_tmp_basic_case(::CXTypeKind::CXType_UInt128);
			lc_tmp_basic_case(::CXTypeKind::CXType_Char_S);
			lc_tmp_basic_case(::CXTypeKind::CXType_SChar);
			lc_tmp_basic_case(::CXTypeKind::CXType_WChar);
			lc_tmp_basic_case(::CXTypeKind::CXType_Short);
			lc_tmp_basic_case(::CXTypeKind::CXType_Int);
			lc_tmp_basic_case(::CXTypeKind::CXType_Long);
			lc_tmp_basic_case(::CXTypeKind::CXType_LongLong);
			lc_tmp_basic_case(::CXTypeKind::CXType_Int128);
			lc_tmp_basic_case(::CXTypeKind::CXType_Float);
			lc_tmp_basic_case(::CXTypeKind::CXType_Double);
			lc_tmp_basic_case(::CXTypeKind::CXType_LongDouble);
			lc_tmp_basic_case(::CXTypeKind::CXType_NullPtr);
			lc_tmp_basic_case(::CXTypeKind::CXType_Overload);
			lc_tmp_basic_case(::CXTypeKind::CXType_Dependent);
			lc_tmp_basic_case(::CXTypeKind::CXType_ObjCId);
			lc_tmp_basic_case(::CXTypeKind::CXType_ObjCClass);
			lc_tmp_basic_case(::CXTypeKind::CXType_ObjCSel);
			lc_tmp_basic_case(::CXTypeKind::CXType_Float128);
			lc_tmp_basic_case(::CXTypeKind::CXType_Half);
			lc_tmp_basic_case(::CXTypeKind::CXType_Float16);
			lc_tmp_basic_case(::CXTypeKind::CXType_ShortAccum);
			lc_tmp_basic_case(::CXTypeKind::CXType_Accum);
			lc_tmp_basic_case(::CXTypeKind::CXType_LongAccum);
			lc_tmp_basic_case(::CXTypeKind::CXType_UShortAccum);
			lc_tmp_basic_case(::CXTypeKind::CXType_UAccum);
			lc_tmp_basic_case(::CXTypeKind::CXType_ULongAccum);
			lc_tmp_basic_case(::CXTypeKind::CXType_BFloat16);
			lc_tmp_basic_case(::CXTypeKind::CXType_Ibm128);
			//lc_tmp_basic_case(::CXTypeKind::CXType_FirstBuiltin);
			//lc_tmp_basic_case(::CXTypeKind::CXType_LastBuiltin);
			lc_tmp_basic_case(::CXTypeKind::CXType_Complex);
			//lc_tmp_basic_case(::CXTypeKind::CXType_Pointer);
			lc_tmp_basic_case(::CXTypeKind::CXType_BlockPointer);
			//lc_tmp_basic_case(::CXTypeKind::CXType_LValueReference);
			//lc_tmp_basic_case(::CXTypeKind::CXType_RValueReference);
			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11 lc_tmp_basic_case(::CXTypeKind::CXType_Record);
			//lc_tmp_basic_case(::CXTypeKind::CXType_Enum);
			//lc_tmp_basic_case(::CXTypeKind::CXType_Typedef);
			lc_tmp_basic_case(::CXTypeKind::CXType_ObjCInterface);
			lc_tmp_basic_case(::CXTypeKind::CXType_ObjCObjectPointer);
			lc_tmp_basic_case(::CXTypeKind::CXType_FunctionNoProto);
			lc_tmp_basic_case(::CXTypeKind::CXType_FunctionProto);
			lc_tmp_basic_case(::CXTypeKind::CXType_ConstantArray);
			lc_tmp_basic_case(::CXTypeKind::CXType_Vector);
			lc_tmp_basic_case(::CXTypeKind::CXType_IncompleteArray);
			lc_tmp_basic_case(::CXTypeKind::CXType_VariableArray);
			lc_tmp_basic_case(::CXTypeKind::CXType_DependentSizedArray);
			lc_tmp_basic_case(::CXTypeKind::CXType_MemberPointer);
			lc_tmp_basic_case(::CXTypeKind::CXType_Auto);
			//lc_tmp_basic_case(::CXTypeKind::CXType_Elaborated);
			lc_tmp_basic_case(::CXTypeKind::CXType_Pipe);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage1dRO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage1dArrayRO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage1dBufferRO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dRO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dArrayRO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dDepthRO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dArrayDepthRO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dMSAARO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dArrayMSAARO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dMSAADepthRO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dArrayMSAADepthRO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage3dRO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage1dWO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage1dArrayWO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage1dBufferWO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dWO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dArrayWO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dDepthWO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dArrayDepthWO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dMSAAWO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dArrayMSAAWO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dMSAADepthWO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dArrayMSAADepthWO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage3dWO);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage1dRW);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage1dArrayRW);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage1dBufferRW);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dRW);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dArrayRW);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dDepthRW);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dArrayDepthRW);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dMSAARW);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dArrayMSAARW);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dMSAADepthRW);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage2dArrayMSAADepthRW);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLImage3dRW);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLSampler);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLEvent);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLQueue);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLReserveID);
			lc_tmp_basic_case(::CXTypeKind::CXType_ObjCObject);
			lc_tmp_basic_case(::CXTypeKind::CXType_ObjCTypeParam);
			lc_tmp_basic_case(::CXTypeKind::CXType_Attributed);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLIntelSubgroupAVCMcePayload);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLIntelSubgroupAVCImePayload);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLIntelSubgroupAVCRefPayload);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLIntelSubgroupAVCSicPayload);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLIntelSubgroupAVCMceResult);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLIntelSubgroupAVCImeResult);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLIntelSubgroupAVCRefResult);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLIntelSubgroupAVCSicResult);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLIntelSubgroupAVCImeResultSingleRefStreamout);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLIntelSubgroupAVCImeResultDualRefStreamout);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLIntelSubgroupAVCImeSingleRefStreamin);
			lc_tmp_basic_case(::CXTypeKind::CXType_OCLIntelSubgroupAVCImeDualRefStreamin);
			lc_tmp_basic_case(::CXTypeKind::CXType_ExtVector);
			lc_tmp_basic_case(::CXTypeKind::CXType_Atomic);
#endif
			// END basic Types



			default:
				assert(0);
				break;
			}

			if (result)
			{
				result->constQualified = ctype.constQualified();
				result->volatileQualified = ctype.volatileQualified();
			}

			return result;
		}

		shared_ptr<CXXType> to_type(const clang::TypeRef& ctype, SymbolRegistry& registry)
		{
			// TODO reuse already created types
			return make_type(ctype, registry);
		}
	}
}

namespace lcdoc
{
	void CXXDocumentParser::parse(const path& fileName, const vector<string>& args)
	{
		clang::TranslationUnit TU(
			m_index,
			fileName,
			args
			//{ "-DPIPPO_", "-std=c++20", "-IC:/Program Files/LLVM/include" }
		);

		for (const auto& cursor : TU.cursor().listChildren())
		{
			if (!cursor.location().isFromMainFile())
				continue;

			if (cursor.isDeclaration() || cursor.isDefinition())
				gt::record(cursor, this->registry);
		}
	}
}