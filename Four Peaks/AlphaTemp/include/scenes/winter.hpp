// ----------------------------------------------------------------------------
// Done By: Hong Yang, Arun, Skyler, Justin
// ----------------------------------------------------------------------------

#ifndef WINTERHPP
#define WINTERHPP

//---------------------------------------------------------------------------
// Copyright (C) 2026 Team Game++ (Four Peaks)
// All rights reserved.
//
// This file is part of the Four Peaks project. All code, design, and original
// assets are the work of LewinA and team members unless otherwise stated.
//
// Audio assets are sourced from Soundly and used under appropriate licensing.
//
// Reproduction, distribution, or modification of this file or its contents,
// in whole or in part, without prior written permission is strictly prohibited.
//
//---------------------------------------------------------------------------

#include <cstdint>
#include <vector>
#include <array>
#include "engine/sprite.hpp"

typedef uint32_t u32;

// -------------------------------------------------------------------------
// Represents a single falling snowflake in the snow particle system.
// Each particle tracks its position, velocity, size, alpha, and active state.
// -------------------------------------------------------------------------
struct SnowParticle {
    float x, y;
    float velX, velY;
    float size;
    float alpha;
    bool active = false;
};

static constexpr int MAX_SNOW = 120;

namespace game {

    // -------------------------------------------------------------------------
    // Winter Stage 1 — loads Assets/Levels/winter_s1.txt on construction.
    // Transitions to WinterS2 (scene code 20) when the player reaches the
    // teleport zone at columns 28-29, row 19.
    // -------------------------------------------------------------------------
    class WinterS1 {
    public:
        WinterS1();
        ~WinterS1();
        WinterS1(const WinterS1&) = delete;

        // -------------------------------------------------------------------------
        // Advances stage logic: initialises snow on the first call, toggles the
        // debug grid, updates the player, checks the teleport zone, advances
        // animated tiles, and updates snow particles.
        // Returns 20 to transition to WinterS2, or 0 to remain in this stage.
        // -------------------------------------------------------------------------
        int update(float dt);

        // -------------------------------------------------------------------------
        // Renders the background, all tiles, the optional debug grid, the teleport
        // zone indicator, the player, and the snow particle layer.
        // -------------------------------------------------------------------------
        void draw() const;

        // -------------------------------------------------------------------------
        // Grid dimensions: 32 columns wide, 20 rows tall.
        // -------------------------------------------------------------------------
        static const int gridCols = 32;
        static const int gridRows = 20;

        // -------------------------------------------------------------------------
        // Returns a pointer to the live 2-D tile map so the collision system can
        // read and modify tile values (e.g. clearing a destroyed tile to 0).
        // -------------------------------------------------------------------------
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        int  tileMap[gridRows][gridCols];

        mutable std::array<SnowParticle, MAX_SNOW> snowParticles{};
        bool  snowInitialized = false;
        float snowSpawnTimer = 0.0f;

        // -------------------------------------------------------------------------
        // Returns the debug-draw fallback colour for a given tile type ID.
        // -------------------------------------------------------------------------
        u32 getTileColor(int tileType) const;

        // -------------------------------------------------------------------------
        // Draws the debug grid overlay as thin rectangles aligned to tile cell
        // boundaries across the full viewport.
        // -------------------------------------------------------------------------
        void drawGrid() const;

        // -------------------------------------------------------------------------
        // Iterates every tile in the map and draws the appropriate sprite or colour.
        // Spike tiles are offset and scaled to close the visible gap at their bases.
        // Border lines are drawn on the exposed edges of solid tiles.
        // -------------------------------------------------------------------------
        void drawTiles() const;

        // -------------------------------------------------------------------------
        // Converts a tile (col, row) index to world-space top-left position and
        // cell dimensions, accounting for the current viewport bounds.
        // -------------------------------------------------------------------------
        void gridToWorld(int col, int row, float& xWorld, float& yWorld,
            float& cellW, float& cellH) const;
    };

    // -------------------------------------------------------------------------
    // Winter Stage 2 — loads Assets/Levels/winter_s2.txt on construction.
    // Transitions to WinterS3 (scene code 21) when the player reaches the
    // teleport zone at column 31, rows 18-19.
    // -------------------------------------------------------------------------
    class WinterS2 {
    public:
        WinterS2();
        ~WinterS2();
        WinterS2(const WinterS2&) = delete;

        // -------------------------------------------------------------------------
        // Advances stage logic: initialises snow on the first call, toggles the
        // debug grid, updates the player, checks the teleport zone, advances
        // animated tiles, and updates snow particles.
        // Returns 21 to transition to WinterS3, or 0 to remain in this stage.
        // -------------------------------------------------------------------------
        int update(float dt);

        // -------------------------------------------------------------------------
        // Renders the background, all tiles, the optional debug grid, the teleport
        // zone indicator, the player, and the snow particle layer.
        // -------------------------------------------------------------------------
        void draw() const;

        // -------------------------------------------------------------------------
        // Grid dimensions: 32 columns wide, 20 rows tall.
        // -------------------------------------------------------------------------
        static const int gridCols = 32;
        static const int gridRows = 20;

        // -------------------------------------------------------------------------
        // Returns a pointer to the live 2-D tile map so the collision system can
        // read and modify tile values (e.g. clearing a destroyed tile to 0).
        // -------------------------------------------------------------------------
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        int  tileMap[gridRows][gridCols];

        mutable std::array<SnowParticle, MAX_SNOW> snowParticles{};
        bool  snowInitialized = false;
        float snowSpawnTimer = 0.0f;

        // -------------------------------------------------------------------------
        // Returns the debug-draw fallback colour for a given tile type ID.
        // -------------------------------------------------------------------------
        u32 getTileColor(int tileType) const;

        // -------------------------------------------------------------------------
        // Draws the debug grid overlay as thin rectangles aligned to tile cell
        // boundaries across the full viewport.
        // -------------------------------------------------------------------------
        void drawGrid() const;

        // -------------------------------------------------------------------------
        // Iterates every tile in the map and draws the appropriate sprite or colour.
        // Spike tiles are offset and scaled to close the visible gap at their bases.
        // Border lines are drawn on the exposed edges of solid tiles.
        // -------------------------------------------------------------------------
        void drawTiles() const;

        // -------------------------------------------------------------------------
        // Converts a tile (col, row) index to world-space top-left position and
        // cell dimensions, accounting for the current viewport bounds.
        // -------------------------------------------------------------------------
        void gridToWorld(int col, int row, float& xWorld, float& yWorld,
            float& cellW, float& cellH) const;
    };

    // -------------------------------------------------------------------------
    // Tracks the crack-and-destroy animation state for a single breakable ice
    // tile. The collision system queues a tile for triggering; the stage update
    // drives the timer and frame selection until destroyed is set.
    // -------------------------------------------------------------------------
    struct IceTileState {
        int   row{}, col{};
        bool  triggered = false;
        float timer = 0.0f;
        int   crackFrame = 0;
        bool  destroyed = false;
    };

    // -------------------------------------------------------------------------
    // Winter Stage 3 — loads Assets/Levels/winter_s3.txt on construction.
    // Features breakable ice tiles (type 1) that crack and disappear when the
    // player stands on them. Transitions to WinterS4 (scene code 22).
    // -------------------------------------------------------------------------
    class WinterS3 {
    public:
        WinterS3();
        ~WinterS3();
        WinterS3(const WinterS3&) = delete;

        // -------------------------------------------------------------------------
        // Advances stage logic: initialises snow on the first call, toggles the
        // debug grid, updates the player, checks the teleport zone, processes ice
        // tile crack timers and tile removal, resets ice on player respawn, advances
        // animated tiles, and updates snow particles.
        // Returns 22 to transition to WinterS4, or 0 to remain in this stage.
        // -------------------------------------------------------------------------
        int update(float dt);

        // -------------------------------------------------------------------------
        // Renders the background, all tiles (including cracking ice frames), the
        // optional debug grid, the teleport zone indicator, the player, and the
        // snow particle layer.
        // -------------------------------------------------------------------------
        void draw() const;

        // -------------------------------------------------------------------------
        // Resets all IceTileState entries to their initial untriggered values and
        // restores each tile in the map to type 1.
        // Call this before transitioning back into the stage from another scene.
        // -------------------------------------------------------------------------
        void resetBreakableIce();

        // -------------------------------------------------------------------------
        // Grid dimensions: 32 columns wide, 20 rows tall.
        // -------------------------------------------------------------------------
        static const int gridCols = 32;
        static const int gridRows = 20;

        // -------------------------------------------------------------------------
        // Returns a pointer to the live 2-D tile map so the collision system can
        // read and modify tile values (e.g. clearing a destroyed tile to 0).
        // -------------------------------------------------------------------------
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        int  tileMap[gridRows][gridCols];

        mutable std::array<SnowParticle, MAX_SNOW> snowParticles{};
        bool  snowInitialized = false;
        float snowSpawnTimer = 0.0f;

        // -------------------------------------------------------------------------
        // Returns the debug-draw fallback colour for a given tile type ID.
        // -------------------------------------------------------------------------
        u32 getTileColor(int tileType) const;

        // -------------------------------------------------------------------------
        // Draws the debug grid overlay as thin rectangles aligned to tile cell
        // boundaries across the full viewport.
        // -------------------------------------------------------------------------
        void drawGrid() const;

        // -------------------------------------------------------------------------
        // Iterates every tile in the map and draws the appropriate sprite or colour.
        // Ice tiles (type 1) look up their per-tile crack frame from iceTiles and
        // select the matching UV region from the crack sprite sheet.
        // Border lines are drawn on the exposed edges of solid tiles.
        // -------------------------------------------------------------------------
        void drawTiles() const;

        // -------------------------------------------------------------------------
        // Converts a tile (col, row) index to world-space top-left position and
        // cell dimensions, accounting for the current viewport bounds.
        // -------------------------------------------------------------------------
        void gridToWorld(int col, int row, float& xWorld, float& yWorld,
            float& cellW, float& cellH) const;

        std::vector<IceTileState> iceTiles;
    };

    // -------------------------------------------------------------------------
    // Winter Stage 4 — loads Assets/Levels/winter_s4.txt on construction.
    // Features breakable ice tiles (type 1) that crack and disappear when the
    // player stands on them. Transitions to SummerS1 (scene code 23).
    // -------------------------------------------------------------------------
    class WinterS4 {
    public:
        WinterS4();
        ~WinterS4();
        WinterS4(const WinterS4&) = delete;

        // -------------------------------------------------------------------------
        // Advances stage logic: initialises snow on the first call, toggles the
        // debug grid, updates the player, advances animated tiles, processes ice
        // tile crack timers and tile removal, resets ice on player respawn, checks
        // the teleport zone, and updates snow particles.
        // Returns 23 to transition to SummerS1, or 0 to remain in this stage.
        // -------------------------------------------------------------------------
        int update(float dt);

        // -------------------------------------------------------------------------
        // Renders the background, all tiles (including cracking ice frames), the
        // optional debug grid, the teleport zone indicator, the player, and the
        // snow particle layer.
        // -------------------------------------------------------------------------
        void draw() const;

        // -------------------------------------------------------------------------
        // Resets all IceTileState entries to their initial untriggered values and
        // restores each tile in the map to type 1.
        // Call this before transitioning back into the stage from another scene.
        // -------------------------------------------------------------------------
        void resetBreakableIce();

        // -------------------------------------------------------------------------
        // Grid dimensions: 32 columns wide, 20 rows tall.
        // -------------------------------------------------------------------------
        static const int gridCols = 32;
        static const int gridRows = 20;

        // -------------------------------------------------------------------------
        // Returns a pointer to the live 2-D tile map so the collision system can
        // read and modify tile values (e.g. clearing a destroyed tile to 0).
        // -------------------------------------------------------------------------
        int (*getTileMap())[gridCols] { return tileMap; }

    private:
        bool gridVisible;
        int  tileMap[gridRows][gridCols];

        mutable std::array<SnowParticle, MAX_SNOW> snowParticles{};
        bool  snowInitialized = false;
        float snowSpawnTimer = 0.0f;

        // -------------------------------------------------------------------------
        // Returns the debug-draw fallback colour for a given tile type ID.
        // -------------------------------------------------------------------------
        u32 getTileColor(int tileType) const;

        // -------------------------------------------------------------------------
        // Draws the debug grid overlay as thin rectangles aligned to tile cell
        // boundaries across the full viewport.
        // -------------------------------------------------------------------------
        void drawGrid() const;

        // -------------------------------------------------------------------------
        // Iterates every tile in the map and draws the appropriate sprite or colour.
        // Ice tiles (type 1) look up their per-tile crack frame from iceTiles and
        // select the matching UV region from the crack sprite sheet.
        // The winter artifact (type 34) bobs vertically using a sine wave offset.
        // Border lines are drawn on the exposed edges of solid tiles.
        // -------------------------------------------------------------------------
        void drawTiles() const;

        // -------------------------------------------------------------------------
        // Converts a tile (col, row) index to world-space top-left position and
        // cell dimensions, accounting for the current viewport bounds.
        // -------------------------------------------------------------------------
        void gridToWorld(int col, int row, float& xWorld, float& yWorld,
            float& cellW, float& cellH) const;

        std::vector<IceTileState> iceTiles;
    };

} // namespace game

#endif // WINTERS1HPP