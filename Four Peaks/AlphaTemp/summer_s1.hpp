#ifndef SUMMERS1HPP
#define SUMMERS1HPP

#include <vector>
#include <cstdint>

#include "player.hpp"

typedef uint32_t u32;

namespace game {
    class SummerS1 {
    public:
        SummerS1();
        ~SummerS1();
        SummerS1(const SummerS1&) = delete;
        int update();
        void draw() const;

    private:
        bool gridVisible;

        // Player instance for this level
        Player player;

        // Grid dimensions - 32 columns width x 20 rows height.
        static const int gridCols = 32;
        static const int gridRows = 20;

        // Tile map: 0=empty, 1=ground, 2=spikes, etc.
        int tileMap[gridRows][gridCols];
        u32 getTileColor(int tileType) const;

        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };
}

#endif // SUMMERS1HPP
