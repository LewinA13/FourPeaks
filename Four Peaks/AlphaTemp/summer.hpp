#ifndef SUMMER_HPP
#define SUMMER_HPP

#include <cstdint>
#include <vector>
#include "sprite.hpp"
#include "winter.hpp"   // IceTileState is defined here — don't redefine it

typedef uint32_t u32;

namespace game {

    // -------------------------------------------------------------------
    // Heat Bar for winter
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
        static const int gridCols = 32;
        static const int gridRows = 20;
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
    // SummerS2 - Stage 2
    // -------------------------------------------------------------------
    class SummerS2 {
    public:
        SummerS2();
        ~SummerS2();
        SummerS2(const SummerS2&) = delete;
        int update(float dt);
        void draw() const;
        static const int gridCols = 32;
        static const int gridRows = 20;
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
    // SummerS3 - Stage 3
    // -------------------------------------------------------------------
    class SummerS3 {
    public:
        SummerS3();
        ~SummerS3();
        SummerS3(const SummerS3&) = delete;
        int update(float dt);
        void draw() const;
        static const int gridCols = 32;
        static const int gridRows = 20;
        int (*getTileMap())[gridCols] { return tileMap; }
    private:
        bool gridVisible;
        int tileMap[gridRows][gridCols];
        u32 getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
        std::vector<IceTileState> iceTiles;  // IceTileState comes from winter.hpp
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
        static const int gridCols = 32;
        static const int gridRows = 20;
        int (*getTileMap())[gridCols] { return tileMap; }
    private:
        bool gridVisible;
        int tileMap[gridRows][gridCols];
        u32 getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
        std::vector<IceTileState> iceTiles;  // IceTileState comes from winter.hpp
    };

} // namespace game

#endif // SUMMER_HPP
