#include <iostream>
#include <chrono>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include "io.hpp"
#include "api.hpp"
#include "opt.hpp"

using Clock = std::chrono::steady_clock;
long long ms_since(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(b - a).count();
}

int main(int argc, char** argv) {
	if (argc < 3) {
		std::cerr << "erro: uso eh ./enter {caminho_dataset.json} {caminho_files}\n";
		return(1);
	}
	const char* chave = std::getenv("OPENAI_API_KEY");
	if (!chave) {
		std::cerr << "erro: defina OPENAI_API_KEY. use: export OPENAI_API_KEY=\"{sua_chave}\"\n";
		return(1);
	}

	std::string caminho_in = argv[1];
	std::string caminho_files = argv[2];

	auto Q = io::inputa(caminho_in);
	nlohmann::json R = nlohmann::json::array();
	
	for (int i = 0; i < (int)Q.size(); i++) {
		auto& q = Q[i];

		std::vector<Box> A = io::le_pdf_boxes(caminho_files + "/" + q.pdf_path);

		// previsao de possiveis respostas com minimo edit distance de prefixo pra cada text_box
		const int LIM = 3; // maxima edit distance aceita pra considerar prefixo util
		nlohmann::json schema_ = q.extraction_schema;

		for (auto it = schema_.begin(); it != schema_.end(); ++it) {
			if (!it->is_string()) continue;

			std::string padrao = it->get<std::string>();
			std::unordered_set<std::string> visto;
			std::vector<std::string> cand;

			for (auto& a : A) {
				auto [dist, pos] = opt::min_pref_editdistance(padrao, a.S);
				if (dist < LIM) {
					if ((size_t)pos < a.S.size()) {
						std::string suff(a.S.begin() + pos, a.S.end());
						if (suff.size() >= 2 && !visto.count(suff)) {
							visto.insert(suff);
							cand.push_back(std::move(suff));
							if ((int)cand.size() >= 5) break;
						}
					}
				}
			}

			if (!cand.empty()) {
				std::string conc;
				for (int k = 0; k < (int)cand.size(); k++) {
					if (k) conc += " | ";
					conc += cand[k];
				}
				std::string v = padrao;
				v += "\npossiveis respostas: ";
				v += conc;
				*it = std::move(v);
			}
		}

		auto t0 = Clock::now();
		std::string texto = io::le_pdf(caminho_files + "/" + q.pdf_path);
		nlohmann::json conteudo = api::formata_nlohmann(q.label, schema_, texto);	

		std::string res = api::post("https://api.openai.com/v1/responses", chave, conteudo);
		nlohmann::json Re = api::formata_res(res, schema_);
		
		int cont = 0, ok = 0;
		for (auto& [a, b] : Re.items()) {
			cont ++, ok += !b.is_null();
		}
		auto t1 = Clock::now();

		std::cout << "teste " << i << ": res (" << ok << "/" << cont << "),"
				  << " duracao: " << ms_since(t0, t1) << "ms \n"; 

		R.push_back(Re);
	}
	io::outputa("outputs.json", R);

	return(0);
}
