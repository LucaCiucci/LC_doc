
#include <iostream>
#include <functional>
#include <vector>
#include <filesystem>
#include <fstream>
#include <set>
#include <cassert>

#include <cmrc/cmrc.hpp>

#include "clang_interface/TranslationUnit.hpp"

#include "html_page.hpp"
#include "Symbol.hpp"
#include "cxx_parser.hpp"
#include "console.hpp"
#include "list_page.hpp"
#include "Project.hpp"
#include "parse_project.hpp"

CMRC_DECLARE(lcdoc);

using std::make_shared;

void printHelp(void)
{
	// TODO help
	std::cout << "TODO: help" << std::endl;
}

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