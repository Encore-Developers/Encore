#pragma once
//
// Created by maria on 02/11/2026.
//
#include "SQLiteCpp/Database.h"
#include "song/song.h"

#include <filesystem>

namespace Encore {
    struct Score {
        double score;
        int count;
        std::string replay;
        std::vector<std::string> playerids;
        std::vector<int> instrument;
        std::vector<int> difficulty;
        std::vector<double> rating;
    };
    class ScoreDB {
        SQLite::Database db = nullptr;
    public:
        ScoreDB() = default;
        void Init(const std::filesystem::path& prefsPath);
        std::vector<Encore::Score> GetScores(SongHash &hash);
        std::vector<Encore::Score> GetAllScores();

        bool SetScore(SongHash &hash, Score score);
    };

};
extern Encore::ScoreDB TheScoreDatabase;
