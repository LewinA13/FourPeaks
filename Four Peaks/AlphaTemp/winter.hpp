#ifndef WINTER_HPP
#define WINTER_HPP

#include <cstdint>
#include <vector>
#include <array>
#include "sprite.hpp"

typedef uint32_t u32;

// ===================================================================
// SnowParticle - Represents a single falling snow flake
// ===================================================================
struct SnowParticle {
    float x, y;
    float velX, velY;
    float size;
    float alpha;
    bool active = false;
};

static constexpr int MAX_SNOW = 120;

namespace game {

    // -------------------------------------------------------------------
    // WinterS1 - First Stage
    // -------------------------------------------------------------------
    class WinterS1 {
    public:
        // ===================================================================
        // WinterS1::WinterS1
        // Constructor: Initializes stage flags, grid visibility, and 
        // clears the tile map for the first winter level.
        // ===================================================================
        WinterS1();

        // ===================================================================
        // WinterS1::~WinterS1
        // Destructor: Cleans up resources used by the first stage.
        // ===================================================================
        ~WinterS1();

        WinterS1(const WinterS1&) = delete;

        // ===================================================================
        // WinterS1::update
        // Updates snow particle physics and primary stage logic including 
        // player-to-tile collision and world events.
        // ===================================================================
        int update(float dt);

        // ===================================================================
        // WinterS1::draw
        // Renders the stage layout, the active snow particle system, 
        // and the tilemap to the screen.
        // ===================================================================
        void draw() const;

        // Grid dimensions - 32 columns width x 20 rows height.
        static const int gridCols = 32;
        static const int gridRows = 20;

        // ===================================================================
        // WinterS1::getTileMap
        // Provides access to the raw 2D array for external systems like 
        // physics solvers or debuggers.
        // ===================================================================
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        // Tile map: 0=empty, 1=ground, 2=spikes, etc.
        int tileMap[gridRows][gridCols];

        // For Snow Particles
        mutable std::array<SnowParticle, MAX_SNOW> snowParticles{};
        bool snowInitialized = false;
        float snowSpawnTimer = 0.0f;

        // ===================================================================
        // WinterS1::getTileColor
        // Returns the specific hex color code for different tile IDs 
        // (e.g., White for snow, Blue for ice).
        // ===================================================================
        u32 getTileColor(int tileType) const;

        // ===================================================================
        // WinterS1::drawGrid
        // Renders a visual guide for the underlying grid structure for 
        // development and alignment purposes.
        // ===================================================================
        void drawGrid() const;

        // ===================================================================
        // WinterS1::drawTiles
        // Iterates through the tileMap and renders every non-empty cell 
        // at its respective world position.
        // ===================================================================
        void drawTiles() const;

        // ===================================================================
        // WinterS1::gridToWorld
        // Converts grid-based integer indices (col/row) into floating-point 
        // world coordinates for rendering.
        // ===================================================================
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };

    // -------------------------------------------------------------------
    // WinterS2 - Second Stage
    // -------------------------------------------------------------------
    class WinterS2 {
    public:
        // ===================================================================
        // WinterS2::WinterS2
        // ===================================================================
        WinterS2();

        // ===================================================================
        // WinterS2::~WinterS2
        // ===================================================================
        ~WinterS2();

        WinterS2(const WinterS2&) = delete;

        // ===================================================================
        // WinterS2::update
        // Processes physics for the second level, including stage-specific 
        // hazards and the snow particle system.
        // ===================================================================
        int update(float dt);

        // ===================================================================
        // WinterS2::draw
        // ===================================================================
        void draw() const;

        static const int gridCols = 32;
        static const int gridRows = 20;

        // ===================================================================
        // WinterS2::getTileMap
        // ===================================================================
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        int tileMap[gridRows][gridCols];

        mutable std::array<SnowParticle, MAX_SNOW> snowParticles{};
        bool snowInitialized = false;
        float snowSpawnTimer = 0.0f;

        u32 getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };

    // ===================================================================
    // IceTileState - Tracks the cracking and destruction state of a
    //                breakable ice tile
    // ===================================================================
    struct IceTileState {
        int row{}, col{};
        bool triggered = false;
        float timer = 0.0f;
        int crackFrame = 0;
        bool destroyed = false;
    };

    // -------------------------------------------------------------------
    // WinterS3 - Third Stage
    // -------------------------------------------------------------------
    class WinterS3 {
    public:
        // ===================================================================
        // WinterS3::WinterS3
        // Constructor: Prepares the map and initializes the breakable 
        // ice tile state vector.
        // ===================================================================
        WinterS3();

        // ===================================================================
        // WinterS3::~WinterS3
        // ===================================================================
        ~WinterS3();

        WinterS3(const WinterS3&) = delete;

        // ===================================================================
        // WinterS3::update
        // Updates core logic and processes the crumbling/breaking 
        // timers for ice platforms.
        // ===================================================================
        int update(float dt);

        // ===================================================================
        // WinterS3::draw
        // Renders the environment, particles, and the visual cracking 
        // effects of breakable ice tiles.
        // ===================================================================
        void draw() const;

        // ===================================================================
        // WinterS3::resetBreakableIce
        // Re-solidifies all ice tiles and resets their timers, usually 
        // called upon player respawn or stage transition.
        // ===================================================================
        void resetBreakableIce();

        static const int gridCols = 32;
        static const int gridRows = 20;

        // ===================================================================
        // WinterS3::getTileMap
        // ===================================================================
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        int tileMap[gridRows][gridCols];

        mutable std::array<SnowParticle, MAX_SNOW> snowParticles{};
        bool snowInitialized = false;
        float snowSpawnTimer = 0.0f;

        u32 getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;

        /** Vector of objects managing the integrity of breakable floor tiles */
        std::vector<IceTileState> iceTiles;
    };

    // -------------------------------------------------------------------
    // WinterS4 - Fourth Stage
    // -------------------------------------------------------------------
    class WinterS4 {
    public:
        // ===================================================================
        // WinterS4::WinterS4
        // ===================================================================
        WinterS4();

        // ===================================================================
        // WinterS4::~WinterS4
        // ===================================================================
        ~WinterS4();

        WinterS4(const WinterS4&) = delete;

        // ===================================================================
        // WinterS4::update
        // Processes the most complex logic in the winter set, handling 
        // particles, breakable tiles, and advanced stage hazards.
        // ===================================================================
        int update(float dt);

        // ===================================================================
        // WinterS4::draw
        // ===================================================================
        void draw() const;

        // ===================================================================
        // WinterS4::resetBreakableIce
        // ===================================================================
        void resetBreakableIce();

        static const int gridCols = 32;
        static const int gridRows = 20;

        // ===================================================================
        // WinterS4::getTileMap
        // ===================================================================
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        int tileMap[gridRows][gridCols];

        mutable std::array<SnowParticle, MAX_SNOW> snowParticles{};
        bool snowInitialized = false;
        float snowSpawnTimer = 0.0f;

        u32 getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;

        /** Tracks health and destruction for ice tiles in the final winter stage */
        std::vector<IceTileState> iceTiles;
    };

} // namespace game

#endif // WINTER_HPP