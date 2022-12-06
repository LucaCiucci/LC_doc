
#include <iostream>
#include <fstream>
#include <filesystem>

#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

using std::string;
using std::ifstream;
using std::ofstream;
using std::filesystem::path;
using std::exception;
using std::runtime_error;

using nlohmann::json;

json to_json(const YAML::Node& node)
{
	json j;
	
	switch (node.Type())
	{
	case YAML::NodeType::Null:
		j = nullptr;
		break;
	case YAML::NodeType::Scalar:
		j = node.as<string>();
		break;
	case YAML::NodeType::Sequence:
		for (const auto& item : node)
			j.push_back(to_json(item));
		break;
	case YAML::NodeType::Map:
		for (const auto& entry : node)
			j[entry.first.as<string>()] = to_json(entry.second);
		break;
	case YAML::NodeType::Undefined:
		throw runtime_error("Undefined YAML node type");
	}

	// if it is a string and it is a number, convert it to a number
	// TODO better
	if (j.is_string() && j.get<string>().length() > 0)
	{
		string s = j.get<string>();
		if (s.find_first_not_of("+-0123456789") == string::npos)
			j = std::stoi(s);
		else if (s.find_first_not_of("+-0123456789.eEfFdD") == string::npos && s.find_first_of("0123456789") != string::npos)
		{
			std::cout << s << std::endl;
			j = std::stod(s);
		}
	}

	// true / false
	if (j.is_string())
	{
		string s = j.get<string>();
		if (s == "true")
			j = true;
		else if (s == "false")
			j = false;
	}
	
	return j;
}

YAML::Node to_yaml(const json& j)
{
	YAML::Node node;

	switch (j.type())
	{
	case json::value_t::null:
		node = YAML::Node();
		break;
	case json::value_t::string:
		node = j.get<string>();
		break;
	case json::value_t::number_integer:
		node = j.get<int>();
		break;
	case json::value_t::number_unsigned:
		node = j.get<unsigned int>();
		break;
	case json::value_t::number_float:
		node = j.get<double>();
		break;
	case json::value_t::boolean:
		node = j.get<bool>();
		break;
	case json::value_t::array:
		for (const auto& item : j)
			node.push_back(to_yaml(item));
		break;
	case json::value_t::object:
		for (const auto& entry : j.items())
			node[entry.key()] = to_yaml(entry.value());
		break;
	case json::value_t::discarded:
		throw runtime_error("Discarded JSON value");
	}

	return node;
}

void printHelp()
{
	std::cout << "yamljson [options] <yaml-file> <json-file>" << std::endl;
}

int safe_main(int argc, char** argv)
{
	if (argc <= 1)
	{
		printHelp();
		return 0;
	}
	
	string arg1 = argv[1];
	
	if (arg1 == "-h" || arg1 == "--help")
	{
		printHelp();
		return 0;
	}

	if (arg1 == "-v" || arg1 == "--version")
	{
		std::cout << "yamljson 0.0.0" << std::endl;
		return 0;
	}

	const path inputPath = arg1;

	if (!std::filesystem::exists(inputPath))
		throw runtime_error("Input file does not exist.");

	if (inputPath.extension() == ".yaml" || inputPath.extension() == ".yml")
	{
		YAML::Node node = YAML::LoadFile(inputPath.string());
		json j = to_json(node);
		if (argc > 2)
		{
			const path outputPath = argv[2];
			ofstream output(outputPath);
			output << j.dump(4);
		}
		else
			std::cout << j.dump(4) << std::endl;
	}
	else if (inputPath.extension() == ".json")
	{
		ifstream ifs(inputPath);
		json j = json::parse(ifs);
		const auto yaml = to_yaml(j);
		if (argc > 2)
		{
			const path outputPath = argv[2];
			ofstream ofs(outputPath);
			ofs << yaml;
		}
		else
			std::cout << yaml << std::endl;
	}
	else
	{
		throw runtime_error("Input file must be a .yaml or .json file.");
	}
	
	
	return 0;
}

int main(int argc, char** argv)
{
	try
	{
		return safe_main(argc, argv);
	}
	catch (const exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	
	return 0;
}