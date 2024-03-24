#pragma once

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

#include "global.h"

namespace cf {
	struct Rating {
		int positiveC = 0;
		int negativeC = 0;
	};

	std::ostream& operator<<(std::ostream& os, const Rating& c);

	struct Review {
		std::string reviewer;
		bool positive = false;
		//double score = 0.0;
	};

	//review json parsers
	void from_json(const json& json, Review& review);
	void to_json(json& json, const Review& review);

	//rating json parsers
	void from_json(const json& json, Rating& review);
	void to_json(json& json, const Rating& review);

	using MovieRatingsDb = std::unordered_map<std::string, Rating>;
	using MovieReviewsDb = std::unordered_map<std::string, std::vector<Review>>;
	using BlacklistedReviewersDb = std::unordered_set<std::string>;

	Rating calcRating(const std::vector<Review>& reviews, 
		const BlacklistedReviewersDb& blacklistedReviewers);

	Rating addMovieToDatabase(const std::string& movieLink, const std::string& movieName,
		MovieRatingsDb& ratingsDb, MovieReviewsDb& reviewDbs,
		const BlacklistedReviewersDb& blacklistedReviewers);

	void blacklistOnFreshness(const std::string& movieName, bool tomatoQuality, MovieRatingsDb& ratingsDb,
		const MovieReviewsDb& reviewsDb, BlacklistedReviewersDb& blacklistedReviewers);
}
