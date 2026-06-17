//  Author: Nathan Handley (nathanhandley@protonmail.com)
//  Copyright (c) 2026 Nathan Handley
//
//  This program is free software; you can redistribute it and/or modify it
//  under the terms of the GNU Affero General Public License as published by the
//  Free Software Foundation; either version 3 of the License, or (at your
//  option) any later version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.See the GNU Affero General Public License for
//  more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "Chat.h"
#include "Player.h"
#include "ScriptMgr.h"

#include "AltExpBoost.h"

using namespace Acore::ChatCommands;
using namespace std;

class AltExpBoost_CommandScript : public CommandScript
{
public:
    AltExpBoost_CommandScript() : CommandScript("AltExpBoost_CommandScript") { }

    std::vector<ChatCommand> GetCommands() const
    {
        static std::vector<ChatCommand> altExpBoostCommandTable =
        {
            { "on",  HandleAltExpBoostOn,  SEC_PLAYER, Console::No },
            { "off", HandleAltExpBoostOff, SEC_PLAYER, Console::No },
        };

        static std::vector<ChatCommand> commandTable =
        {
            { "altexpboost", altExpBoostCommandTable },
        };
        return commandTable;
    }

    static bool HandleAltExpBoostOn(ChatHandler* handler, const char* /*args*/)
    {
        if (AltExpBoost->IsEnabled == false)
            return true;

        Player* player = handler->GetPlayer();
        if (!player)
            return true;

        AltExpBoost->SetBonusEnabledForPlayer(player, true);
        handler->PSendSysMessage("Alt experience boost is now |cff4CFF00ON|r for this character.");
        AltExpBoost->AnnounceCurrentBonus(player);
        return true;
    }

    static bool HandleAltExpBoostOff(ChatHandler* handler, const char* /*args*/)
    {
        if (AltExpBoost->IsEnabled == false)
            return true;

        Player* player = handler->GetPlayer();
        if (!player)
            return true;

        AltExpBoost->SetBonusEnabledForPlayer(player, false);
        handler->PSendSysMessage("Alt experience boost is now |cffff0000OFF|r for this character.");
        return true;
    }
};

void AddAltExpBoostCommandScript()
{
    new AltExpBoost_CommandScript();
}
