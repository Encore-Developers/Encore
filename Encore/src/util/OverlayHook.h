#pragma once
// overlayHook.h
// Declares a single function that sends an od state message to
// CHEAPOverlay. Only the declaration lives here (no networking headers), so
// this can be safely #included anywhere without dragging Windows socket headers
// into the rest of the project. The actual implementation is in overlayHook.cpp.
//
//   player = which player (0 = first/only player)
//   sp     = true when overdrive turns ON, false when it turns OFF
//   fill   = overdrive meter, 0.0 to 1.0 (extra info for the overlay)

void OverlayEmit(int player, bool sp, double fill);