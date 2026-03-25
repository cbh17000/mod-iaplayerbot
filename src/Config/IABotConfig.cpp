/*
 * mod-iaplayerbot — Layer 2: Configuration implementation
 */

#include "IABotConfig.h"
#include "Config.h"
#include "Log.h"

IABotConfig* IABotConfig::instance()
{
    static IABotConfig inst;
    return &inst;
}

void IABotConfig::Load()
{
    // General
    Enabled               = sConfigMgr->GetOption<bool>  ("IABot.Enabled", true);
    MaxBots               = sConfigMgr->GetOption<uint32>("IABot.MaxBots", 100);
    MaxBotsPerPlayer      = sConfigMgr->GetOption<uint32>("IABot.MaxBotsPerPlayer", 3);
    FollowDistance        = sConfigMgr->GetOption<float> ("IABot.FollowDistance", 15.0f);

    // Scheduler
    SchedulerCpuBudgetUs      = sConfigMgr->GetOption<uint32>("IABot.Scheduler.CpuBudgetUs", 5000);
    SchedulerPriorityRecalcMs = sConfigMgr->GetOption<uint32>("IABot.Scheduler.PriorityRecalcMs", 500);
    SchedulerIntervalCritical = sConfigMgr->GetOption<uint32>("IABot.Scheduler.IntervalCritical", 50);
    SchedulerIntervalHigh     = sConfigMgr->GetOption<uint32>("IABot.Scheduler.IntervalHigh", 100);
    SchedulerIntervalMedium   = sConfigMgr->GetOption<uint32>("IABot.Scheduler.IntervalMedium", 200);
    SchedulerIntervalLow      = sConfigMgr->GetOption<uint32>("IABot.Scheduler.IntervalLow", 500);
    SchedulerIntervalLowest   = sConfigMgr->GetOption<uint32>("IABot.Scheduler.IntervalLowest", 1000);

    // LOD
    LODDistanceFull    = sConfigMgr->GetOption<float>("IABot.LOD.DistanceFull", 30.0f);
    LODDistanceReduced = sConfigMgr->GetOption<float>("IABot.LOD.DistanceReduced", 80.0f);

    // Perception
    PerceptionCacheCombat = sConfigMgr->GetOption<uint32>("IABot.Perception.CacheTTLCombat", 100);
    PerceptionCacheNormal = sConfigMgr->GetOption<uint32>("IABot.Perception.CacheTTLNormal", 200);
    PerceptionCacheFar    = sConfigMgr->GetOption<uint32>("IABot.Perception.CacheTTLFar", 500);

    // Decision cache
    DecisionCacheEnabled    = sConfigMgr->GetOption<bool>  ("IABot.DecisionCache.Enabled", true);
    DecisionCacheDefaultTTL = sConfigMgr->GetOption<uint32>("IABot.DecisionCache.DefaultTTL", 300);

    // Intent
    IntentUpdateInterval = sConfigMgr->GetOption<uint32>("IABot.Intent.UpdateInterval", 750);

    // Bridge
    BridgeEnabled       = sConfigMgr->GetOption<bool>       ("IABot.Bridge.Enabled", false);
    BridgeEndpoint      = sConfigMgr->GetOption<std::string>("IABot.Bridge.Endpoint", "http://127.0.0.1:8080/v1/advice");
    BridgeTimeoutMs     = sConfigMgr->GetOption<uint32>     ("IABot.Bridge.TimeoutMs", 200);
    BridgeBatchSize     = sConfigMgr->GetOption<uint32>     ("IABot.Bridge.BatchSize", 10);
    BridgeMinConfidence = sConfigMgr->GetOption<float>      ("IABot.Bridge.MinConfidence", 0.6f);

    // Personality
    PersonalityEnabled   = sConfigMgr->GetOption<bool>("IABot.Personality.Enabled", true);
    PersonalityRandomize = sConfigMgr->GetOption<bool>("IABot.Personality.Randomize", true);

    // Social
    SocialEnabled         = sConfigMgr->GetOption<bool>  ("IABot.Social.Enabled", false);
    SocialMaxActiveBots   = sConfigMgr->GetOption<uint32>("IABot.Social.MaxActiveBots", 10);
    SocialTickIntervalMs  = sConfigMgr->GetOption<uint32>("IABot.Social.TickIntervalMs", 3000);

    // Gameplay (v3)
    EnableQuesting  = sConfigMgr->GetOption<bool>("IABot.Gameplay.EnableQuesting", false);
    EnableDungeons  = sConfigMgr->GetOption<bool>("IABot.Gameplay.EnableDungeons", false);
    EnableGathering = sConfigMgr->GetOption<bool>("IABot.Gameplay.EnableGathering", false);
    EnableEconomy   = sConfigMgr->GetOption<bool>("IABot.Gameplay.EnableEconomy", false);

    // Debug
    Debug          = sConfigMgr->GetOption<bool>("IABot.Debug", false);
    DebugPerf      = sConfigMgr->GetOption<bool>("IABot.Debug.Perf", false);
    DebugBT        = sConfigMgr->GetOption<bool>("IABot.Debug.ShowBT", false);
    DebugScheduler = sConfigMgr->GetOption<bool>("IABot.Debug.ShowScheduler", false);

    LOG_INFO("module.iabot", "IABotConfig loaded: Enabled={}, MaxBots={}, Bridge={}",
             Enabled, MaxBots, BridgeEnabled);
}
