// ----------------------------------------------------------------------------
// Done By: Justin, Skyler
// ----------------------------------------------------------------------------

#ifndef AUDIO_HPP
#define AUDIO_HPP

// -------------------------------------------------------------------------
// Identifies which background music track should be playing.
// Pass BgmType::None to silence all music.
// -------------------------------------------------------------------------
enum class BgmType
{
    None,
    Winter,
    Summer,
    Spring,
    Autumn,
    MainMenu,
    Tutorial
};

// -------------------------------------------------------------------------
// Identifies a one-shot sound effect.
// -------------------------------------------------------------------------
enum class SfxType
{
    Jump,
    Dash,
    Death,
    Respawn,
    WinterStep
};

// -------------------------------------------------------------------------
// Initialises the audio engine, creates the music and SFX mixer groups,
// loads all audio assets from disk, and applies the initial volume settings
// from the global GameState. Must be called once before any other audio function.
// -------------------------------------------------------------------------
namespace audio
{
    // Start audio system + create groups + load all audio assets.
    void init();

    // Per-frame audio maintenance + BGM switching.
    // Call once per frame from main loop.
    void update(BgmType desiredBgm);

    // Stop/unload everything + shutdown audio system.
    void shutdown();

    // Simple SFX trigger from anywhere (player, UI, etc.).
    void play_sfx(SfxType sfx);

    // Optional helper if you ever need to hard-stop BGM instantly.
    void stop_music();
}

#endif // AUDIO_HPP