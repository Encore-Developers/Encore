# Encore

[![Chat with us on Discord](https://cdn.jsdelivr.net/npm/@intergrav/devins-badges@3/assets/cozy/social/discord-plural_vector.svg)](https://discord.gg/GhkgVUAC9v)

Up-and-coming 4/5 key rhythm game.

Encore supports controllers and keyboard. Songs formatted for other games will NOT work with Encore.

## Installation

### Stable releases

Download from [GitHub Releases](https://github.com/Encore-Developers/Encore-Raylib/releases). 

### Nightly releases

Current nightly build:    
- [Windows 64 bit](https://nightly.link/Encore-Developers/Encore/workflows/build/main/Encore_Win_x64.zip) ([32 bit](https://nightly.link/Encore-Developers/Encore/workflows/build/main/Encore_Win_x86.zip))    
- [Linux x64 bit](https://nightly.link/Encore-Developers/Encore/workflows/build/main/Encore_Linux_x64.zip)    
### Running Encore

Extract the .zip file that you downloaded, either Nightly or Stable, and extract it into a folder that you will be able to remember. If Windows Defender SmartScreen appears, click Read More, and Run. Encore is not a malicious program, and removing the SmartScreen popup would cost us (the developers) more than we're willing to pay.

## Songs

Extract songs into the `/Songs` folder in your Encore folder. They should consist of several files:
Please check [CHARTS.md](https://github.com/Encore-Developers/Encore/blob/main/CHARTS.md) for more song information.
- info.json - the information file required for Encore to read audio, difficulties, the name, and file names.
- lead.ogg (or similar)
- bass.ogg (or similar)
- drums.ogg (or similar)
- vocals.ogg (or similar)
- backing.ogg (or similar)
- notes.mid (typically named after the song's name)

### Contributing

If you have any bugs or suggestions for Encore, please consider making an issue.
For code contributions, you'll need the latest version of Visual Studio or CLion. It's possible that you can use VSCode, but none of us develop with it. These IDEs should install the C++ development toolchains.

## Credits    
Crown icon from [Font Awesome](https://fontawesome.com/) (icon source: [crown](https://fontawesome.com/icons/crown?f=classic&s=solid))



### External libraries used
[raylib](https://github.com/raysan5/raylib)

[raygui](https://github.com/raysan5/raygui)

[RapidJSON](https://github.com/Tencent/rapidjson)

[midifile](https://github.com/craigsapp/midifile)

[nicolausYes's easing-functions](https://github.com/nicolausYes/easing-functions)

[BASS](https://www.un4seen.com/bass.html)

[PicoSHA2](https://github.com/okdshin/PicoSHA2)

[osu! resources](https://github.com/ppy/osu-resources/)
