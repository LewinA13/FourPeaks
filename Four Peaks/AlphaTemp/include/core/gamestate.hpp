#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include "gameplay/player.hpp"
#include <vector>
#include <string>
#include "AEAudio.h"

// -------------------------------------------------------------------------
// Records a single melon the player has picked up. Melons are identified by
// the scene they belong to and their row/column in that scene's tile map,
// so the same melon is never awarded twice across play sessions.
// -------------------------------------------------------------------------
struct CollectedMelon
{
    std::string scene; // Scene name matching g_currentScene
    int row = 0;       // Tile grid row of the melon
    int col = 0;       // Tile grid column of the melon
};

// -------------------------------------------------------------------------
// Central game state object. One global instance (gGame) is shared across
// all systems — audio, collision, dialogue, UI, and the pause menu all read
// from or write to this struct rather than maintaining their own duplicates.
// -------------------------------------------------------------------------
struct GameState
{
    // -------------------------------------------------------------------------
    // The single controllable character.
    // -------------------------------------------------------------------------
    Player player;

    // -------------------------------------------------------------------------
    // Volume settings (range 0.0 - 1.0).
    // -------------------------------------------------------------------------
    float musicVol = 0.6f;
    float sfxVol = 0.9f;

    // -------------------------------------------------------------------------
    // Debug / cheat flags.
    // -------------------------------------------------------------------------
    bool cheatsOn = false; // Master toggle for all cheat shortcuts
    bool noClip = false; // If true, the player passes through all solid tiles

    // -------------------------------------------------------------------------
    // Total seconds elapsed since the current run began.
    // -------------------------------------------------------------------------
    float runTimeSeconds = 0.0f;

    // -------------------------------------------------------------------------
    // Stage progression flags indexed 0-15. Each true entry means that stage
    // has been cleared and the next stage in the sequence is accessible.
    // -------------------------------------------------------------------------
    bool unlockedStages[16] = {};

    // -------------------------------------------------------------------------
    // List of every melon collected this run. Used for the "all melons"
    // achievement check and to prevent double-collection on re-entry.
    // -------------------------------------------------------------------------
    std::vector<CollectedMelon> collectedMelons;

    // -------------------------------------------------------------------------
    // Seasonal artifact progress indexed 0-3 (Winter, Summer, Spring, Autumn).
    // Set to true when the corresponding relic is obtained.
    // -------------------------------------------------------------------------
    bool collectedArtifacts[4] = {};
    bool allArtifactsAchievementShown = false; // Prevents the achievement dialog repeating
    bool allMelonsAchievementShown = false; // Prevents the melon achievement repeating

    // -------------------------------------------------------------------------
    // When true, the main loop should reload all stage tile maps from disk on
    // the next frame (set after the player deletes their save file).
    // -------------------------------------------------------------------------
    bool reloadAllStageMaps = false;

    // -------------------------------------------------------------------------
    // Pause menu state.
    // -------------------------------------------------------------------------
    bool pauseActive = false; // Is the pause overlay currently visible?
    bool pauseShowSettings = false; // Is the settings sub-panel open?
    int  pauseSelectedIndex = 0;     // Currently highlighted menu item index
    int  pauseSettingsRow = 0;     // Currently highlighted row in the settings panel
};

// -------------------------------------------------------------------------
// The one global game state instance. Defined in gamestate.cpp.
// -------------------------------------------------------------------------
extern GameState gGame;

#endif // GAME_STATE_HPP