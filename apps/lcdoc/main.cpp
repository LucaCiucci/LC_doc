
#include <iostream>
#include <functional>
#include <vector>
#include <filesystem>
#include <fstream>
#include <set>
#include <cassert>

#include <cmrc/cmrc.hpp>

#include <yaml-cpp/yaml.h>

#include <glob/glob.h>

#include "clang_interface/TranslationUnit.hpp"

#include "html_page.hpp"
#include "Symbol.hpp"
#include "cxx_parser.hpp"
#include "console.hpp"
#include "list_page.hpp"
#include "Project.hpp"

CMRC_DECLARE(lcdoc);

using std::make_shared;

void printHelp(void)
{
	// TODO help
	std::cout << "TODO: help" << std::endl;
}

void parseLcdocProject(const std::shared_ptr<lcdoc::CXXProject>& project, const std::filesystem::path& projectFile);

int main(int argc, char** argv)
{
	using namespace lcdoc::clang;
	using namespace lcdoc;
	using std::vector;
	using std::exception;
	using std::filesystem::path;

	std::cout << colorize("Hello There!", { 0, 255, 0 }) << std::endl;

	for (int i = 0; i < argc; ++i)
		std::cout << colorize("argv", { 150, 150, 150 }) << "[" << colorize(std::to_string(i), { 181, 206, 168 }) << "]: " << colorize(argv[i], {214, 157, 122}) << std::endl;
	

	// check if help is required
	const bool help = [&]() -> bool {
		for (int i = 0; i < argc; ++i)
			if (string(argv[i]).starts_with("-h") || string(argv[i]).starts_with("--h"))
				return true;
		return false;
	}();

	if (help)
	{
		printHelp();
		return 0;
	}

	// get the first argument
	const string arg = [&]() -> string {
		if (argc <= 1)
			return "";
		return argv[1];
	}();

	// get the working directory and the project file
	const auto& [workingDir, projectFile] = [&]() -> std::pair<path, path> {
		try {
			const path inputPath = std::filesystem::absolute(arg.empty() ? "." : arg);

			if (!std::filesystem::exists(inputPath))
				throw std::runtime_error("input path does not exists");

			if (std::filesystem::is_regular_file(inputPath))
			{
				return { inputPath.parent_path(), inputPath };
			}
			else if (std::filesystem::is_directory(inputPath))
			{
				const path inputFile = inputPath / "lcdoc.yaml";
				if (!std::filesystem::is_regular_file(inputFile))
					throw std::runtime_error("could not find lcdoc.yaml");
				return { inputPath, inputFile };
			}
			else
				throw std::runtime_error("input path is not a file nor a directory");
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to get working dir and project file: " << e.what() << std::endl;
			exit(0); // !!!
		}
		return {};
	}();

	std::cout << "working dir:  " << workingDir << std::endl;
	std::cout << "project file: " << projectFile << std::endl;

	auto project = std::make_shared<CXXProject>();
	project->rootDir = workingDir;
	//project->models["article"] = R"(C:\Users\lucac\Documents\develop\vs\libraries\LC_doc\example_project\doc_src\templates\pages\article.html)";

	try
	{
		parseLcdocProject(project, projectFile);
	}
	catch (const exception& e)
	{
		std::cerr << "failed to parse project file: " << e.what() << std::endl;
		return 0;
	}

	auto parsed = parse(project);

	Generator generator(project, parsed);

	generator.generate();

	return 0;

	project->inputFilesOptions.standard = "c++20";
	project->inputFilesOptions.additionalIncludeDirs.insert(R"(C:\Program Files\LLVM\include)");
	project->inputFiles.push_back(CXXInputSourceFile{ .path = "C:/Users/lucac/Documents/develop/vs/libraries/LC_doc/apps/lcdoc/main.cpp" });

	// !!!
	try {
		std::filesystem::create_directory_symlink(R"amfedpsv(C:\Users\lucac\Documents\develop\node\openphysicsnotes-content)amfedpsv", std::filesystem::absolute("./out2/opn"));
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	//write_list_page(outDir / "ciao.html", parser.registry);

	//test();

	return 0;
}

void parseLcdocCompilationOptions(const YAML::Node& yaml, lcdoc::CXXFileOptions& options)
{
	using std::string;

	// standard
	if (yaml["standard"])
		options.standard = yaml["standard"].as<string>();

	// additional include dirs
	if (yaml["includeDirs"])
	{
		const auto& dirs = yaml["includeDirs"];
		for (const auto& dir : dirs)
			options.additionalIncludeDirs.insert(dir.as<string>());
	}

	// additionalFlags
	if (yaml["additionalFlags"])
	{
		const auto& flags = yaml["additionalFlags"];
		for (const auto& flag : flags)
			options.additionalOptions.push_back(flag.as<string>());
	}

	// defines
	if (yaml["defines"])
	{
		const auto& defines = yaml["defines"];
		for (const auto& define : defines)
			// if string
			if (define.IsScalar())
				options.additionalDefinitions[define.as<string>()] = "";
			// if map
			else if (define.IsMap() && define.size() == 1)
			{
				const auto& key = define.begin()->first.as<string>();
				const auto& value = define.begin()->second.as<string>();
				options.additionalDefinitions[key] = value;
			}
	}

	// undefines
	if (yaml["undefines"])
	{
		const auto& undefines = yaml["undefines"];
		for (const auto& undefine : undefines)
			options.undefines.insert(undefine.as<string>());
	}
}

void parseLcdocProject(const std::shared_ptr<lcdoc::CXXProject>& project, const std::filesystem::path& projectFile)
{
	using namespace std::string_literals;
	using std::string;
	using std::exception;
	using std::runtime_error;
	using std::filesystem::path;
	using YAML::Node;

	assert((bool)project);

	// load and parse the project file
	Node yaml = [&]() -> Node {
		try
		{
			return YAML::LoadFile(projectFile.string());
		}
		catch (const exception& e)
		{
			throw runtime_error("Could not load project file: "s + e.what());
		}
		return {};
	}();

	auto isStringProperty = [](const Node& node, const string& property) -> bool {
		if (!node[property].IsDefined())
			return false;
		if (node[property].IsScalar())
			return true;
		return false;
	};

	// Now we try to get the file properties
	try
	{
		// project name
		if (isStringProperty(yaml, "projectName"))
		{
			const string projectName = yaml["projectName"].as<string>();
		}
		else
			throw runtime_error("\"property projectName of type string is required\"");

		// project version
		if (yaml["projectVersion"].IsDefined())
		{
			if (yaml["projectVersion"].IsScalar())
			{
				const string projectVersion = yaml["projectVersion"].as<string>();
				project->version = lcdoc::split(projectVersion, ".");
			}
			else if (yaml["projectVersion"].IsSequence())
			{
				for (const auto& v : yaml["projectVersion"])
				{
					if (!v.IsScalar())
						throw runtime_error("projectVersion must be a sequence of strings or numbers");
					project->version.push_back(v.as<string>());
				}
			}
			else
				throw runtime_error("projectVersion must be a string, a numebr or a sequence of strings or numbers");
		}
		else
			throw runtime_error("\"property projectVersion of type string, numebr or sequence of strings or numbers is required\"");

		// root dir
		if (isStringProperty(yaml, "rootDir"))
		{
			const path rootDir = yaml["rootDir"].as<string>();
			if (rootDir.is_absolute())
				project->rootDir = rootDir;
			else
				project->rootDir = projectFile.parent_path() / rootDir;
		}

		const auto resolveProjectPath = [&](const path& _path) -> path {
			if (_path.is_absolute())
				return _path;
			else
				return project->rootDir / _path;
		};

		const auto resolveAndGlob = [&](const path& _path) -> std::vector<path> {
			const path resolved = resolveProjectPath(_path.lexically_normal());
			std::cout << "globbing " << resolved << std::endl;
			// TODO doesn't work
			return glob::glob(_path.string());
		};

		// input directory
		if (isStringProperty(yaml, "inputDir"))
		{
			const string inputDir = yaml["inputDir"].as<string>();
			project->inputDir = resolveProjectPath(inputDir);
		}
		else
			throw runtime_error("\"property inputDir of type string is required\"");

		// output directory
		if (isStringProperty(yaml, "outDir"))
		{
			const string outputDir = yaml["outDir"].as<string>();
			project->outDir = resolveProjectPath(outputDir);
		}
		else
			throw runtime_error("\"property outDir of type string is required\"");

		// additionalMaterial
		if (yaml["additionalMaterial"].IsDefined())
		{
			if (yaml["additionalMaterial"].IsSequence())
			{
				for (const auto& m : yaml["additionalMaterial"])
				{
					if (m.IsMap())
						for (const auto& entry : m)
						{
							const auto target = entry.first.as<string>();
							const auto source = entry.second.as<string>();
							project->additionalMaterial.insert({ resolveProjectPath(source), target });
						}
					if (m.IsScalar())
						project->additionalMaterial.insert({ resolveProjectPath(m.as<string>()), path(m.as<string>()).filename() });
				}
			}
			else
				throw runtime_error("additionalMaterial must be a string or a sequence of strings");
		}

		// compilationOptions
		if (yaml["compilationOptions"].IsDefined())
		{
			if (yaml["compilationOptions"].IsMap())
			{
				const auto& options = yaml["compilationOptions"];
				parseLcdocCompilationOptions(options, project->inputFilesOptions);
			}
			else
				throw runtime_error("compilationOptions must be a map");
		}

		// files
		if (yaml["inputFiles"].IsDefined())
		{
			if (yaml["inputFiles"].IsSequence())
			{
				const auto& files = yaml["inputFiles"];
				for (const auto& entry : files)
				{
					if (entry.IsScalar())
						for (const auto& path : resolveAndGlob(entry.as<string>()))
						{
							lcdoc::CXXInputSourceFile file;
							file.path = path.string();
							project->inputFiles.push_back(file);
						}
					if (entry.IsMap())
					{
						lcdoc::CXXFileOptions options;
						if (entry["compilationOptions"].IsDefined())
						{
							if (entry["compilationOptions"].IsMap())
							{
								const auto& cfgOptions = entry["compilationOptions"];
								parseLcdocCompilationOptions(cfgOptions, options);
							}
							else
								throw runtime_error("compilationOptions must be a map");
						}
						
						if (isStringProperty(entry, "path"))
							for (const auto& path : resolveAndGlob(entry["path"].as<string>()))
							{
								lcdoc::CXXInputSourceFile file;
								file.path = path.string();
								file.options = options;
								project->inputFiles.push_back(file);
							}
						else
							throw runtime_error("\"property path of type string is required\"");

						// TODO excludes
					}
				}
			}
			else
				throw runtime_error("files must be a sequence of strings");
		}
		else
			throw runtime_error("\"property inputFiles of type sequence of strings is required\"");

		// templates
		if (yaml["templates"].IsDefined())
		{
			if (yaml["templates"]["models"].IsDefined())
			{
				if (yaml["templates"]["models"].IsMap())
				{
					for (const auto& m : yaml["templates"]["models"])
					{
						const auto key = m.first.as<string>();
						const auto path = m.second.as<string>();

						project->models[key] = resolveProjectPath(path);
					}
				}
				else
					throw runtime_error("templates.models must be a map");
			}
		}

		std::cout << project->inputFiles.size() << " files found" << std::endl;
	}
	catch (const std::exception& e)
	{
		throw runtime_error("Invalid project file: "s + e.what());
	}
}