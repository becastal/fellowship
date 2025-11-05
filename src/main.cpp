#include <iostream>
#include "io.hpp"

int main(int argc, char** argv) {
	if (argc < 3) {
		std::cerr << "erro: uso eh ./enter {caminho_dataset.json} {caminho_files}\n";
		return(1);
	}
	std::string caminho_in = argv[1];
	std::string caminho_files = argv[2];

	auto Q = io::inputa(caminho_in);
	
	for (auto q : Q) {
		std::cout << q.label << ": " << q.pdf_path << '\n';
		std::string pdf = io::le_pdf(caminho_files + "/" + q.pdf_path);
		std::cout << pdf << '\n';
		std::cout << std::string(100, '-')  << '\n';
	}

	return(0);
}
