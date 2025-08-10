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
    DisplayMessageAnnounceAddonInUseOnLogin(false),
    DisplayMessageBonusOnLoginAndLevelChange(true),
    DisplayMessageIfAboveMaxAppliedCharLevel(true),
    ExtraEXPPercentKill(0.5),
    //ExtraEXPPercentQuest(0.5), NYI
    //ExtraEXPPercentDiscover(0.5) NYI
    MinInfluencingCharLevel(10),
    MaxAppliedCharLevel(0)
{
}

AltExpBoostMod::~AltExpBoostMod()
{

}

int AltExpBoostMod::GetNumOfInfluencingCharHigherThanLoggedInChar(Player* player)
{
    int numOfHigherChars = 0;
    auto charIter = InfluencingCharacterLevelsByPlayerGUID.find(player->GetGUID().GetCounter());
    if (charIter != InfluencingCharacterLevelsByPlayerGUID.end())
    {
        for (const auto& level : charIter->second)
        {
            if (level <= player->GetLevel())
                continue;
            if (MinInfluencingCharLevel > 0 && level < MinInfluencingCharLevel)
                continue;
            numOfHigherChars++;
        }
    }

    return numOfHigherChars;
}

float AltExpBoostMod::GetExtraEXPOnKillBonus(Player* player)
{
    if (MaxAppliedCharLevel > 0 && player->GetLevel() > MaxAppliedCharLevel)
        return 0;
    int numOfHigherChars = GetNumOfInfluencingCharHigherThanLoggedInChar(player);
    float bonusAmount = (float)numOfHigherChars * ExtraEXPPercentKill;
    return bonusAmount;
}

void AltExpBoostMod::LoadInfluencingCharacterLevelsForPlayer(Player* player)
{
    InfluencingCharacterLevelsByPlayerGUID.erase(player->GetGUID().GetCounter());

    // Some classes are disabled
    if (DisabledAppliedClassIDs.find(player->getClass()) != DisabledAppliedClassIDs.end())
        return;

    // Repopulate the list
    string queryString;
    if (DisabledInfluencingClassIDs.empty() == true)
        queryString = fmt::format("SELECT `class`, `level` FROM characters WHERE account = {} AND guid <> {}", player->GetSession()->GetAccountId(), player->GetGUID().GetCounter());
    else
    {
        std::ostringstream classIDStream;
        for (auto it = DisabledInfluencingClassIDs.begin(); it != DisabledInfluencingClassIDs.end(); ++it)
        {
            if (it != DisabledInfluencingClassIDs.begin())
                classIDStream << ",";
            classIDStream << *it;
        }
        queryString = fmt::format("SELECT `level` FROM characters WHERE account = {} AND guid <> {} AND class NOT IN ({})", player->GetSession()->GetAccountId(), player->GetGUID().GetCounter(), classIDStream.str());
    }
    InfluencingCharacterLevelsByPlayerGUID.insert(std::make_pair(player->GetGUID().GetCounter(), std::vector<uint32>()));
    QueryResult queryResult = CharacterDatabase.Query(queryString);
    if (queryResult && queryResult->GetRowCount() > 0)
    {
        do
        {
            Field* fields = queryResult->Fetch();
            uint32 level = fields[0].Get<uint32>();
            InfluencingCharacterLevelsByPlayerGUID[player->GetGUID().GetCounter()].push_back(level);
        } while (queryResult->NextRow());
    }
}

void AltExpBoostMod::AnnounceCurrentBonus(Player* player)
{
    // If configured, some classes have no bonus
    if (DisabledAppliedClassIDs.find(player->getClass()) != DisabledAppliedClassIDs.end())
        return;

    // If the player is above the maximum, tell them
    if (MaxAppliedCharLevel > 0 && player->GetLevel() > MaxAppliedCharLevel)
    {
        if (DisplayMessageBonusOnLoginAndLevelChange == true)
        {
            string text = fmt::format("You will not receive any bonus experience from other characters due to being above level {}", MaxAppliedCharLevel);
            ChatHandler(player->GetSession()).SendSysMessage(text);
        }
        return;
    }

    if (DisplayMessageBonusOnLoginAndLevelChange == false)
        return;

    // If there's a minimum, show a message about it
    int numOfAffectingChar = AltExpBoost->GetNumOfInfluencingCharHigherThanLoggedInChar(player);
    string minLevelFragment = "";
    if (MinInfluencingCharLevel > 0)
        minLevelFragment = fmt::format(" (and > level {})", MinInfluencingCharLevel); 
    uint32 extraEXPKillBonus = (uint32)(AltExpBoost->GetExtraEXPOnKillBonus(player) * 100);
    string text = fmt::format("You have |cff4CFF00{}|r characters higher than your current level{}, granting you |cff4CFF00{}%|r additional experience on kill.", numOfAffectingChar, minLevelFragment, extraEXPKillBonus);
    ChatHandler(player->GetSession()).SendSysMessage(text);
}




