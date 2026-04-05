// ----------------------------------------------------------------------------
// Done By: Justin, Skyler
// ----------------------------------------------------------------------------

// audio.cpp
#include "engine/audio.hpp"
#include "AEAudio.h"
#include "core/gamestate.hpp"

// Groups
static AEAudioGroup s_musicGroup{};
static AEAudioGroup s_sfxGroup{};

// Music
static AEAudio s_winterMusic{};
static AEAudio s_summerMusic{};
static AEAudio s_springMusic{};
static AEAudio s_autumnMusic{};
static AEAudio s_mainMenuMusic{};
static AEAudio s_tutorialMusic{};

// SFX
static AEAudio s_jumpSfx{};
static AEAudio s_dashSfx{};
static AEAudio s_deathSfx{};
static AEAudio s_respawnSfx{};
static AEAudio s_winterStepSfx{};

// Tracks what BGM is currently playing
static BgmType s_currentBgm = BgmType::None;

void audio::init()
{
    // Start audio system
    AEAudioInit();

    // Create groups
    s_musicGroup = AEAudioCreateGroup();
    s_sfxGroup = AEAudioCreateGroup();

    // Load audio assets
    s_winterMusic = AEAudioLoadMusic("Assets/Music/winter.mp3");
    s_summerMusic = AEAudioLoadMusic("Assets/Music/summer.mp3");
    s_springMusic = AEAudioLoadMusic("Assets/Music/spring.mp3");
    s_autumnMusic = AEAudioLoadMusic("Assets/Music/autumn.mp3");
    s_mainMenuMusic = AEAudioLoadMusic("Assets/Music/main_menu.mp3");
    s_tutorialMusic = AEAudioLoadMusic("Assets/Music/tutorial.mp3");

    s_jumpSfx = AEAudioLoadSound("Assets/SFX/jump.mp3");
    s_dashSfx = AEAudioLoadSound("Assets/SFX/dash.mp3");
    s_deathSfx = AEAudioLoadSound("Assets/SFX/death.wav");
    s_respawnSfx = AEAudioLoadSound("Assets/SFX/respawn.wav");
    s_winterStepSfx = AEAudioLoadSound("Assets/SFX/winterStep.wav");

    // Apply initial volumes
    AEAudioSetGroupVolume(s_musicGroup, gGame.musicVol);
    AEAudioSetGroupVolume(s_sfxGroup, gGame.sfxVol);

    s_currentBgm = BgmType::None;
}

// -------------------------------------------------------------------------
// Immediately stops all music and resets the current BGM tracker.
// -------------------------------------------------------------------------
void audio::stop_music()
{
    AEAudioStopGroup(s_musicGroup);
    s_currentBgm = BgmType::None;
}

// -------------------------------------------------------------------------
// Stops any currently playing music and starts the track that corresponds
// to the given BgmType. All tracks loop indefinitely (loop count = -1).
// -------------------------------------------------------------------------
static void PlayBgm_Internal(BgmType type)
{
    // Always stop whatever was playing before starting a new track
    AEAudioStopGroup(s_musicGroup);

    switch (type)
    {
    case BgmType::Winter:
        AEAudioPlay(s_winterMusic, s_musicGroup, 1.0f, 1.0f, -1); // -1 = loop forever
        break;

    case BgmType::Summer:
        AEAudioPlay(s_summerMusic, s_musicGroup, 1.0f, 1.0f, -1);
        break;

    case BgmType::Spring:
        AEAudioPlay(s_springMusic, s_musicGroup, 1.0f, 1.0f, -1);
        break;

    case BgmType::Autumn:
        AEAudioPlay(s_autumnMusic, s_musicGroup, 1.0f, 1.0f, -1);
        break;

    case BgmType::MainMenu:
        AEAudioPlay(s_mainMenuMusic, s_musicGroup, 1.0f, 1.0f, -1);
        break;

    case BgmType::Tutorial:
        AEAudioPlay(s_tutorialMusic, s_musicGroup, 1.0f, 1.0f, -1);
        break;

    case BgmType::None:
    default:
        // silence
        break;
    }
}


// -------------------------------------------------------------------------
// Ticks the audio engine, syncs group volumes to the current game state,
// and switches the BGM track if desiredBgm has changed since the last frame.
// -------------------------------------------------------------------------
void audio::update(BgmType desiredBgm)
{
    AEAudioUpdate();

    // Both groups now follow the same slider
    AEAudioSetGroupVolume(s_musicGroup, gGame.musicVol);
    AEAudioSetGroupVolume(s_sfxGroup, gGame.sfxVol);

    if (desiredBgm != s_currentBgm)
    {
        PlayBgm_Internal(desiredBgm);
        s_currentBgm = desiredBgm;
    }
}

// -------------------------------------------------------------------------
// Plays a one-shot SFX from the SFX group. Each effect plays once at the
// default pitch and volume, without looping (loop count = 0).
// -------------------------------------------------------------------------
void audio::play_sfx(SfxType sfx)
{
    switch (sfx)
    {
    case SfxType::Jump:
        AEAudioPlay(s_jumpSfx, s_sfxGroup, 1.0f, 1.0f, 0);
        break;

    case SfxType::Dash:
        AEAudioPlay(s_dashSfx, s_sfxGroup, 1.0f, 1.0f, 0);
        break;

    case SfxType::Death:
        AEAudioPlay(s_deathSfx, s_sfxGroup, 1.0f, 1.5f, 0);
        break;

    case SfxType::Respawn:
        AEAudioPlay(s_respawnSfx, s_sfxGroup, 1.0f, 1.0f, 0);
        break;

    case SfxType::WinterStep:
        AEAudioPlay(s_winterStepSfx, s_sfxGroup, 1.0f, 1.0f, 0);
        break;

    default:
        break;
    }
}

// -------------------------------------------------------------------------
// Stops all audio, unloads every asset and mixer group, then shuts down
// the audio engine. Safe to call only once during application teardown.
// -------------------------------------------------------------------------
void audio::shutdown()
{
    // Stop music first
    AEAudioStopGroup(s_musicGroup);

    // Unload audio handles
    AEAudioUnloadAudio(s_mainMenuMusic);
    AEAudioUnloadAudio(s_tutorialMusic);
    AEAudioUnloadAudio(s_winterMusic);
    AEAudioUnloadAudio(s_summerMusic);
    AEAudioUnloadAudio(s_springMusic);
    AEAudioUnloadAudio(s_autumnMusic);

    AEAudioUnloadAudio(s_jumpSfx);
    AEAudioUnloadAudio(s_dashSfx);
    AEAudioUnloadAudio(s_deathSfx);
    AEAudioUnloadAudio(s_respawnSfx);
    AEAudioUnloadAudio(s_winterStepSfx);

    // Unload groups
    AEAudioUnloadAudioGroup(s_musicGroup);
    AEAudioUnloadAudioGroup(s_sfxGroup);

    // Shutdown audio system
    AEAudioExit();
}