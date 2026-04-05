// ----------------------------------------------------------------------------
// Done By: Hong Yang, Arun, Skyler, Justin
// ----------------------------------------------------------------------------

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

#pragma once
// ---------------------------------------------------------------------------
// tutorial.hpp
// ---------------------------------------------------------------------------
// Tutorial levels (1–3) declarations.
// Each tutorial manages a 20x32 tile map grid, optional grid overlay, 
// update logic, drawing, and provides access to the tile map.
// ---------------------------------------------------------------------------

namespace game
{
    // ===================================================================
    // Tutorial1
    // ===================================================================
    class Tutorial1
    {
    public:
        // -------------------------------------------------------------------
        // Constructor
        // Initializes tile map to empty (all zeros) and sets up getTileMap pointer.
        // -------------------------------------------------------------------
        Tutorial1();

        // -------------------------------------------------------------------
        // update
        // Advance tutorial state. Called once per frame.
        // Parameters:
        //   dt - delta time since last frame (seconds)
        // Returns:
        //   0 — tutorial continues
        //   1 — tutorial completed / ready to switch scene (placeholder)
        // -------------------------------------------------------------------
        int update(float dt);

        // -------------------------------------------------------------------
        // draw
        // Render the tutorial tile map and optional grid overlay.
        // Call after updating each frame.
        // -------------------------------------------------------------------
        void draw() const;

        // -------------------------------------------------------------------
        // getTileMap
        // Provides external access to the internal 2D tile map.
        // Usage: int (*map)[32] = tutorial.getTileMap();
        // -------------------------------------------------------------------
        int (*getTileMap())[32];

    private:
        static constexpr int gridRows = 20;  // number of rows in tile map
        static constexpr int gridCols = 32;  // number of columns in tile map

        bool gridVisible{ false };           // toggle grid overlay display
        int tileMap[gridRows][gridCols]{};   // 2D array representing tiles
    };

    // ===================================================================
    // Tutorial2
    // ===================================================================
    class Tutorial2
    {
    public:
        // -------------------------------------------------------------------
        // Constructor
        // Initializes tile map and getTileMap pointer.
        // -------------------------------------------------------------------
        Tutorial2();

        // -------------------------------------------------------------------
        // update
        // Advance tutorial state. Called once per frame.
        // -------------------------------------------------------------------
        int update(float dt);

        // -------------------------------------------------------------------
        // draw
        // Render the tutorial tile map and optional grid overlay.
        // -------------------------------------------------------------------
        void draw() const;

        // -------------------------------------------------------------------
        // getTileMap
        // Provides access to internal tile map array.
        // -------------------------------------------------------------------
        int (*getTileMap())[32];

    private:
        static constexpr int gridRows = 20;
        static constexpr int gridCols = 32;

        bool gridVisible{ false };
        int tileMap[gridRows][gridCols]{};
    };

    // ===================================================================
    // Tutorial3
    // ===================================================================
    class Tutorial3
    {
    public:
        // -------------------------------------------------------------------
        // Constructor
        // Initializes tile map and getTileMap pointer.
        // -------------------------------------------------------------------
        Tutorial3();

        // -------------------------------------------------------------------
        // update
        // Advance tutorial state. Called once per frame.
        // -------------------------------------------------------------------
        int update(float dt);

        // -------------------------------------------------------------------
        // draw
        // Render the tutorial tile map and optional grid overlay.
        // -------------------------------------------------------------------
        void draw() const;

        // -------------------------------------------------------------------
        // getTileMap
        // Provides access to internal tile map array.
        // -------------------------------------------------------------------
        int (*getTileMap())[32];

    private:
        static constexpr int gridRows = 20;
        static constexpr int gridCols = 32;

        bool gridVisible{ false };
        int tileMap[gridRows][gridCols]{};
    };
}