#include "credit.hpp"
#include "graphics.hpp"
#include "AEEngine.h"
#include <stdio.h>

#define SCREEN_W  1600.0f
#define SCREEN_H  900.0f
#define HALF_W    800.0f
#define HALF_H    450.0f

extern s8 gFontId;  // Font handle created in main.cpp

// ---- CREDIT DATA ----
static const char* TEAM_NAME = "GAME++";
static const char* GAME_TITLE = "Four Peaks";

static const char* TEAM_MEMBERS[] = { "Arun", "Hong Yang", "Justin", "Skyler" };
static const int   TEAM_MEMBER_COUNT = 4;

static const char* FACULTY[] = {
    "Gerald Wong",
    "Soroor Malekmohammadi Faradounbeh",
    "Tommy Tan"
};
static const int FACULTY_COUNT = 3;
static const int CREDITS_YEAR = 2025;

// ---- TEXT HELPER ----
static void DrawTextCentered(const char* text,
    float ae_cx, float ae_cy,
    float size,
    float r, float g, float b, float a)
{
    float scale = size / 54.0f;

    float textW = 0.0f, textH = 0.0f;
    AEGfxGetPrintSize(gFontId, text, scale, &textW, &textH);

    float ndcX = (ae_cx / HALF_W) - textW * 0.5f;
    float ndcY = ae_cy / HALF_H;

    AEGfxPrint(gFontId, text, ndcX, ndcY, scale, r, g, b, a);
}

// ---- ROLLING CREDITS ----
static void DrawRollingCredits(float baseY)
{
    const float cx = 0.0f;
    const float line_spacing = 80.0f;
    const float sec_spacing = 110.0f;
    float y = baseY;

    // Game title
    DrawTextCentered(GAME_TITLE, cx, y, 260.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    y -= sec_spacing;

    // Team name
    char team_line[128];
    snprintf(team_line, sizeof team_line, "A Game by %s", TEAM_NAME);
    DrawTextCentered(team_line, cx, y, 128.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    y -= line_spacing;

    y -= 40.0f;
    y -= sec_spacing * 0.5f;

    // Development Team
    DrawTextCentered("Development Team", cx, y, 128.0f, 1.0f, 0.85f, 0.3f, 1.0f);
    y -= sec_spacing * 0.8f;
    for (int i = 0; i < TEAM_MEMBER_COUNT; ++i) {
        DrawTextCentered(TEAM_MEMBERS[i], cx, y, 116.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        y -= line_spacing;
    }
    y -= sec_spacing;

    // Faculty & Advisors
    DrawTextCentered("Faculty & Advisors", cx, y, 128.0f, 1.0f, 0.85f, 0.3f, 1.0f);
    y -= sec_spacing * 0.8f;
    for (int i = 0; i < FACULTY_COUNT; ++i) {
        DrawTextCentered(FACULTY[i], cx, y, 116.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        y -= line_spacing;
    }
    y -= sec_spacing;

    // Created at DigiPen
    DrawTextCentered("Created at", cx, y, 128.0f, 1.0f, 0.85f, 0.3f, 1.0f);
    y -= sec_spacing * 0.8f;
    DrawTextCentered("DigiPen Institute of Technology Singapore",
        cx, y, 116.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    y -= sec_spacing;

    // President
    DrawTextCentered("President", cx, y, 116.0f, 1.0f, 0.94f, 0.71f, 1.0f);
    y -= line_spacing;
    DrawTextCentered("CLAUDE COMAIR", cx, y, 112.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    y -= sec_spacing * 0.8f;

    // Executives
    DrawTextCentered("Executives", cx, y, 116.0f, 1.0f, 0.94f, 0.71f, 1.0f);
    y -= line_spacing;

    const char* execs[] = {
        "JASON CHU", "SAMIR ABOU SAMRA", "MICHELE COMAIR",
        "ANGELA KUGLER", "ERIK MOHRMANN", "BENJAMIN ELLINGER", "MELVIN GONSALVEZ"
    };
    for (int i = 0; i < 7; ++i) {
        DrawTextCentered(execs[i], cx, y, 110.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        y -= (i < 6) ? line_spacing : sec_spacing;
    }

    DrawTextCentered("www.digipen.edu", cx, y, 106.0f, 0.7f, 0.7f, 0.7f, 1.0f);
    y -= line_spacing;

    char copyright_line[160];
    snprintf(copyright_line, sizeof copyright_line,
        "All content (C) %d DigiPen Institute of Technology Singapore. All Rights Reserved",
        CREDITS_YEAR);
    DrawTextCentered(copyright_line, cx, y, 100.0f, 0.7f, 0.7f, 0.7f, 1.0f);
    y -= sec_spacing * 1.4f;

    // Additional Credits
    DrawTextCentered("Additional Credits", cx, y, 128.0f, 1.0f, 0.85f, 0.3f, 1.0f);
    y -= sec_spacing * 0.8f;
    DrawTextCentered("Sprite Assets: Kenney.nl & itch.io creators",
        cx, y, 116.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    y -= line_spacing;
    DrawTextCentered("Game Engine: Alpha Engine", cx, y, 116.0f, 1.0f, 1.0f, 1.0f, 1.0f);
}

// ============================================================
//  class Credits
// ============================================================

namespace game {

    Credits::Credits(): scroll_offset(0.0f), scroll_speed(120.0f), credits_total_height(3200.0f){}

    // Returns 0 = stay, 2 = back to main menu
    int Credits::update(float dt)
    {
        scroll_offset += scroll_speed * dt;

        if (AEInputCheckTriggered(AEVK_ESCAPE))
        {
            scroll_offset = 0.0f;
            return 2;
        }

        // Auto return when scrolling finishes
        if (scroll_offset >= credits_total_height + SCREEN_H + 100.0f)
        {
            scroll_offset = 0.0f;
            return 2;
        }

        return 0;
    }

    void Credits::draw() const
    {
        // Reset camera so credits always draw at screen center
        AEGfxSetCamPosition(0.0f, 0.0f);

        gfx::drawRectangle({ 0.0f, 0.0f }, 0.0f,
            { SCREEN_W, SCREEN_H }, 0xFF000000);

        float baseY = -(HALF_H + 100.0f) + scroll_offset;
        DrawRollingCredits(baseY);
    }

} // namespace game