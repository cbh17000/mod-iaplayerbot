/*
 * mod-iaplayerbot — Layer 2: Configuration
 * Loaded from iaplayerbot.conf via AzerothCore ConfigMgr
 */

#pragma once

#include "Define.h"
#include <string>

class IABotConfig
{
public:
    static IABotConfig* instance();

    void Load();

    // General
    bool Enabled = true;
    uint32 MaxBots = 100;
    uint32 MaxBotsPerPlayer = 3;
    float FollowDistance = 15.0f;

    // Scheduler
    uint32 SchedulerCpuBudgetUs = 5000;
    uint32 SchedulerPriorityRecalcMs = 500;
    uint32 SchedulerIntervalCritical = 50;
    uint32 SchedulerIntervalHigh = 100;
    uint32 SchedulerIntervalMedium = 200;
    uint32 SchedulerIntervalLow = 500;
    uint32 SchedulerIntervalLowest = 1000;

    // LOD
    float LODDistanceFull = 30.0f;
    float LODDistanceReduced = 80.0f;

    // Perception
    uint32 PerceptionCacheCombat = 100;
    uint32 PerceptionCacheNormal = 200;
    uint32 PerceptionCacheFar = 500;

    // Decision cache
    bool DecisionCacheEnabled = true;
    uint32 DecisionCacheDefaultTTL = 300;

    // Intent
    uint32 IntentUpdateInterval = 750;

    // Bridge
    bool BridgeEnabled = false;
    std::string BridgeEndpoint = "http://127.0.0.1:8080/v1/advice";
    uint32 BridgeTimeoutMs = 200;
    uint32 BridgeBatchSize = 10;
    float BridgeMinConfidence = 0.6f;

    // Personality
    bool PersonalityEnabled = true;
    bool PersonalityRandomize = true;

    // Social
    bool SocialEnabled = false;
    uint32 SocialMaxActiveBots = 10;
    uint32 SocialTickIntervalMs = 3000;

    // Gameplay (v3)
    bool EnableQuesting = false;
    bool EnableDungeons = false;
    bool EnableGathering = false;
    bool EnableEconomy = false;

    // Debug
    bool Debug = false;
    bool DebugPerf = false;
    bool DebugBT = false;
    bool DebugScheduler = false;

private:
    IABotConfig() = default;
};

#define sIABotConfig IABotConfig::instance()
