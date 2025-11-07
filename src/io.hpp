#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <poppler-document.h>
#include <poppler-page.h>

struct Query {
	std::string label;
	nlohmann::json extraction_schema;
	std::string pdf_path;
};

struct Box {
	int pagina;
	float x0, y0, x1, y1;
	std::string S;
};

namespace io {

	std::vector<Query> inputa(const std::string& path);
	void outputa(const std::string& path, const nlohmann::json& j);
	std::string le_pdf(const std::string& caminho);
	std::vector<Box> le_pdf_boxes(const std::string& caminho);
}
