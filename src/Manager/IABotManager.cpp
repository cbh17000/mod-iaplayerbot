/*
 * mod-iaplayerbot — Layer 2: Bot manager implementation (v2 — with real login)
 * All methods called from game thread only.
 */

#include "IABotManager.h"
#include "IABotSession.h"
#include "IABotLoginMgr.h"
#include "IABotScheduler.h"
#include "IABotConfig.h"
#include "Chat.h"
#include "Player.h"
#include "Log.h"

IABotManager* IABotManager::instance()
{
    static IABotManager inst;
    return &inst;
}

void IABotManager::Initialize()
{
    uint32 accountId = sIABotLoginMgr->GetBotAccountId();
    if (!accountId)
    {
        LOG_ERROR("module.iabot",
            "IABot.Account not configured! Set IABot.Account in iaplayerbot.conf "
            "to a dedicated account with pre-created bot characters.");
    }
    else
    {
        LOG_INFO("module.iabot", "IABotManager initialized (bot account: {})", accountId);
    }
}

void IABotManager::Shutdown()
{
    LOG_INFO("module.iabot", "IABotManager shutting down, removing {} bots", sessions_.size());

    // Logout all bots properly
    std::vector<ObjectGuid> guids;
    guids.reserve(sessions_.size());
    for (auto& [guid, session] : sessions_)
        guids.push_back(guid);

    for (ObjectGuid guid : guids)
        LogoutBot(guid);

    sessions_.clear();
    masterBots_.clear();
}

// ============================================================
// AddBot — starts the ASYNC login flow
// ============================================================
bool IABotManager::AddBot(Player* master, uint8 playerClass, uint8 /*race*/, uint8 /*spec*/)
{
    if (!master)
        return false;

    ObjectGuid masterGuid = master->GetGUID();

    // Check per-player limit
    if (masterBots_.count(masterGuid) &&
        masterBots_[masterGuid].size() >= sIABotConfig->MaxBotsPerPlayer)
    {
        LOG_DEBUG("module.iabot", "AddBot: per-player limit reached for {}", masterGuid.GetCounter());
        return false;
    }

    // Check global limit
    if (sessions_.size() >= sIABotConfig->MaxBots)
    {
        LOG_DEBUG("module.iabot", "AddBot: global limit reached ({})", sIABotConfig->MaxBots);
        return false;
    }

    // Find an available character of the requested class in the bot account
    ObjectGuid charGuid = sIABotLoginMgr->FindAvailableBotChar(playerClass, 0);
    if (charGuid.IsEmpty())
    {
        LOG_WARN("module.iabot",
            "AddBot: no available class {} character in bot account. "
            "Create characters manually in the account configured via IABot.Account.",
            playerClass);
        return false;
    }

    // Start async login (the callback will call RegisterSession)
    return sIABotLoginMgr->BeginBotLogin(charGuid, master);
}

// ============================================================
// RegisterSession — called by IABotLoginMgr when login completes
// ============================================================
void IABotManager::RegisterSession(ObjectGuid botGuid,
                                    std::unique_ptr<IABotSession> session,
                                    ObjectGuid masterGuid)
{
    sessions_[botGuid] = std::move(session);
    masterBots_[masterGuid].push_back(botGuid);

    LOG_DEBUG("module.iabot", "RegisterSession: bot {} registered to master {}",
              botGuid.GetCounter(), masterGuid.GetCounter());
}

// ============================================================
// RemoveBot — unregister and mark for logout
// ============================================================
void IABotManager::RemoveBot(ObjectGuid guid)
{
    auto it = sessions_.find(guid);
    if (it == sessions_.end())
        return;

    // Unregister from scheduler
    sIABotScheduler->Unregister(guid);

    // Unregister from login manager
    sIABotLoginMgr->OnBotRemoved(guid);

    // Remove from master's bot list
    Player* master = it->second->GetMaster();
    if (master)
    {
        ObjectGuid masterGuid = master->GetGUID();
        auto& bots = masterBots_[masterGuid];
        bots.erase(std::remove(bots.begin(), bots.end(), guid), bots.end());
        if (bots.empty())
            masterBots_.erase(masterGuid);
    }

    // Logout the bot Player properly
    LogoutBot(guid);

    sessions_.erase(it);
    LOG_DEBUG("module.iabot", "Bot removed: GUID={}", guid.GetCounter());
}

// ============================================================
// LogoutBot — properly disconnect a bot from the world
// ============================================================
void IABotManager::LogoutBot(ObjectGuid guid)
{
    auto it = sessions_.find(guid);
    if (it == sessions_.end())
        return;

    Player* bot = it->second->GetBot();
    if (!bot)
        return;

    WorldSession* ws = bot->GetSession();
    if (!ws)
        return;

    // Save character, remove from world, cleanup
    ws->LogoutPlayer(true);  // saveToDb = true

    // Delete the WorldSession we created during login
    delete ws;

    LOG_DEBUG("module.iabot", "Bot {} logged out and WorldSession destroyed", guid.GetCounter());
}

uint32 IABotManager::RemoveAllBots(Player* master)
{
    if (!master)
        return 0;

    ObjectGuid masterGuid = master->GetGUID();
    auto it = masterBots_.find(masterGuid);
    if (it == masterBots_.end())
        return 0;

    // Copy because RemoveBot modifies masterBots_
    std::vector<ObjectGuid> toRemove = it->second;
    for (ObjectGuid botGuid : toRemove)
        RemoveBot(botGuid);

    return static_cast<uint32>(toRemove.size());
}

IABotSession* IABotManager::GetSession(ObjectGuid guid) const
{
    auto it = sessions_.find(guid);
    return (it != sessions_.end()) ? it->second.get() : nullptr;
}

bool IABotManager::IsBot(Player* player) const
{
    return player && sessions_.count(player->GetGUID()) > 0;
}

void IABotManager::OnMasterLogin(Player* master)
{
    LOG_DEBUG("module.iabot", "Master login: {} ('{}')",
              master->GetGUID().GetCounter(), master->GetName());
    // Future: auto-spawn saved bots for this master
}

void IABotManager::OnMasterLogout(Player* master)
{
    LOG_DEBUG("module.iabot", "Master logout: {} — removing all bots",
              master->GetGUID().GetCounter());
    RemoveAllBots(master);
}

void IABotManager::OnTargetDeath(Unit* target, Unit* /*killer*/)
{
    if (!target)
        return;

    // Notify all sessions — they'll check if it was their target
    for (auto& [guid, session] : sessions_)
    {
        // Phase 2: session->GetBrain()->OnTargetDeath(target->GetGUID());
    }
}

void IABotManager::ListBots(ChatHandler* handler, Player* master) const
{
    ObjectGuid masterGuid = master->GetGUID();
    auto it = masterBots_.find(masterGuid);

    if (it == masterBots_.end() || it->second.empty())
    {
        handler->SendSysMessage("IABot: no active bots.");
        return;
    }

    handler->PSendSysMessage("=== IABot: {} active bot(s) ===", it->second.size());
    for (ObjectGuid botGuid : it->second)
    {
        auto sit = sessions_.find(botGuid);
        if (sit == sessions_.end())
            continue;

        IABotSession* s = sit->second.get();
        Player* bot = s->GetBot();
        handler->PSendSysMessage("  {} (GUID {}) | Class {} | Lvl {} | LOD {} | Priority {}",
                                 bot ? bot->GetName() : "<loading>",
                                 botGuid.GetCounter(),
                                 s->GetClass(),
                                 bot ? bot->GetLevel() : 0,
                                 static_cast<uint8>(s->GetLOD()),
                                 static_cast<uint8>(s->GetPriority()));
    }
}
