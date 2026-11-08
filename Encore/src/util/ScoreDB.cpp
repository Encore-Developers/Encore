//
// Created by maria on 02/11/2026.
//

#include "ScoreDB.h"

#include "SQLiteCpp/Transaction.h"

Encore::PlayerScore::PlayerScore(std::string data) {
    std::stringstream sRow(data);
    std::string entry;
    std::getline(sRow, entry, ',');
    playerid = entry;
    std::getline(sRow, entry, ',');
    score = stoi(entry);
    std::getline(sRow, entry, ',');
    instrument = stoi(entry);
    std::getline(sRow, entry, ',');
    difficulty = stoi(entry);
    std::getline(sRow, entry);
    rating = stod(entry);
}

Encore::PlayerScore::PlayerScore(const Player *player) {
    playerid = player->PlayerID;
    score = player->engine->stats->Score;
    instrument = player->Instrument;
    difficulty = player->Difficulty;
    rating = player->engine->stats->Accuracy / player->engine->stats->AttemptedNotes;
}

std::string Encore::PlayerScore::toString() {
    return playerid + "," + std::to_string(score) + "," + std::to_string(instrument) + ","
        + std::to_string(difficulty) + "," + std::to_string(rating) + "\n";
}

void Encore::ScoreDB::Init(const std::filesystem::path &prefsPath) {
    db = SQLite::Database(
        prefsPath / "score.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE
    );
    db.exec(
        "CREATE TABLE IF NOT EXISTS scores (hash TEXT, score NUMERIC, playercount INTEGER, replayFilename TEXT, playerData TEXT)"
    );
}

Encore::Score Encore::ScoreDB::CreateScoreFromStatement(SQLite::Statement &stmt) {
    Score score;
    score.score = stmt.getColumn(1);
    score.count = stmt.getColumn(2);
    score.replay = stmt.getColumn(3).getString();
    std::stringstream sData(stmt.getColumn(4).getString());
    for (std::string row; std::getline(sData, row);) {
        score.players.emplace_back(row);
    }
    return score;
}

std::vector<Encore::Score> Encore::ScoreDB::GetScores(SongHash &hash) const {
    SQLite::Statement get(db, "SELECT * FROM scores WHERE hash = ?");
    std::string hashString(reinterpret_cast<char *>(hash.hash), picosha2::k_digest_size);
    get.bind(1, hashString);
    std::vector<Score> scores;
    while (get.executeStep()) {
        scores.push_back(CreateScoreFromStatement(get));
    }
    return scores;
}
std::vector<Encore::Score> Encore::ScoreDB::GetAllScores() const {
    SQLite::Statement get(db, "SELECT * FROM scores");
    std::vector<Score> scores;
    while (get.executeStep()) {
        scores.push_back(CreateScoreFromStatement(get));
    }
    return scores;
}

bool Encore::ScoreDB::SetScore(SongHash &hash, const Score& score) const {
    try {
        SQLite::Statement set(
            db,
            "INSERT INTO scores (hash, score, playercount, replayFilename, playerData) VALUES (?, ?, ?, ?, ?)"
        );

        const std::string hashString(
            reinterpret_cast<char *>(hash.hash), picosha2::k_digest_size
        );
        set.bind(1, hashString);
        set.bind(2, score.score);
        set.bind(3, score.count);
        set.bind(4, score.replay);
        std::string uuids;

        for (auto player : score.players) {
            uuids += player.toString();
        }
        set.bind(5, uuids);
        set.exec();
    } catch (const SQLite::Exception &e) {
        Log::Error(e.what());
    }
    return true;
}