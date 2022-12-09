#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <filesystem>
#include <memory>
#include <optional>

// !!!
#include <format>
#include <inja/inja.hpp>
#include "string_utils.hpp"
#include "Symbol.hpp"

namespace lcdoc
{
	using std::string;
	using std::vector;
	using std::set;
	using std::map;
	using std::pair;
	using std::filesystem::path;
	using std::shared_ptr;
	using std::weak_ptr;
	using std::optional;

	struct CXXFileOptions
	{
		string standard;// = "c++20";
		set<string> additionalIncludeDirs;
		map<string, string> additionalDefinitions;
		set<string> undefines;
		vector<string> additionalOptions;

		// see https://clang.llvm.org/docs/ClangCommandLineReference.html
		vector<string> options() const {
			vector<string> options;

			using std::format;

			// standard
			if (!this->standard.empty())
				options.push_back(format(R"(-std={})", this->standard));

			// include
			for (const string& dir : this->additionalIncludeDirs)
				options.push_back(format(R"(-I{})", dir));

			// definitions
			for (const auto& [name, definitions] : this->additionalDefinitions)
				options.push_back(format(R"(-D{}={})", name, definitions));

			// undefines
			for (const string& undef : this->undefines)
				options.push_back(format(R"(-U{})", undef));

			return std::move(options) | this->additionalOptions;
		}
	};

	struct CXXInputSourceFile
	{
		string path;
		CXXFileOptions options;
	};

	class CXXProject
	{
	public:

		string name;
		vector<string> version;
		path rootDir;
		vector<CXXInputSourceFile> inputFiles;
		CXXFileOptions inputFilesOptions;

		path inputDir = path(LCDOC_SOURCE_DIR) / "example" / "doc_src";
		path outDir = "./out2";
		set<pair<path, path>> additionalMaterial; // TODO https://stackoverflow.com/questions/1902681/expand-file-names-that-have-environment-variables-in-their-path

		map<string, path> models;

	private:

	};

	class ParsedCXXProject
	{
	public:

		weak_ptr<CXXProject> project;

		SymbolRegistry registry;

	private:
	};

	shared_ptr<ParsedCXXProject> parse(const shared_ptr<CXXProject>& project);

	// TODO sposta
	string read2str(std::istream& in);

	class Generator
	{
	public:

		using DocumentTransformer = std::function<void(const path& in, const path& out, const path& basePath, const path& relativeDir, const string& ext)>;

		Generator(const shared_ptr<CXXProject>& project, const shared_ptr<ParsedCXXProject>& parsedProject) :
			project(project),
			parsedProject(parsedProject)
		{
		}

		const shared_ptr<CXXProject> project;
		const shared_ptr<ParsedCXXProject> parsedProject;

		map<string, DocumentTransformer> documentTransformers;

		struct getTransformerName_result {
			path dir;
			string stem;
			string transformerName;
			string ext;
		};
		static optional<getTransformerName_result> getTransformerName(const path& file);

		DocumentTransformer getDocumentTransformerFor(const string& type);

		inja::Environment injaEnv;

		void configInja();

		void generate();

	private:
	};
}