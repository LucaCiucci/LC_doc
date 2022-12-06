#pragma once

#include <string>
#include <format>

namespace lcdoc
{
	using std::string;

	struct terminal_color_t {
		uint8_t r = 255;
		uint8_t g = 255;
		uint8_t b = 255;
	};

	namespace terminal {
		inline constexpr terminal_color_t RED = terminal_color_t{ .r = 255, .g = 0, .b = 0 };
	}

	string colorize(const std::string& str, const terminal_color_t& color, bool enable = true) {
		if (!enable)
			return str;

		return std::format("\x1b[38;2;{1};{2};{3}m{0}\x1b[0m", str, color.r, color.g, color.b);
	}
}