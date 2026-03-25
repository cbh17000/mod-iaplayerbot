/*
 * mod-iaplayerbot — Layer 2: Performance metrics implementation
 */

#include "IABotMetrics.h"
#include "Log.h"

IABotMetrics* IABotMetrics::instance()
{
    static IABotMetrics inst;
    return &inst;
}

void IABotMetrics::OnBotUpdate(ObjectGuid guid, BotPriority priority, uint32 timeMicros)
{
    totalUpdates_++;
    totalTimeUs_ += timeMicros;
    updatesPerPriority_[static_cast<uint8>(priority)]++;

    auto& stats = botStats_[guid];
    stats.totalUpdates++;
    stats.totalTimeUs += timeMicros;
    stats.lastUpdateUs = timeMicros;
}

void IABotMetrics::RecordTick(uint32 totalBots, uint32 /*frameTimeUs*/, bool budgetOverflow)
{
    tickCount_++;
    lastTickBots_ = totalBots;
    if (budgetOverflow)
        overflowCount_++;
}

IABotMetrics::GlobalStats IABotMetrics::GetGlobalStats() const
{
    GlobalStats s;
    s.totalBots = lastTickBots_;

    uint32 ticks = tickCount_.load();
    uint32 updates = totalUpdates_.load();

    s.avgBotsPerTick = ticks > 0 ? static_cast<float>(updates) / ticks : 0.0f;
    s.avgTickBudgetUs = updates > 0 ? totalTimeUs_.load() / ticks : 0;
    s.budgetOverflowRate = ticks > 0 ? static_cast<float>(overflowCount_.load()) / ticks : 0.0f;

    for (uint8 i = 0; i < 5; ++i)
        s.botsPerPriority[i] = updatesPerPriority_[i].load();

    return s;
}

const IABotMetrics::BotStats* IABotMetrics::GetBotStats(ObjectGuid guid) const
{
    auto it = botStats_.find(guid);
    return (it != botStats_.end()) ? &it->second : nullptr;
}

void IABotMetrics::Reset()
{
    totalUpdates_ = 0;
    totalTimeUs_ = 0;
    tickCount_ = 0;
    overflowCount_ = 0;
    for (auto& a : updatesPerPriority_)
        a = 0;
    botStats_.clear();
    lastTickBots_ = 0;
}
