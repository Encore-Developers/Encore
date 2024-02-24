#include "BASS/bass.h"

bool audioInit() {
	return BASS_Init(-1, 44100, 0, 0, NULL);
}