//
// Created by marie on 04/05/2024.
//

#ifndef ENCORE_PLAYER_H
#define ENCORE_PLAYER_H

#include <string>
#include <filesystem>

/*

 note! this is kinda just me throwing shit at the wall to see what makes sense.
 kinda just makin the points and then connecting them later to fit into Encore itself
 this shouldnt exactly impede on builds yet i think
 also yes i know i should probably put the band and player stuff in their own headers
 ill do that later once i got this theorized. already have band.h made so once i get to that point ill slap it there
 its just here for convenience.


*/

// realizing how i could just make a "SelectableEntity" class and then extend Band and Player from it instead of having
// the logic rewritten between the two

class SelectableEntity {
    std::string Name;
    std::filesystem::path SettingsFile;
    std::filesystem::path ScoreFile;
};

// acts as an individual save-file
class Player: public SelectableEntity {
    bool Online;
    int PlayerNum; // zero indexed. local would be 0-3, online would be 4-7.
                   // NOTE! this is only for like. local information and
                   // not actually shared information. i was thinking of a UUID system for online


};

// acts like a system-wide save-file
// think AC:NH islands
// note: will we really have PVP stuff? like. thinking like RB3 here, would there be PVP attributed to bands?
// cuz i think it would severely complicate it if PvP stuff was more "oh this band won with these players" instead of
// just noting in a save file "p1 and p3 worked together and won against p2 and p4" instead of "band 1 with p1 and p3 won
// over band 2 with p2 and p4, especially when that stuff will probably add a win count to players who didnt even
// participate but won (because they were part of the band)
// literally every team game i can think of thats pvp doesnt really do this unless its strict about teams i think
// correct me if im wrong
// still would be useful for co-op band stuff
class Band: public SelectableEntity {

};


class SelectableEntityManager {
    bool LoadEntities(std::filesystem::path EntityFolder); // search through files for entities
    bool SelectEntity(std::filesystem::path EntityFolder); // select the requested entity
};

class PlayerManager: public SelectableEntityManager {
};

class BandManager: public SelectableEntityManager {
};
#endif //ENCORE_PLAYER_H
