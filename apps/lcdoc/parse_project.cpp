
#include <yaml-cpp/yaml.h>

#include <glob/glob.h>

#include "parse_project.hpp"

using std::string;

namespace lcdoc
{
	namespace
	{
		bool nonEmptyString(const YAML::Node& node)
		{
			if (!node.IsDefined())
				return false;

			if (!node.IsScalar())
				return false;

			return !node.as<string>().empty();
		}

		bool isSequence(const YAML::Node& node)
		{
			if (!node.IsDefined())
				return false;

			return node.IsSequence();
		}
		
		void parseLcdocCompilationOptions(const YAML::Node& yaml, lcdoc::CXXFileOptions& options)
		{
			// standard
			if (nonEmptyString(yaml["standard"]))
				options.standard = yaml["standard"].as<string>();

			// additional include dirs
			if (nonEmptyString(yaml["includeDirs"]))
				for (const auto& dir : yaml["includeDirs"])
					if (nonEmptyString(dir))
						options.additionalIncludeDirs.insert(dir.as<string>());

			// additionalFlags
			if (isSequence(yaml["additionalFlags"]))
				for (const auto& flag : yaml["additionalFlags"])
					if (nonEmptyString(flag))
						options.additionalOptions.push_back(flag.as<string>());

			// defines
			if (isSequence(yaml["defines"]))
				for (const auto& define : yaml["defines"])
				{
					// string, example:
					// - SOME_PREPROCESSOR_DEFINITION
					if (nonEmptyString(define))
						options.additionalDefinitions[define.as<string>()] = "";
					
					// map
					// - SOME_PREPROCESSOR_DEFINITION: 42
					if (define.IsMap() && define.size() == 1)
					{
						const auto [key, value] = (pair<YAML::Node, YAML::Node>)*define.begin();
						if (nonEmptyString(key))
							options.additionalDefinitions[key.as<string>()] = value.IsScalar() ? value.as<string>() : "";
					}
				}

			// undefines
			if (isSequence(yaml["undefines"]))
				for (const auto& undefine : yaml["undefines"])
					if (nonEmptyString(undefine))
						options.undefines.insert(undefine.as<string>());
		}
	}

	void parseLcdocProject(const std::shared_ptr<CXXProject>& project, const path& projectFile)
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
}