
#include <fstream>

#include "Project.hpp"

#include "cxx_parser.hpp"

// !!!
#include <iostream>
#include <nlohmann/json.hpp>
#include <inja/inja.hpp>
#include "html_page.hpp"
#include "string_utils.hpp"

using nlohmann::json;
using namespace std::string_literals;

namespace lcdoc
{
	using std::make_shared;

	shared_ptr<ParsedCXXProject> parse(const shared_ptr<CXXProject>& project)
	{
		if (!project)
			return nullptr;

		auto parsed = make_shared<ParsedCXXProject>();
		CXXDocumentParser parser;

		const auto projectOptions = project->inputFilesOptions.options();

		for (const auto& file : project->inputFiles)
			parser.parse(file.path, projectOptions | file.options.options());

		parsed->registry = std::move(parser.registry);
		parsed->project = project;
		return parsed;
	}

	// TODO string_utils
	string read2str(std::istream& file)
	{
		// https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	// TODO string_utils
	string read2str(std::istream&& file)
	{
		return read2str(file);
	}

	Generator::DocumentTransformer Generator::getDocumentTransformerFor(const string& type)
	{
		assert((bool)this->project);

		auto identityTransformer = [](const path& in, const path& out, const path& basePath, const path& relativePath, const string& ext) -> void {
			std::ofstream file(out);
			file << read2str(std::ifstream(in));
		};

		const auto it = this->documentTransformers.find(type);
		if (it != this->documentTransformers.end())
		{
			const auto& [key, transformer] = *it;

			if (transformer)
				return transformer;
		}

		const auto& models = this->project->models;
		const auto modelIt = models.find(type);
		if (modelIt != models.end())
		{
			const auto& [key, modelPath] = *modelIt;

			return [this, modelPath](const path& in, const path& out, const path& basePath, const path& relativePath, const string& ext) -> void {
				const string& raw = read2str(std::ifstream(in));
				auto sep = extractMeta(raw);
				
				// ================================
				
				json data;
				data["meta"] = to_json(YAML::Load(sep.meta));
				data["article"] = sep.content;
				//data["nav"]["path"].push_back(json({ {"name", "ciao"}, {"url", "#url"} }));
				//data["nav"]["path"].push_back(json({ {"name", "ciao"}, {"url", "#url"} }));
				//data["rootPath"] = "."s;
				data["rootPath"] = basePath.string().empty() ? "."s : basePath.string();

				{
					// spit relative path
					string path = relativePath.string();
					replace_all(path, "\\\\", "\\");
					replace_all(path, "\\", "/");
					auto pieces = split(path, "/");
					string complete = "";
					for (const auto& piece : pieces)
					{
						if (piece.empty())
							continue;

						complete += "/" + piece;

						data["nav"]["path"].push_back(json({ {"name", piece}, {"url", complete} }));
					}
				}

				string r;
				try {
					inja::Template templ = this->injaEnv.parse_template(modelPath.string());
					r = this->injaEnv.render(templ, data);
				}
				catch (const std::exception& e)
				{
					r = e.what();
				}

				// ================================
				std::ofstream file(out);
				file << r;
			};
			
		}

		return identityTransformer;
	}

	optional<Generator::getTransformerName_result> Generator::getTransformerName(const path& file)
	{
		if (!file.has_filename())
			return std::nullopt;

		getTransformerName_result result;
		result.dir = file.parent_path();
		result.ext = file.extension().string();

		const string stem = file.stem().string();

		if (stem.empty())
			return std::nullopt;

		const string& separator = "@";

		const auto pos = stem.find_last_of(separator);

		if (pos == std::string::npos)
		{
			result.stem = stem;
			result.transformerName = "";
			return result;
		}

		result.stem = stem.substr(0, pos);
		result.transformerName = stem.substr(pos + 1);
		return result;
	}

	void Generator::configInja()
	{
		this->injaEnv.add_callback("favicon_tag", 1, [](inja::Arguments& args) -> string {
			const string file = args.at(0)->get<string>();
		const string ext = (file.find(".") != std::string::npos) ? split(file, ".").back() : "";
		const string mime_type = [&]() -> string {
			const auto map = default_mime_types();
			auto it = map.find(ext);
			if (it == map.end())
				return "";
			return it->second;
		}();
		if (mime_type.empty())
			return std::format(R"adef(<link rel="shortcut icon" href="{0}"><!-- ! favicon "{0}" as no recognized mime tyoe-->)adef", file);
		return std::format(R"adef(<link rel="shortcut icon" href="{0}" type="{1}">)adef", file, mime_type);
			});
	}

	void Generator::generate()
	{
		// basic santy checks
		{
			if (!this->project)
				throw std::runtime_error("no project");

			if (!this->parsedProject)
				throw std::runtime_error("no parsed project");
		}

		// in/out folder checks
		{
			if (!std::filesystem::exists(this->project->inputDir) || !std::filesystem::is_directory(this->project->inputDir))
				throw std::runtime_error("input dir does not exists");

			if (!std::filesystem::exists(this->project->outDir))
				std::filesystem::create_directories(this->project->outDir);
			else
				if (!std::filesystem::is_directory(this->project->inputDir))
					throw std::runtime_error("output path exists but it is not a directory");
		}

		const path& inputDir = this->project->inputDir;
		const path& outDir = this->project->outDir;

		this->configInja();

		for (const auto& entry : std::filesystem::recursive_directory_iterator(inputDir))
		{
			const auto path = entry.path();
			const auto relative = std::filesystem::relative(path, inputDir);
			const auto relativeDir = relative.parent_path();
			const auto basePath = std::filesystem::relative(inputDir, path.parent_path());

			if (std::filesystem::is_directory(path))
				std::filesystem::create_directories(outDir / relative);

			if (std::filesystem::is_regular_file(path))
			{
				auto sep = this->getTransformerName(relative);
				if (!sep)
					std::cerr << "error for " << path << " " << relative << std::endl;
				else
				{
					const auto transformer = this->getDocumentTransformerFor(sep.value().transformerName);
					
					const auto outFilePath = outDir / relativeDir / (sep.value().stem + sep.value().ext);
					std::cout << "transforming " << (relativeDir / (sep.value().stem + sep.value().ext)) << std::endl;
					transformer(path, outFilePath, basePath, relativeDir / (sep.value().stem + sep.value().ext), sep.value().ext);
				}
			}
		}

		// additionalMaterial
		for (const auto& entry : this->project->additionalMaterial)
		{
			auto [from, to] = entry;
			from = std::filesystem::absolute(from);
			const auto outFilePath = outDir / to;
			std::filesystem::create_directories(outFilePath.parent_path());
			try
			{
				std::filesystem::copy(from, outFilePath, std::filesystem::copy_options::recursive);
			}
			catch (const std::exception& e)
			{
				std::cerr << "error copying " << from << " to " << outFilePath << " " << e.what() << std::endl;
			}
		}
	}
}