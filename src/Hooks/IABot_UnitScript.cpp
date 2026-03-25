/*
 * mod-iaplayerbot — Layer 1: Unit hooks implementation
 */

#include "IABot_UnitScript.h"
#include "IABotManager.h"
#include "IABotSession.h"
#include "IABotScheduler.h"
#include "Player.h"
#include "Log.h"

void IABot_UnitScript::OnUnitDeath(Unit* unit, Unit* killer)
{
    if (!unit)
        return;

    // Case 1: A bot died
    if (unit->GetTypeId() == TYPEID_PLAYER)
    {
        if (IABotSession* session = sIABotMgr->GetSession(unit->GetGUID()))
            session->OnDeath(killer);
    }

    // Case 2: Something that bots were fighting died — notify all bots in combat
    sIABotMgr->OnTargetDeath(unit, killer);
}

void IABot_UnitScript::OnUnitEnterCombat(Unit* unit, Unit* /*victim*/)
{
    if (!unit)
        return;

    // Boost priority when a bot enters combat
    if (unit->GetTypeId() == TYPEID_PLAYER)
    {
        if (sIABotMgr->IsBot(unit->ToPlayer()))
            sIABotScheduler->SetPriority(unit->GetGUID(), BotPriority::HIGH);
    }
}

void IABot_UnitScript::OnDamage(Unit* attacker, Unit* victim, uint32& /*damage*/)
{
    if (!victim)
        return;

    // If a bot takes damage, invalidate its perception cache for immediate threat update
    if (victim->GetTypeId() == TYPEID_PLAYER)
    {
        if (IABotSession* session = sIABotMgr->GetSession(victim->GetGUID()))
            session->OnDamageTaken(attacker);
    }
}
