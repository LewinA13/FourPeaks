#ifndef LEVEL_LOADER_HPP
#define LEVEL_LOADER_HPP

// Simple tile grid loader.
// Reads integers from a text file or CSV file and fills a rows x cols grid.
// Supports separators: spaces, commas, tabs.
// Supports comments: lines starting with '#' or '//' are ignored.
// Rows in file are expected to be: row 0 (bottom) first, row (rows-1) last.

namespace level
{
    // ---------------------------------------------------------------------------
    // Load Tile Map
    // Reads a level grid from disk into outGrid. The loader first attempts to read
    // the exact path it is given. If that file is missing and the path ends in
    // ".txt", it automatically retries with the same base name but ".csv".
    // ---------------------------------------------------------------------------
    bool loadTileMap(const char* filePath, int rows, int cols, int* outGrid);
}

#endif // LEVEL_LOADER_HPP
