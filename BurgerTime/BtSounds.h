#pragma once

// Filename constants for all BurgerTime audio.
// Empty string = no file available, plays nothing (safe to leave as-is).
namespace BtSounds
{
    // Background music
    constexpr const char* BGM_LEVEL    = "Data/BT_Sounds/BGM.wav";
    constexpr const char* BGM_GAMEOVER = "";  // no separate game over track

    // Sound effects
    constexpr const char* SFX_WALK           = "Data/BT_Sounds/Burger_Step.wav";
    constexpr const char* SFX_BURGER_DROP    = "Data/BT_Sounds/Burger_Fall.wav";
    constexpr const char* SFX_BURGER_LAND    = "Data/BT_Sounds/Burger_Land.wav";
    constexpr const char* SFX_SQUISH         = "Data/BT_Sounds/Enemy_Squahed.wav";
    constexpr const char* SFX_ENEMY_FALL     = "Data/BT_Sounds/Enemy_Fall.wav";
    constexpr const char* SFX_PLAYER_DEATH   = "Data/BT_Sounds/Death.wav";
    constexpr const char* SFX_PEPPER         = "Data/BT_Sounds/Pepper_Shake.wav";
    constexpr const char* SFX_ENEMY_SPRAYED  = "Data/BT_Sounds/Enemy_Sprayed.wav";
    constexpr const char* SFX_PICKUP         = "Data/BT_Sounds/Bonus_Obtained.wav";
    constexpr const char* SFX_ITEM_APPEARS   = "Data/BT_Sounds/Bonus_Appear.wav";
    constexpr const char* SFX_LEVEL_COMPLETE = "Data/BT_Sounds/Round_Clear.wav";
    constexpr const char* SFX_GAME_START     = "Data/BT_Sounds/Game_Start.wav";
}
