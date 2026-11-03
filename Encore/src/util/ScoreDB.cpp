//
// Created by maria on 02/11/2026.
//

#include "ScoreDB.h"

#include "SQLiteCpp/Transaction.h"

void Encore::ScoreDB::Init(const std::filesystem::path &prefsPath) {
    db = SQLite::Database(
        prefsPath / "score.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE
    );
    db.exec(
        "CREATE TABLE IF NOT EXISTS scores (hash TEXT, score NUMERIC, playercount INTEGER, replayFilename TEXT, playerData TEXT)"
    );
}

void GetPlayerDataFromCSV(Encore::Score& score, const std::string& data) {
    std::stringstream sData(data);
    for (std::string row; std::getline(sData, row);) {
        if (row.starts_with("playerID,")) continue;
        std::stringstream sRow(row);
        std::string entry;
        std::getline(sRow, entry, ',');
        score.playerids.push_back(entry);
        std::getline(sRow, entry, ',');
        score.instrument.push_back(stoi(entry));
        std::getline(sRow, entry, ',');
        score.difficulty.push_back(stoi(entry));
        std::getline(sRow, entry);
        score.rating.push_back(stod(entry));
    }
};

std::vector<Encore::Score> Encore::ScoreDB::GetScores(SongHash &hash) {
    SQLite::Statement get(db, "SELECT * FROM scores WHERE hash = ?");
    std::string hashString(reinterpret_cast<char *>(hash.hash), picosha2::k_digest_size);
    get.bind(1, hashString);
    std::vector<Score> scores;
    while (get.executeStep())
    {
        Score score;
        score.score = get.getColumn(1);
        score.count = get.getColumn(2);
        score.replay = get.getColumn(3).getString();
        GetPlayerDataFromCSV(score, get.getColumn(4).getString());
        scores.push_back(score);
    }
    return scores;
}
std::vector<Encore::Score> Encore::ScoreDB::GetAllScores() {
    SQLite::Statement get(db, "SELECT * FROM scores");
    std::vector<Score> scores;
    while (get.executeStep())
    {
        Score score;
        score.score = get.getColumn(1);
        score.count = get.getColumn(2);
        score.replay = get.getColumn(3).getString();
        GetPlayerDataFromCSV(score, get.getColumn(4).getString());
        scores.push_back(score);
    }
    return scores;
}

bool Encore::ScoreDB::SetScore(SongHash &hash, Score score) {
    try {
        SQLite::Statement set(
            db, "INSERT INTO scores (hash, score, playercount, replayFilename, playerData) VALUES (?, ?, ?, ?, ?)"
        );

        std::string hashString(
            reinterpret_cast<char *>(hash.hash), picosha2::k_digest_size
        );
        set.bind(1, hashString);
        set.bind(2, score.score);
        set.bind(3, score.count);
        set.bind(4, score.replay);
        std::string uuids = "playerID,instrument,difficulty,rating\n";
        for (int i = 0; i < score.playerids.size(); i++) {
            int diff = score.difficulty.at(i);
            int inst = score.instrument.at(i);
            double rating = score.rating.at(i);
            uuids += score.playerids.at(i) + "," + std::to_string(inst) + "," + std::to_string(diff) + "," + std::to_string(rating) + "\n";
        }
        set.bind(5, uuids);
        set.exec();
    } catch (const SQLite::Exception &e) {
        Log::Error(e.what());
    }
    return true;
}