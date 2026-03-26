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

    // volume settings
    float musicVol = 0.6f;
    float sfxVol = 0.9f;

    bool cheatsOn = false;
    bool noClip = false;

    // total game run time
    float runTimeSeconds = 0.0f;

    // stage unlock flags
    bool unlockedStages[16] = {};

    // collected melon data
    std::vector<CollectedMelon> collectedMelons;

    // collected artifact flags
    // 0 = winter
    // 1 = summer
    // 2 = spring
    // 3 = autumn
    bool collectedArtifacts[4] = {};

    // pause menu state
    bool pauseActive = false;
    bool pauseShowSettings = false;
    int pauseSelectedIndex = 0;
    int pauseSettingsRow = 0;
};

// one global world object
extern GameState gGame;

#endif // GAME_STATE_HPP