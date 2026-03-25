/*
 * mod-iaplayerbot — Layer 1: GM commands
 * Pattern copied from mod-playerbots PlayerbotCommandScript.cpp
 */

#include "Chat.h"
#include "IABotManager.h"
#include "IABotMetrics.h"
#include "Player.h"
#include "ScriptMgr.h"

using namespace Acore::ChatCommands;

class iabot_commandscript : public CommandScript
{
public:
    iabot_commandscript() : CommandScript("iabot_commandscript") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable iabotDebugTable =
        {
            { "perf", HandleDebugPerfCommand, SEC_GAMEMASTER, Console::Yes },
        };

        static ChatCommandTable iabotBridgeTable =
        {
            { "toggle", HandleBridgeToggleCommand, SEC_GAMEMASTER, Console::Yes },
            { "status", HandleBridgeStatusCommand, SEC_GAMEMASTER, Console::Yes },
        };

        static ChatCommandTable iabotTable =
        {
            { "add",    HandleAddCommand,    SEC_GAMEMASTER, Console::No },
            { "remove", HandleRemoveCommand, SEC_GAMEMASTER, Console::No },
            { "list",   HandleListCommand,   SEC_GAMEMASTER, Console::No },
            { "debug",  iabotDebugTable },
            { "bridge", iabotBridgeTable },
        };

        static ChatCommandTable commandTable =
        {
            { "iabot", iabotTable },
        };

        return commandTable;
    }

    static bool HandleAddCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        if (!args || !*args)
        {
            handler->SendSysMessage("Usage: .iabot add <classId>");
            handler->SendSysMessage("Classes: 1=Warrior 2=Paladin 3=Hunter 4=Rogue 5=Priest 6=DK 7=Shaman 8=Mage 9=Warlock");
            return true;
        }

        uint8 playerClass = static_cast<uint8>(atoi(args));

        if (playerClass < 1 || playerClass > 9)
        {
            handler->SendSysMessage("IABot: invalid class (1-9).");
            return true;
        }

        if (sIABotMgr->AddBot(player, playerClass, player->getRace(), 0))
        {
            handler->PSendSysMessage("IABot: bot login started (Class: %u). Bot will appear shortly.", playerClass);
        }
        else
        {
            handler->SendSysMessage("IABot: failed to add bot. Check logs for details.");
        }

        return true;
    }

    static bool HandleRemoveCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        uint32 count = sIABotMgr->RemoveAllBots(player);
        handler->PSendSysMessage("IABot: %u bot(s) removed.", count);
        return true;
    }

    static bool HandleListCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        sIABotMgr->ListBots(handler, player);
        return true;
    }

    static bool HandleDebugPerfCommand(ChatHandler* handler, char const* /*args*/)
    {
        auto stats = sIABotMetrics->GetGlobalStats();
        handler->PSendSysMessage("=== IABot Performance ===");
        handler->PSendSysMessage("Total bots: %u | Avg bots/tick: %.1f",
                                 stats.totalBots, stats.avgBotsPerTick);
        handler->PSendSysMessage("Avg tick budget: %u us | Overflow rate: %.1f%%",
                                 stats.avgTickBudgetUs, stats.budgetOverflowRate * 100.0f);
        return true;
    }

    static bool HandleBridgeToggleCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->SendSysMessage("IABot: bridge not yet implemented (Phase 4).");
        return true;
    }

    static bool HandleBridgeStatusCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->SendSysMessage("IABot: bridge not yet implemented (Phase 4).");
        return true;
    }
};

void AddIABotCommandScripts()
{
    new iabot_commandscript();
}
