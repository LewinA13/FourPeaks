#ifndef WINTERHPP
#define WINTERHPP

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
        WinterS1();
        ~WinterS1();
        WinterS1(const WinterS1&) = delete;
        int update(float dt);
        void draw() const;

        // Grid dimensions - 32 columns width x 20 rows height.
        static const int gridCols = 32;
        static const int gridRows = 20;

        // ===================================================================
        // WinterS1::getTileMap
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
        // ===================================================================
        u32 getTileColor(int tileType) const;

        // ===================================================================
        // WinterS1::drawGrid
        // ===================================================================
        void drawGrid() const;

        // ===================================================================
        // WinterS1::drawTiles
        // ===================================================================
        void drawTiles() const;

        // ===================================================================
        // WinterS1::gridToWorld
        // ===================================================================
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };

    // -------------------------------------------------------------------
    // WinterS2 - Second Stage
    // -------------------------------------------------------------------
    class WinterS2 {
    public:
        WinterS2();
        ~WinterS2();
        WinterS2(const WinterS2&) = delete;
        int update(float dt);
        void draw() const;

        // Grid dimensions - 32 columns width x 20 rows height.
        static const int gridCols = 32;
        static const int gridRows = 20;

        // ===================================================================
        // WinterS2::getTileMap
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
        // WinterS2::getTileColor
        // ===================================================================
        u32 getTileColor(int tileType) const;

        // ===================================================================
        // WinterS2::drawGrid
        // ===================================================================
        void drawGrid() const;

        // ===================================================================
        // WinterS2::drawTiles
        // ===================================================================
        void drawTiles() const;

        // ===================================================================
        // WinterS2::gridToWorld
        // ===================================================================
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };

    // ===================================================================
    // IceTileState - Tracks the cracking and destruction state of a
    //               breakable ice tile
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
        WinterS3();
        ~WinterS3();
        WinterS3(const WinterS3&) = delete;
        int update(float dt);
        void draw() const;

        // ===================================================================
        // WinterS3::resetBreakableIce
        // ===================================================================
        void resetBreakableIce(); // Use for stage changing

        // Grid dimensions - 32 columns width x 20 rows height.
        static const int gridCols = 32;
        static const int gridRows = 20;

        // ===================================================================
        // WinterS3::getTileMap
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
        // WinterS3::getTileColor
        // ===================================================================
        u32 getTileColor(int tileType) const;

        // ===================================================================
        // WinterS3::drawGrid
        // ===================================================================
        void drawGrid() const;

        // ===================================================================
        // WinterS3::drawTiles
        // ===================================================================
        void drawTiles() const;

        // ===================================================================
        // WinterS3::gridToWorld
        // ===================================================================
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;

        std::vector<IceTileState> iceTiles;
    };

    // -------------------------------------------------------------------
    // WinterS4 - Fourth Stage
    // -------------------------------------------------------------------
    class WinterS4 {
    public:
        WinterS4();
        ~WinterS4();
        WinterS4(const WinterS4&) = delete;
        int update(float dt);
        void draw() const;

        // ===================================================================
        // WinterS4::resetBreakableIce
        // ===================================================================
        void resetBreakableIce(); // Use for stage changing

        // Grid dimensions - 32 columns width x 20 rows height.
        static const int gridCols = 32;
        static const int gridRows = 20;

        // ===================================================================
        // WinterS4::getTileMap
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
        // WinterS4::getTileColor
        // ===================================================================
        u32 getTileColor(int tileType) const;

        // ===================================================================
        // WinterS4::drawGrid
        // ===================================================================
        void drawGrid() const;

        // ===================================================================
        // WinterS4::drawTiles
        // ===================================================================
        void drawTiles() const;

        // ===================================================================
        // WinterS4::gridToWorld
        // ===================================================================
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;

        std::vector<IceTileState> iceTiles;
    };

} // namespace game

#endif // SUMMERS1HPP