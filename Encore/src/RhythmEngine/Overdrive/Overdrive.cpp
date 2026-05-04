//
// Created by maria on 26/08/2025.
//

#include "Overdrive.h"
#include "assets.h"
#include "gameplay/enctime.h"
#include "settings/settings.h"
#include "song/audio.h"
#include "util/enclog.h"
#include "RhythmEngine/NoteVector.h"

void Encore::RhythmEngine::Overdrive::Update(double &CurrentTime) {
    if (!Active)
        return;
    // EncoreLog(LOG_DEBUG, TextFormat("Overdrive delta: %4.4f",
    // (TheSongTime.CurrentODTick - TheSongTime.LastODTick))); EncoreLog(LOG_DEBUG,
    // TextFormat("Overdrive tick: %4.4f", (TheSongTime.CurrentODTick)));
    // EncoreLog(LOG_DEBUG, TextFormat("Last Overdrive tick: %4.4f",
    // (TheSongTime.LastODTick)));
    //  two measures per charge at 4/4
    //  8 beats per charge
    //  64 for full charge
    Fill -= (ticks.CurrentODTick - ticks.LastODTick) / 32;

    if (Fill <= 0) {
        Fill = 0;
        EncoreLog(LOG_DEBUG, TextFormat("Last OD Tick: %4.4f", ticks.CurrentODTick));
        Active = false;
    }
}
bool Encore::RhythmEngine::Overdrive::Activate(const double &CurrentTime) {
    if (Fill < 0.25 || Active)
        return false;
    Active = true;
    UseOverdriveLift = true;
    ActivationTime = CurrentTime;
    ActivationTick = ticks.CurrentODTick;
    TheAudioManager.playSample(ASSET(activateSound), TheGameSettings.avMainVolume * TheGameSettings.avSoundEffectVolume);
    EncoreLog(LOG_DEBUG, TextFormat("First OD Tick: %4.4f", ticks.CurrentODTick));
    return true;
}

bool Encore::RhythmEngine::Overdrive::Add(
    const double &CurrentTime, std::shared_ptr<BaseChart> &chart
) {
    float previousFill = Fill;
    Fill += chart->overdrive.CheckOverdrive(CurrentTime);
    if (Fill > 1.0)
        Fill = 1.0;
    if (Fill != previousFill)
        return true;
    return false;
}