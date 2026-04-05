#ifndef SUMMER_HPP
#define SUMMER_HPP

#include <cstdint>
#include <vector>
#include "engine/sprite.hpp"
#include "scenes/winter.hpp"   // IceTileState is defined here — don't redefine it

typedef uint32_t u32;

namespace game {

    // -------------------------------------------------------------------------
    // Heat bar utilities — drives the summer heat-drain mechanic.
    // HeatUpdate decays the player's heat each frame and kills the player if
    // it reaches zero. HeatDraw renders the heat bar HUD in screen space.
    // -------------------------------------------------------------------------
    void HeatUpdate(float dt);
    void HeatDraw();

    // -------------------------------------------------------------------------
    // Summer stages (4) — each stage loads its tile layout from
    // Assets/Levels/summer_sX.txt at construction.
    // Stages 3 and 4 additionally track breakable ice tiles (IceTileState).
    // Each stage class follows the same pattern:
    //   update(dt) — advances game logic and returns a scene-change code
    //   draw()     — renders background, tiles, heat bar, and player
    //   reset()    — reloads the tile map and respawns collectibles on death
    // -------------------------------------------------------------------------

    class SummerS1 {
    public:
        SummerS1();
        ~SummerS1();
        SummerS1(const SummerS1&) = delete;

        // -------------------------------------------------------------------------
        // Runs one frame of stage logic (player, heat decay, teleport zone check).
        // Returns a non-zero scene-transition code or 0 to stay in this stage.
        // -------------------------------------------------------------------------
        int update(float dt);

        // -------------------------------------------------------------------------
        // Renders the background, solid tiles, heat bar HUD, and player.
        // Also draws the teleport zone indicator rectangle.
        // -------------------------------------------------------------------------
        void draw() const;

        // -------------------------------------------------------------------------
        // Reloads the tile map from disk and re-removes any already-collected
        // melons. Call this on player death before re-entering the stage.
        // -------------------------------------------------------------------------
        void reset();

        static const int gridCols = 32;
        static const int gridRows = 20;

        // -------------------------------------------------------------------------
        // Returns a pointer to the 2-D tile map so the collision system can read
        // and modify tile values (e.g. clearing a collected pickup to 0).
        // -------------------------------------------------------------------------
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        int  tileMap[gridRows][gridCols];

        // -------------------------------------------------------------------------
        // Returns the debug-draw colour for a given tile type ID.
        // -------------------------------------------------------------------------
        u32  getTileColor(int tileType) const;

        // -------------------------------------------------------------------------
        // Draws the debug grid overlay (grid lines only, no tile content).
        // -------------------------------------------------------------------------
        void drawGrid() const;

        // -------------------------------------------------------------------------
        // Draws all visible tiles using their sprite or colour representation,
        // including border lines on the exposed edges of solid tiles.
        // -------------------------------------------------------------------------
        void drawTiles() const;

        // -------------------------------------------------------------------------
        // Converts a tile (col, row) index to world-space centre position and cell
        // dimensions, accounting for the current viewport bounds.
        // -------------------------------------------------------------------------
        void gridToWorld(int col, int row, float& xWorld, float& yWorld,
            float& cellW, float& cellH) const;
    };

    class SummerS2 {
    public:
        SummerS2();
        ~SummerS2();
        SummerS2(const SummerS2&) = delete;

        // -------------------------------------------------------------------------
        // Runs one frame of stage logic (player, heat decay, teleport zone check).
        // Returns a non-zero scene-transition code or 0 to stay in this stage.
        // -------------------------------------------------------------------------
        int update(float dt);

        // -------------------------------------------------------------------------
        // Renders the background, solid tiles, heat bar HUD, and player.
        // Also draws the teleport zone indicator rectangle.
        // -------------------------------------------------------------------------
        void draw() const;

        // -------------------------------------------------------------------------
        // Reloads the tile map from disk and re-removes any already-collected
        // melons. Call this on player death before re-entering the stage.
        // -------------------------------------------------------------------------
        void reset();

        static const int gridCols = 32;
        static const int gridRows = 20;

        // -------------------------------------------------------------------------
        // Returns a pointer to the 2-D tile map so the collision system can read
        // and modify tile values (e.g. clearing a collected pickup to 0).
        // -------------------------------------------------------------------------
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        int  tileMap[gridRows][gridCols];

        // -------------------------------------------------------------------------
        // Returns the debug-draw colour for a given tile type ID.
        // -------------------------------------------------------------------------
        u32  getTileColor(int tileType) const;

        // -------------------------------------------------------------------------
        // Draws the debug grid overlay (grid lines only, no tile content).
        // -------------------------------------------------------------------------
        void drawGrid() const;

        // -------------------------------------------------------------------------
        // Draws all visible tiles using their sprite or colour representation,
        // including border lines on the exposed edges of solid tiles.
        // -------------------------------------------------------------------------
        void drawTiles() const;

        // -------------------------------------------------------------------------
        // Converts a tile (col, row) index to world-space centre position and cell
        // dimensions, accounting for the current viewport bounds.
        // -------------------------------------------------------------------------
        void gridToWorld(int col, int row, float& xWorld, float& yWorld,
            float& cellW, float& cellH) const;
    };

    class SummerS3 {
    public:
        SummerS3();
        ~SummerS3();
        SummerS3(const SummerS3&) = delete;

        // -------------------------------------------------------------------------
        // Runs one frame of stage logic (player, heat decay, ice tile crack timers,
        // teleport zone check). Returns a non-zero scene-transition code or 0.
        // -------------------------------------------------------------------------
        int update(float dt);

        // -------------------------------------------------------------------------
        // Renders the background, solid tiles (including cracking ice frames),
        // heat bar HUD, and player. Also draws the teleport zone indicator.
        // -------------------------------------------------------------------------
        void draw() const;

        // -------------------------------------------------------------------------
        // Reloads the tile map, re-removes collected melons, clears and rebuilds
        // the iceTiles list from the fresh map. Call this on player death.
        // -------------------------------------------------------------------------
        void reset();

        static const int gridCols = 32;
        static const int gridRows = 20;

        // -------------------------------------------------------------------------
        // Returns a pointer to the 2-D tile map so the collision system can read
        // and modify tile values (e.g. clearing a destroyed ice tile to 0).
        // -------------------------------------------------------------------------
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        int  tileMap[gridRows][gridCols];

        // -------------------------------------------------------------------------
        // Returns the debug-draw colour for a given tile type ID.
        // -------------------------------------------------------------------------
        u32  getTileColor(int tileType) const;

        // -------------------------------------------------------------------------
        // Draws the debug grid overlay (grid lines only, no tile content).
        // -------------------------------------------------------------------------
        void drawGrid() const;

        // -------------------------------------------------------------------------
        // Draws all visible tiles using their sprite or colour representation.
        // Ice tiles (type 1) are drawn using the current crack animation frame
        // stored in iceTiles; border lines are drawn on exposed solid edges.
        // -------------------------------------------------------------------------
        void drawTiles() const;

        // -------------------------------------------------------------------------
        // Converts a tile (col, row) index to world-space centre position and cell
        // dimensions, accounting for the current viewport bounds.
        // -------------------------------------------------------------------------
        void gridToWorld(int col, int row, float& xWorld, float& yWorld,
            float& cellW, float& cellH) const;

        std::vector<IceTileState> iceTiles; // IceTileState comes from winter.hpp
    };

    class SummerS4 {
    public:
        SummerS4();
        ~SummerS4();
        SummerS4(const SummerS4&) = delete;

        // -------------------------------------------------------------------------
        // Runs one frame of stage logic (player, heat decay, ice tile crack timers,
        // teleport zone check). Returns a non-zero scene-transition code or 0.
        // -------------------------------------------------------------------------
        int update(float dt);

        // -------------------------------------------------------------------------
        // Renders the background, solid tiles (including cracking ice frames),
        // heat bar HUD, and player. Also draws the teleport zone indicator.
        // -------------------------------------------------------------------------
        void draw() const;

        // -------------------------------------------------------------------------
        // Reloads the tile map, re-removes collected melons, clears and rebuilds
        // the iceTiles list from the fresh map. Call this on player death.
        // -------------------------------------------------------------------------
        void reset();

        static const int gridCols = 32;
        static const int gridRows = 20;

        // -------------------------------------------------------------------------
        // Returns a pointer to the 2-D tile map so the collision system can read
        // and modify tile values (e.g. clearing a destroyed ice tile to 0).
        // -------------------------------------------------------------------------
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        int  tileMap[gridRows][gridCols];

        // -------------------------------------------------------------------------
        // Returns the debug-draw colour for a given tile type ID.
        // -------------------------------------------------------------------------
        u32  getTileColor(int tileType) const;

        // -------------------------------------------------------------------------
        // Draws the debug grid overlay (grid lines only, no tile content).
        // -------------------------------------------------------------------------
        void drawGrid() const;

        // -------------------------------------------------------------------------
        // Draws all visible tiles using their sprite or colour representation.
        // Ice tiles (type 1) are drawn using the current crack animation frame
        // stored in iceTiles; border lines are drawn on exposed solid edges.
        // -------------------------------------------------------------------------
        void drawTiles() const;

        // -------------------------------------------------------------------------
        // Converts a tile (col, row) index to world-space centre position and cell
        // dimensions, accounting for the current viewport bounds.
        // -------------------------------------------------------------------------
        void gridToWorld(int col, int row, float& xWorld, float& yWorld,
            float& cellW, float& cellH) const;

        std::vector<IceTileState> iceTiles; // IceTileState comes from winter.hpp
    };

} // namespace game

#endif // SUMMER_HPP