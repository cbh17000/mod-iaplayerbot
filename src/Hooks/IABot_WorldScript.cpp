/*
 * mod-iaplayerbot — Layer 1: World hooks implementation
 */

#include "IABot_WorldScript.h"
#include "IABotManager.h"
#include "IABotScheduler.h"
#include "IABotConfig.h"
#include "Log.h"

void IABot_WorldScript::OnBeforeWorldInitialized()
{
    LOG_INFO("module.iabot", "===========================================");
    LOG_INFO("module.iabot", "  mod-iaplayerbot v2.0 — Initializing...");
    LOG_INFO("module.iabot", "===========================================");

    sIABotConfig->Load();
    sIABotMgr->Initialize();
    sIABotScheduler->Initialize();

    LOG_INFO("module.iabot", "mod-iaplayerbot initialized (MaxBots: {}, BudgetUs: {})",
             sIABotConfig->MaxBots, sIABotConfig->SchedulerCpuBudgetUs);
}

void IABot_WorldScript::OnUpdate(uint32 diff)
{
    if (!sIABotConfig->Enabled)
        return;

    // Single entry point for ALL bot updates
    sIABotScheduler->Update(diff);
}
