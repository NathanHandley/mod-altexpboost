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
#include "ScriptMgr.h"

#include <set>
#include <string>

#include "AltExpBoost.h"

using namespace std;

class AltExpBoost_WorldScript: public WorldScript
{
public:
    AltExpBoost_WorldScript() : WorldScript("AltExpBoost_WorldScript") {}

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        AltExpBoost->IsEnabled = sConfigMgr->GetOption<bool>("AltExpBoost.Enable", true);
        AltExpBoost->AnnourceOnLogin = sConfigMgr->GetOption<bool>("AltExpBoost.AnnourceOnLogin", false);
        AltExpBoost->ShowCurBonusOnLoginAndLevel = sConfigMgr->GetOption<bool>("AltExpBoost.ShowCurBonusOnLoginAndLevel", true);
        AltExpBoost->DisabledConsideredClassIDs = GetClassIDsFromString(sConfigMgr->GetOption<std::string>("AltExpBoost.DisabledConsideredClassIDs", "6"));
        AltExpBoost->DisabledAppliedClassIDs = GetClassIDsFromString(sConfigMgr->GetOption<std::string>("AltExpBoost.DisabledAppliedClassIDs", ""));
        AltExpBoost->ExtraEXPPercentKill = GetConstrainedAndFormattedEXPPercent(sConfigMgr->GetOption<float>("AltExpBoost.ExtraExpPercentPerChar.Kill", 50));
        //AltExpBoost->ExtraEXPPercentQuest = GetConstrainedAndFormattedEXPPercent(sConfigMgr->GetOption<float>("AltExpBoost.ExtraExpPercentPerChar.Quest", 50)); NYI
        //AltExpBoost->ExtraEXPPercentDiscover = GetConstrainedAndFormattedEXPPercent(sConfigMgr->GetOption<float>("AltExpBoost.ExtraExpPercentPerChar.Discover", 50)); NYI
    }

private:
    std::set<uint32> GetClassIDsFromString(std::string classIDString)
    {
        std::string delimitedValue;
        std::stringstream classIDStream;
        std::set<uint32> classIDs;

        // Grab from the string
        classIDStream.str(classIDString);
        while (std::getline(classIDStream, delimitedValue, ',')) // Process each class ID in the string, delimited by the comma ","
        {
            std::string valueOne;
            std::stringstream classIDStream(delimitedValue);
            classIDStream >> valueOne;
            auto characterGUID = atoi(valueOne.c_str());
            if (classIDs.find(characterGUID) == classIDs.end())
                classIDs.insert(characterGUID);
        }
        return classIDs;
    }

    float GetConstrainedAndFormattedEXPPercent(float inputConfigPercent)
    {
        if (inputConfigPercent <= 0)
            return 0;
        else
            return inputConfigPercent / 100;
    }
};

void AddAltExpBoostWorldScript()
{
    new AltExpBoost_WorldScript();
}
