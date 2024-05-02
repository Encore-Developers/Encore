

#include "player.h"

int Player::instrument = 0;
int Player::diff = 0;

int Player::notesHit = 0;
int Player::notesMissed = 0;
int Player::perfectHit = 0;

float Player::selInstVolume = 0.5;
float Player::otherInstVolume = 0.375;
float Player::missVolume = 0.15;

// time in seconds
float Player::goodFrontend = 0.1f;
float Player::goodBackend = 0.1f;
float Player::perfectFrontend = 0.025f;
float Player::perfectBackend = 0.025f;

float Player::VideoOffset = (0);
float Player::InputOffset = 0;

bool Player::MissHighwayColor = false;

bool lastNotePerfect = false;

// make the hitwindow bigger for properly doing lifts
float Player::liftTimingMult = 1.25f;

// 11.5f default --   23.0f 2x
float Player::defaultHighwayLength = 11.5f;

float Player::smasherPos = 2.4f; // used to be 2.7

bool Player::extraGameplayStats = false;

int Player::notes = 0;
int Player::combo = 0;
int Player::maxCombo = 0;
int Player::score = 0;
std::vector<int> Player::sustainScoreBuffer{ 0,0,0,0,0 };
int Player::playerOverhits = 0;

bool Player::goldStars = false;

bool Player::overdrive = false;

bool Player::FC = true;

float Player::health = 100.0f;

bool Player::mute = false;

float Player::xStarThreshold[6] = { 0.05f, 0.175f, 0.325f, 0.5f, 0.7f,  1.0f };

float Player::overdriveFill = 0.0f;
float Player::overdriveActiveFill=0.0f;
double Player::overdriveActiveTime = 0.0;

float Player::uvOffsetX = 0;
float Player::uvOffsetY = 0;

int Player::stars(int baseScore, int difficulty) {
    float starPercent = (float)score/(float)baseScore;
    if (starPercent < xStarThreshold[0]) {return 0;}
	else if (starPercent < xStarThreshold[1]) { return 1; }
    else if (starPercent < xStarThreshold[2]) {return 2;}
    else if (starPercent < xStarThreshold[3]) {return 3;}
    else if (starPercent < xStarThreshold[4]) {return 4;}
    else if (starPercent < xStarThreshold[5]) {return 5;}
	else if (starPercent >= xStarThreshold[5] && difficulty == 3) { goldStars = true; return 5; }
    else return 5;

    return 0;
}

int Player::multiplier(int instrument) {
		int od = overdrive ? 2 : 1;
		
	if (instrument == 1 || instrument == 3){ 

		if (combo < 10) { uvOffsetX = 0; uvOffsetY = 0 + (overdrive ? 0.5f:0); return 1 * od; }
		else if (combo < 20) { uvOffsetX = 0.25f; uvOffsetY = 0 + (overdrive ? 0.5f : 0);  return 2 * od; }
		else if (combo < 30) { uvOffsetX = 0.5f; uvOffsetY = 0 + (overdrive ? 0.5f : 0);  return 3 * od; }
		else if (combo < 40) { uvOffsetX = 0.75f; uvOffsetY = 0 + (overdrive ? 0.5f : 0); return 4 * od; }
		else if (combo < 50) { uvOffsetX = 0; uvOffsetY = 0.25f + (overdrive ? 0.5f : 0); return 5 * od; }
		else if (combo >= 50) { uvOffsetX = 0.25f; uvOffsetY = 0.25f + (overdrive ? 0.5f : 0); return 6 * od; }
		else { return 1 * od; }
	}
	else {
		if (combo < 10) { uvOffsetX = 0; uvOffsetY = 0 + (overdrive ? 0.5 : 0); return 1 * od; }
		else if (combo < 20) { uvOffsetX = 0.25f; uvOffsetY = 0 + (overdrive ? 0.5 : 0); return 2 * od; }
		else if (combo < 30) { uvOffsetX = 0.5f; uvOffsetY = 0 + (overdrive ? 0.5 : 0); return 3 * od; }
		else if (combo >= 30) { uvOffsetX = 0.75f; uvOffsetY = 0 + (overdrive ? 0.5 : 0); return 4 * od; }
		else { return 1 * od; }
	};
}

int Player::maxMultForMeter(int instrument) {
	if (instrument == 1 || instrument == 3)
		return 5;
	else
		return 3;
}

float Player::comboFillCalc(int instrument) {
	if (instrument == 0 || instrument == 2) {
		// For instruments 0 and 2, limit the float value to 0.0 to 0.4
		if (combo >= 30) {
			return 1.0f; // If combo is 30 or more, set float value to 1.0
		}
		else {
			return static_cast<float>(combo % 10) / 10.0f; // Float value from 0.0 to 0.9 every 10 notes
		}
	}
	else {
		// For instruments 1 and 3, limit the float value to 0.0 to 0.6
		if (combo >= 50) {
			return 1.0f; // If combo is 50 or more, set float value to 1.0
		}
		else {
			return static_cast<float>(combo % 10) / 10.0f; // Float value from 0.0 to 0.9 every 10 notes
		}
	}
}

// clone hero defaults

Color Player::accentColor = Color(255,0,255,255);

Color Player::overdriveColor = Color{255,200,0,255};

void Player::resetPlayerStats() {
    notesHit = 0;
    notesMissed = 0;
    perfectHit = 0;
    maxCombo = 0;
    combo = 0;
    score = 0;
    FC = true;
    notes = 0;
    goldStars = false;
    playerOverhits = 0;
    overdrive = false;
    lastNotePerfect = false;
};
void Player::HitNote(bool perfect, int instrument) {
    notesHit += 1;
    combo += 1;
    if (combo > maxCombo)
        maxCombo = combo;
    float perfectMult = perfect ? 1.2f : 1.0f;
    score += (int)((30 * (multiplier(instrument)) * perfectMult));
    perfectHit += perfect ? 1 : 0;
    mute = false;
}
void Player::HitNoteAudio(bool perfect, int instrument) {
    notesHit += 1;
    combo += 1;
    if (combo > maxCombo)
        maxCombo = combo;
    float perfectMult = perfect ? 1.2f : 1.0f;
    score += (int)((30 * (multiplier(instrument)) * perfectMult));
    perfectHit += perfect ? 1 : 0;
    mute = false;
}
void Player::MissNote() {
    notesMissed += 1;
    if (combo > maxCombo)
        maxCombo = combo;
    combo = 0;
    FC = false;
    mute = true;
}
void Player::OverHit() {
    if (combo > maxCombo)
        maxCombo = combo;
    combo = 0;
    playerOverhits += 1;
    FC = false;
    mute = true;
}


