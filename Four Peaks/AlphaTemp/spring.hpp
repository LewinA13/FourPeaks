#ifndef SPRING_HPP
#define SPRING_HPP

#include <cstdint>
#include "sprite.hpp"

typedef uint32_t u32;

namespace game
{
    // -------------------------------------------------------------------
    // Spring stages (4)
    // Layouts are loaded from Assets/Levels/spring_sX.txt
    // -------------------------------------------------------------------
    class SpringS1 {
    public:
        SpringS1();
        ~SpringS1();
        SpringS1(const SpringS1&) = delete;
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

    class SpringS2 {
    public:
        SpringS2();
        ~SpringS2();
        SpringS2(const SpringS2&) = delete;
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

    class SpringS3 {
    public:
        SpringS3();
        ~SpringS3();
        SpringS3(const SpringS3&) = delete;
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

    class SpringS4 {
    public:
        SpringS4();
        ~SpringS4();
        SpringS4(const SpringS4&) = delete;
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

#endif // SPRING_HPP
