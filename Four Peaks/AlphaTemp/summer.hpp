#ifndef SUMMER_HPP
#define SUMMER_HPP

#include <cstdint>
#include <vector>
#include "sprite.hpp"
#include "winter.hpp"   // IceTileState is defined here — don't redefine it

typedef uint32_t u32;

namespace game {

    // -------------------------------------------------------------------
    // Heat Bar Utilities
    // -------------------------------------------------------------------

    // ===================================================================
    // HeatUpdate
    // Manages the heat meter progression, calculating depletion or 
    // accumulation based on environment and delta time.
    // ===================================================================
    void HeatUpdate(float dt);

    // ===================================================================
    // HeatDraw
    // Handles the UI rendering of the heat bar, including foreground 
    // fill levels and background containers.
    // ===================================================================
    void HeatDraw();


    // -------------------------------------------------------------------
    // SummerS1 - First Stage
    // -------------------------------------------------------------------
    class SummerS1 {
    public:
        // ===================================================================
        // SummerS1::SummerS1
        // Constructor: Loads the level layout into the tileMap and 
        // initializes stage-specific flags and variables.
        // ===================================================================
        SummerS1();

        // ===================================================================
        // SummerS1::~SummerS1
        // Destructor: Cleans up allocated resources for the first stage.
        // ===================================================================
        ~SummerS1();

        SummerS1(const SummerS1&) = delete;

        // ===================================================================
        // SummerS1::update
        // Processes player movement, gravity, and basic collision 
        // detection against the tilemap for Stage 1.
        // ===================================================================
        int update(float dt);

        // ===================================================================
        // SummerS1::draw
        // Main render pass: clears the buffer and calls internal 
        // drawing routines for tiles and grid lines.
        // ===================================================================
        void draw() const;

        // Grid dimensions - 32 columns width x 20 rows height.
        static const int gridCols = 32;
        static const int gridRows = 20;

        // ===================================================================
        // SummerS1::getTileMap
        // Provides external access to the raw tile data for physics 
        // resolution or external solvers.
        // ===================================================================
        int (*getTileMap())[gridCols] { return tileMap; }

        // ===================================================================
        // SummerS1::reset
        // Reinitializes the stage state, effectively "re-spawning" 
        // water bottles and resetting player starting positions.
        // ===================================================================
        void reset();

    private:
        bool gridVisible;
        int tileMap[gridRows][gridCols];

        // ===================================================================
        // SummerS1::getTileColor
        // Maps tile IDs to specific color constants (e.g., Sandy Brown 
        // for ground, Red for hazards).
        // ===================================================================
        u32 getTileColor(int tileType) const;

        // ===================================================================
        // SummerS1::drawGrid
        // Renders the underlying coordinate system lines to assist 
        // with level design and object placement.
        // ===================================================================
        void drawGrid() const;

        // ===================================================================
        // SummerS1::drawTiles
        // Iterates through the tileMap array and draws every non-zero 
        // cell to the screen based on its world position.
        // ===================================================================
        void drawTiles() const;

        // ===================================================================
        // SummerS1::gridToWorld
        // Performs the coordinate transform from integer grid indices 
        // to floating-point screen space coordinates.
        // ===================================================================
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };


    // -------------------------------------------------------------------
    // SummerS2 - Second Stage
    // -------------------------------------------------------------------
    class SummerS2 {
    public:
        // ===================================================================
        // SummerS2::SummerS2
        // Constructor: Sets up the Stage 2 map layout and visibility settings.
        // ===================================================================
        SummerS2();

        // ===================================================================
        // SummerS2::~SummerS2
        // Destructor: Standard cleanup for Stage 2 resources.
        // ===================================================================
        ~SummerS2();

        SummerS2(const SummerS2&) = delete;

        // ===================================================================
        // SummerS2::update
        // Updates level logic, specifically handling the slightly more 
        // complex terrain or hazards introduced in the second level.
        // ===================================================================
        int update(float dt);

        // ===================================================================
        // SummerS2::draw
        // Renders Stage 2 elements, including any unique background layers.
        // ===================================================================
        void draw() const;

        static const int gridCols = 32;
        static const int gridRows = 20;

        // ===================================================================
        // SummerS2::reset
        // Restores Stage 2 to its default state after a player death.
        // ===================================================================
        void reset();

        // ===================================================================
        // SummerS2::getTileMap
        // Returns the 2D array pointer for collision detection.
        // ===================================================================
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        int tileMap[gridRows][gridCols];

        u32 getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };


    // -------------------------------------------------------------------
    // SummerS3 - Third Stage
    // -------------------------------------------------------------------
    class SummerS3 {
    public:
        // ===================================================================
        // SummerS3::SummerS3
        // Constructor: Initializes the breakable ice tile tracking system 
        // alongside the standard map data.
        // ===================================================================
        SummerS3();

        // ===================================================================
        // SummerS3::~SummerS3
        // Destructor: Clears the iceTile state vectors and map data.
        // ===================================================================
        ~SummerS3();

        SummerS3(const SummerS3&) = delete;

        // ===================================================================
        // SummerS3::update
        // Handles standard logic plus the specific timers for tiles that 
        // crack and disappear when the player stands on them.
        // ===================================================================
        int update(float dt);

        // ===================================================================
        // SummerS3::draw
        // Renders the map, including specialized overlays for tiles in 
        // various stages of "breaking."
        // ===================================================================
        void draw() const;

        static const int gridCols = 32;
        static const int gridRows = 20;

        // ===================================================================
        // SummerS3::getTileMap
        // Accessor for the stage's 2D tile array.
        // ===================================================================
        int (*getTileMap())[gridCols] { return tileMap; }

        // ===================================================================
        // SummerS3::reset
        // Resets the level and specifically restores all breakable 
        // ice tiles to their solid, unbroken state.
        // ===================================================================
        void reset();

    private:
        bool gridVisible;
        int tileMap[gridRows][gridCols];

        u32 getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;

        /** Manages the individual integrity and visual frames for breakable tiles */
        std::vector<IceTileState> iceTiles;
    };


    // -------------------------------------------------------------------
    // SummerS4 - Fourth Stage
    // -------------------------------------------------------------------
    class SummerS4 {
    public:
        // ===================================================================
        // SummerS4::SummerS4
        // Constructor: Prepares the final stage map and breakable states.
        // ===================================================================
        SummerS4();

        // ===================================================================
        // SummerS4::~SummerS4
        // Destructor: Final cleanup for the end-game stage resources.
        // ===================================================================
        ~SummerS4();

        SummerS4(const SummerS4&) = delete;

        // ===================================================================
        // SummerS4::update
        // Runs the end-stage game loop, processing the most difficult 
        // platforming and hazard logic in the summer module.
        // ===================================================================
        int update(float dt);

        // ===================================================================
        // SummerS4::draw
        // Final render call for the summer environment.
        // ===================================================================
        void draw() const;

        static const int gridCols = 32;
        static const int gridRows = 20;

        // ===================================================================
        // SummerS4::getTileMap
        // Returns the final stage's collision layout.
        // ===================================================================
        int (*getTileMap())[gridCols] { return tileMap; }

        // ===================================================================
        // SummerS4::reset
        // Resets hazards and breakable surfaces for the final stage.
        // ===================================================================
        void reset();

    private:
        bool gridVisible;
        int tileMap[gridRows][gridCols];

        u32 getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;

        /** Vector tracking the health and destruction of breakable tiles in Stage 4 */
        std::vector<IceTileState> iceTiles;
    };

} // namespace game

#endif // SUMMER_HPP