#ifndef AUTUMN_HPP
#define AUTUMN_HPP

#include <cstdint>
#include "engine/sprite.hpp"
#include <vector>

typedef uint32_t u32;

namespace game
{
    // -------------------------------------------------------------------------
    // Tracks the crack-and-destroy animation state for a single breakable tile.
    // The collision system queues a tile for triggering; the stage update drives
    // the timer and frame selection until destroyed is set.
    // -------------------------------------------------------------------------
    struct BreakableTileState {
        int   row{}, col{};       // Tile grid position
        bool  triggered = false;  // True once the player has stepped on the tile
        float timer = 0.0f;   // Seconds elapsed since the tile was triggered
        int   crackFrame = 0;     // Current animation frame index (0 = intact)
        bool  destroyed = false; // True when the tile has been fully removed
    };

    // -------------------------------------------------------------------------
    // Autumn stages (4) — intended as the final season.
    // Tile layouts are loaded from Assets/Levels/autumn_sX.txt at construction.
    // Each stage class follows the same pattern:
    //   update(dt) — advances game logic and returns a scene-change code
    //   draw()     — renders all tiles (and breakable-tile crack frames)
    //   resetBreakableTile() — clears break state when the stage is reloaded
    // -------------------------------------------------------------------------

    class AutumnS1 {
    public:
        AutumnS1();
        ~AutumnS1();
        AutumnS1(const AutumnS1&) = delete;

        // -------------------------------------------------------------------------
        // Runs one frame of stage logic (player interaction, breakable tile timers).
        // Returns a non-zero scene-transition code or 0 to stay in this stage.
        // -------------------------------------------------------------------------
        int  update(float dt);

        // -------------------------------------------------------------------------
        // Renders the background, solid tiles, and any in-progress crack animations.
        // -------------------------------------------------------------------------
        void draw() const;

        // -------------------------------------------------------------------------
        // Resets all BreakableTileState entries to their initial untriggered values.
        // Call this before transitioning back into the stage from another scene.
        // -------------------------------------------------------------------------
        void resetBreakableTile();

        static const int gridCols = 32;
        static const int gridRows = 20;

        // -------------------------------------------------------------------------
        // Returns a pointer to the 2-D tile map so the collision system can read
        // and modify tile values (e.g. clearing a destroyed tile to 0).
        // -------------------------------------------------------------------------
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible{};
        int  tileMap[gridRows][gridCols]{};

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
        // -------------------------------------------------------------------------
        void drawTiles() const;

        // -------------------------------------------------------------------------
        // Converts a tile (col, row) index to world-space centre position and cell
        // dimensions, accounting for the current camera Y offset.
        // -------------------------------------------------------------------------
        void gridToWorld(int col, int row, float& xWorld, float& yWorld,
            float& cellW, float& cellH) const;

        std::vector<BreakableTileState> breakableTiles;
    };

    class AutumnS2 {
    public:
        AutumnS2();
        ~AutumnS2();
        AutumnS2(const AutumnS2&) = delete;

        // -------------------------------------------------------------------------
        // Advances stage logic. Returns a scene-transition code or 0 to continue.
        // -------------------------------------------------------------------------
        int  update(float dt);

        // -------------------------------------------------------------------------
        // Renders the stage tile layer.
        // -------------------------------------------------------------------------
        void draw() const;

        // -------------------------------------------------------------------------
        // Resets all breakable tile animation states for a clean re-entry.
        // -------------------------------------------------------------------------
        void resetBreakableTile();

        static const int gridCols = 32;
        static const int gridRows = 20;

        // -------------------------------------------------------------------------
        // Exposes the live tile map to external systems (e.g. collision).
        // -------------------------------------------------------------------------
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible{};
        int  tileMap[gridRows][gridCols]{};
        u32  getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld,
            float& cellW, float& cellH) const;

        std::vector<BreakableTileState> breakableTiles;
    };

    class AutumnS3 {
    public:
        AutumnS3();
        ~AutumnS3();
        AutumnS3(const AutumnS3&) = delete;

        // -------------------------------------------------------------------------
        // Advances stage logic. Returns a scene-transition code or 0 to continue.
        // -------------------------------------------------------------------------
        int  update(float dt);

        // -------------------------------------------------------------------------
        // Renders the stage tile layer.
        // -------------------------------------------------------------------------
        void draw() const;

        // -------------------------------------------------------------------------
        // Resets all breakable tile animation states for a clean re-entry.
        // -------------------------------------------------------------------------
        void resetBreakableTile();

        static const int gridCols = 32;
        static const int gridRows = 20;

        // -------------------------------------------------------------------------
        // Exposes the live tile map to external systems (e.g. collision).
        // -------------------------------------------------------------------------
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible{};
        int  tileMap[gridRows][gridCols]{};
        u32  getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld,
            float& cellW, float& cellH) const;

        std::vector<BreakableTileState> breakableTiles;
    };

    class AutumnS4 {
    public:
        AutumnS4();
        ~AutumnS4();
        AutumnS4(const AutumnS4&) = delete;

        // -------------------------------------------------------------------------
        // Advances stage logic. Returns a scene-transition code or 0 to continue.
        // -------------------------------------------------------------------------
        int  update(float dt);

        // -------------------------------------------------------------------------
        // Renders the stage tile layer.
        // -------------------------------------------------------------------------
        void draw() const;

        // -------------------------------------------------------------------------
        // Resets all breakable tile animation states for a clean re-entry.
        // -------------------------------------------------------------------------
        void resetBreakableTile();

        static const int gridCols = 32;
        static const int gridRows = 20;

        // -------------------------------------------------------------------------
        // Exposes the live tile map to external systems (e.g. collision).
        // -------------------------------------------------------------------------
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible{};
        int  tileMap[gridRows][gridCols]{};
        u32  getTileColor(int tileType) const;
        void drawGrid() const;
        void drawTiles() const;
        void gridToWorld(int col, int row, float& xWorld, float& yWorld,
            float& cellW, float& cellH) const;

        std::vector<BreakableTileState> breakableTiles;
    };

} // namespace game

#endif // AUTUMN_HPP