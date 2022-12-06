
#include <fstream>
#include <cassert>
#include <set>

#include "html_page.hpp"

#include "list_page.hpp"

namespace lcdoc
{
	using std::set;

	class CxxDocHtmlArticle : public LCHtmlArticle
	{
	public:

		enum class Keyword {
			ConstQualifier,
			Int,
			VoidType,
			Double,
			CharS,
			UnscopedEnum,
			VolatileQualifier,
		};

		enum class Punctuation {
			StarPointer,
			LValueReference,
			RValueReference,
		};

		set<Keyword> usedKeywords;
		set<Punctuation> usedPunctuation;

		CxxDocHtmlArticle();

		static string tooltip(const string& content, const string& tooltip);

		static string tooltipUse(const string& content, const string& use);

		static string useRef(const string& ref);

		static string keywordTooltipId(const Keyword& keyword);

		virtual string tooltipContentFor(const Keyword& keyword);

		string keywordHtml(const Keyword& keyword, const string& spelling);

		string qualifiers_html(const shared_ptr<CXXType>& type, const string& beginIfNotEmpty, const string& endIfNotEmpty);

		string basic_type_to_html(const shared_ptr<BasicCXXType>& basicType);

		string to_html(const shared_ptr<const CXXSymbol>& symbol);

		string to_html(const shared_ptr<CXXType>& type);

		virtual void finish();

	protected:

	private:

	private:

	};

	CxxDocHtmlArticle::CxxDocHtmlArticle()
	{
		this->setupScripts();
		this->setupStyles();
	}

	string CxxDocHtmlArticle::tooltip(const string& content, const string& tooltip)
	{
		const string tooltipPopup = html::HtmlElement("tooltip-popup", tooltip).outherHtml();
		return html::HtmlElement("tool-tip", content + tooltipPopup).outherHtml();
	}

	string CxxDocHtmlArticle::tooltipUse(const string& content, const string& use)
	{
		return html::HtmlElement("tool-tip", content, {{ "use", use }}).outherHtml();
	}

	string CxxDocHtmlArticle::useRef(const string& ref)
	{
		return html::HtmlElement("lc-use", "", {{"ref", ref}}).outherHtml();
	}

	string CxxDocHtmlArticle::keywordTooltipId(const Keyword& keyword)
	{
		switch (keyword)
		{
		case Keyword::ConstQualifier:
			return "const-qualifier-tooltip";
		case Keyword::Int:
			return "int-tooltip";
		case Keyword::VoidType:
			return "void-type-tooltip";
		case Keyword::Double:
			return "double-tooltip";
		case Keyword::CharS:
			return "char-tooltip";
		case Keyword::UnscopedEnum:
			return "unscoped-enum-tooltip";
		case Keyword::VolatileQualifier:
			return "volatile-qualifier-tooltip";
		default:
			assert(0);
			break;
		}

		return "";
	}

	string CxxDocHtmlArticle::tooltipContentFor(const Keyword& keyword)
	{
		switch (keyword)
		{
		case Keyword::ConstQualifier:
			return std::format(R"wqfnewf(
Tells that a type is constant
<ul>
	<li><a href="https://en.cppreference.com/w/cpp/language/cv">reference</a></li>
	<li><a href="https://en.cppreference.com/w/cpp/keyword/const">reference (keyword)</a></li>
	<li><a href="https://learn.microsoft.com/it-it/cpp/cpp/const-cpp?view=msvc-170">reference</a></li>
</ul>
example:
<pre><code>{0} {1} <code-var>n</code-var> = <code-number>42</code-number>;
<code-var>n</code-var> = <code-number>3</code-number>; <code-comment>// &lt;- ERROR</code-comment></pre></code>
)wqfnewf", this->keywordHtml(Keyword::ConstQualifier, "const"), this->keywordHtml(Keyword::Int, "int"));
		case Keyword::Int:
			return std::format(R"afewgrw(Integer type, see <a href="https://en.cppreference.com/w/cpp/language/types">foundamental types</a>)afewgrw");
		case Keyword::VoidType:
			return std::format(R"afewgrw(Void type, see <a href="https://en.cppreference.com/w/cpp/language/types#Void_type">foundamental types</a>)afewgrw");
		case Keyword::Double:
			return std::format(R"afewgrw(Double precision floating point type, see <a href="https://en.cppreference.com/w/cpp/language/types#Floating-point_types">foundamental types</a>)afewgrw");
		case Keyword::CharS:
			return std::format(R"afewgrw(Signed char type, see <a href="https://en.cppreference.com/w/cpp/language/types#Signed_and_unsigned_integer_types">foundamental types</a>)afewgrw");
		case Keyword::UnscopedEnum:
			return std::format(R"afewgrw(Unscoped enum, see <a href="https://en.cppreference.com/w/cpp/language/enum#Unscoped_enumerations">unscoped enumeration</a>)afewgrw");
		case Keyword::VolatileQualifier:
			return std::format(R"afewgrw(Volatile qualifier, See <a href="https://en.cppreference.com/w/cpp/language/cv">type qualifiers</a>)afewgrw");
		default:
			assert(0);
			break;
		}

		return "";
	}

	string CxxDocHtmlArticle::keywordHtml(const Keyword& keyword, const string& spelling)
	{
		this->usedKeywords.insert(keyword);
		const string ttid = this->keywordTooltipId(keyword);

		const string ky = html::HtmlElement("code-keyword", spelling).outherHtml();

		if (!ttid.empty())
			return tooltipUse(ky, ttid);

		return ky;
	}

	string CxxDocHtmlArticle::qualifiers_html(const shared_ptr<CXXType>& type, const string& beginIfNotEmpty, const string& endIfNotEmpty)
	{
		if (!type)
			return "";

		vector<string> qualifiers;

		if (type->volatileQualified)
			qualifiers.push_back(this->keywordHtml(Keyword::VolatileQualifier, "volatile"));

		if (type->constQualified)
			qualifiers.push_back(this->keywordHtml(Keyword::ConstQualifier, "const"));

		if (qualifiers.empty())
			return "";
		return beginIfNotEmpty + join(qualifiers, " ") + endIfNotEmpty;
	}

	string CxxDocHtmlArticle::basic_type_to_html(const shared_ptr<BasicCXXType>& basicType)
	{
		const string spelling = escapeHtml(basicType->spelling());

		if (std::dynamic_pointer_cast<basic_types::IntType>(basicType))
			return this->keywordHtml(Keyword::Int, spelling);
		if (std::dynamic_pointer_cast<basic_types::VoidType>(basicType))
			return this->keywordHtml(Keyword::VoidType, spelling);
		if (std::dynamic_pointer_cast<basic_types::DoubleType>(basicType))
			return this->keywordHtml(Keyword::Double, spelling);
		if (std::dynamic_pointer_cast<basic_types::CharSType>(basicType))
			return this->keywordHtml(Keyword::CharS, spelling);

		return std::format(R"(<span class="red">{}</span>)", spelling);
	}

	string CxxDocHtmlArticle::to_html(const shared_ptr<const CXXSymbol>& symbol)
	{
		if (!symbol)
			return "";

		// TODO link and tooltip if possible
		const string parent = [&symbol, this]() -> string {
			if (auto p = symbol->parent.lock())
				return this->to_html(std::dynamic_pointer_cast<const CXXSymbol>(p));
			return "";
		}();

		if (const auto f = std::dynamic_pointer_cast<const FunctionSymbol>(symbol))
		{
			const string functionHtml = html::HtmlElement("code-function", f->spelling).outherHtml();

			if (!parent.empty())
				return parent + "::" + functionHtml;
			return functionHtml;
		}

		if (const auto ns = std::dynamic_pointer_cast<const NamespaceSymbol>(symbol))
		{
			const string namespaceHtml = html::HtmlElement("code-namespace", ns->spelling).outherHtml();

			if (!parent.empty())
				return parent + "::" + namespaceHtml;
			return namespaceHtml;
		}

		if (const auto cl = std::dynamic_pointer_cast<const ClassSymbol>(symbol))
		{
			const string classHtml = html::HtmlElement("code-class", cl->spelling).outherHtml();

			if (!parent.empty())
				return parent + "::" + classHtml;
			return classHtml;
		}

		if (const auto e = std::dynamic_pointer_cast<const EnumSymbol>(symbol))
		{
			const string enumHtml = html::HtmlElement("code-enum", e->spelling).outherHtml();

			if (!parent.empty())
				return parent + "::" + enumHtml;
			return enumHtml;
		}

		if (const auto tdef = std::dynamic_pointer_cast<const TypedefSymbol>(symbol))
		{
			const string enumHtml = html::HtmlElement("code-typedef", tdef->spelling).outherHtml();

			if (!parent.empty())
				return parent + "::" + enumHtml;
			return enumHtml;
		}

		return std::format(R"(<span style="color:red;">unhandled_CXX_symbol ({0})</span>)", escapeHtml(symbol->kindSpelling()));
	}

	string CxxDocHtmlArticle::to_html(const shared_ptr<CXXType>& type)
	{
		if (!type)
			return "";

		if (auto basicType = std::dynamic_pointer_cast<BasicCXXType>(type))
			return this->qualifiers_html(type, "", " ") + this->basic_type_to_html(basicType);

		if (auto elaborated = std::dynamic_pointer_cast<ElaboratedType>(type))
		{
			if (elaborated->named)
				return this->qualifiers_html(type, "", " ") + this->to_html(elaborated->named);
			return this->qualifiers_html(type, "", " ") + std::format(R"(<span style="color:red;">Elaborated (error)</span>)");
		}

		if (auto record = std::dynamic_pointer_cast<RecordType>(type))
		{
			string s = this->to_html(std::dynamic_pointer_cast<CXXSymbol>(record->recorded));
			if (s.empty())
				return this->qualifiers_html(type, "", " ") + std::format(R"(<span style="color:red;">Record (error)</span>)");
			return this->qualifiers_html(type, "", " ") + s;
		}

		if (auto p = std::dynamic_pointer_cast<PointerType>(type))
		{
			this->usedPunctuation.insert(Punctuation::StarPointer);
			return this->to_html(p->pointee)+ R"(<tool-tip use="star-pointer-tooltip">*</tool-tip>)" + this->qualifiers_html(type, " ", "");
		}

		if (auto p = std::dynamic_pointer_cast<LValueReferenceType>(type))
		{
			this->usedPunctuation.insert(Punctuation::LValueReference);
			return this->to_html(p->pointee) + R"(<tool-tip use="lvalueref-tooltip">&</tool-tip>)";
		}

		if (auto p = std::dynamic_pointer_cast<RValueReferenceType>(type))
		{
			this->usedPunctuation.insert(Punctuation::RValueReference);
			return this->to_html(p->pointee) + R"(<tool-tip use="rvalueref-tooltip">&&</tool-tip>)";
		}

		if (auto e = std::dynamic_pointer_cast<EnumType>(type))
		{
			this->usedPunctuation.insert(Punctuation::RValueReference);
			bool scoped = e->enumSymbol ? e->enumSymbol->scoped : false;
			return this->qualifiers_html(type, "", " ") + (scoped ? "" : (this->keywordHtml(Keyword::UnscopedEnum, "enum") + " ")) + this->to_html(e->enumSymbol);
		}

		if (auto tdef = std::dynamic_pointer_cast<TypedefType>(type))
		{
			string s = this->to_html(std::dynamic_pointer_cast<TypedefSymbol>(tdef->symbol));
			if (s.empty())
				return this->qualifiers_html(type, "", " ") + std::format(R"(<span style="color:red;">Record (error)</span>)");
			return this->qualifiers_html(type, "", " ") + s;
		}

		return std::format(R"(<span style="color:red;">unhandled_CXX_type ({0})</span>)", escapeHtml(type->spelling()));
	}

	void CxxDocHtmlArticle::finish()
	{
		for (const auto& k : this->usedKeywords)
			this->defs[this->keywordTooltipId(k)] = this->tooltipContentFor(k);

		auto has_punctuation = [&](Punctuation punc) { return this->usedPunctuation.find(punc) != this->usedPunctuation.end(); };

		if (has_punctuation(Punctuation::StarPointer))
			this->defs["star-pointer-tooltip"] = R"a<nfnos(Pointer, see <a href="https://en.cppreference.com/w/cpp/language/pointer">pointer declaration</a>)a<nfnos";
		if (has_punctuation(Punctuation::LValueReference))
			this->defs["lvalueref-tooltip"] = R"a<nfnos(<i>L-value reference</i> (aka reference), see <a href="https://en.cppreference.com/w/cpp/language/reference">reference declaration</a> and <a href="https://learn.microsoft.com/en-us/cpp/cpp/lvalue-reference-declarator-amp?view=msvc-170">L-value reference declaration</a>)a<nfnos";
		if (has_punctuation(Punctuation::RValueReference))
			this->defs["rvalueref-tooltip"] = R"a<nfnos(<i>R-value reference</i>, see <a href="https://en.cppreference.com/w/cpp/language/reference">reference declaration</a> and <a href="https://learn.microsoft.com/en-us/cpp/cpp/rvalue-reference-declarator-amp-amp?view=msvc-170">R-value reference declaration</a>)a<nfnos";
	}

	void write_list_page(const path& fileName, const SymbolRegistry& registry)
	{
		CxxDocHtmlArticle article;

		article.article = "<h1>ciao</h1> ciao <h2>ciao</h2><h2>ciao</h2>";

		for (const auto& [id, symbol] : registry.symbolsById)
		{
			if (auto f = std::dynamic_pointer_cast<FunctionSymbol>(symbol))
			{
				string code;
				if (f->signature)
					code += article.to_html(std::dynamic_pointer_cast<CXXType>(f->signature->ret)) + " ";
				code += article.to_html(f);
				code += "(";
				vector<string> args;
				for (const auto& arg : f->signature->args)
				{
					string a;
					a += article.to_html(std::dynamic_pointer_cast<CXXType>(arg.type));
					string name = arg.name.empty() ? "" : (" "s + arg.name);
					a += "<code-pvar>"s + name + "</code-pvar>";
					args.push_back(a);
				}
				code += join(args, ", ");
				code += ")";
				article.article += std::format(R"(<div clas="p"><pre><code>{0}</code></pre></div>)", code);
				
			}
		}

		article.finish();

		std::ofstream file(fileName);
		file << article.html();
	}
}