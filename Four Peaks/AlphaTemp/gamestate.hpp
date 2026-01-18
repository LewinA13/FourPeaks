#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include "player.hpp"

struct GameState
{
    Player player;
    
};

// One global world object (declared here)
extern GameState gGame;

#endif // GAME_STATE_HPP
