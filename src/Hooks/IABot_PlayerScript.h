/*
 * mod-iaplayerbot — Layer 1: Player hooks
 * All signatures verified against AzerothCore PlayerScript.h
 */

#pragma once

#include "PlayerScript.h"

class IABot_PlayerScript : public PlayerScript
{
public:
    IABot_PlayerScript() : PlayerScript("IABot_PlayerScript", {
        // Lifecycle — only hooks we actually override
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_BEFORE_LOGOUT,
        PLAYERHOOK_ON_PLAYER_JUST_DIED,
        PLAYERHOOK_ON_PLAYER_RESURRECT,
        // Combat
        PLAYERHOOK_ON_PLAYER_ENTER_COMBAT,
        PLAYERHOOK_ON_PLAYER_LEAVE_COMBAT,
        // Chat (whisper to bot)
        PLAYERHOOK_CAN_PLAYER_USE_PRIVATE_CHAT
    }) {}

    // --- Lifecycle ---
    // ScriptMgr::OnPlayerLogin(Player*)
    void OnPlayerLogin(Player* player) override;

    // ScriptMgr::OnPlayerBeforeLogout(Player*)
    void OnPlayerBeforeLogout(Player* player) override;

    // ScriptMgr::OnPlayerJustDied(Player*)
    void OnPlayerJustDied(Player* player) override;

    // ScriptMgr::OnPlayerResurrect(Player*, float, bool)
    void OnPlayerResurrect(Player* player, float restorePercent, bool applySickness) override;

    // --- Combat ---
    // ScriptMgr::OnPlayerEnterCombat(Player*, Unit*)
    void OnPlayerEnterCombat(Player* player, Unit* enemy) override;

    // ScriptMgr::OnPlayerLeaveCombat(Player*)
    void OnPlayerLeaveCombat(Player* player) override;

    // --- Chat (whisper interception for bot commands) ---
    // ScriptMgr::OnPlayerCanUseChat(Player*, uint32, uint32, std::string&, Player*)
    // Returns true to allow the message, false to block
    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang,
                            std::string& msg, Player* receiver) override;
};
