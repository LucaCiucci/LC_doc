
#include "string_utils.hpp"

namespace lcdoc
{
	nlohmann::json to_json(const YAML::Node& node)
	{
		if (!node.IsDefined())
			return nlohmann::json();

		if (node.IsNull())
			return nlohmann::json();

		if (node.IsScalar())
			return node.as<std::string>();

		if (node.IsSequence())
		{
			nlohmann::json j;
			for (const auto& child : node)
				j.push_back(to_json(child));
			return j;
		}

		if (node.IsMap())
		{
			nlohmann::json j;
			for (const auto& m : node)
				j[m.first.as<std::string>()] = to_json(m.second);
			return j;
		}

		assert(0);
	}
}