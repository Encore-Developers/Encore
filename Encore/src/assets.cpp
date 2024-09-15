//
// Created by marie on 02/05/2024.
//

#include "assets.h"
#include <filesystem>
#include "raygui.h"

class Assets;

Texture2D
Assets::LoadTextureFilter(const std::filesystem::path &texturePath, int &loadedAssets) {
    Texture2D tex = LoadTexture(texturePath.string().c_str());
    GenTextureMipmaps(&tex);
    SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
    loadedAssets++;
    return tex;
}

Model Assets::LoadModel_(const std::filesystem::path &modelPath, int &loadedAssets) {
    return LoadModel(modelPath.string().c_str());
    loadedAssets++;
}

Font Assets::LoadFontFilter(
    const std::filesystem::path &fontPath, int fontSize, int &loadedAssets
) {
    Font font = LoadFontEx(fontPath.string().c_str(), fontSize, 0, 250);
    font.baseSize = 128;
    font.glyphCount = 250;
    int fileSize = 0;
    unsigned char *fileData = LoadFileData(fontPath.string().c_str(), &fileSize);
    font.glyphs = LoadFontData(fileData, fileSize, 128, 0, 250, FONT_SDF);
    Image atlas = GenImageFontAtlas(font.glyphs, &font.recs, 250, 128, 4, 1);
    font.texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);
    SetTextureFilter(font.texture, TEXTURE_FILTER_TRILINEAR);
    loadedAssets++;
    return font;
}
void Assets::FirstAssets() {
    icon = LoadImage((directory / "Assets/encore_favicon-NEW.png").string().c_str());
    encoreWhiteLogo =
        Assets::LoadTextureFilter((directory / "Assets/encore-white.png"), loadedAssets);
    rubik = Assets::LoadFontFilter(
        (directory / "Assets/fonts/Rubik-Regular.ttf"), 256, loadedAssets
    );
}
void Assets::LoadAssets() {
    Color accentColor = { 255, 0, 255, 255 };
    Color overdriveColor = Color { 255, 200, 0, 255 };
    smasherReg =
        Assets::LoadModel_((directory / "Assets/highway/smasher.obj"), loadedAssets);
    smasherRegTex =
        LoadTexture((directory / "Assets/highway/smasher_reg.png").string().c_str());

    smasherBoardTex =
        Assets::LoadTextureFilter(directory / "Assets/highway/board.png", loadedAssets);
    smasherBoard =
        Assets::LoadModel_((directory / "Assets/highway/board_x.obj"), loadedAssets);
    smasherBoardEMH =
        Assets::LoadModel_((directory / "Assets/highway/board_emh.obj"), loadedAssets);

    lanes = Assets::LoadModel_((directory / "Assets/highway/lanes.obj"), loadedAssets);
    lanesTex =
        Assets::LoadTextureFilter(directory / "Assets/highway/lanes.png", loadedAssets);

    smasherPressed =
        Assets::LoadModel_((directory / "Assets/highway/smasher.obj"), loadedAssets);
    smasherPressTex =
        LoadTexture((directory / "Assets/highway/smasher_press.png").string().c_str());

    star = Assets::LoadTextureFilter(directory / "Assets/ui/star.png", loadedAssets);
    goldStar =
        Assets::LoadTextureFilter(directory / "Assets/ui/gold-star.png", loadedAssets);
    goldStarUnfilled = Assets::LoadTextureFilter(
        directory / "Assets/ui/gold-star_unfilled.png", loadedAssets
    );
    emptyStar =
        Assets::LoadTextureFilter(directory / "Assets/ui/empty-star.png", loadedAssets);

    Highway = LoadShader(
        (directory / "Assets/highway/fLighting.vsh").string().c_str(),
        (directory / "Assets/highway/highwayShader.fsh").string().c_str()
    );
    HighwayTexShaderLoc = GetShaderLocation(Highway, "highwayTex");
    HighwayTimeShaderLoc = GetShaderLocation(Highway, "time");

    odFrame = Assets::LoadModel_((directory / "Assets/ui/od_frame.obj"), loadedAssets);
    odBar = Assets::LoadModel_((directory / "Assets/ui/od_fill.obj"), loadedAssets);
    multFrame =
        Assets::LoadModel_((directory / "Assets/ui/multcircle_frame.obj"), loadedAssets);
    multBar =
        Assets::LoadModel_((directory / "Assets/ui/multcircle_fill.obj"), loadedAssets);
    multCtr3 = Assets::LoadModel_((directory / "Assets/ui/multbar_3.obj"), loadedAssets);
    multCtr5 = Assets::LoadModel_((directory / "Assets/ui/multbar_5.obj"), loadedAssets);
    multNumber =
        Assets::LoadModel_((directory / "Assets/ui/mult_number_plane.obj"), loadedAssets);
    odMultFrame =
        Assets::LoadTextureFilter(directory / "Assets/ui/mult_base.png", loadedAssets);
    odMultFill =
        Assets::LoadTextureFilter(directory / "Assets/ui/mult_fill.png", loadedAssets);
    odMultFillActive =
        Assets::LoadTextureFilter(directory / "Assets/ui/mult_fill_od.png", loadedAssets);
    multNumberTex =
        Assets::LoadTextureFilter(directory / "Assets/ui/mult_number.png", loadedAssets);
    odMultShader = LoadShader(0, "Assets/ui/odmult.fs");
    multNumberShader = LoadShader(0, "Assets/ui/multnumber.fs");

    MultInnerDot = Assets::LoadModel_(
        (directory / "Assets/highway/multiplier/MultCenterDot.obj"), loadedAssets
    );
    MultFill = Assets::LoadModel_(
        (directory / "Assets/highway/multiplier/MultFill.obj"), loadedAssets
    );
    MultOuterFrame = Assets::LoadModel_(
        (directory / "Assets/highway/multiplier/MultOuterFrame.obj"), loadedAssets
    );
    MultInnerFrame = Assets::LoadModel_(
        (directory / "Assets/highway/multiplier/MultInnerFrame.obj"), loadedAssets
    );

    MultFillBase = Assets::LoadTextureFilter(
        (directory / "Assets/highway/multiplier/Untitled.png"), loadedAssets
    );
    MultFCTex1 = Assets::LoadTextureFilter(
        (directory / "Assets/highway/multiplier/od shine.png"), loadedAssets
    );
    MultFCTex2 = Assets::LoadTextureFilter(
        (directory / "Assets/highway/multiplier/od shine2.png"), loadedAssets
    );
    MultFCTex3 = Assets::LoadTextureFilter(
        (directory / "Assets/highway/multiplier/od shine3.png"), loadedAssets
    );

    FullComboIndicator = LoadShader(0, "Assets/ui/fc_ind.fsh");
    BottomTextureLoc = GetShaderLocation(FullComboIndicator, "baseTex");
    MiddleTextureLoc = GetShaderLocation(FullComboIndicator, "tex1");
    TopTextureLoc = GetShaderLocation(FullComboIndicator, "tex2");
    TimeLoc = GetShaderLocation(FullComboIndicator, "time");
    FCColorLoc = GetShaderLocation(FullComboIndicator, "color");
    FCIndLoc = GetShaderLocation(FullComboIndicator, "isFC");
    BasicColorLoc = GetShaderLocation(FullComboIndicator, "basicColor");
    MultInnerFrame.materials[0].shader = FullComboIndicator;

    MultiplierFill = LoadShader(0, "Assets/highway/multiplier/MultiplierFill.fsh");

    MultTextureLoc = GetShaderLocation(MultiplierFill, "BaseTexture"); // sampler
    MultiplierColorLoc = GetShaderLocation(MultiplierFill, "MultiplierColor"); // vec4
    FillPercentageLoc = GetShaderLocation(MultiplierFill, "FillPercentage"); // 0-1 float

    odLoc = GetShaderLocation(odMultShader, "overdrive");
    comboCounterLoc = GetShaderLocation(odMultShader, "comboCounter");
    multLoc = GetShaderLocation(odMultShader, "multBar");
    isBassOrVocalLoc = GetShaderLocation(odMultShader, "isBassOrVocal");
    uvOffsetXLoc = GetShaderLocation(multNumberShader, "uvOffsetX");
    uvOffsetYLoc = GetShaderLocation(multNumberShader, "uvOffsetY");

    HighwayFade = LoadShader(
        (directory / "Assets/highway/fLighting.vsh").string().c_str(),
        (directory / "Assets/highway/highwayFade.frag").string().c_str()
    );
    HighwayFadeStartLoc = GetShaderLocation(HighwayFade, "fadeStart");
    HighwayFadeEndLoc = GetShaderLocation(HighwayFade, "fadeEnd");
    HighwayColorLoc = GetShaderLocation(HighwayFade, "colorForAccent");
    HighwayAccentFadeLoc = GetShaderLocation(HighwayFade, "useInAccent");

    Highway.locs[SHADER_UNIFORM_SAMPLER2D] = GetShaderLocation(Highway, "highwayTex");
    HighwayColorShaderLoc = GetShaderLocation(Highway, "highwayColor");
    HighwayScrollFadeStartLoc = GetShaderLocation(Highway, "fadeStart");
    HighwayScrollFadeEndLoc = GetShaderLocation(Highway, "fadeEnd");

    expertHighwaySides =
        Assets::LoadModel_(directory / "Assets/highway/sides_x.obj", loadedAssets);
    expertHighway =
        Assets::LoadModel_((directory / "Assets/highway/highway_x.obj"), loadedAssets);
    expertHighway.materials[0].shader = Highway;
    emhHighwaySides =
        Assets::LoadModel_((directory / "Assets/highway/sides_emh.obj"), loadedAssets);
    emhHighway =
        Assets::LoadModel_((directory / "Assets/highway/highway_emh.obj"), loadedAssets);
    odHighwayEMH = Assets::LoadModel_(
        (directory / "Assets/highway/overdrive_emh.obj"), loadedAssets
    );
    odHighwayX =
        Assets::LoadModel_((directory / "Assets/highway/overdrive_x.obj"), loadedAssets);
    highwayTexture =
        Assets::LoadTextureFilter(directory / "Assets/highway/highway.png", loadedAssets);
    highwayTextureOD = Assets::LoadTextureFilter(
        directory / "Assets/highway/overdrive.png", loadedAssets
    );
    highwaySidesTexture =
        Assets::LoadTextureFilter(directory / "Assets/highway/sides.png", loadedAssets);

    noteTopModel =
        Assets::LoadModel_((directory / "Assets/notes/note_top.obj"), loadedAssets);
    noteBottomModel =
        Assets::LoadModel_((directory / "Assets/notes/note_bottom.obj"), loadedAssets);

    KickBottomModel =
        Assets::LoadModel_((directory / "Assets/notes/kick.obj"), loadedAssets);
    KickSideModel =
        Assets::LoadModel_((directory / "Assets/notes/kickSides.obj"), loadedAssets);
    KickBottom =
        Assets::LoadTextureFilter(directory / "Assets/notes/kick.png", loadedAssets);
    KickSide =
        Assets::LoadTextureFilter(directory / "Assets/highway/sides.png", loadedAssets);

    CymbalInner = Assets::LoadModel_(
        (directory / "Assets/notes/cymbal/CymbalWhite.obj"), loadedAssets
    );
    CymbalOuter = Assets::LoadModel_(
        (directory / "Assets/notes/cymbal/CymbalColor.obj"), loadedAssets
    );
    CymbalBottom = Assets::LoadModel_(
        (directory / "Assets/notes/cymbal/CymbalBottom.obj"), loadedAssets
    );
    CymbalInner.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
    CymbalBottom.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GRAY;

    YargRings.push_back(
        LoadTextureFilter((directory / "Assets/ui/hugh ring/rings-1.png"), loadedAssets)
    );
    YargRings.push_back(
        LoadTextureFilter((directory / "Assets/ui/hugh ring/rings-2.png"), loadedAssets)
    );
    YargRings.push_back(
        LoadTextureFilter((directory / "Assets/ui/hugh ring/rings-3.png"), loadedAssets)
    );
    YargRings.push_back(
        LoadTextureFilter((directory / "Assets/ui/hugh ring/rings-4.png"), loadedAssets)
    );
    YargRings.push_back(
        LoadTextureFilter((directory / "Assets/ui/hugh ring/rings-5.png"), loadedAssets)
    );
    YargRings.push_back(
        LoadTextureFilter((directory / "Assets/ui/hugh ring/rings-6.png"), loadedAssets)
    );

    BaseRingTexture =
        LoadTextureFilter((directory / "Assets/ui/hugh ring/rings.png"), loadedAssets);

    InstIcons.push_back(
        LoadTextureFilter((directory / "Assets/ui/hugh ring/bass-inv.png"), loadedAssets)
    );
    InstIcons.push_back(
        LoadTextureFilter((directory / "Assets/ui/hugh ring/drums-inv.png"), loadedAssets)
    );
    InstIcons.push_back(
        LoadTextureFilter((directory / "Assets/ui/hugh ring/lead-inv.png"), loadedAssets)
    );
    InstIcons.push_back(
        LoadTextureFilter((directory / "Assets/ui/hugh ring/vox-inv.png"), loadedAssets)
    );

    SoloBox = Assets::LoadModel_((directory / "Assets/highway/Solo.obj"), loadedAssets);
    SoloBackground =
        Assets::LoadTextureFilter(directory / "Assets/highway/SoloBox.png", loadedAssets);
    SoloBox.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = SoloBackground;

    // noteTexture = Assets::LoadTextureFilter(directory / "Assets/notes/note.png",
    // loadedAssets); emitTexture = Assets::LoadTextureFilter(directory /
    // "Assets/notes/note_e_new.png", loadedAssets);

    noteTopModelOD =
        Assets::LoadModel_((directory / "Assets/notes/note_top_od.obj"), loadedAssets);
    noteBottomModelOD =
        Assets::LoadModel_((directory / "Assets/notes/note_bottom.obj"), loadedAssets);

    noteTopModelHP =
        Assets::LoadModel_((directory / "Assets/notes/hopo_top.obj"), loadedAssets);
    noteBottomModelHP =
        Assets::LoadModel_((directory / "Assets/notes/hopo_bottom.obj"), loadedAssets);

    noteTextureOD =
        Assets::LoadTextureFilter(directory / "Assets/notes/note.png", loadedAssets);
    emitTextureOD = Assets::LoadTextureFilter(
        directory / "Assets/notes/note_e_new.png", loadedAssets
    );

    liftModel = Assets::LoadModel_((directory / "Assets/notes/lift.obj"), loadedAssets);
    liftModelOD = Assets::LoadModel_((directory / "Assets/notes/lift.obj"), loadedAssets);

    beatline = LoadModel_((directory / "Assets/highway/beatline.obj"), loadedAssets);
    beatlineTex =
        LoadTextureFilter((directory / "Assets/highway/beatline.png"), loadedAssets);
    beatline.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = beatlineTex;

    songBackground =
        Assets::LoadTextureFilter((directory / "Assets/background.png"), loadedAssets);

    redHatDisplayItalic = Assets::LoadFontFilter(
        (directory / "Assets/fonts/RedHatDisplay-BlackItalic.ttf"), 256, loadedAssets
    );
    redHatDisplayItalicLarge = Assets::LoadFontFilter(
        (directory / "Assets/fonts/RedHatDisplay-BlackItalic.ttf"), 256, loadedAssets
    );
    redHatDisplayBlack = Assets::LoadFontFilter(
        (directory / "Assets/fonts/RedHatDisplay-Black.ttf"), 256, loadedAssets
    );

    rubikBoldItalic = Assets::LoadFontFilter(
        (directory / "Assets/fonts/Rubik-BoldItalic.ttf"), 256, loadedAssets
    );
    rubikBold = Assets::LoadFontFilter(
        (directory / "Assets/fonts/Rubik-Bold.ttf"), 256, loadedAssets
    );
    rubikItalic = Assets::LoadFontFilter(
        (directory / "Assets/fonts/Rubik-Italic.ttf"), 256, loadedAssets
    );

    josefinSansItalic = Assets::LoadFontFilter(
        (directory / "Assets/fonts/JosefinSans-Italic.ttf"), 256, loadedAssets
    );

    fxaa = LoadShader(0, (directory / "Assets/ui/fxaa.frag").string().c_str());
    texLoc = GetShaderLocation(fxaa, "texture0");
    resLoc = GetShaderLocation(fxaa, "resolution");
    sdfShader = LoadShader(0, (directory / "Assets/fonts/sdf.fs").string().c_str());
    bgShader = LoadShader(0, (directory / "Assets/ui/wavy.fs").string().c_str());
    bgTimeLoc = GetShaderLocation(bgShader, "time");
    // clapOD = LoadSound((directory / "Assets/highway/clap.ogg"));
    // SetSoundVolume(clapOD, 0.375);

    discord = Assets::LoadTextureFilter(
        directory / "Assets/ui/discord-mark-white.png", loadedAssets
    );
    github = Assets::LoadTextureFilter(
        directory / "Assets/ui/github-mark-white.png", loadedAssets
    );

    soloTexture =
        Assets::LoadTextureFilter(directory / "Assets/highway/solo.png", loadedAssets);
    sustainTexture =
        Assets::LoadTextureFilter(directory / "Assets/notes/sustain.png", loadedAssets);
    sustainHeldTexture = Assets::LoadTextureFilter(
        directory / "Assets/notes/sustain-held.png", loadedAssets
    );

    smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherRegTex;
    smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;
    smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherRegTex;
    smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;

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

    odMultShader.locs[SHADER_LOC_MAP_EMISSION] =
        GetShaderLocation(odMultShader, "fillTex");

    multNumber.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = multNumberTex;
    multNumber.materials[0].shader = multNumberShader;

    MultFill.materials[0].shader = MultiplierFill;

    SetTextureWrap(highwayTextureOD, TEXTURE_WRAP_CLAMP);
    odHighwayX.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTextureOD;
    odHighwayEMH.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTextureOD;

    odHighwayX.materials[0].maps[MATERIAL_MAP_ALBEDO].color = overdriveColor;
    odHighwayEMH.materials[0].maps[MATERIAL_MAP_ALBEDO].color = overdriveColor;
    expertHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].texture =
        highwaySidesTexture;
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
    sustainMat.maps[MATERIAL_MAP_DIFFUSE].color =
        ColorTint(accentColor, { 180, 180, 180, 255 });
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
