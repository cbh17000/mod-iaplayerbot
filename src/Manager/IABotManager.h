/*
 * mod-iaplayerbot — Layer 2: Bot manager singleton
 * Owns all sessions. Called from game thread only (no mutex needed for sessions).
 */

#pragma once

#include "ObjectGuid.h"
#include <unordered_map>
#include <memory>

class Player;
class Unit;
class ChatHandler;
class IABotSession;

class IABotManager
{
public:
    static IABotManager* instance();

    void Initialize();
    void Shutdown();

    // Bot lifecycle — AddBot starts async login, RegisterSession is the callback
    bool AddBot(Player* master, uint8 playerClass, uint8 race, uint8 spec);
    void RegisterSession(ObjectGuid botGuid, std::unique_ptr<IABotSession> session, ObjectGuid masterGuid);
    void RemoveBot(ObjectGuid guid);
    uint32 RemoveAllBots(Player* master);
    void LogoutBot(ObjectGuid guid);

    // Lookup
    IABotSession* GetSession(ObjectGuid guid) const;
    bool IsBot(Player* player) const;

    // Events from hooks
    void OnMasterLogin(Player* master);
    void OnMasterLogout(Player* master);
    void OnTargetDeath(Unit* target, Unit* killer);

    // GM commands
    void ListBots(ChatHandler* handler, Player* master) const;

    // Iteration (used by Scheduler)
    const std::unordered_map<ObjectGuid, std::unique_ptr<IABotSession>>& GetSessions() const { return sessions_; }

private:
    IABotManager() = default;

    std::unordered_map<ObjectGuid, std::unique_ptr<IABotSession>> sessions_;

    // Master → list of bot GUIDs (for RemoveAllBots)
    std::unordered_map<ObjectGuid, std::vector<ObjectGuid>> masterBots_;
};

#define sIABotMgr IABotManager::instance()
