#include "assets.h"

Texture2D Assets::LoadTextureFilter(PATH_TYPE texturePath, int &loadedAssets) {
    TraceLog(LOG_INFO, "Loading texture: %s", PATH_TO_CSTR(texturePath));
    Texture2D tex = LoadTexture(PATH_TO_CSTR(texturePath));
    if (tex.id == 0) {
        TraceLog(LOG_WARNING, "Failed to load texture: %s", PATH_TO_CSTR(texturePath));
    } else {
        GenTextureMipmaps(&tex);
        SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
        loadedAssets++;
    }
    return tex;
}

Model Assets::LoadModel_(PATH_TYPE modelPath, int &loadedAssets) {
    TraceLog(LOG_INFO, "Loading model: %s", PATH_TO_CSTR(modelPath));
    Model model = LoadModel(PATH_TO_CSTR(modelPath));
    if (model.meshCount == 0) {
        TraceLog(LOG_WARNING, "Failed to load model: %s", PATH_TO_CSTR(modelPath));
    } else {
        loadedAssets++;
    }
    return model;
}

Font Assets::LoadFontFilter(PATH_TYPE fontPath, int fontSize, int &loadedAssets) {
    TraceLog(LOG_INFO, "Loading font: %s", PATH_TO_CSTR(fontPath));
    Font font = LoadFontEx(PATH_TO_CSTR(fontPath), fontSize, nullptr, 250);
    if (font.texture.id == 0) {
        TraceLog(LOG_WARNING, "Failed to load font: %s", PATH_TO_CSTR(fontPath));
        return font;
    }
    font.baseSize = 128;
    font.glyphCount = 250;
    int fileSize = 0;
    unsigned char *fileData = LoadFileData(PATH_TO_CSTR(fontPath), &fileSize);
    if (!fileData) {
        TraceLog(LOG_WARNING, "Failed to load font data: %s", PATH_TO_CSTR(fontPath));
        return font;
    }
    font.glyphs = LoadFontData(fileData, fileSize, 128, nullptr, 250, FONT_SDF);
    Image atlas = GenImageFontAtlas(font.glyphs, &font.recs, 250, 128, 4, 0);
    font.texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);
    UnloadFileData(fileData);
    SetTextureFilter(font.texture, TEXTURE_FILTER_TRILINEAR);
    loadedAssets++;
    return font;
}

void Assets::FirstAssets() {
    icon = LoadImage(PATH_TO_CSTR(PATH_CONCAT("Assets/encore_favicon-NEW.png")));
    encoreWhiteLogo = LoadTextureFilter(PATH_CONCAT("Assets/encore-white.png"), loadedAssets);
    rubik = LoadFontFilter(PATH_CONCAT("Assets/fonts/Rubik-Regular.ttf"), 256, loadedAssets);
    redHatDisplayItalic = LoadFontFilter(PATH_CONCAT("Assets/fonts/RedHatDisplay-BlackItalic.ttf"), 256, loadedAssets);
    redHatDisplayItalicLarge = LoadFontFilter(PATH_CONCAT("Assets/fonts/RedHatDisplay-BlackItalic.ttf"), 256, loadedAssets);
    redHatDisplayBlack = LoadFontFilter(PATH_CONCAT("Assets/fonts/RedHatDisplay-Black.ttf"), 256, loadedAssets);
    rubikBoldItalic = LoadFontFilter(PATH_CONCAT("Assets/fonts/Rubik-BoldItalic.ttf"), 256, loadedAssets);
    rubikBold = LoadFontFilter(PATH_CONCAT("Assets/fonts/Rubik-Bold.ttf"), 256, loadedAssets);
    rubikItalic = LoadFontFilter(PATH_CONCAT("Assets/fonts/Rubik-Italic.ttf"), 256, loadedAssets);
    josefinSansItalic = LoadFontFilter(PATH_CONCAT("Assets/fonts/JosefinSans-Italic.ttf"), 256, loadedAssets);
    josefinSansBold = LoadFontFilter(PATH_CONCAT("Assets/fonts/JosefinSans-Bold.ttf"), 256, loadedAssets);
    josefinSansNormal = LoadFontFilter(PATH_CONCAT("Assets/fonts/JosefinSans-Normal.ttf"), 256, loadedAssets);
}

void Assets::LoadAssets() {
    Color accentColor = { 255, 0, 255, 255 };
    Color overdriveColor = { 255, 200, 0, 255 };
#ifdef PLATFORM_NX
    std::string highwayDir = directory + "gameplay/highway/";
#else
    std::filesystem::path highwayDir = directory / "Assets" / "gameplay" / "highway";
#endif

    smasherInner = LoadModel_(PATH_CONCAT("gameplay/highway/smasherInner.obj"), loadedAssets);
    smasherInnerTex = LoadTexture(PATH_TO_CSTR(PATH_CONCAT("gameplay/highway/smasherbase.png")));
    smasherInner.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherInnerTex;

    smasherOuter = LoadModel_(PATH_CONCAT("gameplay/highway/smasherOuter.obj"), loadedAssets);
    smasherOuterTex = LoadTexture(PATH_TO_CSTR(PATH_CONCAT("gameplay/highway/smasherframe.png")));
    smasherOuter.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherOuterTex;

    smasherTopPressedTex = LoadTexture(PATH_TO_CSTR(PATH_CONCAT("gameplay/highway/smasher-on.png")));
    smasherTopUnpressedTex = LoadTexture(PATH_TO_CSTR(PATH_CONCAT("gameplay/highway/smasher-off.png")));

    smasherBoardTex = LoadTextureFilter(PATH_CONCAT("gameplay/highway/board.png"), loadedAssets);
    smasherBoard = LoadModel_(PATH_CONCAT("gameplay/highway/board_x.obj"), loadedAssets);
    smasherBoardEMH = LoadModel_(PATH_CONCAT("gameplay/highway/board_emh.obj"), loadedAssets);

    lanes = LoadModel_(PATH_CONCAT("gameplay/highway/lanes.obj"), loadedAssets);
    lanesTex = LoadTextureFilter(PATH_CONCAT("gameplay/highway/lanes.png"), loadedAssets);

    smasherPressed = LoadModel_(PATH_CONCAT("gameplay/highway/smasher.obj"), loadedAssets);
    smasherPressTex = LoadTexture(PATH_TO_CSTR(PATH_CONCAT("gameplay/highway/smasher_press.png")));

    star = LoadTextureFilter(PATH_CONCAT("Assets/ui/star.png"), loadedAssets);
    goldStar = LoadTextureFilter(PATH_CONCAT("Assets/ui/gold-star.png"), loadedAssets);
    goldStarUnfilled = LoadTextureFilter(PATH_CONCAT("Assets/ui/gold-star_unfilled.png"), loadedAssets);
    emptyStar = LoadTextureFilter(PATH_CONCAT("Assets/ui/empty-star.png"), loadedAssets);

#ifdef PLATFORM_NX
    Highway = LoadShader(nullptr, PATH_TO_CSTR(PATH_CONCAT("gameplay/highway/highwayShader.fsh")));
#else
    Highway = LoadShader(PATH_TO_CSTR(PATH_CONCAT("gameplay/highway/fLighting.vsh")), PATH_TO_CSTR(PATH_CONCAT("gameplay/highway/highwayShader.fsh")));
#endif
    if (Highway.id == 0) {
        TraceLog(LOG_ERROR, "Failed to load Highway shader");
    }
    HighwayTexShaderLoc = GetShaderLocation(Highway, "highwayTex");
    HighwayTimeShaderLoc = GetShaderLocation(Highway, "time");

    odFrame = LoadModel_(PATH_CONCAT("Assets/ui/od_frame.obj"), loadedAssets);
    odBar = LoadModel_(PATH_CONCAT("Assets/ui/od_fill.obj"), loadedAssets);
    multFrame = LoadModel_(PATH_CONCAT("Assets/ui/multcircle_frame.obj"), loadedAssets);
    multBar = LoadModel_(PATH_CONCAT("Assets/ui/multcircle_fill.obj"), loadedAssets);
    multCtr3 = LoadModel_(PATH_CONCAT("Assets/ui/multbar_3.obj"), loadedAssets);
    multCtr5 = LoadModel_(PATH_CONCAT("Assets/ui/multbar_5.obj"), loadedAssets);
    multNumber = LoadModel_(PATH_CONCAT("Assets/ui/mult_number_plane.obj"), loadedAssets);
    odMultFrame = LoadTextureFilter(PATH_CONCAT("Assets/ui/mult_base.png"), loadedAssets);
    odMultFill = LoadTextureFilter(PATH_CONCAT("Assets/ui/mult_fill.png"), loadedAssets);
    odMultFillActive = LoadTextureFilter(PATH_CONCAT("Assets/ui/mult_fill_od.png"), loadedAssets);
    multNumberTex = LoadTextureFilter(PATH_CONCAT("Assets/ui/mult_number.png"), loadedAssets);

#ifdef PLATFORM_NX
    odMultShader = LoadShader(nullptr, PATH_TO_CSTR(PATH_CONCAT("Assets/ui/odmult.fs")));
    multNumberShader = LoadShader(nullptr, PATH_TO_CSTR(PATH_CONCAT("Assets/ui/multnumber.fs")));
#else
    odMultShader = LoadShader(PATH_TO_CSTR(PATH_CONCAT("gameplay/highway/fLighting.vsh")), PATH_TO_CSTR(PATH_CONCAT("Assets/ui/odmult.fs")));
    multNumberShader = LoadShader(nullptr, PATH_TO_CSTR(PATH_CONCAT("Assets/ui/multnumber.fs")));
#endif

    MultInnerDot = LoadModel_(PATH_CONCAT("gameplay/highway/multiplier/MultCenterDot.obj"), loadedAssets);
    MultFill = LoadModel_(PATH_CONCAT("gameplay/highway/multiplier/MultFill.obj"), loadedAssets);
    MultOuterFrame = LoadModel_(PATH_CONCAT("gameplay/highway/multiplier/MultOuterFrame.obj"), loadedAssets);
    MultInnerFrame = LoadModel_(PATH_CONCAT("gameplay/highway/multiplier/MultInnerFrame.obj"), loadedAssets);

    MultFillBase = LoadTextureFilter(PATH_CONCAT("gameplay/highway/multiplier/Untitled.png"), loadedAssets);
    MultFCTex1 = LoadTextureFilter(PATH_CONCAT("gameplay/highway/multiplier/od-shine.png"), loadedAssets);
    MultFCTex2 = LoadTextureFilter(PATH_CONCAT("gameplay/highway/multiplier/od-shine2.png"), loadedAssets);
    MultFCTex3 = LoadTextureFilter(PATH_CONCAT("gameplay/highway/multiplier/od-shine3.png"), loadedAssets);

    FullComboIndicator = LoadShader(nullptr, PATH_TO_CSTR(PATH_CONCAT("Assets/ui/fc_ind.fsh")));
    BottomTextureLoc = GetShaderLocation(FullComboIndicator, "baseTex");
    MiddleTextureLoc = GetShaderLocation(FullComboIndicator, "tex1");
    TopTextureLoc = GetShaderLocation(FullComboIndicator, "tex2");
    TimeLoc = GetShaderLocation(FullComboIndicator, "time");
    FCColorLoc = GetShaderLocation(FullComboIndicator, "color");
    FCIndLoc = GetShaderLocation(FullComboIndicator, "isFC");
    BasicColorLoc = GetShaderLocation(FullComboIndicator, "basicColor");
    MultInnerFrame.materials[0].shader = FullComboIndicator;

    MultiplierFill = LoadShader(nullptr, PATH_TO_CSTR(PATH_CONCAT("gameplay/highway/multiplier/MultiplierFill.fsh")));
    MultTextureLoc = GetShaderLocation(MultiplierFill, "BaseTexture");
    MultiplierColorLoc = GetShaderLocation(MultiplierFill, "MultiplierColor");
    FillPercentageLoc = GetShaderLocation(MultiplierFill, "FillPercentage");

    odLoc = GetShaderLocation(odMultShader, "overdrive");
    comboCounterLoc = GetShaderLocation(odMultShader, "comboCounter");
    multLoc = GetShaderLocation(odMultShader, "multBar");
    isBassOrVocalLoc = GetShaderLocation(odMultShader, "isBassOrVocal");
    uvOffsetXLoc = GetShaderLocation(multNumberShader, "uvOffsetX");
    uvOffsetYLoc = GetShaderLocation(multNumberShader, "uvOffsetY");

#ifdef PLATFORM_NX
    HighwayFade = LoadShader(nullptr, PATH_TO_CSTR(PATH_CONCAT("gameplay/highway/highwayFade.frag")));
#else
    HighwayFade = LoadShader(PATH_TO_CSTR(PATH_CONCAT("gameplay/highway/fLighting.vsh")), PATH_TO_CSTR(PATH_CONCAT("gameplay/highway/highwayFade.frag")));
#endif
    HighwayFadeStartLoc = GetShaderLocation(HighwayFade, "fadeStart");
    HighwayFadeEndLoc = GetShaderLocation(HighwayFade, "fadeEnd");
    HighwayColorLoc = GetShaderLocation(HighwayFade, "colorForAccent");
    HighwayAccentFadeLoc = GetShaderLocation(HighwayFade, "useInAccent");
    HighwayFade.locs[SHADER_LOC_COLOR_DIFFUSE] = HighwayColorLoc;

    HighwayColorShaderLoc = GetShaderLocation(Highway, "highwayColor");
    HighwayScrollFadeStartLoc = GetShaderLocation(Highway, "fadeStart");
    HighwayScrollFadeEndLoc = GetShaderLocation(Highway, "fadeEnd");

    expertHighwaySides = LoadModel_(PATH_CONCAT("gameplay/highway/sides_x.obj"), loadedAssets);
    DarkerHighwayThing = LoadModel_(PATH_CONCAT("gameplay/highway/highway_x.obj"), loadedAssets);
    expertHighway = LoadModel_(PATH_CONCAT("gameplay/highway/highway_x.obj"), loadedAssets);
    expertHighway.materials[0].shader = Highway;

    emhHighwaySides = LoadModel_(PATH_CONCAT("gameplay/highway/sides_emh.obj"), loadedAssets);
    emhHighway = LoadModel_(PATH_CONCAT("gameplay/highway/highway_emh.obj"), loadedAssets);
    odHighwayEMH = LoadModel_(PATH_CONCAT("gameplay/highway/overdrive_emh.obj"), loadedAssets);
    emhHighway.materials[0].shader = Highway;
    emhHighwaySides.materials[0].shader = HighwayFade;
    odHighwayEMH.materials[0].shader = Highway;
    odHighwayX = LoadModel_(PATH_CONCAT("gameplay/highway/overdrive_x.obj"), loadedAssets);
    odHighwayX.materials[0].shader = Highway;
    highwayTexture = LoadTextureFilter(PATH_CONCAT("gameplay/highway/highway.png"), loadedAssets);
    highwayTextureOD = LoadTextureFilter(PATH_CONCAT("gameplay/highway/overdrive.png"), loadedAssets);
    highwaySidesTexture = LoadTextureFilter(PATH_CONCAT("gameplay/highway/sides.png"), loadedAssets);

    noteTopModel = LoadModel_(PATH_CONCAT("Assets/notes/note_top.obj"), loadedAssets);
    noteBottomModel = LoadModel_(PATH_CONCAT("Assets/notes/note_bottom.obj"), loadedAssets);

    KickBottomModel = LoadModel_(PATH_CONCAT("Assets/notes/kick.obj"), loadedAssets);
    KickSideModel = LoadModel_(PATH_CONCAT("Assets/notes/kickSides.obj"), loadedAssets);
    KickBottom = LoadTextureFilter(PATH_CONCAT("Assets/notes/kick.png"), loadedAssets);
    KickSide = LoadTextureFilter(PATH_CONCAT("gameplay/highway/sides.png"), loadedAssets);

    CymbalInner = LoadModel_(PATH_CONCAT("Assets/notes/cymbal/CymbalWhite.obj"), loadedAssets);
    CymbalOuter = LoadModel_(PATH_CONCAT("Assets/notes/cymbal/CymbalColor.obj"), loadedAssets);
    CymbalBottom = LoadModel_(PATH_CONCAT("Assets/notes/cymbal/CymbalBottom.obj"), loadedAssets);
    CymbalInner.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
    CymbalBottom.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GRAY;

    CodaLaneTex = LoadTextureFilter(PATH_CONCAT("Assets/notes/codaLanes.png"), loadedAssets);
    CodaLane = LoadMaterialDefault();
    CodaLane.maps[MATERIAL_MAP_ALBEDO].texture = CodaLaneTex;
    CodaLane.shader = HighwayFade;

    SoloSides = LoadMaterialDefault();
    SoloSides.maps[MATERIAL_MAP_ALBEDO].texture = soloTexture;
    SoloSides.shader = HighwayFade;

    YargRings.push_back(LoadTextureFilter(PATH_CONCAT("Assets/ui/hugh ring/rings-1.png"), loadedAssets));
    YargRings.push_back(LoadTextureFilter(PATH_CONCAT("Assets/ui/hugh ring/rings-2.png"), loadedAssets));
    YargRings.push_back(LoadTextureFilter(PATH_CONCAT("Assets/ui/hugh ring/rings-3.png"), loadedAssets));
    YargRings.push_back(LoadTextureFilter(PATH_CONCAT("Assets/ui/hugh ring/rings-4.png"), loadedAssets));
    YargRings.push_back(LoadTextureFilter(PATH_CONCAT("Assets/ui/hugh ring/rings-5.png"), loadedAssets));
    YargRings.push_back(LoadTextureFilter(PATH_CONCAT("Assets/ui/hugh ring/rings-6.png"), loadedAssets));

    BaseRingTexture = LoadTextureFilter(PATH_CONCAT("Assets/ui/hugh ring/rings.png"), loadedAssets);

    InstIcons.push_back(LoadTextureFilter(PATH_CONCAT("Assets/ui/hugh ring/drums-inv.png"), loadedAssets));
    InstIcons.push_back(LoadTextureFilter(PATH_CONCAT("Assets/ui/hugh ring/bass-inv.png"), loadedAssets));
    InstIcons.push_back(LoadTextureFilter(PATH_CONCAT("Assets/ui/hugh ring/lead-inv.png"), loadedAssets));
    InstIcons.push_back(LoadTextureFilter(PATH_CONCAT("Assets/ui/hugh ring/keys-inv.png"), loadedAssets));
    InstIcons.push_back(LoadTextureFilter(PATH_CONCAT("Assets/ui/hugh ring/vox-inv.png"), loadedAssets));

    SoloBox = LoadModel_(PATH_CONCAT("gameplay/highway/Solo.obj"), loadedAssets);
    SoloBackground = LoadTextureFilter(PATH_CONCAT("gameplay/highway/SoloBox.png"), loadedAssets);
    SoloBox.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = SoloBackground;

    Scorebox = LoadTextureFilter(PATH_CONCAT("Assets/gameplay/ui/Scorebox.png"), loadedAssets);
    Timerbox = LoadTextureFilter(PATH_CONCAT("Assets/gameplay/ui/Timerbox.png"), loadedAssets);
    TimerboxOutline = LoadTextureFilter(PATH_CONCAT("Assets/gameplay/ui/TimerboxOutline.png"), loadedAssets);

    noteTopModelOD = LoadModel_(PATH_CONCAT("Assets/notes/note_top_od.obj"), loadedAssets);
    noteBottomModelOD = LoadModel_(PATH_CONCAT("Assets/notes/note_bottom.obj"), loadedAssets);

    noteTopModelHP = LoadModel_(PATH_CONCAT("Assets/notes/hopo_top.obj"), loadedAssets);
    noteBottomModelHP = LoadModel_(PATH_CONCAT("Assets/notes/hopo_bottom.obj"), loadedAssets);

    noteTextureOD = LoadTextureFilter(PATH_CONCAT("Assets/notes/note.png"), loadedAssets);
    emitTextureOD = LoadTextureFilter(PATH_CONCAT("Assets/notes/note_e_new.png"), loadedAssets);

    liftModel = LoadModel_(PATH_CONCAT("Assets/notes/lift.obj"), loadedAssets);
    liftModelOD = LoadModel_(PATH_CONCAT("Assets/notes/lift.obj"), loadedAssets);

    beatline = LoadModel_(PATH_CONCAT("gameplay/highway/beatline.obj"), loadedAssets);
    beatlineTex = LoadTextureFilter(PATH_CONCAT("gameplay/highway/beatline.png"), loadedAssets);
    beatline.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = beatlineTex;

    songBackground = LoadTextureFilter(PATH_CONCAT("Assets/background.png"), loadedAssets);

    redHatMono = LoadFontFilter(PATH_CONCAT("Assets/fonts/RedHatMono-Bold.ttf"), 256, loadedAssets);
    fxaa = LoadShader(nullptr, PATH_TO_CSTR(PATH_CONCAT("Assets/ui/fxaa.frag")));
    texLoc = GetShaderLocation(fxaa, "texture0");
    resLoc = GetShaderLocation(fxaa, "resolution");
    sdfShader = LoadShader(nullptr, PATH_TO_CSTR(PATH_CONCAT("Assets/fonts/sdf.fs")));
    bgShader = LoadShader(nullptr, PATH_TO_CSTR(PATH_CONCAT("Assets/ui/wavy.fs")));
    bgTimeLoc = GetShaderLocation(bgShader, "time");

#ifdef PLATFORM_NX
    clapOD = LoadSound(PATH_TO_CSTR(PATH_CONCAT("gameplay/highway/clap.ogg")));
    SetSoundVolume(clapOD, 0.375f);
#else
    // BASS audio code for non-Switch platforms (replace with your existing BASS implementation)
#endif

#ifndef PLATFORM_NX
    discord = LoadTextureFilter(PATH_CONCAT("Assets/ui/discord-mark-white.png"), loadedAssets);
    github = LoadTextureFilter(PATH_CONCAT("Assets/ui/github-mark-white.png"), loadedAssets);
#else
    discord = { 0 }; // Empty texture
    github = { 0 };  // Empty texture
#endif

    soloTexture = LoadTextureFilter(PATH_CONCAT("gameplay/highway/overdrive-old.png"), loadedAssets);
    sustainTexture = LoadTextureFilter(PATH_CONCAT("Assets/notes/sustain.png"), loadedAssets);
    sustainHeldTexture = LoadTextureFilter(PATH_CONCAT("Assets/notes/sustain-held.png"), loadedAssets);

    smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherPressTex;
    smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;

    smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherBoardTex;
    smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GRAY;

    smasherBoardEMH.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherBoardTex;
    smasherBoardEMH.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GRAY;

    lanes.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = lanesTex;
    lanes.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;

    odFrame.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
    odFrame.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;
    odBar.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
    odBar.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;
    multFrame.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
    multFrame.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;
    multBar.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
    multBar.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;
    multCtr3.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
    multCtr3.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;
    multCtr5.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
    multCtr5.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;
    odBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
    multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
    multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
    multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
    odBar.materials[0].shader = odMultShader;
    multBar.materials[0].shader = odMultShader;
    multCtr3.materials[0].shader = odMultShader;
    multCtr5.materials[0].shader = odMultShader;

    odMultShader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(odMultShader, "fillTex");

    multNumber.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = multNumberTex;
    multNumber.materials[0].shader = multNumberShader;

    MultFill.materials[0].shader = MultiplierFill;

    odHighwayEMH.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTextureOD;
    odHighwayX.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTextureOD;
    odHighwayX.materials[0].maps[MATERIAL_MAP_ALBEDO].color = overdriveColor;
    odHighwayEMH.materials[0].maps[MATERIAL_MAP_ALBEDO].color = overdriveColor;
    expertHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwaySidesTexture;
    expertHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;
    expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
    expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GRAY;
    emhHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwaySidesTexture;
    emhHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;
    emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
    emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GRAY;

    noteTopModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;
    noteBottomModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;

    KickBottomModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = KickBottom;
    KickSideModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = KickSide;

    noteTopModelOD.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
    noteBottomModelOD.materials[0].maps[MATERIAL_MAP_ALBEDO].color = overdriveColor;

    noteTopModelHP.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;
    noteBottomModelHP.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;

    liftModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;
    liftModelOD.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;

    sustainMat = LoadMaterialDefault();
    soloMat = LoadMaterialDefault();
    sustainMatHeld = LoadMaterialDefault();
    sustainMatOD = LoadMaterialDefault();
    sustainMatHeldOD = LoadMaterialDefault();
    sustainMatMiss = LoadMaterialDefault();
    soloMat.maps[MATERIAL_MAP_DIFFUSE].texture = soloTexture;
    soloMat.maps[MATERIAL_MAP_DIFFUSE].color = SKYBLUE;
    sustainMat.maps[MATERIAL_MAP_DIFFUSE].texture = sustainTexture;
    sustainMat.maps[MATERIAL_MAP_DIFFUSE].color = ColorTint(accentColor, { 180, 180, 180, 255 });
    sustainMatHeld.maps[MATERIAL_MAP_EMISSION].texture = sustainHeldTexture;
    sustainMatHeld.maps[MATERIAL_MAP_EMISSION].color = WHITE;
    sustainMatHeld.maps[MATERIAL_MAP_EMISSION].value = 1;
    sustainMatHeld.maps[MATERIAL_MAP_DIFFUSE].texture = sustainHeldTexture;
    sustainMatHeld.maps[MATERIAL_MAP_DIFFUSE].color = accentColor;
    sustainMatOD.maps[MATERIAL_MAP_DIFFUSE].texture = sustainTexture;
    sustainMatOD.maps[MATERIAL_MAP_DIFFUSE].color = { 180, 180, 180, 255 };
    sustainMatHeldOD.maps[MATERIAL_MAP_DIFFUSE].texture = sustainHeldTexture;
    sustainMatHeldOD.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    sustainMatMiss.maps[MATERIAL_MAP_DIFFUSE].texture = sustainTexture;
    sustainMatMiss.maps[MATERIAL_MAP_DIFFUSE].color = DARKGRAY;
}