#include "rater.h"

namespace cf {
	std::ostream& operator<<(std::ostream& os, const Rating& r) {
		os << r.positiveC << " / " << (r.positiveC + r.negativeC) << '\n';
		return os;
	}

	void from_json(const json& json, Review& review) {
		review.reviewer = json.at("reviewer").get<std::string>();
		review.positive = json.at("positive").get<bool>();
	}
	void to_json(json& json, const Review& review) {
		json["reviewer"] = review.reviewer;
		json["positive"] = review.positive;
	}

	//rating json parsers
	void from_json(const json& json, Rating& review) {
		review.positiveC = json.at("positiveC").get<int>();
		review.negativeC = json.at("negativeC").get<int>();
	}
	void to_json(json& json, const Rating& review) {
		json["positiveC"] = review.positiveC;
		json["negativeC"] = review.negativeC;
	}

	Rating calcRating(const std::vector<Review>& reviews, 
		const BlacklistedReviewersDb& blacklistedReviewers)
	{
		namespace rngs = std::ranges;

		int positiveReviewC = 0;
		int negativeReviewC = 0;

		for (const auto& [reviewer, tomato] : reviews) {
			if (!blacklistedReviewers.contains(reviewer)) {
				if (tomato) {
					positiveReviewC++;
				}
				else {
					negativeReviewC++;
				}
			}
			else {
				std::cout << "Ignoring " << reviewer << R"('s review)" << '\n';
			}
		}
		
		return Rating{ positiveReviewC, negativeReviewC };
	}

	Rating addMovieToDatabase(const std::string& movieLink, const std::string& movieName,
		MovieRatingsDb& ratingsDb, MovieReviewsDb& reviewsDb,
		const BlacklistedReviewersDb& blacklistedReviewers)
	{
		/*start python process to remote control firefox and
		print out all the review HTML of rotten tomatoes, then write all that text to a file*/
		const std::string tempHtmlFileName = "temp_src.txt";
		
		auto scriptCommand = "python python/rotten_tomatoes_requester.py " + movieLink;
		system(scriptCommand.c_str());
		
		//read in all the text 
		std::ifstream inputHtmlFile{ tempHtmlFileName };
		if (!inputHtmlFile.is_open()) {
			std::cerr << "Error: could not open: temp HTML file\n";
		}
		std::vector<Review> reviews;
		Review review;

		auto isSpaceOrUnicode = [](auto chr) {
			return (chr < -1 || chr > 255) || std::isspace(chr);
		};

		std::string line;
		while (std::getline(inputHtmlFile, line)) {
			if (line.contains("c-li")) {
				std::getline(inputHtmlFile, line); //skip to next line with critic name

				/*replace space in the middle of name with a dash, then remove all spaces in the std::string,
				then replace dash with space*/
				namespace rngs = std::ranges;
				auto letterPos = rngs::find_if(line, [](auto c) { return std::isalpha(c); }); //BROKEN WITH UNICODE
				auto spacePos = rngs::find_if(letterPos, line.end(), isSpaceOrUnicode);
				if (spacePos != line.end()) {
					*spacePos = '-';
					auto dashIdx = rngs::distance(letterPos, spacePos);
					std::erase_if(line, isSpaceOrUnicode);
					line[dashIdx] = ' ';
				} else {
					std::erase_if(line, isSpaceOrUnicode);
				}
				review.reviewer = line;
			} else {
				auto currIdx = line.find("state=");
				if (currIdx == std::string::npos) {
					continue;
				}
				currIdx += 7;
				switch (line[currIdx]) {
				case 'f': //"fresh"
					review.positive = true;
					break;
				case 'c': //"certified-fresh"
					review.positive = true;
					break;
				case 'r': //"rotten"
					review.positive = false;
					break;
				}
				if (!review.reviewer.empty()) {
					reviews.push_back(review);
				}
			}
		}
		inputHtmlFile.close();
		std::filesystem::remove(tempHtmlFileName);
		
		auto rating = calcRating(reviews, blacklistedReviewers);
		ratingsDb[movieName] = rating;
		reviewsDb[movieName] = reviews;
		return rating;
	}

	static Rating updateBlacklist(const std::vector<Review>& reviews, bool tomatoQuality, 
		BlacklistedReviewersDb& blacklistedReviewers) 
	{
		int positiveReviewC = 0;
		int negativeReviewC = 0;
		for (const auto& [reviewer, tomato] : reviews) {
			if (tomato == tomatoQuality) {
				blacklistedReviewers.insert(reviewer);
			} else if (tomato) {
				positiveReviewC++;
			} else {
				negativeReviewC++;
			}
		}
		return Rating{ positiveReviewC, negativeReviewC };
	}

	void blacklistOnFreshness(const std::string& movieName, bool tomatoQuality, MovieRatingsDb& ratingsDb, 
		const MovieReviewsDb& reviewsDb, BlacklistedReviewersDb& blacklistedReviewers) 
	{
		if (!reviewsDb.contains(movieName)) {
			std::cout << "Error: review-list does not contain " << movieName << '\n';
			std::cout << "It contains: \n";
			for (const auto& [movie, rating] : reviewsDb) {
				std::cout << movie << '\n';
			}
			return;
		}
		updateBlacklist(reviewsDb.at(movieName), tomatoQuality, blacklistedReviewers);

		for (const auto& [movieName, reviews] : reviewsDb) {
			auto newRating = calcRating(reviews, blacklistedReviewers);
			std::cout << "New rating of " << movieName << ": " << newRating << '\n';
			ratingsDb[movieName] = newRating;
		}
	}
}