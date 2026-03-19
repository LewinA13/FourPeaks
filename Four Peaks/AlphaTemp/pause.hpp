#ifndef PAUSE_HPP
#define PAUSE_HPP

namespace pause
{
    enum Action
    {
        None = 0,
        Resume,
        MainMenu,
        ExitGame
    };

    Action update();
    void draw();
}

#endif // PAUSE_HPP