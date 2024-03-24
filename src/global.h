#pragma once

#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

namespace cf {
	using nlohmann::json;

	template<typename T>
	T parse(const std::string& path) {
		std::ifstream file{ path };
		assert(file.is_open());
		auto json = json::parse(file);
		file.close();
		return json.get<T>();
	}

	template<typename T>
	void save(const std::string& path, const T& t) {
		std::ofstream file{ path };
		assert(file.is_open());
		json j = t;
		file << j.dump(2);
		file.close();
	}
}