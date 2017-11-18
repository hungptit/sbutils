#pragma once
namespace test {
	enum class CoverageType {STMT, METHOD, COND};
	struct Line {
		unsigned int num;
		unsigned int count;
		unsigned int truecount;
		unsigend int falsecout;
		CoverageType type;
	};

	struct Metrics {
		unsigned int elements;
		unsigned int covered_elements;
		unsigned int statements;
		unsigned int covered_statements;
		unsigned int loc;
		unsigned int ncloc;		// ??
		unsigned int conditional;
		unsigned int methods;
		unsigned int covered_methods;
		bool complexity;
		std::string name;
	};

	struct SourceClass {
		std::string name;
		Metrics metrics;
	};

	struct SourceFile {
		std::string path;
		std::string name;
		Metrics metrics;
		SourceClass source_class;
		std::vector<Line> lines;
	};

	struct Package {
		std::string name;
		Metrics metrics;
		std::vector<SourceFile> files;
	};

	struct Project {
		std::string name;
		std::string timestamp;
		Metrics metrics;
		std::vector<Package> packages;
	};
}
