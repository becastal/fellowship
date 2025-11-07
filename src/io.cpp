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
		delete doc;
		
		//limpa_branco(R);
		return R;
	}

	std::vector<Box> le_pdf_boxes(const std::string& caminho) {
		poppler::document* doc = poppler::document::load_from_file(caminho);
		if (!doc) {
			std::cerr << "erro: deu bosta lendo pdf\n";
			delete doc;
			return {};
		}

		std::vector<Box> B;
		const int n = doc->pages();

		for (int i = 0; i < n; i++) {
			poppler::page* pag = doc->create_page(i);
			if (!pag) continue;

			std::vector<poppler::text_box> L = pag->text_list();
			for (auto& tb : L) {
				auto s_ = tb.text().to_utf8();
				std::string s(s_.begin(), s_.end());

				auto r = tb.bbox();
				Box b;

				b.pagina = i;
				b.x0 = (float)r.left();
				b.y0 = (float)r.top();
				b.x1 = (float)r.right();
				b.y1 = (float)r.bottom();
				b.S = std::move(s);

				B.push_back(std::move(b));
			}

			delete pag;
		}
		
		// merge das proximas lado a lado. nlogn.

		std::sort(B.begin(), B.end(), [](const Box& a, const Box& b) {
			if (a.pagina != b.pagina) return a.pagina < b.pagina;
			if (std::abs(a.y0 - b.y0) > 1e-3f) return a.y0 < b.y0;
			return a.x0 < b.x0;
		});

		std::vector<Box> R;

		const float Y_TOL = 3.0f;
		const float X_GAP_TOL = 10.0f;
		auto ycenter = [](const Box& b){ return 0.5 * (b.y0 + b.y1); };

		int m = B.size();
		for (int i = 0; i < m; ++i) {
			const Box& agr = B[i];

			if (!R.empty()) {
				Box& ult = R.back();

				bool same_page = (ult.pagina == agr.pagina);
				bool same_line = std::fabs(ycenter(agr) - ycenter(ult)) <= Y_TOL;
				bool close_x   = (agr.x0 <= ult.x1 + X_GAP_TOL);

				if (same_page and same_line and close_x) {
					ult.S.push_back(' ');
					ult.S += agr.S;
					ult.x0 = std::min(ult.x0, agr.x0);
					ult.y0 = std::min(ult.y0, agr.y0);
					ult.x1 = std::max(ult.x1, agr.x1);
					ult.y1 = std::max(ult.y1, agr.y1);
					continue;
				}
			}

			R.push_back(agr);
		}

		delete doc;
		return R;
	}
}
