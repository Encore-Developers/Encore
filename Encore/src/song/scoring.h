//
// Created by maria on 24/11/2024.
//

#ifndef SCORING_H
#define SCORING_H
#include <vector>

/**
 * @brief Values for scoring in gameplay
 */

inline constexpr double BASE_NOTE_POINT = 25; /// default is 30
inline constexpr double SUSTAIN_POINTS_PER_BEAT = 12;
inline constexpr float OVERDRIVE_MULTIPLIER = 2.0f;
inline constexpr float PERFECT_MULTIPLIER = 1.2f;
inline constexpr float BASE_SCORE_NOTE_MULT = 1.1f; // for more balanced stars
inline constexpr double BASE_SCORE_NOTE_POINT = (BASE_NOTE_POINT * BASE_SCORE_NOTE_MULT) * 4;
inline constexpr double BASE_SCORE_SUSTAIN_POINTS = (SUSTAIN_POINTS_PER_BEAT * BASE_SCORE_NOTE_MULT) * 4;
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

#endif // SCORING_H
