#pragma once

// This file has policy classes for filtering paths.

#include <string>
#include <array>

namespace sbutils {
	struct NullPolicy {
		template <typename String> bool isValidStem(String &&) { return true; }
		template <typename String> bool isValidExt(String &&) { return true; }
	};

	class NormalPolicy {
	public:

		template<typename T>
		bool isValidExt(T &&anExtension) {
			return std::find(ExcludedExtensions.begin(), ExcludedExtensions.end(),
							 anExtension) == ExcludedExtensions.end();
		}

		template<typename T>
		bool isValidStem(T &&aStem) {
			return std::find(ExcludedStems.begin(), ExcludedStems.end(), aStem) ==
				ExcludedStems.end();
		}

	private:
		const std::array<std::string, 2> ExcludedExtensions = {{".git", ".cache"}};
		const std::array<std::string, 2> ExcludedStems = {{"CMakeFiles", "CMakeTmp"}};
	};
}
