#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace io {
	struct Query {
		std::string label;
		nlohmann::json extraction_schema;
		std::string pdf_path;
	};

	std::vector<Query> inputa(const std::string& path);
	void outputa(const std::string& path, const nlohmann::json& j);
	std::string le_pdf(const std::string& caminho);
}
