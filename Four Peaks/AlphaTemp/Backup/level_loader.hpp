#ifndef LEVEL_LOADER_HPP
#define LEVEL_LOADER_HPP

// Simple tile grid loader.
// Reads integers from a text file and fills a rows x cols grid.
// Supports separators: spaces, commas, tabs.
// Supports comments: lines starting with '#' or '//' are ignored.
// Rows in file are expected to be: row 0 (bottom) first, row (rows-1) last.

namespace level
{
    // Returns true if file was read successfully AND exactly rows*cols integers were found.
    // On failure, outGrid is still filled with whatever was parsed so far; caller can decide to fallback.
    bool loadTileMap(const char* filePath, int rows, int cols, int* outGrid);
}

#endif // LEVEL_LOADER_HPP
