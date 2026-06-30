struct String {
    u64 length;
    char content[length];
};

enum SongPart : u32 {
    PartDrums = 0,
    PartBass,
    PartGuitar,
    PartKeys,
    PartVocals,
    PlasticDrums,
    PlasticBass,
    PlasticGuitar,
    PlasticKeys,
    PitchedVocals,
    PlasticVocals,
    Invalid,
    BeatLines,
    Events
};

enum Difficulty : u32 {
    Easy,
    Medium,
    Hard,
    Expert
};

enum ControllerBindingType : u8 {
    GUITAR = 0,
    GUITAR_GHPS3,
    PAD,
    DRUMS
};

struct Participant {
    String name;
    SongPart instrument;
    Difficulty difficulty;
    u32 activeSlot;
    ControllerBindingType bindingType;
    
    float noteSpeed;
    float trackLength;
};

enum InputChannel : s8 {
    LANE_1 = 0,
    LANE_2 = 1,
    LANE_3 = 2,
    LANE_4 = 3,
    LANE_5 = 4,
    LANE_6 = 5,
    STRUM_UP = 7,
    STRUM_DOWN = 8,
    PAUSE = 9,
    OVERDRIVE = 10,
    WHAMMY = 11,
    INPUT_LEFT = 12,
    INPUT_RIGHT = 13,
    INVALID = -1
};

enum Action : s8 {
    INVALID = -1,
    PRESS = 1,
    RELEASE = 0,
    REPEAT = 2 // not needed but whatever
};

struct InputEvent {
    InputChannel channel;
    Action action;
    u8 axis;
    u8 slot;
    double timestamp;
};

struct Replay {
    u32 header;
    u32 version;
    u8 songHash[32];
    
    u64 participantCount;
    Participant participants[participantCount];
    
    u64 inputCount;
    InputEvent inputs[inputCount];
};

Replay replay @ 0x00;