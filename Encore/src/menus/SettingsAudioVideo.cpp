//
// Created by Jaydenz on 04/29/2025.
//

#include "SettingsAudioVideo.h"

#include "MenuManager.h"
#include "gameMenu.h"
#include "raygui.h"
#include "assets.h"
#include "settings.h"
#include "settingsOptionRenderer.h"
#include "uiUnits.h"
#include "gameplay/enctime.h"
#include "OvershellMenu.h"
#include "util/settings-text.h"

bool ShowAudioVisualSettings = true;
bool showVolumeSettings = false;

// settings variables
int AudioOffset = 0;
int Framerate = 0;
float avMainVolume = 0.0f;
float avActiveInstrumentVolume = 0.0f;
float avInactiveInstrumentVolume = 0.0f;
float avMuteVolume = 0.0f;
float avMenuMusicVolume = 0.0f;
float avSoundEffectVolume = 0.0f;
bool BackgroundBeatFlash = false;
bool VerticalSync = false;

void SettingsAudioVideo::Draw() {
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();
    SettingsOld &settingsMain = SettingsOld::getInstance();
    SongTime &enctime = TheSongTime;
    settingsOptionRenderer sor;

    const float boxWidthPct = 0.55f;

    GameMenu::DrawAlbumArtBackground(TheSongList.curSong->albumArtBlur);
    DrawRectangle(u.LeftSide, 0, u.winpct(1.0f), GetScreenHeight(), Color{0, 0, 0});

    encOS::DrawTopOvershell(0.15f);
    GameMenu::DrawVersion();
    GameMenu::DrawBottomOvershell();

    float SidebarLeft = u.LeftSide + u.winpct(0.70f);
    float SidebarWidth = u.wpct(0.235f);
    float SidebarTop = u.hinpct(0.15f);
    float SidebarHeight = u.hpct(0.85f);
    float SidebarHeaderHeight = u.hinpct(0.10f);
    float borderWidth = u.winpct(0.002f);
    float innerTop = SidebarTop + borderWidth;

    DrawLineEx({SidebarLeft - borderWidth, SidebarTop}, {SidebarLeft - borderWidth, SidebarTop + SidebarHeight}, borderWidth, WHITE);
    DrawLineEx({SidebarLeft + SidebarWidth, SidebarTop}, {SidebarLeft + SidebarWidth, SidebarTop + SidebarHeight}, borderWidth, WHITE);
    DrawLineEx({SidebarLeft - borderWidth, SidebarTop + SidebarHeight}, {SidebarLeft + SidebarWidth + borderWidth, SidebarTop + SidebarHeight}, borderWidth, WHITE);
    DrawRectangle(SidebarLeft, SidebarTop, SidebarWidth, SidebarHeight, Color{31, 31, 50, 255});

    struct SidebarContent {
        const char* header;
        const char* body;
    };
    SidebarContent sidebarContents[] = {
        // sidebar text
        // Audio Calibration
        {
            "Audio Calibration",
            "Increases the time between the audio \nplayback and note display to make notes\n line up with the audio.\nThe higher the number, the later the\naudio, and the closer to the screen will\nthe notes line up."
        },
        // Volume
        {
            "Volume Settings",
            "Placeholder"
        },
        // Main Output
        {
            "Main Output Volume",
            "Placeholder"
        },
        // Active Instrument
        {
            "Active Instrument Volume",
            "Placeholder"
        },
        // Inactive Instrument
        {
            "Inactive Instrument Volume",
            "Placeholder"
        },
        // Mute Instrument
        {
            "Mute Instrument Volume",
            "Placeholder"
        },
        // Menu Music
        {
            "Menu Music Volume",
            "Placeholder"
        },
        // Sound Effects
        {
            "Sound Effects Volume",
            "Placeholder"
        },
        // Background Beat Flash
        {
            "Background Beat Flash",
            "Placeholder"
        },
        // Framerate
        {
            "Framerate",
            "Placeholder"
        },
        // V-Sync
        {
            "V-Sync",
            "Placeholder"
        }
    };

    static int selectedIndex = 0;
    Vector2 mousePos = GetMousePosition();
    bool isHovering = false;

    const char* headerText = sidebarContents[selectedIndex].header;
    const char* sidebarBodyText = sidebarContents[selectedIndex].body;
    float headerFontSize = u.hinpct(0.030f);
    float headerLineSpacing = headerFontSize * 1.2f;
    std::vector<std::string> headerLines = split(headerText, "\n");
    float maxHeaderWidth = 0;
    for (const std::string& line : headerLines) {
        Vector2 lineSize = MeasureTextEx(assets.rubikBold, line.c_str(), headerFontSize, 0);
        if (lineSize.x > maxHeaderWidth) {
            maxHeaderWidth = lineSize.x;
        }
    }
    float currentHeaderY = innerTop + u.hinpct(0.02f);
    for (const std::string& line : headerLines) {
        float lineX = SidebarLeft + (SidebarWidth - maxHeaderWidth) / 2;
        DrawTextEx(assets.rubikBold, line.c_str(), {lineX, currentHeaderY}, headerFontSize, 0, WHITE);
        currentHeaderY += headerLineSpacing;
    }
    float bodyFontSize = u.hinpct(0.030f);
    float lineSpacing = bodyFontSize * 1.2f;
    std::vector<std::string> lines = split(sidebarBodyText, "\n");
    float currentY = SidebarTop + SidebarHeaderHeight + u.hinpct(0.02f);
    for (const std::string& line : lines) {
        Vector2 lineSize = MeasureTextEx(assets.rubik, line.c_str(), bodyFontSize, 0);
        float lineX = SidebarLeft + (SidebarWidth - lineSize.x) / 2;
        DrawTextEx(assets.rubik, line.c_str(), {lineX, currentY}, bodyFontSize, 0, WHITE);
        currentY += lineSpacing;
    }

    float TextPlacementTB = u.hpct(0.05f);
    float TextPlacementLR = u.wpct(0.05f);
    DrawTextEx(assets.rubik, "Settings", {TextPlacementLR, u.hpct(0.027f)}, u.hinpct(0.042f), 0, LIGHTGRAY);
    GameMenu::mhDrawText(assets.redHatDisplayBlack, "AUDIO / VISUAL", {TextPlacementLR, TextPlacementTB}, u.hinpct(0.125f), WHITE, assets.sdfShader, LEFT);

    float settingsOffsetX = 0.0f;
    float settingsOffsetY = 0.0f;
    float EntryFontSize = u.hinpct(0.03f);
    float EntryHeight = u.hinpct(0.06f) + 30.0f - 50.0f + 10.0f + 7.0f;
    float EntryTop = TextPlacementTB + u.hinpct(0.125f) + u.hinpct(0.01f) + settingsOffsetY - 30.0f - 2.0f - 2.0f;
    float verticalGap = 0.0f;
    float verticalSubmenuGap = 10.0f;
    float boxLeft = u.LeftSide + u.winpct(0.025f) + settingsOffsetX + 75.0f - 50.0f - 2.0f;
    float boxWidth = u.wpct(boxWidthPct) + 50.0f + 17.0f + 7.0f - 2.0f - 2.0f;
    float OptionLeft = boxLeft;
    float OptionWidth = boxWidth;
    float boxPadding = 0.0f;
    Color boxBackground = Color{31, 31, 50, 255};
    Color boxBorder = WHITE;
    Color glowColor = Color{142, 13, 148, 220};
    float highlightBorderWidth = 4.0f;

    Vector2 textSize15 = MeasureTextEx(assets.rubikBold, "-15", u.hinpct(0.02f), 0);
    float buttonWidth15 = textSize15.x + 30.0f;
    float buttonWidth15Inc = textSize15.x + u.winpct(0.05f) + 30.0f;
    float buttonHeight = EntryHeight;
    float sliderWidthFactor = 0.6f;
    float adjustedSliderWidth = (OptionWidth - buttonWidth15 - buttonWidth15Inc) * sliderWidthFactor;
    float sliderTotalWidth = buttonWidth15 + adjustedSliderWidth + buttonWidth15Inc;
    float volumeSliderWidthFactor = 0.5f;
    float adjustedVolumeSliderWidth;
    float toggleButtonWidth = ((OptionWidth / 2) * 0.3f) - 30.0f;
    float toggleOffset = 50.0f;

    Color sliderNormal = Color{24, 24, 39, 178};
    Color sliderHovered = Color{84, 13, 88, 200};
    Color sliderFocused = Color{142, 13, 148, 220};
    GuiSetStyle(SLIDER, BASE_COLOR_NORMAL, ColorToInt(sliderNormal));
    GuiSetStyle(SLIDER, BASE_COLOR_FOCUSED, ColorToInt(sliderHovered));
    GuiSetStyle(SLIDER, BASE_COLOR_PRESSED, ColorToInt(sliderFocused));
    GuiSetStyle(SLIDER, TEXT_COLOR_FOCUSED, ColorToInt(sliderFocused));
    GuiSetStyle(SLIDER, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
    GuiSetStyle(SLIDER, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(SLIDER, BORDER_WIDTH, 2);
    GuiSetStyle(SLIDER, SLIDER_WIDTH, 0);
    GuiSetStyle(SLIDER, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);

    Color activeColor = Color{255, 105, 180, 255};
    int defaultColor = GuiGetStyle(BUTTON, BASE_COLOR_PRESSED);

    // Audio Calibration
    int settingOffset = 0;
    float calibrationTop = EntryTop + (EntryHeight + verticalGap) * settingOffset;
    Rectangle calibrationBoxRect = {boxLeft - borderWidth, calibrationTop - borderWidth, boxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth};
    DrawRectangle(boxLeft - borderWidth, calibrationTop - borderWidth, boxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth, boxBorder);
    DrawRectangle(boxLeft, calibrationTop, boxWidth, EntryHeight, boxBackground);
    Vector2 calibTextSize = MeasureTextEx(assets.rubikBold, "Audio Calibration", EntryFontSize, 0);
    DrawTextEx(assets.rubikBold, "Audio Calibration", {boxLeft + u.winpct(0.01f), calibrationTop + (EntryHeight - calibTextSize.y) / 2}, EntryFontSize, 0, WHITE);
    Rectangle decButtonRect = {OptionLeft + OptionWidth - sliderTotalWidth, calibrationTop, buttonWidth15, buttonHeight};
    Rectangle sliderRect = {decButtonRect.x + buttonWidth15, calibrationTop, adjustedSliderWidth, buttonHeight};
    Rectangle incButtonRect = {sliderRect.x + sliderRect.width, calibrationTop, buttonWidth15Inc, buttonHeight};
    if (CheckCollisionPointRec(mousePos, decButtonRect) || CheckCollisionPointRec(mousePos, sliderRect) || CheckCollisionPointRec(mousePos, incButtonRect)) {
        selectedIndex = 0;
        isHovering = true;
        DrawRectangleLinesEx(calibrationBoxRect, highlightBorderWidth, glowColor);
    }
    if (GuiButton(decButtonRect, "-5")) {
        AudioOffset -= 5;
        if (AudioOffset < -100) AudioOffset = -100;
    }
    float tempAudioOffset = static_cast<float>(AudioOffset);
    ::GuiSlider(sliderRect, nullptr, nullptr, &tempAudioOffset, -100.0f, 400.0f);
    AudioOffset = static_cast<int>(roundf(tempAudioOffset));
    if (AudioOffset < -100) AudioOffset = -100;
    if (AudioOffset > 400) AudioOffset = 400;
    if (GuiButton(incButtonRect, "+5")) {
        AudioOffset += 5;
        if (AudioOffset > 400) AudioOffset = 400;
    }
    std::string msText = std::to_string(AudioOffset) + " ms";
    float msFontSize = u.hinpct(0.02f) + 10.0f;
    Vector2 msTextSize = MeasureTextEx(assets.rubikBold, msText.c_str(), msFontSize, 0);
    DrawTextEx(assets.rubikBold, msText.c_str(), {sliderRect.x + (sliderRect.width - msTextSize.x) / 2, sliderRect.y + (sliderRect.height - msTextSize.y) / 2}, msFontSize, 0, WHITE);

    // Volume
    settingOffset++;
    float volumeTop = EntryTop + (EntryHeight + verticalGap) * settingOffset;
    Rectangle volumeBoxRect = {boxLeft - borderWidth, volumeTop - borderWidth, boxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth};
    DrawRectangle(boxLeft - borderWidth, volumeTop - borderWidth, boxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth, boxBorder);
    DrawRectangle(boxLeft, volumeTop, boxWidth, EntryHeight, boxBackground);
    Vector2 volTextSize = MeasureTextEx(assets.rubikBold, "Volume", EntryFontSize, 0);
    DrawTextEx(assets.rubikBold, "Volume", {boxLeft + u.winpct(0.01f), volumeTop + (EntryHeight - volTextSize.y) / 2}, EntryFontSize, 0, WHITE);
    Rectangle volumeButtonRect = {OptionLeft + OptionWidth - buttonWidth15Inc, volumeTop, buttonWidth15Inc, buttonHeight};
    if (CheckCollisionPointRec(mousePos, volumeButtonRect)) {
        selectedIndex = 1;
        isHovering = true;
        DrawRectangleLinesEx(volumeBoxRect, highlightBorderWidth, glowColor);
    }
    if (GuiButton(volumeButtonRect, showVolumeSettings ? "v" : ">")) {
        showVolumeSettings = !showVolumeSettings;
    }

    float volumeIndent = 50.0f;
    float volumeBoxShift = 50.0f;
    float volumeBoxLeft = boxLeft + volumeBoxShift;
    float volumeBoxWidth = boxWidth - 100.0f;
    float volumeOptionLeft = volumeBoxLeft + volumeIndent;
    float volumeOptionWidth = volumeBoxWidth - volumeIndent;
    float volumeShift = 150.0f;
    Vector2 longestTextSize = MeasureTextEx(assets.rubikBold, "Inactive Instrument", EntryFontSize, 0);
    float maxTextWidth = longestTextSize.x;
    float percentTextWidth = MeasureTextEx(assets.rubikBold, "100%", EntryFontSize, 0).x;
    float availableSliderSpace = volumeOptionWidth - maxTextWidth - percentTextWidth - 90.0f;
    adjustedVolumeSliderWidth = availableSliderSpace * volumeSliderWidthFactor;

    if (showVolumeSettings) {
        // Main Output
        settingOffset++;
        float mainVolTop = EntryTop + (EntryHeight + verticalGap) * settingOffset + verticalSubmenuGap - 7.0f;
        Rectangle mainVolBoxRect = {volumeBoxLeft - borderWidth, mainVolTop - borderWidth, volumeBoxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth};
        DrawRectangle(volumeBoxLeft - borderWidth, mainVolTop - borderWidth, volumeBoxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth, boxBorder);
        DrawRectangle(volumeBoxLeft, mainVolTop, volumeBoxWidth, EntryHeight, boxBackground);
        Vector2 mainVolTextSize = MeasureTextEx(assets.rubikBold, "Main Output", EntryFontSize, 0);
        DrawTextEx(assets.rubikBold, "Main Output", {volumeBoxLeft + u.winpct(0.01f), mainVolTop + (EntryHeight - mainVolTextSize.y) / 2}, EntryFontSize, 0, WHITE);
        Rectangle mainVolSliderRect = {volumeOptionLeft + maxTextWidth + volumeShift, mainVolTop, adjustedVolumeSliderWidth, buttonHeight};
        if (CheckCollisionPointRec(mousePos, mainVolSliderRect)) {
            selectedIndex = 2;
            isHovering = true;
            DrawRectangleLinesEx(mainVolBoxRect, highlightBorderWidth, glowColor);
        }
        float prevMainVolume = avMainVolume;
        ::GuiSlider(mainVolSliderRect, nullptr, nullptr, &avMainVolume, 0.0f, 1.0f);
        if (avMainVolume != prevMainVolume) {
            avMainVolume = roundf(avMainVolume * 20.0f) / 20.0f;
        }
        std::string percentText = std::to_string(static_cast<int>(avMainVolume * 100.0f)) + "%";
        Vector2 textSize = MeasureTextEx(assets.rubikBold, percentText.c_str(), EntryFontSize, 0);
        float percentX = mainVolSliderRect.x + adjustedVolumeSliderWidth + u.winpct(0.01f) + 90.0f;
        float percentY = mainVolTop + (EntryHeight - textSize.y) / 2;
        DrawTextEx(assets.rubikBold, percentText.c_str(), {percentX, percentY}, EntryFontSize, 0, WHITE);

        // Active Instrument
        settingOffset++;
        float activeVolTop = EntryTop + (EntryHeight + verticalGap) * settingOffset + verticalSubmenuGap - 7.0f;
        Rectangle activeVolBoxRect = {volumeBoxLeft - borderWidth, activeVolTop - borderWidth, volumeBoxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth};
        DrawRectangle(volumeBoxLeft - borderWidth, activeVolTop - borderWidth, volumeBoxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth, boxBorder);
        DrawRectangle(volumeBoxLeft, activeVolTop, volumeBoxWidth, EntryHeight, boxBackground);
        Vector2 activeVolTextSize = MeasureTextEx(assets.rubikBold, "Active Instrument", EntryFontSize, 0);
        DrawTextEx(assets.rubikBold, "Active Instrument", {volumeBoxLeft + u.winpct(0.01f), activeVolTop + (EntryHeight - activeVolTextSize.y) / 2}, EntryFontSize, 0, WHITE);
        Rectangle activeVolSliderRect = {volumeOptionLeft + maxTextWidth + volumeShift, activeVolTop, adjustedVolumeSliderWidth, buttonHeight};
        if (CheckCollisionPointRec(mousePos, activeVolSliderRect)) {
            selectedIndex = 3;
            isHovering = true;
            DrawRectangleLinesEx(activeVolBoxRect, highlightBorderWidth, glowColor);
        }
        float prevActiveVolume = avActiveInstrumentVolume;
        ::GuiSlider(activeVolSliderRect, nullptr, nullptr, &avActiveInstrumentVolume, 0.0f, 1.0f);
        if (avActiveInstrumentVolume != prevActiveVolume) {
            avActiveInstrumentVolume = roundf(avActiveInstrumentVolume * 20.0f) / 20.0f;
        }
        percentText = std::to_string(static_cast<int>(avActiveInstrumentVolume * 100.0f)) + "%";
        textSize = MeasureTextEx(assets.rubikBold, percentText.c_str(), EntryFontSize, 0);
        percentX = activeVolSliderRect.x + adjustedVolumeSliderWidth + u.winpct(0.01f) + 90.0f;
        percentY = activeVolTop + (EntryHeight - textSize.y) / 2;
        DrawTextEx(assets.rubikBold, percentText.c_str(), {percentX, percentY}, EntryFontSize, 0, WHITE);

        // Inactive Instrument
        settingOffset++;
        float inactiveVolTop = EntryTop + (EntryHeight + verticalGap) * settingOffset + verticalSubmenuGap - 7.0f;
        Rectangle inactiveVolBoxRect = {volumeBoxLeft - borderWidth, inactiveVolTop - borderWidth, volumeBoxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth};
        DrawRectangle(volumeBoxLeft - borderWidth, inactiveVolTop - borderWidth, volumeBoxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth, boxBorder);
        DrawRectangle(volumeBoxLeft, inactiveVolTop, volumeBoxWidth, EntryHeight, boxBackground);
        Vector2 inactiveVolTextSize = MeasureTextEx(assets.rubikBold, "Inactive Instrument", EntryFontSize, 0);
        DrawTextEx(assets.rubikBold, "Inactive Instrument", {volumeBoxLeft + u.winpct(0.01f), inactiveVolTop + (EntryHeight - inactiveVolTextSize.y) / 2}, EntryFontSize, 0, WHITE);
        Rectangle inactiveVolSliderRect = {volumeOptionLeft + maxTextWidth + volumeShift, inactiveVolTop, adjustedVolumeSliderWidth, buttonHeight};
        if (CheckCollisionPointRec(mousePos, inactiveVolSliderRect)) {
            selectedIndex = 4;
            isHovering = true;
            DrawRectangleLinesEx(inactiveVolBoxRect, highlightBorderWidth, glowColor);
        }
        float prevInactiveVolume = avInactiveInstrumentVolume;
        ::GuiSlider(inactiveVolSliderRect, nullptr, nullptr, &avInactiveInstrumentVolume, 0.0f, 1.0f);
        if (avInactiveInstrumentVolume != prevInactiveVolume) {
            avInactiveInstrumentVolume = roundf(avInactiveInstrumentVolume * 20.0f) / 20.0f;
        }
        percentText = std::to_string(static_cast<int>(avInactiveInstrumentVolume * 100.0f)) + "%";
        textSize = MeasureTextEx(assets.rubikBold, percentText.c_str(), EntryFontSize, 0);
        percentX = inactiveVolSliderRect.x + adjustedVolumeSliderWidth + u.winpct(0.01f) + 90.0f;
        percentY = inactiveVolTop + (EntryHeight - textSize.y) / 2;
        DrawTextEx(assets.rubikBold, percentText.c_str(), {percentX, percentY}, EntryFontSize, 0, WHITE);

        // Mute Instrument
        settingOffset++;
        float muteVolTop = EntryTop + (EntryHeight + verticalGap) * settingOffset + verticalSubmenuGap - 7.0f;
        Rectangle muteVolBoxRect = {volumeBoxLeft - borderWidth, muteVolTop - borderWidth, volumeBoxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth};
        DrawRectangle(volumeBoxLeft - borderWidth, muteVolTop - borderWidth, volumeBoxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth, boxBorder);
        DrawRectangle(volumeBoxLeft, muteVolTop, volumeBoxWidth, EntryHeight, boxBackground);
        Vector2 muteVolTextSize = MeasureTextEx(assets.rubikBold, "Mute Instrument", EntryFontSize, 0);
        DrawTextEx(assets.rubikBold, "Mute Instrument", {volumeBoxLeft + u.winpct(0.01f), muteVolTop + (EntryHeight - muteVolTextSize.y) / 2}, EntryFontSize, 0, WHITE);
        Rectangle muteVolSliderRect = {volumeOptionLeft + maxTextWidth + volumeShift, muteVolTop, adjustedVolumeSliderWidth, buttonHeight};
        if (CheckCollisionPointRec(mousePos, muteVolSliderRect)) {
            selectedIndex = 5;
            isHovering = true;
            DrawRectangleLinesEx(muteVolBoxRect, highlightBorderWidth, glowColor);
        }
        float prevMuteVolume = avMuteVolume;
        ::GuiSlider(muteVolSliderRect, nullptr, nullptr, &avMuteVolume, 0.0f, 1.0f);
        if (avMuteVolume != prevMuteVolume) {
            avMuteVolume = roundf(avMuteVolume * 20.0f) / 20.0f;
        }
        percentText = std::to_string(static_cast<int>(avMuteVolume * 100.0f)) + "%";
        textSize = MeasureTextEx(assets.rubikBold, percentText.c_str(), EntryFontSize, 0);
        percentX = muteVolSliderRect.x + adjustedVolumeSliderWidth + u.winpct(0.01f) + 90.0f;
        percentY = muteVolTop + (EntryHeight - textSize.y) / 2;
        DrawTextEx(assets.rubikBold, percentText.c_str(), {percentX, percentY}, EntryFontSize, 0, WHITE);

        // Menu Music
        settingOffset++;
        float menuVolTop = EntryTop + (EntryHeight + verticalGap) * settingOffset + verticalSubmenuGap - 7.0f;
        Rectangle menuVolBoxRect = {volumeBoxLeft - borderWidth, menuVolTop - borderWidth, volumeBoxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth};
        DrawRectangle(volumeBoxLeft - borderWidth, menuVolTop - borderWidth, volumeBoxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth, boxBorder);
        DrawRectangle(volumeBoxLeft, menuVolTop, volumeBoxWidth, EntryHeight, boxBackground);
        Vector2 menuVolTextSize = MeasureTextEx(assets.rubikBold, "Menu Music", EntryFontSize, 0);
        DrawTextEx(assets.rubikBold, "Menu Music", {volumeBoxLeft + u.winpct(0.01f), menuVolTop + (EntryHeight - menuVolTextSize.y) / 2}, EntryFontSize, 0, WHITE);
        Rectangle menuVolSliderRect = {volumeOptionLeft + maxTextWidth + volumeShift, menuVolTop, adjustedVolumeSliderWidth, buttonHeight};
        if (CheckCollisionPointRec(mousePos, menuVolSliderRect)) {
            selectedIndex = 6;
            isHovering = true;
            DrawRectangleLinesEx(menuVolBoxRect, highlightBorderWidth, glowColor);
        }
        float prevMenuVolume = avMenuMusicVolume;
        ::GuiSlider(menuVolSliderRect, nullptr, nullptr, &avMenuMusicVolume, 0.0f, 1.0f);
        if (avMenuMusicVolume != prevMenuVolume) {
            avMenuMusicVolume = roundf(avMenuMusicVolume * 20.0f) / 20.0f;
        }
        percentText = std::to_string(static_cast<int>(avMenuMusicVolume * 100.0f)) + "%";
        textSize = MeasureTextEx(assets.rubikBold, percentText.c_str(), EntryFontSize, 0);
        percentX = menuVolSliderRect.x + adjustedVolumeSliderWidth + u.winpct(0.01f) + 90.0f;
        percentY = menuVolTop + (EntryHeight - textSize.y) / 2;
        DrawTextEx(assets.rubikBold, percentText.c_str(), {percentX, percentY}, EntryFontSize, 0, WHITE);

        // Sound Effects
        settingOffset++;
        float sfxVolTop = EntryTop + (EntryHeight + verticalGap) * settingOffset + verticalSubmenuGap - 7.0f;
        Rectangle sfxVolBoxRect = {volumeBoxLeft - borderWidth, sfxVolTop - borderWidth, volumeBoxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth};
        DrawRectangle(volumeBoxLeft - borderWidth, sfxVolTop - borderWidth, volumeBoxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth, boxBorder);
        DrawRectangle(volumeBoxLeft, sfxVolTop, volumeBoxWidth, EntryHeight, boxBackground);
        Vector2 sfxVolTextSize = MeasureTextEx(assets.rubikBold, "Sound Effects", EntryFontSize, 0);
        DrawTextEx(assets.rubikBold, "Sound Effects", {volumeBoxLeft + u.winpct(0.01f), sfxVolTop + (EntryHeight - sfxVolTextSize.y) / 2}, EntryFontSize, 0, WHITE);
        Rectangle sfxVolSliderRect = {volumeOptionLeft + maxTextWidth + volumeShift, sfxVolTop, adjustedVolumeSliderWidth, buttonHeight};
        if (CheckCollisionPointRec(mousePos, sfxVolSliderRect)) {
            selectedIndex = 7;
            isHovering = true;
            DrawRectangleLinesEx(sfxVolBoxRect, highlightBorderWidth, glowColor);
        }
        float prevSfxVolume = avSoundEffectVolume;
        ::GuiSlider(sfxVolSliderRect, nullptr, nullptr, &avSoundEffectVolume, 0.0f, 1.0f);
        if (avSoundEffectVolume != prevSfxVolume) {
            avSoundEffectVolume = roundf(avSoundEffectVolume * 20.0f) / 20.0f;
        }
        percentText = std::to_string(static_cast<int>(avSoundEffectVolume * 100.0f)) + "%";
        textSize = MeasureTextEx(assets.rubikBold, percentText.c_str(), EntryFontSize, 0);
        percentX = sfxVolSliderRect.x + adjustedVolumeSliderWidth + u.winpct(0.01f) + 90.0f;
        percentY = sfxVolTop + (EntryHeight - textSize.y) / 2;
        DrawTextEx(assets.rubikBold, percentText.c_str(), {percentX, percentY}, EntryFontSize, 0, WHITE);
    }

    // Background Beat Flash
    settingOffset++;
    float beatFlashTop = EntryTop + (EntryHeight + verticalGap) * settingOffset;
    if (showVolumeSettings) {
        beatFlashTop += verticalSubmenuGap - 7.0f;
    }
    Rectangle beatFlashBoxRect = {boxLeft - borderWidth, beatFlashTop - borderWidth, boxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth};
    DrawRectangle(boxLeft - borderWidth, beatFlashTop - borderWidth, boxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth, boxBorder);
    DrawRectangle(boxLeft, beatFlashTop, boxWidth, EntryHeight, boxBackground);
    Vector2 beatFlashTextSize = MeasureTextEx(assets.rubikBold, "Background Beat Flash", EntryFontSize, 0);
    DrawTextEx(assets.rubikBold, "Background Beat Flash", {boxLeft + u.winpct(0.01f), beatFlashTop + (EntryHeight - beatFlashTextSize.y) / 2}, EntryFontSize, 0, WHITE);
    Rectangle offButtonRect1 = {OptionLeft + OptionWidth - 2 * toggleButtonWidth - toggleOffset, beatFlashTop, toggleButtonWidth, buttonHeight};
    Rectangle onButtonRect1 = {OptionLeft + OptionWidth - toggleButtonWidth - toggleOffset, beatFlashTop, toggleButtonWidth, buttonHeight};
    if (CheckCollisionPointRec(mousePos, offButtonRect1) || CheckCollisionPointRec(mousePos, onButtonRect1)) {
        selectedIndex = 8;
        isHovering = true;
        DrawRectangleLinesEx(beatFlashBoxRect, highlightBorderWidth, glowColor);
    }
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, BackgroundBeatFlash ? defaultColor : ColorToInt(activeColor));
    if (GuiButton(offButtonRect1, "Off")) {
        BackgroundBeatFlash = false;
    }
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, BackgroundBeatFlash ? ColorToInt(activeColor) : defaultColor);
    if (GuiButton(onButtonRect1, "On")) {
        BackgroundBeatFlash = true;
    }
    if (!BackgroundBeatFlash) {
        DrawRectangleLinesEx(offButtonRect1, highlightBorderWidth, glowColor);
    } else {
        DrawRectangleLinesEx(onButtonRect1, highlightBorderWidth, glowColor);
    }
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, defaultColor);

    // Framerate
    settingOffset++;
    float framerateTop = EntryTop + (EntryHeight + verticalGap) * settingOffset;
    if (showVolumeSettings) {
        framerateTop += verticalSubmenuGap - 7.0f;
    }
    Rectangle framerateBoxRect = {boxLeft - borderWidth, framerateTop - borderWidth, boxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth};
    DrawRectangle(boxLeft - borderWidth, framerateTop - borderWidth, boxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth, boxBorder);
    DrawRectangle(boxLeft, framerateTop, boxWidth, EntryHeight, boxBackground);
    Vector2 framerateTextSize = MeasureTextEx(assets.rubikBold, "Framerate", EntryFontSize, 0);
    DrawTextEx(assets.rubikBold, "Framerate", {boxLeft + u.winpct(0.01f), framerateTop + (EntryHeight - framerateTextSize.y) / 2}, EntryFontSize, 0, WHITE);
    Rectangle frDecButtonRect = {OptionLeft + OptionWidth - sliderTotalWidth, framerateTop, buttonWidth15, buttonHeight};
    Rectangle frSliderRect = {frDecButtonRect.x + buttonWidth15, framerateTop, adjustedSliderWidth, buttonHeight};
    Rectangle frIncButtonRect = {frSliderRect.x + frSliderRect.width, framerateTop, buttonWidth15Inc, buttonHeight};
    if (CheckCollisionPointRec(mousePos, frDecButtonRect) || CheckCollisionPointRec(mousePos, frSliderRect) || CheckCollisionPointRec(mousePos, frIncButtonRect)) {
        selectedIndex = 9;
        isHovering = true;
        DrawRectangleLinesEx(framerateBoxRect, highlightBorderWidth, glowColor);
    }
    if (GuiButton(frDecButtonRect, "-15")) {
        Framerate -= 15;
        if (Framerate < 30) Framerate = 30;
    }
    float tempFramerate = static_cast<float>(Framerate);
    ::GuiSlider(frSliderRect, nullptr, nullptr, &tempFramerate, 30.0f, 1500.0f);
    Framerate = static_cast<int>(roundf(tempFramerate));
    if (Framerate < 30) Framerate = 30;
    if (Framerate > 1500) Framerate = 1500;
    if (GuiButton(frIncButtonRect, "+15")) {
        Framerate += 15;
        if (Framerate > 1500) Framerate = 1500;
    }
    std::string fpsText = std::to_string(Framerate) + " fps";
    float fpsFontSize = u.hinpct(0.02f) + 10.0f;
    Vector2 fpsTextSize = MeasureTextEx(assets.rubikBold, fpsText.c_str(), fpsFontSize, 0);
    DrawTextEx(assets.rubikBold, fpsText.c_str(), {frSliderRect.x + (frSliderRect.width - fpsTextSize.x) / 2, frSliderRect.y + (frSliderRect.height - fpsTextSize.y) / 2}, fpsFontSize, 0, WHITE);

    // V-Sync
    settingOffset++;
    float vsyncTop = EntryTop + (EntryHeight + verticalGap) * settingOffset;
    if (showVolumeSettings) {
        vsyncTop += verticalSubmenuGap - 7.0f;
    }
    Rectangle vsyncBoxRect = {boxLeft - borderWidth, vsyncTop - borderWidth, boxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth};
    DrawRectangle(boxLeft - borderWidth, vsyncTop - borderWidth, boxWidth + 2 * borderWidth, EntryHeight + 2 * borderWidth, boxBorder);
    DrawRectangle(boxLeft, vsyncTop, boxWidth, EntryHeight, boxBackground);
    Vector2 vsyncTextSize = MeasureTextEx(assets.rubikBold, "V-Sync", EntryFontSize, 0);
    DrawTextEx(assets.rubikBold, "V-Sync", {boxLeft + u.winpct(0.01f), vsyncTop + (EntryHeight - vsyncTextSize.y) / 2}, EntryFontSize, 0, WHITE);
    Rectangle offButtonRect2 = {OptionLeft + OptionWidth - 2 * toggleButtonWidth - toggleOffset, vsyncTop, toggleButtonWidth, buttonHeight};
    Rectangle onButtonRect2 = {OptionLeft + OptionWidth - toggleButtonWidth - toggleOffset, vsyncTop, toggleButtonWidth, buttonHeight};
    if (CheckCollisionPointRec(mousePos, offButtonRect2) || CheckCollisionPointRec(mousePos, onButtonRect2)) {
        selectedIndex = 10;
        isHovering = true;
        DrawRectangleLinesEx(vsyncBoxRect, highlightBorderWidth, glowColor);
    }
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, VerticalSync ? defaultColor : ColorToInt(activeColor));
    if (GuiButton(offButtonRect2, "Off")) {
        VerticalSync = false;
    }
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, VerticalSync ? ColorToInt(activeColor) : defaultColor);
    if (GuiButton(onButtonRect2, "On")) {
        VerticalSync = true;
    }
    if (!VerticalSync) {
        DrawRectangleLinesEx(offButtonRect2, highlightBorderWidth, glowColor);
    } else {
        DrawRectangleLinesEx(onButtonRect2, highlightBorderWidth, glowColor);
    }
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, defaultColor);

    if (!isHovering) {
        selectedIndex = 0;
    }

    GameMenu::DrawBottomOvershell();
    DrawOvershell();
}


void SettingsAudioVideo::KeyboardInputCallback(int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        Save();
        TheMenuManager.SwitchScreen(SETTINGS);
    }
}

void SettingsAudioVideo::ControllerInputCallback(int joypadID, GLFWgamepadstate state) {
    if (state.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS) {
        Save();
        TheMenuManager.SwitchScreen(SETTINGS);
    }
}

void SettingsAudioVideo::Load() {
    TheGameSettings.LoadFromFile("settings.json");

    AudioOffset = TheGameSettings.AudioOffset;
    Framerate = TheGameSettings.Framerate;
    avMainVolume = TheGameSettings.avMainVolume;
    avActiveInstrumentVolume = TheGameSettings.avActiveInstrumentVolume;
    avInactiveInstrumentVolume = TheGameSettings.avInactiveInstrumentVolume;
    avMuteVolume = TheGameSettings.avMuteVolume;
    avMenuMusicVolume = TheGameSettings.avMenuMusicVolume;
    avSoundEffectVolume = TheGameSettings.avSoundEffectVolume;
    BackgroundBeatFlash = TheGameSettings.BackgroundBeatFlash;
    VerticalSync = TheGameSettings.VerticalSync;

    TraceLog(LOG_INFO, "Loaded audio/video settings: AudioOffset=%d, Framerate=%d, avMainVolume=%.2f",
             AudioOffset, Framerate, avMainVolume);
}

void SettingsAudioVideo::Save() {
    TheGameSettings.AudioOffset = AudioOffset;
    TheGameSettings.Framerate = Framerate;
    TheGameSettings.avMainVolume = avMainVolume;
    TheGameSettings.avActiveInstrumentVolume = avActiveInstrumentVolume;
    TheGameSettings.avInactiveInstrumentVolume = avInactiveInstrumentVolume;
    TheGameSettings.avMuteVolume = avMuteVolume;
    TheGameSettings.avMenuMusicVolume = avMenuMusicVolume;
    TheGameSettings.avSoundEffectVolume = avSoundEffectVolume;
    TheGameSettings.BackgroundBeatFlash = BackgroundBeatFlash;
    TheGameSettings.VerticalSync = VerticalSync;

    TheGameSettings.SaveToFile("settings.json");

    TraceLog(LOG_INFO, "Saved audio/video settings: AudioOffset=%d, Framerate=%d, avMainVolume=%.2f",
             AudioOffset, Framerate, avMainVolume);
}