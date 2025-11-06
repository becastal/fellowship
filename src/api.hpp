#pragma once
#include "io.hpp"
#include <string>
#include <nlohmann/json.hpp>

namespace api {
	std::string post(const std::string& url, const std::string& chave, const nlohmann::json& conteudo);
	nlohmann::json formata_schema(const nlohmann::json& extraction_schema);
	nlohmann::json formata_nlohmann(const std::string& label, const nlohmann::json& extraction_schema, const std::string& texto_pdf);
	nlohmann::json formata_res(const std::string& S, const nlohmann::json& extraction_schema);
}
