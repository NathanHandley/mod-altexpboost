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

#include "Configuration/Config.h"
#include "Chat.h"
#include "ScriptMgr.h"
#include "Player.h"

#include "AltExpBoost.h"

using namespace std;

AltExpBoostMod::AltExpBoostMod() :
    IsEnabled(true),
    AnnourceOnLogin(false),
    ShowCurBonusOnLoginAndLevel(true),
    ExtraEXPPercentKill(1)
    //ExtraEXPPercentQuest(1), NYI
    //ExtraEXPPercentDiscover(1) NYI
{
}

AltExpBoostMod::~AltExpBoostMod()
{

}

int AltExpBoostMod::GetNumOfCharHigherThanLoggedInChar()
{
    return 0;
}

float AltExpBoostMod::GetExtraEXPOnKillBonus()
{
    return 0;
}

void AltExpBoostMod::AnnounceCurrentBonus(Player* player)
{
    int numOfAffectingChar = AltExpBoost->GetNumOfCharHigherThanLoggedInChar();
    float extraEXPKillBonus = AltExpBoost->GetExtraEXPOnKillBonus();
    string text = fmt::format("You have |cff4CFF00{}|r characters higher than your current level, granting you |cff4CFF00{}%|r additional experience on kill.", numOfAffectingChar, extraEXPKillBonus);
    ChatHandler(player->GetSession()).SendSysMessage(text);
}




