/*
 * mod-iaplayerbot — Layer 1: Player hooks implementation
 */

#include "IABot_PlayerScript.h"
#include "IABotManager.h"
#include "IABotSession.h"
#include "IABotScheduler.h"
#include "Log.h"
#include "Player.h"

void IABot_PlayerScript::OnPlayerLogin(Player* player)
{
    if (!player)
        return;

    if (sIABotMgr->IsBot(player))
    {
        // Bot reconnecting (edge case after crash recovery)
        if (IABotSession* session = sIABotMgr->GetSession(player->GetGUID()))
            session->OnLogin();
    }
    else
    {
        // Real player login — notify manager for potential bot spawning
        sIABotMgr->OnMasterLogin(player);
    }
}

void IABot_PlayerScript::OnPlayerBeforeLogout(Player* player)
{
    if (!player)
        return;

    if (sIABotMgr->IsBot(player))
    {
        sIABotMgr->RemoveBot(player->GetGUID());
    }
    else
    {
        // Real player leaving — remove all their bots
        sIABotMgr->OnMasterLogout(player);
    }
}

void IABot_PlayerScript::OnPlayerJustDied(Player* player)
{
    if (!player)
        return;

    if (IABotSession* session = sIABotMgr->GetSession(player->GetGUID()))
        session->OnDeath(nullptr);
}

void IABot_PlayerScript::OnPlayerResurrect(Player* player, float /*restorePercent*/, bool /*applySickness*/)
{
    if (!player)
        return;

    if (IABotSession* session = sIABotMgr->GetSession(player->GetGUID()))
        session->OnResurrect();
}

void IABot_PlayerScript::OnPlayerEnterCombat(Player* player, Unit* /*enemy*/)
{
    if (!player)
        return;

    if (IABotSession* session = sIABotMgr->GetSession(player->GetGUID()))
        sIABotScheduler->SetPriority(player->GetGUID(), BotPriority::CRITICAL);
}

void IABot_PlayerScript::OnPlayerLeaveCombat(Player* player)
{
    if (!player)
        return;

    if (IABotSession* session = sIABotMgr->GetSession(player->GetGUID()))
    {
        // Priority will be recalculated by scheduler on next cycle
        // For now, drop back to MEDIUM as a safe default
        sIABotScheduler->SetPriority(player->GetGUID(), BotPriority::MEDIUM);
    }
}

bool IABot_PlayerScript::OnPlayerCanUseChat(Player* player, uint32 type, uint32 /*lang*/,
                                             std::string& msg, Player* receiver)
{
    if (!receiver)
        return true;

    // If someone whispers a bot, route the message to the bot's brain
    if (IABotSession* session = sIABotMgr->GetSession(receiver->GetGUID()))
    {
        session->OnWhisperReceived(player, msg);
        // Don't block the message — let it appear in chat normally
    }

    return true;
}
