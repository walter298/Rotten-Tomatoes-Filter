#include "rater.h"

int main() {
	const std::string ratingsDbFname = "database.txt";
	const std::string reviewsDbFname = "movie_reviews.txt";
	const std::string blacklistedReviewsDbFname = "blacklisted_reviewers.txt";

	try {
		auto ratingsDb = cf::parse<cf::MovieRatingsDb>(ratingsDbFname);
		auto reviewsDb = cf::parse<cf::MovieReviewsDb>(reviewsDbFname);
		auto blacklistedReviewersDb = cf::parse<cf::BlacklistedReviewersDb>(blacklistedReviewsDbFname);

		while (true) {
			std::string input;
			std::cin >> input;

			if (input == "save") {
				break;
			} else if (input == "blacklist_reviewers_of") {
				std::cout << "Movie name and positive/negative: ";
				std::string movieName;
				std::string positive;
				std::cin >> movieName >> positive;
				bool tomatoQuality = (positive == "positive");
				cf::blacklistOnFreshness(movieName, tomatoQuality, ratingsDb, reviewsDb, blacklistedReviewersDb);
				continue;
			}

			auto movieRating = ratingsDb.find(input);
			if (movieRating == ratingsDb.end()) {
				std::cout << "Movie not found in database. Please type in the link to the rotten tomatoes reviews page: ";
				std::string movieLink;
				std::cin >> movieLink;

				std::cout << cf::addMovieToDatabase(movieLink, input, ratingsDb, reviewsDb, blacklistedReviewersDb) << '\n';
			}
			else {
				std::cout << movieRating->second << '\n';
			}
		}

		cf::save(ratingsDbFname, ratingsDb);
		cf::save(reviewsDbFname, reviewsDb);
		cf::save(blacklistedReviewsDbFname, blacklistedReviewersDb);
	} catch (nlohmann::json::exception e) {
		std::cout << e.what() << '\n';
	}
}