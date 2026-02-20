#ifndef WINTERHPP
#define WINTERHPP

#include <cstdint>
#include <vector>
#include <array>
#include "sprite.hpp"

typedef uint32_t u32;

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
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        // Tile map: 0=empty, 1=ground, 2=spikes, etc.
        int tileMap[gridRows][gridCols];
        u32 getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
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
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        // Tile map: 0=empty, 1=ground, 2=spikes, etc.
        int tileMap[gridRows][gridCols];
        u32 getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };

    struct IceTileState{
        int row, col;
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

        // Grid dimensions - 32 columns width x 20 rows height.
        static const int gridCols = 32;
        static const int gridRows = 20;
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        // Tile map: 0=empty, 1=ground, 2=spikes, etc.
        int tileMap[gridRows][gridCols];
        u32 getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
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

        // Grid dimensions - 32 columns width x 20 rows height.
        static const int gridCols = 32;
        static const int gridRows = 20;
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        // Tile map: 0=empty, 1=ground, 2=spikes, etc.
        int tileMap[gridRows][gridCols];
        u32 getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;

        std::vector<IceTileState> iceTiles;
    };

} // namespace game

#endif // SUMMERS1HPP
