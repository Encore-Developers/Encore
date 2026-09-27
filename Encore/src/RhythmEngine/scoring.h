//
// Created by maria on 24/11/2024.
//

#ifndef SCORING_H
#define SCORING_H
#include "raylib.h"
#include "raymath.h"

#include <vector>
#include <array>

/**
 * @brief Values for scoring in gameplay
 */

inline constexpr double BASE_NOTE_POINT = 25; /// default is 30
inline constexpr double SUSTAIN_POINTS_PER_BEAT = 12;
inline constexpr float OVERDRIVE_MULTIPLIER = 2.0f;
inline constexpr float PERFECT_MULTIPLIER = 1.2f;
inline constexpr float BASE_SCORE_NOTE_MULT = 1.175f; // for more balanced stars
inline constexpr double BASE_SCORE_NOTE_POINT = (BASE_NOTE_POINT * BASE_SCORE_NOTE_MULT);
inline constexpr double BASE_SCORE_SUSTAIN_POINTS = (SUSTAIN_POINTS_PER_BEAT * BASE_SCORE_NOTE_MULT);
inline constexpr float CYMBAL_MULTIPLIER = 1.25f;
inline constexpr float OVERDRIVE_QUARTER_BAR = 0.25f;
inline constexpr double OVERDRIVE_DRAIN_PER_BEAT = 0.03125;

inline constexpr float STAR_THRESHOLDS[5][6] = {
    { 0.06f, 0.12f, 0.20f, 0.45f, 0.75f, 1.09f }, // Drums
    { 0.05f, 0.10f, 0.19f, 0.47f, 0.78f, 1.15f }, // Bass
    { 0.06f, 0.12f, 0.20f, 0.47f, 0.78f, 1.15f }, // Guitar
    { 0.06f, 0.12f, 0.20f, 0.47f, 0.78f, 1.15f }, // Keys
    { 0.05f, 0.11f, 0.19f, 0.46f, 0.77f, 1.06f } // Vocals
};

inline constexpr float BAND_STAR_THRESHOLD[6] =
    { 0.06f, 0.12f, 0.20f, 0.47f, 0.78f, 1.15f };

struct Grade {
    struct Range {
        double bottom;
        double top;
    };
    Color color = WHITE;
    const char *Letter;
    Range range = {0,0};
    explicit Grade(const Color _color, const char _letter[1], const Range _range)
        : color(_color), Letter(_letter), range(_range) {}

    /// 0 is lower (-), 1 is normal, 2 is upper (+)
    [[nodiscard]] int GetSubdiv(double acc) const {
        if (acc >= range.top - (range.top - range.bottom) * 0.3) return 2;
        if (acc < range.bottom + (range.bottom - range.top) * 0.3) return 0;
        return 1;
    }

    /// Returns 0 at the bottom of the grade's range, 1 at the top
    float GetFraction(double acc) const {
        if (range.bottom == range.top) {
            return acc == range.bottom;
        }
        return Remap(acc, range.bottom, range.top, 0, 1);
    }
};

extern std::array<Grade, 7> Grades;
/// Returns the index of the grade, grade param is set to pointer of the grade (can be null)
int GetGrade(double acc, Grade** grade);

#endif // SCORING_H
