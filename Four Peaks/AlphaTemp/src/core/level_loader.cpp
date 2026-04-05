// ----------------------------------------------------------------------------
// Done By: Arun
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

// ----------------------------------------------------------------------------
// Level Loader Implementation
// Handles loading tile maps from text-based level files into a fixed-size grid.
// ----------------------------------------------------------------------------
#include "core/level_loader.hpp"
#include <fstream>
#include <string>
#include <cctype>
#include <cstdlib>

// ---------------------------------------------------------------------------
// Replace Commas With Spaces
// Converts commas into spaces so CSV rows can be parsed using the same integer
// scanning logic as TXT rows.
// ---------------------------------------------------------------------------
static void replaceCommasWithSpaces(std::string& s)
{
    for (char& c : s)
    {
        if (c == ',')
            c = ' ';
    }
}

// ---------------------------------------------------------------------------
// Build Csv Fallback Path
// If the requested file path ends in ".txt", creates a second path that points
// to a file with the same base name but a ".csv" extension.
// ---------------------------------------------------------------------------
static bool buildCsvFallbackPath(const char* filePath, std::string& outCsvPath)
{
    if (!filePath)
        return false;

    outCsvPath = filePath;

    const std::string txtExt = ".txt";
    if (outCsvPath.size() < txtExt.size())
        return false;

    const size_t extPos = outCsvPath.size() - txtExt.size();
    if (outCsvPath.compare(extPos, txtExt.size(), txtExt) != 0)
        return false;

    outCsvPath.replace(extPos, txtExt.size(), ".csv");
    return true;
}

// ---------------------------------------------------------------------------
// Parse Tile Map Stream
// Reads signed integers from the provided text stream, ignoring blank lines and
// comment lines, until the destination grid is fully populated.
// ---------------------------------------------------------------------------
static bool parseTileMapStream(std::istream& in, int rows, int cols, int* outGrid)
{
    if (!outGrid || rows <= 0 || cols <= 0)
        return false;

    const int total = rows * cols;
    for (int i = 0; i < total; ++i)
        outGrid[i] = 0;

    int count = 0;
    std::string line;

    while (std::getline(in, line))
    {
        size_t i = 0;
        while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i])))
            ++i;

        if (i >= line.size())
            continue;

        if (line[i] == '#')
            continue;

        if (line[i] == '/' && (i + 1) < line.size() && line[i + 1] == '/')
            continue;

        replaceCommasWithSpaces(line);

        const char* s = line.c_str();
        char* end = nullptr;

        while (*s)
        {
            while (*s && !std::isdigit(static_cast<unsigned char>(*s)) && *s != '-' && *s != '+')
                ++s;

            if (!*s)
                break;

            long v = std::strtol(s, &end, 10);
            if (s == end)
            {
                ++s;
                continue;
            }

            if (count < total)
                outGrid[count] = static_cast<int>(v);

            ++count;
            s = end;
        }

        if (count >= total)
            break;
    }

    return count == total;
}

namespace level
{
    // ---------------------------------------------------------------------------
    // Load Tile Map
    // Loads a level from the given path. If the original file cannot be opened
    // and it was a TXT path, the loader automatically tries the matching CSV
    // file so the rest of the game code does not need to change.
    // ---------------------------------------------------------------------------
    bool loadTileMap(const char* filePath, int rows, int cols, int* outGrid)
    {
        if (!filePath || !outGrid || rows <= 0 || cols <= 0)
            return false;

        std::ifstream in(filePath);
        if (in.is_open())
            return parseTileMapStream(in, rows, cols, outGrid);

        std::string csvPath;
        if (!buildCsvFallbackPath(filePath, csvPath))
            return false;

        std::ifstream csvIn(csvPath.c_str());
        if (!csvIn.is_open())
            return false;

        return parseTileMapStream(csvIn, rows, cols, outGrid);
    }
}
