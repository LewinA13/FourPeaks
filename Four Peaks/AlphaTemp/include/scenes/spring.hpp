#ifndef SPRING_HPP
#define SPRING_HPP

#include <cstdint>
#include "engine/sprite.hpp"

typedef uint32_t u32;

namespace game
{
    // -------------------------------------------------------------------
    // Spring stages (4)
    // Layouts are loaded from Assets/Levels/spring_sX.txt
    // -------------------------------------------------------------------
    class SpringS1 {
    public:
// ---------------------------------------------------------------------------
// Spring S1
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        SpringS1();
// ---------------------------------------------------------------------------
// Spring S1
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        ~SpringS1();
        SpringS1(const SpringS1&) = delete;
// ---------------------------------------------------------------------------
// Update
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        int update(float dt);
// ---------------------------------------------------------------------------
// Draw
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void draw() const;

        static const int gridCols = 32;
        static const int gridRows = 20;
// ---------------------------------------------------------------------------
// Int
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible{};
        int tileMap[gridRows][gridCols]{};
// ---------------------------------------------------------------------------
// Get Tile Color
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        u32 getTileColor(int tileType) const;
// ---------------------------------------------------------------------------
// Draw Grid
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void drawGrid() const;
// ---------------------------------------------------------------------------
// Draw Tiles
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void drawTiles() const;
// ---------------------------------------------------------------------------
// Grid To World
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };

    class SpringS2 {
    public:
// ---------------------------------------------------------------------------
// Spring S2
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        SpringS2();
// ---------------------------------------------------------------------------
// Spring S2
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        ~SpringS2();
        SpringS2(const SpringS2&) = delete;
// ---------------------------------------------------------------------------
// Update
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        int update(float dt);
// ---------------------------------------------------------------------------
// Draw
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void draw() const;

        static const int gridCols = 32;
        static const int gridRows = 20;
// ---------------------------------------------------------------------------
// Int
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible{};
        int tileMap[gridRows][gridCols]{};
// ---------------------------------------------------------------------------
// Get Tile Color
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        u32 getTileColor(int tileType) const;
// ---------------------------------------------------------------------------
// Draw Grid
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void drawGrid() const;
// ---------------------------------------------------------------------------
// Draw Tiles
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void drawTiles() const;
// ---------------------------------------------------------------------------
// Grid To World
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };

    class SpringS3 {
    public:
// ---------------------------------------------------------------------------
// Spring S3
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        SpringS3();
// ---------------------------------------------------------------------------
// Spring S3
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        ~SpringS3();
        SpringS3(const SpringS3&) = delete;
// ---------------------------------------------------------------------------
// Update
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        int update(float dt);
// ---------------------------------------------------------------------------
// Draw
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void draw() const;

        static const int gridCols = 32;
        static const int gridRows = 20;
// ---------------------------------------------------------------------------
// Int
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible{};
        int tileMap[gridRows][gridCols]{};
// ---------------------------------------------------------------------------
// Get Tile Color
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        u32 getTileColor(int tileType) const;
// ---------------------------------------------------------------------------
// Draw Grid
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void drawGrid() const;
// ---------------------------------------------------------------------------
// Draw Tiles
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void drawTiles() const;
// ---------------------------------------------------------------------------
// Grid To World
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };

    class SpringS4 {
    public:
// ---------------------------------------------------------------------------
// Spring S4
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        SpringS4();
// ---------------------------------------------------------------------------
// Spring S4
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        ~SpringS4();
        SpringS4(const SpringS4&) = delete;
// ---------------------------------------------------------------------------
// Update
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        int update(float dt);
// ---------------------------------------------------------------------------
// Draw
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void draw() const;

        static const int gridCols = 32;
        static const int gridRows = 20;
// ---------------------------------------------------------------------------
// Int
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible{};
        int tileMap[gridRows][gridCols]{};
// ---------------------------------------------------------------------------
// Get Tile Color
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        u32 getTileColor(int tileType) const;
// ---------------------------------------------------------------------------
// Draw Grid
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void drawGrid() const;
// ---------------------------------------------------------------------------
// Draw Tiles
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void drawTiles() const;
// ---------------------------------------------------------------------------
// Grid To World
// Explains what this function does and where its main work happens.
// ---------------------------------------------------------------------------
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };
}

#endif // SPRING_HPP
