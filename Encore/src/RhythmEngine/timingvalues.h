#pragma once
#include "timetypes.h"

/*!
 * Modifyable values to change how the hitwindow feels.
 * All times are set in seconds
 */

constexpr TimeSeconds goodFrontend = 0.075; /**< Size of the "good" (lower point) early/positive hitwindow */
constexpr TimeSeconds goodBackend = 0.075; /**< Size of the "good" (lower point) late/negative hitwindow */
constexpr TimeSeconds perfectFrontend = 0.025; /**< Size of the "perfect" (higher point) early/positive hitwindow */
constexpr TimeSeconds perfectBackend = 0.025; /**< Size of the "perfect" (higher point) late/negative hitwindow */

constexpr TimeSeconds hopoFrontend = 0.025; /**< Allows the player to hold a fret earlier than expected and still hit a hammer on. Is added on top of the goodFrontend variable */
constexpr TimeSeconds hopoBackend = 0;
constexpr TimeSeconds fretAfterStrumTime = 0.05; /**< Allows the player to strum earlier than expected and still hit a note. */

constexpr TimeSeconds liftLeniencyTime = 0.1; /**< Allows the player to strike right after hitting a lift without penalty. */
constexpr TimeSeconds overdriveHitLeniency = 0.1; /**< Allows the player to strike right after activation overdrive without penalty */

constexpr TimeSeconds dynamicHitwindowRatio = 1.25;
constexpr TimeSeconds minimumHitwindowSize = 0.015;

constexpr TimeODTicks whammyHoldTime = 0.5;
constexpr double whammyGainPerTick = 1 / (double)30;

constexpr float healthLossPerNote = 0.03f;
constexpr float healthGainPerNote = 0.015f;
constexpr float healthLossSustainDrop = 0.01f;
constexpr float healthOverdriveGainMult = 2.5f;
constexpr float healthOverdriveLossMult = 2.0f;
constexpr double defaultHealth = 0.75f;
#define SUSTAIN_DROP_THRESHOLD 240
#define NOTE_POOL_SIZE 100