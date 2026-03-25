/*
 * mod-iaplayerbot — Layer 1: Unit hooks
 * Signatures verified against AzerothCore UnitScript.h
 */

#pragma once

#include "UnitScript.h"

class IABot_UnitScript : public UnitScript
{
public:
    IABot_UnitScript() : UnitScript("IABot_UnitScript", true, {
        UNITHOOK_ON_UNIT_DEATH,
        UNITHOOK_ON_UNIT_ENTER_COMBAT,
        UNITHOOK_ON_DAMAGE
    }) {}

    // ScriptMgr::OnUnitDeath(Unit* unit, Unit* killer) — 2 params
    void OnUnitDeath(Unit* unit, Unit* killer) override;

    // ScriptMgr::OnUnitEnterCombat(Unit* unit, Unit* victim)
    void OnUnitEnterCombat(Unit* unit, Unit* victim) override;

    // ScriptMgr::OnDamage(Unit* attacker, Unit* victim, uint32& damage)
    void OnDamage(Unit* attacker, Unit* victim, uint32& damage) override;
};
