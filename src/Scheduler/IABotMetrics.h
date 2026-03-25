/*
 * mod-iaplayerbot — Layer 2: Performance metrics
 * Thread-safe for read access (GM commands), write is game-thread only.
 */

#pragma once

#include "ObjectGuid.h"
#include "IABotDefines.h"
#include <array>
#include <atomic>
#include <unordered_map>

class IABotMetrics
{
public:
    static IABotMetrics* instance();

    // Called by Scheduler after each bot update
    void OnBotUpdate(ObjectGuid guid, BotPriority priority, uint32 timeMicros);

    // Called by Scheduler at end of each tick
    void RecordTick(uint32 totalBots, uint32 frameTimeUs, bool budgetOverflow);

    // Read by GM commands (may be called from different context)
    struct GlobalStats
    {
        uint32 totalBots;
        float avgBotsPerTick;
        uint32 avgTickBudgetUs;
        float budgetOverflowRate;
        std::array<uint32, 5> botsPerPriority;
    };

    GlobalStats GetGlobalStats() const;

    // Per-bot stats (for .iabot debug perf <bot>)
    struct BotStats
    {
        uint32 totalUpdates = 0;
        uint32 totalTimeUs = 0;
        uint32 lastUpdateUs = 0;
        float AvgUs() const { return totalUpdates > 0 ? static_cast<float>(totalTimeUs) / totalUpdates : 0.0f; }
    };

    const BotStats* GetBotStats(ObjectGuid guid) const;

    void Reset();

private:
    IABotMetrics() = default;

    // Global counters
    std::atomic<uint32> totalUpdates_{0};
    std::atomic<uint32> totalTimeUs_{0};
    std::atomic<uint32> tickCount_{0};
    std::atomic<uint32> overflowCount_{0};
    std::array<std::atomic<uint32>, 5> updatesPerPriority_{};

    // Per-bot (game thread only — no concurrent writes)
    std::unordered_map<ObjectGuid, BotStats> botStats_;

    // Last tick snapshot
    uint32 lastTickBots_ = 0;
};

#define sIABotMetrics IABotMetrics::instance()
