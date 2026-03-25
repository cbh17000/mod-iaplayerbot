/*
 * mod-iaplayerbot — Layer 1: World hooks
 * OnUpdate is the SOLE entry point for all bot updates (scheduler-driven)
 */

#pragma once

#include "WorldScript.h"

class IABot_WorldScript : public WorldScript
{
public:
    IABot_WorldScript() : WorldScript("IABot_WorldScript", {
        WORLDHOOK_ON_BEFORE_WORLD_INITIALIZED,
        WORLDHOOK_ON_UPDATE
    }) {}

    void OnBeforeWorldInitialized() override;
    void OnUpdate(uint32 diff) override;
};
