/*
 * mod-iaplayerbot — Layer 2: Bot session
 * One session = one bot Player* with LOD gate, priority, and master reference
 */

#pragma once

#include "ObjectGuid.h"
#include "IABotDefines.h"
#include <memory>

class Player;
class Unit;
class IABotBrain;
class IClassModule;
struct BotPersonality;

class IABotSession
{
public:
    IABotSession(Player* bot, Player* master, uint8 playerClass, uint8 race, uint8 spec);
    ~IABotSession();

    // Called by Scheduler
    void Update(uint32 diff);

    // Events from Layer 1 hooks
    void OnLogin();
    void OnDeath(Unit* killer);
    void OnResurrect();
    void OnDamageTaken(Unit* attacker);
    void OnWhisperReceived(Player* sender, const std::string& msg);

    // LOD
    AILOD GetLOD() const { return lod_; }
    void RecalculateLOD();

    // Priority
    BotPriority GetPriority() const { return priority_; }
    void SetPriority(BotPriority p) { priority_ = p; }

    // Accessors
    Player* GetBot() const { return bot_; }
    Player* GetMaster() const { return master_; }
    ObjectGuid GetBotGUID() const;
    uint8 GetClass() const { return playerClass_; }
    uint8 GetSpec() const { return spec_; }

private:
    void UpdateMinimal(uint32 diff);

    Player* bot_;
    Player* master_;
    uint8 playerClass_;
    uint8 race_;
    uint8 spec_;

    AILOD lod_ = AILOD::FULL;
    BotPriority priority_ = BotPriority::MEDIUM;

    // Brain will be created in Phase 2
    // std::unique_ptr<IABotBrain> brain_;
    // std::unique_ptr<IClassModule> classModule_;
};
