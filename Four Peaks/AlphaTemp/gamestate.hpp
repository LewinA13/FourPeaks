#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include "player.hpp"
#include <vector>
#include <string>
#include "AEAudio.h"   // audio types (AEAudio, AEAudioGroup)

struct CollectedMelon
{
    std::string scene;
    int row = 0;
    int col = 0;
};

struct GameState
{
    Player player;

    // Volume settings (0.0f to 1.0f)
    float musicVol = 0.6f;
    float sfxVol = 0.9f;

    bool cheatsOn = false;
    bool noClip = false;

    // total game run time. only when playing. pause doesnt affect
    float runTimeSeconds = 0.0f;

    //vector to store the collected melons
    std::vector<CollectedMelon> collectedMelons;
};

// One global world object (declared here)
extern GameState gGame;

#endif // GAME_STATE_HPP
