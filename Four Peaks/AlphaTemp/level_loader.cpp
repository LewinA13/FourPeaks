#include "level_loader.hpp"

#include <fstream>
#include <string>
#include <cctype>
#include <cstdlib>

// Small helper: treat comma as whitespace so >> works consistently.
static void replaceCommasWithSpaces(std::string& s)
{
    for (char& c : s)
    {
        if (c == ',')
            c = ' ';
    }
}

namespace level
{
    bool loadTileMap(const char* filePath, int rows, int cols, int* outGrid)
    {
        if (!outGrid || rows <= 0 || cols <= 0)
            return false;

        // Default-fill with 0 so partially-read files behave predictably.
        const int total = rows * cols;
        for (int i = 0; i < total; ++i)
            outGrid[i] = 0;

        std::ifstream in(filePath);
        if (!in.is_open())
            return false;

        int count = 0;

        std::string line;
        while (std::getline(in, line))
        {
            // Trim leading whitespace
            size_t i = 0;
            while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i])))
                ++i;

            // Skip empty lines
            if (i >= line.size())
                continue;

            // Skip comments: #... or //...
            if (line[i] == '#')
                continue;
            if (line[i] == '/' && (i + 1) < line.size() && line[i + 1] == '/')
                continue;

            replaceCommasWithSpaces(line);

            // Parse ints from this line manually (robust against extra text at end)
            // We scan tokens and try to parse ints.
            const char* s = line.c_str();
            char* end = nullptr;

            while (*s)
            {
                // Skip non-number, non-sign chars
                while (*s && !std::isdigit(static_cast<unsigned char>(*s)) && *s != '-' && *s != '+')
                    ++s;
                if (!*s)
                    break;

                long v = std::strtol(s, &end, 10);
                if (s == end) // no progress
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
}
