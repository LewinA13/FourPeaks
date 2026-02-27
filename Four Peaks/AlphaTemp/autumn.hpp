#ifndef AUTUMN_HPP
#define AUTUMN_HPP

#include <cstdint>
#include "sprite.hpp"

typedef uint32_t u32;

namespace game
{
    // -------------------------------------------------------------------
    // Autumn stages (4) - intended as the last season.
    // Layouts are loaded from Assets/Levels/autumn_sX.txt
    // -------------------------------------------------------------------
    class AutumnS1 {
    public:
        AutumnS1();
        ~AutumnS1();
        AutumnS1(const AutumnS1&) = delete;
        int update(float dt);
        void draw() const;

        static const int gridCols = 32;
        static const int gridRows = 20;
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible{};
        int tileMap[gridRows][gridCols]{};
        u32 getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };

    class AutumnS2 {
    public:
        AutumnS2();
        ~AutumnS2();
        AutumnS2(const AutumnS2&) = delete;
        int update(float dt);
        void draw() const;

        static const int gridCols = 32;
        static const int gridRows = 20;
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible{};
        int tileMap[gridRows][gridCols]{};
        u32 getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };

    class AutumnS3 {
    public:
        AutumnS3();
        ~AutumnS3();
        AutumnS3(const AutumnS3&) = delete;
        int update(float dt);
        void draw() const;

        static const int gridCols = 32;
        static const int gridRows = 20;
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible{};
        int tileMap[gridRows][gridCols]{};
        u32 getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };

    class AutumnS4 {
    public:
        AutumnS4();
        ~AutumnS4();
        AutumnS4(const AutumnS4&) = delete;
        int update(float dt);
        void draw() const;

        static const int gridCols = 32;
        static const int gridRows = 20;
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible{};
        int tileMap[gridRows][gridCols]{};
        u32 getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld, float& cellW, float& cellH) const;
    };
}

#endif // AUTUMN_HPP
