// ---------------------------------------------------------------------------
// mainmenu.hpp
// ---------------------------------------------------------------------------
//
// Simple main menu for the seasonal platformer.
// Shows three text "buttons":
//   - Play
//   - How To Play (placeholder)
//   - Exit
//
// update() return codes:
//   0 = stay on main menu
//   1 = start SummerS1
//   2 = exit game
//

#ifndef MAINMENU_HPP
#define MAINMENU_HPP

namespace game
{
    class MainMenu
    {
    public:
        MainMenu();

        int update();   // handle input
        void draw() const; // draw menu each frame

    private:
        int  selectedIndex; // 0=Play, 1=How To Play, 2=Exit
        bool showHowTo;     // true when help screen is shown

        void drawHowToPlay() const;
    };
}

#endif // MAINMENU_HPP
