//  Author: Nathan Handley (nathanhandley@protonmail.com)
//  Copyright (c) 2025 Nathan Handley
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

using namespace std;

class AltExpBoost_PlayerScript : public PlayerScript
{
public:
    AltExpBoost_PlayerScript() : PlayerScript("AltExpBoost_PlayerScript") {}

    void OnPlayerLogin(Player* player) override
    {
        if (AltExpBoost->IsEnabled == false)
            return;
        if (AltExpBoost->DisplayMessageAnnounceAddonInUseOnLogin == true)
            ChatHandler(player->GetSession()).SendSysMessage("This server is running the Alt EXP Boost module.");
        AltExpBoost->LoadInfluencingCharacterLevelsForPlayer(player);
        AltExpBoost->AnnounceCurrentBonus(player);
    }

    void OnPlayerLogout(Player* player) override
    {
        AltExpBoost->InfluencingCharacterLevelsByPlayerGUID.erase(player->GetGUID().GetCounter());
    }

    void OnPlayerLevelChanged(Player* player, uint8 /*oldLevel*/) override
    {
        if (AltExpBoost->IsEnabled == false)
            return;
        AltExpBoost->AnnounceCurrentBonus(player);
    }

    void OnPlayerGiveXP(Player* player, uint32& amount, Unit* /*victim*/, uint8 /*xpSource*/) override
    {
        if (AltExpBoost->IsEnabled == false)
            return;
        amount = amount + (uint32)(amount * AltExpBoost->GetExtraEXPOnKillBonus(player));
    }
};

void AddAltExpBoostPlayerScript()
{
    new AltExpBoost_PlayerScript();
}
