#include "scenes/credit.hpp"
#include "engine/graphics.hpp"
#include "AEEngine.h"
#include <stdio.h>

extern s8 gFontId;

// -------------------------------------------------------------------------
// Credit data — edit these arrays to update the credits screen.
// -------------------------------------------------------------------------
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

// -------------------------------------------------------------------------
// Draws a null-terminated string horizontally centred at the given Alpha
// Engine world-space coordinate (ae_cx, ae_cy). size is the font scale
// passed to AEGfxPrint, and r/g/b/a are the RGBA colour components in [0,1].
// -------------------------------------------------------------------------
static void DrawTextCentered(const char* text,
    float ae_cx, float ae_cy,
    float size,
    float r, float g, float b, float a)
{
    float textW = 0.0f, textH = 0.0f;
    AEGfxGetPrintSize(gFontId, text, size, &textW, &textH);

    // -------------------------------------------------------------------------
    // Convert from AE world space to the NDC range expected by AEGfxPrint,
    // then offset left by half the text width to achieve horizontal centring.
    // -------------------------------------------------------------------------
    float ndcX = (ae_cx / (AEGfxGetWindowWidth() / 2.0f)) - textW * 0.5f;
    float ndcY = ae_cy / (AEGfxGetWindowHeight() / 2.0f);

    AEGfxPrint(gFontId, text, ndcX, ndcY, size, r, g, b, a);
}

// -------------------------------------------------------------------------
// Renders the complete rolling credits layout starting at the given baseY
// world coordinate and proceeding downward by decreasing y. Each section
// uses consistent spacing so adding or removing entries keeps the layout uniform.
// -------------------------------------------------------------------------
static void DrawRollingCredits(float baseY)
{
    const float cx = 0.0f;
    const float line_spacing = 80.0f;
    const float sec_spacing = 110.0f;
    const float head_spacing = 88.0f;
    float y = baseY;

    DrawTextCentered(GAME_TITLE, cx, y, 4.81f, 1.0f, 1.0f, 1.0f, 1.0f);
    y -= sec_spacing;

    char team_line[128];
    snprintf(team_line, sizeof team_line, "A Game by %s", TEAM_NAME);
    DrawTextCentered(team_line, cx, y, 2.37f, 1.0f, 1.0f, 1.0f, 1.0f);
    y -= line_spacing;
    y -= 40.0f;
    y -= sec_spacing * 0.5f;

    // -------------------------------------------------------------------------
    // Development team members.
    // -------------------------------------------------------------------------
    DrawTextCentered("Development Team", cx, y, 2.37f, 1.0f, 0.85f, 0.3f, 1.0f);
    y -= head_spacing;
    for (int i = 0; i < TEAM_MEMBER_COUNT; ++i) {
        DrawTextCentered(TEAM_MEMBERS[i], cx, y, 2.15f, 1.0f, 1.0f, 1.0f, 1.0f);
        y -= line_spacing;
    }
    y -= sec_spacing;

    // -------------------------------------------------------------------------
    // Faculty and advisors.
    // -------------------------------------------------------------------------
    DrawTextCentered("Faculty & Advisors", cx, y, 2.37f, 1.0f, 0.85f, 0.3f, 1.0f);
    y -= head_spacing;
    for (int i = 0; i < FACULTY_COUNT; ++i) {
        DrawTextCentered(FACULTY[i], cx, y, 2.15f, 1.0f, 1.0f, 1.0f, 1.0f);
        y -= line_spacing;
    }
    y -= sec_spacing;

    // -------------------------------------------------------------------------
    // DigiPen institution credit.
    // -------------------------------------------------------------------------
    DrawTextCentered("Created at", cx, y, 2.37f, 1.0f, 0.85f, 0.3f, 1.0f);
    y -= head_spacing;
    DrawTextCentered("DigiPen Institute of Technology Singapore",
        cx, y, 2.15f, 1.0f, 1.0f, 1.0f, 1.0f);
    y -= sec_spacing;

    // -------------------------------------------------------------------------
    // DigiPen president.
    // -------------------------------------------------------------------------
    DrawTextCentered("President", cx, y, 2.15f, 1.0f, 0.85f, 0.3f, 1.0f);
    y -= line_spacing;
    DrawTextCentered("CLAUDE COMAIR", cx, y, 2.07f, 1.0f, 1.0f, 1.0f, 1.0f);
    y -= head_spacing;

    // -------------------------------------------------------------------------
    // DigiPen executives.
    // -------------------------------------------------------------------------
    DrawTextCentered("Executives", cx, y, 2.15f, 1.0f, 0.85f, 0.3f, 1.0f);
    y -= line_spacing;

    const char* execs[] = {
        "JASON CHU", "SAMIR ABOU SAMRA", "MICHELE COMAIR",
        "ANGELA KUGLER", "ERIK MOHRMANN", "BENJAMIN ELLINGER", "MELVIN GONSALVEZ"
    };
    for (int i = 0; i < 7; ++i) {
        DrawTextCentered(execs[i], cx, y, 2.04f, 1.0f, 1.0f, 1.0f, 1.0f);
        y -= (i < 6) ? line_spacing : sec_spacing;
    }

    DrawTextCentered("www.digipen.edu", cx, y, 1.96f, 0.7f, 0.7f, 0.7f, 1.0f);
    y -= line_spacing;

    char copyright_line[160];
    snprintf(copyright_line, sizeof copyright_line,
        "All content (C) %d DigiPen Institute of Technology Singapore. All Rights Reserved",
        CREDITS_YEAR);
    DrawTextCentered(copyright_line, cx, y, 1.85f, 0.7f, 0.7f, 0.7f, 1.0f);
    y -= sec_spacing * 1.4f;

    // -------------------------------------------------------------------------
    // Third-party asset and engine credits.
    // -------------------------------------------------------------------------
    DrawTextCentered("Additional Credits", cx, y, 2.37f, 1.0f, 0.85f, 0.3f, 1.0f);
    y -= head_spacing;
    DrawTextCentered("Sprite Assets: Kenney.nl & itch.io creators",
        cx, y, 2.15f, 1.0f, 1.0f, 1.0f, 1.0f);
    y -= line_spacing;
    DrawTextCentered("Game Engine: Alpha Engine", cx, y, 2.15f, 1.0f, 1.0f, 1.0f, 1.0f);
}

namespace game {

    // -------------------------------------------------------------------------
    // Sets the initial scroll offset to zero and configures the scroll speed
    // and an estimate of the total content height.
    // -------------------------------------------------------------------------
    Credits::Credits()
        : scroll_offset(0.0f)
        , scroll_speed(120.0f)
        , credits_total_height(3200.0f)
    {
    }

    // -------------------------------------------------------------------------
    // Advances the scroll by scroll_speed * dt and checks for early exit.
    // Returns 2 (back to main menu) if Escape is pressed or the credits have
    // scrolled far enough to have fully passed off-screen.
    // -------------------------------------------------------------------------
    int Credits::update(float dt)
    {
        scroll_offset += scroll_speed * dt;

        if (AEInputCheckTriggered(AEVK_ESCAPE))
        {
            scroll_offset = 0.0f;
            return 2;
        }

        // -------------------------------------------------------------------------
        // Auto-return once all content has scrolled past the top of the screen.
        // -------------------------------------------------------------------------
        if (scroll_offset >= credits_total_height + AEGfxGetWindowHeight() + 100.0f)
        {
            scroll_offset = 0.0f;
            return 2;
        }

        return 0;
    }

    // -------------------------------------------------------------------------
    // Draws a solid black background then renders the rolling credits text at
    // a Y position derived from the current scroll offset. The camera is reset
    // to (0,0) so the background always fills the screen regardless of where
    // the gameplay camera is positioned.
    // -------------------------------------------------------------------------
    void Credits::draw() const
    {
        AEGfxSetCamPosition(0.0f, 0.0f);

        gfx::drawRectangle({ 0.0f, 0.0f }, 0.0f,
            { static_cast<float>(AEGfxGetWindowWidth()),
              static_cast<float>(AEGfxGetWindowHeight()) },
            0xFF000000);

        // -------------------------------------------------------------------------
        // baseY starts just below the screen and rises as scroll_offset increases.
        // -------------------------------------------------------------------------
        float baseY = -((AEGfxGetWindowHeight() / 2.0f) + 100.0f) + scroll_offset;
        DrawRollingCredits(baseY);
    }

} // namespace game