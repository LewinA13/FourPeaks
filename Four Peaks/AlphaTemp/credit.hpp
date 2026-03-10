#ifndef CREDIT_HPP
#define CREDIT_HPP

#include "AEEngine.h"

namespace game {

    class Credits {
    public:
        Credits();
        int  update(float dt);  // returns 2 = back to main menu
        void draw() const;

    private:
        float         scroll_offset;
        float         scroll_speed;
        float         credits_total_height;
       
    };

} // namespace game

#endif