#pragma once
/*!
 * Modifyable values to change how the hitwindow feels.
 * All times are set in seconds
 */

constexpr float goodFrontend = 0.075f; /**< Size of the "good" (lower point) early/positive hitwindow */
constexpr float goodBackend = 0.075f; /**< Size of the "good" (lower point) late/negative hitwindow */
constexpr float perfectFrontend = 0.025f; /**< Size of the "perfect" (higher point) early/positive hitwindow */
constexpr float perfectBackend = 0.025f; /**< Size of the "perfect" (higher point) late/negative hitwindow */

constexpr float hopoFrontend = 0.025f; /**< Allows the player to hold a fret earlier than expected and still hit a hammer on. Is added on top of the goodFrontend variable */
constexpr float hopoBackend = 0;
constexpr float fretAfterStrumTime = 0.05f; /**< Allows the player to strum earlier than expected and still hit a note. */

constexpr float liftLeniencyTime = 0.1f; /**< Allows the player to strike right after hitting a lift without penalty. */

constexpr float healthLossPerNote = 0.03f;
constexpr float healthGainPerNote = 0.015f;
constexpr float healthLossSustainDrop = 0.01f;
constexpr float healthOverdriveGainMult = 3.0f;