#pragma once

namespace game
{
    class Tutorial1
    {
    public:
        Tutorial1();
        int update(float dt);
        void draw() const;
        int (*getTileMap())[32];

    private:
        static constexpr int gridRows = 20;
        static constexpr int gridCols = 32;

        bool gridVisible{ true };
        int tileMap[gridRows][gridCols]{};
    };

    class Tutorial2
    {
    public:
        Tutorial2();
        int update(float dt);
        void draw() const;
        int (*getTileMap())[32];

    private:
        static constexpr int gridRows = 20;
        static constexpr int gridCols = 32;

        bool gridVisible{ true };
        int tileMap[gridRows][gridCols]{};
    };

    class Tutorial3
    {
    public:
        Tutorial3();
        int update(float dt);
        void draw() const;
        int (*getTileMap())[32];

    private:
        static constexpr int gridRows = 20;
        static constexpr int gridCols = 32;

        bool gridVisible{ true };
        int tileMap[gridRows][gridCols]{};
    };
}
