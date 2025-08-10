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
    ExtraEXPPercentKill(0.5)
    //ExtraEXPPercentQuest(0.5), NYI
    //ExtraEXPPercentDiscover(0.5) NYI
{
}

AltExpBoostMod::~AltExpBoostMod()
{

}

int AltExpBoostMod::GetNumOfCharHigherThanLoggedInChar(Player* player)
{
    int numOfHigherChars = 0;
    auto charIter = ConsideredCharacterLevelsByPlayerGUID.find(player->GetGUID().GetCounter());
    if (charIter != ConsideredCharacterLevelsByPlayerGUID.end())
    {
        for (const auto& level : charIter->second)
        {
            if (level > player->GetLevel())
                numOfHigherChars++;
        }
    }

    return numOfHigherChars;
}

float AltExpBoostMod::GetExtraEXPOnKillBonus(Player* player)
{
    int numOfHigherChars = GetNumOfCharHigherThanLoggedInChar(player);
    float bonusAmount = (float)numOfHigherChars * ExtraEXPPercentKill;
    return bonusAmount;
}

void AltExpBoostMod::LoadConsideredCharacterLevelsForPlayer(Player* player)
{
    ConsideredCharacterLevelsByPlayerGUID.erase(player->GetGUID().GetCounter());

    // Some classes are disabled
    if (DisabledAppliedClassIDs.find(player->getClass()) != DisabledAppliedClassIDs.end())
        return;

    // Repopulate the list
    string queryString;
    if (DisabledConsideredClassIDs.empty() == true)
        queryString = fmt::format("SELECT `class`, `level` FROM characters WHERE account = {} AND guid <> {}", player->GetSession()->GetAccountId(), player->GetGUID().GetCounter());
    else
    {
        std::ostringstream classIDStream;
        for (auto it = DisabledConsideredClassIDs.begin(); it != DisabledConsideredClassIDs.end(); ++it)
        {
            if (it != DisabledConsideredClassIDs.begin())
                classIDStream << ",";
            classIDStream << *it;
        }
        queryString = fmt::format("SELECT `level` FROM characters WHERE account = {} AND guid <> {} AND class NOT IN ({})", player->GetSession()->GetAccountId(), player->GetGUID().GetCounter(), classIDStream.str());
    }
    ConsideredCharacterLevelsByPlayerGUID.insert(std::make_pair(player->GetGUID().GetCounter(), std::vector<uint32>()));
    QueryResult queryResult = CharacterDatabase.Query(queryString);
    if (queryResult && queryResult->GetRowCount() > 0)
    {
        do
        {
            Field* fields = queryResult->Fetch();
            uint32 level = fields[0].Get<uint32>();
            ConsideredCharacterLevelsByPlayerGUID[player->GetGUID().GetCounter()].push_back(level);
        } while (queryResult->NextRow());
    }
}

void AltExpBoostMod::AnnounceCurrentBonus(Player* player)
{
    // If configured, some classes have no bonus
    if (DisabledAppliedClassIDs.find(player->getClass()) != DisabledAppliedClassIDs.end())
        return;

    int numOfAffectingChar = AltExpBoost->GetNumOfCharHigherThanLoggedInChar(player);
    uint32 extraEXPKillBonus = (uint32)(AltExpBoost->GetExtraEXPOnKillBonus(player) * 100);
    string text = fmt::format("You have |cff4CFF00{}|r characters higher than your current level, granting you |cff4CFF00{}%|r additional experience on kill.", numOfAffectingChar, extraEXPKillBonus);
    ChatHandler(player->GetSession()).SendSysMessage(text);
}




