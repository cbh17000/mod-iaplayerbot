/*
 * mod-iaplayerbot — Bot Player creation & login
 *
 * This is the critical piece: creating a real Player* in-world from a DB character.
 *
 * Flow (same as mod-playerbots, adapted for our architecture):
 *   1. GM does ".iabot add warrior" → IABotManager::AddBot()
 *   2. We look up (or create) a character GUID in the bot account
 *   3. We create a LoginQueryHolder and submit it async to CharacterDatabase
 *   4. On callback (game thread), we create a WorldSession + call HandlePlayerLoginFromDB
 *   5. We get a valid Player* → create IABotSession → register in Scheduler
 *
 * Prerequisites:
 *   - A dedicated account must exist in DB (config: IABot.Account)
 *   - Characters for bots must exist in that account
 *   - Or: we create characters on-the-fly (Phase 1b)
 */

#pragma once

#include "ObjectGuid.h"
#include "WorldSession.h"
#include <unordered_set>

class Player;

class IABotLoginQueryHolder : public LoginQueryHolder
{
public:
    IABotLoginQueryHolder(uint32 accountId, ObjectGuid guid, ObjectGuid masterGuid)
        : LoginQueryHolder(accountId, guid)
        , masterGuid_(masterGuid)
    {}

    ObjectGuid GetMasterGUID() const { return masterGuid_; }

private:
    ObjectGuid masterGuid_;
};

class IABotLoginMgr
{
public:
    static IABotLoginMgr* instance();

    // Start the async login process for a bot character
    // Returns false if the character is already loading or doesn't exist
    bool BeginBotLogin(ObjectGuid charGuid, Player* master);

    // Called on the game thread when the DB query completes
    void OnLoginQueryComplete(IABotLoginQueryHolder const& holder);

    // Called by IABotManager when a bot is removed
    void OnBotRemoved(ObjectGuid charGuid);

    // Check if a character is currently in the loading pipeline
    bool IsLoading(ObjectGuid charGuid) const;

    // Find an available character GUID in the bot account for the given class
    // Returns empty GUID if none found
    ObjectGuid FindAvailableBotChar(uint8 playerClass, uint8 race) const;

    // Get the configured bot account ID
    uint32 GetBotAccountId() const;

private:
    IABotLoginMgr() = default;

    // GUIDs currently in the async login pipeline (prevent double-login)
    std::unordered_set<ObjectGuid> loading_;
};

#define sIABotLoginMgr IABotLoginMgr::instance()
