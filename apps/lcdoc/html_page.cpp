
#include <format>

#include <yaml-cpp/yaml.h>

#include "string_utils.hpp"

#include "html_page.hpp"

using namespace std::string_literals;

namespace lcdoc
{
	map<string, string> image_mime_types = {
		{ "jpg", "image/jpeg" },
		{ "jpeg", "image/jpeg" },
		{ "png", "image/png" },
		{ "gif", "image/gif" },
		{ "svg", "image/svg+xml" },
		{ "ico", "image/x-icon" },
		{ "bmp", "image/bmp" },
		{ "webp", "image/webp" },
		{ "tiff", "image/tiff" },
		{ "tif", "image/tiff" },
		{ "jfif", "image/jpeg" },
		{ "jpe", "image/jpeg" },
		{ "jpx", "image/jpeg" },
		{ "j2k", "image/jpeg" },
		{ "jp2", "image/jpeg" },
		{ "j2c", "image/jpeg" },
		{ "jpc", "image/jpeg" },
	};

	map<string, string> javascript_mime_types = {
		{ "js", "application/javascript" },
		{ "mjs", "application/javascript" },
		{ "ts", "application/javascript" },
		{ "tsx", "application/javascript" },
	};

	map<string, string> text_mime_types = {
		{ "html", "text/html" },
		{ "htm", "text/html" },
		{ "shtml", "text/html" },
		{ "xhtml", "text/html" },
		{ "xml", "text/xml" },
		{ "xsl", "text/xml" },
		{ "xslt", "text/xml" },
		{ "xsd", "text/xml" },
		{ "xslth", "text/xml" },
		{ "xht", "text/xml" },
		{ "xhtm", "text/xml" },
		{ "md", "text/plain" },
		{ "txt", "text/plain" },
	};

	map<string, string> css_mime_types = {
		{ "css", "text/css" },
		{ "less", "text/css" },
		{ "scss", "text/css" },
		{ "sass", "text/css" },
		{ "styl", "text/css" },
		{ "stylus", "text/css" },
	};

	map<string, string> font_mime_types = {
		{ "woff", "font/woff" },
		{ "woff2", "font/woff2" },
		{ "ttf", "font/ttf" },
		{ "otf", "font/otf" },
		{ "eot", "font/eot" },
	};

	map<string, string> other_mime_types = {};

	map<string, string> default_mime_types()
	{
		map<string, string> types;

		auto add = [&](const map<string, string>& tt) {
			for (const auto& t : tt)
				types.insert(t);
		};

		add(image_mime_types);
		add(javascript_mime_types);
		add(text_mime_types);
		add(css_mime_types);
		add(font_mime_types);
		add(other_mime_types);

		return types;
	}

	string HtmlDocument::html() const
	{
		return std::format(R"wengpr(
<!DOCTYPE html>
<html{0}>
<head>
{1}
</head>
<body>
{2}
</body>
</html>
)wengpr",
			this->lang.empty() ? "" : (" lang=\""s + this->lang + "\""),
			indent(this->head()),
			this->body()
		);
	}

	string StandardHtmlDocument::head() const
	{
		const string content = [this]() -> string {
			vector<string> pieces;
			pieces.push_back(this->meta_tags_html());
			if (!this->title.empty())
				pieces.push_back(indent(this->title_tag_html()));
			if (!this->favicon.empty())
				pieces.push_back(indent(this->favicon_tag_html()));
			for (const auto& piece : this->style_tags_html(false))
				pieces.push_back(piece);
			for (const auto& piece : this->font_tags_html())
				pieces.push_back(piece);
			for (const auto& piece : this->script_tags_html(false))
				pieces.push_back(piece);
			pieces.push_back(this->additional_head_content());

			return join(pieces, "\n");
		}();

		return content;
	}

	string StandardHtmlDocument::body() const
	{
		const string content = [this]() -> string {
			vector<string> pieces;
			pieces.push_back(this->bodyContent());
			for (const auto& piece : this->script_tags_html(true))
				pieces.push_back(piece);

			return join(pieces, "\n");
		}();

		return content;
	}

	string StandardHtmlDocument::meta_tags_html() const
	{
		vector<string> tags;

		auto makeMetaTag = [](const string& name, const string& content) -> string {
			return std::format("<meta name=\"{0}\" content=\"{0}\">", content);
		};

		// !!!
		tags.push_back(R"(<meta http-equiv="X-UA-Compatible" content="IE=edge">)");
		tags.push_back(R"(<meta name="viewport" content="width=device-width, initial-scale=1.0">)");

		if (!this->charset.empty())
			tags.push_back(std::format("<meta charset=\"{}\">", this->charset));

		for (const auto& [name, content] : this->meta_tags)
			if (!content.empty())
				tags.push_back(makeMetaTag(name, content));

		return join(tags, "\n");
	}

	string StandardHtmlDocument::title_tag_html() const
	{
		string title = this->slug;

		if (title.empty())
			title = this->title;

		if (title.empty())
			return "";

		return "<title>"s + title + "</title>"s;
	}

	string StandardHtmlDocument::favicon_tag_html() const
	{
		if (this->favicon.empty())
			return "";

		// get favicon extension
		// TODO using path
		const string ext = (this->favicon.find(".") != std::string::npos) ? split(this->favicon, ".").back() : "";

		const string mime_type = [&]() -> string {
			const auto map = default_mime_types();
			auto it = map.find(ext);
			if (it == map.end())
				return "";
			return it->second;
		}();

		if (mime_type.empty())
			return std::format(R"adef(<!--Warning, favicon "{0}" as no recognized mime tyoe-->
<link rel="shortcut icon" href="{0}">)adef", this->favicon);

		return std::format(R"adef(<link rel="shortcut icon" href="{0}" type="{1}">)adef", this->favicon, mime_type);
	}

	vector<string> StandardHtmlDocument::style_tags_html(bool atEnd) const
	{
		vector<string> tags;

		for (const auto& sheet : this->styleSheets)
			if (!sheet.src.empty())
				tags.push_back(html::htmlStylesheetLinkElement(sheet.src).to_html());
			else if (!sheet.css.empty())
				tags.push_back(html::HtmlStyleElement(sheet.css).to_html());

		return tags;
	}

	vector<string> StandardHtmlDocument::font_tags_html() const
	{
		vector<string> tags;

		for (const auto& font : this->fonts)
			if (!font.empty())
				tags.push_back(html::htmlStylesheetLinkElement(font).to_html());

		return tags;
	}

	vector<string> StandardHtmlDocument::script_tags_html(bool atEnd) const
	{
		vector<string> tags;

		for (const auto& scr : this->scripts)
			if (scr.atEndOfBody == atEnd)
			{
				html::HtmlScriptElement script;
				if (!scr.src.empty())
					script.attributes["src"] = scr.src;
				if (!scr.code.empty())
					script.children.push_back(std::make_unique<html::RawCodeNode>(scr.code));
				if (scr.async)
					script.attributes["async"]; // <- TODO
				if (scr.defer)
					script.attributes["defer"]; // <- TODO
				if (!scr.type.empty())
					script.attributes["type"] = scr.type; // <- TODO
				tags.push_back(script.to_html());
			}

		return tags;
	}

	string LCHtmlArticle::bodyContent() const {

		string defs;
		for (const auto& [id, def] : this->defs)
		{
			html::HtmlElement e("lc-def");
			e.attributes["id"] = id;
			e.children.push_back(std::make_unique<html::RawCodeNode>(def));
			defs += e.outherHtml() + "\n";

		}

		const string sidebar = [this]() -> string {
			string content;
			if (!this->relatedArticles.empty())
			{
				content += "<h2>Related articles</h2>\n";
				content += "<ul>\n";
				for (const auto& article : this->relatedArticles)
					content += std::format(R"(<li><a href="{1}">{0}</a></li>)", article.slug.empty() ? article.title : article.slug, article.url) + "\n";
				content += "</ul>\n";
			}

			if (content.empty())
				return "";
			return "<lc-sidebar>\n"s + content +"</lc-sidebar>\n";
		}();
		

		return std::format(R"esnfgro(

<lc-defs>
{0}
</lc-defs>

<header>
    <div>
        <!--div class="button"><a href="">OPN</a></div-->
        <!--a href="/"><img class="button logo" src="//google.com/favicon.ico" alt="OPN"></img></a-->
        <div class="button" style="font-size: 125%;"><a href="${{href}}">lcdoc</a></div>
        <div class="links">
            <div class="button"><a href="${{href}}">ciao</a></div>
            <div class="button"><a href="${{href}}">ciao</a></div>
            <div class="button"><a href="${{href}}">ciao</a></div>
            <div class="button"><a href="${{href}}">ciao</a></div>
        </div>
        <!--div class="button"><a href="">search</a></div-->
        <!--div class="search-form"><div class="gcse-search" data-gname="storesearch"></div></div-->
    </div>
</header>

<lc-topnav><div><a href="${{url}}">some</a><a href="${{url}}">path</a></div></lc-topnav>
<!--div class="top-notice orange"><b>experiemntal</b></div-->

<div class="top-notice orange">
    Experimental
</div>

<lc-content>

{2}

    <article>
{1}
    </article>

    <div class="out-nav-index-container"><lc-nav-index class="out-nav-index"></lc-nav-index></div>

</lc-content>

)esnfgro", defs, article, sidebar);
	}

	void LCHtmlArticle::setupScripts()
	{
		using js = StandardHtmlDocument::js_script;

		// tooltip
		this->scripts.push_back(js{ .src = this->opnContentBase + "/js/tool-tip.js", .async = true });

		// math and refs
		this->scripts.push_back(js{ .src = "//cdn.jsdelivr.net/npm/tex-math@latest/dist/tex-math.js", .async = true });
		this->scripts.push_back(js{ .src = "//cdn.jsdelivr.net/npm/lc-ref@latest/dist/lc-ref.js", .async = true });

		// dynamic syntax highlighting
		this->scripts.push_back(js{ .src = "//cdnjs.cloudflare.com/ajax/libs/highlight.js/11.5.0/highlight.min.js" });
		this->scripts.push_back(js{ .src = "//cdnjs.cloudflare.com/ajax/libs/highlight.js/11.5.0/languages/latex.min.js" });
		this->scripts.push_back(js{ .code =
R"enfoirnger(//hljs.highlightAll();
document.addEventListener("DOMContentLoaded", function() {{
    document.querySelectorAll('code:not(.nohighlight)').forEach((block) => {{
        //hljs.highlightElement(block);
    }});
    // https://github.com/highlightjs/highlight.js/issues/1737
    document.querySelectorAll('code:not([class*="language-"])').forEach(function($) {
        $.className = $.className ? $.className + ' no-highlight' : 'no-highlight';
    });
    hljs.configure({cssSelector: 'code[class*="language-"]'});
    hljs.highlightAll();
}}))enfoirnger"
			});

		this->scripts.push_back(js{ .src = opnContentBase + "/js/index.js", .atEndOfBody = true });
	}

	void LCHtmlArticle::setupStyles()
	{
		using style = StandardHtmlDocument::style_sheet;

		this->styleSheets.push_back(style{ .src = opnContentBase + "/css/style.css" });
		this->styleSheets.push_back(style{ .src = opnContentBase + "/css/tool-tip.css" }); // ! temporary
		this->styleSheets.push_back(style{ .css = "code span { text-decoration: none; color: inherit; }" });
	}

	void LCHtmlArticle::useYamlMeta(const string& _yaml)
	{
		YAML::Node yaml = YAML::Load(_yaml);

		if (yaml["title"].IsScalar() && !yaml["title"].as<string>().empty())
		{
			string title = yaml["title"].as<string>();
			this->title = title;
			this->meta_tags["og:title"] = title;
			this->meta_tags["twitter:title"] = title;
		}

		if (yaml["related_articles"].IsSequence())
		{
			for (const auto& related_article : yaml["related_articles"])
			{
				RelatedArticle article;

				if (related_article["title"].IsScalar() && !related_article["title"].as<string>().empty())
					article.title = related_article["title"].as<string>();
				else
					article.title = "no-title";

				if (related_article["url"].IsScalar() && !related_article["url"].as<string>().empty())
					article.url = related_article["url"].as<string>();
				else
					article.title = "#no-url";

				if (related_article["slug"].IsDefined() && related_article["slug"].IsScalar() && !related_article["slug"].as<string>().empty())
					article.url = related_article["slug"].as<string>();

				this->relatedArticles.push_back(article);
			}
		}

		if (yaml["favicon"].IsScalar() && !yaml["favicon"].as<string>().empty())
			this->favicon = yaml["favicon"].as<string>();

		if (yaml["keywords"].IsSequence())
		{
			vector<string> keywords;
			for (const auto& keyword : yaml["keywords"])
				if (keyword.IsScalar())
					keywords.push_back(keyword.as<string>());
			this->meta_tags["keywords"] = join(keywords, ", ");
		}

		if (yaml["keyword"].IsScalar() && !yaml["keyword"].as<string>().empty())
			this->meta_tags["keywords"] += (this->meta_tags["keywords"].empty() ? "" : ", ") + yaml["keyword"].as<string>();

		if (yaml["description"].IsScalar() && !yaml["description"].as<string>().empty())
		{
			const string description = yaml["description"].as<string>();
			this->meta_tags["description"] = description;
			this->meta_tags["og:description"] = description;
			this->meta_tags["twitter:description"] = description;
		}

		if (yaml["subject"].IsScalar() && !yaml["subject"].as<string>().empty())
			this->meta_tags["page-subject"] = yaml["subject"].as<string>();

		if (yaml["image"].IsScalar() && !yaml["image"].as<string>().empty())
		{
			const string image = yaml["image"].as<string>();
			this->meta_tags["og:image"] = image;
			this->meta_tags["twitter:image"] = image;
		}

		// TODO
		// if (yaml.meta && typeof yaml.meta === "object") {
		// 	for (let key in yaml.meta) {
		// 		this.meta[key] = yaml.meta[key];
		// 	}
		// }

		if (yaml["additional_head_content"].IsScalar() && !yaml["additional_head_content"].as<string>().empty())
			this->additionalHeadContent += yaml["additional_head_content"].as<string>();
	}
}