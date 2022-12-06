#pragma once

#include <string>
#include <vector>
#include <map>
#include <list>
#include <format>

// !!!
#include "string_utils.hpp"

namespace lcdoc
{
	using std::string;
	using std::vector;
	using std::map;
	using std::list;
	using namespace std::string_literals;

	extern map<string, string> image_mime_types;
	extern map<string, string> javascript_mime_types;
	extern map<string, string> text_mime_types;
	extern map<string, string> css_mime_types;
	extern map<string, string> font_mime_types;
	extern map<string, string> other_mime_types;

	map<string, string> default_mime_types();

	namespace html
	{
		struct Attributes : map<string, string>
		{
			using map<string, string>::map;

			string to_html_element_attributes() const {
				vector<string> attrs;
				for (const auto& [name, value] : *this)
					attrs.push_back(std::format("{0}=\"{1}\"", name, value));
				return join(attrs, " ");
			}
		};

		class Node // TODO abstract
		{
		public:

			virtual string to_html() const = 0;

		private:

		};

		class TextNode final : public Node
		{
		public:

			TextNode();
			TextNode(const TextNode&) = default;
			TextNode(TextNode&&) = default;
			TextNode(const string& text) : text(text) {}

			string to_html() const override {
				// TODO replace escapes
				return this->text;
			}

			string text;

		private:

		};

		class RawCodeNode final : public Node
		{
		public:

			RawCodeNode();
			RawCodeNode(const RawCodeNode&) = default;
			RawCodeNode(RawCodeNode&&) = default;
			RawCodeNode(const string& text) : text(text) {}

			string to_html() const override {
				return this->text;
			}

			string text;

		private:

		};

		class Element : public Node
		{
		public:

		private:

		};

		class HtmlElement : public Element
		{
		public:

			HtmlElement(const string& tag) : m_tag(tag) {};
			HtmlElement(HtmlElement&&) = default;

			HtmlElement(const string& tag, const string& content, const Attributes& attributes = {}) :
				HtmlElement(tag)
			{
				this->children.push_back(std::make_unique<RawCodeNode>(content));
				this->attributes = attributes;
			}

			vector<std::unique_ptr<Node>> children;

			Attributes attributes;

			string to_html() const {
				const string attrs = [this]() -> string {
					if (this->attributes.empty()) return "";
					return " "s + this->attributes.to_html_element_attributes();
				}();
				const string html = this->innerHtml();
				if (html.empty() && this->compactable())
					return std::format("<{0}{1}/>", this->tag(), attrs);
				return std::format("<{0}{1}>{2}</{0}>", this->tag(), attrs, html);
			}

			string tag() const {
				return m_tag;
			}

			string innerHtml() const {
				string html;
				for (const auto& pc : this->children)
					if (pc)
						html += pc->to_html();
				return html;
			}

			string outherHtml() const {
				return this->to_html();
			}

		protected:

			virtual bool compactable() const { return false; }

			virtual bool finalSlash() const { return true; }

		private:
			const string m_tag;
		};

		class HtmlLinkElement : public HtmlElement
		{
		public:

			HtmlLinkElement() :
				HtmlElement("link")
			{
			}

			HtmlLinkElement(const string& rel, const string& href) :
				HtmlLinkElement()
			{
				this->attributes["rel"] = rel;
				this->attributes["href"] = href;
			}

		protected:

			bool compactable() const override final {
				return true;
			}

			bool finalSlash() const override final {
				return false;
			}

		private:

		};

		inline HtmlLinkElement htmlStylesheetLinkElement(const string& href) {
			return HtmlLinkElement("stylesheet", href);
		}

		class HtmlStyleElement : public HtmlElement
		{
		public:

			HtmlStyleElement(const string& content = "") :
				HtmlElement("style")
			{
				this->children.push_back(std::make_unique<RawCodeNode>(content));
			}

		private:
		};

		class HtmlScriptElement : public HtmlElement
		{
		public:

			HtmlScriptElement() :
				HtmlElement("script")
			{
			}

		private:
		};
	}

	class TextDocument
	{
	public:

		virtual string text() const = 0;

	private:
	};

	class HtmlDocument : public TextDocument
	{
	public:

		string text() const override final { return this->html(); }

		string html() const;

		virtual string head() const = 0;
		virtual string body() const = 0;

		string lang = "eng";

	private:
	};

	class StandardHtmlDocument : public HtmlDocument
	{
	public:

		struct style_sheet
		{
			string src;
			string css;
			//bool atEndOfBody = false;
		};

		struct js_script
		{
			string src;
			bool async = false;
			string type;
			string code;
			bool defer = false;
			bool atEndOfBody = false;
		};

		string charset = "UTF-8";

		string title = "Title";

		/**
		 * @see https://developer.mozilla.org/en-US/docs/MDN/Guidelines/Writing_style_guide#choosing_titles_and_slugs
		 * @see https://developer.mozilla.org/en-US/docs/Glossary/Slug
		 * @see https://www.w3.org/International/articles/language-tags/
		 */
		string slug = "";

		/**
		 * Additional content to be added to the head section of the html page
		 */
		virtual string additional_head_content() const { return ""; }

		/**
		 * Additional meta tags to be added to the head section of the html page
		 * @see https://developer.mozilla.org/en-US/docs/Web/HTML/Element/meta
		 */
		map<string, string> meta_tags = {
			{ "description", "" },
			{ "author", "" },
			{ "keywords", "" },
			{ "page-subject", "" },
		};

		/**
		 * Stylesheets paths to be added to the page
		 */
		list<style_sheet> styleSheets;

		/**
		 * Fonts paths to be added to the page
		 */
		vector<string> fonts;

		/**
		 * Scripts path to be added to the page, example:
		 * ```
		 * let page = new BasicHtml5Page();
		 * page.scripts.push("/some/script.js");
		 * page.scripts.push({
		 *     src: "/some/other/script.js" },
		 *     async: true
		 * });
		 * page.scripts.push({
		 *     src: "/some/other/script.mjs" },
		 *     async: true,
		 *     type: "module"
		 * });
		 * ```
		 */
		list<js_script> scripts;

		/**
		 * A different favicon can be specified by setting the favicon property.
		 * Seto to "" to disable and use the default favicon.
		 * @see https://developer.mozilla.org/en-US/docs/Web/HTML/Link_types/Icon
		 */
		string favicon = "";

		virtual string bodyContent() const = 0;

		string head() const override final;
		string body() const override final;

	private:

		string meta_tags_html() const;
		string title_tag_html() const;
		string favicon_tag_html() const;
		vector<string> style_tags_html(bool atEnd) const;
		vector<string> font_tags_html() const;
		vector<string> script_tags_html(bool atEnd) const;

	};

	class LCHtmlArticle : public StandardHtmlDocument
	{
	public:

		map<string /*id*/, string> defs;

		struct RelatedArticle {
			string url;
			string title;
			string slug;
		};

		vector<RelatedArticle> relatedArticles;

		string additionalHeadContent;
		string additional_head_content() const override { return ""; }

		//const string opnContentBase = "//cdn.jsdelivr.net/gh/OpenPhysicsNotes/openphysicsnotes-content";
		const string opnContentBase = "./opn";
		//const string opnContentBase = R"amfedpsv(C:\Users\lucac\Documents\develop\node\openphysicsnotes-content)amfedpsv";

		string article;

		string bodyContent() const override final;

		void setupScripts();

		void setupStyles();

		void useYamlMeta(const string& yaml);

	private:

	};
}