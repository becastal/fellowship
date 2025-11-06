#include "api.hpp"
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <vector>


namespace api {
	static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
		auto* out = static_cast<std::string*>(userdata);
		out->append(ptr, size * nmemb);
		return size * nmemb;
	}

	std::string post(const std::string& url, const std::string& chave, const nlohmann::json& conteudo) {
		std::string R;

		CURL* curl = curl_easy_init();
		if (!curl) return {};

		struct curl_slist* headers = nullptr;
		std::string auth = "Authorization: Bearer " + chave;
		headers = curl_slist_append(headers, "Content-Type: application/json");
		headers = curl_slist_append(headers, "Accept: application/json");
		headers = curl_slist_append(headers, auth.c_str());

		const std::string body = conteudo.dump();

		char errbuf[CURL_ERROR_SIZE] = {0};
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &R);

		curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 30L);
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 15L);

		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 33L); // TODO: ver se eh ok pra 10s tle 33s porque gabriel jesus

		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);

		CURLcode res = curl_easy_perform(curl);

		long http_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

		if (res != CURLE_OK) {
			if (errbuf[0]) std::cerr << "[curl] " << errbuf << "\n";
			else std::cerr << "[curl] " << curl_easy_strerror(res) << "\n";
		}
		if (http_code and (http_code < 200 or http_code >= 300)) {
			std::cerr << "[http] status: " << http_code << "\n";
		}

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);

		return R;
	}

	nlohmann::json formata_res(const std::string& S, const nlohmann::json& extraction_schema) {
		auto nulls = [&]{
			nlohmann::json j = nlohmann::json::object();
			for (const auto& [k, _]: extraction_schema.items()) j[k] = nullptr;
			return j;
		};
		if (S.empty()) return nulls();

		nlohmann::json r = nlohmann::json::parse(S, nullptr, false);
		if (r.is_discarded()) return nulls();

		if (r.contains("output_parsed") and r["output_parsed"].is_array()
				and !r["output_parsed"].empty() and r["output_parsed"][0].is_object()) {
			return r["output_parsed"][0];
		}

		if (r.contains("output") and r["output"].is_array()) {
			for (const auto& item : r["output"]) {
				if (item.contains("content") and item["content"].is_array()) {
					for (const auto& c : item["content"]) {
						if (c.contains("parsed") and c["parsed"].is_object()) return c["parsed"];
						if (c.contains("text") and c["text"].is_string()) {
							auto s = c["text"].get<std::string>();
							auto j = nlohmann::json::parse(s, nullptr, false);
							if (j.is_object()) return j;
						}
					}
				}
			}
		}
		return nulls();
	}

	nlohmann::json formata_schema(const nlohmann::json& extraction_schema) {
		nlohmann::json properties = nlohmann::json::object();
		std::vector<std::string> required;

		for (const auto& [nome, desc_] : extraction_schema.items()) {
			const std::string desc = desc_.is_string() ? desc_.get<std::string>() : "";

			nlohmann::json f;
			f["anyOf"] = {
				nlohmann::json{{"type","string"}},
				nlohmann::json{{"type","null"}}
			};
			if (!desc.empty()) f["description"] = desc;

			properties[nome] = f;
			required.push_back(nome);
		}

		nlohmann::json schema;
		schema["name"] = "extracted_fields";
		schema["strict"] = true;
		schema["schema"] = {
			{"type","object"},
			{"properties",properties},
			{"required",required},
			{"additionalProperties",false}
		};

		return schema;
	}

	nlohmann::json formata_nlohmann(const std::string& label, const nlohmann::json& extraction_schema, const std::string& texto_pdf) {
		std::string prompt = "Extract fields per the JSON schema."
		"Use exactly the schema keys; use null if missing.\n\n" 
		"Label: " + label + "\n"
		"PDF TEXT:\n" + texto_pdf + "\n";

		nlohmann::json js = formata_schema(extraction_schema);

		nlohmann::json conteudo;
		conteudo["model"] = "gpt-5-mini";
		conteudo["input"] = prompt;

		conteudo["text"] = {
			{ "format", {
							{ "type",   "json_schema" },
							{ "name",   js["name"]   },
							{ "schema", js["schema"] },
							{ "strict", js["strict"] }
						}
			}
		};
		conteudo["max_output_tokens"] = 1024;

		return conteudo;
	}
}

