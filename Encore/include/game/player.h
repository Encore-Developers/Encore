//look this is the first header file ive ever created


int notesHit = 0;
int notesMissed = 0;
int perfectHit = 0;

int combo = 0;
int score = 0;

bool overdrive = false;

bool FC = true;

float health = 100.0f;

int multiplier(int instrument) {
		int od = overdrive ? 2 : 1;
	
	if (instrument == 1 || instrument == 3){ 

		if (combo < 10) { return 1 * od; }
		else if (combo < 20) { return 2 * od; }
		else if (combo < 30) { return 3 * od; }
		else if (combo < 40) { return 4 * od; }
		else if (combo < 50) { return 5 * od; }
		else if (combo >= 50) { return 6 * od; }
		else { return 1 * od; };
	}
	else {
		if (combo < 10) { return 1 * od; }
		else if (combo < 20) { return 2 * od; }
		else if (combo < 30) { return 3 * od; }
		else if (combo >= 30) { return 4 * od; }
		else { return 1 * od; }
	};
}

int maxMultForMeter(int instrument) {
	if (instrument == 1 || instrument == 3)
		return 5;
	else
		return 3;
}

float comboFillCalc(int instrument) {
	if (instrument == 0 || instrument == 2) {
        // conversation mbr
        // shouldve been 0 and 2 :sob:
        // also also it stops at the max combo, it doesnt do it one more time
		// For instruments 2 and 4, limit the float value to 0.0 to 0.4
		if (combo >= 30) {
			return 1.0f; // If combo is 40 or more, set float value to 1.0
		}
		else {
			return static_cast<float>(combo % 10) / 10.0f; // Float value from 0.0 to 0.9 every 10 notes
		}
	}
	else {
		// For instruments 1 and 3, limit the float value to 0.0 to 0.6
		if (combo >= 50) {
			return 1.0f; // If combo is 60 or more, set float value to 1.0
		}
		else {
			return static_cast<float>(combo % 10) / 10.0f; // Float value from 0.0 to 0.9 every 10 notes
		}
	}
}


// clone hero defaults
float starThreshold[7] = { 0.3f, 0.7f, 1.0f, 2.0f, 2.8f, 3.6f, 4.4f };



class player {
public:
	static void resetPlayerStats() {
		notesHit = 0;
		notesMissed = 0;
        perfectHit = 0;
		combo = 0;
		score = 0;
		FC = true;
	};

	static void HitNote(bool perfect, int instrument) {
		notesHit += 1;
		combo += 1;
		float perfectMult = perfect ? 1.2f : 1.0f;
		score += (int)((30 * (multiplier(instrument)) * perfectMult));
		perfectHit += perfect ? 1 : 0;
	}
	static void MissNote() {
		notesMissed += 1;
		combo = 0;
		FC = false;
	}
};

