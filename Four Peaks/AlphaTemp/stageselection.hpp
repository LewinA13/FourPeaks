#ifndef STAGESELECTION_HPP
#define STAGESELECTION_HPP

namespace game {
    class StageSelect {
    public:
        StageSelect();
        int  update(float dt);  
        void draw() const;
    private:
        int seasonIndex;   
        int stageIndex;   
        bool selectingStage; 
    };
}

#endif
