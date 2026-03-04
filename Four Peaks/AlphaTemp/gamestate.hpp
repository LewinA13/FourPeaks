#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include "player.hpp"
#include "AEAudio.h"   // audio types (AEAudio, AEAudioGroup)

struct GameState
{
    Player player;

    // Volume settings (0.0f to 1.0f)
    float musicVol = 0.6f;
    float sfxVol = 0.9f;

    bool cheatsOn = false;
    bool noClip = false;
};

// One global world object (declared here)
extern GameState gGame;

#endif // GAME_STATE_HPP
