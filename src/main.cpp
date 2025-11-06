#include <iostream>
#include <chrono>
#include <nlohmann/json.hpp>
#include "io.hpp"
#include "api.hpp"

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
		std::cerr << "erro: defina OPENAI_API_KEY\n";
		return(1);
	}

	std::string caminho_in = argv[1];
	std::string caminho_files = argv[2];

	auto Q = io::inputa(caminho_in);
	nlohmann::json R = nlohmann::json::array();
	
	for (int i = 0; i < (int)Q.size(); i++) {
		auto& q = Q[i];

		auto t0 = Clock::now();
		std::string texto = io::le_pdf(caminho_files + "/" + q.pdf_path);
		nlohmann::json conteudo = api::formata_nlohmann(q.label, q.extraction_schema, texto);	

		std::string res = api::post("https://api.openai.com/v1/responses", chave, conteudo);
		nlohmann::json Re = api::formata_res(res, q.extraction_schema);
		
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
