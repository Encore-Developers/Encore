#pragma once
//
// Created by maria on 02/11/2026.
//
#include "SQLiteCpp/Database.h"
#include "song/song.h"
#include "users/player.h"

#include <filesystem>


// TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
// Scores need to be split entirely, this csv bullshit Does Not Work correctly
// When saving a score, each player gets a score in one table, and 1 band score gets
// saved to a different table. The player scores should simply refer to the band score
// with an ID and then we use a query to go from band -> player scores

namespace Encore {
    struct PlayerScore {
        PlayerScore(std::string data);
        PlayerScore(const Player *player);
        std::string playerid;
        double score;
        int instrument;
        int difficulty;
        double rating;
        std::string toString();
    };
    struct Score {
        double score;
        int count;
        std::string replay;
        std::vector<PlayerScore> players;
    };
    class ScoreDB {
        SQLite::Database db = nullptr;
        static Score CreateScoreFromStatement(SQLite::Statement& stmt);
    public:
        ScoreDB() = default;
        void Init(const std::filesystem::path& prefsPath);
        std::vector<Score> GetScores(SongHash &hash) const;
        std::vector<Score> GetAllScores() const;

        bool SetScore(SongHash &hash, const Score& score) const;
    };

};
extern Encore::ScoreDB TheScoreDatabase;
