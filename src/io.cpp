#include "io.hpp"

#include <fstream>
#include <simdjson.h>
#include <nlohmann/json.hpp>
#include <poppler-document.h>
#include <poppler-page.h>

namespace io {
	nlohmann::json to_nlohmann(simdjson::ondemand::value v) {
		auto S = simdjson::to_json_string(v);
		return nlohmann::json::parse(std::string(S.value()));
	}

	std::vector<Query> inputa(const std::string& caminho) {
		simdjson::ondemand::parser parser;

		simdjson::padded_string S = simdjson::padded_string::load(caminho);

		auto D = parser.iterate(S);
		auto A = D.get_array();

		std::vector<Query> R;

		for (auto v : A) {
			auto obj = v.get_object();

			Query q;
			q.label = std::string(obj["label"].get_string().value_unsafe());
			q.pdf_path = std::string(obj["pdf_path"].get_string().value_unsafe());

			q.extraction_schema = to_nlohmann(obj["extraction_schema"].value());
			R.push_back(q);
		}

		return R;
	}

	void outputa(const std::string& caminho, const nlohmann::json& out) {
		std::ofstream O(caminho, std::ios::binary);
		O << out.dump(2);

	}

	void limpa_branco(std::string& S) {
		std::string R; R.reserve(S.size());
		int n = (int)S.size();
		char C[4] = {' ', ' ', '\t', '\n'};

		auto branco = [](char c) {
			return c == '\n' ? 3 : c == '\t' ? 2 : c == ' ' ? 1 : 0;
		};
		for (int l = 0, r; l < n; l = r) {
			r = l + 1;
			if (!branco(S[l])) {
				R.push_back(S[l]);
				continue;
			}

			int agr = 0, c;
			while (r < n and (c = branco(S[r]))) {
				r++;
				agr = std::max(agr, c);
			}
			R.push_back(C[agr]);
		}

		S.swap(R);
	}

	std::string le_pdf(const std::string& caminho) {
		poppler::document* doc = poppler::document::load_from_file(caminho);

		if (!doc) {
			std::cerr << "erro: deu bosta lendo pdf\n";
			delete doc;
			return {};
		}

		std::string R;
		int n = doc->pages();

		for (int i = 0; i < n; i++) {
			poppler::page* pag = doc->create_page(i);
			if (!pag) continue;

			auto bytes = pag->text(pag->page_rect()).to_utf8();
			R.append(bytes.begin(), bytes.end());
			if (i < n - 1) R.push_back('\n');
			delete pag;
		}
		
		//limpa_branco(R);
		return R;
	}
}
