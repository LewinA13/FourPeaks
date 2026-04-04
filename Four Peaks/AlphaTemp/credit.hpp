#ifndef CREDIT_HPP
#define CREDIT_HPP

#include "AEEngine.h"

namespace game {

    // -------------------------------------------------------------------------
    // Manages the rolling end-credits sequence. Scrolls a list of names and
    // attributions upward across a black background. Pressing Escape or waiting
    // for the scroll to finish returns to the main menu.
    // -------------------------------------------------------------------------
    class Credits {
    public:
        // -------------------------------------------------------------------------
        // Initialises scroll position and speed constants.
        // -------------------------------------------------------------------------
        Credits();

        // -------------------------------------------------------------------------
        // Advances the scroll position by scroll_speed * dt pixels.
        // Returns 2 when the player presses Escape or the credits finish scrolling,
        // signalling the game manager to return to the main menu. Returns 0 otherwise.
        // -------------------------------------------------------------------------
        int  update(float dt);

        // -------------------------------------------------------------------------
        // Draws the black background and all credit lines at their current
        // scrolled positions. Camera is temporarily reset to (0,0) so the
        // credits always render relative to the screen centre.
        // -------------------------------------------------------------------------
        void draw() const;

    private:
        float scroll_offset;        // Pixels scrolled from the bottom so far
        float scroll_speed;         // Pixels per second
        float credits_total_height; // Estimated total height of all credit content
    };

} // namespace game

#endif