#ifndef SUMMERS1HPP
#define SUMMERS1HPP

#include <vector>
#include <cstdint>
#include <cmath>
#include "sprite.hpp"

typedef uint32_t u32;

namespace game {

    // -------------------------------------------------------------------
    // SummerS1 - First Stage
    // -------------------------------------------------------------------
    class SummerS1 {
    public:
        SummerS1();
        ~SummerS1();
        SummerS1(const SummerS1&) = delete;

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
    // SummerS2 - Second Stage
    // -------------------------------------------------------------------
    class SummerS2 {
    public:
        SummerS2();
        ~SummerS2();
        SummerS2(const SummerS2&) = delete;

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

} // namespace game

#endif // SUMMERS1HPP
