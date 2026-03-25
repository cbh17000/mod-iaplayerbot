/*
 * mod-iaplayerbot — Module entry point
 * Registers all hook scripts with AzerothCore ScriptMgr
 */

#include "IABot_PlayerScript.h"
#include "IABot_WorldScript.h"
#include "IABot_UnitScript.h"
#include "IABot_CommandScript.h"

void Addmod_iaplayerbotScripts()
{
    new IABot_PlayerScript();
    new IABot_WorldScript();
    new IABot_UnitScript();
    AddIABotCommandScripts();
}
