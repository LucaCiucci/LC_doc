
#include <iostream>
#include <functional>
#include <vector>
#include <filesystem>
#include <fstream>
#include <set>
#include <cassert>

#include <cmrc/cmrc.hpp>

// !!!
#include <efsw/efsw.hpp>

#include <argparse/argparse.hpp>

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

// Inherits from the abstract listener class, and implements the the file action handler
class FuncitonalUpdateListener final : public efsw::FileWatchListener {
public:

	using Listener = std::function<void(efsw::WatchID watchid, const std::string& dir,
		const std::string& filename, efsw::Action action,
		std::string oldFilename)>;

	Listener listener;
	std::function<void(void)> boldLstener;

	void handleFileAction(efsw::WatchID watchid, const std::string& dir,
		const std::string& filename, efsw::Action action,
		std::string oldFilename) override {

		if (this->listener)
			this->listener(watchid, dir, filename, action, oldFilename);

		if (this->boldLstener)
			this->boldLstener();
	}
};

void prepareConsole(void);

void printHelp(void)
{
	// TODO help
	std::cout << "TODO: help" << std::endl;
}

int main(int argc, const char* argv[])
{
	using namespace lcdoc::clang;
	using namespace lcdoc;
	using std::vector;
	using std::exception;
	using std::filesystem::path;
	
	prepareConsole();

	argparse::ArgumentParser program("lcdoc", "0.0.0");

	program.add_description("LC documentation generator");
	program.add_epilog("TODO ...");

	program
		.add_argument("path")
		.help("the folder containing a lcdoc.yaml project or a specific .yaml project file")
		.default_value(string("."));

	program
		.add_argument("-w", "--watch")
		.help("watch for changes")
		.default_value(false)
		.implicit_value(true);

	try
	{
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err)
	{
		std::cerr << err.what() << std::endl;
		std::cerr << program;
		return EXIT_FAILURE;
	}

	// get the working directory and the project file
	const auto& [workingDir, projectFile] = [&]() -> std::pair<path, path> {
		try {
			const path inputPath = std::filesystem::absolute(program.get<string>("path"));

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
			exit(EXIT_FAILURE); // !!!
		}
		return {};
	}();

	std::cout << "working dir:  " << workingDir << std::endl;
	std::cout << "project file: " << projectFile << std::endl;

	// create the main project
	auto project = std::make_shared<CXXProject>();
	project->rootDir = workingDir;

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

	// !!!
	{
		efsw::FileWatcher fileWatcher;

		FuncitonalUpdateListener listener;
		listener.boldLstener = [&]() {
			generator.generate();
		};

		auto watchID = fileWatcher.addWatch(project->inputDir.string(), &listener, true);

		fileWatcher.watch();

		if (program["--watch"] == true)
			// loop forever
			while (true)
				std::this_thread::yield();
	}

	return EXIT_SUCCESS;

	project->inputFilesOptions.standard = "c++20";
	project->inputFilesOptions.additionalIncludeDirs.insert(R"(C:\Program Files\LLVM\include)");
	project->inputFiles.push_back(CXXInputSourceFile{ .path = "C:/Users/lucac/Documents/develop/vs/libraries/LC_doc/apps/lcdoc/main.cpp" });

	return EXIT_SUCCESS;
}

void prepareConsole(void)
{
#ifdef WIN32
	// TODO sposta su una libreria
	// Allow usage of ANSI escape sequences on Windows 10
	// and UTF-8 console output
	std::system("chcp 65001 > NUL");
#endif
}