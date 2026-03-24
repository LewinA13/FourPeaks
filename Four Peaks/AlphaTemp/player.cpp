#include "player.hpp"
#include "graphics.hpp"
#include "collision.hpp"
#include <fstream>
#include <iomanip>
#include <string>
#include "camera.hpp"
#include "gamestate.hpp"
#include "audio.hpp"
#include "dialogue.hpp"

#include <iostream>


// =========================================================
// HELPER FUNCTIONS (RESPAWN DIE)
// =========================================================

gfx::Vec2 PlayerGetFeetWorld(const Player& p)
{
    // feet point directly under collider center
    return { p.pos.x, p.pos.y - (p.colliderSize.y * 0.5f) };
}

void PlayerSetFeetWorld(Player& p, gfx::Vec2 feetWorld)
{
    // place collider so its bottom sits on feetWorld
    p.pos.x = feetWorld.x;
    p.pos.y = feetWorld.y + (p.colliderSize.y * 0.5f);
}

void PlayerSetRespawn(Player& p, gfx::Vec2 pos) // PlayerSetRespawn(gGame.p, gfx::Vec2 pos)
{
    p.respawnPos = pos;
}

bool PlayerSaveCheckpoint(const Player& p, const char* filename)
{
    if (filename == nullptr || filename[0] == '\0')
        return false;

    // trunc is used to remove old checkpoint and add in new one
    std::ofstream out(filename, std::ios::out | std::ios::trunc);
    if (!out.is_open())
        return false;

    out << "checkpoint_v7\n";
    out << p.checkpointScene << "\n";
    out << std::fixed << std::setprecision(3);

    // Save respawn position, melon count, death count, and run timer.
    out << p.respawnPos.x << " "
        << p.respawnPos.y << " "
        << p.melonsCollected << " "
        << p.deathCount << " "
        << gGame.runTimeSeconds << "\n";

    // Save all 16 seasonal unlock flags as 0/1 values.
    for (int i = 0; i < 16; ++i)
    {
        out << (gGame.unlockedStages[i] ? 1 : 0);
        if (i < 15) out << " ";
    }
    out << "\n";

    out << gGame.collectedMelons.size() << "\n";

    for (const CollectedMelon& melon : gGame.collectedMelons)
    {
        out << melon.scene << " "
            << melon.row << " "
            << melon.col << "\n";
    }

    std::cout << "Saved checkpoint to file";
    return out.good();
}

bool PlayerLoadCheckpoint(Player& p, const char* filename, bool teleportToRespawn)
{
    if (filename == nullptr || filename[0] == '\0')
        return false;

    std::ifstream in(filename);
    if (!in.is_open())
        return false;

    std::string header;
    in >> header;

    float x = 0.0f;
    float y = 0.0f;

    // Safe defaults if file is old or missing values
    p.melonsCollected = 0;
    p.deathCount = 0;
    gGame.runTimeSeconds = 0.0f;
    gGame.collectedMelons.clear();

    for (int i = 0; i < 16; ++i)
        gGame.unlockedStages[i] = false;

    std::string scene;
    if (!(in >> scene))
        return false;

    p.checkpointScene = scene;

    // Newest format: checkpoint_v7
    if (header == "checkpoint_v7")
    {
        if (!(in >> x >> y >> p.melonsCollected >> p.deathCount >> gGame.runTimeSeconds))
            return false;

        for (int i = 0; i < 16; ++i)
        {
            int unlocked = 0;
            if (!(in >> unlocked))
                return false;

            gGame.unlockedStages[i] = (unlocked != 0);
        }

        int collectedCount = 0;
        if (!(in >> collectedCount))
            return false;

        for (int i = 0; i < collectedCount; ++i)
        {
            CollectedMelon melon;
            if (!(in >> melon.scene >> melon.row >> melon.col))
                return false;

            gGame.collectedMelons.push_back(melon);
        }
    }
    else if (header == "checkpoint_v6")
    {
        if (!(in >> x >> y >> p.melonsCollected >> p.deathCount >> gGame.runTimeSeconds))
            return false;

        int collectedCount = 0;
        if (!(in >> collectedCount))
            return false;

        for (int i = 0; i < collectedCount; ++i)
        {
            CollectedMelon melon;
            if (!(in >> melon.scene >> melon.row >> melon.col))
                return false;

            gGame.collectedMelons.push_back(melon);
        }
    }
    else
    {
        return false;
    }
}

bool IsMelonCollected(const char* sceneName, int row, int col)
{
    if (!sceneName) return false;

    for (const CollectedMelon& melon : gGame.collectedMelons)
    {
        if (melon.scene == sceneName && melon.row == row && melon.col == col)
            return true;
    }

    return false;
}

bool MarkMelonCollected(const char* sceneName, int row, int col)
{
    if (!sceneName) return false;

    if (IsMelonCollected(sceneName, row, col))
        return false;

    CollectedMelon melon;
    melon.scene = sceneName;
    melon.row = row;
    melon.col = col;

    gGame.collectedMelons.push_back(melon);
    return true;
}

void ApplyCollectedMelonsToTileMap(const char* sceneName, int rows, int tileMap[][32])
{
    if (!sceneName || !tileMap) return;

    for (const CollectedMelon& melon : gGame.collectedMelons)
    {
        if (melon.scene != sceneName)
            continue;

        if (melon.row < 0 || melon.row >= rows)
            continue;

        if (melon.col < 0 || melon.col >= 32)
            continue;

        if (tileMap[melon.row][melon.col] == 8)
            tileMap[melon.row][melon.col] = 0;
    }
}

void PlayerRespawn(Player& p)
{
    // Restore core life state
    p.hp = p.maxHp;
    p.alive = true;
    p.dead = false;
    p.deadTimer = 0.0f;
    p.heat = p.maxHeat;


    p.currGroundType = Player::GroundType::Normal;

    // Teleport to respawn point
    PlayerSetFeetWorld(p, p.respawnPos);
    p.justRespawned = true;

    p.grounded = true;   // prevents brief fall animation frame
    p.velY = 0.0f;

    // Reset motion
    p.velX = 0.0f;
    p.velY = 0.0f;
    p.horzSpeed = 0.0f;

    // Reset common states that are initialized in player_init
    p.grounded = false;
    p.wallHanging = false;
    p.onWallLeft = false;
    p.onWallRight = false;

    p.dashing = false;
    p.dashTimer = 0.0f;
    p.dashCount = p.maxDashCount;

    // reset anim frames so it looks consistent
    p.idleFrame = p.runFrame = p.jumpFrame = p.fallFrame = p.wallSlideFrame = p.dashFrame = 0;
    p.idleAnimTimer = p.runAnimTimer = p.jumpAnimTimer = p.fallAnimTimer = p.wallSlideAnimTimer = p.dashAnimTimer = 0.0f;

    // Start respawn animation: play death backwards from last frame to 0
    p.respawning = true;
    p.respawnTimer = p.respawnDuration;

    p.respawnFrame = p.deathFrameCount - 1;   // start from last death frame
    p.respawnAnimTimer = 0.0f;

    audio::play_sfx(SfxType::Respawn);
}

void PlayerKill(Player& p) // PlayerKill(gGame.player)
{
    if (p.dead) return;
    if (gGame.cheatsOn) return;

    p.hp = 0;
    p.alive = false;
    p.dead = true;

    // increase death count
    p.deathCount += 1;

    // save updated death count, only save if we have valid checkpoint
    if (!p.checkpointScene.empty())
    {
        PlayerSaveCheckpoint(p, "checkpoint.txt");
    }

    // Start death delay timer
    p.deadTimer = p.deadDuration;

    // reset death animation playback
    p.deathFrame = 0;
    p.deathAnimTimer = 0.0f;

    // Freeze motion immediately
    p.velX = 0.0f;
    p.velY = 0.0f;
    p.horzSpeed = 0.0f;

    // Cancel special states
    p.dashing = false;
    p.wallHanging = false;

    audio::play_sfx(SfxType::Death);
}

void PlayerDamage(Player& p, int dmg)
{
    if (p.dead) return;
    if (gGame.cheatsOn) return;
    if (dmg <= 0) return;

    p.hp -= dmg;
    if (p.hp <= 0)
    {
        PlayerKill(p);
    }
}


void PlayerInit(Player& p)
{
    p.pos = { 100.0f, 100.0f };
    p.size = { 100.0f, 90.0f };
    p.speed = 120.0f;

    p.velY = 0.0f;
    p.grounded = false;

    p.gravity = -2000.0f;
    p.terminalVel = -1200.0f;

    p.jumpVel = 900.0f;

    p.coyoteTime = 0.08f;  // tweak
    p.coyoteTimer = 0.0f;

    p.jumpCutMult = 2.5f; // tweak

    // player kill/respawn
    p.maxHp = 1;
    p.hp = p.maxHp;

    p.alive = true;
    p.dead = false;

    p.melonsCollected = 0;
    p.deathCount = 0;
    gGame.runTimeSeconds = 0.0f;

    PlayerLoadCheckpoint(p, "checkpoint.txt", true);
    p.justRespawned = false;


    p.deadDuration = 0.50f;    // tweak (lower val = faster respawn)
    p.deadTimer = 0.0f;

    // death animation
    p.deathTex = AEGfxTextureLoad("Assets/player/male_hero-death.png");

    p.deathFrame = 0;
    p.deathFrameCount = 23;

    p.deathAnimTimer = 0.0f;
    p.deathFrameTime = 0.04f;     // tweak

    p.deadDuration = p.deathFrameCount * p.deathFrameTime;

    // respawn animation
    p.respawning = false;
    p.respawnDuration = p.deathFrameCount * p.deathFrameTime; // matches death length
    p.respawnTimer = 0.0f;

    p.respawnFrame = 0;
    p.respawnAnimTimer = 0.0f;
    p.respawnFrameTime = p.deathFrameTime; // keep consistent

    // sprite initialisation
    p.idleTex = AEGfxTextureLoad("Assets/player/male_hero-idle.png");

    p.idleFrame = 0;
    p.idleFrameCount = 10;

    p.idleAnimTimer = 0.0f;
    p.idleFrameTime = 0.10f; // 10 FPS idle animation

    // running sprite initialisation
    p.runTex = AEGfxTextureLoad("Assets/player/male_hero-run.png");

    p.runFrame = 0;
    p.runFrameCount = 10;     // run sheet has 10 frames
    p.runAnimTimer = 0.0f;
    p.runFrameTime = 0.07f;   // faster than idle

    p.facing = 1;

    // jumping sprite initialisation
    p.jumpTex = AEGfxTextureLoad("Assets/player/male_hero-jump.png");

    p.jumpFrame = 0;
    p.jumpFrameCount = 6;      // 6 frames
    p.jumpAnimTimer = 0.0f;
    p.jumpFrameTime = 0.06f;   // can change

    // wall jump
    p.wallRegrabTimer = 0.0f;
    p.wallRegrabTime = 0.55f;
    p.wallHangBufferTime = 0.15f; // 
    p.wallHangBufferTimer = 0.0f;
    p.wallClimbSpeed = 260.0f;      // tweak
    p.wallClimbDownSpeed = 180.0f;  // tweak

    // wall slide sprite initialisation
    p.wallSlideTex = AEGfxTextureLoad("Assets/player/male_hero-wall_slide.png");

    p.wallSlideFrame = 0;
    p.wallSlideFrameCount = 4;
    p.wallSlideAnimTimer = 0.0f;
    p.wallSlideFrameTime = 0.06f;



    // falling sprite initialisation
    p.fallTex = AEGfxTextureLoad("Assets/player/male_hero-fall_loop.png");

    p.fallFrame = 0;
    p.fallFrameCount = 3;      
    p.fallAnimTimer = 0.0f;
    p.fallFrameTime = 0.08f;   // can change

    p.horzSpeed = 0.0f;


    // collider box
    p.colliderSize = { 35.0f, 45.0f };  // literally size of collider box
    //p.spriteSize = { 300.0f, 300.0f };  // player is square sprite
    p.spriteOffsetX = 0.0f;
    p.wallHangOffsetX = 10.0f;   // tweak until it "sticks" nicely

    p.spriteSize = { 140.0f, 140.0f };  // player is square sprite
    p.spriteOffsetY = -50.0f;

    // wall detection
    p.onWallLeft = false;   //player start not on the wall
    p.onWallRight = false;

    // wall movement
    p.wallHanging = false;
    p.wallSlideSpeed = 20.0f;       // 
    p.wallHangGravityScale = 0.02f;  //
    p.wallHangRequested = false;

    // dashing
    p.dashing = false;
    p.dashTimer = 0.0f;
    p.dashDuration = 0.15f;    // 0.12–0.18 feels good

    p.dashSpeed = 950.0f;   // tweak later
    p.dashDir = 1;

    p.maxDashCount = 1;
    p.dashCount = p.maxDashCount;

    // dash animation
    p.dashTex = AEGfxTextureLoad("Assets/player/male_hero-dash.png");

    p.dashFrame = 0;
    p.dashFrameCount = 5;
    p.dashAnimTimer = 0.0f;
    p.dashFrameTime = 0.04f;   // tweak feel later

    // dash trail 
    for (int i = 0; i < Player::TRAIL_MAX; i++)
        p.trail[i] = { {0,0}, 0.0f };

    //heat bar
    p.maxHeat = 1.0f;
    p.heat = p.maxHeat;

}



void PlayerUpdate(Player& p, float dt)
{
    // =========================================================
    // 0) SETUP
    // =========================================================
    const bool wasGrounded = p.grounded;
    const bool jumpPressed = AEInputCheckTriggered(AEVK_SPACE);
    const bool jumpHeld = AEInputCheckCurr(AEVK_SPACE);

    const bool grabHeld = AEInputCheckCurr(AEVK_L);
    const bool dashPressed = AEInputCheckTriggered(AEVK_LSHIFT);

    const bool pressUp = AEInputCheckCurr(AEVK_W);
    const bool pressDown = AEInputCheckCurr(AEVK_S);

    bool didWallJumpThisFrame = false;

    static constexpr float SNOW_STEP_INTERVAL = 0.10f; // tweak: 0.22-0.35 feels normal
    static constexpr float WALK_SPEED_EPS = 5.0f; // tweak based on your velocity scale

    // Safety to avoid giant dt spikes exploding physics
    if (dt > 0.05f) dt = 0.05f;

    // =========================================================
    // 1) Dead state (if dead, the rest dont matter)
    // =========================================================
    if (p.dead)
    {
        // advance death animation
        p.deathAnimTimer += dt;
        while (p.deathAnimTimer >= p.deathFrameTime)
        {
            p.deathAnimTimer -= p.deathFrameTime;
            if (p.deathFrame < p.deathFrameCount - 1)
                p.deathFrame++;
        }

        // countdown to respawn
        p.deadTimer -= dt;
        if (p.deadTimer <= 0.0f)
        {
            PlayerRespawn(p);
        }

        return; // do not run movement/physics while dead
    }

    if (p.respawning)
    {
        // advance respawn animation backwards
        p.respawnAnimTimer += dt;
        while (p.respawnAnimTimer >= p.respawnFrameTime)
        {
            p.respawnAnimTimer -= p.respawnFrameTime;
            if (p.respawnFrame > 0)
                p.respawnFrame--;
        }

        // lock movement (but let collision settle)
        p.velX = 0.0f;
        p.velY = 0.0f;
        p.horzSpeed = 0.0f;

        CollisionUpdate(p, dt);
        CollisionUpdateWallFlags(p);

        // end respawn once we reach frame 0 (or timer ends)
        p.respawnTimer -= dt;
        if (p.respawnTimer <= 0.0f || p.respawnFrame == 0)
        {
            p.respawning = false;
            p.respawnTimer = 0.0f;
        }

        return;
    }

    // lock player if player exactly on ground and reading dialog box
    if (OnGroundExactly(p) && p.grounded && UI::gDialog.dialogBoxShowing()) {
        p.velX = 0.0f;
        p.horzSpeed = 0.0f;
        p.dashing = false;
        p.wallHanging = false;

        CollisionUpdate(p, dt);
        CollisionUpdateWallFlags(p);
        
        return;
    }

   

    // =========================================================
    // 1) INPUT (read once)
    // =========================================================
    f32 moveX = 0.0f;
    if (AEInputCheckCurr(AEVK_A)) { moveX -= 1.0f; }
    if (AEInputCheckCurr(AEVK_D)) { moveX += 1.0f; }

    // dont update facing direction when wall hanging
    if (!p.wallHanging && moveX != 0.0f)
    {
        p.facing = (moveX > 0.0f) ? 1 : -1;
    }

    


    // =========================================================
    // 2) TIMERS / BUFFERS / DASH
    // =========================================================

    // Wall regrab prevention
    if (p.wallRegrabTimer > 0.0f)
    {
        p.wallRegrabTimer -= dt;
        if (p.wallRegrabTimer < 0.0f) p.wallRegrabTimer = 0.0f;
    }

    // Coyote time + dash 
    if (wasGrounded) // checks if you are on the ground LAST FRAME
    {
        p.coyoteTimer = p.coyoteTime;   // when on ground, constantly refresh coyote jump timer
        p.dashCount = p.maxDashCount;   // refresh dash when on groud
    }
    else
    {
        p.coyoteTimer -= dt;
        if (p.coyoteTimer < 0.0f) p.coyoteTimer = 0.0f;
    }
    const bool canCoyoteJump = (p.coyoteTimer > 0.0f);

    // =========================================================
    // 3) DASH START (state only)
    // =========================================================
    if (!p.dashing && dashPressed && p.dashCount > 0) // check conditions for dash
    {
        // initialize dash
        p.dashFrame = 0;
        p.dashAnimTimer = 0.0f;

        p.dashing = true;
        p.dashTimer = p.dashDuration;
        p.dashCount--;

        // dash direction
        if (AEInputCheckCurr(AEVK_A))      p.dashDir = -1;
        else if (AEInputCheckCurr(AEVK_D)) p.dashDir = +1;
        else                              p.dashDir = p.facing;

        // dash breaks wall hang
        p.wallHanging = false;
        
        //dash sound
        audio::play_sfx(SfxType::Dash);
    }

    // =========================================================
    // 4) HORIZONTAL MOVE (only if not dashing)
    //    - This block owns p.horzSpeed which is used to get p.velX
    // =========================================================
    if (!p.dashing)
    {
        const f32 maxHorzSpeed = 2.0f;
        const f32 minHorzSpeed = -2.0f;

        if (moveX != 0.0f)
        {
            p.horzSpeed += moveX * p.accel * dt;
        }
        else
        {
            if (p.horzSpeed > 0.0f)
            {
                p.horzSpeed -= p.decel * dt;
                //! prevent overshooting
                if (p.horzSpeed < 0.0f) p.horzSpeed = 0.0f;
            }
            else if (p.horzSpeed < 0.0f)
            {
                p.horzSpeed += p.decel * dt;
                //! prevent overshooting
                if (p.horzSpeed > 0.0f) p.horzSpeed = 0.0f;
            }
        }

        if (p.horzSpeed > maxHorzSpeed) p.horzSpeed = maxHorzSpeed;
        if (p.horzSpeed < minHorzSpeed) p.horzSpeed = minHorzSpeed;
    }

    // Convert to velocity used for integration
    p.velX = p.horzSpeed * p.speed;

    if (gGame.cheatsOn)
    {
        // Fly controls (replace jump in no-gravity mode)
        const float flySpeed = 450.0f;

        if (AEInputCheckCurr(AEVK_W)
            || AEInputCheckCurr(AEVK_SPACE))
            p.velY = flySpeed;
        else if (AEInputCheckCurr(AEVK_S))
            p.velY = -flySpeed;
        else
            p.velY = 0.0f;

        // Optional: keep dash refilled
        p.dashCount = p.maxDashCount;
    }

    // =========================================================
    // 5) JUMP (ground / coyote)
    // coyote jump is allowing jump for a mini micro time even
    // after player has left the platform. improves playability.
    // =========================================================
    if (!p.dashing)
    {
        if (jumpPressed && (p.grounded || canCoyoteJump))
        {
            p.velY = p.jumpVel;
            p.grounded = false;
            p.coyoteTimer = 0.0f;

            // reset jump anim
            p.jumpFrame = 0;
            p.jumpAnimTimer = 0.0f;
            audio::play_sfx(SfxType::Jump);
        }
    }

    // =========================================================
    // 6) GRAVITY (vertical physics)
    //    Note: wall hanging/climb will override velY
    // =========================================================
    if (!p.dashing && !gGame.cheatsOn)
    {
        float gravityScale = 1.0f;

        // variable jump height rising but released jump
        if (p.velY > 0.0f && !jumpHeld)
        {
            // Add extra downward pull
            p.velY += p.gravity * (p.jumpCutMult - 1.0f) * dt;
        }

        // Apply gravity when not grounded
        if (!p.grounded)
        {
            p.velY += p.gravity * gravityScale * dt;

            if (p.velY < p.terminalVel)
                p.velY = p.terminalVel;
        }
        else
        {
            if (p.velY < 0.0f) p.velY = 0.0f;
        }
    }

    // =========================================================
    // 7) DASH UPDATE (override everything)
    // =========================================================
    if (p.dashing)
    {

        gfx::Vec2 startPos = p.pos;

   

        p.dashTimer -= dt;

        p.velX = p.dashDir * p.dashSpeed;
        p.velY = 0.0f;

        // dash trail
        // Spawn trail ghost at current position 
        for (int i = Player::TRAIL_MAX - 1; i > 0; i--)
            p.trail[i] = p.trail[i - 1];
        p.trail[0] = { p.pos, 0.8f };

        if (p.dashTimer <= 0.0f)
            p.dashing = false;

        gfx::Vec2 endPos;
        endPos.x = p.pos.x + (p.velX * dt);
        endPos.y = p.pos.y + (p.velY * dt);

        CheckPathForCheckpoint(p, startPos, endPos);

    }

    // outside p.dashing to ensure even after dash end, they are fading
    const float TRAIL_FADE = 6.0f;  // how fast they fade — tweak this
    for (int i = 0; i < Player::TRAIL_MAX; i++)
    {
        p.trail[i].alpha -= TRAIL_FADE * dt;
        if (p.trail[i].alpha < 0.0f) p.trail[i].alpha = 0.0f;
    }

    // =========================================================
    // 8) GROUND PROBE (prevents grounded flickering)
    //   If we were grounded last frame and we're not doing a special move,
    //   we push slightly downward so collision can "re-ground" reliably.
    // =========================================================
    const float GROUND_PROBE_SPEED = 60.0f;
    const bool doingSpecialVertical = p.dashing || p.wallHanging || jumpPressed;

    if (!gGame.cheatsOn && wasGrounded && !doingSpecialVertical)
    {
        if (p.velY <= 0.0f)
            p.velY = -GROUND_PROBE_SPEED;
    }

    // =========================================================
    // 9) INTEGRATE POSITION
    // =========================================================
    p.pos.x += p.velX * dt;
    p.pos.y += p.velY * dt;

    // =========================================================
    // 10) COLLISION + WALL FLAGS
    //    IMPORTANT: grounded must be recomputed each frame by collision
    // =========================================================
    p.grounded = false; // reset it and assume player not grounded. use collision to ensure everything is correct
    // DEBUG: press K to kill player and test respawn
    if (AEInputCheckTriggered(AEVK_H))
    {
        gfx::Vec2 feet = PlayerGetFeetWorld(p);
        feet.y += 1.0f;     // +Y is up in your project
        PlayerSetRespawn(p, feet);
        PlayerSaveCheckpoint(p, "checkpoint.txt");

        // If grounded, lift the saved respawn a tiny bit so we don't respawn intersecting tiles.
        if (p.grounded)
        {
            // Choose direction depending on your coordinate system:
            // If +Y is UP in your game, use feet.y += 1.0f;
            // If +Y is DOWN (screen space), use feet.y -= 1.0f;
            feet.y += 1.0f; // <-- adjust sign if needed
        }

        PlayerSetRespawn(p, feet);
    }
    if (AEInputCheckTriggered(AEVK_K))
    {
        PlayerKill(p);
    }

    if (!gGame.noClip)
    {
        CollisionUpdate(p, dt);
        CollisionUpdateWallFlags(p);
    }
    else
    {
        p.grounded = false;
    }

    const bool touchingWall = (p.onWallLeft || p.onWallRight);

    // =========================================================
    // 11) WALL HANG ENTER (after wall flags are correct)
    // =========================================================
    if (!p.dashing)
    {
        const bool canStick = (!p.grounded && touchingWall && p.wallRegrabTimer <= 0.0f);

        if (!p.wallHanging && canStick && (grabHeld || p.wallHangBufferTimer > 0.0f))
        {
            p.wallHanging = true;
            p.wallHangBufferTimer = 0.0f; // consume buffer

            if (p.onWallLeft && !p.onWallRight)
                p.facing = 1;   // wall left only -> face right
            else if (p.onWallRight && !p.onWallLeft)
                p.facing = -1;  // wall right only -> face left
        }
    }

    // =========================================================
    // 12) WALL MOVEMENT (climb / constant slide)
    //    - While hanging, this block OWNS velY.
    // =========================================================
    if (!p.dashing && p.wallHanging && !didWallJumpThisFrame)
    {
        // Exit conditions
        if (!grabHeld || p.grounded || !touchingWall)
        {
            p.wallHanging = false;
        }
        else
        {
            if (pressUp && !pressDown)
            {
                p.velY = +p.wallClimbSpeed;
            }
            else if (pressDown && !pressUp)
            {
                p.velY = -p.wallClimbDownSpeed;
            }
            else
            {
                // Constant slide speed while hanging
                p.velY = -p.wallSlideSpeed;
            }
        }
    }

    // =========================================================
    // 13) WALL JUMP (works even if not hanging)
    //    - Must run AFTER collision so onWall flags are accurate.
    //    - Must run AFTER wall movement so jump doesn't get overwritten.
    // =========================================================
    if (!p.dashing && jumpPressed)
    {
        const bool canWallJump = (!p.grounded && touchingWall && p.wallRegrabTimer <= 0.0f);

        if (canWallJump)
        {
            float jumpDir = 0.0f;
            if (p.onWallLeft)       jumpDir = +1.0f;  // wall on left -> jump right
            else if (p.onWallRight) jumpDir = -1.0f;  // wall on right -> jump left
            else                    jumpDir = (float)p.facing;

            p.velY = p.jumpVel;
            didWallJumpThisFrame = true;

            // Horizontal push (your existing tuning)
            p.horzSpeed = jumpDir * 2.0f;
            p.facing = (jumpDir > 0.0f) ? 1 : -1;
            p.velX = p.horzSpeed * p.speed;

            p.wallRegrabTimer = p.wallRegrabTime;
            p.wallHanging = false;

            p.grounded = false;
            p.coyoteTimer = 0.0f;

            // reset jump anim
            p.jumpFrame = 0;
            p.jumpAnimTimer = 0.0f;

            audio::play_sfx(SfxType::Jump);
        }
    }

    // =========================================================
    // 14) ANIMATION (based on final state)
    // =========================================================
    const bool isMoving = (AEInputCheckCurr(AEVK_A) || AEInputCheckCurr(AEVK_D));


    // Wall slide anim (only while wall hanging)
    if (p.wallHanging)
    {
        p.wallSlideAnimTimer += dt;
        while (p.wallSlideAnimTimer >= p.wallSlideFrameTime)
        {
            p.wallSlideAnimTimer -= p.wallSlideFrameTime;
            p.wallSlideFrame = (p.wallSlideFrame + 1) % p.wallSlideFrameCount;
        }
    }
    else
    {
        p.wallSlideFrame = 0;
        p.wallSlideAnimTimer = 0.0f;
    }

    // Idle
    if (p.grounded && !isMoving)
    {
        p.idleAnimTimer += dt;
        while (p.idleAnimTimer >= p.idleFrameTime)
        {
            p.idleAnimTimer -= p.idleFrameTime;
            p.idleFrame = (p.idleFrame + 1) % p.idleFrameCount;
        }
    }
    else
    {
        p.idleFrame = 0;
        p.idleAnimTimer = 0.0f;
    }

    // Run
    if (p.grounded && isMoving)
    {
        p.runAnimTimer += dt;
        while (p.runAnimTimer >= p.runFrameTime)
        {
            p.runAnimTimer -= p.runFrameTime;
            p.runFrame = (p.runFrame + 1) % p.runFrameCount;
        }
    }
    else
    {
        p.runFrame = 0;
        p.runAnimTimer = 0.0f;
    }

    // Air (jump/fall)
    if (!p.grounded)
    {
        if (p.velY > 0.0f)
        {
            p.jumpAnimTimer += dt;
            while (p.jumpAnimTimer >= p.jumpFrameTime)
            {
                p.jumpAnimTimer -= p.jumpFrameTime;
                if (p.jumpFrame < p.jumpFrameCount - 1)
                    p.jumpFrame += 1;
            }
        }
        else
        {
            p.fallAnimTimer += dt;
            while (p.fallAnimTimer >= p.fallFrameTime)
            {
                p.fallAnimTimer -= p.fallFrameTime;
                p.fallFrame = (p.fallFrame + 1) % p.fallFrameCount;
            }
        }
    }
    else
    {
        p.fallFrame = 0;
        p.fallAnimTimer = 0.0f;
    }

    // dashing

    if (p.dashing)
    {
        p.dashAnimTimer += dt;
        while (p.dashAnimTimer >= p.dashFrameTime)
        {
            p.dashAnimTimer -= p.dashFrameTime;

            // Usually dash anim should not loop; it should clamp at last frame.
            if (p.dashFrame < p.dashFrameCount - 1)
                p.dashFrame++;
        }
    }
    else
    {
        p.dashFrame = 0;
        p.dashAnimTimer = 0.0f;
    }

    // =========================================================
    // 15) AUDIO BASED ON FINAL STATE
    // =========================================================

    // Conditions: on ground + moving horizontally + not dashing (optional)
    const float vxAbs = fabsf(p.velX);

    const bool isWalking =
        p.grounded &&
        (moveX != 0.0f) &&
        (vxAbs > WALK_SPEED_EPS) &&
        !p.dashing;

    if (isWalking)
    {
        p.winterStepTimer += dt;

        if (p.winterStepTimer >= SNOW_STEP_INTERVAL)
        {
            // Play one footstep
            audio::play_sfx(SfxType::WinterStep);

            // Keep leftover time so cadence stays stable even if dt fluctuates
            p.winterStepTimer -= SNOW_STEP_INTERVAL;
        }
    }
    else
    {
        // Reset so the first step happens quickly after you start walking again
        p.winterStepTimer = 0.0f;
    }
}




void PlayerDraw(Player& p)
{

    if (p.dead)
    {
        AEGfxTexture* tex = p.deathTex;
        int frame = p.deathFrame;
        int frameCount = p.deathFrameCount;

        // Compute UV slice for the chosen frame
        f32 frameW = 1.0f / (f32)frameCount;
        f32 u0 = frame * frameW;
        f32 u1 = u0 + frameW;

        // If you DON'T want flipping during death, comment this out.
        if (p.facing < 0)
        {
            f32 tmp = u0;
            u0 = u1;
            u1 = tmp;
        }

        // same drawPos logic as normal draw
        gfx::Vec2 feetWorld = { p.pos.x, p.pos.y - (p.colliderSize.y * 0.5f) };

        gfx::Vec2 drawPos;
        drawPos.y = feetWorld.y + (p.spriteSize.y * 0.5f) + p.spriteOffsetY;
        drawPos.x = feetWorld.x + p.spriteOffsetX;

        // No wall hang offset during death (prevents weird shifts)
        gfx::drawSprite(tex, drawPos, 0.0f, p.spriteSize, u0, 0.0f, u1, 1.0f);
        return;
    }

    if (p.respawning)
    {
        AEGfxTexture* tex = p.deathTex;       // reuse death texture
        int frame = p.respawnFrame;
        int frameCount = p.deathFrameCount;

        f32 frameW = 1.0f / (f32)frameCount;
        f32 u0 = frame * frameW;
        f32 u1 = u0 + frameW;

        // optional: flip based on facing
        if (p.facing < 0)
        {
            f32 tmp = u0;
            u0 = u1;
            u1 = tmp;
        }

        gfx::Vec2 feetWorld = { p.pos.x, p.pos.y - (p.colliderSize.y * 0.5f) };

        gfx::Vec2 drawPos;
        drawPos.y = feetWorld.y + (p.spriteSize.y * 0.5f) + p.spriteOffsetY;
        drawPos.x = feetWorld.x + p.spriteOffsetX;

        gfx::drawSprite(tex, drawPos, 0.0f, p.spriteSize, u0, 0.0f, u1, 1.0f);
        return;
    }

    bool isMoving = ((AEInputCheckCurr(AEVK_A) || AEInputCheckCurr(AEVK_D))) && !UI::gDialog.dialogBoxShowing();

    AEGfxTexture* tex = nullptr;
    int frame = 0;
    int frameCount = 1;

    // Priority: air states first

    if (p.dashing)
    {
        tex = p.dashTex;
        frame = p.dashFrame;
        frameCount = p.dashFrameCount;
    }else if (p.wallHanging)
    {
        tex = p.wallSlideTex;
        frame = p.wallSlideFrame;
        frameCount = p.wallSlideFrameCount;
    }
    else if (!p.grounded)
    {
        if (p.velY > 0.0f)
        {
            tex = p.jumpTex;
            frame = p.jumpFrame;
            frameCount = p.jumpFrameCount;
        }
        else
        {
            tex = p.fallTex;
            frame = p.fallFrame;
            frameCount = p.fallFrameCount;
        }
    }
    else
    {
        // Ground states
        if (isMoving)
        {
            tex = p.runTex;
            frame = p.runFrame;
            frameCount = p.runFrameCount;
        }
        else
        {
            tex = p.idleTex;
            frame = p.idleFrame;
            frameCount = p.idleFrameCount;
        }
    }

    // Compute UV slice for the chosen frame
    f32 frameW = 1.0f / (f32)frameCount;
    f32 u0 = frame * frameW;
    f32 u1 = u0 + frameW;

    // Flip left/right by swapping u0/u1
    if (p.facing < 0)
    {
        f32 tmp = u0;
        u0 = u1;
        u1 = tmp;
    }

    // new variable for collider box
    gfx::Vec2 feetWorld = { p.pos.x, p.pos.y - (p.colliderSize.y * 0.5f) };



    // sprite center position so the sprite bottom sits on feetWorld
    gfx::Vec2 drawPos;
    drawPos.y = feetWorld.y + (p.spriteSize.y * 0.5f) + p.spriteOffsetY;
    drawPos.x = feetWorld.x + p.spriteOffsetX;   // base X offset (usually 0)

    // extra "stick to wall" offset
    if (p.wallHanging)
    {
        if (p.onWallRight)
            drawPos.x += p.wallHangOffsetX;  // push sprite right into right wall
        else if (p.onWallLeft)
            drawPos.x -= p.wallHangOffsetX;  // push sprite left into left wall
    }


    // Draw dash trail
    for (int i = Player::TRAIL_MAX - 1; i >= 0; i--)
    {
        if (p.trail[i].alpha <= 0.0f) continue;

        float u0 = (float)p.dashFrame / (float)p.dashFrameCount;
        float u1 = u0 + 1.0f / (float)p.dashFrameCount;
        float flipU0 = (p.facing == -1) ? u1 : u0;
        float flipU1 = (p.facing == -1) ? u0 : u1;

        gfx::drawSprite(p.dashTex, p.trail[i].pos, 0.0f, p.spriteSize,
            flipU0, 0.0f, flipU1, 1.0f, p.trail[i].alpha);
    }


    gfx::drawSprite(tex, drawPos, 0.0f, p.spriteSize, u0, 0.0f, u1, 1.0f);
}

void PlayerShutdown(Player& p)
{
    if (p.wallSlideTex)
    {
        AEGfxTextureUnload(p.wallSlideTex);
        p.wallSlideTex = nullptr;
    }
    if (p.idleTex)
    {
        AEGfxTextureUnload(p.idleTex);
        p.idleTex = nullptr;
    }
    if (p.runTex)
    {
        AEGfxTextureUnload(p.runTex);
        p.runTex = nullptr;
    }
    if (p.jumpTex)
    {
        AEGfxTextureUnload(p.jumpTex);
        p.jumpTex = nullptr;
    }
    if (p.fallTex)
    {
        AEGfxTextureUnload(p.fallTex);
        p.fallTex = nullptr;
    }
    if (p.dashTex)
    {
        AEGfxTextureUnload(p.dashTex);
        p.dashTex = nullptr;
    }
    if (p.deathTex)
    {
        AEGfxTextureUnload(p.deathTex);
        p.deathTex = nullptr;
    }
}