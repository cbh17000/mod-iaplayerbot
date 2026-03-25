/*
 * mod-iaplayerbot — Layer 2: Global scheduler implementation
 *
 * Key design decisions:
 *   - accumulatedDiff is incremented for ALL bots every tick
 *   - Only bots that are actually updated get their accumulatedDiff reset
 *   - frameTimeUsed ALWAYS counts real CPU time (never skip counting)
 *   - Round-robin advances forward after processing each priority level
 *   - No mutex: game thread only
 */

#include "IABotScheduler.h"
#include "IABotSession.h"
#include "IABotMetrics.h"
#include "IABotConfig.h"
#include "Log.h"
#include <algorithm>
#include <chrono>

IABotScheduler* IABotScheduler::instance()
{
    static IABotScheduler inst;
    return &inst;
}

void IABotScheduler::Initialize()
{
    maxFrameBudgetUs_ = sIABotConfig->SchedulerCpuBudgetUs;
    priorityRecalcIntervalMs_ = sIABotConfig->SchedulerPriorityRecalcMs;

    for (auto& bucket : buckets_)
        bucket.reserve(32);

    LOG_INFO("module.iabot", "IABotScheduler initialized (budget: {} us, recalc: {} ms)",
             maxFrameBudgetUs_, priorityRecalcIntervalMs_);
}

void IABotScheduler::Register(ObjectGuid guid, IABotSession* session)
{
    uint8 prio = static_cast<uint8>(session->GetPriority());
    buckets_[prio].push_back({ guid, session, 0, 200.0f });
}

void IABotScheduler::Unregister(ObjectGuid guid)
{
    for (auto& bucket : buckets_)
    {
        auto it = std::remove_if(bucket.begin(), bucket.end(),
            [&guid](const BotTask& t) { return t.guid == guid; });
        if (it != bucket.end())
        {
            bucket.erase(it, bucket.end());
            return;
        }
    }
}

void IABotScheduler::SetPriority(ObjectGuid guid, BotPriority newPriority)
{
    uint8 newPrio = static_cast<uint8>(newPriority);

    for (uint8 p = 0; p < 5; ++p)
    {
        auto& bucket = buckets_[p];
        auto it = std::find_if(bucket.begin(), bucket.end(),
            [&guid](const BotTask& t) { return t.guid == guid; });

        if (it != bucket.end())
        {
            if (p == newPrio)
                return; // Already in the right bucket

            BotTask task = *it;
            bucket.erase(it);
            task.session->SetPriority(newPriority);
            buckets_[newPrio].push_back(task);
            return;
        }
    }
}

uint32 IABotScheduler::GetMinInterval(BotPriority prio) const
{
    switch (prio)
    {
        case BotPriority::CRITICAL: return sIABotConfig->SchedulerIntervalCritical;
        case BotPriority::HIGH:     return sIABotConfig->SchedulerIntervalHigh;
        case BotPriority::MEDIUM:   return sIABotConfig->SchedulerIntervalMedium;
        case BotPriority::LOW:      return sIABotConfig->SchedulerIntervalLow;
        case BotPriority::LOWEST:   return sIABotConfig->SchedulerIntervalLowest;
    }
    return 200;
}

void IABotScheduler::RecalculatePriorities()
{
    // Move bots between buckets based on their session's computed priority
    // We iterate all buckets and check if any bot should change
    for (uint8 p = 0; p < 5; ++p)
    {
        auto& bucket = buckets_[p];
        for (size_t i = 0; i < bucket.size(); )
        {
            BotTask& task = bucket[i];
            BotPriority current = task.session->GetPriority();
            uint8 shouldBe = static_cast<uint8>(current);

            if (shouldBe != p)
            {
                BotTask moved = task;
                bucket.erase(bucket.begin() + i);
                buckets_[shouldBe].push_back(moved);
                // Don't increment i — next element shifted into position
            }
            else
            {
                ++i;
            }
        }
    }
}

void IABotScheduler::Update(uint32 diff)
{
    // Step 1: Accumulate diff for ALL bots (even those not updated this tick)
    for (auto& bucket : buckets_)
        for (auto& task : bucket)
            task.accumulatedDiff += diff;

    // Step 2: Periodically recalculate priorities
    priorityRecalcTimer_ += diff;
    if (priorityRecalcTimer_ >= priorityRecalcIntervalMs_)
    {
        RecalculatePriorities();
        priorityRecalcTimer_ = 0;
    }

    // Step 3: Process buckets from highest to lowest priority
    uint32 frameTimeUsed = 0;

    for (uint8 p = 0; p < 5; ++p)
    {
        if (frameTimeUsed >= maxFrameBudgetUs_)
            break;

        auto& bucket = buckets_[p];
        if (bucket.empty())
            continue;

        BotPriority prio = static_cast<BotPriority>(p);
        uint32 minInterval = GetMinInterval(prio);
        size_t& rrIdx = rrIndex_[p];
        size_t processed = 0;

        for (size_t i = 0; i < bucket.size(); ++i)
        {
            if (frameTimeUsed >= maxFrameBudgetUs_)
                break;

            size_t idx = (rrIdx + i) % bucket.size();
            BotTask& task = bucket[idx];

            // Skip if not enough time accumulated (respects interval)
            if (task.accumulatedDiff < minInterval)
                continue;

            // Execute update
            auto start = std::chrono::steady_clock::now();
            uint32 updateDiff = std::min(task.accumulatedDiff, 500u);
            task.session->Update(updateDiff);
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start).count();

            uint32 taskTimeUs = static_cast<uint32>(elapsed);

            // ALWAYS count real CPU time
            frameTimeUsed += taskTimeUs;

            // Reset accumulated diff — this bot was updated
            task.accumulatedDiff = 0;

            // Rolling average CPU cost (80% old, 20% new)
            task.cpuCostAvgUs = task.cpuCostAvgUs * 0.8f + taskTimeUs * 0.2f;

            // Metrics
            sIABotMetrics->OnBotUpdate(task.guid, prio, taskTimeUs);

            ++processed;
        }

        // Advance round-robin index for next tick
        if (!bucket.empty())
            rrIdx = (rrIdx + processed) % bucket.size();
    }

    // Step 4: Record global metrics
    uint32 totalBots = 0;
    for (auto& b : buckets_)
        totalBots += b.size();

    sIABotMetrics->RecordTick(totalBots, frameTimeUsed, frameTimeUsed >= maxFrameBudgetUs_);
}
