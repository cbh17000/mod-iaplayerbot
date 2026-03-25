/*
 * mod-iaplayerbot — Bot Player creation & login implementation
 *
 * This follows the exact same pattern as mod-playerbots PlayerbotHolder::AddPlayerBot:
 *   1. Create LoginQueryHolder with the bot's account + GUID
 *   2. Submit to CharacterDatabase.DelayQueryHolder (async)
 *   3. On callback: create WorldSession, call HandlePlayerLoginFromDB
 *   4. Extract Player*, wire into IABotSession
 *
 * Key difference from mod-playerbots: we don't need PlayerbotAI, we wire
 * directly into our IABotSession/Brain architecture.
 */

#include "IABotLoginMgr.h"
#include "IABotManager.h"
#include "IABotSession.h"
#include "IABotScheduler.h"
#include "IABotConfig.h"
#include "CharacterCache.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "World.h"
#include "WorldSession.h"

IABotLoginMgr* IABotLoginMgr::instance()
{
    static IABotLoginMgr inst;
    return &inst;
}

uint32 IABotLoginMgr::GetBotAccountId() const
{
    return sConfigMgr->GetOption<uint32>("IABot.Account", 0);
}

bool IABotLoginMgr::IsLoading(ObjectGuid charGuid) const
{
    return loading_.count(charGuid) > 0;
}

ObjectGuid IABotLoginMgr::FindAvailableBotChar(uint8 playerClass, uint8 /*race*/) const
{
    uint32 accountId = GetBotAccountId();
    if (!accountId)
        return ObjectGuid::Empty;

    // Query: find a character of the requested class in the bot account
    // that is NOT currently online and NOT currently loading
    QueryResult result = CharacterDatabase.Query(
        "SELECT guid FROM characters WHERE account = {} AND class = {} AND online = 0 ORDER BY RAND() LIMIT 1",
        accountId, playerClass);

    if (!result)
        return ObjectGuid::Empty;

    uint32 guidLow = result->Fetch()[0].Get<uint32>();
    ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(guidLow);

    // Check not already online or loading
    if (ObjectAccessor::FindConnectedPlayer(guid))
        return ObjectGuid::Empty;
    if (IsLoading(guid))
        return ObjectGuid::Empty;

    return guid;
}

bool IABotLoginMgr::BeginBotLogin(ObjectGuid charGuid, Player* master)
{
    if (!master || charGuid.IsEmpty())
        return false;

    // Already loading?
    if (loading_.count(charGuid))
    {
        LOG_DEBUG("module.iabot", "BeginBotLogin: {} already loading", charGuid.GetCounter());
        return false;
    }

    // Already online?
    if (ObjectAccessor::FindConnectedPlayer(charGuid))
    {
        LOG_DEBUG("module.iabot", "BeginBotLogin: {} already online", charGuid.GetCounter());
        return false;
    }

    // Get account ID for this character
    uint32 accountId = sCharacterCache->GetCharacterAccountIdByGuid(charGuid);
    if (!accountId)
    {
        LOG_ERROR("module.iabot", "BeginBotLogin: no account for GUID {}", charGuid.GetCounter());
        return false;
    }

    // Create the async query holder (same as WoW client login)
    ObjectGuid masterGuid = master->GetGUID();
    auto holder = std::make_shared<IABotLoginQueryHolder>(accountId, charGuid, masterGuid);

    if (!holder->Initialize())
    {
        LOG_ERROR("module.iabot", "BeginBotLogin: LoginQueryHolder init failed for {}", charGuid.GetCounter());
        return false;
    }

    // Mark as loading
    loading_.insert(charGuid);

    LOG_INFO("module.iabot", "BeginBotLogin: starting async login for GUID {} (master: {})",
             charGuid.GetCounter(), masterGuid.GetCounter());

    // Submit async query — callback runs on the game thread
    sWorld->AddQueryHolderCallback(CharacterDatabase.DelayQueryHolder(holder))
        .AfterComplete(
            [](SQLQueryHolderBase const& queryHolder)
            {
                IABotLoginQueryHolder const& h =
                    static_cast<IABotLoginQueryHolder const&>(queryHolder);
                sIABotLoginMgr->OnLoginQueryComplete(h);
            });

    return true;
}

void IABotLoginMgr::OnLoginQueryComplete(IABotLoginQueryHolder const& holder)
{
    ObjectGuid charGuid = holder.GetGuid();
    ObjectGuid masterGuid = holder.GetMasterGUID();
    uint32 botAccountId = holder.GetAccountId();

    // Remove from loading set
    loading_.erase(charGuid);

    // Find the master player (they might have logged out during async load)
    Player* master = ObjectAccessor::FindConnectedPlayer(masterGuid);
    if (!master)
    {
        LOG_WARN("module.iabot", "OnLoginQueryComplete: master {} disconnected, aborting bot {} login",
                 masterGuid.GetCounter(), charGuid.GetCounter());
        return;
    }

    // Check limits again (might have changed during async load)
    if (sIABotMgr->GetSessions().size() >= sIABotConfig->MaxBots)
    {
        LOG_WARN("module.iabot", "OnLoginQueryComplete: global bot limit reached, aborting");
        return;
    }

    // ========================================================
    // CREATE THE WORLD SESSION (the critical part)
    // ========================================================
    // This is exactly what mod-playerbots does:
    //   new WorldSession(accountId, "", nullptr, SEC_PLAYER, expansion, ...)
    // The last parameter `true` marks it as a bot session.
    WorldSession* botSession = new WorldSession(
        botAccountId,                                        // accountId
        "",                                                  // accountName (empty for bots)
        0x0,                                                 // sockets/flags
        nullptr,                                             // no real socket
        SEC_PLAYER,                                          // security level
        EXPANSION_WRATH_OF_THE_LICH_KING,                    // expansion
        time_t(0),                                           // mute time
        sWorld->GetDefaultDbcLocale(),                       // locale
        0,                                                   // recruit-a-friend
        false,                                               // isARecruiter
        false,                                               // skipQueue
        0,                                                   // totalTime
        true                                                 // isBot
    );

    // ========================================================
    // LOGIN THE PLAYER FROM DB (loads gear, spells, quests, etc.)
    // ========================================================
    botSession->HandlePlayerLoginFromDB(holder);

    Player* bot = botSession->GetPlayer();
    if (!bot)
    {
        LOG_ERROR("module.iabot", "OnLoginQueryComplete: HandlePlayerLoginFromDB failed for {}",
                  charGuid.GetCounter());
        botSession->LogoutPlayer(true);
        delete botSession;
        return;
    }

    // ========================================================
    // GET CLASS & RACE FROM THE LOADED PLAYER
    // ========================================================
    uint8 playerClass = bot->getClass();
    uint8 playerRace = bot->getRace();
    uint8 spec = 0; // TODO: detect active spec from talents

    LOG_INFO("module.iabot", "Bot logged in: GUID={} Name='{}' Class={} Level={}",
             charGuid.GetCounter(), bot->GetName(), playerClass, bot->GetLevel());

    // ========================================================
    // CREATE THE IABOT SESSION AND WIRE EVERYTHING
    // ========================================================
    auto session = std::make_unique<IABotSession>(bot, master, playerClass, playerRace, spec);
    IABotSession* sessionPtr = session.get();

    // Store in manager
    sIABotMgr->RegisterSession(charGuid, std::move(session), master->GetGUID());

    // Register in scheduler
    sIABotScheduler->Register(charGuid, sessionPtr);

    // Teleport bot to master's location
    if (master->IsInWorld())
    {
        bot->TeleportTo(master->GetMapId(),
                        master->GetPositionX(),
                        master->GetPositionY(),
                        master->GetPositionZ(),
                        master->GetOrientation());
    }

    LOG_INFO("module.iabot", "Bot {} fully initialized and added to world near master {}",
             charGuid.GetCounter(), masterGuid.GetCounter());
}

void IABotLoginMgr::OnBotRemoved(ObjectGuid charGuid)
{
    loading_.erase(charGuid);
}
