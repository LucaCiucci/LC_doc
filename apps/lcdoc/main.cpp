
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
		switch (action) {
		case efsw::Actions::Add:
			std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Added"
				<< std::endl;
			break;
		case efsw::Actions::Delete:
			std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Delete"
				<< std::endl;
			break;
		case efsw::Actions::Modified:
			std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Modified"
				<< std::endl;
			break;
		case efsw::Actions::Moved:
			std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Moved from ("
				<< oldFilename << ")" << std::endl;
			break;
		default:
			std::cout << "Should never happen!" << std::endl;
		}

		if (this->listener)
			this->listener(watchid, dir, filename, action, oldFilename);

		if (this->boldLstener)
			this->boldLstener();
	}
};

void prepareConsole(void);
void printArgs(int argc, char** argv);

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
	
	prepareConsole();

	std::cout << colorize("Hello There!", { 0, 255, 0 }) << std::endl;

	printArgs(argc, argv);
	

	// check if help is required
	const bool help = [&]() -> bool {
		for (int i = 0; i < argc; ++i)
			if (string(argv[i]).starts_with("-h") || string(argv[i]).starts_with("--h"))
				return true;
		return false;
	}();

	if (help)
		return printHelp(), EXIT_SUCCESS;

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

		// loop forever
		while (true) {
			std::this_thread::yield();
		}
	}

	return EXIT_SUCCESS;

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

void printArgs(int argc, char** argv)
{
	using std::cout;
	using std::endl;
	using std::to_string;
	using lcdoc::colorize;
	using lcdoc::terminal_color_t;

	const terminal_color_t pvarColor = { 150, 150, 150 };
	const terminal_color_t numberColor = { 181, 206, 168 };
	const terminal_color_t stringColor = { 214, 157, 122 };
	
	for (int i = 0; i < argc; ++i)
		cout << colorize("argv", pvarColor) << "[" << colorize(to_string(i), numberColor) << "]: " << colorize(argv[i], stringColor) << endl;
}