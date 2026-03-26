#ifndef AUDIO_HPP
#define AUDIO_HPP

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

enum class SfxType
{
    Jump,
    Dash,
    Death,
    Respawn,
    WinterStep
};

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