// ----------------------------------------------------------------------------
// Done By: Justin
// ----------------------------------------------------------------------------

#ifndef PAUSE_HPP
#define PAUSE_HPP

// enum for pause menu actions, returned by update() to indicate what the main game loop should do next
namespace pause
{
    enum Action
    {
        None = 0,
        Resume,
        MainMenu,
        ExitGame
    };

// ---------------------------------------------------------------------------
// Update
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    Action update();
// ---------------------------------------------------------------------------
// Draw
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
    void draw();
}

#endif // PAUSE_HPP
