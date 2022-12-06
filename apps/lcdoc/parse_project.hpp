#pragma once

#include <memory>
#include <filesystem>

#include "Project.hpp"

namespace lcdoc
{
	using std::shared_ptr;
	using std::filesystem::path;

	void parseLcdocProject(const std::shared_ptr<CXXProject>& project, const path& projectFile);
}