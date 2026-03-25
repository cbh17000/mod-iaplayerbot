/*
 * mod-iaplayerbot — Layer 2: Global scheduler
 * Distributes bot updates across ticks with priority-based time-slicing.
 * Called from game thread only via WorldScript::OnUpdate.
 */

#pragma once

#include "ObjectGuid.h"
#include "IABotDefines.h"
#include <array>
#include <vector>
#include <chrono>

class IABotSession;

class IABotScheduler
{
public:
    static IABotScheduler* instance();

    void Initialize();

    // Called once per world tick from IABot_WorldScript::OnUpdate
    void Update(uint32 diff);

    // Session management
    void Register(ObjectGuid guid, IABotSession* session);
    void Unregister(ObjectGuid guid);
    void SetPriority(ObjectGuid guid, BotPriority newPriority);

private:
    // Recalculate priorities for all bots (called periodically, not every tick)
    void RecalculatePriorities();

    // Get minimum update interval for a priority level
    uint32 GetMinInterval(BotPriority prio) const;

    struct BotTask
    {
        ObjectGuid guid;
        IABotSession* session;
        uint32 accumulatedDiff;   // Time accumulated since last update
        float cpuCostAvgUs;       // Rolling average cost in microseconds
    };

    // 5 priority buckets (CRITICAL=0 .. LOWEST=4)
    std::array<std::vector<BotTask>, 5> buckets_;

    // Round-robin index per priority level
    std::array<size_t, 5> rrIndex_ = {};

    // Priority recalculation timer
    uint32 priorityRecalcTimer_ = 0;

    // Config (cached from IABotConfig at Initialize)
    uint32 maxFrameBudgetUs_ = 5000;
    uint32 priorityRecalcIntervalMs_ = 500;
};

#define sIABotScheduler IABotScheduler::instance()
