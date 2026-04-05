#ifndef SUMMER_HPP
#define SUMMER_HPP

#include <cstdint>
#include <vector>
#include "../engine/sprite.hpp"
#include "../scenes/winter.hpp"   // IceTileState is defined here — don't redefine it

typedef uint32_t u32;

namespace game {

    // -------------------------------------------------------------------
    // Heat Bar Utilities
    // -------------------------------------------------------------------
    void HeatUpdate(float dt);
    void HeatDraw();

    // -------------------------------------------------------------------
    // SummerS1 - Stage 1
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

        // ===================================================================
        // SummerS1::getTileMap
        // ===================================================================
        int (*getTileMap())[gridCols] { return tileMap; }

        // ===================================================================
        // SummerS1::reset
        // ===================================================================
        void reset(); // For spawning the water bottles again after death

    private:
        bool gridVisible;
        // Tile map: 0=empty, 1=ground, etc.
        int tileMap[gridRows][gridCols];

        // ===================================================================
        // SummerS1::getTileColor
        // ===================================================================
        u32 getTileColor(int tileType) const;

        // ===================================================================
        // SummerS1::drawGrid
        // ===================================================================
        void drawGrid() const;

        // ===================================================================
        // SummerS1::drawTiles
        // ===================================================================
        void drawTiles() const;

        // ===================================================================
        // SummerS1::gridToWorld
        // ===================================================================
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };

    // -------------------------------------------------------------------
    // SummerS2 - Stage 2
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

        // ===================================================================
        // SummerS2::getTileMap
        // ===================================================================
        int (*getTileMap())[gridCols] { return tileMap; }

        // ===================================================================
        // SummerS2::reset
        // ===================================================================
        void reset();

    private:
        bool gridVisible;
        // Tile map: 0=empty, 1=ground, etc.
        int tileMap[gridRows][gridCols];

        // ===================================================================
        // SummerS2::getTileColor
        // ===================================================================
        u32 getTileColor(int tileType) const;

        // ===================================================================
        // SummerS2::drawGrid
        // ===================================================================
        void drawGrid() const;

        // ===================================================================
        // SummerS2::drawTiles
        // ===================================================================
        void drawTiles() const;

        // ===================================================================
        // SummerS2::gridToWorld
        // ===================================================================
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };

    // -------------------------------------------------------------------
    // SummerS3 - Stage 3
    // -------------------------------------------------------------------
    class SummerS3 {
    public:
        SummerS3();
        ~SummerS3();
        SummerS3(const SummerS3&) = delete;
        int update(float dt);
        void draw() const;

        // Grid dimensions - 32 columns width x 20 rows height.
        static const int gridCols = 32;
        static const int gridRows = 20;

        // ===================================================================
        // SummerS3::getTileMap
        // ===================================================================
        int (*getTileMap())[gridCols] { return tileMap; }

        // ===================================================================
        // SummerS3::reset
        // ===================================================================
        void reset();

    private:
        bool gridVisible;
        // Tile map: 0=empty, 1=ground, etc.
        int tileMap[gridRows][gridCols];

        // ===================================================================
        // SummerS3::getTileColor
        // ===================================================================
        u32 getTileColor(int tileType) const;

        // ===================================================================
        // SummerS3::drawGrid
        // ===================================================================
        void drawGrid() const;

        // ===================================================================
        // SummerS3::drawTiles
        // ===================================================================
        void drawTiles() const;

        // ===================================================================
        // SummerS3::gridToWorld
        // ===================================================================
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;

        std::vector<IceTileState> iceTiles; // IceTileState comes from winter.hpp
    };

    // -------------------------------------------------------------------
    // SummerS4 - Stage 4
    // -------------------------------------------------------------------
    class SummerS4 {
    public:
        SummerS4();
        ~SummerS4();
        SummerS4(const SummerS4&) = delete;
        int update(float dt);
        void draw() const;

        // Grid dimensions - 32 columns width x 20 rows height.
        static const int gridCols = 32;
        static const int gridRows = 20;

        // ===================================================================
        // SummerS4::getTileMap
        // ===================================================================
        int (*getTileMap())[gridCols] { return tileMap; }

        // ===================================================================
        // SummerS4::reset
        // ===================================================================
        void reset();

    private:
        bool gridVisible;
        // Tile map: 0=empty, 1=ground, etc.
        int tileMap[gridRows][gridCols];

        // ===================================================================
        // SummerS4::getTileColor
        // ===================================================================
        u32 getTileColor(int tileType) const;

        // ===================================================================
        // SummerS4::drawGrid
        // ===================================================================
        void drawGrid() const;

        // ===================================================================
        // SummerS4::drawTiles
        // ===================================================================
        void drawTiles() const;

        // ===================================================================
        // SummerS4::gridToWorld
        // ===================================================================
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;

        std::vector<IceTileState> iceTiles; // IceTileState comes from winter.hpp
    };

} // namespace game

#endif // SUMMER_HPP