/*
 * mod-iaplayerbot — Layer 2: Bot session implementation
 */

#include "IABotSession.h"
#include "IABotConfig.h"
#include "Player.h"
#include "Log.h"

IABotSession::IABotSession(Player* bot, Player* master, uint8 playerClass, uint8 race, uint8 spec)
    : bot_(bot)
    , master_(master)
    , playerClass_(playerClass)
    , race_(race)
    , spec_(spec)
    , lod_(AILOD::FULL)
    , priority_(BotPriority::MEDIUM)
{
    // Brain and ClassModule created in Phase 2
}

IABotSession::~IABotSession() = default;

ObjectGuid IABotSession::GetBotGUID() const
{
    return bot_ ? bot_->GetGUID() : ObjectGuid::Empty;
}

void IABotSession::RecalculateLOD()
{
    if (!bot_ || !bot_->IsInWorld())
    {
        lod_ = AILOD::MINIMAL;
        return;
    }

    // Combat always gets FULL
    if (bot_->IsInCombat())
    {
        lod_ = AILOD::FULL;
        return;
    }

    // No master or master offline → MINIMAL
    if (!master_ || !master_->IsInWorld())
    {
        lod_ = AILOD::MINIMAL;
        return;
    }

    // Distance-based LOD
    float dist = bot_->GetDistance(master_);

    if (dist <= sIABotConfig->LODDistanceFull)
        lod_ = AILOD::FULL;
    else if (dist <= sIABotConfig->LODDistanceReduced)
        lod_ = AILOD::REDUCED;
    else
        lod_ = AILOD::MINIMAL;
}

void IABotSession::Update(uint32 diff)
{
    if (!bot_ || !bot_->IsInWorld())
        return;

    RecalculateLOD();

    switch (lod_)
    {
        case AILOD::FULL:
            // Phase 2: brain_->Think(diff);
            // For now, just follow master
            UpdateMinimal(diff);
            break;

        case AILOD::REDUCED:
            // Phase 2: brain_->ThinkReduced(diff);
            UpdateMinimal(diff);
            break;

        case AILOD::MINIMAL:
            UpdateMinimal(diff);
            break;
    }
}

void IABotSession::UpdateMinimal(uint32 /*diff*/)
{
    if (!bot_ || bot_->isDead())
        return;

    // Follow master if too far
    if (master_ && master_->IsInWorld() && bot_->GetMapId() == master_->GetMapId())
    {
        float dist = bot_->GetDistance(master_);
        if (dist > sIABotConfig->FollowDistance * 2.0f)
        {
            bot_->GetMotionMaster()->MoveFollow(master_, sIABotConfig->FollowDistance, bot_->GetFollowAngle());
        }
    }

    // Auto-regen: sit if low health and not in combat
    if (!bot_->IsInCombat())
    {
        if (bot_->GetHealthPct() < 50.0f && bot_->getStandState() != UNIT_STAND_STATE_SIT)
            bot_->SetStandState(UNIT_STAND_STATE_SIT);
        else if (bot_->GetHealthPct() > 90.0f && bot_->getStandState() == UNIT_STAND_STATE_SIT)
            bot_->SetStandState(UNIT_STAND_STATE_STAND);
    }
}

// --- Events ---

void IABotSession::OnLogin()
{
    LOG_DEBUG("module.iabot", "IABotSession::OnLogin bot={}", GetBotGUID().GetCounter());
}

void IABotSession::OnDeath(Unit* /*killer*/)
{
    LOG_DEBUG("module.iabot", "IABotSession::OnDeath bot={}", GetBotGUID().GetCounter());
    // Phase 2: brain will handle dead state + corpse run
}

void IABotSession::OnResurrect()
{
    LOG_DEBUG("module.iabot", "IABotSession::OnResurrect bot={}", GetBotGUID().GetCounter());
}

void IABotSession::OnDamageTaken(Unit* /*attacker*/)
{
    // Phase 2: invalidate perception cache for immediate threat refresh
    // perception_->InvalidateCache();
}

void IABotSession::OnWhisperReceived(Player* /*sender*/, const std::string& /*msg*/)
{
    // Phase 2: route to Brain's command parser
}
