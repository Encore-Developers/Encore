//
// Created by marie on 03/09/2024.
//

#ifndef CACHELOADINGSCREEN_H
#define CACHELOADINGSCREEN_H
#include "menu.h"
#include "raylib.h"

class cacheLoadingScreen : public Menu {
    Texture2D encoreLogo;
    Font RedHatDisplay;
    Font RubikBold;
    Font JosefinSansItalic;
    int SplashSel;
    Texture2D LoadingScreenBackground;

public:
    cacheLoadingScreen();
    virtual ~cacheLoadingScreen();
    virtual void Draw();
    virtual void Load();
};

#endif // CACHELOADINGSCREEN_H
