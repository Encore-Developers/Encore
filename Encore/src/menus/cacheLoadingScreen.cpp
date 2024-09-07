//
// Created by marie on 03/09/2024.
//

#include "cacheLoadingScreen.h"

#include <filesystem>
#include <thread>

#include "gameMenu.h"
#include "raymath.h"
#include "settings.h"
#include "uiUnits.h"
#include "song/songlist.h"
#include "raygui.h"

Font LoadFontFilter(const std::filesystem::path &fontPath, int fontSize) {
	Font font = LoadFontEx(fontPath.string().c_str(), fontSize, 0, 250);
	font.baseSize = 128;
	font.glyphCount = 250;
	int fileSize = 0;
	unsigned char* fileData = LoadFileData(fontPath.string().c_str(), &fileSize);
	font.glyphs = LoadFontData(fileData, fileSize, 128, 0, 250, FONT_SDF);
	Image atlas = GenImageFontAtlas(font.glyphs, &font.recs, 250, 128, 4, 1);
	font.texture = LoadTextureFromImage(atlas);
	UnloadImage(atlas);
	SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
	return font;
}

void cacheLoadingScreen::DrawTextRubikBold(const char* text, float posX, float posY, float fontSize, Color color)  {
	BeginShaderMode(sdfShader);
	DrawTextEx(RubikBold, text, { posX,posY }, fontSize, 0, color);
	EndShaderMode();
}

void cacheLoadingScreen::DrawTextJosefinSansItalic(const char *text, float posX, float posY, float fontSize,
	Color color) {
	BeginShaderMode(sdfShader);
	DrawTextEx(JosefinSansItalic, text, { posX,posY }, fontSize, 0, color);
	EndShaderMode();
}

void cacheLoadingScreen::DrawTextRedHatDisplay(const char* text, float posX, float posY, float fontSize, Color color)  {
	BeginShaderMode(sdfShader);
	DrawTextEx(RedHatDisplay, text, { posX,posY }, fontSize, 0, color);
	EndShaderMode();
}

Texture2D LoadTextureFilter(const std::filesystem::path &texturePath) {
	Texture2D tex = LoadTexture(texturePath.string().c_str());
	GenTextureMipmaps(&tex);
	SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
	return tex;
}

std::vector<std::string> CacheSplash = {
	"Want a break from the cache?",
	"Make sure to join the Discord if you havent!",
	"Remember when you could make coffee during these loading screens?",
	"When's the next train?",
	"Sorry for holding up the party!",
	"You can skip this screen by pressing Alt-F4!",
	"You're gonna be here for a bit..."
};

void cacheLoadingScreen::Load() {
	std::filesystem::path assetsdir = GetApplicationDirectory();
	assetsdir /= "Assets";
	sdfShader = LoadShader(0, (assetsdir / "fonts/sdf.fs").string().c_str());
	RedHatDisplay = LoadFontFilter(assetsdir / "fonts/RedHatDisplay-Black.ttf", 128);
	RubikBold = LoadFontFilter(assetsdir / "fonts/Rubik-Bold.ttf", 128);
	JosefinSansItalic = LoadFontFilter(assetsdir / "fonts/JosefinSans-Italic.ttf", 128);
	encoreLogo = LoadTextureFilter(assetsdir / "encore_favicon-NEW.png");
	SplashSel = GetRandomValue(0, CacheSplash.size()-1);
}

// todo(3drosalia): make another class for drawing these things without having to uh. implement it in every menu class
bool finished = false;
bool started = false;

void LoadCache() {
	SongList& list = SongList::getInstance();
	Settings& settings = Settings::getInstance();
	list.LoadCache(settings.songPaths);
	finished = true;
}



void cacheLoadingScreen::Draw() {
	Units u = Units::getInstance();
	TheGameMenu.DrawTopOvershell(0.15f);
	// float logoHeight = u.hinpct(0.145f);
	// float logoWidth = Remap(encoreLogo.height, 0, encoreLogo.width / 4.25, 0, u.winpct(0.5f));
	// Rectangle LogoRect = { u.RightSide - u.winpct(0.01f) - logoWidth, u.hpct(0.035f), logoWidth, logoHeight};
	// DrawTexturePro(encoreLogo, {0,0,(float)encoreLogo.width,(float)encoreLogo.height}, LogoRect, {0,0}, 0, WHITE);
	DrawRectangle(0, u.hpct(0.15f), Remap(CurrentChartNumber, 0, MaxChartsToLoad, 0, GetScreenWidth()),
				  u.hinpct(0.01f), MAGENTA);

	DrawTextRedHatDisplay("LOADING CACHE", u.LeftSide, u.hpct(0.05f),
							u.hinpct(0.125f), WHITE);
	float RubikFontSize = u.hinpct(0.05f);
	int loaded = CurrentChartNumber;
	int toLoad = MaxChartsToLoad;
	std::string LoadingText = TextFormat("%d/%d songs loaded", loaded, toLoad);
	float lwidth = MeasureTextEx(RubikBold, LoadingText.c_str(), RubikFontSize, 0).x;
	DrawTextRubikBold(LoadingText.c_str(), u.RightSide - lwidth, u.hpct(0.085f),
							RubikFontSize, LIGHTGRAY);
	TheGameMenu.DrawBottomOvershell();

	Rectangle LogoRect = { u.LeftSide + u.hinpct(0.075f), GetScreenHeight() - u.hpct(0.14f) + u.hinpct(0.07f), u.hinpct(0.14f), u.hinpct(0.14f)};
	DrawTexturePro(encoreLogo, {0,0,(float)encoreLogo.width,(float)encoreLogo.height}, LogoRect, {u.hinpct(0.07f),u.hinpct(0.07f)}, 0, WHITE);
	DrawTextJosefinSansItalic(CacheSplash[SplashSel].c_str(), u.LeftSide + u.hinpct(0.16), GetScreenHeight() - u.hpct(0.14f) + u.hinpct(0.055f),  RubikFontSize/1.5f, WHITE);
	if (!started) {
		started = true;
		std::thread CacheLoader(LoadCache);
		CacheLoader.detach();
		TheGameMenu.songsLoaded = true;
	}
	if (finished)
		TheGameMenu.SwitchScreen(MENU);
}

cacheLoadingScreen::~cacheLoadingScreen() {}
cacheLoadingScreen::cacheLoadingScreen() {}


