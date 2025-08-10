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

#ifndef ALTEXPBOOST_H
#define ALTEXPBOOST_H

#include "Common.h"
#include "Player.h"

#include <vector>
#include <map>
#include <set>

class AltExpBoostMod
{
private:
    AltExpBoostMod();

public:
    static AltExpBoostMod* instance()
    {
        static AltExpBoostMod instance;
        return &instance;
    }
    ~AltExpBoostMod();

    bool IsEnabled;
    bool DisplayMessageAnnounceAddonInUseOnLogin;
    bool DisplayMessageBonusOnLoginAndLevelChange;
    bool DisplayMessageIfAboveMaxAppliedCharLevel;
    std::set<uint32> DisabledInfluencingClassIDs;
    std::set<uint32> DisabledAppliedClassIDs;
    float ExtraEXPPercentKill;
    uint32 MinInfluencingCharLevel;
    uint32 MaxAppliedCharLevel;
    //float ExtraEXPPercentQuest; NYI
    //float ExtraEXPPercentDiscover; NYI
    std::map<ObjectGuid::LowType, std::vector<uint32>> InfluencingCharacterLevelsByPlayerGUID;

    int GetNumOfInfluencingCharHigherThanLoggedInChar(Player* player);
    float GetExtraEXPOnKillBonus(Player* player);

    void LoadInfluencingCharacterLevelsForPlayer(Player* player);
    void AnnounceCurrentBonus(Player* player);
};

#define AltExpBoost AltExpBoostMod::instance()

#endif //ALTEXPBOOST_H
