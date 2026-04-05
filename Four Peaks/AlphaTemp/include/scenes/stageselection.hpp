// ----------------------------------------------------------------------------
// Done By: Hong Yang, Justin
// ----------------------------------------------------------------------------

#ifndef STAGESELECTION_HPP
#define STAGESELECTION_HPP

namespace game {

    // ---------------------------------------------------------------------------
    // StageSelect
    // Handles the stage selection menu logic and rendering.
    // Allows player to choose season and stage.
    // ---------------------------------------------------------------------------
    class StageSelect {
    public:

        // ---------------------------------------------------------------------------
        // Constructor: initialize selection state
        // ---------------------------------------------------------------------------
        StageSelect();

        // ---------------------------------------------------------------------------
        // Update selection logic (input handling)
        // Returns action code when a stage is selected
        // ---------------------------------------------------------------------------
        int  update(float dt);

        // ---------------------------------------------------------------------------
        // Render the stage selection UI
        // ---------------------------------------------------------------------------
        void draw() const;

        // ---------------------------------------------------------------------------
        // Reset selection state to default
        // ---------------------------------------------------------------------------
        void reset();

    private:
        int seasonIndex;     // Current selected season (0~3)
        int stageIndex;      // Current selected stage (0~3)
        bool selectingStage; // Whether player is selecting stage or season
    };
}

#endif