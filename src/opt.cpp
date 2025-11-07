#include <vector>
#include <string>
#include <algorithm>

namespace opt {
	std::pair<int, int> min_pref_editdistance(const std::string& A, const std::string& B) {
		int m = (int)A.size(), n = (int)B.size();

		std::vector<int> ult(m + 1), agr(m + 1);
		for (int i = 0; i <= m; i++) {
			ult[i] = i;
		}

		std::pair<int, int> res = {ult[m], 0};

		for (int i = 1; i <= n; i++) {
			agr[0] = i;

			for (int j = 1; j <= m; j++) {
				int preco = A[j - 1] != B[i - 1];
				int del = ult[j] + 1;
				int ins = agr[j - 1] + 1;
				int sub = ult[j - 1] + preco;
				agr[j] = std::min({del, ins, sub});
			}

			res = std::min(res, std::pair<int, int>(agr[m], i));
			std::swap(ult, agr);
		}

		return res;
	}
}
