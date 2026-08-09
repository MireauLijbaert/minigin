#pragma once
#include "Observer.h"
#include "Event.h"
#include "ServiceLocator.h"
#include "BtSounds.h"

// Listens to game events and maps them to sound calls.
// Add this as an observer to any Subject-component that fires audio-relevant events.
class SoundObserver final : public dae::Observer
{
public:
    void Notify(const dae::Event& event, dae::GameObject*) override
    {
        auto& ss = dae::ServiceLocator::GetSoundSystem();

        if      (event.id == "BurgerSegmentPressed") ss.Play(BtSounds::SFX_WALK, 128);
        else if (event.id == "BurgerDropped")        ss.Play(BtSounds::SFX_BURGER_DROP, 128);
        else if (event.id == "BurgerLanded")         ss.Play(BtSounds::SFX_BURGER_LAND, 128);
        else if (event.id == "EnemySquished")        ss.Play(BtSounds::SFX_SQUISH, 128);
        else if (event.id == "EnemyFell")            ss.Play(BtSounds::SFX_ENEMY_FALL, 128);
        else if (event.id == "EnemyStunned")         ss.Play(BtSounds::SFX_ENEMY_SPRAYED, 128);
        else if (event.id == "PepperFired")          ss.Play(BtSounds::SFX_PEPPER, 128);
        else if (event.id == "PlayerDied")           { ss.StopMusic(); ss.PlayMusicAfter(BtSounds::BGM_LEVEL, BtSounds::SFX_PLAYER_DEATH, 128); }
        else if (event.id == "BonusAppeared")        ss.Play(BtSounds::SFX_ITEM_APPEARS, 128);
        else if (event.id == "BonusPickedUp")        ss.Play(BtSounds::SFX_PICKUP, 128);
        else if (event.id == "LevelComplete")        ss.Play(BtSounds::SFX_LEVEL_COMPLETE, 128);
    }
};
