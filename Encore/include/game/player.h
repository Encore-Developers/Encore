//look this is the first header file ive ever created


int notesHit = 0;
int notesMissed = 0;

int combo = 0;
int score = 0;

int multiplier() {
	if (combo < 11)			{ return 1; }
	else if (combo < 21)	{ return 2; }
	else if (combo < 31)	{ return 3; }
	else if (combo > 31)	{ return 4; }
	else					{ return 1; }
}

// clone hero defaults
float starThreshold[7] = { 0.3f, 0.7f, 1.0f, 2.0f, 2.8f, 3.6f, 4.4f };

bool FC = true;

class player {
public:
	static void resetPlayerStats() {
		notesHit = 0;
		notesMissed = 0;

		combo = 0;
		score = 0;
		FC = true;
	};
};

