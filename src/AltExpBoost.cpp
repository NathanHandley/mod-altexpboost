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
    EnableForEverQuestMod(false),
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
    if (GetBonusEnabledForPlayer(player) == false)
        return 0;
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

    uint32 accountID = player->GetSession()->GetAccountId();
    uint32 playerGUID = player->GetGUID().GetCounter();

    // Build a comma separated list of any classes that should not influence the bonus
    std::string disabledClassList;
    if (DisabledInfluencingClassIDs.empty() == false)
    {
        std::ostringstream classIDStream;
        for (auto it = DisabledInfluencingClassIDs.begin(); it != DisabledInfluencingClassIDs.end(); ++it)
        {
            if (it != DisabledInfluencingClassIDs.begin())
                classIDStream << ",";
            classIDStream << *it;
        }
        disabledClassList = classIDStream.str();
    }

    // Repopulate the list
    string queryString;
    if (EnableForEverQuestMod == true)
    {
        // When running alongside the mod-everquest module, a single character can hold additional
        // secondary classes in mod_everquest_characters, each with its own stored level.  Treat each
        // influencing character's level as the highest level between its active class (characters.level)
        // and any of its stored secondary classes (mod_everquest_characters.level).  For example, a
        // character that is level 10 on its active class but has a secondary class at level 25 counts as 25.
        std::string charClassFilter = disabledClassList.empty() == true ? "" : fmt::format(" AND c.class NOT IN ({})", disabledClassList);
        std::string eqClassFilter = disabledClassList.empty() == true ? "" : fmt::format(" AND eq.class NOT IN ({})", disabledClassList);
        queryString = fmt::format(
            "SELECT GREATEST(c.`level`, IFNULL((SELECT MAX(eq.`level`) FROM mod_everquest_characters eq WHERE eq.guid = c.guid{}), 0)) "
            "FROM characters c WHERE c.account = {} AND c.guid <> {}{}",
            eqClassFilter, accountID, playerGUID, charClassFilter);
    }
    else if (disabledClassList.empty() == true)
        queryString = fmt::format("SELECT `level` FROM characters WHERE account = {} AND guid <> {}", accountID, playerGUID);
    else
        queryString = fmt::format("SELECT `level` FROM characters WHERE account = {} AND guid <> {} AND class NOT IN ({})", accountID, playerGUID, disabledClassList);
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

    if (DisplayMessageBonusOnLoginAndLevelChange == false)
        return;

    // Current per-character toggle state, shown green when on and red when off
    bool bonusEnabled = GetBonusEnabledForPlayer(player);
    string statusFragment = bonusEnabled == true ? "|cff4CFF00ON|r" : "|cffff0000OFF|r";

    // If the bonus has been turned off for this character, just report that
    if (bonusEnabled == false)
    {
        string text = fmt::format("Alt experience boost is currently {} for this character. Type '.altexpboost on' to enable it.", statusFragment);
        ChatHandler(player->GetSession()).SendSysMessage(text);
        return;
    }

    // If the player is above the maximum, tell them
    if (MaxAppliedCharLevel > 0 && player->GetLevel() > MaxAppliedCharLevel)
    {
        string text = fmt::format("Alt experience boost is {}, but you will not receive any bonus experience from other characters due to being above level {}", statusFragment, MaxAppliedCharLevel);
        ChatHandler(player->GetSession()).SendSysMessage(text);
        return;
    }

    // If there's a minimum, show a message about it
    int numOfAffectingChar = AltExpBoost->GetNumOfInfluencingCharHigherThanLoggedInChar(player);
    string minLevelFragment = "";
    if (MinInfluencingCharLevel > 0)
        minLevelFragment = fmt::format(" (and > level {})", MinInfluencingCharLevel);
    uint32 extraEXPKillBonus = (uint32)(AltExpBoost->GetExtraEXPOnKillBonus(player) * 100);
    string charactersStringPluralS = "";
    if (numOfAffectingChar != 1)
        charactersStringPluralS = "s";
    string text = fmt::format("Alt experience boost is {}. You have |cff4CFF00{}|r character{} higher than your current level{}, granting you |cff4CFF00{}%|r additional experience on kill. Disable with '.altexpboost off'", statusFragment, numOfAffectingChar, charactersStringPluralS, minLevelFragment, extraEXPKillBonus);
    ChatHandler(player->GetSession()).SendSysMessage(text);
}

bool AltExpBoostMod::GetBonusEnabledForPlayer(Player* player)
{
    auto enabledIter = BonusEnabledByPlayerGUID.find(player->GetGUID().GetCounter());
    if (enabledIter != BonusEnabledByPlayerGUID.end())
        return enabledIter->second;
    // Default to enabled when no preference has been stored or loaded
    return true;
}

void AltExpBoostMod::LoadBonusEnabledForPlayer(Player* player)
{
    uint32 playerGUID = player->GetGUID().GetCounter();
    bool enabled = true;
    QueryResult queryResult = CharacterDatabase.Query("SELECT `enabled` FROM mod_altexpboost_character_settings WHERE guid = {}", playerGUID);
    if (queryResult && queryResult->GetRowCount() > 0)
    {
        Field* fields = queryResult->Fetch();
        enabled = fields[0].Get<uint8>() == 1 ? true : false;
    }
    BonusEnabledByPlayerGUID[playerGUID] = enabled;
}

void AltExpBoostMod::SetBonusEnabledForPlayer(Player* player, bool enabled)
{
    uint32 playerGUID = player->GetGUID().GetCounter();
    CharacterDatabase.DirectExecute("REPLACE INTO `mod_altexpboost_character_settings` (`guid`, `enabled`) VALUES ({}, {})", playerGUID, enabled == true ? 1 : 0);
    BonusEnabledByPlayerGUID[playerGUID] = enabled;
}

void AltExpBoostMod::PerformPlayerDelete(ObjectGuid guid)
{
    uint32 playerGUID = guid.GetCounter();
    CharacterDatabase.DirectExecute("DELETE FROM mod_altexpboost_character_settings WHERE guid = {}", playerGUID);
    BonusEnabledByPlayerGUID.erase(playerGUID);
}




